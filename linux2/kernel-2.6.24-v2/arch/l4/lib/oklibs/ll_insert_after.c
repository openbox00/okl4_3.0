/* @LICENCE("National ICT Australia", "2005")@ */
/* @LICENCE("Open Kernel Labs, Inc.", "2007")@ */

#include <asm/ll.h>

/*
 * Insert a new item after the current one 
 */
struct ll *
ll_insert_after(struct ll *ll, void *data)
{
    struct ll *new;
    new = malloc(sizeof(struct ll));
    if (new != NULL) {
        new->prev = ll;
        ll->next->prev = new;
        new->next = ll->next;
        ll->next = new;
        new->data = data;
    }
    return new;
}
