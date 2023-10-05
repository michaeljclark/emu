#pragma once

#ifdef __cplusplus
extern "C" {
#endif

#include "types.h"

void poweroff_halt(llong rc);

typedef struct poweroff_device {
    void (*init)();
    void (*poweroff)(llong);
} poweroff_device_t;

void register_poweroff(poweroff_device_t *dev);

extern poweroff_device_t *poweroff_dev;
extern poweroff_device_t poweroff_none;
extern poweroff_device_t poweroff_cf9;
extern poweroff_device_t poweroff_vmm;

#ifdef __cplusplus
}
#endif
