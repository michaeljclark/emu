#pragma once

#include "mem.h"

void vmm_poweroff(llong code);
int vmm_query_mem(ullong *count, emu_mem_desc *memdesc);
