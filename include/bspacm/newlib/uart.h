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
 * @brief File-descriptor capabilities associated with UART/CONSOLE peripherals
 *
 * @homepage http://github.com/pabigot/bspacm
 * @copyright Copyright 2014, Peter A. Bigot.  Licensed under <a href="http://www.opensource.org/licenses/BSD-3-Clause">BSD-3-Clause</a>
 */

#ifndef BSPACM_NEWLIB_UART_H
#define BSPACM_NEWLIB_UART_H

#include <bspacm/core.h>
#include <bspacm/newlib/fdops.h>
#include <bspacm/periph/uart.h>

/** Utility function to bind a UART peripheral to file descriptor state.
 *
 * This function allocates a file descriptor state object, assigns @p
 * usp into its @link sBSPACMnewlibFDOPSfile::dev dev@endlink field,
 * and assigning the standard UART operations table.
 *
 * @note If @p usp refers to the default console device, and that
 * device is already open, the call will fail with @c errno set to
 * EBUSY.  Similarly, successfully binding the default console device
 * through this API will prevent fBSPACMnewlibFDOPSdriverCONSOLE()
 * from succeeding.
 *
 * @param usp a pointer to a UART peripheral.  The peripheral is @b
 * not configured by this function.  (The peripheral is deconfigured
 * by the implementation of close() in the operations table assigned
 * by this function.
 *
 * @return a newly allocated file descriptor state object, or null if
 * something went wrong. */
hBSPACMnewlibFDOPSfile hBSPACMnewlibFDOPSdriverUARTbind (hBSPACMperiphUART usp);

/** A driver function to support @c /dev/console as a 115200 8N1 UART.
 *
 * This should be placed into #xBSPACMnewlibFDOPSdriver if the BSPACM
 * board default UART is to be recognized as the console device for
 * stdin/stdout/stderr.
 *
 * Because there is only one console device, but it may need to be
 * referenced by multiple descriptors, the returned structure is
 * reference counted.  The associated UART is deconfigured and the
 * structure released only when no descriptors still reference it.
 *
 * If the console device was already bound to a non-console descriptor
 * through hBSPACMnewlibFDOPSdriverUARTbind(), the call will fail with
 * @c errno set to @c EBUSY.
 *
 * @param pathname the path that was passed to open(). This function
 * returns a null pointer if @pathname is not equal to
 * <tt>"/dev/console"</tt>.
 *
 * @param flags ignored
 *
 * @return pointer to the singleton instance of the console descriptor
 * state. */
sBSPACMnewlibFDOPSfile *
fBSPACMnewlibFDOPSdriverCONSOLE (const char * pathname,
                                 int flags);

#endif /* BSPACM_NEWLIB_UART_H */
