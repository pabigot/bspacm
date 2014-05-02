/* BSPACM - tm4c/dma demonstration application
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
#include <bspacm/utility/misc.h>
#include <inc/hw_udma.h>
#include <string.h>
#include <inttypes.h>
#include <stdio.h>
#include <fcntl.h>

/* Allocate 32 primary and alternative channels.  The channel table
 * must be aligned to a 1 KiBy boundary. */
BSPACM_CORE_ALIGNED_OBJECT(1024)
UDMA_CHANNEL_Type dma_channel_table[32 * 2];

#ifndef DMA_BUFFER_SIZE
#define DMA_BUFFER_SIZE 1024
#endif /* DMA_BUFFER_SIZE */
uint8_t src_buffer[DMA_BUFFER_SIZE];
uint8_t dst_buffer[DMA_BUFFER_SIZE];

void
UDMA_IRQHandler (void)
{
}

volatile unsigned int ndmaerr;

void
UDMAERR_IRQHandler (void)
{
  ++ndmaerr;
  BSPACM_CORE_BITBAND_PERIPH(UDMA->ERRCLR, 0) = 0;
}

#define DMA_CHANNEL_MM 30
#define DMA_SELECT_MM 4

void main ()
{
  int i;
  unsigned int t0;
  unsigned int t1;
  vBSPACMledConfigure();
  unsigned int const transfer_size_By = sizeof(src_buffer);
  unsigned int const display_size_By = 128;
  UDMA_CHANNEL_Type * swch = dma_channel_table + DMA_CHANNEL_MM;

  BSPACM_CORE_ENABLE_CYCCNT();
  BSPACM_CORE_ENABLE_INTERRUPT();

  printf("\n" __DATE__ " " __TIME__ "\n");
  printf("System clock %lu Hz\n", SystemCoreClock);

  printf("DMA transfers channel %u source %u\n", DMA_CHANNEL_MM, DMA_SELECT_MM);

  for (i = 0; i < sizeof(src_buffer); ++i) {
    src_buffer[i] = i;
  }
  printf("Source at %" PRIxPTR ":\n", (uintptr_t)src_buffer);
  vBSPACMconsoleDisplayMemoryOctets(src_buffer, display_size_By, 0);
  printf("Destination at %" PRIxPTR ":\n", (uintptr_t)dst_buffer);
  vBSPACMconsoleDisplayMemoryOctets(dst_buffer, display_size_By, (uintptr_t)dst_buffer);

  /* DMA available in active and sleep, but not in deep sleep */
  BSPACM_CORE_BITBAND_PERIPH(SYSCTL->RCGCDMA, 0) = 1;
  BSPACM_CORE_BITBAND_PERIPH(SYSCTL->SCGCDMA, 0) = 1;
  BSPACM_CORE_BITBAND_PERIPH(SYSCTL->DCGCDMA, 0) = 0;

  /* Enable DMA module and specify the control table */
  UDMA->CFG = UDMA_CFG_MASTEN;
  UDMA->CTLBASE = (uintptr_t)dma_channel_table;

  /* Specify the source selector for the channel that will be used*/
  vBSPACMcoreSetPinNybble(&UDMA->CHMAP0, DMA_CHANNEL_MM, DMA_SELECT_MM);

  /* Configure properties of transfer.  Allow single+burst, use
   * primary control structure, normal priority, clear request
   * flag, clear interrupt status. */
  BSPACM_CORE_BITBAND_PERIPH(UDMA->USEBURSTCLR, DMA_CHANNEL_MM) = 1;
  BSPACM_CORE_BITBAND_PERIPH(UDMA->ALTCLR, DMA_CHANNEL_MM) = 1;
  BSPACM_CORE_BITBAND_PERIPH(UDMA->PRIOCLR, DMA_CHANNEL_MM) = 1;
  BSPACM_CORE_BITBAND_PERIPH(UDMA->REQMASKCLR, DMA_CHANNEL_MM) = 1;
  BSPACM_CORE_BITBAND_PERIPH(UDMA->CHIS, DMA_CHANNEL_MM) = 1;

  /* Configure channel for byte-wise transfer of the entire src_buffer
   * to dst_buffer with one request, arbitrating every 64
   * transfers. */
  swch->srcendp = src_buffer + sizeof(src_buffer) - 1;
  swch->dstendp = dst_buffer + sizeof(dst_buffer) - 1;
  swch->chctl = 0
    | UDMA_CHCTL_DSTINC_8       /* Byte increment */
    | UDMA_CHCTL_DSTSIZE_8      /* Byte transfer */
    | UDMA_CHCTL_SRCINC_8       /* Byte increment */
    | UDMA_CHCTL_SRCSIZE_8      /* Byte transfer */
    | UDMA_CHCTL_ARBSIZE_64     /* Arbitrate at 64 transfers */
    | (UDMA_CHCTL_XFERSIZE_M & ((transfer_size_By - 1) << UDMA_CHCTL_XFERSIZE_S))
    | UDMA_CHCTL_XFERMODE_AUTO  /* One request to transfer them all */
    ;

  /* Enable DMA error interrupts */
  NVIC_ClearPendingIRQ(UDMAERR_IRQn);
  NVIC_EnableIRQ(UDMAERR_IRQn);

  /* Enable the channel */
  BSPACM_CORE_BITBAND_PERIPH(UDMA->ENASET, DMA_CHANNEL_MM) = 1;

  printf("UDMA STAT %lx CFG %lx ENA %lx CHIS %lx\n",
         UDMA->STAT, UDMA->CFG, UDMA->ENASET, UDMA->CHIS);
  printf("Transfer %u bytes: ctrl %" PRIx32 " from %" PRIxPTR " to %" PRIxPTR "\n",
         transfer_size_By,
         swch->chctl, (uintptr_t)swch->srcendp, (uintptr_t)swch->dstendp);
  fflush(stdout);
  ioctl(1, BSPACM_IOCTL_FLUSH, O_WRONLY);

  t0 = BSPACM_CORE_CYCCNT();
  /* Trigger the DMA and wait for completion */
  BSPACM_CORE_BITBAND_PERIPH(UDMA->SWREQ, DMA_CHANNEL_MM) = 1;
  while (! BSPACM_CORE_BITBAND_PERIPH(UDMA->CHIS, DMA_CHANNEL_MM)) {
  }
  t1 = BSPACM_CORE_CYCCNT();
  printf("Completed ctrl %" PRIx32 " from %" PRIxPTR " to %" PRIxPTR " in %u cycles\n",
         swch->chctl, (uintptr_t)swch->srcendp, (uintptr_t)swch->dstendp, t1-t0);
  printf("UDMA STAT %lx CFG %lx ENA %lx CHIS %lx\n",
         UDMA->STAT, UDMA->CFG, UDMA->ENASET, UDMA->CHIS);

  printf("Destination:\n");
  vBSPACMconsoleDisplayMemoryOctets(dst_buffer, display_size_By, (uintptr_t)dst_buffer);
  t0 = BSPACM_CORE_CYCCNT();
  memset(dst_buffer, 0, sizeof(dst_buffer));
  t1 = BSPACM_CORE_CYCCNT();
  printf("Clear destination took %u cycles\n", t1-t0);

  /* Same transfer but with 32-bit values and maximum delay before
   * arbitration (more than total transfer length) */
  swch->chctl = 0
    | UDMA_CHCTL_DSTINC_32      /* Word increment */
    | UDMA_CHCTL_DSTSIZE_32     /* Word transfer */
    | UDMA_CHCTL_SRCINC_32      /* Word increment */
    | UDMA_CHCTL_SRCSIZE_32     /* Word transfer */
    | UDMA_CHCTL_ARBSIZE_1024   /* Arbitrate at 1024 transfers */
    | (UDMA_CHCTL_XFERSIZE_M & (((transfer_size_By / 4) - 1) << UDMA_CHCTL_XFERSIZE_S))
    | UDMA_CHCTL_XFERMODE_AUTO  /* One request to transfer them all */
    ;
  BSPACM_CORE_BITBAND_PERIPH(UDMA->ENASET, DMA_CHANNEL_MM) = 1;
  BSPACM_CORE_BITBAND_PERIPH(UDMA->CHIS, DMA_CHANNEL_MM) = 1;

  printf("UDMA STAT %lx CFG %lx ENA %lx CHIS %lx\n",
         UDMA->STAT, UDMA->CFG, UDMA->ENASET, UDMA->CHIS);
  printf("Transfer %u words: ctrl %" PRIx32 " from %" PRIxPTR " to %" PRIxPTR "\n",
         transfer_size_By / 4,
         swch->chctl, (uintptr_t)swch->srcendp, (uintptr_t)swch->dstendp);

  fflush(stdout);
  ioctl(1, BSPACM_IOCTL_FLUSH, O_WRONLY);

  t0 = BSPACM_CORE_CYCCNT();
  BSPACM_CORE_BITBAND_PERIPH(UDMA->SWREQ, DMA_CHANNEL_MM) = 1;
  while (! BSPACM_CORE_BITBAND_PERIPH(UDMA->CHIS, DMA_CHANNEL_MM)) {
  }
  t1 = BSPACM_CORE_CYCCNT();

  printf("Word transfer took %u cycles\n", t1-t0);

  printf("Destination:\n");
  vBSPACMconsoleDisplayMemoryOctets(dst_buffer, display_size_By, (uintptr_t)dst_buffer);

  NVIC_DisableIRQ(UDMAERR_IRQn);
  printf("%u DMA errors\n", ndmaerr);
  fflush(stdout);
  ioctl(1, BSPACM_IOCTL_FLUSH, O_WRONLY);
  BSPACM_CORE_DEEP_SLEEP();
}
