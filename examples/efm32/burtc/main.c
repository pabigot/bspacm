/* BSPACM - efm32/burtc demonstration application
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
#include <em_burtc.h>
#include <string.h>
#include <stdio.h>

#ifndef WITH_ULFRCO
#define WITH_ULFRCO 1
#endif /* WITH_ULFRCO */

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

#define MAGIC 0x14235342
typedef struct sRetainedState {
  uint32_t magic;
  uint32_t boots;
} sRetainedState;
sRetainedState * const retained_state = (sRetainedState *)BURTC->RET;

volatile uint8_t burtc_if;
void BURTC_IRQHandler (void)
{
  burtc_if = BURTC->IF;
  BURTC->IFC = burtc_if;
}

void main ()
{
  uint32_t lfa_Hz;
  uint16_t reset_cause;
  int sleep_mode = 0;
  uint32_t burtc_ctrl;

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
  BURTC_Enable(true);

  if (MAGIC != retained_state->magic) {
    memset(retained_state, 0, sizeof(*retained_state));
    retained_state->magic = MAGIC;
    printf("Resetting retained state\n");
  }
  ++retained_state->boots;
  printf("Boot count %lu\n", retained_state->boots);

  printf("BURTC clock source: ");
#if (WITH_ULFRCO - 0)
  /* Use ULFRCO, which enables EM4 wakeup but is pretty inaccurate. */
  printf("ULFRCO\n");
  /* NB: DIV2 means 1 kHz instead of 2 kHz */
  burtc_ctrl = BURTC_CTRL_CLKSEL_ULFRCO | BURTC_CTRL_PRESC_DIV2;
  CMU_ClockSelectSet(cmuClock_LFA, cmuSelect_ULFRCO);
  lfa_Hz = CMU_ClockFreqGet(cmuClock_LFA);
  {
    EMU_EM4Init_TypeDef cfg = {
      .lockConfig = 1,
      .osc = EMU_EM4CONF_OSC_ULFRCO,
      .buRtcWakeup = 1,
      .vreg = 1,
    };
    EMU_EM4Init(&cfg);
  }
#elif (WITH_LFRCO - 0)
  printf("LFRCO/32\n");
  CMU_ClockSelectSet(cmuClock_LFA, cmuSelect_LFRCO);
  burtc_ctrl = BURTC_CTRL_CLKSEL_LFRCO | BURTC_CTRL_PRESC_DIV32;
  lfa_Hz = CMU_ClockFreqGet(cmuClock_LFA) / 32;
#else
  printf("LFXO/32\n");
  CMU_ClockSelectSet(cmuClock_LFA, cmuSelect_LFXO);
  burtc_ctrl = BURTC_CTRL_CLKSEL_LFXO | BURTC_CTRL_PRESC_DIV32;
  lfa_Hz = CMU_ClockFreqGet(cmuClock_LFA) / 32;
#endif /* LFA source */
  printf("LFA clock at %lu Hz ; wake every %u seconds\n", lfa_Hz, WAKE_INTERVAL_S);

  /* Initialize BURTC. */
  if (! (RMU_RSTCAUSE_EM4WURST & reset_cause)) {
    printf("Initializing BURTC\n");
    BURTC->FREEZE = BURTC_FREEZE_REGFREEZE;
    BURTC->LPMODE = BURTC_LPMODE_LPMODE_DISABLE;
    BURTC->CTRL = burtc_ctrl /* CLKSEL + PRESC */
      | BURTC_CTRL_RSTEN
      | BURTC_CTRL_MODE_EM4EN
      ;
    BURTC->COMP0 = WAKE_INTERVAL_S * lfa_Hz;
    BURTC->IEN = BURTC_IF_COMP0;
    BURTC->CTRL &= ~BURTC_CTRL_RSTEN;
    BURTC->FREEZE = 0;
  } else {
    while (BURTC_SYNCBUSY_COMP0 & BURTC->SYNCBUSY) {
    }
    BURTC->COMP0 += WAKE_INTERVAL_S * lfa_Hz;
  }
  BURTC->IFC = BURTC_IFC_COMP0;
  NVIC_EnableIRQ(BURTC_IRQn);

  printf("BURTC CTRL %lx IEN %lx\n", BURTC->CTRL, BURTC->IEN);
  (void)iBSPACMperiphUARTflush(hBSPACMdefaultUART, eBSPACMperiphUARTfifoState_TX);
  while (1) {
    static const sBSPACMperiphUARTconfiguration cfg = { .speed_baud = 0 };
    printf("Sleeping in mode %u, %lu to %lu rtc_if %x or %lx\n", sleep_mode, BURTC->CNT, BURTC->COMP0, burtc_if, BURTC->IF);
    BSPACM_CORE_DISABLE_INTERRUPT();
    do {
      (void)iBSPACMperiphUARTflush(hBSPACMdefaultUART, eBSPACMperiphUARTfifoState_TX);
      hBSPACMperiphUARTconfigure(hBSPACMdefaultUART, 0);
      switch (sleep_mode) {
        case 0:
          while (! (BURTC_IF_COMP0 & BURTC->IF)) {
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
          if (cmuSelect_ULFRCO == CMU_ClockSelectGet(cmuClock_LFA)) {
            ++sleep_mode;
          } else {
            sleep_mode = 0;
          }
          break;
        case 4:
          EMU_EnterEM4();
          sleep_mode = 0;
          break;
      }
    } while (0);
    BSPACM_CORE_ENABLE_INTERRUPT();
    hBSPACMperiphUARTconfigure(hBSPACMdefaultUART, &cfg);
    while (BURTC_SYNCBUSY_COMP0 & BURTC->SYNCBUSY) {
    }
    BURTC->COMP0 += WAKE_INTERVAL_S * lfa_Hz;
    /* Giant Gecko
     *  Source    EM0    EM1    EM2     EM3    EM4
     * ULFRCO    2.5m   1.1m   622n    622n   450n
     */
  }

}
