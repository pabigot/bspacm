/* BSPACM - bootstrap/blink demonstration application
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

#if (BSPACM_DEVICE_SERIES_TM4C - 0)
#include <driverlib/sysctl.h>
#include <driverlib/rom.h>
#include <driverlib/rom_map.h>

#elif (BSPACM_DEVICE_SERIES_EFM32 - 0)
#include <em_device.h>
#include <em_chip.h>
#include <em_cmu.h>
#include <em_gpio.h>

static volatile unsigned int ticks_ms;

void
SysTick_Handler (void)
{
  ++ticks_ms;
}
#elif (BSPACM_DEVICE_SERIES_NRF51 - 0)
#include "nrf_delay.h"
#endif

uint32_t rl = SysTick_LOAD_RELOAD_Msk;

void main ()
{
  int idx = 0;

  /* Configure the clock and determine frequency for use in delay. */
#if (BSPACM_DEVICE_LINE_TM4C123 - 0)
  MAP_SysCtlClockSet(SYSCTL_SYSDIV_1 | SYSCTL_USE_OSC | SYSCTL_OSC_MAIN |
                     SYSCTL_XTAL_16MHZ);
  SystemCoreClock = MAP_SysCtlClockGet();
#elif (BSPACM_DEVICE_LINE_TM4C129 - 0)
  SystemCoreClock = MAP_SysCtlClockFreqSet(SYSCTL_XTAL_25MHZ
                                           | SYSCTL_OSC_MAIN
                                           | SYSCTL_USE_PLL
                                           | SYSCTL_CFG_VCO_480, 40000000);
#elif (BSPACM_DEVICE_SERIES_EFM32 - 0)
  CHIP_Init();
  if (SysTick_Config(CMU_ClockFreqGet(cmuClock_CORE) / 1000)) while (1) ;
#elif (BSPACM_DEVICE_SERIES_NRF51 - 0)
  /* nRF51 does not implement systick, though it does keep a variable
   * to hold the clock speed. */
  SystemCoreClockUpdate();
#endif /* BSPACM_DEVICE SERIES */
  vBSPACMledConfigure();
  idx = 0;
  while (1) {
    ++idx;
    if (idx >= nBSPACMleds) {
      idx = 0;
    }
    vBSPACMledSet(idx, (1 == nBSPACMleds) ? -1 : 1);
#if (BSPACM_DEVICE_SERIES_TM4C - 0)
    /* Delay for one second.  Parameter is number of iterations of a
     * 3-cycle loop. */
    MAP_SysCtlDelay(SystemCoreClock / 3);
#elif (BSPACM_DEVICE_SERIES_EFM32 - 0)
    {
      unsigned int start_ms = ticks_ms;
      while ((int)(ticks_ms - start_ms) < 500) {
      }
    }
#elif (BSPACM_DEVICE_SERIES_NRF51 - 0)
    nrf_delay_ms(1000);
#endif
    if (1 < nBSPACMleds) {
      vBSPACMledSet(idx, 0);
    }
  }
}
