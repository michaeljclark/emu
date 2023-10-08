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

# pattern rules for common libs

common_lib_dirs = common/crypto
common_lib_objs = $(addprefix $(OBJ)/,$(patsubst %.c,%.o,$(wildcard $(1)/*.c)))
common_lib_name = $(lastword $(subst /, ,$(1)))
common_lib_var = $(addprefix $(OBJ)/,$(addprefix $(1)/,$($(2)_common_libs)))

define common_lib_rule =
$(OBJ)/$(1)/%.o: $(1)/%.c
	mkdir -p $$(dir $$@) ; $$(CC) $$(GUEST_CFLAGS) -c -o $$@ $$^
$(OBJ)/common/$(2).a: $(call common_lib_objs,$(1))
	mkdir -p $$(dir $$@) ; $$(AR) cr $$@ $$^
endef

$(foreach d,$(common_lib_dirs),$(eval $(call common_lib_rule,$(d),$(call common_lib_name,$(d)))))

# pattern rules for guest libs

guest_lib_dirs = guest/kern guest/libc
guest_lib_objs = $(addprefix $(OBJ)/,$(patsubst %.c,%.o,$(wildcard $(1)/*.c)))
guest_lib_name = $(lastword $(subst /, ,$(1)))
guest_lib_var = $(addprefix $(OBJ)/,$(addprefix $(1)/,$($(2)_guest_libs)))

define guest_lib_rule =
$(OBJ)/$(1)/%.o: $(1)/%.c
	mkdir -p $$(dir $$@) ; $$(CC) $$(GUEST_CFLAGS) -c -o $$@ $$^
$(OBJ)/guest/$(2).a: $(call guest_lib_objs,$(1))
	mkdir -p $$(dir $$@) ; $$(AR) cr $$@ $$^
endef

$(foreach d,$(guest_lib_dirs),$(eval $(call guest_lib_rule,$(d),$(call guest_lib_name,$(d)))))

# pattern rules for common tests

common_test_root = common/tests
common_test_makes := $(foreach dir,$(common_test_root),$(wildcard ${dir}/*/rules.mk))
$(foreach makefile,$(common_test_makes),$(eval include $(makefile)))
common_test_dirs := $(foreach m,$(common_test_makes),$(m:/rules.mk=))
common_test_name = $(lastword $(subst /, ,$(1)))
common_test_objs = $(addprefix $(OBJ)/,$(addprefix $(1)/,$($(2)_objs)))

define common_test_rule =
$(OBJ)/$(1)/%.o: $(1)/%.c
	mkdir -p $$(dir $$@) ; $$(CC) $$(COMMON_CFLAGS) -I$(1)/include -c -o $$@ $$^
$(BIN)/test_$(2): $(call common_test_objs,$(1),$(2)) $(call common_lib_var,common,$(2))
	mkdir -p $$(dir $$@) ; $$(CC) $$(COMMON_CFLAGS) -o $$@ $$^
all_exec += $(BIN)/test_$(2)
endef

$(foreach d,$(common_test_dirs),$(eval $(call common_test_rule,$(d),$(call common_test_name,$(d)))))

# pattern rules for guest tests

guest_test_root = guest/tests
guest_test_makes := $(foreach dir,$(guest_test_root),$(wildcard ${dir}/*/rules.mk))
$(foreach makefile,$(guest_test_makes),$(eval include $(makefile)))
guest_test_dirs := $(foreach m,$(guest_test_makes),$(m:/rules.mk=))
guest_test_name = $(lastword $(subst /, ,$(1)))
guest_test_objs = $(addprefix $(OBJ)/,$(addprefix $(1)/,$($(2)_objs)))

define guest_test_rule =
$(OBJ)/$(1)/%.o: $(1)/%.[cS]
	mkdir -p $$(dir $$@) ; $$(CC) $$(GUEST_CFLAGS) -I$(1)/include -c -o $$@ $$^
$(BOOT)/$(1)/system.elf: $(call guest_test_objs,$(1),$(2)) \
$(call guest_lib_var,guest,$(2)) $(call common_lib_var,common,$(2))
	mkdir -p $$(dir $$@) ; $$(LD) $$(GUEST_LDFLAGS) -o $$@ $$^
all_exec += $(BOOT)/$(1)/system.elf
endef

$(foreach d,$(guest_test_dirs),$(eval $(call guest_test_rule,$(d),$(call guest_test_name,$(d)))))

all_exec: $(all_exec)
