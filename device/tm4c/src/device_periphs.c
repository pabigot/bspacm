/* Copyright 2014, Peter A. Bigot
 *
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 * * Redistributions of source code must retain the above copyright notice,
 *   this list of conditions and the following disclaimer.
 *
 * * Redistributions in binary form must reproduce the above copyright notice,
 *   this list of conditions and the following disclaimer in the documentation
 *   and/or other materials provided with the distribution.
 *
 * * Neither the name of the software nor the names of its contributors may be
 *   used to endorse or promote products derived from this software without
 *   specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 */

#include <bspacm/core.h>
#include <bspacm/device/periphs.h>

/* Some devices provide only the Advanced High-Performance Bus base
 * name, sometimes only for some of the GPIOs.  Make things
 * consistent. */
#if defined(GPIOA_AHB_BASE) && ! defined(GPIOA_BASE)
#define GPIOA_BASE GPIOA_AHB_BASE
#endif /* AHB alias to GPIOA */
#if defined(GPIOB_AHB_BASE) && ! defined(GPIOB_BASE)
#define GPIOB_BASE GPIOB_AHB_BASE
#endif /* AHB alias to GPIOB */
#if defined(GPIOC_AHB_BASE) && ! defined(GPIOC_BASE)
#define GPIOC_BASE GPIOC_AHB_BASE
#endif /* AHB alias to GPIOC */
#if defined(GPIOD_AHB_BASE) && ! defined(GPIOD_BASE)
#define GPIOD_BASE GPIOD_AHB_BASE
#endif /* AHB alias to GPIOD */
#if defined(GPIOE_AHB_BASE) && ! defined(GPIOE_BASE)
#define GPIOE_BASE GPIOE_AHB_BASE
#endif /* AHB alias to GPIOE */
#if defined(GPIOF_AHB_BASE) && ! defined(GPIOF_BASE)
#define GPIOF_BASE GPIOF_AHB_BASE
#endif /* AHB alias to GPIOF */
#if defined(GPIOG_AHB_BASE) && ! defined(GPIOG_BASE)
#define GPIOG_BASE GPIOG_AHB_BASE
#endif /* AHB alias to GPIOG */
#if defined(GPIOH_AHB_BASE) && ! defined(GPIOH_BASE)
#define GPIOH_BASE GPIOH_AHB_BASE
#endif /* AHB alias to GPIOH */
#if defined(GPIOJ_AHB_BASE) && ! defined(GPIOJ_BASE)
#define GPIOJ_BASE GPIOJ_AHB_BASE
#endif /* AHB alias to GPIOJ */
#if defined(GPIOK_AHB_BASE) && ! defined(GPIOK_BASE)
#define GPIOK_BASE GPIOK_AHB_BASE
#endif /* AHB alias to GPIOK */
#if defined(GPIOL_AHB_BASE) && ! defined(GPIOL_BASE)
#define GPIOL_BASE GPIOL_AHB_BASE
#endif /* AHB alias to GPIOL */
#if defined(GPIOM_AHB_BASE) && ! defined(GPIOM_BASE)
#define GPIOM_BASE GPIOM_AHB_BASE
#endif /* AHB alias to GPIOM */
#if defined(GPION_AHB_BASE) && ! defined(GPION_BASE)
#define GPION_BASE GPION_AHB_BASE
#endif /* AHB alias to GPION */
#if defined(GPIOP_AHB_BASE) && ! defined(GPIOP_BASE)
#define GPIOP_BASE GPIOP_AHB_BASE
#endif /* AHB alias to GPIOP */
#if defined(GPIOQ_AHB_BASE) && ! defined(GPIOQ_BASE)
#define GPIOQ_BASE GPIOQ_AHB_BASE
#endif /* AHB alias to GPIOQ */
#if defined(GPIOR_AHB_BASE) && ! defined(GPIOR_BASE)
#define GPIOR_BASE GPIOR_AHB_BASE
#endif /* AHB alias to GPIOR */
#if defined(GPIOS_AHB_BASE) && ! defined(GPIOS_BASE)
#define GPIOS_BASE GPIOS_AHB_BASE
#endif /* AHB alias to GPIOS */
#if defined(GPIOT_AHB_BASE) && ! defined(GPIOT_BASE)
#define GPIOT_BASE GPIOT_AHB_BASE
#endif /* AHB alias to GPIOT */
#define GPIO_MAX_IDX -1
#define GPIOA_IDX 0
#if defined(GPIOA_BASE)
#undef GPIO_MAX_IDX
#define GPIO_MAX_IDX GPIOA_IDX
#endif /* GPIOA_BASE */
#define GPIOB_IDX 1
#if defined(GPIOB_BASE)
#undef GPIO_MAX_IDX
#define GPIO_MAX_IDX GPIOB_IDX
#endif /* GPIOB_BASE */
#define GPIOC_IDX 2
#if defined(GPIOC_BASE)
#undef GPIO_MAX_IDX
#define GPIO_MAX_IDX GPIOC_IDX
#endif /* GPIOC_BASE */
#define GPIOD_IDX 3
#if defined(GPIOD_BASE)
#undef GPIO_MAX_IDX
#define GPIO_MAX_IDX GPIOD_IDX
#endif /* GPIOD_BASE */
#define GPIOE_IDX 4
#if defined(GPIOE_BASE)
#undef GPIO_MAX_IDX
#define GPIO_MAX_IDX GPIOE_IDX
#endif /* GPIOE_BASE */
#define GPIOF_IDX 5
#if defined(GPIOF_BASE)
#undef GPIO_MAX_IDX
#define GPIO_MAX_IDX GPIOF_IDX
#endif /* GPIOF_BASE */
#define GPIOG_IDX 6
#if defined(GPIOG_BASE)
#undef GPIO_MAX_IDX
#define GPIO_MAX_IDX GPIOG_IDX
#endif /* GPIOG_BASE */
#define GPIOH_IDX 7
#if defined(GPIOH_BASE)
#undef GPIO_MAX_IDX
#define GPIO_MAX_IDX GPIOH_IDX
#endif /* GPIOH_BASE */
#define GPIOJ_IDX 8
#if defined(GPIOJ_BASE)
#undef GPIO_MAX_IDX
#define GPIO_MAX_IDX GPIOJ_IDX
#endif /* GPIOJ_BASE */
#define GPIOK_IDX 9
#if defined(GPIOK_BASE)
#undef GPIO_MAX_IDX
#define GPIO_MAX_IDX GPIOK_IDX
#endif /* GPIOK_BASE */
#define GPIOL_IDX 10
#if defined(GPIOL_BASE)
#undef GPIO_MAX_IDX
#define GPIO_MAX_IDX GPIOL_IDX
#endif /* GPIOL_BASE */
#define GPIOM_IDX 11
#if defined(GPIOM_BASE)
#undef GPIO_MAX_IDX
#define GPIO_MAX_IDX GPIOM_IDX
#endif /* GPIOM_BASE */
#define GPION_IDX 12
#if defined(GPION_BASE)
#undef GPIO_MAX_IDX
#define GPIO_MAX_IDX GPION_IDX
#endif /* GPION_BASE */
#define GPIOP_IDX 13
#if defined(GPIOP_BASE)
#undef GPIO_MAX_IDX
#define GPIO_MAX_IDX GPIOP_IDX
#endif /* GPIOP_BASE */
#define GPIOQ_IDX 14
#if defined(GPIOQ_BASE)
#undef GPIO_MAX_IDX
#define GPIO_MAX_IDX GPIOQ_IDX
#endif /* GPIOQ_BASE */
#define GPIOR_IDX 15
#if defined(GPIOR_BASE)
#undef GPIO_MAX_IDX
#define GPIO_MAX_IDX GPIOR_IDX
#endif /* GPIOR_BASE */
#define GPIOS_IDX 16
#if defined(GPIOS_BASE)
#undef GPIO_MAX_IDX
#define GPIO_MAX_IDX GPIOS_IDX
#endif /* GPIOS_BASE */
#define GPIOT_IDX 17
#if defined(GPIOT_BASE)
#undef GPIO_MAX_IDX
#define GPIO_MAX_IDX GPIOT_IDX
#endif /* GPIOT_BASE */
const uint32_t wBSPACMdeviceTM4CperiphGPIO[] = {
#if GPIOA_IDX <= GPIO_MAX_IDX
#if defined(GPIOA_BASE)
    GPIOA_BASE,
#else /* GPIOA available */
    0,
#endif /* GPIOA available */
#endif /* GPIOA below max index */
#if GPIOB_IDX <= GPIO_MAX_IDX
#if defined(GPIOB_BASE)
    GPIOB_BASE,
#else /* GPIOB available */
    0,
#endif /* GPIOB available */
#endif /* GPIOB below max index */
#if GPIOC_IDX <= GPIO_MAX_IDX
#if defined(GPIOC_BASE)
    GPIOC_BASE,
#else /* GPIOC available */
    0,
#endif /* GPIOC available */
#endif /* GPIOC below max index */
#if GPIOD_IDX <= GPIO_MAX_IDX
#if defined(GPIOD_BASE)
    GPIOD_BASE,
#else /* GPIOD available */
    0,
#endif /* GPIOD available */
#endif /* GPIOD below max index */
#if GPIOE_IDX <= GPIO_MAX_IDX
#if defined(GPIOE_BASE)
    GPIOE_BASE,
#else /* GPIOE available */
    0,
#endif /* GPIOE available */
#endif /* GPIOE below max index */
#if GPIOF_IDX <= GPIO_MAX_IDX
#if defined(GPIOF_BASE)
    GPIOF_BASE,
#else /* GPIOF available */
    0,
#endif /* GPIOF available */
#endif /* GPIOF below max index */
#if GPIOG_IDX <= GPIO_MAX_IDX
#if defined(GPIOG_BASE)
    GPIOG_BASE,
#else /* GPIOG available */
    0,
#endif /* GPIOG available */
#endif /* GPIOG below max index */
#if GPIOH_IDX <= GPIO_MAX_IDX
#if defined(GPIOH_BASE)
    GPIOH_BASE,
#else /* GPIOH available */
    0,
#endif /* GPIOH available */
#endif /* GPIOH below max index */
#if GPIOJ_IDX <= GPIO_MAX_IDX
#if defined(GPIOJ_BASE)
    GPIOJ_BASE,
#else /* GPIOJ available */
    0,
#endif /* GPIOJ available */
#endif /* GPIOJ below max index */
#if GPIOK_IDX <= GPIO_MAX_IDX
#if defined(GPIOK_BASE)
    GPIOK_BASE,
#else /* GPIOK available */
    0,
#endif /* GPIOK available */
#endif /* GPIOK below max index */
#if GPIOL_IDX <= GPIO_MAX_IDX
#if defined(GPIOL_BASE)
    GPIOL_BASE,
#else /* GPIOL available */
    0,
#endif /* GPIOL available */
#endif /* GPIOL below max index */
#if GPIOM_IDX <= GPIO_MAX_IDX
#if defined(GPIOM_BASE)
    GPIOM_BASE,
#else /* GPIOM available */
    0,
#endif /* GPIOM available */
#endif /* GPIOM below max index */
#if GPION_IDX <= GPIO_MAX_IDX
#if defined(GPION_BASE)
    GPION_BASE,
#else /* GPION available */
    0,
#endif /* GPION available */
#endif /* GPION below max index */
#if GPIOP_IDX <= GPIO_MAX_IDX
#if defined(GPIOP_BASE)
    GPIOP_BASE,
#else /* GPIOP available */
    0,
#endif /* GPIOP available */
#endif /* GPIOP below max index */
#if GPIOQ_IDX <= GPIO_MAX_IDX
#if defined(GPIOQ_BASE)
    GPIOQ_BASE,
#else /* GPIOQ available */
    0,
#endif /* GPIOQ available */
#endif /* GPIOQ below max index */
#if GPIOR_IDX <= GPIO_MAX_IDX
#if defined(GPIOR_BASE)
    GPIOR_BASE,
#else /* GPIOR available */
    0,
#endif /* GPIOR available */
#endif /* GPIOR below max index */
#if GPIOS_IDX <= GPIO_MAX_IDX
#if defined(GPIOS_BASE)
    GPIOS_BASE,
#else /* GPIOS available */
    0,
#endif /* GPIOS available */
#endif /* GPIOS below max index */
#if GPIOT_IDX <= GPIO_MAX_IDX
#if defined(GPIOT_BASE)
    GPIOT_BASE,
#else /* GPIOT available */
    0,
#endif /* GPIOT available */
#endif /* GPIOT below max index */
};
const uint8_t nBSPACMdeviceTM4CperiphGPIO = sizeof(wBSPACMdeviceTM4CperiphGPIO)/sizeof(*wBSPACMdeviceTM4CperiphGPIO);

#define I2C_MAX_IDX -1
#define I2C0_IDX 0
#if defined(I2C0_BASE)
#undef I2C_MAX_IDX
#define I2C_MAX_IDX I2C0_IDX
#endif /* I2C0_BASE */
#define I2C1_IDX 1
#if defined(I2C1_BASE)
#undef I2C_MAX_IDX
#define I2C_MAX_IDX I2C1_IDX
#endif /* I2C1_BASE */
#define I2C2_IDX 2
#if defined(I2C2_BASE)
#undef I2C_MAX_IDX
#define I2C_MAX_IDX I2C2_IDX
#endif /* I2C2_BASE */
#define I2C3_IDX 3
#if defined(I2C3_BASE)
#undef I2C_MAX_IDX
#define I2C_MAX_IDX I2C3_IDX
#endif /* I2C3_BASE */
#define I2C4_IDX 4
#if defined(I2C4_BASE)
#undef I2C_MAX_IDX
#define I2C_MAX_IDX I2C4_IDX
#endif /* I2C4_BASE */
#define I2C5_IDX 5
#if defined(I2C5_BASE)
#undef I2C_MAX_IDX
#define I2C_MAX_IDX I2C5_IDX
#endif /* I2C5_BASE */
#define I2C6_IDX 6
#if defined(I2C6_BASE)
#undef I2C_MAX_IDX
#define I2C_MAX_IDX I2C6_IDX
#endif /* I2C6_BASE */
#define I2C7_IDX 7
#if defined(I2C7_BASE)
#undef I2C_MAX_IDX
#define I2C_MAX_IDX I2C7_IDX
#endif /* I2C7_BASE */
#define I2C8_IDX 8
#if defined(I2C8_BASE)
#undef I2C_MAX_IDX
#define I2C_MAX_IDX I2C8_IDX
#endif /* I2C8_BASE */
#define I2C9_IDX 9
#if defined(I2C9_BASE)
#undef I2C_MAX_IDX
#define I2C_MAX_IDX I2C9_IDX
#endif /* I2C9_BASE */
const uint32_t wBSPACMdeviceTM4CperiphI2C[] = {
#if I2C0_IDX <= I2C_MAX_IDX
#if defined(I2C0_BASE)
    I2C0_BASE,
#else /* I2C0 available */
    0,
#endif /* I2C0 available */
#endif /* I2C0 below max index */
#if I2C1_IDX <= I2C_MAX_IDX
#if defined(I2C1_BASE)
    I2C1_BASE,
#else /* I2C1 available */
    0,
#endif /* I2C1 available */
#endif /* I2C1 below max index */
#if I2C2_IDX <= I2C_MAX_IDX
#if defined(I2C2_BASE)
    I2C2_BASE,
#else /* I2C2 available */
    0,
#endif /* I2C2 available */
#endif /* I2C2 below max index */
#if I2C3_IDX <= I2C_MAX_IDX
#if defined(I2C3_BASE)
    I2C3_BASE,
#else /* I2C3 available */
    0,
#endif /* I2C3 available */
#endif /* I2C3 below max index */
#if I2C4_IDX <= I2C_MAX_IDX
#if defined(I2C4_BASE)
    I2C4_BASE,
#else /* I2C4 available */
    0,
#endif /* I2C4 available */
#endif /* I2C4 below max index */
#if I2C5_IDX <= I2C_MAX_IDX
#if defined(I2C5_BASE)
    I2C5_BASE,
#else /* I2C5 available */
    0,
#endif /* I2C5 available */
#endif /* I2C5 below max index */
#if I2C6_IDX <= I2C_MAX_IDX
#if defined(I2C6_BASE)
    I2C6_BASE,
#else /* I2C6 available */
    0,
#endif /* I2C6 available */
#endif /* I2C6 below max index */
#if I2C7_IDX <= I2C_MAX_IDX
#if defined(I2C7_BASE)
    I2C7_BASE,
#else /* I2C7 available */
    0,
#endif /* I2C7 available */
#endif /* I2C7 below max index */
#if I2C8_IDX <= I2C_MAX_IDX
#if defined(I2C8_BASE)
    I2C8_BASE,
#else /* I2C8 available */
    0,
#endif /* I2C8 available */
#endif /* I2C8 below max index */
#if I2C9_IDX <= I2C_MAX_IDX
#if defined(I2C9_BASE)
    I2C9_BASE,
#else /* I2C9 available */
    0,
#endif /* I2C9 available */
#endif /* I2C9 below max index */
};
const uint8_t nBSPACMdeviceTM4CperiphI2C = sizeof(wBSPACMdeviceTM4CperiphI2C)/sizeof(*wBSPACMdeviceTM4CperiphI2C);

#define SSI_MAX_IDX -1
#define SSI0_IDX 0
#if defined(SSI0_BASE)
#undef SSI_MAX_IDX
#define SSI_MAX_IDX SSI0_IDX
#endif /* SSI0_BASE */
#define SSI1_IDX 1
#if defined(SSI1_BASE)
#undef SSI_MAX_IDX
#define SSI_MAX_IDX SSI1_IDX
#endif /* SSI1_BASE */
#define SSI2_IDX 2
#if defined(SSI2_BASE)
#undef SSI_MAX_IDX
#define SSI_MAX_IDX SSI2_IDX
#endif /* SSI2_BASE */
#define SSI3_IDX 3
#if defined(SSI3_BASE)
#undef SSI_MAX_IDX
#define SSI_MAX_IDX SSI3_IDX
#endif /* SSI3_BASE */
const uint32_t wBSPACMdeviceTM4CperiphSSI[] = {
#if SSI0_IDX <= SSI_MAX_IDX
#if defined(SSI0_BASE)
    SSI0_BASE,
#else /* SSI0 available */
    0,
#endif /* SSI0 available */
#endif /* SSI0 below max index */
#if SSI1_IDX <= SSI_MAX_IDX
#if defined(SSI1_BASE)
    SSI1_BASE,
#else /* SSI1 available */
    0,
#endif /* SSI1 available */
#endif /* SSI1 below max index */
#if SSI2_IDX <= SSI_MAX_IDX
#if defined(SSI2_BASE)
    SSI2_BASE,
#else /* SSI2 available */
    0,
#endif /* SSI2 available */
#endif /* SSI2 below max index */
#if SSI3_IDX <= SSI_MAX_IDX
#if defined(SSI3_BASE)
    SSI3_BASE,
#else /* SSI3 available */
    0,
#endif /* SSI3 available */
#endif /* SSI3 below max index */
};
const uint8_t nBSPACMdeviceTM4CperiphSSI = sizeof(wBSPACMdeviceTM4CperiphSSI)/sizeof(*wBSPACMdeviceTM4CperiphSSI);

#define TIMER_MAX_IDX -1
#define TIMER0_IDX 0
#if defined(TIMER0_BASE)
#undef TIMER_MAX_IDX
#define TIMER_MAX_IDX TIMER0_IDX
#endif /* TIMER0_BASE */
#define TIMER1_IDX 1
#if defined(TIMER1_BASE)
#undef TIMER_MAX_IDX
#define TIMER_MAX_IDX TIMER1_IDX
#endif /* TIMER1_BASE */
#define TIMER2_IDX 2
#if defined(TIMER2_BASE)
#undef TIMER_MAX_IDX
#define TIMER_MAX_IDX TIMER2_IDX
#endif /* TIMER2_BASE */
#define TIMER3_IDX 3
#if defined(TIMER3_BASE)
#undef TIMER_MAX_IDX
#define TIMER_MAX_IDX TIMER3_IDX
#endif /* TIMER3_BASE */
#define TIMER4_IDX 4
#if defined(TIMER4_BASE)
#undef TIMER_MAX_IDX
#define TIMER_MAX_IDX TIMER4_IDX
#endif /* TIMER4_BASE */
#define TIMER5_IDX 5
#if defined(TIMER5_BASE)
#undef TIMER_MAX_IDX
#define TIMER_MAX_IDX TIMER5_IDX
#endif /* TIMER5_BASE */
#define TIMER6_IDX 6
#if defined(TIMER6_BASE)
#undef TIMER_MAX_IDX
#define TIMER_MAX_IDX TIMER6_IDX
#endif /* TIMER6_BASE */
#define TIMER7_IDX 7
#if defined(TIMER7_BASE)
#undef TIMER_MAX_IDX
#define TIMER_MAX_IDX TIMER7_IDX
#endif /* TIMER7_BASE */
const uint32_t wBSPACMdeviceTM4CperiphTIMER[] = {
#if TIMER0_IDX <= TIMER_MAX_IDX
#if defined(TIMER0_BASE)
    TIMER0_BASE,
#else /* TIMER0 available */
    0,
#endif /* TIMER0 available */
#endif /* TIMER0 below max index */
#if TIMER1_IDX <= TIMER_MAX_IDX
#if defined(TIMER1_BASE)
    TIMER1_BASE,
#else /* TIMER1 available */
    0,
#endif /* TIMER1 available */
#endif /* TIMER1 below max index */
#if TIMER2_IDX <= TIMER_MAX_IDX
#if defined(TIMER2_BASE)
    TIMER2_BASE,
#else /* TIMER2 available */
    0,
#endif /* TIMER2 available */
#endif /* TIMER2 below max index */
#if TIMER3_IDX <= TIMER_MAX_IDX
#if defined(TIMER3_BASE)
    TIMER3_BASE,
#else /* TIMER3 available */
    0,
#endif /* TIMER3 available */
#endif /* TIMER3 below max index */
#if TIMER4_IDX <= TIMER_MAX_IDX
#if defined(TIMER4_BASE)
    TIMER4_BASE,
#else /* TIMER4 available */
    0,
#endif /* TIMER4 available */
#endif /* TIMER4 below max index */
#if TIMER5_IDX <= TIMER_MAX_IDX
#if defined(TIMER5_BASE)
    TIMER5_BASE,
#else /* TIMER5 available */
    0,
#endif /* TIMER5 available */
#endif /* TIMER5 below max index */
#if TIMER6_IDX <= TIMER_MAX_IDX
#if defined(TIMER6_BASE)
    TIMER6_BASE,
#else /* TIMER6 available */
    0,
#endif /* TIMER6 available */
#endif /* TIMER6 below max index */
#if TIMER7_IDX <= TIMER_MAX_IDX
#if defined(TIMER7_BASE)
    TIMER7_BASE,
#else /* TIMER7 available */
    0,
#endif /* TIMER7 available */
#endif /* TIMER7 below max index */
};
const uint8_t nBSPACMdeviceTM4CperiphTIMER = sizeof(wBSPACMdeviceTM4CperiphTIMER)/sizeof(*wBSPACMdeviceTM4CperiphTIMER);

#define UART_MAX_IDX -1
#define UART0_IDX 0
#if defined(UART0_BASE)
#undef UART_MAX_IDX
#define UART_MAX_IDX UART0_IDX
#endif /* UART0_BASE */
#define UART1_IDX 1
#if defined(UART1_BASE)
#undef UART_MAX_IDX
#define UART_MAX_IDX UART1_IDX
#endif /* UART1_BASE */
#define UART2_IDX 2
#if defined(UART2_BASE)
#undef UART_MAX_IDX
#define UART_MAX_IDX UART2_IDX
#endif /* UART2_BASE */
#define UART3_IDX 3
#if defined(UART3_BASE)
#undef UART_MAX_IDX
#define UART_MAX_IDX UART3_IDX
#endif /* UART3_BASE */
#define UART4_IDX 4
#if defined(UART4_BASE)
#undef UART_MAX_IDX
#define UART_MAX_IDX UART4_IDX
#endif /* UART4_BASE */
#define UART5_IDX 5
#if defined(UART5_BASE)
#undef UART_MAX_IDX
#define UART_MAX_IDX UART5_IDX
#endif /* UART5_BASE */
#define UART6_IDX 6
#if defined(UART6_BASE)
#undef UART_MAX_IDX
#define UART_MAX_IDX UART6_IDX
#endif /* UART6_BASE */
#define UART7_IDX 7
#if defined(UART7_BASE)
#undef UART_MAX_IDX
#define UART_MAX_IDX UART7_IDX
#endif /* UART7_BASE */
const uint32_t wBSPACMdeviceTM4CperiphUART[] = {
#if UART0_IDX <= UART_MAX_IDX
#if defined(UART0_BASE)
    UART0_BASE,
#else /* UART0 available */
    0,
#endif /* UART0 available */
#endif /* UART0 below max index */
#if UART1_IDX <= UART_MAX_IDX
#if defined(UART1_BASE)
    UART1_BASE,
#else /* UART1 available */
    0,
#endif /* UART1 available */
#endif /* UART1 below max index */
#if UART2_IDX <= UART_MAX_IDX
#if defined(UART2_BASE)
    UART2_BASE,
#else /* UART2 available */
    0,
#endif /* UART2 available */
#endif /* UART2 below max index */
#if UART3_IDX <= UART_MAX_IDX
#if defined(UART3_BASE)
    UART3_BASE,
#else /* UART3 available */
    0,
#endif /* UART3 available */
#endif /* UART3 below max index */
#if UART4_IDX <= UART_MAX_IDX
#if defined(UART4_BASE)
    UART4_BASE,
#else /* UART4 available */
    0,
#endif /* UART4 available */
#endif /* UART4 below max index */
#if UART5_IDX <= UART_MAX_IDX
#if defined(UART5_BASE)
    UART5_BASE,
#else /* UART5 available */
    0,
#endif /* UART5 available */
#endif /* UART5 below max index */
#if UART6_IDX <= UART_MAX_IDX
#if defined(UART6_BASE)
    UART6_BASE,
#else /* UART6 available */
    0,
#endif /* UART6 available */
#endif /* UART6 below max index */
#if UART7_IDX <= UART_MAX_IDX
#if defined(UART7_BASE)
    UART7_BASE,
#else /* UART7 available */
    0,
#endif /* UART7 available */
#endif /* UART7 below max index */
};
const uint8_t nBSPACMdeviceTM4CperiphUART = sizeof(wBSPACMdeviceTM4CperiphUART)/sizeof(*wBSPACMdeviceTM4CperiphUART);

#define WTIMER_MAX_IDX -1
#define WTIMER0_IDX 0
#if defined(WTIMER0_BASE)
#undef WTIMER_MAX_IDX
#define WTIMER_MAX_IDX WTIMER0_IDX
#endif /* WTIMER0_BASE */
#define WTIMER1_IDX 1
#if defined(WTIMER1_BASE)
#undef WTIMER_MAX_IDX
#define WTIMER_MAX_IDX WTIMER1_IDX
#endif /* WTIMER1_BASE */
#define WTIMER2_IDX 2
#if defined(WTIMER2_BASE)
#undef WTIMER_MAX_IDX
#define WTIMER_MAX_IDX WTIMER2_IDX
#endif /* WTIMER2_BASE */
#define WTIMER3_IDX 3
#if defined(WTIMER3_BASE)
#undef WTIMER_MAX_IDX
#define WTIMER_MAX_IDX WTIMER3_IDX
#endif /* WTIMER3_BASE */
#define WTIMER4_IDX 4
#if defined(WTIMER4_BASE)
#undef WTIMER_MAX_IDX
#define WTIMER_MAX_IDX WTIMER4_IDX
#endif /* WTIMER4_BASE */
#define WTIMER5_IDX 5
#if defined(WTIMER5_BASE)
#undef WTIMER_MAX_IDX
#define WTIMER_MAX_IDX WTIMER5_IDX
#endif /* WTIMER5_BASE */
const uint32_t wBSPACMdeviceTM4CperiphWTIMER[] = {
#if WTIMER0_IDX <= WTIMER_MAX_IDX
#if defined(WTIMER0_BASE)
    WTIMER0_BASE,
#else /* WTIMER0 available */
    0,
#endif /* WTIMER0 available */
#endif /* WTIMER0 below max index */
#if WTIMER1_IDX <= WTIMER_MAX_IDX
#if defined(WTIMER1_BASE)
    WTIMER1_BASE,
#else /* WTIMER1 available */
    0,
#endif /* WTIMER1 available */
#endif /* WTIMER1 below max index */
#if WTIMER2_IDX <= WTIMER_MAX_IDX
#if defined(WTIMER2_BASE)
    WTIMER2_BASE,
#else /* WTIMER2 available */
    0,
#endif /* WTIMER2 available */
#endif /* WTIMER2 below max index */
#if WTIMER3_IDX <= WTIMER_MAX_IDX
#if defined(WTIMER3_BASE)
    WTIMER3_BASE,
#else /* WTIMER3 available */
    0,
#endif /* WTIMER3 available */
#endif /* WTIMER3 below max index */
#if WTIMER4_IDX <= WTIMER_MAX_IDX
#if defined(WTIMER4_BASE)
    WTIMER4_BASE,
#else /* WTIMER4 available */
    0,
#endif /* WTIMER4 available */
#endif /* WTIMER4 below max index */
#if WTIMER5_IDX <= WTIMER_MAX_IDX
#if defined(WTIMER5_BASE)
    WTIMER5_BASE,
#else /* WTIMER5 available */
    0,
#endif /* WTIMER5 available */
#endif /* WTIMER5 below max index */
};
const uint8_t nBSPACMdeviceTM4CperiphWTIMER = sizeof(wBSPACMdeviceTM4CperiphWTIMER)/sizeof(*wBSPACMdeviceTM4CperiphWTIMER);
