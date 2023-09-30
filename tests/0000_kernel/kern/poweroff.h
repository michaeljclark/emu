#pragma once

#ifdef __cplusplus
extern "C" {
#endif

void poweroff_halt(int rc);

typedef struct poweroff_device {
    void (*init)();
    void (*poweroff)(int);
} poweroff_device_t;

void register_poweroff(poweroff_device_t *dev);

extern poweroff_device_t *poweroff_dev;
extern poweroff_device_t poweroff_none;
extern poweroff_device_t poweroff_cf9;

#ifdef __cplusplus
}
#endif
