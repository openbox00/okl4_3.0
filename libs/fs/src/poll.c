/*
 * Copyright (c) 2008 Open Kernel Labs, Inc. (Copyright Holder).
 * All rights reserved.
 *
 * 1. Redistribution and use of OKL4 (Software) in source and binary
 * forms, with or without modification, are permitted provided that the
 * following conditions are met:
 *
 *     (a) Redistributions of source code must retain this clause 1
 *         (including paragraphs (a), (b) and (c)), clause 2 and clause 3
 *         (Licence Terms) and the above copyright notice.
 *
 *     (b) Redistributions in binary form must reproduce the above
 *         copyright notice and the Licence Terms in the documentation and/or
 *         other materials provided with the distribution.
 *
 *     (c) Redistributions in any form must be accompanied by information on
 *         how to obtain complete source code for:
 *        (i) the Software; and
 *        (ii) all accompanying software that uses (or is intended to
 *        use) the Software whether directly or indirectly.  Such source
 *        code must:
 *        (iii) either be included in the distribution or be available
 *        for no more than the cost of distribution plus a nominal fee;
 *        and
 *        (iv) be licensed by each relevant holder of copyright under
 *        either the Licence Terms (with an appropriate copyright notice)
 *        or the terms of a licence which is approved by the Open Source
 *        Initative.  For an executable file, "complete source code"
 *        means the source code for all modules it contains and includes
 *        associated build and other files reasonably required to produce
 *        the executable.
 *
 * 2. THIS SOFTWARE IS PROVIDED ``AS IS'' AND, TO THE EXTENT PERMITTED BY
 * LAW, ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO,
 * THE IMPLIED WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR
 * PURPOSE, OR NON-INFRINGEMENT, ARE DISCLAIMED.  WHERE ANY WARRANTY IS
 * IMPLIED AND IS PREVENTED BY LAW FROM BEING DISCLAIMED THEN TO THE
 * EXTENT PERMISSIBLE BY LAW: (A) THE WARRANTY IS READ DOWN IN FAVOUR OF
 * THE COPYRIGHT HOLDER (AND, IN THE CASE OF A PARTICIPANT, THAT
 * PARTICIPANT) AND (B) ANY LIMITATIONS PERMITTED BY LAW (INCLUDING AS TO
 * THE EXTENT OF THE WARRANTY AND THE REMEDIES AVAILABLE IN THE EVENT OF
 * BREACH) ARE DEEMED PART OF THIS LICENCE IN A FORM MOST FAVOURABLE TO
 * THE COPYRIGHT HOLDER (AND, IN THE CASE OF A PARTICIPANT, THAT
 * PARTICIPANT). IN THE LICENCE TERMS, "PARTICIPANT" INCLUDES EVERY
 * PERSON WHO HAS CONTRIBUTED TO THE SOFTWARE OR WHO HAS BEEN INVOLVED IN
 * THE DISTRIBUTION OR DISSEMINATION OF THE SOFTWARE.
 *
 * 3. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR ANY OTHER PARTICIPANT BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR
 * BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
 * WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE
 * OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN
 * IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#include <sys/poll.h>
#include <errno.h>
#include <unimplemented.h>

#include <fs/fs.h>

#include <threadstate.h>
#include <iguana/thread.h>
#include <l4/utcb.h>
#include <iguana/tls.h>

static int
pipe_poll(struct pollfd *fds, nfds_t nfds, int timeout)
{
    struct fdesc *f;
    struct blocked_thread *bt;

    if (nfds > 1) {
        UNIMPLEMENTED();
    }

    if (timeout > 0) {
        UNIMPLEMENTED();
    }

    if (fds[0].events != POLLIN) {
        UNIMPLEMENTED();
    }

    f = &(ftable[fds[0].fd]);

    if (timeout == 0) {
        if (f->U.p.rp->pipe_size == 0) {
            fds[0].revents = 0;
        }
        else {
            fds[0].revents = POLLIN;
        }

        return 1;
    }

    bt = malloc(sizeof(struct blocked_thread));
    okl4_libmutex_lock(f->U.p.rp->mutex);
    if (f->U.p.rp->pipe_size == 0) {
        bt->tid.raw = ((L4_Word_t *)__L4_TCR_ThreadLocalStorage())[TLS_THREAD_ID];
        STAILQ_INSERT_HEAD(&(f->U.p.rp->readq), bt, links);
        okl4_libmutex_unlock(f->U.p.rp->mutex);
        block_myself();
    }
    free(bt);

    fds[0].revents = POLLIN;

    return 1;
}

int
okl4_poll(struct pollfd *fds, nfds_t nfds, int timeout)
{
    int fildes = fds[0].fd;

    if (fildes < 0 || fildes >= MAX_FILES) {
        errno = EINVAL;
        return -1;
    }

    switch (ftable[fildes].ftype) {
        case REGULAR:
            UNIMPLEMENTED();
            break;
        case SAS_PIPE:
            return pipe_poll(fds, nfds, timeout);
            break;
        case SERIAL:
            UNIMPLEMENTED();
            break;
        default:
            UNIMPLEMENTED();
            break;
    }

    return 0;
}
