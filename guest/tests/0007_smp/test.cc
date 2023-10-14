#include <cstdio>
#include <cstddef>
#include <cstdlib>
#include <cstdint>
#include <cstring>
#include <cassert>

#include <uv.h>
#include <atomic.h>

#include "sys.h"

#define MAX_CPUS 64

int log_level = emu_log_debug;
int ncpus = 4;

typedef struct test_system test_system;
typedef struct test_ctx test_ctx;

struct test_system
{
    emu_system *sys;
    emu_cpu **cpu;
    emu_device *a20, *pit, *rcr, *uart;
};

struct test_ctx
{
    uv_thread_t t;
    test_system *s;
    int n, r;
};

void cpu_thread(void* arg)
{
    test_ctx *ctx = (test_ctx*)arg;
    while (emu_running(ctx->s->cpu[ctx->n])) emu_launch(ctx->s->cpu[ctx->n]);
    emu_halt(ctx->s->sys);
}

int main(int argc, char **argv)
{
    test_system s;
    test_ctx *c;
    llong mem_size = 1024 * 1024;
    int dump = argc > 1 && strcmp(argv[1], "-d") == 0;

    s.cpu = (emu_cpu **)_alloca(ncpus * sizeof(emu_cpu*));
    c = (test_ctx *)_alloca(ncpus * sizeof(test_ctx));

    CHECK_ERROR(emu_init());
    CHECK_ERROR(emu_create_sys(&s.sys, mem_size, ncpus));
    CHECK_ERROR(emu_create_device(&s.a20, s.sys, "a20", nullptr));
    CHECK_ERROR(emu_create_device(&s.rcr, s.sys, "rcr", nullptr));
    CHECK_ERROR(emu_create_device(&s.uart, s.sys, "uart", nullptr));
    CHECK_ERROR(emu_load(s.sys, "build/out/x86/guest/tests/0007_smp/system.elf"));
    for (int i = 0; i < ncpus; i++) {
        CHECK_ERROR(emu_create_cpu(&s.cpu[i], s.sys, i));
        CHECK_ERROR(emu_set_vmcall(s.cpu[i], emu_vmcall, nullptr));
    }
    for (int i = 0; i < ncpus; i++) {
        c[i].s = &s;
        c[i].n = i;
        CHECK_BOOL(uv_thread_create(&c[i].t, cpu_thread, &c[i]) == 0);
    }
    for (int i = 0; i < ncpus; i++) {
        CHECK_BOOL(uv_thread_join(&c[i].t) == 0);
    }
    for (int i = 0; i < ncpus; i++) {
        if (dump || emu_lerror(s.cpu[i])) {
            CHECK_ERROR(emu_dump_regs(s.cpu[i]));
        }
        CHECK_ERROR(emu_destroy_cpu(s.cpu[i]));
    }
    CHECK_ERROR(emu_destroy_sys(s.sys));

error:
    return 0;
}
