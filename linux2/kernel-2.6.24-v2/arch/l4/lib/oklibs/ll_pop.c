/* @LICENCE("National ICT Australia", "2005")@ */
/* @LICENCE("Open Kernel Labs, Inc.", "2007")@ */
/*
 * Author: Ben Leslie 
 */

#include <asm/ll.h>

void *
ll_pop(struct ll *ll)
{
    struct ll *first = ll->next;
    void *value = first->data;

    if (first != ll) {
        ll_delete(first);
    }
    return value;
}
