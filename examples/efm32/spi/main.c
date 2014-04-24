/* BSPACM - efm32/spi demonstration application
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

/* Just a proof-of-concept grossly reconfiguring a USART to SPI mode
 * to interact with an Anaren AIR CC110L boosterpack.
 *
 * Using USART1 for SPI, so UART0 is used for console through standard
 * BSPACM mechanisms.
 *
 * BP  Color   Function    STK3700 Ext
 * A.1 red     3V3         12 (PD4)
 * A.2 brn     GDO2        n/c
 * A.7 yel     SCLK        8
 * B.1 blk     GND         19
 * B.2 grn     GDO0        n/c
 * B.3 oran    CSn         10
 * B.6 whi     MOSI        4
 * B.7 purp    MISO/GDO1   6 */

#include <bspacm/utility/led.h>
#include <bspacm/periph/uart.h>
#include <bspacm/periph/gpio.h>
#include <bspacm/newlib/ioctl.h>
#include <bspacm/internal/utility/fifo.h>
#include <em_cmu.h>
#include <em_usart.h>
#include <em_gpio.h>
#include <stdio.h>
#include <fcntl.h>

static const sBSPACMdeviceEFM32pinmux pwr_pinmux = {
  /* PD4 on expansion header 12 */
  .port = GPIO->P + gpioPortD, .pin = 4, .mode = gpioModePushPull
};
static const sBSPACMdeviceEFM32pinmux csn_pinmux = {
  /* PD5 on expansion header 14 */
  .port = GPIO->P + gpioPortD, .pin = 5, .mode = gpioModePushPull
};

hBSPACMperiphUART spi = &xBSPACMdeviceEFM32periphUSART1;

static
hBSPACMperiphUART
spi_configure (sBSPACMperiphUARTstate * usp,
               const sBSPACMperiphUARTconfiguration * cfgp)
{
  USART_TypeDef * usart;
  const sBSPACMdeviceEFM32periphUSARTdevcfg * devcfgp;

  if (! (usp && usp->uart)) {
    return NULL;
  }
  usart = (USART_TypeDef *)usp->uart;
  devcfgp = (const sBSPACMdeviceEFM32periphUSARTdevcfg *)usp->devcfg.ptr;

  /* If enabling configuration, enable the high-frequency peripheral
   * clock and the clock for the uart itself.
   *
   * If disabling configuration, disable the interrupts. */
  if (cfgp) {
    CMU_ClockEnable(cmuClock_HFPER, true);
    CMU_ClockEnable(devcfgp->uart.common.clock, true);
  } else {
    NVIC_DisableIRQ(devcfgp->uart.rx_irqn);
    NVIC_DisableIRQ(devcfgp->uart.tx_irqn);
    NVIC_ClearPendingIRQ(devcfgp->uart.rx_irqn);
    NVIC_ClearPendingIRQ(devcfgp->uart.tx_irqn);
  }
  USART_Reset(usart);
  if (usp->rx_fifo_ni_) {
    fifo_reset(usp->rx_fifo_ni_);
  }
  if (usp->tx_fifo_ni_) {
    fifo_reset(usp->tx_fifo_ni_);
  }
  usp->tx_state_ = 0;

  if (cfgp) {
    /* Setting baudrate */
    usart->CLKDIV = 128 * (SystemCoreClock / 1000000UL - 2);

    /* Configure USART */
    /* Using synchronous (USART) mode, MSB first */
    usart->CTRL = USART_CTRL_SYNC | USART_CTRL_MSBF;
    // NOT AUTOCS
    // usart->CTRL |= USART_CTRL_AUTOCS
    /* Clearing old transfers/receptions, and disabling interrupts */
    usart->CMD = USART_CMD_CLEARRX | USART_CMD_CLEARTX;
    usart->IEN = 0;

    /* Enabling Master, TX and RX */
    CMU_ClockEnable(cmuClock_GPIO, true);
  } else {
    /* Done with device; turn it off */
    CMU_ClockEnable(devcfgp->uart.common.clock, false);
  }

  /* Enable or disable UART pins. To avoid false start, when enabling
   * configure TX as high.  This relies on a comment in the EMLIB code
   * that manipulating registers of disabled modules has no effect
   * (unlike TM4C where it causes a HardFault).  We'll see. */
  vBSPACMdeviceEFM32pinmuxConfigure(&devcfgp->uart.common.rx_pinmux, !!cfgp, 0);
  vBSPACMdeviceEFM32pinmuxConfigure(&devcfgp->uart.common.tx_pinmux, !!cfgp, 0);
  vBSPACMdeviceEFM32pinmuxConfigure(&devcfgp->clk_pinmux, !!cfgp, 0);
  vBSPACMdeviceEFM32pinmuxConfigure(&devcfgp->cs_pinmux, !!cfgp, 1);

  if (cfgp) {
    /* Enabling pins and setting location */
    usart->ROUTE = USART_ROUTE_TXPEN | USART_ROUTE_RXPEN | USART_ROUTE_CLKPEN | USART_ROUTE_CSPEN | devcfgp->uart.common.location;

    /* Clear and enable RX interrupts.  TX interrupts are enabled at the
     * peripheral when there's something to transmit.  TX and RX are
     * enabled at the NVIC now. */
    usart->IFC = _USART_IF_MASK;
    //usart->IEN = USART_IF_RXDATAV;
    NVIC_ClearPendingIRQ(devcfgp->uart.rx_irqn);
    NVIC_ClearPendingIRQ(devcfgp->uart.tx_irqn);
    NVIC_EnableIRQ(devcfgp->uart.rx_irqn);
    NVIC_EnableIRQ(devcfgp->uart.tx_irqn);

    /* Configuration complete; enable the USART */
    usart->CMD = USART_CMD_MASTEREN | USART_CMD_TXEN | USART_CMD_RXEN;
  }

  return usp;
}

int
spi_tx_rx (hBSPACMperiphUART spi,
           const uint8_t * txp,
           int tx_bytes,
           int rx_bytes,
           uint8_t * rxp)
{
  USART_TypeDef * usart = (USART_TypeDef *)spi->uart;
  const uint8_t * const txpe = txp + tx_bytes;
  tx_bytes += rx_bytes;
  rx_bytes = tx_bytes;

  while (0 < rx_bytes) {
    if ((0 < tx_bytes) && (USART_STATUS_TXBL & usart->STATUS)) {
      usart->TXDATA = (txp < txpe) ? *txp++ : 0;
      --tx_bytes;
    }
    if (USART_STATUS_RXDATAV & usart->STATUS) {
      unsigned int rxdatax = usart->RXDATAX;
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

void main ()
{
  const unsigned int pwr_mask = (1U << pwr_pinmux.pin);
  const unsigned int csn_mask = (1U << csn_pinmux.pin);
  const sBSPACMdeviceEFM32periphUSARTdevcfg * spicfgp = (const sBSPACMdeviceEFM32periphUSARTdevcfg *)spi->devcfg.ptr;
  const sBSPACMdeviceEFM32pinmux * const gdo1_pinmuxp = &spicfgp->uart.common.rx_pinmux;
  GPIO_P_TypeDef * const pwr_port = pwr_pinmux.port;
  GPIO_P_TypeDef * const csn_port = csn_pinmux.port;
  GPIO_P_TypeDef * const gdo1_port = gdo1_pinmuxp->port;
  const unsigned int gdo1_mask = (1U << gdo1_pinmuxp->pin);
  USART_TypeDef * usart = (USART_TypeDef *)spi->uart;
  const sBSPACMperiphUARTconfiguration cfg = { .speed_baud = 0 };
  int rv;
  uint8_t rc = 0;
  (void)rv;

  vBSPACMledConfigure();
  SystemCoreClockUpdate();
  BSPACM_CORE_ENABLE_INTERRUPT();
  BSPACM_CORE_ENABLE_CYCCNT();

  printf("\n" __DATE__ " " __TIME__ "\n");
  printf("System clock %lu Hz\n", SystemCoreClock);
  spi = spi_configure(spi, &cfg);
  csn_port->DOUTSET = csn_mask;
  printf("SPI at %p usart at %p\n", spi, usart);
  printf("CTRL %lx\n", usart->CTRL);
  vBSPACMdeviceEFM32pinmuxConfigure(&pwr_pinmux, 1, 0);
  vBSPACMdeviceEFM32pinmuxConfigure(&csn_pinmux, 1, 1);

  printf("GDO1 %d\n", !!(gdo1_port->DIN & gdo1_mask));
  pwr_port->DOUTSET = pwr_mask;
  csn_port->DOUTCLR = csn_mask;
  while (! (gdo1_mask & gdo1_port->DIN)) {
  }

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
  csn_port->DOUTSET = csn_mask;
  printf("SPWD got %d\n", rc);

  fflush(stdout);
  ioctl(1, BSPACM_IOCTL_FLUSH, O_WRONLY);
}
