/*
 * Ticket Lock
 *
 * Ticket locks are fair spinlocks that order acccess on a first-in,
 * first-out basis. The lock is composed of a head counter and a tail
 * counter. The head counter indicates the ticket number of the next
 * lock owner. The tail counter indicates the next issued ticket number.
 * To acquire the lock, the acquiring thread atomically fetches and
 * increments the tail counter to assign itself a ticket number and set
 * the value of the next available ticket number. It then waits until
 * its assigned ticket number is seen in the head counter. If the lock
 * is not contended, then head will equal the ticket number just assigned.
 * To release the lock, the lock owner atomically increments head counter
 * thereby allowing access to the thread holding the next ticket number.
 *
 * The head value is biased so that the lock can be zero initialized.
 *
 * To support a lock-free trylock, lock and unlock round and incremented
 * tail by two and trylock increments tail by one, so that it can get a
 * ticket number unconditionally and notify to the next trylock or locker
 * that a lock was taken or attempted to be taken via trylock. trylock
 * fails immediately if tail is odd, then gets the ticket number in tail
 * per usual, and increments tail by one. it now may or may no have the
 * ticket based on the current head value which it returns. it holds the
 * lock because its increment by one is rounded up by lock and unlock.
 *
 * Initializtion - the lock is initialized unlocked, so that the
 * next tail increment returns a ticket that will acquires the lock.
 *
 *   tail   | head
 *   -------|-------
 *   0x0000 | 0x0000
 *
 * Lock - increments tail by one and bit-ands with the complement of one
 * to round tail to an even ticket number, then increments tail by two.
 * the thread then waits until head equals its ticket before returning
 * control, which is the normal case if the lock is uncontended.
 *
 *   tail   | head
 *   -------|-------
 *   0x0002 | 0x0000
 *
 * Unlock - the head is incremented by two, so that head will now equal
 * the value of the ticket of the next waiter, if any. tail is rounded
 * to even so that unlock for a lock taken with trylock will allow other
 * trylocks to advance, as they return immediately if the tail is odd.
 *
 *   tail   | head
 *   -------|-------
 *   0x0002 | 0x0002
 *
 * TryLock - returns false if tail is odd because we raced with another
 * trylock that is in progress. if even, increments tail by one. and
 * returns true if our ticket is next in the queue. lock code will fix
 * the odd tail in the next lock request.
 *
 *   tail   | head
 *   -------|-------
 *   0x0003 | 0x0002
 */

#include "spinlock.h"
#include "atomic.h"

void spinlock_lock(spinlock *l)
{
    spinlock s;
    ullong lve, lvd;
    uint ticket;

    lve = atomic_load((atomic_ullong*)&l->lockval);

    /* round up odd tail from potential collision with trylock
     * then advance two-way ticket by incrementing tail by two. */
    do {
        s.lockval = lve;
        s.tail = (s.tail + 1) & ~1;
        ticket = s.tail;
        s.tail = s.tail + 2;
        lvd = s.lockval;
    } while (!atomic_compare_exchange_weak(
        (atomic_ullong*)&l->lockval, &lve, lvd
    ));

    /* wait for our ticket */
    while (ticket != s.head) {
        s.lockval = atomic_load((atomic_ullong*)&l->lockval);
    }
}

int spinlock_trylock(spinlock *l)
{
    spinlock s;
    ullong lve, lvd;
    uint ticket;

    lve = atomic_load((atomic_ullong*)&l->lockval);

    s.lockval = lve;

    /* fail because another trylock is in progress. */
    if ((s.tail & 1) == 1) return 0;

    /* get one-way ticket by incrementing tail by one to notify
     * next lock or unlock request to return our ticket. */
    do {
        s.lockval = lve;
        ticket = s.tail;
        s.tail = s.tail + 1;
        lvd = s.lockval;
    } while (!atomic_compare_exchange_weak(
        (atomic_ullong*)&l->lockval, &lve, lvd
    ));

    s.lockval = lve;

    /* next lock taker will round up an odd tail value. */
    return (ticket == s.head);
}

void spinlock_unlock(spinlock *l)
{
    spinlock s;
    ullong lve, lvd;

    lve = atomic_load((atomic_ullong*)&l->lockval);

    /* return two-way ticket. fix odd tail if lock was taken by trylock
     * so that other trylocks can advance, as they bail if tail is odd. */
    do {
        s.lockval = lve;
        s.head = s.head + 2;
        s.tail = (s.tail + 1) & ~1;
        lvd = s.lockval;
    } while (!atomic_compare_exchange_weak(
        (atomic_ullong*)&l->lockval, &lve, lvd
    ));
}
