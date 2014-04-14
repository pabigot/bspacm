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
 * @brief ioctl() support for BSPACM's newlib file descriptor operations
 *
 * This file defines ioctl request values common to multiple BSPACM
 * devices.  More specific ioctl requests may be defined in other
 * headers.
 *
 * Applications should make no attempt to interpret ioctl values; they
 * are to be treated as opaque integers that are constant within a
 * given application.
 *
 * @homepage http://github.com/pabigot/bspacm
 * @copyright Copyright 2014, Peter A. Bigot.  Licensed under <a href="http://www.opensource.org/licenses/BSD-3-Clause">BSD-3-Clause</a>
 */

#ifndef BSPACM_NEWLIB_IOCTL_H
#define BSPACM_NEWLIB_IOCTL_H

#include <bspacm/newlib/fdops.h>

/** Flush read and write buffers at the device layer.
 *
 * This blocks, with interrupts enabled, as long as the underlying
 * device still has data pending for transmission or reception;
 * e.g. see iBSPACMperiphUARTfifoState() for an example device-layer
 * utility used by this ioctl.
 *
 * A common use for this would be to flush pending information written
 * through @c stdout prior to entering a sleep mode that might disable
 * the peripheral that supports the device.  E.g.:
 *
 * @code
 *   // Flush past any libc buffering
 *   fflush(stdout);
 *   // Flush past any device buffering
 *   (void)ioctl(1, BSPACM_IOCTL_FLUSH, O_WRONLY);
 *   // Now go to sleep
 *   BSPACM_CORE_DEEP_SLEEP();
 * @endcode
 *
 * @param flags One of the values @c O_RDONLY (0), @c O_WRONLY (1), or
 * @c O_RDWR (2) as defined in <fcntl.h>.  @c O_RDONLY and @c O_RDWR
 * will block until all material received by the device has been
 * consumed by the application.  @c O_WRONLY and @c O_RDWR will block
 * until all material submitted to the device has been written.
 *
 * @return 0 if all data could be flushed, or a negative error code. */
#define BSPACM_IOCTL_FLUSH 0x100

/* Forward declaration */
struct sBSPACMnewlibIOCTLfile;

/** Manipulate the underlying parameters of the device.
 *
 * The semantics of this are specific to @p request and to the device
 * that @p fd references.
 *
 * @param fd descriptor for an open device
 * @param request operation-specific request identifier
 * @param ... any parameters associated with @p request
 *
 * @return Usually zero on success, with errors indicated by returning
 * -1 and setting @p errno. */
int ioctl (int fd, int request, ...);

#endif /* BSPACM_NEWLIB_IOCTL_H */
