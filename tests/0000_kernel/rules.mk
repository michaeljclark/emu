kern_objs = kern/boot.o kern/main.o kern/console.o \
	kern/uart.o kern/poweroff.o kern/cf9.o

lib_objs = lib/memchr.o lib/memcmp.o lib/memcpy.o \
	lib/memset.o lib/strchr.o lib/strcmp.o \
	lib/strlen.o lib/strncmp.o lib/strncpy.o \
	lib/puts.o lib/printf.o lib/vprintf.o \
	lib/vsnprintf.o

0000_kernel_objs = $(kern_objs) $(lib_objs)
