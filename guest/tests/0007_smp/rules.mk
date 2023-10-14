0007_smp_objs = bpboot.o apboot.o \
                bpstart.o apstart.o \
                trapentry.o trap.o \
                x86_cpu.o x86_lapic.o \
                main.o
0007_smp_guest_libs = kern.a libc.a
0007_smp_common_libs = crypto.a
