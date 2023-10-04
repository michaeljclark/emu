#pragma once

#include "types.h"

static inline uchar x86_inb(short port)
{
    uchar data;
    asm volatile ("inb %1,%0" : "=a" (data) : "d" (port));
    return data;
}

static inline void x86_outb(short port, uchar data)
{
    asm volatile ("outb %1,%0" : : "d" (port), "a" (data));
}

static inline ushort x86_inw(short port)
{
    ushort data;
    asm volatile ("inw %1,%0" : "=a" (data) : "d" (port));
    return data;
}

static inline void x86_outw(short port, ushort data)
{
    asm volatile ("outw %1,%0" : : "d" (port), "a" (data));
}

static inline void x86_cpuid(int *p, int ax, int cx)
{
    asm volatile ("cpuid"
                  : "=a" (p[0]), "=b" (p[3]), "=c" (p[1]), "=d" (p[2])
                  :  "0" (ax), "2" (cx));
}
