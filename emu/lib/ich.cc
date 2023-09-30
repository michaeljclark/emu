#include <cstdio>
#include <cstddef>
#include <cstdlib>
#include <cstdint>
#include <cassert>
#include <cstdarg>

#include "sys.h"

// Gate A20 emulation (ioport 0x60, 0x64)

int emu_a20_create(emu_system *sys, emu_device_class *devclass,
    emu_device *dev, const char* devattrs)
{
    emu_io *io1, *io2;
    CHECK_ERROR(emu_map_io(&io1, dev, emu_io_type_portio, 0, 0x60, 1));
    CHECK_ERROR(emu_map_io(&io2, dev, emu_io_type_portio, 4, 0x64, 1));
    return 0;

error:
    return -1;
}

int emu_a20_destroy(emu_system *sys, emu_device_class *devclass,
    emu_device *dev)
{
    return 0;
}

int emu_a20_io(emu_io *io, emu_io_op op, emu_io_addr addr, ullong size,
    ullong *val)
{
    static uchar ps2cmd, ps2data;

    switch (addr) {
    case 0x60:
        switch (op) {
        case emu_io_op_read:
            *val = 0;
            break;
        case emu_io_op_write:
            ps2data = (uchar)*val;
            if (ps2cmd == 0xd1 && ps2data == 0xdf) {
                emu_debugf("emu_a20: gate a20 enabled\n");
            }
            else if (ps2cmd == 0xd1 && ps2data == 0xdd) {
                emu_debugf("emu_a20: gate a20 disabled\n");
            }
            break;
        }
        break;
    case 0x64:
        switch (op) {
        case emu_io_op_read:
            *val = 0;
            break;
        case emu_io_op_write:
            ps2cmd = (uchar)*val;
            break;
        }
        break;
    }

    return 0;
}

emu_device_class emu_a20 = {
    "a20", "gate a20 micro controller",
    emu_a20_create, emu_a20_destroy, emu_a20_io
};

// Reset Control Register (ioport 0xcf9)

int emu_rcr_create(emu_system *sys, emu_device_class *devclass,
    emu_device *dev, const char* devattrs)
{
    emu_io *io;
    CHECK_ERROR(emu_map_io(&io, dev, emu_io_type_portio, 0, 0xcf9, 2));
    return 0;

error:
    return -1;
}

int emu_rcr_destroy(emu_system *sys, emu_device_class *devclass,
    emu_device *dev)
{
    return 0;
}

int emu_rcr_io(emu_io *io, emu_io_op op, emu_io_addr addr, ullong size,
    ullong *val)
{
    switch (addr) {
    case 0xcf9:
        switch (op) {
            case emu_io_op_read:
                *val = 0;
                break;
            case emu_io_op_write:
                switch (*val) {
                case 0x800:
                    emu_debugf("emu_rcr: halt\n");
                    emu_halt(io->dev->sys);
                    break;
                }
                break;
        }
        break;
    }

    return 0;
}

emu_device_class emu_rcr = {
    "rcr", "reset control register",
    emu_rcr_create, emu_rcr_destroy, emu_rcr_io
};

// ICH init

int emu_ich_init()
{
    emu_register_device_class(&emu_a20);
    emu_register_device_class(&emu_rcr);
    return 0;
}
