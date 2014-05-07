/* @LICENCE("National ICT Australia", "2005")@ */
/* @LICENCE("Open Kernel Labs, Inc.", "2007")@ */
/*
 * Author: Ben Leslie 
 */

#include <asm/ll.h>

struct ll *
ll_add_front(struct ll *ll, void *data)
{
    return ll_insert_after(ll, data);
}
