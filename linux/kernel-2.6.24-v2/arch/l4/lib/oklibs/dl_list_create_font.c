/* @LICENCE("National ICT Australia", "2005")@ */
/* @LICENCE("Open Kernel Labs, Inc.", "2007")@ */

#include <asm/ll.h>

void *
dl_list_create_front(struct double_list *dl, unsigned long payload)
{
    struct ll *new;
    new = malloc(sizeof(struct ll) + payload);
    if (new != NULL) {
        new->prev = (void *)dl;
        dl->head->prev = new;
        new->next = dl->head;
        dl->head = new;
        return &new->data;
    }
    return NULL;
}
