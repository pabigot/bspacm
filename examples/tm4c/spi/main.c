/* BSPACM - tm4c/spi demonstration application
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

/* This is a prototype example for SPI interfacing.  Lack of proper
 * CMSIS headers from TI makes for extra work.  The example involves
 * interacting with an Anaren AIR C110L RF booster pack. */

#include <bspacm/utility/led.h>
#include <bspacm/periph/uart.h>
#include <bspacm/periph/gpio.h>
#include <bspacm/newlib/ioctl.h>
#include <bspacm/internal/utility/fifo.h>
#include <stdio.h>
#include <fcntl.h>

#define _SSI_SR_TFE_SHIFT 0
#define SSI_SR_TFE (1U << _SSI_SR_TFE_SHIFT)
#define _SSI_SR_TNF_SHIFT 1
#define SSI_SR_TNF (1U << _SSI_SR_TNF_SHIFT)
#define _SSI_SR_RNE_SHIFT 2
#define SSI_SR_RNE (1U << _SSI_SR_RNE_SHIFT)
#define _SSI_SR_RFF_SHIFT 3
#define SSI_SR_RFF (1U << _SSI_SR_RFF_SHIFT)
#define _SSI_SR_BSY_SHIFT 4
#define SSI_SR_BSY (1U << _SSI_SR_BSY_SHIFT)

#define _SSI_CR1_SSE_SHIFT 1
#define _SSI_CR0_SCR_SHIFT 8
#define _SSI_CR0_SPH_SHIFT 7
#define SSI_CR0_SPH (1U << _SSI_CR0_SPH_SHIFT)
#define _SSI_CR0_SPO_SHIFT 6
#define SSI_CR0_SPO (1U << _SSI_CR0_SPO_SHIFT)
#define SSI_CR0_FFS_SPI 0x00
#define SSI_CR0_DSS_8 0x07

SSI0_Type * spi;

#define CC_SSI_INSTANCE 2

static const sBSPACMdeviceTM4Cpinmux mosi_pinmux = {
  /* PB7 on D.6 as SSI2Tx#2 */
  .port = GPIOB, .pin = 7, .pctl = 2
};

static const sBSPACMdeviceTM4Cpinmux miso_pinmux = {
  /* PB6 on D.7 as SSI2Rx#2 ; also serves as GDO1 */
  .port = GPIOB, .pin = 6, .pctl = 2, .irqn = GPIOB_IRQn
};

static const sBSPACMdeviceTM4Cpinmux sclk_pinmux = {
  /* PB4 on A.7 as SSI2Clk#2 */
  .port = GPIOB, .pin = 4, .pctl = 2
};

#if (PWR_CONTROLLABLE - 0)
/* PWR hard-wired to 3V3 in boosterpack configuration */
static const sBSPACMdeviceTM4Cpinmux pwr_pinmux = {
  /* PB2 on D.2 as GPIO */
  .port = GPIOB, .pin = 2
};
#endif /* PWR_CONTROLLABLE */

static const sBSPACMdeviceTM4Cpinmux csn_pinmux = {
  /* PE0 on D.3 as GPIO */
  .port = GPIOE, .pin = 0
};

static const sBSPACMdeviceTM4Cpinmux gdo0_pinmux = {
  /* PB2 on D.2 as GPIO */
  .port = GPIOB, .pin = 2, .irqn = GPIOB_IRQn
};

static const sBSPACMdeviceTM4Cpinmux gdo2_pinmux = {
  /* PB5 on A.2 as GPIO */
  .port = GPIOB, .pin = 5, .irqn = GPIOB_IRQn
};

int
spi_tx_rx (SSI0_Type * spi,
           const uint8_t * txp,
           int tx_bytes,
           int rx_bytes,
           uint8_t * rxp)
{
  const uint8_t * const txpe = txp + tx_bytes;
  tx_bytes += rx_bytes;
  rx_bytes = tx_bytes;

  /* SSI has 8-deep 16-bit FIFOs for both TX and RX.  Keep the TX FIFO
   * as full as possible, and the RX FIFO as empty as possible, until
   * the transaction is complete.  */
  while (0 < rx_bytes) {
    while ((0 < tx_bytes) && (SSI_SR_TNF & spi->SR)) {
      spi->DR = (txp < txpe) ? *txp++ : 0;
      --tx_bytes;
    }
    while ((0 < rx_bytes) && (SSI_SR_RNE & spi->SR)) {
      unsigned int rxdatax = spi->DR;
      if (rxp) {
        *rxp++ = rxdatax;
      }
      --rx_bytes;
    }
  }
  return 0;
}

static uint8_t
sendStrobe (uint8_t reg)
{
  uint8_t rc = 0;

  (void)spi_tx_rx(spi, &reg, 1, 0, &rc);
  return rc;
}

static uint8_t
readRegister (uint8_t reg)
{
  uint8_t rxbuf[2];

  /* If this is a status register add the BURST bit */
  if (0x30 <= reg) {
    reg |= 0x40;
  }
  /* Add the READ bit */
  reg |= 0x80;
  (void)spi_tx_rx(spi, &reg, 1, 1, rxbuf);
  return rxbuf[1];
}

static int
writeRegister (uint8_t reg,
               uint8_t val)
{
  uint8_t txbuf[2];
  uint8_t rxbuf[2];

  txbuf[0] = reg;
  txbuf[1] = val;
  (void)spi_tx_rx(spi, txbuf, 2, 0, rxbuf);
  return rxbuf[1];
}

void dump_port (GPIOCommon_Type * gpio)
{
  int shift = iBSPACMdeviceTM4CgpioPortShift(gpio);
  int tag = iBSPACMdeviceTM4CgpioPortTagFromShift(shift);

  printf("GPIO%c: CR %lx DIR %lx AFSEL %lx PCTL %lx PUR %lx PDR %lx DEN %lx\n",
         tag,
         gpio->CR, gpio->DIR, gpio->AFSEL, gpio->PCTL,
         gpio->PUR, gpio->PDR, gpio->DEN);
}

void main ()
{
#if (PWR_CONTROLLABLE - 0)
  volatile uint32_t * const pwr_bitband = &BSPACM_CORE_BITBAND_PERIPH(pwr_pinmux.port->DATA, pwr_pinmux.pin);
#endif /* PWR_CONTROLLABLE */
  volatile uint32_t * const csn_bitband = &BSPACM_CORE_BITBAND_PERIPH(csn_pinmux.port->DATA, csn_pinmux.pin);
  volatile uint32_t * const gdo0_bitband = &BSPACM_CORE_BITBAND_PERIPH(gdo0_pinmux.port->DATA, gdo0_pinmux.pin);
  volatile uint32_t * const gdo1_afsel_bitband = &BSPACM_CORE_BITBAND_PERIPH(miso_pinmux.port->AFSEL, miso_pinmux.pin);
  volatile uint32_t * const gdo1_bitband = &BSPACM_CORE_BITBAND_PERIPH(miso_pinmux.port->DATA, miso_pinmux.pin);
  volatile uint32_t * const gdo2_bitband = &BSPACM_CORE_BITBAND_PERIPH(gdo2_pinmux.port->DATA, gdo2_pinmux.pin);
  int rv;
  uint8_t rc = 0;
  (void)rv;

  vBSPACMledConfigure();
  SystemCoreClockUpdate();
  BSPACM_CORE_ENABLE_INTERRUPT();
  BSPACM_CORE_ENABLE_CYCCNT();

  printf("\n" __DATE__ " " __TIME__ "\n");
  printf("System clock %lu Hz\n", SystemCoreClock);
  printf("CSn at P%c%u\n",
         iBSPACMdeviceTM4CgpioPortTagFromShift(iBSPACMdeviceTM4CgpioPortShift(csn_pinmux.port)),
         csn_pinmux.pin);

  BSPACM_CORE_BITBAND_PERIPH(SYSCTL->RCGCGPIO, iBSPACMdeviceTM4CgpioPortShift(GPIOA)) = 1;
  BSPACM_CORE_BITBAND_PERIPH(SYSCTL->RCGCGPIO, iBSPACMdeviceTM4CgpioPortShift(GPIOB)) = 1;
  BSPACM_CORE_BITBAND_PERIPH(SYSCTL->RCGCGPIO, iBSPACMdeviceTM4CgpioPortShift(GPIOE)) = 1;
  __NOP(); __NOP(); __NOP(); /* delay 3 cycles */
#if (PWR_CONTROLLABLE - 0)
  BSPACM_CORE_BITBAND_PERIPH(pwr_pinmux.port->DIR, pwr_pinmux.pin) = 1;
  BSPACM_CORE_BITBAND_PERIPH(pwr_pinmux.port->DEN, pwr_pinmux.pin) = 1;
  *pwr_bitband = 0;
#endif /* PWR_CONTROLLABLE */
  BSPACM_CORE_BITBAND_PERIPH(csn_pinmux.port->DIR, csn_pinmux.pin) = 1;
  BSPACM_CORE_BITBAND_PERIPH(csn_pinmux.port->DEN, csn_pinmux.pin) = 1;
  *csn_bitband = 1;

  spi = SSI2;
  BSPACM_CORE_BITBAND_PERIPH(SYSCTL->RCGCSSI, CC_SSI_INSTANCE) = 1;
  vBSPACMdeviceTM4CpinmuxConfigure(&mosi_pinmux, 1, 0);
  vBSPACMdeviceTM4CpinmuxConfigure(&miso_pinmux, 1, 0);
  vBSPACMdeviceTM4CpinmuxConfigure(&sclk_pinmux, 1, 0);
  dump_port(GPIOB);
  printf("Device up\n");

  BSPACM_CORE_BITBAND_PERIPH(SSI2->CR1, _SSI_CR1_SSE_SHIFT) = 0;
  SSI2->CR1 = 0;
  SSI2->CPSR = 2;
  SSI2->CR0 = ((8 - 1) << _SSI_CR0_SCR_SHIFT) | SSI_CR0_DSS_8;
  BSPACM_CORE_BITBAND_PERIPH(SSI2->CR1, _SSI_CR1_SSE_SHIFT) = 1;
  printf("SSI CR0 %lx CR1 %lx CPSR %lx SR %lx\n",
         SSI2->CR0, SSI2->CR1, SSI2->CPSR, SSI2->SR);

#if (PWR_CONTROLLABLE - 0)
  *pwr_bitband = 1;
#endif /* PWR_CONTROLLABLE */
  /* Switch back to GPIO to detect ready signal after power-up completes */
  *gdo1_afsel_bitband = 0;
  *csn_bitband = 0;
  do {
    printf("GDO1 %ld\n", *gdo1_bitband);
  } while (*gdo1_bitband);
  *gdo1_afsel_bitband = 1;
  printf("GPIOEDATA %lx bit %lx\n", GPIOE->DATA, *csn_bitband);

  do {
    rc = sendStrobe(0x30);
    printf("Reset got %x\n", rc);
  } while (0x0f != rc);
  printf("PARTNUM response %#02x\n", readRegister(0x30));
  printf("VERSION response %#02x\n", readRegister(0x31));
  printf("IOCFG2 read %#02x\n", readRegister(0x00));
  printf("IOCFG1 read %#02x\n", readRegister(0x01));
  printf("IOCFG0 read %#02x\n", readRegister(0x02));

  /* ChipCon radios consume 1.4mA when idle.  That goes down to
   * nominally 400 nA if the GDOs are configured to "HW to 0" and the
   * chip is told to power-down on loss of CSn.  On the EXP430F5438
   * the RF PWR header indicates that a CC1101 is using 40 nA in this
   * mode.*/
  rc = writeRegister(0x00, 0x2f);
  rc = writeRegister(0x01, 0x2f);
  rc = writeRegister(0x02, 0x2f);
  printf("Cleared IOCFG\n");
  printf("IOCFG2 read %#02x\n", readRegister(0x00));
  printf("IOCFG1 read %#02x\n", readRegister(0x01));
  printf("IOCFG0 read %#02x\n", readRegister(0x02));

  /* SPWD */
  rc = sendStrobe(0x39);
  *csn_bitband = 1;
  printf("SPWD got %d\n", rc);

  fflush(stdout);
  ioctl(1, BSPACM_IOCTL_FLUSH, O_WRONLY);
  (void)gdo0_bitband;
  (void)gdo2_bitband;
}
