#include <l4.h>

#include <linux/kernel.h>
#include <asm/io.h>
#include <asm/bug.h>

static unsigned long mips_io_encode(unsigned long port);
static void mips_barrier(void);

struct io_space io_space[1] = {
    { -1ul, mips_io_encode, mips_barrier },
};

void l4_init_io_space(unsigned int num)
{
	BUG();
}

static unsigned long mips_io_encode(unsigned long port)
{
	printk("%s called\n", __func__);
	return port;
}

static void mips_barrier(void)
{
	printk("%s called\n", __func__);
}


