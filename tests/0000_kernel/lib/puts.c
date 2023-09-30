#include "stdio.h"

int puts(const char *s)
{
    const char *p = s;
    while (*p) putchar(*p++);
    putchar('\n');
    return 1 + p - s;
}
