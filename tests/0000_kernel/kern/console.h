#pragma once

#ifdef __cplusplus
extern "C" {
#endif

int console_putchar(int ch);

typedef struct console_device {
    void (*init)();
    int (*getchar)();
    int (*putchar)(int);
} console_device_t;

void register_console(console_device_t *dev);

extern console_device_t *console_dev;
extern console_device_t console_none;
extern console_device_t console_ns16550a;

#ifdef __cplusplus
}
#endif
