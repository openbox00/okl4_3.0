/* @LICENCE("National ICT Australia", "2004")@ */
/* @LICENCE("Open Kernel Labs, Inc.", "2007")@ */
/*
 * Description: Linked list header file. Defines the functions and the struct
 * ll.
 * 
 * Authors: Ben Leslie <benjl@cse.unsw.edu.au> 
 */

#ifndef _LL_H_
#define _LL_H_

#include <stddef.h>

struct ll {
    struct ll *next;            /* Head */
    struct ll *prev;            /* Tail */
    void *data;
};

struct double_list {
    struct ll *head;
    struct ll *tail;
};

extern void *malloc(size_t);
extern void free(void *);

void *dl_list_create_front(struct double_list *list, unsigned long payload);
void *dl_list_create_back(struct double_list *dl, unsigned long payload);
void dl_init(struct double_list *list);

/*
 * FIXME: Implement these 
 */
void dl_list_delete(struct ll *node);
void dl_list_clear(struct double_list *list);

void ll_init(struct ll *lp);
void dl_list_init(struct double_list *dl);

struct ll *ll_new(void);
void ll_free(struct ll *ll);

struct ll *ll_insert_after(struct ll *ll, void *data);
struct ll *ll_insert_before(struct ll *ll, void *data);

struct ll *ll_add_front(struct ll *ll, void *data);
struct ll *ll_add_end(struct ll *ll, void *data);

struct ll *ll_delete(struct ll *ll);
void *ll_pop(struct ll *ll);

#endif /* _LL_H_ */
