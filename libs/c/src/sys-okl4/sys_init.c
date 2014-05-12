#include <compat/c.h>
#include <stdint.h>
#include <stdlib.h>
#include <threadstate.h>
#include <okl4/env.h>
#include <okl4/init.h>
#include <okl4/types.h>

extern void __thread_state_init(void *);

void __malloc_init(uintptr_t head_base,  uintptr_t heap_end);
void __sys_entry(void * env);
void __sys_thread_entry(void);
int main(int argc, char **argv);

#ifdef THREAD_SAFE
static struct thread_state __ts;
#endif

#if defined(OKL4_KERNEL_MICRO)
void *_okl4_forced_symbols = &okl4_forced_symbols;
#endif /* OKL4_KERNEL_MICRO */
void
__sys_entry(void *env)
{
    okl4_env_segment_t *heap;
    okl4_env_args_t *args;
    int argc = 0;
    char **argv = NULL;
    int result;
#if defined(OKL4_KERNEL_MICRO)
    /* Ensure forced symbols are linked into final binary.
     *
     * GCC is too smart for its own good, and compiles away any trivial
     * reference to 'forced_symbols'. Thus, we need to perform this next
     * bit of trickiness to confuse the compiler sufficiently to emit
     * the symbol.
     */
    if ((int)&_okl4_forced_symbols == 1) {
        for (;;);
    }
#endif /* OKL4_KERNEL_MICRO */
    /* Setup environment. */
    __okl4_environ = env;

    /* Get the heap address and size from the environment. */
    heap = okl4_env_get_segment("MAIN_HEAP");
    assert(heap != NULL);

    /* Get the command line arguments from the environment. */
    args = OKL4_ENV_GET_ARGS("MAIN_ARGS");
    if (args != NULL) {
        argc = args->argc;
        argv = &args->argv;
    }

    /* Initialise heap. */
    __malloc_init(heap->virt_addr, heap->virt_addr + heap->size);

#ifdef THREAD_SAFE
    __thread_state_init(&__ts);
#endif

    result = main(argc, argv);
    exit(result);
}

void
__sys_thread_entry(void)
{
    int result;

#ifdef THREAD_SAFE
    __thread_state_init(malloc(sizeof(struct thread_state)));
#endif
    result = main(0, NULL);
    exit(result);
}

/*
 * Libokl4 requires that a certain set of symbols in the library are
 * available at weave time. Unfortunately, the linker will discard
 * these symbols unless they are actively used by the binary we are
 * being linked against.
 *
 * This function will create a reference to those symbols, forcing
 * the linker to put them into the final binary, and hence making
 * them available to ElfWeaver.
 *
 * Once more, because we are ourself a library, these forced symbols must be
 * somewhere where they will always be linked against the user's binary. We
 * assume that the user will always call one of these init functions, pulling
 * these symbols in with it.
 */
#define FORCED_SYMBOL(x) \
    extern void *x; \
    static void **__forced_symbol__##x UNUSED = &x;
#if defined(OKL4_KERNEL_MICRO)
FORCED_SYMBOL(_okl4_utcb_memsec_lookup)
FORCED_SYMBOL(_okl4_utcb_memsec_map)
FORCED_SYMBOL(_okl4_utcb_memsec_destroy)
#endif /* OKL4_KERNEL_MICRO */

