
#include <linux/init.h>
#include <linux/console.h>

#include <l4/kdebug.h>

static void l4kdb_cons_write(struct console *co, const char *p, unsigned count)
{
	while (count-- > 0) {
		L4_KDB_PrintChar(*p++);
	}
}

static struct console l4kdb_cons = {
	.name	    = "l4con",
	.write	    = l4kdb_cons_write,
	.flags	    = CON_PRINTBUFFER,
	.index	    = -1,
};

static int __init l4kdb_console_init(void)
{
	register_console(&l4kdb_cons);
	return 0;
}

console_initcall(l4kdb_console_init);

#ifdef CONFIG_EARLY_PRINTK
void
enable_early_printk(void)
{
	register_console(&l4kdb_cons);
}

void
disable_early_printk(void)
{
	unregister_console(&l4kdb_cons);
}
#endif

