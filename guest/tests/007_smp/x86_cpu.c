#include "stdio.h"
#include "stdlib.h"
#include "string.h"
#include "page.h"
#include "stack.h"
#include "x86_intrin.h"
#include "x86_mmu.h"
#include "x86_msr.h"
#include "x86_cpu.h"

int ncpus;
x86_cpu cpulist[MAXCPUS];
uchar* trapvec;

#define gdt_desc(addr,limit,type,kind,dpl,present,avail_sys,long_mode,big_dfl,\
                 granularity)                                                 \
{ (limit & 0xffff), (addr & 0xffff), ((addr >> 16) & 0xff), type, kind, dpl,  \
  present, ((limit >> 16) & 0xf), avail_sys, long_mode, big_dfl, granularity, \
  ((addr >> 24) & 0xff) }

#define tss_desc(addr,limit,type,kind,dpl,present,avail_sys,long_mode,big_dfl,\
                 granularity)                                                 \
{ (limit & 0xffff), (addr & 0xffff), ((addr >> 16) & 0xff), type, kind, dpl,  \
  present, ((limit >> 16) & 0xf), avail_sys, long_mode, big_dfl, granularity, \
  ((addr >> 24) & 0xff), ((addr >> 32) & 0xffffffff), 0 }

#define idt_desc(addr,seg_sel,ist,type,dpl,present)                           \
{ (addr & 0xffff), seg_sel, ist, 0, type, 0, dpl, present,                    \
  ((addr >> 16) & 0xffff), ((addr >> 32) & 0xffffffff), 0 }

#define ADDR_HI(addr) (uint)((ullong)addr >> 32)
#define ADDR_LO(addr) (uint)((ullong)addr & 0xffffffff);

void x86_boot_cpu_init()
{
    /* create trap vector trampoline to add error code and save irq number */
    trapvec = alloc_pages(1);
    for (int irq = 0; irq < 256; irq++) {
        uint has_errcode = (irq == 8 || (irq >= 10 && irq <= 14) || irq == 17);
        uchar code[8] = {
            /* if this exception doesn't have an error code we push a
             * copy of rflags so that the stack frame is consistent */
            has_errcode ? 0x90 /* nop */ : 0x9c /* pushfq */,
            0x6a, irq,                          /* push $0xNN */
            0xe9, 0x00, 0x00, 0x00, 0x00        /* jmp REL32 */
        };
        int trapdisp = (int)((intp)&trapentry - (intp)(trapvec + irq * 8 + 8));
        memcpy(code + 4, &trapdisp, sizeof(trapdisp));
        memcpy(trapvec + irq * 8, code, sizeof(code));
    }
    /* initialize x86 per cpu GDT, IDT, TSS and kernel stack */
    for (int cpu = 0; cpu < ncpus; cpu++) {
        cpulist[cpu].gdt = malloc(GDT_COUNT * sizeof(x86_64_gdt_storage));
        cpulist[cpu].idt = malloc(IDT_COUNT * sizeof(x86_64_idt_storage));
        cpulist[cpu].tss = malloc(sizeof(x86_64_tss));
        cpulist[cpu].kstack = alloc_pages(STACK_PAGES) + STACK_SIZE;
        x86_64_gdt_desc nulld = gdt_desc(0, 0, 0x0/*   */, 0, 0, 0, 0, 0, 0, 0);
        x86_64_gdt_desc kcode = gdt_desc(0, 0, 0xe/*rxc*/, 1, 0, 1, 0, 1, 0, 1);
        x86_64_gdt_desc kdata = gdt_desc(0, 0, 0x2/* rw*/, 1, 0, 1, 0, 0, 0, 1);
        x86_64_gdt_desc ucode = gdt_desc(0, 0, 0xe/*rxc*/, 1, 3, 1, 0, 1, 0, 1);
        x86_64_gdt_desc udata = gdt_desc(0, 0, 0x2/* rw*/, 1, 3, 1, 0, 0, 0, 1);
        x86_64_tss_desc tss = tss_desc((ullong)cpulist[cpu].tss,
            sizeof(x86_64_tss) - 1, 9, 0, 0, 1, 0, 0, 0, 0);
        memcpy(&cpulist[cpu].gdt[SEG_NULL], &nulld, sizeof(x86_64_gdt_desc));
        memcpy(&cpulist[cpu].gdt[SEG_BCODE], &nulld, sizeof(x86_64_gdt_desc));
        memcpy(&cpulist[cpu].gdt[SEG_KCODE], &kcode, sizeof(x86_64_gdt_desc));
        memcpy(&cpulist[cpu].gdt[SEG_KDATA], &kdata, sizeof(x86_64_gdt_desc));
        memcpy(&cpulist[cpu].gdt[SEG_UCODE], &ucode, sizeof(x86_64_gdt_desc));
        memcpy(&cpulist[cpu].gdt[SEG_UDATA], &udata, sizeof(x86_64_gdt_desc));
        memcpy(&cpulist[cpu].gdt[SEG_TSS], &tss, sizeof(x86_64_tss_desc));
        memset(cpulist[cpu].idt, 0, IDT_COUNT * sizeof(x86_64_idt_storage));
        cpulist[cpu].tss->rsp0[0] = ADDR_LO(cpulist[cpu].kstack);
        cpulist[cpu].tss->rsp0[1] = ADDR_HI(cpulist[cpu].kstack);
        cpulist[cpu].tss->iomapbase = sizeof(x86_64_tss);
        for (int irq = 0; irq < 256; irq++) {
            uintp trapaddr = (uintp)trapvec + irq * 8;
            x86_64_idt_desc idesc = idt_desc(trapaddr,SEG_KCODE<<3,0,0xe,0,1);
            memcpy(&cpulist[cpu].idt[irq], &idesc, sizeof(x86_64_idt_storage));
        }
    }
}

static void x86_cpu_info(x86_cpu *cpu)
{
    int leaf_0[4], leaf_1[4], leaf_2[4], leaf_3[4], leaf_4[4];

    memset(cpu->cpu_name, 0, sizeof(cpu->cpu_name));
    x86_cpuid(leaf_0, 0x80000000, 0);
    if (leaf_0[0] >= 0x80000004)
    {
        x86_cpuid(leaf_2, 0x80000002, 0);
        x86_cpuid(leaf_3, 0x80000003, 0);
        x86_cpuid(leaf_4, 0x80000004, 0);
        memcpy(cpu->cpu_name + 0x00, leaf_2, 0x10);
        memcpy(cpu->cpu_name + 0x10, leaf_3, 0x10);
        memcpy(cpu->cpu_name + 0x20, leaf_4, 0x10);
    }
}

void x86_alloc_xsave(int cpu)
{
    /* Get XSAVE area size */
    int leaf[4];
    x86_cpuid(leaf, 0xd, 0);
    uint xsave_size = leaf[1] /* ebx */;

    /* Allocate XSAVE area */
    cpulist[cpu].xsave = aligned_alloc(64, xsave_size);
}

void x86_cpu_init(int cpu)
{
    cpulist[cpu].init_tsc = x86_rdtsc();
    x86_64_lgdt(cpulist[cpu].gdt, sizeof(x86_64_gdt_storage) * GDT_COUNT);
    x86_64_lidt(cpulist[cpu].idt, sizeof(x86_64_idt_storage) * IDT_COUNT);
    x86_ltr(SEG_TSS << 3);
    x86_wrmsr(MSR_IA32_GS_BASE, (ullong)(cpulist + cpu));
    x86_alloc_xsave(cpu);
    x86_cpu_info(cpulist + cpu);
}
