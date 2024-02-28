#include <cstdio>
#include <cstddef>
#include <cstdlib>
#include <cstdint>
#include <cassert>

#include "sys.h"

int log_level = emu_log_debug;

int test_mmio_create(emu_system *sys, emu_device_class *devclass, emu_device *dev, const char* devattrs)
{
    emu_io *io;
    emu_map_io(&io, dev, emu_io_type_mmio, 0, 0x40000000, 0x1000);
    return 0;
}

int test_mmio_destroy(emu_system *sys, emu_device_class *devclass, emu_device *dev)
{
    return 0;
}

int test_mmio_io(emu_io *io, emu_io_op op, emu_io_addr addr, ullong size, ullong *val)
{
    emu_debugf("test_mmio_io: op=%d addr=0x%llx\n", op, addr);
    return 0;
}

emu_device_class test_mmio_dev = {
    "test_mmio", "test mmio",
    test_mmio_create, test_mmio_destroy, test_mmio_io
};

int main()
{
    emu_system *sys;
    emu_cpu *cpu;
    emu_device *mmio, *rcr;

    llong mem_size = 1024 * 1024;

    CHECK_ERROR(emu_init());
    CHECK_ERROR(emu_create_sys(&sys, mem_size, 1));
    CHECK_ERROR(emu_create_cpu(&cpu, sys, 0));
    emu_register_device_class(&test_mmio_dev);
    CHECK_ERROR(emu_create_device(&mmio, sys, "test_mmio", nullptr));
    CHECK_ERROR(emu_create_device(&rcr, sys, "rcr", nullptr));
    CHECK_ERROR(emu_load(sys, "build/out/x86/guest/tests/005_mmio/system.elf"));
    while (emu_running(cpu)) emu_launch(cpu);
    CHECK_ERROR(emu_destroy_cpu(cpu));
    CHECK_ERROR(emu_destroy_sys(sys));

error:
    return 0;
}
