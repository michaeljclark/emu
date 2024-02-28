#include "stddef.h"
#include "stdio.h"
#include "string.h"
#include "x86_intrin.h"
#include "x86_cpu.h"
#include "spinlock.h"

int main(int argc, const char **argv)
{
    printf("# emu kernel: version %d.%d\n", 0, 1);
    printf("# cpu model: %s\n", cpulist[0].cpu_name);
    return 0;
}
