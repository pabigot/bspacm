/* BSPACM - efm32/rtc demonstration application
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
#include <em_chip.h>
#include <em_emu.h>
#include <em_cmu.h>
#include <em_rmu.h>
#include <em_rtc.h>
#include <string.h>
#include <stdio.h>

#ifndef WAKE_INTERVAL_S
#define WAKE_INTERVAL_S 10
#endif /* WAKE_INTERVAL_S */

typedef struct sResetCause {
  uint16_t value;
  uint16_t mask;
  const char * name;
} sResetCause;
static const sResetCause xResetCause[] = {
  { 0x01, 0x01, "PO" },                   /**< 0 Power On Reset */
  { 0x02, 0x83, "BODUNREG" },             /**< 1 Brown Out Detector Unregulated Domain Reset */
  { 0x04, 0x1f, "BODREG" },               /**< 2 Brown Out Detector Regulated Domain Reset */
  { 0x08, 0x0b, "EXT" },                  /**< 3 External Pin Reset */
  { 0x10, 0x13, "WDOG" },                 /**< 4 Watchdog Reset */
  { 0x20, 0x7ff, "LOCKUP" },              /**< 5 LOCKUP Reset */
  { 0x40, 0x7df, "SYSREQ" },              /**< 6 System Request Reset */
  { 0x80, 0x799, "EM4RST" },              /**< 7 EM4 Reset */
  { 0x180, 0x799, "EM4WURST" },           /**< 8 EM4 Wake-up Reset */
  { 0x200, 0x61f, "BODAVDD0" },           /**< 9 AVDD0 Bod Reset */
  { 0x400, 0x61f, "BODAVDD1" },           /**< 10 AVDD1 Bod Reset */
  { 0x800, 0x809, "BUBODVDDDREG" },       /**< 11 Backup Brown Out Detector, VDD_DREG */
  { 0x1000, 0x1009, "BUBODBUVIN" },       /**< 12 Backup Brown Out Detector, BU_VIN */
  { 0x2000, 0x2009, "BUBODUNREG" },       /**< 13 Backup Brown Out Detector Unregulated Domain */
  { 0x4000, 0x4009, "BUBODREG" },         /**< 14 Backup Brown Out Detector Regulated Domain */
  { 0x8000, 0x8001, "BUMODE" },           /**< 15 Backup mode reset */
};

volatile uint8_t rtc_if;
void RTC_IRQHandler (void)
{
  rtc_if = RTC->IF;
  RTC->IFC = rtc_if;
}

void main ()
{
  uint32_t rtc_Hz;
  uint16_t reset_cause;
  int sleep_mode = 0;

  CHIP_Init();
  reset_cause = RMU->RSTCAUSE;
  RMU_ResetCauseClear();
#if ! defined(_EFM32_ZERO_FAMILY)
  BSPACM_CORE_BITBAND_PERIPH(RMU->CTRL, _RMU_CTRL_BURSTEN_SHIFT) = 0;
#endif
  SystemCoreClockUpdate();
  vBSPACMledConfigure();

  setvbuf(stdout, NULL, _IONBF, 0);

  BSPACM_CORE_ENABLE_INTERRUPT();

  printf("\n" __DATE__ " " __TIME__ "\n");
  printf("System clock %lu Hz\n", SystemCoreClock);
  {
    int i = sizeof(xResetCause)/sizeof(*xResetCause);

    printf("Reset cause [%04x]:", reset_cause);
    while (0 <= --i) {
      const sResetCause * const rcp = xResetCause + i;
      if (rcp->value == (reset_cause & rcp->mask)) {
        printf(" %s", rcp->name);
      }
    }
    printf("\nRMU CTRL %lx\n", RMU->CTRL);
  }

  /* Enable low-energy support. */
  CMU_ClockEnable(cmuClock_CORELE, true);

  printf("RTC clock source: ");

#if (WITH_ULFRCO - 0)
  /* Use ULFRCO, which enables EM3 wakeup but is pretty inaccurate. */
  printf("ULFRCO\n");
  CMU_ClockSelectSet(cmuClock_LFA, cmuSelect_ULFRCO);
#elif (WITH_LFRCO - 0)
  printf("LFRCO/32\n");
  CMU_ClockSelectSet(cmuClock_LFA, cmuSelect_LFRCO);
  CMU_ClockDivSet(cmuClock_RTC, cmuClkDiv_32);
#else
  printf("LFXO/32\n");
  CMU_ClockSelectSet(cmuClock_LFA, cmuSelect_LFXO);
  CMU_ClockDivSet(cmuClock_RTC, cmuClkDiv_32);
#endif /* LFA source */

  /* Initialize RTC. */
  CMU_ClockEnable(cmuClock_RTC, true);
  rtc_Hz = CMU_ClockFreqGet(cmuClock_RTC);
  printf("RTC clock at %lu Hz ; wake every %u seconds\n", rtc_Hz, WAKE_INTERVAL_S);
  RTC->FREEZE = RTC_FREEZE_REGFREEZE;
  RTC->COMP0 = WAKE_INTERVAL_S * rtc_Hz;
  RTC->IFC = ~0U;
  RTC->IEN |= RTC_IEN_COMP0;
  RTC->CTRL = RTC_CTRL_COMP0TOP | RTC_CTRL_EN;
  RTC->FREEZE = 0;
  NVIC_EnableIRQ(RTC_IRQn);

  (void)iBSPACMperiphUARTflush(hBSPACMdefaultUART, eBSPACMperiphUARTfifoState_TX);
  while (1) {
    static const sBSPACMperiphUARTconfiguration cfg = { .speed_baud = 0 };
    printf("Sleeping in mode %u, rtc_if %x or %lx\n", sleep_mode, rtc_if, RTC->IF);
    BSPACM_CORE_DISABLE_INTERRUPT();
    do {
      (void)iBSPACMperiphUARTflush(hBSPACMdefaultUART, eBSPACMperiphUARTfifoState_TX);
      hBSPACMperiphUARTconfigure(hBSPACMdefaultUART, 0);
      switch (sleep_mode) {
        case 0:
          while (! (RTC_IF_COMP0 & RTC->IF)) {
          }
          ++sleep_mode;
          break;
        case 1:
          EMU_EnterEM1();
          ++sleep_mode;
          break;
        case 2:
          EMU_EnterEM2(true);

          SCB->SCR &= ~SCB_SCR_SLEEPDEEP_Msk;
          if (cmuSelect_ULFRCO == CMU_ClockSelectGet(cmuClock_LFA)) {
            ++sleep_mode;
          } else {
            sleep_mode = 0;
          }
          break;
        case 3:
          EMU_EnterEM3(true);
          SCB->SCR &= ~SCB_SCR_SLEEPDEEP_Msk;
          sleep_mode = 0;
          break;
      }
      hBSPACMperiphUARTconfigure(hBSPACMdefaultUART, &cfg);
    } while (0);
    BSPACM_CORE_ENABLE_INTERRUPT();
    /* Giant Gecko
     *  Source    EM0    EM1    EM2     EM3
     * LFXO/32   2.7m   1.2m   1.1u
     * LFRCO/32  2.7m   1.2m   1.0u
     * ULFRCO    2.8m   1.2m   590n    590n
     */

    /* Roughly 350 nA on STK3200 with ULFRCO EM3 */
  }

}
