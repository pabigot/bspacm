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
 * @brief BSPACM file descriptor interface for UART peripherals
 *
 * @homepage http://github.com/pabigot/bspacm
 * @copyright Copyright 2014, Peter A. Bigot.  Licensed under <a href="http://www.opensource.org/licenses/BSD-3-Clause">BSD-3-Clause</a>
 */

#include <bspacm/core.h>
#include <bspacm/periph/uart.h>
#include <bspacm/newlib/fdops.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <stdarg.h>
#include <fcntl.h>

/** State for the console.  This is a little different, since there's
 * a unique device that may be opened multiple times (e.g. once each
 * for stdin, stdout, stderr).  Share the handle, but reference count
 * it so we can close the ones we don't need (stdin? stderr?) without
 * affecting ones we do need.  Still allow the UART to be reclaimed by
 * shutting it down once all console references are closed. */
static struct sConsoleState {
  /** Shared handle for open console instances.  This is a null
   * pointer when the corresponding UART is available for other
   * use. */
  sBSPACMnewlibFDOPSfile * handle;

  /** Number of active console references.  This is a negative number
   * when the UART associated with the console has been opened as
   * normal UART device. */
  int references;
} console_state;

static ssize_t
uart_read (struct sBSPACMnewlibFDOPSfile * fp,
           void * buf,
           size_t nbyte)
{
  hBSPACMperiphUART usp = (hBSPACMperiphUART)fp->dev;
  ssize_t rv = iBSPACMperiphUARTread(usp, buf, nbyte);
  if (0 == rv) {
    errno = EAGAIN;
    rv = -1;
  }
  return rv;
}

static ssize_t
uart_write (struct sBSPACMnewlibFDOPSfile * fp,
            const void * buf,
            size_t nbyte)
{
  hBSPACMperiphUART usp = (hBSPACMperiphUART)fp->dev;
  ssize_t rv;

  /* The BSPACM UART abstraction does not promise to write any data.
   * newlib, on the other hand, is ok with partial writes but
   * considers it an error if no data is written at all.
   * EWOULDBLOCK/EAGAIN bakes no custard with newlib.  So if we can't
   * write anything at all, but it's not an error, flush out whatever
   * we've already written and try again. */
  do {
    rv = iBSPACMperiphUARTwrite(usp, buf, nbyte);
    if (0 == rv) {
      (void)iBSPACMperiphUARTflush(usp, eBSPACMperiphUARTfifoState_TX);
    }
  } while (0 == rv);
  return rv;
}

static int
uart_close (struct sBSPACMnewlibFDOPSfile * fp)
{
  hBSPACMperiphUART usp = (hBSPACMperiphUART)fp->dev;
  int rv = -1;

  do {
    if (! usp) {
      errno = EINVAL;
      break;
    }
    (void)hBSPACMperiphUARTconfigure(usp, 0);
    fp->dev = 0;
    rv = 0;
  } while (0);
  free(fp);
  return rv;
}

static int
uart_ioctl (struct sBSPACMnewlibFDOPSfile * fp,
            int request,
            va_list ap)
{
  BSPACM_CORE_SAVED_INTERRUPT_STATE(istate);
  hBSPACMperiphUART usp = (hBSPACMperiphUART)fp->dev;
  int rv = -1;

  do {
    int flags = 1 + va_arg(ap, int);
    int fifo_mask = 0;
    if (! usp) {
      errno = EINVAL;
      break;
    }
    if (_FREAD & flags) {
      fifo_mask |= eBSPACMperiphUARTfifoState_RX;
    }
    if (_FWRITE & flags) {
      fifo_mask |= eBSPACMperiphUARTfifoState_TX;
    }
    BSPACM_CORE_DISABLE_INTERRUPT();
    rv = iBSPACMperiphUARTflush(usp, fifo_mask);
  } while (0);
  BSPACM_CORE_REENABLE_INTERRUPT(istate);
  return rv;
}

static const sBSPACMnewlibFDOPSfileOps xBSPACMnewlibFDOPSopsUART = {
  .op_close = uart_close,
  .op_read = uart_read,
  .op_write = uart_write,
  .op_ioctl = uart_ioctl,
};

hBSPACMnewlibFDOPSfile
hBSPACMnewlibFDOPSdriverUARTbind (hBSPACMperiphUART usp)
{
  sBSPACMnewlibFDOPSfile * fp = 0;

  do {
    if (! usp) {
      errno = EINVAL;
      break;
    }
    if ((hBSPACMdefaultUART == usp)
        && (0 != console_state.references)) {
      errno = EBUSY;
      break;
    }
    fp = malloc(sizeof(*fp));
    if (! fp) {
      (void)hBSPACMperiphUARTconfigure(usp, 0);
      errno = ENOMEM;
      break;
    }
    fp->dev = usp;
    fp->ops = &xBSPACMnewlibFDOPSopsUART;
    if (hBSPACMdefaultUART == usp) {
      console_state.references = -1;
    }
  } while (0);
  return fp;
}

static int
console_isatty (struct sBSPACMnewlibFDOPSfile * fp)
{
  return 1;
}

static int
console_close (struct sBSPACMnewlibFDOPSfile * fp)
{
  console_state.references -= 1;
  if (0 == console_state.references) {
    (void)uart_close(fp);
    console_state.handle = NULL;
  }
  return 0;
}

/* Console behaves differently on close, and returns true for
 * isatty(). */
static const sBSPACMnewlibFDOPSfileOps console_ops = {
  .op_close = console_close,
  .op_isatty = console_isatty,
  .op_read = uart_read,
  .op_write = uart_write,
  .op_ioctl = uart_ioctl,
};

__attribute__((__weak__))
const sBSPACMperiphUARTconfiguration xBSPACMnewlibFDOPSconsoleConfiguration = {
  .speed_baud = 0
};

sBSPACMnewlibFDOPSfile *
fBSPACMnewlibFDOPSdriverCONSOLE (const char * pathname,
                                 int flags)
{
  sBSPACMnewlibFDOPSfile * fp = NULL;
  static const char dev_name[] = "/dev/console";

#if 1
  { /* Inline compare takes 60 bytes.  Using memcmp for this adds 88
     * bytes; using strcmp adds over 500 bytes. */
    const char * dp = dev_name;
    const char * pp = pathname;
    while (*dp && pp && (*pp == *dp)) {
      ++dp;
      ++pp;
    }
    if (*dp || (! pp) || *pp) {
      return 0;
    }
  }
#elif 0
  if ((! pathname) || (0 != memcmp(dev_name, pathname, sizeof(dev_name)))) {
    return 0;
  }
#elif 0
  if ((! pathname) || (0 != strcmp(dev_name, pathname))) {
    return 0;
  }
#else
#endif
  do {
    fp = console_state.handle;
    if (! fp) {
      hBSPACMperiphUART usp = hBSPACMperiphUARTconfigure(hBSPACMdefaultUART, &xBSPACMnewlibFDOPSconsoleConfiguration);
      if (! usp) {
        errno = ENXIO;
        break;
      }
      if (0 != console_state.references) {
        errno = EBUSY;
        break;
      }
      fp = hBSPACMnewlibFDOPSdriverUARTbind(usp);
      if (fp) {
        /* Set flags and override ops */
        usp->flags = BSPACM_PERIPH_UART_FLAG_ONLCR;
        fp->ops = &console_ops;
        console_state.references = 0;
        console_state.handle = fp;
      } else {
        (void)hBSPACMperiphUARTconfigure(hBSPACMdefaultUART, 0);
      }
    }
  } while (0);
  if (fp) {
    console_state.references += 1;
  }
  return fp;
}

/** Weakly provide file-descriptor support for the console */
__attribute__((__weak__))
const fBSPACMnewlibFDOPSdriver xBSPACMnewlibFDOPSdriver[] = {
  fBSPACMnewlibFDOPSdriverCONSOLE,
  0
};
