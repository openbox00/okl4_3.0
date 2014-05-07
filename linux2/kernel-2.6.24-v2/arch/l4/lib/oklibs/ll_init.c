/* @LICENCE("National ICT Australia", "2005")@ */
/* @LICENCE("Open Kernel Labs, Inc.", "2007")@ */

/*
 * Author: Ben Leslie 
 */

#include <asm/ll.h>

void
ll_init(struct ll *lp)
{
    lp->next = lp->prev = lp;
    lp->data = NULL;
}
