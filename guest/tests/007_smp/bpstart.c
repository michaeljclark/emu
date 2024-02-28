#include "stddef.h"
#include "stdio.h"
#include "stdint.h"
#include "stdlib.h"
#include "string.h"
#include "console.h"
#include "poweroff.h"
#include "x86_mmu.h"
#include "x86_cpu.h"
#include "x86_lapic.h"
#include "x86_intrin.h"
#include "page.h"
#include "vmm.h"
#include "stack.h"
#include "mem.h"

void mem_setup();
int main(int argc, char **argv);

static inline void arch_boot()
{
    register_console(&console_ns16550a);
    register_poweroff(&poweroff_vmm);
    ncpus = vmm_query_ncpus();
    mem_setup();
    x86_boot_cpu_init();
    x86_cpu_init(0);
}

static int arch_main(int argc, char **argv)
{
    int ret;
    x86_lapic_init(0);
    x86_sti();
    ret = main(argc, argv);
    x86_wfi();
    return ret;
}

void bpstart_c(void)
{
    char *argv[] = { "emu", NULL };
    arch_boot();
    exit(pivot_stack(1, argv, cpulist[0].kstack, arch_main));
}
