ifneq ($(KBUILD_SRC),)
KBUILD_CFLAGS	+= -I$(KBUILD_SRC)/arch/l4/sys-arm/mach-kzm/sdma/iapi/include \
		-I$(KBUILD_SRC)/include/linux -DMCU -DOS=LINUX \
		-DL_I_T_T_L_E_ENDIAN=0 -DB_I_G_ENDIAN=1 \
		-DENDIANNESS=L_I_T_T_L_E_ENDIAN
else
KBUILD_CFLAGS	+= -Iarch/l4/sys-arm/mach-kzm/sdma/iapi/include \
		-Iinclude/linux -DMCU -DOS=LINUX -DL_I_T_T_L_E_ENDIAN=0 \
		-DB_I_G_ENDIAN=1 -DENDIANNESS=L_I_T_T_L_E_ENDIAN
endif

obj-$(CONFIG_MXC_SDMA_API)		+= sdma.o sdma_malloc.o dma_sdma.o
obj-$(CONFIG_MXC_SDMA_API)		+= iapi/
