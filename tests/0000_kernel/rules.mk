kern_objs = kern/boot.o \
	kern/start.o kern/main.o \
	kern/console.o kern/uart.o \
	kern/poweroff.o kern/cf9.o

lib_objs = lib/abort.o lib/exit.o \
	lib/malloc.o lib/memchr.o \
	lib/memcmp.o lib/memcpy.o \
	lib/memset.o lib/strchr.o \
	lib/strcmp.o lib/strlen.o \
	lib/strncmp.o lib/strncpy.o \
	lib/putchar.o lib/puts.o \
	lib/printf.o lib/vprintf.o \
	lib/vsnprintf.o

0000_kernel_objs = $(kern_objs) $(lib_objs)
