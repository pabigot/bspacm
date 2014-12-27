/* BSPACM - nRF51 timer stuff
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
#include <stdio.h>
#include <fcntl.h>

volatile unsigned int irqs;
volatile bool do_wake;
void TIMER0_IRQHandler ()
{
  NRF_TIMER0->EVENTS_COMPARE[0] = 0;
  if (0 == (0x03 & ++irqs)) {
    do_wake = true;
  }
  vBSPACMledSet(irqs & 3, -1);
}

void main ()
{
  vBSPACMledConfigure();
  BSPACM_CORE_ENABLE_INTERRUPT();

  printf("\n" __DATE__ " " __TIME__ "\n");
  printf("System clock %lu Hz\n", SystemCoreClock);

  printf("Initial stat: HF %08lx LF %08lx src %lu\n",
         NRF_CLOCK->HFCLKSTAT, NRF_CLOCK->LFCLKSTAT, NRF_CLOCK->LFCLKSRC);

  /* HFCLK starts as the RC oscillator.  Start the crystal for HFCLK,
   * and start one for LFCLK. */
  NRF_CLOCK->EVENTS_HFCLKSTARTED = 0;
  NRF_CLOCK->EVENTS_LFCLKSTARTED = 0;
  NRF_CLOCK->LFCLKSRC = (CLOCK_LFCLKSTAT_SRC_Xtal << CLOCK_LFCLKSTAT_SRC_Pos);
  NRF_CLOCK->TASKS_HFCLKSTART = 1;
  NRF_CLOCK->TASKS_LFCLKSTART = 1;
  while (! (NRF_CLOCK->EVENTS_HFCLKSTARTED && NRF_CLOCK->EVENTS_LFCLKSTARTED)) {
  }

  printf("Post start stat: HF %08lx LF %08lx src %lu\n",
         NRF_CLOCK->HFCLKSTAT, NRF_CLOCK->LFCLKSTAT, NRF_CLOCK->LFCLKSRC);

  /* Timers only work on HFCLK.  Set one up using a zero prescaler
   * (runs at HFCLK), to wake at 1 Hz. */

  /* 24-bit compare in timer mode using undivided HFCLK */
  NRF_TIMER0->MODE = (TIMER_MODE_MODE_Timer << TIMER_MODE_MODE_Pos);
  NRF_TIMER0->PRESCALER = 0;
  NRF_TIMER0->BITMODE = (TIMER_BITMODE_BITMODE_24Bit << TIMER_BITMODE_BITMODE_Pos);

  /* Clear previous events, reset counter, raise event at 1Hz */
  NRF_TIMER0->EVENTS_COMPARE[0] = 0;
  NRF_TIMER0->TASKS_CLEAR = 1;
  NRF_TIMER0->CC[0] = SystemCoreClock;

  /* Issue a CLEAR task when event COMPARE[0] occurs.
   *
   * This eliminates the addition of a wakeup delay to the inter-event
   * interval. */
  NRF_TIMER0->SHORTS = (TIMER_SHORTS_COMPARE0_CLEAR_Enabled << TIMER_SHORTS_COMPARE0_CLEAR_Pos);

  /* Clear peripheral pending interrupts and enable interrupt on
   * COMPARE0.  Then clear NVIC interrupts and enable interrupt from
   * TIMER0. */
  NRF_TIMER0->INTENCLR = ~0;
  NRF_TIMER0->INTENSET = (TIMER_INTENSET_COMPARE0_Enabled << TIMER_INTENSET_COMPARE0_Pos);
  NVIC_ClearPendingIRQ(TIMER0_IRQn);
  NVIC_EnableIRQ(TIMER0_IRQn);

  /* Start the whole thing off. */
  NRF_TIMER0->TASKS_START = 1;
  printf("PRIMASK %lx SCB %lx\n", __get_PRIMASK(), SCB->SCR);

  while (1) {
    /* Select your delay solution */
#if 1
    /* Check the condition then block if it's false and no events have
     * occurred since the last time we executed the loop body (which
     * was before we checked the condition.  This is safe when
     * interrupts remain enabled. */
    while (! do_wake) {
      __WFE();
    }
#elif 0
    /* Similar to WFI.  The first WFE clears the __SEV() used to
     * ensure that the first WFE doesn't block; the second WFE
     * blocks until an event occurs. */
    __SEV();
    __WFE();
    __WFE();
#elif 0
    /* Block until an event */
    __WFI();
#else
    BSPACM_CORE_SAVED_INTERRUPT_STATE(istate);

    /* Old MSP430 way where we have to disable interrupts to check the
     * condition without a window where it might become true after
     * we've committed to waiting for it. */
    BSPACM_CORE_DISABLE_INTERRUPT();
    if (! do_wake) {
      __WFI();
    }
    /* Unlike MSP430 interrupts remain disabled here regardless of
     * which conditional branch was taken, which simplifies some
     * logic. */
    BSPACM_CORE_REENABLE_INTERRUPT(istate);
#endif

    printf("Wokeup %u PRIMASK %lx SCB %lx\n", irqs, __get_PRIMASK(), SCB->SCR);
    do_wake = false;
  }

  fflush(stdout);
  ioctl(1, BSPACM_IOCTL_FLUSH, O_WRONLY);
  BSPACM_CORE_DEEP_SLEEP();
}
