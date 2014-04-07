/* Copyright 2014, Peter A. Bigot
 *
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 * * Redistributions of source code must retain the above copyright notice,
 *   this list of conditions and the following disclaimer.
 *
 * * Redistributions in binary form must reproduce the above copyright notice,
 *   this list of conditions and the following disclaimer in the documentation
 *   and/or other materials provided with the distribution.
 *
 * * Neither the name of the software nor the names of its contributors may be
 *   used to endorse or promote products derived from this software without
 *   specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 */

/** @file
 *
 * @brief Alternative implementations of _sbrk(2) for BSPACM
 *
 * This file contains implementations of _sbrk(2) that follow a
 * variety of policies.  The desired implementation is selected at
 * link time by defining the _sbrk symbol to refer to one of these.
 * If @c -ffunction-sections is used, exactly one implementation is
 * included.
 *
 * The code assumes the standard CMSIS startup structure and linker
 * script is used.  This organizes RAM from lower addresses to upper,
 * with initialized and uninitialized data at the bottom, the end
 * marked with the symbol @c end, followed by any allocated heap
 * ending at symbol @c __HeapLimit, followed by any allocated stack
 * sections.  End of RAM is indicated by symbol @c __StackTop, with @c
 * __StackLimit a symbol far enough below that to hold all allocated
 * stack sections.
 *
 * @homepage http://github.com/pabigot/bspacm
 * @copyright Copyright 2014, Peter A. Bigot.  Licensed under <a href="http://www.opensource.org/licenses/BSD-3-Clause">BSD-3-Clause</a>
 */

#include <bspacm/core.h>
#include <sys/types.h>
#include <errno.h>

/* We're providing system call implementation here, so ensure we have
 * visible prototypes that match what newlib is expecting. */
#define _COMPILING_NEWLIB
#include <sys/unistd.h>

/** This function is invoked whenever _sbrk() runs out of memory.  By
 * default it causes the application to hang, but since the definition
 * is weak the application may provide an alternative implementation
 * that is more diagnostic or that returns the responsibility of
 * handling out-of-memory to the application (see description of
 * return value).
 *
 * The API could be changed to support mapping new memory at @p brk,
 * but this seems unlikely to every be used.
 *
 * @param brk the current program break
 *
 * @param current total number of bytes allocated by previous
 * successful invocations of _sbrk() (i.e., allocated bytes preceding
 * @p brk)
 *
 * @param increment the number of bytes in the request _sbrk() cannot
 * satisfy
 *
 * @return This implementation does not return.  If superseded, an
 * implementation that does return must set @c errno to @c ENOMEM and
 * return <tt>(void*)-1</tt>. */
void *
__attribute__((__weak__))
_bspacm_sbrk_error (void * brk,
                    ptrdiff_t current,
                    ptrdiff_t increment)
{
  BSPACM_CORE_DISABLE_INTERRUPT();
  while (1) {
    /* spin */
  }
}

/* Implement allocation with a policy-dependent upper bound. */
static BSPACM_CORE_INLINE_FORCED
void *
common_sbrk (char * const upper_bound,
             ptrdiff_t increment)
{
  static char * brk;        /* the current program break */
  extern char end;          /* symbol at which heap starts */
  char * nbrk;
  void * rv;

  if (0 == brk) {
    brk = &end;
  }
  nbrk = increment + brk;
  if (upper_bound < nbrk) {
    return _bspacm_sbrk_error(brk, brk - &end, increment);
  }
  rv = brk;
  brk = nbrk;
  return rv;
}

/** An sbrk() implementation that rejects any attempt to allocate
 * memory dynamically.  The behavior is equivalent to
 * _bspack_sbrk_heap() with a zero-sized heap. */
void *
_bspacm_sbrk_fatal (ptrdiff_t increment)
{
  return _bspacm_sbrk_error(0, 0, increment);
}

/** An sbrk() implementation that depends on a fixed heap allocated
 * within the standard startup infrastructure.
 *
 * An error is indicated if the reserved heap size would be exceeded.
 * There is no check against the current stack pointer. */
void *
_bspacm_sbrk_heap (ptrdiff_t increment)
{
  extern char __HeapLimit;  /* symbol placed just past end of heap */
  return common_sbrk(&__HeapLimit, increment);
}

/** An sbrk() implementation that allows heap (growing up) to grow to
 * the bottom of a reserved stack region.
 *
 * An error is indicated if the new program break would encroach into
 * the reserved stack space.  There is no check against the current
 * stack pointer.
 *
 * @note This policy is preferred to _bspacm_sbrk_unlimitedstack()
 * when code may be executing in tasks where the stack frame is in
 * previously allocated memory. */
void *
_bspacm_sbrk_fixedstack (ptrdiff_t increment)
{
  extern char __StackLimit;   /* reserved end of stack */
  return common_sbrk(&__StackLimit, increment);
}

/** An sbrk() implementation that allows heap (growing up) and stack
 * (growing down) to share a region of memory.
 *
 * An error is indicated if the new break point would encroach into
 * the current stack space.
 *
 * @note Like _bspacm_sbrk_unlimited(), but eliminating the minimum
 * reserved stack.  Not sure why this would be worth doing, but for
 * completeness.... */
void *
_bspacm_sbrk_dynstack (ptrdiff_t increment)
{
  register char * sp __asm__("sp");
  return common_sbrk(sp, increment);
}

/** An sbrk() implementation that allows heap (growing up) and stack
 * (growing down) to share a region of memory, with a minimum size
 * reserved for the stack but allowing for the stack to grow below
 * that point.
 *
 * An error is indicated if the new break point would encroach into
 * the reserved stack space or the currently used stack space. */
void *
_bspacm_sbrk_unlimitedstack (ptrdiff_t increment)
{
  extern char __StackLimit;   /* reserved end of stack */
  register char * sp __asm__("sp");
  char * upper_bound = &__StackLimit;
  if (sp < upper_bound) {
    upper_bound = sp;
  }
  return common_sbrk(upper_bound, increment);
}
