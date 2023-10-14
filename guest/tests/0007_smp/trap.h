#pragma once

#include "types.h"

typedef struct trapframe trapframe;

struct trapframe
{
    /* pushed by trapentry */
    uintp rax;
    uintp rbx;
    uintp rcx;
    uintp rdx;
    uintp rbp;
    uintp rsi;
    uintp rdi;
    uintp r8;
    uintp r9;
    uintp r10;
    uintp r11;
    uintp r12;
    uintp r13;
    uintp r14;
    uintp r15;
    /* pushed by cpu */
    uintp trapno;
    uintp error;
    uintp rip;
    uintp cs;
    uintp rflags;
    uintp rsp;
    uintp ss;
};
