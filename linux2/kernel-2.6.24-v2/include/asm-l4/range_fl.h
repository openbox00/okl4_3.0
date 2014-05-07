/* @LICENCE("National ICT Australia", "2004")@ */
/* @LICENCE("Open Kernel Labs, Inc.", "2007")@ */
/*
 * Description: Header file for range free list. Describe function definitions
 * and defines error codes.
 * 
 * Authors: Ben Leslie <benjl@cse.unsw.edu.au
 * 
 */

#ifndef _RANGE_FL_H
#define _RANGE_FL_H

#include "ll.h"
#include <stdint.h>

struct range {
    uintptr_t from;
    uintptr_t to;
};

typedef struct ll *rfl_t;

/*
 * Create a new range free list 
 */
rfl_t rfl_new(void);

/*
 * Destroy the range free list 
 */
void rfl_destroy(rfl_t);

/*
 * Insert a range of integers 
 */
int rfl_insert_range(rfl_t rfl, uintptr_t from, uintptr_t to);
int rfl_free(rfl_t rfl, uintptr_t val);
uintptr_t rfl_alloc(rfl_t rfl);
uintptr_t rfl_alloc_range(rfl_t rfl, unsigned long size);
uintptr_t rfl_alloc_first(rfl_t rfl, unsigned long *size);

uintptr_t rfl_find_free_range(rfl_t rfl, unsigned long size);
uintptr_t rfl_alloc_specific_range(rfl_t rfl, uintptr_t from, unsigned long size);

#define RFL_SUCCESS 0
#define E_RFL_INV_RANGE -1
#define E_RFL_OVERLAP -2
#define E_RFL_NOMEM -3

#if 0
void rfl_debug(rfl_t rfl, FILE *out);
#endif

#endif /* _RANGE_FL_H */
