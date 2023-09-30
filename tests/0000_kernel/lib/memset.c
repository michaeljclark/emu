#include "string.h"

void *memset(void *dest, int c, size_t n)
{
    char *p = dest;
    while (n-- > 0) {
        *(char*)dest++ = c;
    }
    return p;
}
