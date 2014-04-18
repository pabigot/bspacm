/* BSPACM - efm32/power demonstration application
 *
 * The following applies to all but the the prime number loop at the
 * end of main() which is licensed separately:
 *
 *   Written in 2014 by Peter A. Bigot <http://pabigot.github.io/bspacm/>
 *
 *   To the extent possible under law, the author(s) have dedicated all
 *   copyright and related and neighboring rights to this software to
 *   the public domain worldwide. This software is distributed without
 *   any warranty.
 *
 *   You should have received a copy of the CC0 Public Domain Dedication
 *   along with this software. If not, see
 *   <http://creativecommons.org/publicdomain/zero/1.0/>.
 *
 * The prime-number work load loop, derived from
 * ann0007_efm32_energymodes/main_prime.c, is subject to the
 * following:
 *
 ******************************************************************************
 * @section License
 * <b>(C) Copyright 2013 Energy Micro AS, http://www.energymicro.com</b>
 *******************************************************************************
 *
 * Permission is granted to anyone to use this software for any purpose,
 * including commercial applications, and to alter it and redistribute it
 * freely, subject to the following restrictions:
 *
 * 1. The origin of this software must not be misrepresented; you must not
 *    claim that you wrote the original software.
 * 2. Altered source versions must be plainly marked as such, and must not be
 *    misrepresented as being the original software.
 * 3. This notice may not be removed or altered from any source distribution.
 * 4. The source and compiled code may only be used on Energy Micro "EFM32"
 *    microcontrollers and "EFR4" radios.
 *
 * DISCLAIMER OF WARRANTY/LIMITATION OF REMEDIES: Energy Micro AS has no
 * obligation to support this Software. Energy Micro AS is providing the
 * Software "AS IS", with no express or implied warranties of any kind,
 * including, but not limited to, any implied warranties of merchantability
 * or fitness for any particular purpose or warranties against infringement
 * of any proprietary rights of a third party.
 *
 * Energy Micro AS will not be liable for any consequential, incidental, or
 * special damages, or any other relief, or for any claim by any third party,
 * arising from your use of this Software.
 */

#include <bspacm/utility/led.h>
#include <bspacm/periph/gpio.h>
#include <bspacm/periph/uart.h>
#include <em_gpio.h>
#include <em_emu.h>
#include <em_cmu.h>
#include <stdio.h>
#include <string.h>

/* Configure as plain input with no filter (DOUT=0).  For STK boards,
 * there's a pull-up, and a 1ms RC filter, so we don't need either of
 * those features.  If there was no filter, then set DOUT=1 so the
 * built-in 50ns filter is available for deglitch.  If there's also no
 * pull-up, then gpioModeInputPullFilter should be used.
 *
 * gpioModeInputPull would be used if you want pullup/pulldown without
 * a filter.  Not sure why you'd want that for an external
 * interrupt. */
const sBSPACMdeviceEFM32pinmux button[] = {
#if (BSPACM_BOARD_EFM32GG_STK3700 - 0)
  { .port = GPIO->P + gpioPortB, .pin = 9, .mode = gpioModeInput },
  { .port = GPIO->P + gpioPortB, .pin = 10, .mode = gpioModeInput },
#else
#error no button configuration available
#endif
};
static const unsigned int nbutton = sizeof(button)/sizeof(*button);

volatile unsigned int count_v[sizeof(button)/sizeof(*button)];
volatile uint32_t flags_v;

static void
irqHandler ()
{
  unsigned int pending = GPIO->IF & GPIO->IEN;
  int i;
  for (i = 0; i < nbutton; ++i) {
    const int pin = button[i].pin;
    const uint32_t mask = 1U << pin;
    if (mask & pending) {
      GPIO->IFC = mask;
      BSPACM_CORE_BITBAND_SRAM32(flags_v, i) = 1;
      count_v[i] += 1;
    }
  }
}

void GPIO_ODD_IRQHandler(void) { irqHandler(); }
void GPIO_EVEN_IRQHandler(void) { irqHandler(); }

const uint32_t hfrco_band[] = {
  _CMU_HFRCOCTRL_BAND_1MHZ,
  _CMU_HFRCOCTRL_BAND_7MHZ,
  _CMU_HFRCOCTRL_BAND_11MHZ,
  _CMU_HFRCOCTRL_BAND_14MHZ,
  _CMU_HFRCOCTRL_BAND_21MHZ,
#if defined( _CMU_HFRCOCTRL_BAND_28MHZ )
  _CMU_HFRCOCTRL_BAND_28MHZ,
#endif
};
const unsigned int nhfrco_band = sizeof(hfrco_band)/sizeof(*hfrco_band);

/* Override the weak default with one that forces 9600 baud.  The
 * default 115200 cannot be supported by the USART when running at
 * 1MHz. */
const sBSPACMperiphUARTconfiguration xBSPACMnewlibFDOPSconsoleConfiguration = {
  .speed_baud = 9600
};

void main ()
{
  int band_idx;
  uint32_t u32;
  vBSPACMledConfigure();
  SystemCoreClockUpdate();
  BSPACM_CORE_ENABLE_INTERRUPT();

  printf("\n" __DATE__ " " __TIME__ "\n");
  printf("System clock %lu Hz\n", SystemCoreClock);
  u32 = CMU_HFRCOBandGet();
  band_idx = 0;
  while ((u32 != hfrco_band[band_idx]) && (band_idx < nhfrco_band)) {
    ++band_idx;
  }
  if (nhfrco_band <= band_idx) {
    printf("No match for HFRCO band %lu\n", u32);
  } else {
    printf("HFRCO band %lu at index %d\n", u32, band_idx);
  }

  {
    BSPACM_CORE_SAVED_INTERRUPT_STATE(istate);
    int i;

    BSPACM_CORE_DISABLE_INTERRUPT();
    for (i = 0; i < nbutton; ++i) {
      const unsigned int port = button[i].port - GPIO->P;
      const unsigned int pin = button[i].pin;
      const uint32_t mask = 1U << pin;

      vBSPACMdeviceEFM32pinmuxConfigure(button+i, 1, 0);
      vBSPACMdeviceEFM32setPinNybble(&GPIO->EXTIPSELL, pin, port);
      GPIO->EXTIFALL |= mask;
      GPIO->EXTIRISE &= ~mask;
      GPIO->IFC = mask;
      GPIO->IEN |= mask;
      NVIC_ClearPendingIRQ((1 & pin) ? GPIO_ODD_IRQn : GPIO_EVEN_IRQn);
      NVIC_EnableIRQ((1 & pin) ? GPIO_ODD_IRQn : GPIO_EVEN_IRQn);
    }
    BSPACM_CORE_RESTORE_INTERRUPT_STATE(istate);
  }
  BSPACM_CORE_ENABLE_CYCCNT();

  while (1) {
    unsigned int count[sizeof(count_v)/sizeof(*count_v)];
    unsigned int flags;
    BSPACM_CORE_DISABLE_INTERRUPT();
    do {
      memcpy(count, (void*)count_v, sizeof(count_v));
      flags = flags_v;
      flags_v = 0;
    } while (0);
    BSPACM_CORE_ENABLE_INTERRUPT();
    if (flags) {
      static const sBSPACMperiphUARTconfiguration cfg = { .speed_baud = 9600 };

      printf("Count %u %u ; flags %x band %d\n", count[0], count[1], flags, band_idx);
      if (flags & 1) {
        band_idx -= 1;
      }
      if (flags & 2) {
        band_idx += 1;
      }
      if (0 > band_idx) {
        band_idx = 0;
      } else if (nhfrco_band <= band_idx) {
        band_idx = nhfrco_band - 1;
      }
      BSPACM_CORE_DISABLE_INTERRUPT();
      do {
        iBSPACMperiphUARTflush(hBSPACMdefaultUART, eBSPACMperiphUARTfifoState_TX);
        hBSPACMperiphUARTconfigure(hBSPACMdefaultUART, 0);
        CMU_HFRCOBandSet(hfrco_band[band_idx]);
        SystemCoreClockUpdate();
        hBSPACMperiphUARTconfigure(hBSPACMdefaultUART, &cfg);
      } while (0);
      BSPACM_CORE_ENABLE_INTERRUPT();
      printf("Now at band %u, core clock %lu, MODE %lx\n", band_idx, SystemCoreClock, MSC->READCTRL);

      /* This is the work load and MSC configuration used by EM to
       * assess power use in active mode.  Repeat it until one of the
       * event flags is set.
       *
       * BEGIN Energy Micro Derived Source */

      /* Supress Conditional Branch Target Prefetch */
      MSC->READCTRL = MSC_READCTRL_MODE_WS2SCBTP;
      {
#define PRIM_NUMS    64
        uint32_t i, d, n;
        uint32_t primes[PRIM_NUMS];

        /* Find prime numbers forever */
        while (! flags_v)
          {
            primes[0] = 1;
            for (i = 1; i < PRIM_NUMS;)
              {
                for (n = primes[i - 1] + 1;; n++)
                  {
                    for (d = 2; d <= n; d++)
                      {
                        if (n == d)
                          {
                            primes[i] = n;
                            goto nexti;
                          }
                        if (n % d == 0) break;
                      }
                  }
              nexti:
                i++;
              }
          }
      }
      /* END Energy Micro Derived Source */
    }
  }
}
