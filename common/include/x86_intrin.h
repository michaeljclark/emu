#pragma once

#include "types.h"
#include "x86_mmu.h"

static inline uchar x86_inb(short port)
{
    uchar data;
    asm volatile ("inb %1,%0" : "=a" (data) : "d" (port));
    return data;
}

static inline ushort x86_inw(short port)
{
    ushort data;
    asm volatile ("inw %1,%0" : "=a" (data) : "d" (port));
    return data;
}

static inline uint x86_inl(short port)
{
    uint data;
    asm volatile ("inl %1,%0" : "=a" (data) : "d" (port));
    return data;
}

static inline void x86_outb(short port, uchar data)
{
    asm volatile ("outb %1,%0" : : "d" (port), "a" (data));
}

static inline void x86_outw(short port, ushort data)
{
    asm volatile ("outw %1,%0" : : "d" (port), "a" (data));
}

static inline void x86_outl(short port, uint data)
{
    asm volatile ("outl %1,%0" : : "d" (port), "a" (data));
}

static inline void x86_cpuid(int *p, int ax, int cx)
{
    asm volatile ("cpuid"
                  : "=a" (p[0]), "=b" (p[3]), "=c" (p[1]), "=d" (p[2])
                  :  "0" (ax), "2" (cx));
}

static inline ullong x86_rdtsc()
{
    uint low, high;
    asm volatile("rdtsc" : "=a" (low), "=d" (high));
    return (ullong)low | ((ullong)high << 32);
}

static inline ullong x86_rdmsr(uint msr)
{
    uint low, high;
    asm volatile ("rdmsr" : "=a" (low), "=d" (high) : "c" (msr));
    return (ullong)low | ((ullong)high << 32);
}

static inline void x86_wrmsr(uint msr, ullong val)
{
    uint low = val, high = val >> 32;
    asm volatile ("wrmsr" : : "a" (low), "d" (high), "c" (msr));
}

static inline void x86_64_lgdt(void *gdt, int size)
{
    volatile x86_64_gdt_reg gdtreg = { size - 1, (ullong)gdt };
    asm volatile ("lgdt (%0)" : : "r" (&gdtreg));
}

static inline void x86_64_lidt(void *idt, int size)
{
    volatile x86_64_idt_reg idtreg = { size - 1, (ullong)idt };
    asm volatile ("lidt (%0)" : : "r" (&idtreg));
}

static inline void x86_ltr(ushort sel)
{
    asm volatile ("ltr %0" : : "r" (sel));
}

static inline uintp x86_read_cr2()
{
    uintp val;
    asm volatile ("mov %%cr2,%0" : "=r" (val));
    return val;
}

static inline uintp x86_read_cr3()
{
    uintp val;
    asm volatile ("mov %%cr3,%0" : "=r" (val));
    return val;
}

static inline void x86_write_cr3(uintp val)
{
    asm volatile ("mov %0,%%cr3" : : "r" (val));
}

static inline void x86_cli()
{
    asm volatile ("cli");
}

static inline void x86_sti()
{
    asm volatile ("sti");
}

static inline void x86_hlt()
{
    asm volatile ("hlt");
}

static inline void x86_wfi()
{
    asm volatile ("0: hlt; jmp 0b");
}
