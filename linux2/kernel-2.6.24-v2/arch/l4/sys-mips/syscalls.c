#include <l4.h>

#include <linux/kernel.h>

#include <asm/syscalls.h>

extern l4_mips_abi_t l4_mips_abi_64;
extern l4_mips_abi_t l4_mips_abi_n32;
extern l4_mips_abi_t l4_mips_abi_o32;

l4_mips_abi_t *l4_mips_abi_list[4] = {
	&l4_mips_abi_64,
	&l4_mips_abi_n32,
	&l4_mips_abi_o32,
	0,
};
