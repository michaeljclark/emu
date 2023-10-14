#include "stddef.h"
#include "stdio.h"
#include "stdint.h"
#include "stdlib.h"
#include "string.h"
#include "alloca.h"
#include "console.h"
#include "poweroff.h"
#include "types.h"
#include "x86_intrin.h"

void mem_setup();
int main(int argc, char **argv);

void arch_setup()
{
    register_console(&console_ns16550a);
    register_poweroff(&poweroff_vmm);
    mem_setup();
}

void start_c(void)
{
    char *argv[] = { "emu", NULL };
    arch_setup();
    exit(main(1, argv));
}
