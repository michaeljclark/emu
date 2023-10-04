#include "stdlib.h"
#include "poweroff.h"

void exit(int status)
{
    poweroff_halt(status);
    __builtin_unreachable();
}
