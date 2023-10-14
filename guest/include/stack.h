#pragma once

#include "x86_mmu.h"

#define STACK_PAGES 2
#define STACK_SIZE (PAGE_SIZE * STACK_PAGES)

#define ARCH_REG_SP "%rsp"

static inline void* get_sp()
{
    register void* sp asm(ARCH_REG_SP);
    asm volatile ("" : "=r" (sp) : : "memory");
    return sp;
}

static inline int pivot_stack(int argc, char **argv, void *ns,
    int (*main)(int argc, char **argv))
{
    register int r;
    register void* sp asm(ARCH_REG_SP);
    asm volatile ("" : "=r" (sp) : : "memory");
    void *o = sp;
    sp = ns;
    asm volatile ("" : : "r" (sp) : "memory");
    r = main(argc, argv);
    sp = o;
    asm volatile ("" : : "r" (sp) : "memory");
    return r;
}
