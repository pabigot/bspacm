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
 * @brief File-descriptor types and declarations to support newlib
 *
 *
 * @homepage http://github.com/pabigot/bspacm
 * @copyright Copyright 2014, Peter A. Bigot.  Licensed under <a href="http://www.opensource.org/licenses/BSD-3-Clause">BSD-3-Clause</a>
 */

#ifndef BSPACM_NEWLIB_FDOPS_H
#define BSPACM_NEWLIB_FDOPS_H

#include <bspacm/core.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <stdarg.h>

/* Forward declaration */
struct sBSPACMnewlibFDOPSfile;

/** Handle for the generic object that implements the
 * file-descriptor--based operations of newlib. */
typedef struct sBSPACMnewlibFDOPSfile * hBSPACMnewlibFDOPSfile;

/** Signature of a the open function for a driver that supports the
 * file-descriptor options of newlib.
 *
 * An implementation of this function is provided for each driver
 * supported by an application/environment, by linking an
 * #xBSPACMnewlibFDOPSdriver table that holds drivers in order of
 * trial.
 *
 * The newlib open() function invokes each driver in turn to see if it
 * can create a handle for a file object, given the @p pathname and @p
 * flags.  The open succeeds only if such a file can be created.
 *
 * @note This function probably dynamically allocates the file object.
 *
 * @param pathname the path to the device.  For example, @c
 * "/dev/uart2" might be used to identify the third UART device
 * available on the platform.
 *
 * @param flags the standard flags from <fcntl.h> that describe how
 * the device should be opened.
 *
 * @return a pointer to a file object that can be used to interact
 * with the device, or a null pointer if the driver is unable to open
 * @p pathname with @p flags.
 */
typedef hBSPACMnewlibFDOPSfile (* fBSPACMnewlibFDOPSdriver) (const char * pathname,
                                                             int flags);

/** A list of drivers supported by the infrastructure.  The list of
 * drivers must end with a null pointer.
 *
 * The @c open() call will iterate through these in order to find a
 * driver capable of creating a file descriptor based on the
 * parameters to the open call.
 *
 * @weakdef The console capability provides a weak definition for this
 * that supports only fBSPACMnewlibFDOPSdriverConsole().  Applications
 * that intend to combine console support with other drivers must
 * provide an alternative implementation to override this.
 * Applications that need file descriptor operations but do not use
 * the console capability must provide an implementation. */
extern const fBSPACMnewlibFDOPSdriver xBSPACMnewlibFDOPSdriver[];

/** Record for the implementations of file descriptor operations that
 * are supported for a particular class of device.
 *
 * Each operation has the standard POSIX environment interface and
 * semantics, except that a #hBSPACMnewlibFDOPSfile is passed instead
 * of a descriptor.
 *
 * The pointer to any operation may be null; if the corresponding
 * operation is invoked the wrapper function will return an error
 * after setting @c errno to @c EBADF (generally; some operations may
 * use an alternative error code). */
typedef struct sBSPACMnewlibFDOPSfileOps {
  /** Release all resources associated with the file.
   *
   * In most cases, #fBSPACMnewlibFDOPSdriver() will dynamically
   * allocate a #sBSPACMnewlibFDOPSfile instance.  This operation is
   * the one that must free that memory.
   *
   * If unimplemented, close() fails with @c ENOSYS.
   *
   * @note Even if this operation is not implemented, invoking @c
   * close() on the corresponding descriptor will remove the
   * descriptor from #xBSPACMnewlibFDOPSfile_. */
  int (* op_close) (struct sBSPACMnewlibFDOPSfile * fp);

  /** Implementation for fstat().  If unimplemented, fstat() fails
   * with @c ENOSYS. */
  int (* op_fstat) (struct sBSPACMnewlibFDOPSfile * fp,
                    struct stat * buf);

  /** Implementation for isatty().  If unimplemented, isatty() fails
   * with @c ENOTTY.  @note A successful isatty() call returns 1; a
   * failed call returns zero after setting @c errno.  Do not return
   * -1 from this implementation. */
  int (* op_isatty) (struct sBSPACMnewlibFDOPSfile * fp);

  /** Implementation for lseek().  If unimplemented, lseek() fails
   * with @c ENOSYS.*/
  off_t (* op_lseek) (struct sBSPACMnewlibFDOPSfile * fp,
                      off_t offset,
                      int whence);

  /** Implementation for read().  If unimplemented, read() fails with
   * @c EBADF. */
  ssize_t (* op_read) (struct sBSPACMnewlibFDOPSfile * fp,
                       void * buf,
                       size_t count);

  /** Implementation for write().  If unimplemented, read() fails with
   * @c EBADF. */
  ssize_t (* op_write) (struct sBSPACMnewlibFDOPSfile * fp,
                        const void * buf,
                        size_t count);

  /** Implementation for ioctl(). */
  int (* op_ioctl) (struct sBSPACMnewlibFDOPSfile * fp,
                    int request,
                    va_list ap);
} sBSPACMnewlibFDOPSfileOps;

/** Record for the state associated with a file descriptor. */
typedef struct sBSPACMnewlibFDOPSfile {
  /** Pointer to data specific to the file or device.  The type is
   * fixed to whatever is expected by the operations in #ops. */
  void * dev;

  /** Pointer to the table that provides the implementation for file
   * descriptor operations.  No descriptor should ever have a null @p
   * ops pointer. */
  const sBSPACMnewlibFDOPSfileOps * ops;
} sBSPACMnewlibFDOPSfile;
typedef sBSPACMnewlibFDOPSfile * hBSPACMnewlibFDOPSfile;

/** Array of handles to active file descriptor states.  These should
 * be in one-to-one correspondence with the set of file descriptors
 * maintained by newlib.  Elements of the array are null if the
 * descriptor associated with the index is inactive.  The least open
 * index is used as the descriptor for the next successful open
 * operation.
 *
 * Generally applications should never need to examine this array.
 * However, they do need to ensure presence of a definition that has
 * sufficient room for the descriptors required by the application.
 *
 * \weakdef A weak definition for a three-element array is provided in
 * the newlib_fdops library; this is sufficient for the stdio
 * descriptors.
 *
 * @see #nBSPACMnewlibFDOPSfile
 */
extern hBSPACMnewlibFDOPSfile xBSPACMnewlibFDOPSfile_[];

/** The number of elements in #xBSPACMnewlibFDOPSfile_.  This is the
 * array length, not the number of descriptors in use.
 *
 * \weakdef A weak definition with value 3, corresponding to the stdio
 * descriptors, is provided in the newlib_fdops library, but should be
 * overridden by any application that provides an alternative
 * #xBSPACMnewlibFDOPSfile_ definition.
 *
 * @see #xBSPACMnewlibFDOPSfile_ */
extern const uint8_t nBSPACMnewlibFDOPSfile;

/** Handle for a file that supports no operations except closing it.
 * This is used by implementations of
 * vBSPACMnewlibFDOPSinitializeStdio() to reserve descriptors 0, 1,
 * and 2 when the application has no device to support them.  The
 * application can then close those descriptors and re-use them. */
extern hBSPACMnewlibFDOPSfile const hBSPACMnewlibFDOPSfileRESERVED;

/** Function to initialize descriptors used by stdio.
 *
 * newlib expects @c stdin (descriptor 0), @c stdout (1), and @c
 * stderr (2) to be immediately available for use, but does not open
 * them or associate them with a device.  To ensure the C library
 * descriptors are consistent with the handles in
 * #xBSPACMnewlibFDOPSfile_, the first invocation of any
 * file-descriptor operation will initialize handles associated with
 * these descriptors.  That is done by this function.
 *
 * The check for stdio being initialized is bypassed for calls to file
 * descriptor operations invoked by this function.  Consequently,
 * implementations of this function may use open(), so long as the
 * resulting descriptors end up as 0, 1, and 2 with their required
 * properties.
 *
 * @weakdef The newlib_fdops library provides a weak default
 * implementation of this, which opens @c /dev/console three times
 * with parameters appropriate for @c stdin, @c stdout, and @c stderr.
 * If something goes wrong it uses #hBSPACMnewlibFDOPSfileRESERVED to
 * prevent the descriptors from being unintentionally used in
 * subsequent open() calls.
 *
 * @note This function may be overridden by the application, but need
 * not be.  It should never be explicitly invoked by the application;
 * it is documented solely to describe the infrastructure requirements
 * for any overriding implementation.
 *
 * @note Although the implementation will open the three standard
 * descriptors, applications are permitted to close any or all of
 * them, based on their needs.  The corresponding slots in
 * #xBSPACMnewlibFDOPSfile_ will then be available for use as normal
 * descriptors or as the standard ones. */
void vBSPACMnewlibFDOPSinitializeStdio_ (void);

#endif /* BSPACM_NEWLIB_FDOPS_H */
