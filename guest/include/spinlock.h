#pragma once

#include "types.h"

typedef struct spinlock spinlock;

struct spinlock {
    union {
        ullong lockval;
        struct { uint head, tail; };
    };
};

void spinlock_lock(spinlock *l);
int spinlock_trylock(spinlock *l);
void spinlock_unlock(spinlock *l);
