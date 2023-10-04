#include "console.h"

void register_console(console_device_t *dev)
{
    console_dev = dev;
    if (dev->init) {
        dev->init();
    }
}

static int default_getchar()
{
    asm volatile("ud2");
    return 0;
}

static int default_putchar(int ch)
{
    asm volatile("ud2");
    return 0;
}

console_device_t console_none = {
    0,
    default_getchar,
    default_putchar
};

console_device_t *console_dev = &console_none;
