#include <cstdio>
#include <cstddef>
#include <cstdlib>
#include <cstdint>
#include <cassert>

#include "sys.h"

int log_level = emu_log_debug;

int test_vmcall_fn(emu_cpu *cpu, void *ctx)
{
    uint regs[1] = { 7 /* rdi */ };
    ullong values[1] = { 0 };

    emu_get_regs(cpu, regs, 1, values);
    emu_debugf("test_vmcall_fn: rdi=0x%llx\n", values[0]);
    values[0]++;
    emu_set_regs(cpu, regs, 1, values);

    return 0;
}

int main()
{
    emu_system *sys;
    emu_cpu *cpu;
    emu_device *rcr;

    llong mem_size = 1024 * 1024;

    CHECK_ERROR(emu_init());
    CHECK_ERROR(emu_create_sys(&sys, mem_size));
    CHECK_ERROR(emu_create_cpu(&cpu, sys, 0));
    CHECK_ERROR(emu_set_vmcall(cpu, test_vmcall_fn, nullptr));
    CHECK_ERROR(emu_create_device(&rcr, sys, "rcr", nullptr));
    CHECK_ERROR(emu_load(sys, "build/out/x86/tests/0006_vmcall/system.elf"));
    while (emu_running(cpu)) emu_launch(cpu);
    CHECK_ERROR(emu_destroy_cpu(cpu));
    CHECK_ERROR(emu_destroy_sys(sys));

error:
    return 0;
}
