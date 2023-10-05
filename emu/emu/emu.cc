#include <cstdio>
#include <cstddef>
#include <cstdlib>
#include <cstdint>
#include <cassert>

#include "sys.h"

int log_level = emu_log_debug;

static int opt_help;
static int opt_dump_reg;
static int opt_mem_size = 1;
static const char* opt_kernel;

static void print_help(int argc, char **argv)
{
    emu_infof(
        "usage: %s [options]\n"
        "\n"
        "Options:\n"
        "  -l, (error|info|debug|trace)       log level\n"
        "  -k, --kernel <file>                kernel image ELF file\n"
        "  -m, --memory <integer>             memory size in MiB\n"
        "  -d, --dump-reg                     dump registers\n"
        "  -h, --help                         help text\n",
        argv[0]
    );
}

static int match_opt(const char *arg, const char *opt, const char *longopt)
{
    return strcmp(arg, opt) == 0 || strcmp(arg, longopt) == 0;
}

static int parse_options(int argc, char **argv)
{
    int i = 1;
    while (i < argc) {
        if (match_opt(argv[i], "-l", "--level")) {
            char* level = argv[++i];
            if (strcmp(level, "none") == 0) {
                log_level = emu_log_none;
            } else if (strcmp(level, "error") == 0) {
                log_level = emu_log_error;
            } else if (strcmp(level, "info") == 0) {
                log_level = emu_log_info;
            } else if (strcmp(level, "debug") == 0) {
                log_level = emu_log_debug;
            } else if (strcmp(level, "trace") == 0) {
                log_level = emu_log_trace;
            }
            i++;
        } else if (match_opt(argv[i], "-k", "--kernel")) {
            opt_kernel = argv[++i];
            i++;
        } else if (match_opt(argv[i], "-m", "--memory")) {
            opt_mem_size = atoi(argv[++i]);
            i++;
        } else if (match_opt(argv[i], "-d", "--dump-reg")) {
            opt_dump_reg++;
            i++;
        } else if (match_opt(argv[i], "-h", "--help")) {
            opt_help++;
            i++;
        } else {
            emu_errorf("error: unknown option: %s\n", argv[i]);
            opt_help++;
            break;
        }
    }

    if (!opt_kernel) {
        emu_errorf("error: --kernel option missing\n");
        opt_help++;
    }

    if (opt_help) {
        print_help(argc, argv);
        return -1;
    }

    return 0;
}

static int emu_run(int argc, char **arg)
{
    emu_system *sys;
    emu_cpu *cpu;
    emu_device *a20, *rcr, *uart;

    CHECK_ERROR(emu_init());
    CHECK_ERROR(emu_create_sys(&sys, (ullong)opt_mem_size << 20));
    CHECK_ERROR(emu_create_cpu(&cpu, sys, 0));
    CHECK_ERROR(emu_create_device(&a20, sys, "a20", nullptr));
    CHECK_ERROR(emu_create_device(&rcr, sys, "rcr", nullptr));
    CHECK_ERROR(emu_create_device(&uart, sys, "uart", nullptr));
    CHECK_ERROR(emu_set_vmcall(cpu, emu_vmcall, nullptr));
    CHECK_ERROR(emu_load(sys, opt_kernel));
    while (emu_running(cpu)) emu_launch(cpu);
    if (opt_dump_reg) CHECK_ERROR(emu_dump_regs(cpu));
    CHECK_ERROR(emu_destroy_cpu(cpu));
    CHECK_ERROR(emu_destroy_sys(sys));

error:
    return 0;
}

int main(int argc, char **argv)
{
    if (parse_options(argc, argv) < 0) return 1;
    return emu_run(argc, argv);
}
