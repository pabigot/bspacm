/* Vector table declaration for Energy Micro (SiLabs) EFM32ZG Zero Gecko microcontrollers. */
/* NMI_Handler 2 */
/* HardFault_Handler 3 */
/* MemManage_Handler 4 */
/* BusFault_Handler 5 */
/* UsageFault_Handler 6 */
/* Reserved 7 */
/* Reserved 8 */
/* Reserved 9 */
/* Reserved 10 */
/* SVC_Handler 11 */
/* DebugMon_Handler 12 */
/* Reserved 13 */
/* PendSV_Handler 14 */
/* SysTick_Handler 15 */
BSPACM_INC_EXPAND_IRQ_REFERENCE(DMA)           /* 16 = 16+0 EFM32 DMA Interrupt */
BSPACM_INC_EXPAND_IRQ_REFERENCE(GPIO_EVEN)     /* 17 = 16+1 EFM32 GPIO_EVEN Interrupt */
BSPACM_INC_EXPAND_IRQ_REFERENCE(TIMER0)        /* 18 = 16+2 EFM32 TIMER0 Interrupt */
BSPACM_INC_EXPAND_IRQ_REFERENCE(ACMP0)         /* 19 = 16+3 EFM32 ACMP0 Interrupt */
BSPACM_INC_EXPAND_IRQ_REFERENCE(ADC0)          /* 20 = 16+4 EFM32 ADC0 Interrupt */
BSPACM_INC_EXPAND_IRQ_REFERENCE(I2C0)          /* 21 = 16+5 EFM32 I2C0 Interrupt */
BSPACM_INC_EXPAND_IRQ_REFERENCE(GPIO_ODD)      /* 22 = 16+6 EFM32 GPIO_ODD Interrupt */
BSPACM_INC_EXPAND_IRQ_REFERENCE(TIMER1)        /* 23 = 16+7 EFM32 TIMER1 Interrupt */
BSPACM_INC_EXPAND_IRQ_REFERENCE(USART1_RX)     /* 24 = 16+8 EFM32 USART1_RX Interrupt */
BSPACM_INC_EXPAND_IRQ_REFERENCE(USART1_TX)     /* 25 = 16+9 EFM32 USART1_TX Interrupt */
BSPACM_INC_EXPAND_IRQ_REFERENCE(LEUART0)       /* 26 = 16+10 EFM32 LEUART0 Interrupt */
BSPACM_INC_EXPAND_IRQ_REFERENCE(PCNT0)         /* 27 = 16+11 EFM32 PCNT0 Interrupt */
BSPACM_INC_EXPAND_IRQ_REFERENCE(RTC)           /* 28 = 16+12 EFM32 RTC Interrupt */
BSPACM_INC_EXPAND_IRQ_REFERENCE(CMU)           /* 29 = 16+13 EFM32 CMU Interrupt */
BSPACM_INC_EXPAND_IRQ_REFERENCE(VCMP)          /* 30 = 16+14 EFM32 VCMP Interrupt */
BSPACM_INC_EXPAND_IRQ_REFERENCE(MSC)           /* 31 = 16+15 EFM32 MSC Interrupt */
BSPACM_INC_EXPAND_IRQ_REFERENCE(AES)           /* 32 = 16+16 EFM32 AES Interrupt */

/*
 * Local Variables:
 * mode:c
 * End:
 */
