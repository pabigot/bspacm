/* BSPACM - tm4c/clock demonstration application
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
#include <driverlib/sysctl.h>
#include <stdio.h>
#include <fcntl.h>

void main ()
{
#if (BSPACM_DEVICE_LINE_TM4C123 - 0)
#if 0
  SysCtlClockSet(SYSCTL_SYSDIV_1 | SYSCTL_USE_OSC | SYSCTL_OSC_MAIN | SYSCTL_XTAL_16MHZ); /* OSC/1 = 16 MHz */
#elif 0
  /* NOTE: Setting to 80 MHz requires a patch to SysCtlClockGet() when
   * using TivaWare_C_Series-2.1.0.12573.  See
   * http://e2e.ti.com/support/microcontrollers/tiva_arm/f/908/p/330524/1156581.aspx#1156581 */
  SysCtlClockSet(SYSCTL_SYSDIV_2_5 | SYSCTL_USE_PLL | SYSCTL_OSC_MAIN | SYSCTL_XTAL_16MHZ); /* 2*PLL/5 = 80 MHz */
#elif 0
  SysCtlClockSet(SYSCTL_SYSDIV_3 | SYSCTL_USE_PLL | SYSCTL_OSC_MAIN | SYSCTL_XTAL_16MHZ); /* PLL/3 = 66.67 MHz */
#elif 0
  SysCtlClockSet(SYSCTL_SYSDIV_4 | SYSCTL_USE_PLL | SYSCTL_OSC_MAIN | SYSCTL_XTAL_16MHZ); /* PLL/4 = 50 MHz */
#elif 0
  SysCtlClockSet(SYSCTL_SYSDIV_5 | SYSCTL_USE_PLL | SYSCTL_OSC_MAIN | SYSCTL_XTAL_16MHZ); /* PLL/5 = 40 MHz */
#elif 0
  SysCtlClockSet(SYSCTL_SYSDIV_10 | SYSCTL_USE_PLL | SYSCTL_OSC_MAIN | SYSCTL_XTAL_16MHZ); /* PLL/10 = 20 MHz */
#elif 0
  SysCtlClockSet(SYSCTL_SYSDIV_20 | SYSCTL_USE_PLL | SYSCTL_OSC_MAIN | SYSCTL_XTAL_16MHZ); /* PLL/20 = 10 MHz */
#elif 0
  SysCtlClockSet(SYSCTL_SYSDIV_40 | SYSCTL_USE_PLL | SYSCTL_OSC_MAIN | SYSCTL_XTAL_16MHZ); /* PLL/40 = 5 MHz */
#elif 1
  SysCtlClockSet(SYSCTL_SYSDIV_64 | SYSCTL_USE_PLL | SYSCTL_OSC_MAIN | SYSCTL_XTAL_16MHZ); /* PLL/64 = 3.125 MHz */
#endif /* pick clock speed */
  SystemCoreClock = SysCtlClockGet();
#elif (BSPACM_DEVICE_LINE_TM4C129 - 0)
#if 0
  SystemCoreClock = SysCtlClockFreqSet((SYSCTL_OSC_INT | SYSCTL_USE_OSC | SYSCTL_MAIN_OSC_DIS), 16000000);
#elif 1
  SystemCoreClock = SysCtlClockFreqSet((SYSCTL_XTAL_25MHZ | SYSCTL_OSC_MAIN | SYSCTL_USE_PLL | SYSCTL_CFG_VCO_480), 120000000);
#elif 0
  SystemCoreClock = SysCtlClockFreqSet((SYSCTL_XTAL_25MHZ | SYSCTL_OSC_MAIN | SYSCTL_USE_PLL | SYSCTL_CFG_VCO_480), 40000000);
#endif /* pick and record clock speed */
#endif /* LINE */
  vBSPACMledConfigure();
  BSPACM_CORE_ENABLE_CYCCNT();
  BSPACM_CORE_ENABLE_INTERRUPT();

  printf("\n" __DATE__ " " __TIME__ "\n");
  printf("System clock %lu Hz\n", SystemCoreClock);
  while (1) {
    vBSPACMledSet(0, -1);
    BSPACM_CORE_DELAY_CYCLES(SystemCoreClock);
  }
  fflush(stdout);
  ioctl(1, BSPACM_IOCTL_FLUSH, O_WRONLY);
  BSPACM_CORE_DEEP_SLEEP();
}
