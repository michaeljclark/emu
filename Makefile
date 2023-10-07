CC=cc
LD=ld
AR=ar

COMMON_CFLAGS=-g -O2 -std=gnu11 -Icommon/include -Iguest/include \
	-m64 -mno-sse -fpie -ffunction-sections -fdata-sections \
	-fno-omit-frame-pointer -fno-stack-protector
GUEST_CFLAGS = $(COMMON_CFLAGS) -nostdinc
GUEST_LDFLAGS = --nmagic --gc-sections -T guest/tests/default.lds

OBJ=build/out/obj
BIN=build/out/bin
BOOT=build/out/x86

all: all_exec
clean: ; rm -fr build/out

# rules for kern and libc

kern_objs = \
	guest/kern/console.o guest/kern/uart.o \
	guest/kern/poweroff.o guest/kern/cf9.o \
	guest/kern/mem.o guest/kern/page.o \
	guest/kern/vmm.o

$(OBJ)/guest/kern/%.o: guest/kern/%.c
	mkdir -p $(dir $@) ; $(CC) $(GUEST_CFLAGS) -c -o $@ $^
$(OBJ)/guest/kern.a: $(addprefix $(OBJ)/,$(kern_objs))
	mkdir -p $(dir $@) ; $(AR) cr $@ $^

libc_objs =	\
	guest/libc/abort.o guest/libc/exit.o \
	guest/libc/malloc.o guest/libc/memchr.o \
	guest/libc/memcmp.o guest/libc/memcpy.o \
	guest/libc/memset.o guest/libc/strchr.o \
	guest/libc/strcmp.o guest/libc/strlen.o \
	guest/libc/strncmp.o guest/libc/strncpy.o \
	guest/libc/putchar.o guest/libc/puts.o \
	guest/libc/printf.o guest/libc/vprintf.o \
	guest/libc/vsnprintf.o

$(OBJ)/guest/libc/%.o: guest/libc/%.c
	mkdir -p $(dir $@) ; $(CC) $(GUEST_CFLAGS) -c -o $@ $^
$(OBJ)/guest/libc.a: $(addprefix $(OBJ)/,$(libc_objs))
	mkdir -p $(dir $@) ; $(AR) cr $@ $^

# pattern rules for guest tests

test_root = guest/tests
test_makes := $(foreach dir,$(test_root),$(wildcard ${dir}/*/rules.mk))
$(foreach makefile,$(test_makes),$(eval include $(makefile)))
test_dirs := $(foreach m,$(test_makes),$(m:/rules.mk=))
test_name = $(lastword $(subst /, ,$(1)))
test_objs = $(addprefix $(OBJ)/,$(addprefix $(1)/,$($(2)_objs)))
test_libs = $(addprefix $(OBJ)/,$(addprefix $(1)/,$($(2)_libs)))

define test_rule =
$(OBJ)/$(1)/%.o: $(1)/%.[cS]
	mkdir -p $$(dir $$@) ; $$(CC) $$(GUEST_CFLAGS) -I$(1)/include -c -o $$@ $$^
$(BOOT)/$(1)/system.elf: $(call test_objs,$(1),$(2)) $(call test_libs,guest,$(2))
	mkdir -p $$(dir $$@) ; $$(LD) $$(GUEST_LDFLAGS) -o $$@ $$^
all_exec += $(BOOT)/$(1)/system.elf
endef

$(foreach d,$(test_dirs),$(eval $(call test_rule,$(d),$(call test_name,$(d)))))

# pattern rules for common tests

common_root = common/tests
common_makes := $(foreach dir,$(common_root),$(wildcard ${dir}/*/rules.mk))
$(foreach makefile,$(common_makes),$(eval include $(makefile)))
common_dirs := $(foreach m,$(common_makes),$(m:/rules.mk=))
common_name = $(lastword $(subst /, ,$(1)))
common_objs = $(addprefix $(OBJ)/,$(addprefix $(1)/,$($(2)_objs)))

define common_rule =
$(OBJ)/$(1)/%.o: $(1)/%.c
	mkdir -p $$(dir $$@) ; $$(CC) $$(COMMON_CFLAGS) -I$(1)/include -c -o $$@ $$^
$(BIN)/$(1)/test: $(call common_objs,$(1),$(2))
	mkdir -p $$(dir $$@) ; $$(CC) $$(COMMON_CFLAGS) -o $$@ $$^
all_exec += $(BIN)/$(1)/test
endef

$(foreach d,$(common_dirs),$(eval $(call common_rule,$(d),$(call common_name,$(d)))))

all_exec: $(all_exec)
