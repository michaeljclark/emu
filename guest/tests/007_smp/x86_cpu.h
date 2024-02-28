#pragma once

#define MAXCPUS 64

#define GDT_COUNT 8
#define IDT_COUNT 256

#define SEG_NULL  0  // null segment
#define SEG_BCODE 1  // boot code
#define SEG_KCODE 2  // kern code
#define SEG_KDATA 3  // kern data
#define SEG_UCODE 4  // user code
#define SEG_UDATA 5  // user data
#define SEG_TSS   6  // tss segment

#ifndef __ASSEMBLER__

#include "types.h"
#include "x86_mmu.h"

typedef struct x86_cpu x86_cpu;

struct x86_cpu
{
	int cpu;
	int lapic_id;
	void *kstack;
	void *xsave;
	x86_64_gdt_storage *gdt;
	x86_64_idt_storage *idt;
	x86_64_tss *tss;
	char cpu_name[64];
	ullong init_tsc;
};

extern int ncpus;
extern x86_cpu cpulist[MAXCPUS];
extern uchar* trapvec;
extern uchar* trapentry;

void x86_boot_cpu_init();
void x86_cpu_init(int cpu);

#endif
