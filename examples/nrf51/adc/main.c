/* BSPACM - nRF51 battery voltage sampler
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
#include <bspacm/newlib/ioctl.h>
#include <bspacm/utility/hires.h>
#include <stdio.h>
#include <fcntl.h>

#include "nrf51_bitfields.h"

#ifndef ADC_BITS
#define ADC_BITS 8
#endif /* ADC_BITS */

/* Empirically, onversion takes about 80 + (256 << (ADC_BITS - 8))
 * clock cycles, adjusting for measurement error.  This is consistent
 * with the estimates in section 31.1.2 of the nRF51 SRM. */
#if (8 == ADC_BITS)
#define ADC_DIVISOR (1 << 8)
#define ADC_CONFIG_RES ADC_CONFIG_RES_8bit
#elif (9 == ADC_BITS)
#define ADC_DIVISOR (1 << 9)
#define ADC_CONFIG_RES ADC_CONFIG_RES_9bit
#elif (10 == ADC_BITS)
#define ADC_DIVISOR (1 << 10)
#define ADC_CONFIG_RES ADC_CONFIG_RES_10bit
#else /* ADC_BITS */
#error Unsupported ADC_BITS
#endif /* ADC_BITS */

/* Measure the supply voltage divided by three */
#define ADC_CONFIG_INPSEL ADC_CONFIG_INPSEL_SupplyOneThirdPrescaling

/* Supply divisor, use as multiple to reconstruct supply voltage */
#define ADC_INPSEL_DIVISOR 3

#ifndef ADC_VBAT_OFFSET_mV
/* Per nRF51 examples/ble_peripheral/ble_app_proximity_low_power/main.c
 * the value measured is after a diode-induced voltage drop.
 * Define the correction for this drop. */
#define ADC_VBAT_OFFSET_mV 270
#endif /* ADC_VBAT_OFFSET_mV */

void main ()
{
  vBSPACMledConfigure();

  setvbuf(stdout, NULL, _IONBF, 0);
  printf("\n" __DATE__ " " __TIME__ "\n");
  printf("System clock %lu Hz\n", SystemCoreClock);

  do {

    if ((0 != iBSPACMhiresInitialize(SystemCoreClock))
        || (0 != iBSPACMhiresSetEnabled(true))) {
      printf("ERR: Failed to start high-resolution timer\n");
      break;
    }

    /* Prevent UART interrupts from affecting timing */
    fflush(stdout);
    ioctl(1, BSPACM_IOCTL_FLUSH, O_WRONLY);

    NRF_ADC->CONFIG = 0
      | (ADC_CONFIG_RES << ADC_CONFIG_RES_Pos)
      | (ADC_CONFIG_INPSEL << ADC_CONFIG_INPSEL_Pos)
      | (ADC_CONFIG_REFSEL_VBG << ADC_CONFIG_REFSEL_Pos)
      | (ADC_CONFIG_PSEL_Disabled << ADC_CONFIG_PSEL_Pos)
      | (ADC_CONFIG_EXTREFSEL_None << ADC_CONFIG_EXTREFSEL_Pos)
      ;
    NRF_ADC->EVENTS_END = 0;
    BSPACM_HIRES_TIMER->TASKS_CAPTURE[0] = 1;
    NRF_ADC->TASKS_START = 1;
    while (! NRF_ADC->EVENTS_END) {
    }
    BSPACM_HIRES_TIMER->TASKS_CAPTURE[1] = 1;

    unsigned int t0 = BSPACM_HIRES_TIMER->CC[0];
    unsigned int t1 = BSPACM_HIRES_TIMER->CC[1];
    unsigned int vbat_mV = ADC_VBAT_OFFSET_mV + (ADC_INPSEL_DIVISOR * 1000 * (unsigned)NRF_ADC->RESULT) / ADC_DIVISOR;
    printf("Result: %d mV in %u clocks\n", vbat_mV, t1-t0);

  } while (0);

  fflush(stdout);
  ioctl(1, BSPACM_IOCTL_FLUSH, O_WRONLY);
  BSPACM_CORE_DEEP_SLEEP();
}
