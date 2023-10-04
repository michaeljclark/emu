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

int main(int argc, char **argv);

void start_c(void)
{
    char *argv[] = { "emu", NULL };
    setup();
    exit(main(1, argv));
}
