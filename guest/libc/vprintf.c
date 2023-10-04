#include "stdio.h"
#include "alloca.h"

int vprintf(const char* s, va_list vl)
{
    va_list vlc;
    char *out;
    va_copy(vlc, vl);
    int res = vsnprintf(NULL, 0, s, vl);
    out = alloca(res + 1);
    vsnprintf(out, res + 1, s, vlc);
    va_end(vlc);
    while (*out) putchar(*out++);
    return res;
}

