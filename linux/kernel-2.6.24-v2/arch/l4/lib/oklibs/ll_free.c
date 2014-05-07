/* @LICENCE("National ICT Australia", "2005")@ */
/* @LICENCE("Open Kernel Labs, Inc.", "2007")@ */
/*
 * Author: Ben Leslie 
 */

#include <asm/ll.h>

void
ll_free(struct ll *ll)
{
    struct ll *tmp = ll->next;

    while (tmp != ll) {
        tmp = ll_delete(tmp);
    }
    free(ll);
}
