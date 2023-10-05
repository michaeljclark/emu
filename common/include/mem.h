#pragma once

#include "types.h"

typedef ullong emu_phys_addr;
typedef ullong emu_virt_addr;

typedef enum {
	emu_mem_type_reserved,
	emu_mem_type_boot_code,
	emu_mem_type_boot_data,
	emu_mem_type_main_memory,
	emu_mem_type_mmio
} emu_mem_type;

typedef enum {
	emu_mem_attr_none = 0x0,
	emu_mem_attr_w    = 0x1,
	emu_mem_attr_r    = 0x2,
	emu_mem_attr_rw   = 0x3,
	emu_mem_attr_x    = 0x4,
	emu_mem_attr_wx   = 0x5,
	emu_mem_attr_rx   = 0x6,
	emu_mem_attr_rwx  = 0x7
} emu_mem_attr;

typedef struct {
	uint mem_type;
	uint mem_attrs;
	emu_phys_addr phys_start;
	emu_virt_addr virt_start;
	ullong length;
} emu_mem_desc;
