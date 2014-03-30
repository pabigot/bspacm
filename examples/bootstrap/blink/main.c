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
#endif

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
#endif /* BSPACM_DEVICE SERIES */
  vBSPACMledConfigure();
  idx = 0;
  while (1) {
    ++idx;
    if (idx >= nBSPACMleds) {
      idx = 0;
    }
    vBSPACMledSet(idx, (1 == nBSPACMleds) ? -1 : 1);
    /* Delay for one second.  Parameter is number of iterations of a
     * 3-cycle loop. */
#if (BSPACM_DEVICE_SERIES_TM4C - 0)
    MAP_SysCtlDelay(SystemCoreClock / 3);
#elif (BSPACM_DEVICE_SERIES_EFM32 - 0)
    {
      unsigned int start_ms = ticks_ms;
      while ((int)(ticks_ms - start_ms) < 500) {
      }
    }
#endif
    if (1 < nBSPACMleds) {
      vBSPACMledSet(idx, 0);
    }
  }
}
