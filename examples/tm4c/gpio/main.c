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

#include <bspacm/utility/led.h>
#include <bspacm/newlib/ioctl.h>
#include <bspacm/periph/gpio.h>
#include <bspacm/periph/uart.h>
#include <inc/hw_timer.h>
#include <stdio.h>
#include <fcntl.h>

#define TIMER0_MODULE 0

typedef struct pin_type {
  const char * hdr;
  sBSPACMdeviceTM4Cpinmux mux;
} pin_type;

typedef struct pins_type {
  pin_type gpio_input;
  pin_type gpio_output;
  pin_type timer_capture;
  pin_type uart_transmit;
} pins_type;

const pins_type pins = {
  .gpio_input = { .hdr = "J1.9", .mux = { .port = GPIOA, .pin = 6 } },
  .gpio_output = { .hdr = "J1.10", .mux = { .port = GPIOA, .pin = 7 } },
  /* This is T0CCP0 */
  .timer_capture = { .hdr = "J2.7", .mux = { .port = GPIOB, .pin = 6, .pctl = 7 } },
  /* This is on UART5 */
  .uart_transmit = { .hdr = "J1.6", .mux = { .port = GPIOE, .pin = 5, .pctl = 1 } },
};

static void
describe_pin (const char * id,
              const pin_type * pp)
{
  printf("%s P%c%u(%u) on %s\n",
         id,
         iBSPACMdeviceTM4CgpioPortTagFromShift(iBSPACMdeviceTM4CgpioPortShift(pp->mux.port)),
         pp->mux.pin, pp->mux.pctl, pp->hdr);
}

void main ()
{
  hBSPACMperiphUART usp;
  const sBSPACMperiphUARTconfiguration uart_cfg = { .speed_baud = 0 };

  vBSPACMledConfigure();

  BSPACM_CORE_ENABLE_CYCCNT();
  BSPACM_CORE_ENABLE_INTERRUPT();

  printf("\n" __DATE__ " " __TIME__ "\n");
  printf("System clock %lu Hz\n", SystemCoreClock);
  describe_pin("GPIO input", &pins.gpio_input);
  describe_pin("GPIO output", &pins.gpio_output);
  describe_pin("Timer capture", &pins.timer_capture);
  describe_pin("UART transmit", &pins.uart_transmit);

  BSPACM_CORE_BITBAND_PERIPH(SYSCTL->RCGCGPIO, iBSPACMdeviceTM4CgpioPortShift(pins.gpio_input.mux.port)) = 1;
  BSPACM_CORE_BITBAND_PERIPH(SYSCTL->RCGCGPIO, iBSPACMdeviceTM4CgpioPortShift(pins.gpio_output.mux.port)) = 1;
  BSPACM_CORE_BITBAND_PERIPH(SYSCTL->RCGCGPIO, iBSPACMdeviceTM4CgpioPortShift(pins.timer_capture.mux.port)) = 1;
  BSPACM_CORE_BITBAND_PERIPH(SYSCTL->RCGCGPIO, iBSPACMdeviceTM4CgpioPortShift(pins.uart_transmit.mux.port)) = 1;

  BSPACM_CORE_BITBAND_PERIPH(SYSCTL->RCGCTIMER, TIMER0_MODULE) = 1;

  usp = hBSPACMperiphUARTconfigure(UART_HANDLE, &uart_cfg);
  printf("UART handle %p\n", usp);

#if 0
  {
    int in = -1;
    printf("Polling GPIO input for changes\n");
    /* Disable pinmux to configure as input. */
    vBSPACMdeviceTM4CpinmuxConfigure(&pins.gpio_input.mux, 0);
    while (1) {
      int new_in = BSPACM_CORE_BITBAND_PERIPH(pins.gpio_input.mux.port->DATA, pins.gpio_input.mux.pin);
      if (in != new_in) {
        in = new_in;
        printf("Read %s value %u\n", pins.gpio_input.hdr, in);
      }
    }
  }
#endif /* GPIO input */

#if 1
  {
    /* This test confirms that the GPIO interface DATA value reflects
     * the signal state even when the pin is configured for a capture
     * rather than as a GPIO input. */
    printf("Polling timer capture on %s for changes\n", pins.timer_capture.hdr);
    vBSPACMdeviceTM4CpinmuxConfigure(&pins.timer_capture.mux, 1);
    TIMER0->CFG = TIMER_CFG_16_BIT;
    TIMER0->TAILR = -1;
    TIMER0->TAPR = -1;
    TIMER0->IMR = 0;
    TIMER0->TAMR = TIMER_TAMR_TACMR | TIMER_TAMR_TAMR_CAP;
    TIMER0->CTL = TIMER_CTL_TAEVENT_BOTH | TIMER_CTL_TAEN;
    printf("State %u now %lx\n",
           !!BSPACM_CORE_BITBAND_PERIPH(pins.timer_capture.mux.port->DATA, pins.timer_capture.mux.pin),
           TIMER0->TAV);
    while (1) {
      unsigned int ris = TIMER0->RIS;
      TIMER0->ICR = ris;
      if (TIMER_RIS_CAERIS & ris) {
        int tar = TIMER0->TAR;
        int tav = TIMER0->TAV;
        int delta_ta = tar - tav;
        if (0 > delta_ta) {
          delta_ta += (1U << 24);
        }
        printf("State %u now %x captured at %x age %u ticks\n",
               !!BSPACM_CORE_BITBAND_PERIPH(pins.timer_capture.mux.port->DATA, pins.timer_capture.mux.pin),
               tav, tar, delta_ta);
      }
    }
  }
#endif /* Timer capture */

#if 0
  {
    int out = 0;
    printf("Toggling GPIO output at Hz/4\n");
    vBSPACMdeviceTM4CpinmuxConfigure(&pins.gpio_output.mux, 1);
    while (1) {
      printf("Set %s to %u\n", pins.gpio_output.hdr, out);
      BSPACM_CORE_BITBAND_PERIPH(pins.gpio_output.mux.port->DATA, pins.gpio_output.mux.pin) = out;
      out = ! out;
      BSPACM_CORE_DELAY_CYCLES(4 * SystemCoreClock);
    }
  }
#endif /* GPIO output */

#if 0
  {
    int out = 0;

    printf("Toggling UART transmit output at Hz/4\n");
    /* The result of this is that pin stays high at all times; */
    while (1) {
      printf("Set %s to %u as UART\n", pins.uart_transmit.hdr, out);
      BSPACM_CORE_BITBAND_PERIPH(pins.uart_transmit.mux.port->DATA, pins.uart_transmit.mux.pin) = out;
      out = ! out;
      BSPACM_CORE_DELAY_CYCLES(4 * SystemCoreClock);
    }
  }
#endif /* UART transmit */

  fflush(stdout);
  ioctl(1, BSPACM_IOCTL_FLUSH, O_WRONLY);
  BSPACM_CORE_DEEP_SLEEP();
}
