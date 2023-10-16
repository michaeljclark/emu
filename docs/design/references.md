# References

Work in progress draft on the design of a reference counting system.

## Primitive operations on reference counters

Summary of atomic operations required to implement reference counting.
All reference counter operations take a pointer to a reference counter
object. Reference to *relaxed*, *acquire* and *release* memory ordering on
the primitive operations primarily refer to the barrier that protects the
reference counted ordering invariant in regards to operations on the
protected object, not the ordering required to perform the operation itself.

An atomic fetch and add implicitly requires atomicity, and thus an *acquire*
for the atomic fetch and add, but there is no ordering barrier in between an
operation that adds to the reference counter and operations on its invariant
object as the thread implicitly owns the data after successfully incrementing
the reference counter, thus reference counter addition does not require a
*release* barrier. Also conditionally adding to reference counters, such as
when acquiring weak references, can use a *relaxed* load to fetch the counter
value to avoid unnecessary cache traffic.

On the other hand, an atomic fetch and subtract requires *release* memory
ordering to create a barrier between the reference counter going to zero and
running destructors so that the zero reference counter is visible to other
threads before running the destructor. Synchronization on the reference
counter going to zero is why operations that subtract from reference counters
are specified with *release* ordering whereas operations that add to
reference counters used *relaxed* ordering to read the counter value. This
is in light of cache contention on conditionally incrementing reference
counters, which if unsuccessful will only have issued *relaxed* loads thus
avoiding unnecessary cache traffic.

### Adding to reference counters

_Unconditionally adding to reference counters_ requires *acquire* memory
ordering to update the counter value, but there is *no* release barrier after
the operation as the thread continues to own the cache line and data.
_Conditionally adding to reference counters_ requires *relaxed* memory
ordering to fetch the counter value, and *acquire* memory ordering to
update the counter value using a compare and swap.

### Subtracting from reference counters

_Unconditionally subtracting from reference counters_ requres *acquire* then
*release* memory ordering as releasing references is the synchronization
point where the cache line is made visible to other threads, before
destructors are run. This is why subtraction requires *release* memory
ordering as there is a barrier after the operation. _Conditionally
subtracting from reference counters_ requires *relaxed* memory ordering to
fetch the counter value, and *acquire* then *release* memory ordering to
update the counter value using a compare and swap, then finally make the
value visible to other threads.

### `rc_val rc_read( rc_t * )`
_rc-read_ performs atomic fetch of the reference counter,
with *acquire* memory ordering. Returns reference count.

### `rc_val rc_write( rc_t *, rc_val n )`
_rc-write_ performs atomic store to the reference counter,
with *release* memory ordering. Returns new reference count.

### `rc_val rc_add( rc_t *, rc_val n )`
_rc-add_ performs unconditional atomic fetch and add _n_ to the reference
counter, with *acquire* memory ordering. Returns new reference count.

### `rc_val rc_sub( rc_t *, rc_val n )`
_rc-sub_ performs unconditional atomic fetch and subtract _n_ from the
reference count, with *acquire* then *release* memory ordering. Returns new
reference count.

### `rc_val rc_inc( rc_t * )`
_rc-inc_ performs unconditional atomic fetch and add one to the reference
counter, with *acquire* memory ordering. returns new reference count.

### `rc_val rc_dec( rc_t * )`
_rc-dec_ performs unconditional atomic fetch and subtract one to the
reference count, with *acquire* then *release* memory ordering. Returns new
reference count.

## Acquiring references

_rc-inc_ is used to unconditionally increment the reference counter because
new objects are created with a non-zero reference counter and clients
incrementing the reference counter already hold a reference to the object
so they can be sure the reference counter is non zero when adding to it.

## Releasing references

### `bool rc_sub_test_zero( rc_t *, rc_val n )`
_rc-sub-test-zero_ is used for releasing references.
_rc-sub-test-zero_ performs an unconditional atomic fetch and subtract,
with *relaxed* ordering to read the counter value and *release* memory
ordering to update it, returning true if the value after subtraction
is zero. If the value is zero, an *acquire* fence is issued and the
client must call the object's destructor.

### `bool rc_dec_test_zero( rc_t * )`
_rc-dec-test-zero_ same as prior only the subtrahend is _one_.

## Weak references

The design specifies weak references to be heap indirect handles that
contain a pointer to a reference counted object without increasing its
reference count, and a delete notifier is registered on the target object
to zero the weak reference pointer during ordered destruction. Zeroing only
ever takes place on the heap never the stack. This is the reason why there
are operations that expect to read a zero reference counter, because they
are reading the reference count *during* object destruction.

## Acquiring references from weak references

### `bool rc_add_unless_zero( rc_t *, rc_val n )`
_rc-add-unless-zero_ is used for acquiring references from weak-references.
_rc-add-unless-zero_ performs a *relaxed* read of the reference counter and
if the value is not zero it will perform an atomic compare and swap of the
prior value and the prior value plus the addend. A compare and swap loop is
required to handle the case where there is an intervening write between the
checking for non zero value and writing back the result. The value is
written back with relaxed memory ordering. Returns true if the reference
counter is zero. This operation must check overflow.

There is a point in-between the reference counter going to zero when zeroing
weak references via handles on the heap, to remove their notifiers, and
running destructors for the object and freeing its memory. This is why there
are some operations conditioned on reading a zero reference counter value
because this occurs during an ordered destruction where the reference
counter is zero but the object is still visible to weak reference holders.

### `bool rc_inc_unless_zero( rc_t * )`
_rc-inc-unless-zero_ same as prior only the addend is _one_.

## Releasing acquired references from weak references

### `bool rc_sub_unless_one( rc_t *, rc_val n )`
_rc-sub-unless-one_ is used for releasing an acquired reference from a weak
reference. If caller holds an acquired weak reference, the reference count
must be non-zero and they must call the destructor if the count is one.
_rc-sub-unless-one_ performs a *relaxed* read of the reference counter and if
not one performs an atomic compare and swap of the prior value and the prior
value minus the subtrahend. A compare and swap loop is required to handle
the case where there is an intervening write between the checking for not
one and writing back the result. the value is written back with release
memory ordering. returns true if the reference counter is one, in which case
the client must call the object's destructor.

### `bool rc_dec_unless_one( rc_t * )`
_rc-dec-unless-one_ same as prior only the subtrahend is _one_.
