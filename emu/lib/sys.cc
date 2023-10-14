#pragma comment (lib, "WinHvPlatform.lib")
#pragma comment (lib, "WinHvEmulation.lib")

#include <cstdio>
#include <cstddef>
#include <cstdlib>
#include <cstdint>
#include <cassert>
#include <cstdarg>

#include "sys.h"
#include "uart.h"
#include "vmcall.h"

#include <map>
#include <vector>
#include <string>
#include <utility>

#include <fcntl.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <io.h>

#ifdef WIN32
#define open _open
#define read _read
#define write _write
#define close _close
#endif

static const char* all_reg_mnemonics[] =
{
    // X64 General purpose registers (Rip & Rflags are in exit context)
    "Rax", "Rcx", "Rdx", "Rbx", "Rsp", "Rbp", "Rsi", "Rdi",
    "R8",  "R9",  "R10", "R11", "R12", "R13", "R14", "R15",
    // X64 Segment registers
    "Es", "Cs", "Ss", "Ds", "Fs", "Gs", "Ldtr", "Tr",
    // X64 Table registers
    "Idtr", "Gdtr",
    // X64 Control Registers
    "Cr0", "Cr2", "Cr3", "Cr4", "Cr8",
    // X64 Debug Registers
    "Dr0", "Dr1", "Dr2", "Dr3", "Dr6", "Dr7",
    // X64 Extended Control Registers
    "XCr0",
    // X64 Floating Point and Vector Registers
    "Xmm0", "Xmm1", "Xmm2", "Xmm3", "Xmm4", "Xmm5", "Xmm6", "Xmm7",
    "Xmm8", "Xmm9", "Xmm10", "Xmm11", "Xmm12", "Xmm13", "Xmm14", "Xmm15",
    "FpMmx0", "FpMmx1", "FpMmx2", "FpMmx3",
    "FpMmx4", "FpMmx5", "FpMmx6", "FpMmx7",
    // X64 MSRs
    "Tsc", "Efer", "KernelGsBase", "ApicBase",
    "Pat", "SysenterCs", "SysenterEip", "SysenterEsp",
    "Star", "Lstar", "Cstar", "Sfmask",
    "TscAux", "TscOffset",
    // APIC state
    "ApicId", "ApicVersion",
    // FP CSRs
    "FpCSR", "MxCSR",
};

static WHV_REGISTER_NAME all_reg_names[] =
{
    // X64 General purpose registers (Rip & Rflags are in exit context)
    WHvX64RegisterRax, WHvX64RegisterRcx,
    WHvX64RegisterRdx, WHvX64RegisterRbx,
    WHvX64RegisterRsp, WHvX64RegisterRbp,
    WHvX64RegisterRsi, WHvX64RegisterRdi,
    WHvX64RegisterR8, WHvX64RegisterR9,
    WHvX64RegisterR10, WHvX64RegisterR11,
    WHvX64RegisterR12, WHvX64RegisterR13,
    WHvX64RegisterR14, WHvX64RegisterR15,
    // X64 Segment registers
    WHvX64RegisterEs, WHvX64RegisterCs,
    WHvX64RegisterSs, WHvX64RegisterDs,
    WHvX64RegisterFs, WHvX64RegisterGs,
    WHvX64RegisterLdtr, WHvX64RegisterTr,
    // X64 Table registers
    WHvX64RegisterIdtr, WHvX64RegisterGdtr,
    // X64 Control Registers
    WHvX64RegisterCr0, WHvX64RegisterCr2,
    WHvX64RegisterCr3, WHvX64RegisterCr4,
    WHvX64RegisterCr8,
    // X64 Debug Registers
    WHvX64RegisterDr0, WHvX64RegisterDr1,
    WHvX64RegisterDr2, WHvX64RegisterDr3,
    WHvX64RegisterDr6, WHvX64RegisterDr7,
    // X64 Extended Control Registers
    WHvX64RegisterXCr0,
    // X64 Floating Point and Vector Registers
    WHvX64RegisterXmm0, WHvX64RegisterXmm1,
    WHvX64RegisterXmm2, WHvX64RegisterXmm3,
    WHvX64RegisterXmm4, WHvX64RegisterXmm5,
    WHvX64RegisterXmm6, WHvX64RegisterXmm7,
    WHvX64RegisterXmm8, WHvX64RegisterXmm9,
    WHvX64RegisterXmm10, WHvX64RegisterXmm11,
    WHvX64RegisterXmm12, WHvX64RegisterXmm13,
    WHvX64RegisterXmm14, WHvX64RegisterXmm15,
    WHvX64RegisterFpMmx0, WHvX64RegisterFpMmx1,
    WHvX64RegisterFpMmx2, WHvX64RegisterFpMmx3,
    WHvX64RegisterFpMmx4, WHvX64RegisterFpMmx5,
    WHvX64RegisterFpMmx6, WHvX64RegisterFpMmx7,
    // X64 MSRs
    WHvX64RegisterTsc, WHvX64RegisterEfer,
    WHvX64RegisterKernelGsBase, WHvX64RegisterApicBase,
    WHvX64RegisterPat, WHvX64RegisterSysenterCs,
    WHvX64RegisterSysenterEip, WHvX64RegisterSysenterEsp,
    WHvX64RegisterStar, WHvX64RegisterLstar,
    WHvX64RegisterCstar, WHvX64RegisterSfmask,
    WHvX64RegisterTscAux, WHvX64RegisterTscVirtualOffset,
    // APIC state
    WHvX64RegisterApicId, WHvX64RegisterApicVersion,
    // X64 CSRs
    WHvX64RegisterFpControlStatus, WHvX64RegisterXmmControlStatus,
};

typedef enum { GPR, SEG, TAB, CR, DBG, XCR, AVX, FPU, CSR, MSR } WVV_REG_TYPE;

static WVV_REG_TYPE all_reg_types[] =
{
    // X64 General purpose registers
    GPR, GPR, GPR, GPR,
    GPR, GPR, GPR, GPR,
    GPR, GPR, GPR, GPR,
    GPR, GPR, GPR, GPR,
    // X64 Segment registers
    SEG, SEG, SEG, SEG,
    SEG, SEG, SEG, SEG,
    // X64 Table registers
    TAB, TAB,
    // X64 Control Registers
    CR, CR, CR, CR, CR,
    // X64 Debug Registers
    DBG, DBG, DBG, DBG, DBG, DBG,
    // X64 Extended Control Registers
    XCR,
    // X64 Floating Point and Vector Registers
    AVX, AVX, AVX, AVX, AVX, AVX, AVX, AVX,
    AVX, AVX, AVX, AVX, AVX, AVX, AVX, AVX,
    FPU, FPU, FPU, FPU, FPU, FPU, FPU, FPU,
    // X64 MSRs
    MSR, MSR, MSR, MSR, MSR, MSR, MSR, MSR,
    MSR, MSR, MSR, MSR, MSR, MSR, MSR, MSR,
    // X64 CSRs
    CSR, CSR,
};

static WHV_REGISTER_NAME init_reg_names[] =
{
    WHvX64RegisterRip,
    WHvX64RegisterRax, WHvX64RegisterRcx,
    WHvX64RegisterRdx, WHvX64RegisterRbx,
    WHvX64RegisterRsp, WHvX64RegisterRbp,
    WHvX64RegisterRsi, WHvX64RegisterRdi,
    WHvX64RegisterR8,  WHvX64RegisterR9,
    WHvX64RegisterR10, WHvX64RegisterR11,
    WHvX64RegisterR12, WHvX64RegisterR13,
    WHvX64RegisterR14, WHvX64RegisterR15,
};

int emu_console_has_char()
{
    // todo
    return 0;
}

int emu_console_read_char()
{
    // todo
    return 0;
}

void emu_console_write_char(int ch)
{
    char buf[5];
    utf32_to_utf8(buf, sizeof(buf), ch);
    fprintf(stdout, "%s", buf);
    fflush(stdout);
}

static std::map<std::string,emu_device_class*> devclasses;

int emu_register_device_class(emu_device_class *devclass)
{
    devclasses.insert(devclasses.end(),
        std::make_pair(std::string(devclass->devclassname), devclass));
    return 0;
}

int emu_lookup_device_class(const char *devclassname,
    emu_device_class **devclass)
{
    if (devclass) {
        auto i = devclasses.find(std::string(devclassname));
        *devclass = i != devclasses.end() ? i->second : nullptr;
    }
    return 0;
}

int emu_create_device(emu_device **devp, emu_system *sys,
    const char *devclassname, const char *devattrs)
{
    emu_device_class *devclass;
    CHECK_ERROR(emu_lookup_device_class(devclassname, &devclass));
    emu_device *dev = *devp = new emu_device(sys, devclass);
    CHECK_ERROR(devclass->create_fn(sys, devclass, dev, devattrs));
    sys->devlist.push_back(dev);
    return 0;

error:
    return -1;
}

int emu_destroy_device(emu_device *dev)
{
    emu_system *sys = dev->sys;
    emu_device_class *devclass = dev->devclass;
    std::vector<emu_io*> iolistcopy = dev->iolist;
    for (emu_io *io : iolistcopy) {
        emu_unmap_io(io);
    }
    auto i = std::find(sys->devlist.begin(), sys->devlist.end(), dev);
    if (i != sys->devlist.end()) {
        dev->devclass->destroy_fn(sys, devclass, dev);
        sys->devlist.erase(i);
        delete dev;
    }
    return 0;
}

int emu_map_io(emu_io **iop, emu_device *dev, emu_io_type iotype,
    ullong ioid, ullong base, ullong len)
{
    emu_io *io = *iop = new emu_io(dev, iotype, ioid, base, len);
    dev->iolist.push_back(io);
    return 0;
}

int emu_find_io(emu_io **iop, emu_system *sys, emu_io_type iotype,
    ullong addr, ullong size)
{
    for (emu_device *dev : sys->devlist) {
        for (emu_io *io : dev->iolist) {
            if ((iotype == io->iotype) &&
                (addr >= io->base) &&
                (addr + size) <= (io->base + io->len))
            {
                *iop = io;
                return 0;
            }
        }
    }
    *iop = nullptr;
    return -1;
}

int emu_unmap_io(emu_io *io)
{
    emu_device *dev = io->dev;
    auto i = std::find(dev->iolist.begin(), dev->iolist.end(), io);
    if (i != dev->iolist.end()) {
        dev->iolist.erase(i);
    }
    return 0;
}

void *emu_virtaddr(emu_system *sys, emu_phys_addr pa)
{
    return (uchar*)sys->mem + pa;
}

void emu_copy_in(emu_system *sys, emu_phys_addr dst_pa, void *src, ullong n)
{
    uchar *dst = (uchar*)emu_virtaddr(sys, dst_pa);
    if (dst >= (uchar*)sys->mem && dst < ((uchar*)sys->mem + sys->mem_size)) {
        memcpy(dst, src, n);
    }
}

void emu_copy_out(emu_system *sys, void *dst, emu_phys_addr src_pa, ullong n)
{
    uchar *src = (uchar*)emu_virtaddr(sys, src_pa);
    if (src >= (uchar*)sys->mem && src < ((uchar*)sys->mem + sys->mem_size)) {
        memcpy(dst, src, n);
    }
}

int emu_query_ncpus(emu_system *sys)
{
    return sys->ncpus;
}

int emu_query_mem(emu_system *sys, ullong *count, emu_mem_desc *memdesc)
{
    ullong incount = 0, outcount = 0;
    if (count) {
        incount = *count;
        *count =sys->memlist.size();
    }
    if (memdesc) {
        ullong i = 0;
        for (auto desc : sys->memlist) {
            if (i >= incount) break;
            else memdesc[i++] = desc;
        }
    }
    return 0;
}

int emu_map_mem(emu_system *sys, emu_mem_desc memdesc)
{
    // trim intersection on first pass
    ullong a1 = memdesc.phys_start;
    ullong a2 = memdesc.phys_start + memdesc.length;
    for (auto i = sys->memlist.begin(); i != sys->memlist.end(); ) {
        ullong b1 = i->phys_start;
        ullong b2 = i->phys_start + i->length;
        if (a1 <= b2 && a2 >= b1) {
            if (a1 <= b1 && a2 >= b2) {
                i = sys->memlist.erase(i);
            } else if (a1 < b2) {
                i->phys_start = a2;
                i->length = b2 - a2;
                i++;
            } else if (a2 > b1) {
                i->length = a1 - b1;
                i++;
            }
        } else {
            i++;
        }
    }

    // insert the record
    auto i = std::find_if(sys->memlist.begin(), sys->memlist.end(),
        [&] (const emu_mem_desc &desc) {
            return memdesc.phys_start + memdesc.length <= desc.phys_start; });
    sys->memlist.insert(i, memdesc);

    return 0;
}

int emu_vmcall(emu_cpu *cpu, void *ctx)
{
    uint regs[7] = {
        0 /* RAX */,
        7 /* RDI */, 6 /* RSI */, 2 /* RDX */,
        1 /* RCX */, 8 /* R8  */, 9 /* R9  */
    };
    ullong values[7] = { 0 };
    llong result = -1;

    emu_get_regs(cpu, regs, 7, values);
    ullong vmcall_nr = values[0];
    switch (vmcall_nr) {
    case vmcall_poweroff:
        emu_debugf("emu_vmcall: poweroff code=%lld\n", values[1]);
        emu_halt(cpu->sys);
        /* non zero code clears error so that emu_lerror returns 0 */
        if (values[1] == 0) {
            cpu->exit.ExitReason = WHvRunVpExitReasonNone;
        }
        break;
    case vmcall_query_ncpus:
        emu_debugf("emu_vmcall: query_ncpus\n");
        result = emu_query_ncpus(cpu->sys);
        break;
    case vmcall_query_mem:
        emu_debugf("emu_vmcall: query_mem count=0x%llx memdesc=0x%llx\n",
            values[1], values[2]);
        result = emu_query_mem(cpu->sys,
            (ullong*)emu_virtaddr(cpu->sys, values[1]),
            (emu_mem_desc*)emu_virtaddr(cpu->sys, values[2]));
        break;
    default:
        emu_debugf("emu_vmcall: unknown\n");
        break;
    }
    values[0] = result;
    emu_set_regs(cpu, regs, 1, values);

    return 0;
}

static HRESULT CALLBACK emu_ioport_emu(_In_ VOID * Context,
    _Inout_ WHV_EMULATOR_IO_ACCESS_INFO *IoAccess)
{
    static int ps2cmd;
    static int ps2data;

    emu_cpu *cpu = (emu_cpu*)Context;

    emu_tracef("emu_ioport_emu: %s port:0x%03x size:%d data:0x%x\n",
       IoAccess->Direction ? "out" : "in",
       IoAccess->Port, IoAccess->AccessSize, IoAccess->Data);

    emu_io *io;
    if (emu_find_io(&io, cpu->sys, emu_io_type_portio,
        IoAccess->Port, IoAccess->AccessSize) == 0)
    {
        ullong val = 0;
        switch (IoAccess->Direction) {
        case emu_io_op_read: /* in */
            io->dev->devclass->io_fn(io, emu_io_op_read,
                IoAccess->Port, IoAccess->AccessSize, &val);
            IoAccess->Data = (uint)val;
            break;
        case emu_io_op_write: /* out */
            val = (ullong)IoAccess->Data;
            io->dev->devclass->io_fn(io, emu_io_op_write,
                IoAccess->Port, IoAccess->AccessSize, &val);
            break;
        }
    }

    return S_OK;
}

static HRESULT CALLBACK emu_mmio_emu(_In_ VOID* Context,
    _Inout_ WHV_EMULATOR_MEMORY_ACCESS_INFO* MemoryAccess)
{
    emu_cpu *cpu = (emu_cpu*)Context;

    emu_tracef("emu_mmio_emu: %s addr:0x%llx size:%d\n",
           MemoryAccess->Direction ? "write" : "read",
           MemoryAccess->GpaAddress, MemoryAccess->AccessSize);

    emu_io *io;
    if (emu_find_io(&io, cpu->sys, emu_io_type_mmio,
        MemoryAccess->GpaAddress, MemoryAccess->AccessSize) == 0)
    {
        ullong val = 0;
        switch (MemoryAccess->Direction) {
        case 0: /* read */
            io->dev->devclass->io_fn(io, emu_io_op_read,
                MemoryAccess->GpaAddress, MemoryAccess->AccessSize, &val);
            memcpy(MemoryAccess->Data, &val, sizeof(val));
            break;
        case 1: /* write */
            memcpy(&val, MemoryAccess->Data, sizeof(val));
            io->dev->devclass->io_fn(io, emu_io_op_write,
                MemoryAccess->GpaAddress, MemoryAccess->AccessSize, &val);
            break;
        }
    }

    return S_OK;
}

static HRESULT CALLBACK emu_getreg_emu(_In_ VOID* Context,
    _In_reads_(RegisterCount) const WHV_REGISTER_NAME* RegisterNames,
    _In_ UINT32 RegisterCount,
    _Out_writes_(RegisterCount) WHV_REGISTER_VALUE* RegisterValues)
{
    emu_cpu *cpu = (emu_cpu*)Context;

    HRESULT err = WHvGetVirtualProcessorRegisters(cpu->sys->part, cpu->vpi,
        RegisterNames, RegisterCount, RegisterValues);

    return err;
}

static HRESULT CALLBACK emu_setreg_emu(_In_ VOID* Context,
    _In_reads_(RegisterCount) const WHV_REGISTER_NAME* RegisterNames,
    _In_ UINT32 RegisterCount,
    _In_reads_(RegisterCount) const WHV_REGISTER_VALUE* RegisterValues)
{
    emu_cpu *cpu = (emu_cpu*)Context;

    HRESULT err = WHvSetVirtualProcessorRegisters(cpu->sys->part, cpu->vpi,
        RegisterNames, RegisterCount, RegisterValues);

    return err;
}

static HRESULT CALLBACK  emu_translate_emu(void* Context,
    WHV_GUEST_VIRTUAL_ADDRESS Gva,
    WHV_TRANSLATE_GVA_FLAGS TranslateFlags,
    WHV_TRANSLATE_GVA_RESULT_CODE* TranslationResult,
    WHV_GUEST_PHYSICAL_ADDRESS* Gpa)
{
    emu_cpu *cpu = (emu_cpu*)Context;
    WHV_TRANSLATE_GVA_RESULT Result;

    HRESULT err = WHvTranslateGva(cpu->sys->part, cpu->vpi, Gva,
        TranslateFlags, &Result, Gpa);
    *TranslationResult = Result.ResultCode;

    return err;
}

static const char* flags_str(int flags)
{
    switch (flags & 7) {
    case 1: return "x";
    case 2: return "w";
    case 3: return "wx";
    case 4: return "r";
    case 5: return "rx";
    case 6: return "rw";
    case 7: return "rwx";
    default: break;
    }
    return "";
}

static const uint flags_attr(int flags)
{
    switch (flags & 7) {
    case 1: return emu_mem_attr_x;
    case 2: return emu_mem_attr_w;
    case 3: return emu_mem_attr_wx;
    case 4: return emu_mem_attr_r;
    case 5: return emu_mem_attr_rx;
    case 6: return emu_mem_attr_rw;
    case 7: return emu_mem_attr_rwx;
    default: break;
    }
    return emu_mem_attr_none;
}

int emu_init()
{
    emu_debugf("emu_init: emu: version %d.%d\n", 0, 1);

    emu_ich_init();
    emu_uart_init();
    
    return 0;  
}

int emu_load(emu_system *sys, const char *filename)
{
    HANDLE fh;
    HANDLE fmaph;
    SSIZE_T length;
    void *addr;
    struct _FILE_STANDARD_INFO finfo;

    if ((fh = CreateFileA(filename, GENERIC_READ, FILE_SHARE_READ, NULL,
        OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL)) == INVALID_HANDLE_VALUE) {
        emu_errorf("emu_load: error: CreateFile: file=%s err=%d\n",
            filename, GetLastError());
        return -1;
    }
    if (!GetFileInformationByHandleEx(fh, FileStandardInfo, &finfo,
            sizeof(finfo))) {
        emu_errorf("emu_load: error: GetFileInformationByHandleEx: "
            "file=%s err=%d\n", filename, GetLastError());
        return -1;
    }
    length = (SSIZE_T)finfo.EndOfFile.LowPart |
             (SSIZE_T)finfo.EndOfFile.HighPart << 32;
    if ((fmaph = CreateFileMapping(fh, NULL, PAGE_READONLY, 0, 0, NULL)) ==
        INVALID_HANDLE_VALUE) {
        CloseHandle(fh);
        emu_errorf("emu_load: error: CreateFileMapping: file=%s err=%d\n",
            filename, GetLastError());
        return -1;
    }
    if ((addr = MapViewOfFile(fmaph, FILE_MAP_READ, 0, 0, 0)) == NULL) {
        CloseHandle(fmaph);
        CloseHandle(fh);
        emu_errorf("emu_load: error: MapViewOfFile: file=%s err=%d\n",
            filename, GetLastError());
        return -1;
    } else {
        emu_debugf("emu_load: loading %s\n", filename);
    }

    // read ELF header
    Elf64_Ehdr ehdr;
    char eident[EI_NIDENT + 1] = {};
    memcpy(&ehdr, addr, sizeof(ehdr));
    memcpy(eident, ehdr.e_ident, EI_NIDENT);

    CHECK_BOOL(ehdr.e_ident[EI_MAG0] == ELFMAG0);
    CHECK_BOOL(ehdr.e_ident[EI_MAG1] == ELFMAG1);
    CHECK_BOOL(ehdr.e_ident[EI_MAG2] == ELFMAG2);
    CHECK_BOOL(ehdr.e_ident[EI_MAG3] == ELFMAG3);
    CHECK_BOOL(ehdr.e_ident[EI_CLASS] == ELFCLASS64);
    CHECK_BOOL(ehdr.e_ident[EI_DATA] == ELFDATA2LSB);
    CHECK_BOOL(ehdr.e_ident[EI_OSABI] == ELFOSABI_SYSV);
    CHECK_BOOL(ehdr.e_ident[EI_ABIVERSION] == ELFABIVERSION_NONE);
    CHECK_BOOL(ehdr.e_type = ET_EXEC);
    CHECK_BOOL(ehdr.e_machine == EM_X86_64);

    // print executable header
    emu_tracef("emu_load:      "
        "%6s %6s %6s %16s %16s %16s %6s %6s %6s %6s %6s %6s %8s\n",
        "type", "mach", "vers", "entry", "phoff", "shoff", "flags",
        "ehsize", "phsize", "phnum", "shsize", "shnum", "shstrndx");
    emu_tracef("emu_load:      %6d %6d %6d %016llx %016llx %016llx "
        "%6d %6d %6d %6d %6d %6d %8d\n",
        ehdr.e_type, ehdr.e_machine, ehdr.e_version, ehdr.e_entry,
        ehdr.e_phoff, ehdr.e_shoff, ehdr.e_flags, ehdr.e_ehsize,
        ehdr.e_phentsize, ehdr.e_phnum, ehdr.e_shentsize, ehdr.e_shnum,
        ehdr.e_shstrndx);

    // print program headers
    emu_tracef("emu_load:      %6s %6s %6s %16s %16s %16s %16s %16s %16s\n",
        "idx", "type", "flags", "offset", "vaddr", "paddr", "filesz", "memsz",
        "align");
    Elf64_Phdr *phdr_begin = (Elf64_Phdr*)((uchar*)addr + ehdr.e_phoff);
    Elf64_Phdr *phdr_end = phdr_begin + ehdr.e_phnum;
    for (Elf64_Phdr *phdr_ptr = phdr_begin; phdr_ptr != phdr_end; phdr_ptr++) {
        Elf64_Phdr phdr;
        int i = (int)(phdr_ptr - phdr_begin);
        memcpy(&phdr, phdr_ptr, sizeof(phdr));
        if (phdr.p_type == PT_LOAD && phdr.p_filesz > 0) {
            emu_tracef("emu_load:      %6d %6d %6s %016llx %016llx "
                "%016llx %016llx %016llx %016llx\n", i, phdr.p_type,
                flags_str(phdr.p_flags), phdr.p_offset, phdr.p_vaddr,
                phdr.p_paddr, phdr.p_filesz, phdr.p_memsz, phdr.p_align);
        }
    }

    // load text+data specified in program headers
    for (Elf64_Phdr *phdr_ptr = phdr_begin; phdr_ptr != phdr_end; phdr_ptr++) {
        Elf64_Phdr phdr;
        memcpy(&phdr, phdr_ptr, sizeof(phdr));
        if (phdr.p_type == PT_LOAD && phdr.p_filesz > 0) {
            memcpy((uchar*)sys->mem + phdr.p_vaddr,
                (uchar*)addr + phdr.p_offset, phdr.p_filesz);
            //emu_dump_mem(sys, phdr.p_vaddr, phdr.p_filesz);
        }
        if (phdr.p_memsz > 0) {
            ullong phys_addr = phdr.p_vaddr & ~PAGE_MASK;
            ullong phys_len = ((phdr.p_vaddr + phdr.p_memsz + PAGE_MASK)
                & ~(PAGE_MASK)) - phys_addr;
            switch (flags_attr(phdr.p_flags)) {
            case emu_mem_attr_w:
            case emu_mem_attr_r:
            case emu_mem_attr_rw:
                emu_map_mem(sys, emu_mem_desc{emu_mem_type_boot_data,
                    emu_mem_attr_rwx, phys_addr, 0, phys_len});
                break;
            case emu_mem_attr_x:
            case emu_mem_attr_wx:
            case emu_mem_attr_rx:
            case emu_mem_attr_rwx:            
                emu_map_mem(sys, emu_mem_desc{emu_mem_type_boot_code,
                    emu_mem_attr_rwx, phys_addr, 0, phys_len});
                break;
            case emu_mem_attr_none:
            default: break;
            }
        }
    }

error:
    if (!UnmapViewOfFile(addr)) {
        emu_errorf("emu_load: error: UnmapViewOfFile: file=%s err=%d\n",
            filename, GetLastError());
    }
    CloseHandle(fmaph);
    CloseHandle(fh);

    return 0;
}

int emu_dump_mem(emu_system *sys, ullong offset, ullong len)
{
    ullong i = 0;
    uchar *addr = (uchar*)sys->mem + offset;
    while (i < len) {
        if (i % 16 == 0) emu_debugf("%012llx: ", offset + i);
        emu_debugf("%02hhx", addr[i]);
        if (i % 16 == 15) emu_debugf("\n");
        i++;
    }
    emu_debugf("\n");
    return 0;
}

int emu_dump_regs(emu_cpu *cpu)
{
    WHV_RUN_VP_EXIT_CONTEXT *exit = &cpu->exit;
    WHV_VP_EXIT_CONTEXT *vpctx = &exit->VpContext;

    WHV_REGISTER_VALUE *reg_values = (WHV_REGISTER_VALUE*)
        calloc(array_size(all_reg_names), sizeof(WHV_REGISTER_VALUE));

    CHECK_HRESULT(WHvGetVirtualProcessorRegisters(cpu->sys->part, cpu->vpi,
        all_reg_names, array_size(all_reg_names), reg_values));

    emu_debugf("%04x: %10s:0x%04x", cpu->vpi, "ExitReason", cpu->exit.ExitReason);
    emu_debugf("      %s:%d", "Cpl", vpctx->ExecutionState.Cpl);
    emu_debugf("      %s:%d", "Cr0.PE", vpctx->ExecutionState.Cr0Pe);
    emu_debugf("      %s:%d", "Cr0.AM", vpctx->ExecutionState.Cr0Am);
    emu_debugf("      %s:%d", "EferLma", vpctx->ExecutionState.EferLma);
    emu_debugf("      %s:%d", "DebugAct", vpctx->ExecutionState.DebugActive);
    emu_debugf("      %s:%d", "IntPending", vpctx->ExecutionState.InterruptionPending);
    emu_debugf("      %s:%d", "InstLength", vpctx->InstructionLength);
    emu_debugf("      %s:%d\n", "Cr8", vpctx->Cr8);
    emu_debugf("%16s:0x%016llx 0x%08x Idx:0x%03x Loc:%d Cpl:%d  ", "Cs",
           vpctx->Cs.Base,
           vpctx->Cs.Limit,
           vpctx->Cs.Selector >> 3,
           ((vpctx->Cs.Selector >> 2) & 1),
           vpctx->Cs.Selector & 3);
    emu_debugf("%16s:0x%016llx", "Rip", vpctx->Rip);
    emu_debugf("%16s:0x%06llx\n", "Rflags", vpctx->Rflags);

    int cols, colc = 0;
    for (size_t i = 0; i < array_size(all_reg_names); i++) {
        const char *mnem = all_reg_mnemonics[i];
        switch (all_reg_types[i]) {
        case SEG:
            emu_debugf("%16s:0x%016llx 0x%08x Idx:0x%03x Loc:%d Dpl:%d  ",
                mnem,
                reg_values[i].Segment.Base,
                reg_values[i].Segment.Limit,
                reg_values[i].Segment.Selector >> 3,
                (reg_values[i].Segment.Selector >> 2) & 1,
                reg_values[i].Segment.DescriptorPrivilegeLevel);
            cols = 2;
            break;
        case TAB:
            emu_debugf("%16s:0x%016llx 0x%04x " "                           ",
                mnem,
                reg_values[i].Table.Base,
                reg_values[i].Table.Limit);
            cols = 2;
            break;
        case GPR:
        case CR:
        case DBG:
        case XCR:
        case AVX:
        case FPU:
        case CSR:
        case MSR:
        default:
            emu_debugf("%16s:0x%016llx", mnem, reg_values[i].Reg64);
            cols = 4;
            break;
        }
        if (colc % cols == cols - 1) {
            emu_debugf("\n");
            colc = 0;
        } else {
            colc++;
        }
    }
    emu_debugf("\n");
    fflush(stdout);

    free(reg_values);
    return 0;

error:
    free(reg_values);
    return -1;
}

int emu_set_vmcall(emu_cpu *cpu, emu_vmcall_fn fn, void *ctx)
{
    cpu->vmcall_fn = fn;
    cpu->vmcall_ctx = ctx;
    return 0;
}

int emu_get_regs(emu_cpu *cpu, uint *regs, uint count, ullong *values)
{
    WHV_REGISTER_NAME *reg_names = (WHV_REGISTER_NAME*)
        alloca(sizeof(WHV_REGISTER_NAME) * count);
    WHV_REGISTER_VALUE *reg_values = (WHV_REGISTER_VALUE*)
        alloca(sizeof(WHV_REGISTER_VALUE) * count);

    for (size_t i = 0; i < count; i++) {
        reg_names[i] = (WHV_REGISTER_NAME)((uint)WHvX64RegisterRax + regs[i]);
    }

    CHECK_HRESULT(WHvGetVirtualProcessorRegisters(cpu->sys->part, cpu->vpi,
        reg_names, count, reg_values));

    for (uint i = 0; i < count; i++) {
        values[i] = reg_values[i].Reg64;
    }

    return 0;

error:
    return -1;
}

int emu_set_regs(emu_cpu *cpu, uint *regs, uint count, ullong *values)
{
    WHV_REGISTER_NAME *reg_names = (WHV_REGISTER_NAME*)
        alloca(sizeof(WHV_REGISTER_NAME) * count);
    WHV_REGISTER_VALUE *reg_values = (WHV_REGISTER_VALUE*)
        alloca(sizeof(WHV_REGISTER_VALUE) * count);

    for (uint i = 0; i < count; i++) {
        reg_names[i] = (WHV_REGISTER_NAME)((uint)WHvX64RegisterRax + regs[i]);
        reg_values[0].Reg64 = values[i];
    }

    CHECK_HRESULT(WHvSetVirtualProcessorRegisters(cpu->sys->part, cpu->vpi,
        reg_names, count, reg_values));

    return 0;

error:
    return -1;
}

int emu_mmio(emu_cpu *cpu)
{
    WHV_RUN_VP_EXIT_CONTEXT *exit = &cpu->exit;

    emu_tracef("emu_mmio: gpa:0x%llx gva:0x%llx "
        "info:[accesstype:%d,gpaunmapped:%d,gvavalid:%d]\n",
        exit->MemoryAccess.Gpa,
        exit->MemoryAccess.Gva,
        exit->MemoryAccess.AccessInfo.AccessType,
        exit->MemoryAccess.AccessInfo.GpaUnmapped,
        exit->MemoryAccess.AccessInfo.GvaValid);

    CHECK_HRESULT(WHvEmulatorTryMmioEmulation(cpu->emu, cpu,
        &exit->VpContext, &exit->MemoryAccess, &cpu->status));

    if (!cpu->status.EmulationSuccessful) {
        emu_errorf("emu_mmio: emulation failed: err=%u\n",
            cpu->status.AsUINT32);
        return -1;
    }

    return 0;

error:
    return -1;
}

int emu_ioport(emu_cpu *cpu)
{
    WHV_RUN_VP_EXIT_CONTEXT *exit = &cpu->exit;

    CHECK_HRESULT(WHvEmulatorTryIoEmulation(cpu->emu, cpu,
        &exit->VpContext, &exit->IoPortAccess, &cpu->status));

    if (!cpu->status.EmulationSuccessful) {
        emu_errorf("emu_ioport: emulation failed: err=%u\n",
            cpu->status.AsUINT32);
        return -1;
    }

    return 0;

error:
    return -1;
}

int emu_cpuid(emu_cpu *cpu)
{
    WHV_RUN_VP_EXIT_CONTEXT *exit = &cpu->exit;
    WHV_REGISTER_NAME reg_names[5] = {
        WHvX64RegisterRax, WHvX64RegisterRcx,
        WHvX64RegisterRdx, WHvX64RegisterRbx,
        WHvX64RegisterRip
    };
    WHV_REGISTER_VALUE reg_values[5];
    int level, count, info[4];

    CHECK_HRESULT(WHvGetVirtualProcessorRegisters(cpu->sys->part, cpu->vpi,
        reg_names, array_size(reg_names), reg_values));

    level = reg_values[0].Reg32;
    count = reg_values[1].Reg32;
    __cpuidex(info, level, count);
    reg_values[0].Reg64 = info[0];
    reg_values[1].Reg64 = info[1];
    reg_values[2].Reg64 = info[2];
    reg_values[3].Reg64 = info[3];
    reg_values[4].Reg64 = exit->VpContext.Rip + exit->VpContext.InstructionLength;

    emu_debugf("emu_cpuid: level:0x%08x count:0x%08x "
        "result:[0x%08x,0x%08x,0x%08x,0x%08x]\n",
        level, count, info[0], info[1], info[2], info[3]);

    CHECK_HRESULT(WHvSetVirtualProcessorRegisters(cpu->sys->part, cpu->vpi,
        reg_names, array_size(reg_names), reg_values));

    return 0;

error:
    return -1;
}

static constexpr WHV_REGISTER_NAME msrnuemu_to_reg(int msrnum)
{
    switch(msrnum) {
    case MSR_IA32_TSC_ADJUST: return WHvX64RegisterTsc;
    case MSR_IA32_TSC_AUX: return WHvX64RegisterTscAux;
    case MSR_IA32_APIC_BASE: return WHvX64RegisterApicBase;
    case MSR_IA32_EFER: return WHvX64RegisterEfer;
    case MSR_IA32_PAT: return WHvX64RegisterPat;
    case MSR_IA32_STAR: return WHvX64RegisterStar;
    case MSR_IA32_LSTAR: return WHvX64RegisterLstar;
    case MSR_IA32_CSTAR: return WHvX64RegisterCstar;
    case MSR_IA32_FMASK: return WHvX64RegisterSfmask;
    case MSR_IA32_KERNEL_GS_BASE: return WHvX64RegisterKernelGsBase;
    default: break;
    }
    return WHvX64RegisterRax;
}

int emu_msr(emu_cpu *cpu)
{
    WHV_RUN_VP_EXIT_CONTEXT *exit = &cpu->exit;
    WHV_REGISTER_NAME reg_names[4] = {
        msrnuemu_to_reg(exit->MsrAccess.MsrNumber),
        WHvX64RegisterRax, WHvX64RegisterRdx,
        WHvX64RegisterRip
    };
    WHV_REGISTER_VALUE reg_values[4];

    if (reg_names[0] == WHvX64RegisterRax) {
        emu_errorf("emu_msr: unknown msr 0x%08x\n", exit->MsrAccess.MsrNumber);
        return -1;
    }

    emu_debugf("emu_msr: msrnum:0x%08x\n", exit->MsrAccess.MsrNumber);

    CHECK_HRESULT(WHvGetVirtualProcessorRegisters(cpu->sys->part, cpu->vpi,
        reg_names, array_size(reg_names), reg_values));

    if (exit->MsrAccess.AccessInfo.IsWrite) {
        llong v = (exit->MsrAccess.Rax & 0xffffffff) |
                  (exit->MsrAccess.Rdx << 32);
        reg_values[0].Reg64 = v;
    } else {
        reg_values[1].Reg64 = (reg_values[0].Reg64 >> 0)  & 0xffffffff;
        reg_values[2].Reg64 = (reg_values[0].Reg64 >> 32) & 0xffffffff;
    }
    reg_values[3].Reg64 = exit->VpContext.Rip + exit->VpContext.InstructionLength;

    CHECK_HRESULT(WHvSetVirtualProcessorRegisters(cpu->sys->part, cpu->vpi,
        reg_names, array_size(reg_names), reg_values));

    return 0;

error:
    return -1;
}

static inline const char* x86_exception_str(int exception)
{
    switch (exception) {
    case x86_exception_divide: return "divide";
    case x86_exception_debug: return "debug";
    case x86_exception_nmi: return "nmi";
    case x86_exception_breakpoint: return "breakpoint";
    case x86_exception_overflow:return "overflow";
    case x86_exception_bound_range: return "bound_range";
    case x86_exception_invalid_opcode: return "invalid_opcode";
    case x86_exception_fpu_unavail: return "fpu_unavail";
    case x86_exception_double_fault: return "double_fault";
    case x86_exception_fpu_reserved: return "fpu_reserved";
    case x86_exception_invalid_tss: return "invalid_tss";
    case x86_exception_segment_fault: return "segment_fault";
    case x86_exception_stack_fault: return "stack_fault";
    case x86_exception_general_prot: return "general_prot";
    case x86_exception_page_fault: return "page_fault";
    case x86_exception_reserved: return "reserved";
    case x86_exception_math_fault: return "math_fault";
    case x86_exception_align_check: return "align_check";
    case x86_exception_machine_check: return "machine_check";
    case x86_exception_simd_fault: return "simd_fault";
    case x86_exception_ept_exception: return "ept_exception";
    case x86_exception_cet_exception: return "cet_exception";
    default: return "unknown";
    }
}

int emu_exception(emu_cpu *cpu)
{
    WHV_RUN_VP_EXIT_CONTEXT *exit = &cpu->exit;
    WHV_REGISTER_NAME reg_names[1] = {
        WHvX64RegisterRip
    };
    WHV_REGISTER_VALUE reg_values[1];

    uchar *insn = exit->VpException.InstructionBytes;
    uint insn_len = exit->VpException.InstructionByteCount;
    uint step_len = 0;

    emu_tracef("emu_exception: type=%s code=%d parameter=%lld\n",
        x86_exception_str(exit->VpException.ExceptionType),
        exit->VpException.ErrorCode,
        exit->VpException.ExceptionParameter);

    switch (exit->VpException.ExceptionType) {
    case x86_exception_debug:
    case x86_exception_breakpoint:
        break;
    case x86_exception_invalid_opcode:
        // Intel vmcall (0f 01 c1), AMD vmmcall (0f 01 d9)
        if (insn_len >= 3 && insn[0] == 0x0f && insn[1] == 0x01 &&
            (insn[2] == 0xc1 || insn[2] == 0xd9)) {
            step_len = 3;
            cpu->vmcall_fn(cpu, cpu->vmcall_ctx);
        }
        break;
    }

    if (step_len > 0) {
        reg_values[0].Reg64 = exit->VpContext.Rip + step_len;
        CHECK_HRESULT(WHvSetVirtualProcessorRegisters(cpu->sys->part, cpu->vpi,
            reg_names, array_size(reg_names), reg_values));
    } else {
        emu_halt(cpu->sys);
    }

    return 0;

error:
    return -1;
}

int emu_create_sys(emu_system **sysp, ullong mem_size, int ncpus)
{
    WHV_PARTITION_PROPERTY prop;
    WHV_CAPABILITY cap;
    uint cap_size;
    unsigned cpuid_exit_list[64];

    emu_system *sys = *sysp = new emu_system(mem_size, ncpus);

    CHECK_HRESULT(WHvGetCapability(WHvCapabilityCodeHypervisorPresent,
        &cap, sizeof(cap), &cap_size));

    if (!cap.HypervisorPresent) {
        emu_errorf("hypervisor not present\n");
        goto error;
    }

    CHECK_HRESULT(WHvCreatePartition(&sys->part));

    memset(&prop, 0, sizeof(prop));
    prop.ProcessorCount = ncpus;

    CHECK_HRESULT(WHvSetPartitionProperty(sys->part,
                                 WHvPartitionPropertyCodeProcessorCount,
                                 &prop, sizeof(prop)));

    memset(&prop, 0, sizeof(prop));
    prop.ExtendedVmExits.X64CpuidExit = 1;
    prop.ExtendedVmExits.X64MsrExit = 0;
    prop.ExtendedVmExits.ExceptionExit  = 1;

    CHECK_HRESULT(WHvSetPartitionProperty(sys->part,
        WHvPartitionPropertyCodeExtendedVmExits, &prop, sizeof(prop)));

    memset(&prop, 0, sizeof(prop));
    prop.ExceptionExitBitmap = (1LL << WHvX64ExceptionTypeDebugTrapOrFault)
                             | (1LL << WHvX64ExceptionTypeBreakpointTrap)
                             | (1LL << WHvX64ExceptionTypeInvalidOpcodeFault);

    CHECK_HRESULT(WHvSetPartitionProperty(sys->part,
        WHvPartitionPropertyCodeExceptionExitBitmap,  &prop, sizeof(prop)));

    memset(&prop, 0, sizeof(prop));
    prop.X64MsrExitBitmap.UnhandledMsrs = 1;
    prop.X64MsrExitBitmap.TscMsrWrite = 1;
    prop.X64MsrExitBitmap.ApicBaseMsrWrite = 1;

    CHECK_HRESULT(WHvSetPartitionProperty(sys->part,
        WHvPartitionPropertyCodeX64MsrExitBitmap, &prop, sizeof(prop)));

    for (int i = 0; i < 32; i++) {
        cpuid_exit_list[i] = i;
        cpuid_exit_list[i+32] = 0x80000000 + i;
    }

    CHECK_HRESULT(WHvSetPartitionProperty(sys->part,
        WHvPartitionPropertyCodeCpuidExitList,
        cpuid_exit_list, sizeof(cpuid_exit_list)));

    memset(&prop, 0, sizeof(prop));
    prop.LocalApicEmulationMode = WHvX64LocalApicEmulationModeXApic;

    CHECK_HRESULT(WHvSetPartitionProperty(sys->part,
        WHvPartitionPropertyCodeLocalApicEmulationMode,
        &prop, sizeof(prop)));

    CHECK_HRESULT(WHvSetupPartition(sys->part));

    sys->mem = VirtualAlloc(NULL, sys->mem_size,
        MEM_COMMIT, PAGE_EXECUTE_READWRITE);

    CHECK_HRESULT(WHvMapGpaRange(sys->part, sys->mem, 0, sys->mem_size,
        WHvMapGpaRangeFlagRead |
        WHvMapGpaRangeFlagWrite |
        WHvMapGpaRangeFlagExecute));

    emu_map_mem(sys, emu_mem_desc{emu_mem_type_main_memory,
        emu_mem_attr_rwx, 0, 0, sys->mem_size});

    return 0;

error:
    return -1;
}

int emu_create_cpu(emu_cpu **cpup, emu_system *sys, int vpi)
{
    WHV_REGISTER_VALUE *reg_values = (WHV_REGISTER_VALUE*)
        calloc(array_size(init_reg_names), sizeof(WHV_REGISTER_VALUE));

    emu_cpu *cpu = *cpup = new emu_cpu(sys, vpi, true);

    cpu->eemu_cb.Size = sizeof(cpu->eemu_cb);
    cpu->eemu_cb.Reserved = 0;
    cpu->eemu_cb.WHvEmulatorIoPortCallback = emu_ioport_emu;
    cpu->eemu_cb.WHvEmulatorMemoryCallback = emu_mmio_emu;
    cpu->eemu_cb.WHvEmulatorGetVirtualProcessorRegisters = emu_getreg_emu;
    cpu->eemu_cb.WHvEmulatorSetVirtualProcessorRegisters = emu_setreg_emu;
    cpu->eemu_cb.WHvEmulatorTranslateGvaPage = emu_translate_emu;

    CHECK_HRESULT(WHvEmulatorCreateEmulator(&cpu->eemu_cb, &cpu->emu));
    CHECK_HRESULT(WHvCreateVirtualProcessor(sys->part, vpi, 0));
    CHECK_HRESULT(WHvSetVirtualProcessorRegisters(sys->part, vpi,
        init_reg_names, array_size(init_reg_names), reg_values));

    sys->cpulist.insert(sys->cpulist.end(), cpu);

    free(reg_values);
    return 0;

error:
    free(reg_values);
    return -1;
}

static inline const char* exit_reason_str(WHV_RUN_VP_EXIT_REASON reason)
{
    switch (reason) {
    case WHvRunVpExitReasonNone: return "None";
    case WHvRunVpExitReasonMemoryAccess: return "MemoryAccess";
    case WHvRunVpExitReasonX64IoPortAccess: return "X64IoPortAccess";
    case WHvRunVpExitReasonUnrecoverableException: return "UnrecoverableException";
    case WHvRunVpExitReasonInvalidVpRegisterValue: return "InvalidVpRegisterValue";
    case WHvRunVpExitReasonUnsupportedFeature: return "UnsupportedFeature";
    case WHvRunVpExitReasonX64InterruptWindow: return "X64InterruptWindow";
    case WHvRunVpExitReasonX64Halt: return "X64Halt";
    case WHvRunVpExitReasonX64MsrAccess: return "X64MsrAccess";
    case WHvRunVpExitReasonX64Cpuid: return "X64Cpuid";
    case WHvRunVpExitReasonException: return "Exception";
    case WHvRunVpExitReasonX64Rdtsc: return "X64Rdtsc";
    case WHvRunVpExitReasonCanceled: return "Canceled";
    default: return "Unknown";
    }
}

int emu_halt(emu_system *sys)
{
    for (emu_cpu *cpu : sys->cpulist) {
        cpu->running = false;
        WHvCancelRunVirtualProcessor(sys->part, cpu->vpi, 0);
    }
    return 0;
}

int emu_running(emu_cpu *cpu)
{
    return cpu->running;
}

int emu_launch(emu_cpu *cpu)
{
    ullong st, et;

    if (!cpu->running) {
        emu_errorf("emu_launch: cpu-%-3d not running\n", cpu->vpi);
        goto error;
    }

    st = __rdtsc();
    emu_tracef("emu_launch: cpu-%-3d vmlaunch tsc:%llu\n", cpu->vpi, st);
    CHECK_HRESULT(WHvRunVirtualProcessor(cpu->sys->part, cpu->vpi,
        &cpu->exit, sizeof(cpu->exit)));
    et = __rdtsc();
    emu_tracef("emu_launch: cpu-%-3d   vmexit tsc:%llu reason:%s\n",
        cpu->vpi, et, exit_reason_str(cpu->exit.ExitReason));

    switch (cpu->exit.ExitReason) {
    case WHvRunVpExitReasonMemoryAccess:
        CHECK_ERROR(emu_mmio(cpu)); break;
    case WHvRunVpExitReasonX64IoPortAccess:
        CHECK_ERROR(emu_ioport(cpu)); break;
    case WHvRunVpExitReasonX64Cpuid:
        CHECK_ERROR(emu_cpuid(cpu)); break;
    case WHvRunVpExitReasonX64MsrAccess:
        CHECK_ERROR(emu_msr(cpu)); break;
    case WHvRunVpExitReasonException:
        CHECK_ERROR(emu_exception(cpu)); break;
    case WHvRunVpExitReasonUnrecoverableException:
    case WHvRunVpExitReasonInvalidVpRegisterValue:
    case WHvRunVpExitReasonUnsupportedFeature:
    case WHvRunVpExitReasonX64InterruptWindow:
    case WHvRunVpExitReasonX64Halt:
    case WHvRunVpExitReasonX64ApicEoi:
    case WHvRunVpExitReasonCanceled:
    default:
        cpu->running = false;
        break;
    }

    return 0;

error:
    return -1;
}

int emu_lerror(emu_cpu *cpu)
{
    switch (cpu->exit.ExitReason) {
    case WHvRunVpExitReasonNone:
    case WHvRunVpExitReasonCanceled:
        return 0;
    default:
        return 1;
    }
}

int emu_destroy_cpu(emu_cpu *cpu)
{
    emu_system *sys = cpu->sys;

    auto i = std::find(sys->cpulist.begin(), sys->cpulist.end(), cpu);
    if (i != sys->cpulist.end()) {
        sys->cpulist.erase(i);
    }

    CHECK_HRESULT(WHvDeleteVirtualProcessor(cpu->sys->part, cpu->vpi));
    CHECK_HRESULT(WHvEmulatorDestroyEmulator(cpu->emu));

    delete cpu;

    return 0;

error:
    return -1;
}

int emu_destroy_sys(emu_system *sys)
{
    std::vector<emu_device*> devlistcopy = sys->devlist;

    CHECK_HRESULT(WHvUnmapGpaRange(sys->part, 0, sys->mem_size));
    CHECK_BOOL(VirtualFree(sys->mem, 0, MEM_RELEASE));
    CHECK_HRESULT(WHvDeletePartition(sys->part));

    for (emu_device* dev : devlistcopy) {
        CHECK_ERROR(emu_destroy_device(dev));
    }

    delete sys;

    return 0;

error:
    return -1;
}
