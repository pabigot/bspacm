/* BSPACM - periph/uart demonstration application
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

#include <bspacm/utility/led.h>
#include <bspacm/periph/uart.h>
#include <stdio.h>

#ifndef UART_HANDLE
#define UART_HANDLE hBSPACMdefaultUART
#endif /* UART_HANDLE */

void main ()
{
  hBSPACMperiphUART usp = UART_HANDLE;
  const sBSPACMperiphUARTconfiguration cfg = { .speed_baud = 0 };
  unsigned int nrx = 0;
  char tx_buffer[80];
  char rx_buffer[4];
  unsigned int ctr = 0;

  SystemCoreClockUpdate();
  vBSPACMledConfigure();
  BSPACM_CORE_ENABLE_CYCCNT();
  BSPACM_CORE_ENABLE_INTERRUPT();
  while (usp) {
    int rc;
    unsigned int len;
    char * sp;

    usp = hBSPACMperiphUARTconfigure(usp, &cfg);
    if (! usp) {
      break;
    }
    vBSPACMledSet(BSPACM_LED_GREEN, 11);
    BSPACM_CORE_DELAY_CYCLES(SystemCoreClock / 2);
    rc = iBSPACMperiphUARTread(usp, rx_buffer, sizeof(rx_buffer)-1);
    if (0 < rc) {
      rx_buffer[rc] = 0;
      len = snprintf(tx_buffer, sizeof(tx_buffer), "Read %u: '%s'\r\n", rc, rx_buffer);
      nrx += rc;
    } else {
      len = snprintf(tx_buffer, sizeof(tx_buffer), "Iteration %u, total read %u\r\n", ctr, nrx);
    }
    sp = tx_buffer;
    while (0 < len) {
      rc = iBSPACMperiphUARTwrite(usp, sp, len);
      if (0 < rc) {
        len -= rc;
        sp += rc;
      }
    }
    /* Wait until everything we've queued for transmission has been
     * flushed.  (Ignore RX material which we are not reading any
     * more.) */
    while ((eBSPACMperiphUARTfifoState_HWTX
            | eBSPACMperiphUARTfifoState_SWTX)
           & iBSPACMperiphUARTfifoState(usp)) {
      /* spin */
    }
    usp = hBSPACMperiphUARTconfigure(usp, NULL);
    vBSPACMledSet(BSPACM_LED_GREEN, 0);
    BSPACM_CORE_DELAY_CYCLES(SystemCoreClock/2);
    ++ctr;
  }
  vBSPACMledSet(BSPACM_LED_RED, 1);
}
