#include "vmm.h"
#include "vmcall_x86.h"
#include "vmcall_disp.h"
#include "vmcall.h"
#include "poweroff.h"

void vmm_poweroff(llong code)
{
	__vmcall(vmcall_poweroff, code);
	__builtin_unreachable();
}

int vmm_query_mem(ullong *count, emu_mem_desc *memdesc)
{
	return __vmcall(vmcall_query_mem, count, memdesc);
}

poweroff_device_t poweroff_vmm = {
    0,
    vmm_poweroff,
};
