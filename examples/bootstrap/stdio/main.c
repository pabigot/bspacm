/* BSPACM - bootstrap/stdio demonstration application
 *
 * Written in 2014 by Peter A. Bigot <http://pabigot.github.io/bspacm/>
 *
 * To the extent possible under law, the author(s) have dedicated all
 * copyright and related and neighboring rights to this software to
 * the public domain worldwide. This software is distributed without
 * any warranty.
 *
 * You should have received a copy of the CC0 Public Domain Dedication
 * along with this software. If not, see
 * <http://creativecommons.org/publicdomain/zero/1.0/>.
 */

#include <bspacm/core.h>
#include <bspacm/utility/led.h>
#include <bspacm/periph/uart.h>
#include <bspacm/utility/console.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <errno.h>

/* How newlib from GNU Toolchain for ARM Embedded works:
 * + write() invokes _write_r() with the current reentrancy context;
 * + _write_r() invokes _write() and copies errno appropriately;
 * + _write() must be provided by something, such as -lnosys or BSPACM
 */

/* We're providing the system call implementation here, so ensure we
 * have visible prototypes that match what newlib is expecting. */
#define _COMPILING_NEWLIB
#include <sys/unistd.h>

static hBSPACMperiphUART uart_statep;

sBSPACMperiphUARTstate *
configure_console ()
{
  const sBSPACMperiphUARTconfiguration cfg = { .speed_baud = 115200 };
  hBSPACMperiphUART rv;

  while (! hBSPACMutilityCONSOLEuart) {
    /* Somebody didn't provide a uart */
  }
#if 1
  /* Test whether deconfigure is safe when never configured, and in
   * fact whether it works at all.  Spin here if this turns out to be
   * unsafe, so we can fix things. */
  rv = hBSPACMperiphUARTconfigure(hBSPACMutilityCONSOLEuart, 0);
  while (! rv);
#endif
  rv = hBSPACMperiphUARTconfigure(hBSPACMutilityCONSOLEuart, &cfg);
  uart_statep = rv;
  return rv;
}

int
_read (int fd, void * buf, size_t nbyte)
{
  int rv = iBSPACMperiphUARTread(uart_statep, buf, nbyte);
  if (0 == rv) {
    errno = EAGAIN;
    rv = -1;
  }
  return rv;
}

int
_write (int fd, const void * buf,  size_t nbyte)
{
  int rv = iBSPACMperiphUARTwrite(uart_statep, buf, nbyte);
  if (0 == rv) {
    errno = EAGAIN;
    rv = -1;
  }
  return rv;
}

static void deconfigure_console ()
{
  if (uart_statep) {
    uart_statep = hBSPACMperiphUARTconfigure(uart_statep, 0);
  }
}

void main ()
{
  int ctr;
  unsigned int last_rxcount;
  hBSPACMperiphUART usp;

  vBSPACMledConfigure();
  usp = configure_console();
  usp->flags = BSPACM_PERIPH_UART_FLAG_ONLCR;
  last_rxcount = usp->rx_count;

  BSPACM_CORE_ENABLE_INTERRUPT();

  /* NB: setvbuf() has no effect when using newlib-nano */
  setvbuf(stdout, NULL, _IONBF, 0);
  printf("\n" __DATE__ " " __TIME__ "\n");
  printf("System clock %lu Hz\n", SystemCoreClock);

  ctr = 0;
  while (1) {
    const int MAX_PER_LINE = 40;
    volatile uint32_t delay = 1000001;

    ++ctr;
    if (MAX_PER_LINE == (ctr % (2 + MAX_PER_LINE))) {
      unsigned int txcount;
      unsigned int rxcount;
      unsigned int errcount;
      uint8_t fe;
      uint8_t pe;
      uint8_t be;
      uint8_t oe;
      uint16_t de;

      BSPACM_CORE_DISABLE_INTERRUPT();
      do {
        txcount = usp->tx_count;
        rxcount = usp->rx_count;
        fe = usp->rx_frame_errors;
        pe = usp->rx_parity_errors;
        be = usp->rx_break_errors;
        oe = usp->rx_overrun_errors;
        de = usp->rx_dropped_errors;
      } while (0);
      BSPACM_CORE_ENABLE_INTERRUPT();
      errcount = fe + pe + be + oe + de;

      printf("  %u %u %u : %02x\n", txcount, rxcount, errcount, iBSPACMperiphUARTfifoState(usp));
      vBSPACMledSet(BSPACM_LED_GREEN, -1);

      if (last_rxcount != rxcount) {
        int ch;

        while (0 <= ((ch = getchar()))) {
          printf("read 0x%02x '%c'\n", ch, ch);
        }
        last_rxcount = rxcount;
        while (0 != iBSPACMperiphUARTfifoState(usp)) {
          /* NOT SAFE for new RX */
        }
        deconfigure_console();
#if 1
        while (--delay) {
        }
        delay = 1000001;
#endif
        usp = configure_console();
      }
      ctr = 0;
#if 1
      while (--delay) {
      }
      delay = 1000001;
#endif
    } else {
      putchar('0' + (ctr % 10));
    }
#if 0
    while (--delay) {
    }
    delay = 1000001;
#endif
    (void)ctr;
    (void)delay;
  }
}
