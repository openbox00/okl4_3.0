#include "assert.h"
#include "l4.h"




#include <linux/kernel.h>

void
__assert(const char *msg, const char *file, const char* function, int line)
{
        char abuf[256];

        printk( "\n\nAssertion failed: %s, function %s, "
            "file %s, line %d.\n", msg, function, file, line);

        snprintf(abuf, sizeof(abuf),
                 "Assertion failed: %s, function %s, "
                 "file %s, line %d.\n", msg, function, file, line);

#ifdef __i386__
        /* Can only pass a static string on x86 */
        L4_KDB_Enter("Assertion failed");
#else
        L4_KDB_Enter(abuf);
#endif
}
