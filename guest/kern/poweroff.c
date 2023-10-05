#include "poweroff.h"

void poweroff_halt(llong rc)
{
    poweroff_dev->poweroff(rc);
}

void register_poweroff(poweroff_device_t *dev)
{
    poweroff_dev = dev;
    if (dev->init) {
        dev->init();
    }
}

static void default_poweroff(llong status)
{
    asm volatile("ud2");
    while (1) {
        asm volatile("" : : : "memory");
    }
}

poweroff_device_t poweroff_none = {
    0,
    default_poweroff,
};

poweroff_device_t *poweroff_dev = &poweroff_none;
