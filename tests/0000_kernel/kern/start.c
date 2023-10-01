#include "stddef.h"
#include "stdio.h"
#include "stdint.h"
#include "stdlib.h"
#include "string.h"
#include "console.h"
#include "poweroff.h"
#include "types.h"
#include "x86_intrin.h"

extern char _memory_start;
extern char _memory_end;

void setup()
{
    register_console(&console_ns16550a);
    register_poweroff(&poweroff_cf9);
     _malloc_addblock(&_memory_start, &_memory_end - &_memory_start);
}

void cpu_info(char* cpu_name)
{
    int leaf_0[4], leaf_2[4], leaf_3[4], leaf_4[4];
    
    x86_cpuid(leaf_0, 0x80000000, 0);

    if (leaf_0[0] >= 0x80000004)
    {
        x86_cpuid(leaf_2, 0x80000002, 0);
        x86_cpuid(leaf_3, 0x80000003, 0);
        x86_cpuid(leaf_4, 0x80000004, 0);
        memcpy(cpu_name + 0x00, leaf_2, 0x10);
        memcpy(cpu_name + 0x10, leaf_3, 0x10);
        memcpy(cpu_name + 0x20, leaf_4, 0x10);
    }
}

char cpu_name[64];

void start_c(void)
{
    setup();
    cpu_info(cpu_name);
    printf("# emu kernel: version %d.%d\n", 0, 1);
    printf("# cpu model: %s\n", cpu_name);
    printf("# kernel heap %p - %p\n", &_memory_start, &_memory_end);
    poweroff_halt(0);
}
