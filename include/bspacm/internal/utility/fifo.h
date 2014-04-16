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
 * @brief Header for a first-in-first-out supporting datastructure
 *
 * @warning This header is intended to be included in implementation
 * files only.  The types, macros, and functions described are not
 * namespace-clean.
 *
 * @warning All @c fifo_* operations are assumed to operate in mutex
 * context: i.e. no other system (thread, interrupt handler, etc) may
 * be interacting with the FIFO.  See #sFIFO for details.
 *
 * @homepage http://github.com/pabigot/bspacm
 * @copyright Copyright 2014, Peter A. Bigot.  Licensed under <a href="http://www.opensource.org/licenses/BSD-3-Clause">BSD-3-Clause</a>
 */

#ifndef BSPACM_INTERNAL_UTILITY_FIFO_H
#define BSPACM_INTERNAL_UTILITY_FIFO_H

#include <bspacm/core.h>
#include <stddef.h>

/** Data structure for a first-in-first-out circular buffer of
 * arbitrary size, holding octet (@c uint8_t) values.
 *
 * This struct uses the flexible array capability standardized in C99,
 * along with the FIFO_DEFINE_ALLOCATION() and FIFO_FROM_ALLOCATION()
 * macros, to avoid dynamic memory allocation while still allowing
 * variation in size between instances of the data type.
 *
 * Invariants and policies:
 *
 * @li @p head is equal to @p tail only when the FIFO is empty.
 * @li @p head+1 is equal to @p tail modulo @p size when the FIFO is
 * full.
 * @li Values are added at the head, which then moves to the next cell.
 * @li Values are removed from the tail, which then moves to the next
 * cell.
 * @li Newly-received values are retained in preference to older
 * values when the buffer overflows.
 *
 * This data structure is intended to be shared between interrupt
 * handlers and framework code.  It probably should not be accessed
 * directly by user code.  The following operations are provided to
 * help maintain the invariants above:
 *
 * @li #fifo_reset
 * @li #fifo_length
 * @li #fifo_empty
 * @li #fifo_full
 * @li #fifo_pop_tail
 * @li #fifo_push_head
 * @li #fifo_pop_into_buffer
 *
 * Although these inline functions perform common operations, for full
 * efficiency it is sometimes necessary to manipulate the @p head and
 * @p tail fields directly.  The fields that are expected to be read
 * and written by both interrupt handlers and framework code are
 * marked volatile to ensure they are not cached inappropriately.
 * Nonetheless, all changes to the volatile fields of this structure
 * must be done in a context that inhibits external modifications.  As
 * an optimization, framework code executed with interrupts off may
 * use local cached variables holding @p head or @p tail while
 * performing operations that involve multiple cells of the buffer, so
 * long as the final values are written back to the structure before
 * interrupts are re-enabled.  Be sure to satisfy the data structure
 * invariants when writing such code.
 *
 */
typedef struct sFIFO {
  /** The @p head of the FIFO is the cell into which the next value to
   * be stored will be written (things enter the head, and leave the
   * tail). */
  volatile uint16_t head;

  /** The @p tail of the FIFO is the cell holding the next value to be
   * read. */
  volatile uint16_t tail;

  /** The @p size of the FIFO is the number of cells that can store
   * values.  The @a length of the FIFO is the number of cells that
   * currently store values.  The relationship between length and size
   * is 0 <= length < size.  A fifo is empty with length is zero (head
   * == tail); it is full when length == size-1 (the next cell after
   * head is tail). */
  const uint16_t size;

  /** The @p buffer of the FIFO holds the values currently in the
   * FIFO. */
  volatile uint8_t cell[];
} sFIFO;

/** Define a #sFIFO instance that supports at least @p size_ value
 * cells.
 *
 * @warning The definition has @c static storage class.  A reference
 * to the fifo instance must be shared with the infrastructure that
 * uses it.
 *
 * @see FIFO_FROM_ALLOCATION()
 *
 * @param allocation_ the name of the allocation instance
 *
 * @param size_ the desired number of cells.  The actual number of
 * cells may be slightly larger due to alignment padding. */
#define FIFO_DEFINE_ALLOCATION(allocation_, size_)                      \
  static union {                                                        \
    sFIFO fifo;                                                         \
    uint8_t allocation[(size_) + sizeof(sFIFO)];                        \
  } allocation_ = { { 0, 0, sizeof(allocation_) - offsetof(sFIFO, cell) } }

/** Obtain a pointer to the #sFIFO instance stored within @p
 * allocation_, which must have been defined using
 * FIFO_DEFINE_ALLOCATION().
 *
 * @param allocation_ the name of the allocation instance
 *
 * @return a pointer to the #sFIFO instance within the allocation */
#define FIFO_FROM_ALLOCATION(allocation_) (&(allocation_).fifo)

/** Perform the adjustments necessary to convert a head/tail offset
 * back into the domain of the FIFO cell buffer.
 *
 * @param fp_ a pointer to an #sFIFO instance
 *
 * @param v_ a @c uint16_t value that may have been incremented past
 * the size of @p fp_'s cell buffer
 *
 * @return the offset of the cell that is identified by @p v_ modulo
 * the size of @p fp_'s cell buffer
 *
 * (Oddly, making this an inline function increases code size; hence it is a macro.) */
#define FIFO_ADJUST_OFFSET(fp_,v_) ((v_) % (fp_)->size)

/** Reset the FIFO back to an empty state. */
static BSPACM_CORE_INLINE
void
fifo_reset (sFIFO * fp)
{
  fp->head = 0;
  fp->tail = 0;
}

/** Return the number of cells actively used by the FIFO.
 *
 * This ranges from zero to <tt>fp->size - 1</tt>. */
static BSPACM_CORE_INLINE
uint16_t
fifo_length (const sFIFO * fp)
{
  uint16_t h = fp->head;
  uint16_t t = fp->tail;
  return (h - t) + ((h < t) ? fp->size : 0U);
}

/** Return a true value iff @p fp has no buffered values. */
static BSPACM_CORE_INLINE
int
fifo_empty (const sFIFO * fp)
{
  return fp->head == fp->tail;
}

/** Return a true value iff @p fp has no room for additional buffered
 * values. */
static BSPACM_CORE_INLINE
int
fifo_full (const sFIFO * fp)
{
  return (fp->size - 1) == fifo_length(fp);
}

/** Return the oldest value within the FIFO, or a negative error code.
 *
 * @param fp pointer to the FIFO structure
 *
 * @param force_if_empty If this is a true value, the tail of the FIFO
 * will be adjusted to consume its last value, whether or not there
 * appears to be such a value.  This allows a new value to be stored
 * into the array prior to checking whether there is space for it, and
 * causes the oldest unread value to be forgotten.  When false, the
 * tail will not be incremented if it appears there no values.
 *
 * @return a non-negative value from the buffer, or a negative value
 * indicating that the FIFO was empty. */
static BSPACM_CORE_INLINE
int
fifo_pop_tail (sFIFO *fp,
               int force_if_empty)
{
  uint16_t t = fp->tail;
  int was_empty = (fp->head == t);
  int rv = was_empty ? -1 : fp->cell[t];
  if (force_if_empty || (! was_empty)) {
    fp->tail = FIFO_ADJUST_OFFSET(fp, 1U + t);
  }
  return rv;
}

/** Push @p v onto FIFO @p fp.
 *
 * The return value indicates the previous state of the FIFO, allowing
 * interrupt configuration to follow the empty/non-empty/full state of
 * the FIFO.  In all cases @p v will be saved; in some cases an
 * earlier value in the FIFO will be lost.
 *
 * @param fp the FIFO pointer
 *
 * @param v the value to be added
 *
 * @return A negative value if the fifo was full (this will save the
 * new value, but discard the oldest unread value); zero if the value
 * was saved to a fifo that already had values in it; a positive value
 * if the value was saved to a previously-empty FIFO. */
static BSPACM_CORE_INLINE
int
fifo_push_head (sFIFO * fp, uint8_t v)
{
  uint16_t h = fp->head;
  uint16_t t = fp->tail;
  int rv = (h != t);

  fp->cell[h] = v;
  h = FIFO_ADJUST_OFFSET(fp, 1U + h);
  fp->head = h;
  if (h == t) {
    (void)fifo_pop_tail(fp, 1);
    rv = -1;
  }
  return rv;
}

/** Copy multiple elements from the FIFO into a buffer.
 *
 * @param fp pointer the FIFO pointer
 *
 * @param bps a pointer to the first location into which FIFO contents
 * should be stored
 *
 * @param bpe a pointer to the end location, immediately following the
 * last location where a FIFO value may be stored
 *
 * @return the number of elements stored starting at @p bps.  The
 * smaller of <tt>bpe-bps</tt> and the length of the FIFO will be
 * copied out. */
static BSPACM_CORE_INLINE
int
fifo_pop_into_buffer (sFIFO * fp,
                      uint8_t * bps,
                      uint8_t * bpe)
{
  uint16_t h = fp->head;
  uint16_t t = fp->tail;
  uint8_t * bp = bps;
  while ((bp < bpe) && (h != t)) {
    *bp++ = fp->cell[t];
    t = FIFO_ADJUST_OFFSET(fp, 1U + t);
  }
  fp->tail = t;
  return bp - bps;
}

#endif /* BSPACM_INTERNAL_UTILITY_FIFO_H */
