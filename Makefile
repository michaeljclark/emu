CC=cc
LD=ld
CFLAGS=-g -O2 -m64 -std=gnu99 -mno-sse -nostdinc -Iemu/include \
	-fpie -fno-stack-protector -fno-omit-frame-pointer \
	-ffunction-sections -fdata-sections

LDFLAGS = --nmagic --gc-sections
OBJ=build/out/obj
BOOT=build/out/x86

all: all_exec
clean: ; rm -fr build/out

# pattern rules for tests

test_root = tests
test_makes := $(foreach dir,$(test_root),$(wildcard ${dir}/*/rules.mk))
$(foreach makefile,$(test_makes),$(eval include $(makefile)))
test_dirs := $(foreach m,$(test_makes),$(m:/rules.mk=))
test_name = $(lastword $(subst /, ,$(1)))
test_objs = $(addprefix $(OBJ)/,$(addprefix $(1)/,$($(2)_objs)))

define test_rule =
$(OBJ)/$(1)/%.o: $(1)/%.[cS]
	mkdir -p $$(dir $$@) ; $$(CC) $$(CFLAGS) -I$(1)/include -c -o $$@ $$^
$(BOOT)/$(1)/system.elf: $(call test_objs,$(1),$(2))
	mkdir -p $$(dir $$@) ; $$(LD) $$(LDFLAGS) -T tests/default.lds -o $$@ $$^
all_exec += $(BOOT)/$(1)/system.elf
endef

$(foreach d,$(test_dirs),$(eval $(call test_rule,$(d),$(call test_name,$(d)))))

all_exec: $(all_exec)
