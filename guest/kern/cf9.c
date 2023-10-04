#include "types.h"
#include "x86_intrin.h"
#include "poweroff.h"

void cf9_poweroff(int rc)
{
    x86_outw(0xcf9, 0x800);
}

poweroff_device_t poweroff_cf9 = {
    0,
    cf9_poweroff,
};
