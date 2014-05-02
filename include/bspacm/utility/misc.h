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

#ifndef BSPACM_UTILITY_MISC_H
#define BSPACM_UTILITY_MISC_H

#include <bspacm/core.h>

/** Display the contents of a block of memory expressed as octets
 *
 * This function displays on the console the contents of a memory
 * region as octets and printable characters.  The memory must by
 * byte-addressable.  One line is emitted for each 16-byte block: it
 * begins with the address of the displayed data (per @p base),
 * followed by up to 16 octet values, followed by the values as
 * printable characters.  The emitted addresses are always 16-byte
 * aligned; if @p base is not aligned, the first line will be padded
 * out to the location of the first data value.
 *
 * @param dp pointer to start of memory region
 * @param count number of octets to display
 * @param base base displayed address for first octet
 */
void vBSPACMconsoleDisplayMemoryOctets (const uint8_t * dp,
                                        size_t count,
                                        uintptr_t base);

/** Display the contents of a block of memory expressed as hex words
 *
 * This function displays on the console the contents of a memory
 * region as 32-bit words in hexadecimal.  One line is emitted for
 * each 4-word block: it begins with the address of the displayed data
 * (per @p base), followed by up to 4 word values.  The emitted
 * addresses are always 16-byte aligned; if @p base is not aligned,
 * the first line will be padded out to the location of the first data
 * value.
 *
 * @param dp pointer to start of memory region
 * @param count number of words to display
 * @param base base displayed address for first octet
 */
void vBSPACMconsoleDisplayMemoryWords (const uint32_t * dp,
                                       size_t count,
                                       uintptr_t base);

/** Display a short memory region contents of a block of memory.
 *
 * This function displays a sequence of octets on the console.  It
 * differs from vBSPACMconsoleDisplayMemory() in that there is no
 * address information, no printable character display, and no attempt
 * to split the output into individual lines.
 *
 * @param dp pointer to start of memory region
 * @param count number of octetst to display
 */
void vBSPACMconsoleDisplayOctets (const uint8_t * dp,
                                  size_t count);

#endif /* BSPACM_UTILITY_MISC_H */
