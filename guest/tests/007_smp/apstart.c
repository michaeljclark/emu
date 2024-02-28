#include "stdlib.h"
#include "x86_cpu.h"
#include "x86_lapic.h"
#include "x86_intrin.h"
#include "spinlock.h"

static spinlock init_lock;

void apstart_c(int cpu)
{
    spinlock_lock(&init_lock);
    x86_cpu_init(cpu);
    x86_lapic_init(cpu);
    spinlock_unlock(&init_lock);
    x86_sti();
    x86_wfi();
}
