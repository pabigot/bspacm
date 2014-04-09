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
 * @brief Stubs required for full newlib system support
 *
 * How newlib from GNU Toolchain for ARM Embedded works:
 * @li @c write() invokes @c _write_r() with the current reentrancy context;
 * @li @c _write_r() invokes @c _write() and copies errno appropriately;
 * @li @c _write() must be provided by something
 *
 * The set of functions and data objects corresponds to those provided
 * by newlib's <tt>-specs=nosys.specs</tt> option, which provides
 * default implementations when there is no system environment.  We're
 * not using that solution, because BSPACM does provide some pieces of
 * a system environment sometimes and I don't want to have to worry
 * about whose empty implementation gets found by the linker first.
 *
 * There are weak definitions for all functions here; these
 * implementations simply return an error.  The expectation is that
 * something (libbspacm or the user application) provides any required
 * non-trivial implementation.
 *
 * @homepage http://github.com/pabigot/bspacm
 * @copyright Copyright 2014, Peter A. Bigot.  Licensed under <a href="http://www.opensource.org/licenses/BSD-3-Clause">BSD-3-Clause</a>
 */

#include <bspacm/core.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <sys/times.h>
#include <errno.h>

/* We're providing system call stub implementations here, so ensure we
 * have visible prototypes that match what newlib is expecting. */
#define _COMPILING_NEWLIB
#include <sys/unistd.h>

static char * env[] = { 0 };

__attribute__((__weak__))
char ** environ = env;

__attribute__((__weak__))
int
_chown (const char * path,
        uid_t owner,
        gid_t group)
{
  errno = ENOSYS;
  return -1;
}

__attribute__((__weak__))
int
_close (int fd)
{
  errno = ENOSYS;
  return -1;
}

__attribute__((__weak__))
int
_execve (const char * filename,
         char * const argv[],
         char * const envp[])
{
  errno = ENOSYS;
  return -1;
}

__attribute__((__weak__))
pid_t
_fork (void)
{
  errno = ENOSYS;
  return -1;
}

__attribute__((__weak__))
int
_fstat (int fd,
        struct stat * buf)
{
  errno = ENOSYS;
  return -1;
}

__attribute__((__weak__))
pid_t
_getpid (void)
{
  errno = ENOSYS;
  return 1;
}

__attribute__((__weak__))
int
_gettimeofday (struct timeval * tv,
               struct timezone * tz)
{
  errno = ENOSYS;
  return -1;
}

__attribute__((__weak__))
int
_isatty (int fd)
{
  errno = ENOSYS;
  return -1;
}

__attribute__((__weak__))
int
_kill (pid_t pid,
       int sig)
{
  errno = ENOSYS;
  return -1;
}

__attribute__((__weak__))
int
_link (const char * oldpath,
       const char * newpath)
{
  errno = ENOSYS;
  return -1;
}

__attribute__((__weak__))
off_t
_lseek (int fd,
        off_t offset,
        int whence)
{
  errno = ENOSYS;
  return -1;
}

__attribute__((__weak__))
int
_open (const char * pathname,
       int flags)
{
  errno = ENOSYS;
  return -1;
}

__attribute__((__weak__))
ssize_t
_read (int fd,
       void * buf,
       size_t count)
{
  errno = ENOSYS;
  return -1;
}

__attribute__((__weak__))
ssize_t
_readlink (const char * path,
           char * buf,
           size_t bufsiz)
{
  errno = ENOSYS;
  return -1;
}

/* The weak default for sbrk is in newlib/sbrk.c */

__attribute__((__weak__))
int
_stat (const char * path,
       struct stat * buf)
{
  errno = ENOSYS;
  return -1;
}

__attribute__((__weak__))
int
_symlink (const char * oldpath,
          const char * newpath)
{
  errno = ENOSYS;
  return -1;
}

__attribute__((__weak__))
clock_t
_times (struct tms *buf)
{
  errno = ENOSYS;
  return -1;
}

__attribute__((__weak__))
int
_unlink (const char * pathname)
{
  errno = ENOSYS;
  return -1;
}

__attribute__((__weak__))
pid_t
_wait (int * status)
{
  errno = ENOSYS;
  return -1;
}

__attribute__((__weak__))
ssize_t
_write (int fd,
        const void * buf,
        size_t count)
{
  errno = ENOSYS;
  return -1;
}

__attribute__((__weak__))
void
_exit (int status)
{
  errno = ENOSYS;
  while (1);
}
