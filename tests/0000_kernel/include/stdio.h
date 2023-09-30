#pragma once

#ifdef __cplusplus
extern "C" {
#endif

#include "stdarg.h"
#include "stddef.h"

int getchar(void);
int printf(const char *, ...) __attribute__((format(printf,1,2)));;
int putchar(int);
int puts(const char *);
int snprintf(char *, size_t, const char *, ...)  __attribute__((format(printf,3,4)));
int vprintf(const char *, va_list) __attribute__((format(printf,1,0)));
int vsnprintf(char *, size_t, const char *, va_list)  __attribute__((format(printf,3,0)));

#ifdef __cplusplus
}
#endif
