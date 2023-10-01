#pragma once

#include <windows.h>
#include <WinHvPlatform.h>
#include <WinHvEmulation.h>

#include <vector>

#include "elf.h"
#include "utf8.h"
#include "types.h"
#include "log.h"

#include "x86_msr.h"
#include "x86_mmu.h"
#include "x86_exception.h"

#define array_size(arr) (sizeof(arr) / sizeof(arr[0]))

#define CHECK_HRESULT(expr) {                                     \
    HRESULT hr = expr;                                            \
    if (FAILED(hr)) {                                             \
        printf("%s failed hr=%08lx\n", #expr, hr); goto error;    \
    }                                                             \
}
#define CHECK_ERROR(expr) {                                       \
    int ret = expr;                                               \
    if (ret<0) {                                                  \
        printf("%s failed ret=%d\n", #expr, ret); goto error;     \
    }                                                             \
}
#define CHECK_BOOL(expr) {                                        \
    bool br = expr;                                               \
    if (!br) {                                                    \
        printf("%s failed\n", #expr); goto error;                 \
    }                                                             \
}

struct emu_io;
struct emu_device;
struct emu_device_class;
struct emu_system;
struct emu_cpu;

typedef ullong emu_io_addr;

typedef enum {
    emu_io_type_map,
    emu_io_type_mmio,
    emu_io_type_portio,
} emu_io_type;

typedef enum {
    emu_io_op_read,
    emu_io_op_write,
} emu_io_op;

struct emu_io
{
    emu_device *dev;
    emu_io_type iotype;
    ullong ioid;
    ullong base;
    ullong len;

    emu_io(emu_device *dev, emu_io_type iotype, ullong ioid,
        ullong base, ullong len) : dev(dev), iotype(iotype),
        ioid(ioid), base(base), len(len) {}
};

struct emu_device
{
    emu_system *sys;
    emu_device_class *devclass;
    std::vector<emu_io*> iolist;
    void *state;

    emu_device(emu_system *sys, emu_device_class *devclass) :
        sys(sys), devclass(devclass), iolist(), state() {}
};

struct emu_device_class
{
    const char *devclassname;
    const char *devclassdescription;
    int (*create_fn)(emu_system *sys, emu_device_class *devclass,
        emu_device *dev, const char* devattrs);
    int (*destroy_fn)(emu_system *sys, emu_device_class *devclass,
        emu_device *dev);
    int (*io_fn)(emu_io *io, emu_io_op op, emu_io_addr addr, ullong size,
        ullong *val);
};

struct emu_system
{
    ullong mem_size;
    void* mem;
    std::vector<emu_cpu*> cpulist;
    std::vector<emu_device*> devlist;
    WHV_PARTITION_HANDLE part;

    emu_system(ullong mem_size) :
        mem_size(mem_size), mem(nullptr), cpulist(), devlist(), part() {}
};

typedef int (*emu_vmcall_fn)(emu_cpu *cpu, void *ctx);

struct emu_cpu
{
    emu_system *sys;
    int vpi;
    bool running;
    WHV_EMULATOR_HANDLE emu;
    WHV_EMULATOR_STATUS status;
    WHV_EMULATOR_CALLBACKS eemu_cb;
    WHV_RUN_VP_EXIT_CONTEXT exit;
    emu_vmcall_fn vmcall_fn;
    void *vmcall_ctx;

    emu_cpu(emu_system *sys, int vpi, bool running) :
        sys(sys), vpi(vpi), running(running), emu(), eemu_cb(), exit(),
        vmcall_fn(), vmcall_ctx() {}
};

extern emu_device_class emu_uart;

int emu_ich_init();
int emu_uart_init();

int emu_console_has_char();
int emu_console_read_char();
void emu_console_write_char(int ch);

int emu_register_device_class(emu_device_class *devclass);
int emu_lookup_device_class(const char *devclassname,
    emu_device_class **devclass);
int emu_create_device(emu_device **devp, emu_system *sys,
    const char *devclassname, const char *devattrs);
int emu_destroy_device(emu_device *dev);
int emu_map_io(emu_io **iop, emu_device *dev, emu_io_type iotype,
    ullong ioid, ullong base, ullong len);
int emu_find_io(emu_io **iop, emu_system *sys, emu_io_type iotype,
    ullong addr, ullong size);
int emu_unmap_io(emu_io *io);

int emu_init();
int emu_load(emu_system *sys, const char *filename);
int emu_dump_mem(emu_system *sys, ullong offset, ullong len);
int emu_dump_regs(emu_cpu *cpu);
int emu_set_vmcall(emu_cpu *cpu, emu_vmcall_fn fn, void *ctx);
int emu_get_regs(emu_cpu *cpu, uint *regs, uint count, ullong *values);
int emu_set_regs(emu_cpu *cpu, uint *regs, uint count, ullong *values);
int emu_create_sys(emu_system **sys, ullong meemu_size);
int emu_create_cpu(emu_cpu **cpu, emu_system *sys, int vpi);
int emu_halt(emu_system *sys);
int emu_running(emu_cpu *cpu);
int emu_launch(emu_cpu *cpu);
int emu_destroy_cpu(emu_cpu *cpu);
int emu_destroy_sys(emu_system *sys);
