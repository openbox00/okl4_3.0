# 
# Copyright (C) 2002 Jeff Dike (jdike@karaya.com)
# Licensed under the GPL
#

ARCH_DIR = arch/l4

# Determine our sys- directory.
ifeq ($(CONFIG_ARCH_MIPS64),y)
	SYSTEM=mips
# Gcc prefines mips so it will get expanded to 1 if you
# are not careful
	KBUILD_CFLAGS += -Umips
	KBUILD_CFLAGS += -D_L4_WORD=64
# For L4 includes
	KBUILD_CFLAGS += -DMIPS64_BIG_ENDIAN
	KBUILD_CFLAGS += -D__linux__
	KBUILD_CFLAGS += -Umips
	KBUILD_CFLAGS += -D__linux__
	KBUILD_CFLAGS += -D_L4_WORD=64
  ifeq ($(CONFIG_U4600),y)
	MACHINE=u4600
	PLAT = u4600
  endif
endif

ifeq ($(CONFIG_ARCH_ARM),y)
	SYSTEM=arm
	SYSINC=arm
#FIXME: benjl I don't think we need these...
	KBUILD_CFLAGS += -fno-builtin
	KBUILD_CFLAGS += -D_L4_WORD=32 -O2
# For L4 includes
  ifeq ($(CONFIG_OMAP1510),y)
	PLAT = innovator
	KBUILD_CFLAGS += -DARM_LITTLE_ENDIAN -mlittle-endian  -march=armv4 -mtune=arm9tdmi -mshort-load-bytes -D__LINUX_ARM_ARCH__=4
	MACHINE=omap1510
  endif
  ifeq ($(CONFIG_SA1100),y)
	PLAT = pleb
	KBUILD_CFLAGS += -DARM_LITTLE_ENDIAN -mlittle-endian -D__LINUX_ARM_ARCH__=4 -march=armv4t -DARM_PID_RELOC
	KBUILD_AFLAGS += -DARM_PID_RELOC
	MACHINE=sa1100
  endif
  ifeq ($(CONFIG_PXA),y)
	PLAT = pxa
	KBUILD_CFLAGS += -DARM_LITTLE_ENDIAN -mlittle-endian -D__LINUX_ARM_ARCH__=5 -march=armv5te -DARM_PID_RELOC -DARM_SHARED_DOMAINS
	LDFLAGS += -EL
	KBUILD_AFLAGS += -mlittle-endian -DARM_PID_RELOC -DARM_SHARED_DOMAINS
	MACHINE=pxa
  endif
  ifeq ($(CONFIG_KZM),y)
	PLAT = kzm
	KBUILD_CFLAGS += -DARM_LITTLE_ENDIAN -mlittle-endian -D__LINUX_ARM_ARCH__=6 -march=armv6j
	MACHINE=kzm
  endif
  ifeq ($(CONFIG_VERSATILE),y)
	PLAT = versatile
	KBUILD_CFLAGS += -DARM_LITTLE_ENDIAN -mlittle-endian -D__LINUX_ARM_ARCH__=5 -march=armv5te -DARM_PID_RELOC -DARM_SHARED_DOMAINS
	KBUILD_AFLAGS += -DARM_PID_RELOC -DARM_SHARED_DOMAINS
	MACHINE = versatile
  endif
  ifeq ($(CONFIG_IXP4XX),y)
	PLAT = ixp4xx
	KBUILD_CFLAGS += -DARM_BIG_ENDIAN -mbig-endian -D__LINUX_ARM_ARCH__=5 -march=armv5te
	LDFLAGS += -EB
	KBUILD_AFLAGS += -mbig-endian
	MACHINE=ixp4xx
  endif
	MODFLAGS += -mlong-calls
include/asm-l4/.arch: $(wildcard include/config/arch/*.h) 
	@echo '  SYMLINK include/asm-l4/sys-arm/arch -> include/asm-l4/arm/$(INCDIR)'
ifneq ($(KBUILD_SRC),)
	$(Q)mkdir -p include/asm-l4/asm
	$(Q)ln -fsn $(srctree)/include/asm-l4/arm/$(INCDIR) include/asm-l4/asm/arch
else
	$(Q)mkdir -p include/asm-l4/asm
	$(Q)ln -fsn $(srctree)/include/asm-l4/arm/$(INCDIR) include/asm-l4/asm/arch
	#$(Q)ln -fsn $(INCDIR) include/asm-l4/asm/arch
endif
	@touch $@

archprepare: maketools

.PHONY: maketools FORCE
maketools: include/linux/version.h include/asm-l4/.arch FORCE
	#$(Q)$(MAKE) $(build)=arch/arm/tools include/asm-arm/mach-types.h
endif

ifeq ($(CONFIG_ARCH_I386),y)
# Gcc prefines i386 so it will get expanded to 1 if you
# are not careful
	KBUILD_CFLAGS += -Ui386
	SYSTEM=i386
	SYSINC=x86
	KBUILD_CFLAGS += -D_L4_WORD=32
# For L4 includes

  ifeq ($(CONFIG_PC99),y)
	PLAT = pc99
	MACHINE=pc99
  endif
endif

ifeq ($(CONFIG_BIG_ENDIAN),y)
KBUILD_CFLAGS += -D__L4EB__
else
KBUILD_CFLAGS += -D__L4EL__
endif

CPPFLAGS_vmlinux.lds +=  -U$(ELF_ARCH) -DELF_ARCH=$(ELF_ARCH)

# @todo FIXME: configure mutex type properly using build system - adamc
KBUILD_CFLAGS += -DCONFIG_HYBRID_MUTEXES

core-y			+= $(ARCH_DIR)/kernel/		 \
			   $(ARCH_DIR)/mm/  \
			   $(ARCH_DIR)/lib/  \
			   $(ARCH_DIR)/sys-$(SYSTEM)/ \

core-$(CONFIG_OKL4FS) += $(ARCH_DIR)/kernel/fs/okl4fs/
ifeq ($(CONFIG_OKL4FS),y)
KBUILD_CFLAGS += -D__ENABLE_OKL4FS
endif

drivers-y                  += $(ARCH_DIR)/drivers/

libs-y += $(WOMBAT_LDFLAGS)

#libs-$(CONFIG_ARCH_PXA) += -lpxa

include $(TOPDIR)/$(ARCH_DIR)/sys-$(SYSTEM)/Makefile

EXTRAVERSION := $(EXTRAVERSION)-$(SYSTEM)

# -Derrno=kernel_errno - This turns all kernel references to errno into
# kernel_errno to separate them from the libc errno.  This allows -fno-common
# in KBUILD_CFLAGS.  Otherwise, it would cause ld to complain about the two different
# errnos.

KBUILD_CFLAGS += -D__arch_l4__ -D__SYSTEM__=$(SYSTEM) -D__SYSINC__=$(SYSINC) -D__MACHINE__=mach-$(MACHINE) -D__MACHINE_INC__=arch-$(MACHINE)
KBUILD_AFLAGS += -D__arch_l4__ -D__SYSTEM__=$(SYSTEM) -D__SYSINC__=$(SYSINC) -D__MACHINE__=mach-$(MACHINE)

KBUILD_CFLAGS += -Iarch/l4/include -Iinclude/asm-l4
KBUILD_CFLAGS += -I$(CONFIG_SDK)/include
KBUILD_CFLAGS += -I../../../tools/magpie/include
KBUILD_CFLAGS += $(WOMBAT_CFLAGS)

KBUILD_AFLAGS += -Iarch/l4/include
KBUILD_AFLAGS += -I$(CONFIG_SDK)/include
KBUILD_AFLAGS += $(WOMBAT_CFLAGS)

include/linux/version.h: arch/$(ARCH)/Makefile

%archprepare: $(SYS_HEADERS) $(GEN_HEADERS) include/asm-$(ARCH)/param.h include/asm-$(ARCH)/constants.h sysprepare

LIBGCC=$(CC) -print-libgcc-file-name
LIBGCCDIR=$(shell $(LIBGCC) | xargs dirname)

LDFLAGS_vmlinux += -L$(LIBGCCDIR) -Ttext=$(LINK_ADDRESS)
LDFLAGS += -L$(CONFIG_SDK)/lib

arch/$(ARCH)/sys-$(SYSTEM)/asm-offsets.s: include/asm include/linux/version.h

include/asm-$(ARCH)/constants.h: arch/$(ARCH)/sys-$(SYSTEM)/asm-offsets.s
	$(call filechk,gen-asm-offsets)

include/asm-$(ARCH)/param.h: arch/$(ARCH)/sys-$(SYSTEM)/asm-offsets.s
	$(call filechk,gen-asm-offsets)

archclean: sysclean
	@echo "Arch clean"

CLEAN_FILES += include/asm-$(ARCH)/constants.h* include/asm-$(ARCH)/param.h*

export SYSTEM MACHINE
