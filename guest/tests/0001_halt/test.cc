#include <cstdio>
#include <cstddef>
#include <cstdlib>
#include <cstdint>
#include <cassert>

#include "sys.h"

int log_level = emu_log_debug;

int main()
{
    emu_system *sys;
    emu_cpu *cpu;
    emu_device *rcr;

    llong mem_size = 1024 * 1024;

    CHECK_ERROR(emu_init());
    CHECK_ERROR(emu_create_sys(&sys, mem_size, 1));
    CHECK_ERROR(emu_create_cpu(&cpu, sys, 0));
    CHECK_ERROR(emu_create_device(&rcr, sys, "rcr", nullptr));
    CHECK_ERROR(emu_load(sys, "build/out/x86/guest/tests/0001_halt/system.elf"));
    while (emu_running(cpu)) emu_launch(cpu);
    CHECK_ERROR(emu_destroy_cpu(cpu));
    CHECK_ERROR(emu_destroy_sys(sys));

error:
    return 0;
}
