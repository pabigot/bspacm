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
 * @brief Miscellaneous utility functions
 *
 * @homepage http://github.com/pabigot/bspacm
 * @copyright Copyright 2014, Peter A. Bigot.  Licensed under <a href="http://www.opensource.org/licenses/BSD-3-Clause">BSD-3-Clause</a>
 */

#include <bspacm/utility/misc.h>
#include <inttypes.h>
#include <stdio.h>
#include <ctype.h>

void
vBSPACMconsoleDisplayOctets (const uint8_t * dp,
                             size_t count)
{
  const uint8_t * const edp = dp + count;
  while (dp < edp) {
    printf("%02x", *dp++);
    if (dp < edp) {
      putchar(' ');
    }
  }
}

static void BSPACM_CORE_INLINE
emit_address (uintptr_t addr)
{
  printf("%08" PRIxPTR " ", addr);
}


static unsigned int
align_base (uintptr_t base)
{
  unsigned int offset = base & 0x0F;
  if (0 != offset) {
    emit_address(base - offset);
    return offset;
  }
  return 0;
}

void
vBSPACMconsoleDisplayMemoryOctets (const uint8_t * dp,
                                   size_t count,
                                   uintptr_t base)
{
  static const char data_text_spacer[] = "  ";
  const uint8_t * const edp = dp + count;
  const uint8_t * adp = dp;
  bool with_nl = false;
  unsigned int skips = align_base(base);

  /* If the base is not 16-byte aligned, backfill for the material
   * that belongs in the first partial line. */
  if (skips) {
    unsigned int n;
    for (n = 0; n < skips; ++n) {
      unsigned int nsp = 3 + (8 == n);
      while (0 < nsp--) {
        putchar(' ');
      }
    }
    with_nl = true;
  }
  while (dp < edp) {
    if (0 == (base & 0x0F)) {
      if (adp < dp) {
        printf(data_text_spacer);
        while (skips) {
          putchar(' ');
          --skips;
        }
        while (adp < dp) {
          putchar(isprint(*adp) ? *adp : '.');
          ++adp;
        }
      }
      adp = dp;
      if (with_nl) {
        putchar('\n');
      } else {
        with_nl = true;
      }
      emit_address(base);
    } else if (0 == (base & 0x07)) {
      putchar(' ');
    }
    printf(" %02x", *dp++);
    ++base;
  }
  if (adp < dp) {
    while (base & 0x0F) {
      if (0 == (base & 0x07)) {
        putchar(' ');
      }
      printf("   ");
      ++base;
    }
    printf("  ");
    while (adp < dp) {
      putchar(isprint(*adp) ? *adp : '.');
      ++adp;
    }
  }
  putchar('\n');
}

void
vBSPACMconsoleDisplayMemoryWords (const uint32_t * dp,
                                  size_t count,
                                  uintptr_t base)
{
  const uint32_t * const edp = dp + count;
  bool with_nl = false;
  unsigned int skips;

  /* Force word alignment.  Otherwise things get ugly with the skip
   * logic. */
  base &= ~(uintptr_t)3;
  skips = align_base(base);
  if (skips) {
    unsigned int n;
    skips /= 4;
    for (n = 0; n < skips; ++n) {
      unsigned int nsp = 9;
      while (0 < nsp--) {
        putchar(' ');
      }
    }
    with_nl = true;
  }
  while (dp < edp) {
    if (0 == (base & 0x0F)) {
      if (with_nl) {
        putchar('\n');
      } else {
        with_nl = true;
      }
      emit_address(base);
    }
    printf(" %08" PRIx32, *dp++);
    base += sizeof(*dp);
  }
  putchar('\n');
}
