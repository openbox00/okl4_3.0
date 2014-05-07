/* @LICENCE("National ICT Australia", "2004")@ */
/* @LICENCE("Open Kernel Labs, Inc.", "2007")@ */
/*
 * Description: This file implements a doubly-linked list. Each element has a
 * next and prev pointer, and a "data" pointer.
 * 
 * The 0-th element in the linked list (returned by ll_new()) is used to
 * represent the list as a whole. An empty list therfore has just this element
 * with next and prev pointing to itself.
 * 
 * Authors: Ben Leslie <benjl@cse.unsw.edu.au> 
 */

#include <stddef.h>
#include <assert.h>
#include <asm/ll.h>

struct ll *
ll_new(void)
{
    struct ll *ll;
    ll = malloc(sizeof(struct ll));
    if (ll != NULL) {
        ll->next = ll;
        ll->prev = ll;
        ll->data = NULL;
    }
    return ll;
}

/*
 * Insert before the current item 
 */
struct ll *
ll_insert_before(struct ll *ll, void *data)
{
    struct ll *new;
    new = malloc(sizeof(struct ll));
    if (new != NULL) {
        new->next = ll;
        ll->prev->next = new;
        new->prev = ll->prev;
        ll->prev = new;
        new->data = data;
    }
    return new;
}

struct ll *
ll_delete(struct ll *ll)
{
    struct ll *next = ll->next;

    ll->next->prev = ll->prev;
    ll->prev->next = ll->next;
    free(ll);
    return next;
}

void
dl_list_init(struct double_list *dl)
{
    dl->head = (void *)dl;
    dl->tail = (void *)dl;
}

void *
dl_list_create_back(struct double_list *dl, unsigned long payload)
{
    struct ll *new;
    new = malloc(sizeof(struct ll) + payload);
    if (new != NULL) {
        new->next = (void *)dl;
        dl->tail->next = new;
        new->prev = dl->tail;
        dl->tail = new;
        return &new->data;
    }
    return NULL;
}
