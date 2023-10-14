#include "stdio.h"
#include "stddef.h"
#include "stdlib.h"
#include "alloca.h"
#include "page.h"
#include "x86_mmu.h"
#include "vmm.h"

static const char *mem_type_str(uint type)
{
    switch (type) {
    case emu_mem_type_reserved: return "reserved";
    case emu_mem_type_boot_code: return "boot_code";
    case emu_mem_type_boot_data: return "boot_data";
    case emu_mem_type_main_memory: return "main_memory";
    case emu_mem_type_mmio: return "mmio";
    default: return "unknown";
    }
}

static const char *mem_attr_str(uint type)
{
    switch (type & 7) {
    case emu_mem_attr_w: return "-w-";
    case emu_mem_attr_r: return "r--";
    case emu_mem_attr_rw: return "rw-";
    case emu_mem_attr_x: return "--x";
    case emu_mem_attr_wx: return "-wx";
    case emu_mem_attr_rx: return "r-x";
    case emu_mem_attr_rwx: return "rwx";
    default: return "---";
    }
}

void mem_setup()
{
    ullong count = 0;
    emu_mem_desc *memdesc;
    ullong memory_start = -1, memory_end = 0;

    vmm_query_mem(&count, NULL);
    memdesc = (emu_mem_desc*)alloca(sizeof(emu_mem_desc) * count);
    vmm_query_mem(&count, memdesc);

    for (ullong i = 0; i < count; i++) {
        ullong start = memdesc[i].phys_start;
        ullong end = memdesc[i].phys_start + memdesc[i].length;
        printf("# memory map  %llx - %llx %s %s\n",
            start, end,
            mem_attr_str(memdesc[i].mem_attrs),
            mem_type_str(memdesc[i].mem_type));
        if (memdesc[i].mem_type != emu_mem_type_main_memory) {
            continue;
        }
        if (memory_start == (ullong)-1LL || start < memory_start) {
            memory_start = start;
        }
        if (end > memory_end) {
            memory_end = end;
        }
    }
    if (memory_start != -1) {
        init_pages(memory_start, memory_end);
        ullong heap_pages = pages_free() >> 3;
        void *heap_start = alloc_pages(heap_pages);
        _malloc_addblock(heap_start, heap_pages << PAGE_SHIFT);
        printf("# kernel heap %llx - %llx [%lld pages]\n",
            (ullong)heap_start,
            (ullong)heap_start + (heap_pages << PAGE_SHIFT),
            heap_pages);
    }
}
