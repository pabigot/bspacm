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
 * @brief Wrappers supporting file descriptor operations
 *
 *
 * @homepage http://github.com/pabigot/bspacm
 * @copyright Copyright 2014, Peter A. Bigot.  Licensed under <a href="http://www.opensource.org/licenses/BSD-3-Clause">BSD-3-Clause</a>
 */

#include <bspacm/core.h>
#include <bspacm/newlib/fdops.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <stdarg.h>
#include <fcntl.h>

/* We're providing system call implementation here, so ensure we have
 * visible prototypes that match what newlib is expecting. */
#define _COMPILING_NEWLIB
#include <sys/unistd.h>

static int
dummy_close (struct sBSPACMnewlibFDOPSfile * fp)
{
  return 0;
}
static const sBSPACMnewlibFDOPSfileOps dummy_ops = {
  .op_close = dummy_close
};
static sBSPACMnewlibFDOPSfile dummy_file = {
  .ops = &dummy_ops
};
hBSPACMnewlibFDOPSfile const hBSPACMnewlibFDOPSfileRESERVED = &dummy_file;

__attribute__((__weak__))
hBSPACMnewlibFDOPSfile xBSPACMnewlibFDOPSfile_[3] = { 0 };
__attribute__((__weak__))
const uint8_t nBSPACMnewlibFDOPSfile = sizeof(xBSPACMnewlibFDOPSfile_)/sizeof(*xBSPACMnewlibFDOPSfile_);

__attribute__((__weak__))
void
vBSPACMnewlibFDOPSinitializeStdio_ (void)
{
  int fd;

  (void)open("/dev/console", O_RDONLY); /* stdin */
  (void)open("/dev/console", O_RDWR);   /* stdout */
  (void)open("/dev/console", O_RDWR);   /* stderr */
  for (fd = 0; fd < 3; ++fd) {
    if (nBSPACMnewlibFDOPSfile <= fd) {
      break;
    }
    if (0 == xBSPACMnewlibFDOPSfile_[fd]) {
      xBSPACMnewlibFDOPSfile_[fd] = hBSPACMnewlibFDOPSfileRESERVED;
    }
  }
}

static unsigned char stdio_initialized_;

static void
initialize_stdio_ ()
{
  if (! stdio_initialized_) {
    stdio_initialized_ = 1;
    vBSPACMnewlibFDOPSinitializeStdio_();
  }
}

/** Common validation/mapping from descriptor to handle.
 *
 * This returns a file handle if the descriptor is valid; otherwise it
 * sets @c errno and returns a null pointer.  Callers are guaranteed
 * that the descriptor has an ops table assigned, but must validate
 * any specific operation the caller needs. */
static hBSPACMnewlibFDOPSfile
validated_handle (int fd)
{
  hBSPACMnewlibFDOPSfile rv = NULL;
  if (! stdio_initialized_) {
    initialize_stdio_();
  }
  do {
    if (fd >= nBSPACMnewlibFDOPSfile) {
      errno = EBADF;
      break;
    }
    rv = xBSPACMnewlibFDOPSfile_[fd];
    if ((! rv) || (! rv->ops)) {
      errno = EBADF;
      rv = NULL;
    }
  } while (0);
  return rv;
}

int
_open (const char * pathname,
       int flags)
{
  int rv = -1;

  if (! stdio_initialized_) {
    initialize_stdio_();
  }
  do {
    int fd;
    const fBSPACMnewlibFDOPSdriver * dp = xBSPACMnewlibFDOPSdriver;
    hBSPACMnewlibFDOPSfile fh;

    /* Search for an open descriptor */
    for (fd = 0; fd < nBSPACMnewlibFDOPSfile; ++fd) {
      if (! xBSPACMnewlibFDOPSfile_[fd]) {
        break;
      }
    }
    if (nBSPACMnewlibFDOPSfile <= fd) {
      errno = ENFILE;
      break;
    }

    /* Search for a driver able to create a handle for the device */
    errno = ENODEV;
    while (*dp) {
      fh = (*dp)(pathname, flags);
      /* Only accept handles that will pass basic validation. */
      if (fh && fh->ops) {
        xBSPACMnewlibFDOPSfile_[fd] = fh;
        errno = 0;
        rv = fd;
        break;
      }
      ++dp;
      errno = ENODEV;
    }
  } while (0);
  return rv;
}

int
_close (int fd)
{
  int rv = -1;
  hBSPACMnewlibFDOPSfile fh = validated_handle(fd);

  do {
    if (! fh) {
      break;
    }
    /* Remove the descriptor from the table, regardless of
     * success/failure of the close operation. */
    xBSPACMnewlibFDOPSfile_[fd] = 0;
    if (! fh->ops->op_close) {
      errno = ENOSYS;
      break;
    }
    rv = fh->ops->op_close(fh);
  } while (0);
  return rv;
}

int
_fstat (int fd,
        struct stat * buf)
{
  int rv = -1;
  hBSPACMnewlibFDOPSfile fh = validated_handle(fd);
  do {
    if (! fh) {
      break;
    }
    if (! fh->ops->op_fstat) {
      errno = ENOSYS;
      break;
    }
    rv = fh->ops->op_fstat(fh, buf);
  } while (0);
  return rv;
}

int
_isatty (int fd)
{
  int rv = 0;                   /* NOTE: Nonstandard error return */
  hBSPACMnewlibFDOPSfile fh = validated_handle(fd);

  do {
    if (! fh) {
      break;
    }
    if (! fh->ops->op_isatty) {
      errno = ENOTTY;
      break;
    }
    rv = fh->ops->op_isatty(fh);
  } while (0);
  return rv;
}

off_t
_lseek (int fd,
        off_t offset,
        int whence)
{
  off_t rv = (off_t)-1;
  hBSPACMnewlibFDOPSfile fh = validated_handle(fd);

  do {
    if (! fh) {
      break;
    }
    if (! fh->ops->op_lseek) {
      errno = ENOSYS;
      break;
    }
    rv = fh->ops->op_lseek(fh, offset, whence);
  } while (0);
  return rv;
}

ssize_t
_read (int fd,
       void * buf,
       size_t count)
{
  int rv = -1;
  hBSPACMnewlibFDOPSfile fh = validated_handle(fd);

  do {
    if (! fh) {
      break;
    }
    if (! fh->ops->op_read) {
      /* Descriptor ok but read not accepted = EBADF */
      errno = EBADF;
      break;
    }
    rv = fh->ops->op_read(fh, buf, count);
  } while (0);
  return rv;
}

ssize_t
_write (int fd,
        const void * buf,
        size_t count)
{
  int rv = -1;
  hBSPACMnewlibFDOPSfile fh = validated_handle(fd);

  do {
    if (! fh) {
      break;
    }
    if (! fh->ops->op_write) {
      /* Descriptor ok but write not accepted = EBADF */
      errno = EBADF;
      break;
    }
    rv = fh->ops->op_write(fh, buf, count);
  } while (0);
  return rv;
}

int
fcntl (int fd,
       int cmd,
       ...)
{
  hBSPACMnewlibFDOPSfile fh;
  ssize_t rv = -1;
  va_list ap;

  va_start(ap, cmd);
  fh = validated_handle(fd);
  do {
    if (! fh) {
      break;
    }
    if (! fh->ops->op_fcntl) {
      errno = EBADF;
      break;
    }
    rv = fh->ops->op_fcntl(fh, cmd, ap);
  } while (0);
  va_end(ap);
  return rv;
}
