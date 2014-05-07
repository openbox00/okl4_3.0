/*
  @LICENSE@
*/
/*
  Author: Ben Leslie
  Created: Wed Oct  6 2004 
*/

#include <linux/kernel.h>

#include <assert.h>

/*
 * Called when Linux startup fails / exit server
 * XXX: Send status to iguana/loader?
 */
void
_Exit(int status)
{
#ifndef NDEBUG
	printk("Linux exit status: %d\n", status);
	L4_KDB_Enter("Linux exit");
#endif
	assert(!"Can't exit -- shouldn't reach here");
	L4_WaitForever();
	while(1);
}

void __div0(void)
{
	printk("Division by ZERO in Linux!\nExiting...\n");
	_Exit(10);
}

