/* BSPACM - efm32 adc demonstration application
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
#include <em_system.h>
#include <em_cmu.h>
#include <em_adc.h>
#include <stdio.h>
#include <fcntl.h>

#ifndef SAMPLING_RATE_HZ
/* Desired ADCCLK.  Maximum value is 13 MHz.  Note that the ADC
 * prescaler has only 7 bits so the value can't be less than HFPER/128
 * (110 kHz for 14 MHz).  There's no particular reason to make it
 * slow. */
#define SAMPLING_RATE_HZ 13000000
#endif /* SAMPLING_RATE_HZ */
#ifndef THERMOMETER_GRADIENT_uV_dCel
/* Thermometer gradient in microvolts per tenth degree Celsius.  This
 * value is found in the TGRAD_ADCTH cell of the data sheet Electrical
 * Characteristics: Analog Digital Converter table in units mV/Cel */
#define THERMOMETER_GRADIENT_uV_dCel -192
#endif /* THERMOMETER_GRADIENT_uV_dCel */

void main ()
{
  int adcth_base_Cel;
  int adcth_base_adc;
  vBSPACMledConfigure();
  SystemCoreClockUpdate();
  BSPACM_CORE_ENABLE_INTERRUPT();

  printf("\n" __DATE__ " " __TIME__ "\n");
  printf("System clock %lu Hz\n", SystemCoreClock);
  printf("HF clock %lu Hz\n", CMU_ClockFreqGet(cmuClock_HF));
  printf("HFPER clock %lu Hz\n", CMU_ClockFreqGet(cmuClock_HFPER));
  do {
    /* Require minimum 2 us acquisition time for VDD/3 and
     * temperature; see
     * http://community.silabs.com/t5/32-Bit-Discussion/Acquisition-time-for-internal-temperature/m-p/99343.
     * Above we configured for 13MHz ADC clock (though we probably
     * only got 7 MHz) so at most need 26 clocks.  32 will cover
     * it. */
    uint32_t const singlectrl = ADC_SINGLECTRL_AT_32CYCLES | ADC_SINGLECTRL_REF_1V25 | ADC_SINGLECTRL_RES_12BIT;

    CMU_ClockEnable(cmuClock_ADC0, true);
    {
      uint32_t cal;

      ADC_Init_TypeDef cfg = ADC_INIT_DEFAULT;
      cfg.timebase = ADC_TimebaseCalc(0);
      cfg.prescale = ADC_PrescaleCalc(SAMPLING_RATE_HZ, 0);
      if (128 <= cfg.prescale) {
        printf("ERROR: Sampling rate too low for prescale\n");
        break;
      }
      ADC_Init(ADC0, &cfg);

      /* Set calibration for 1.25V reference.  This was done on
       * powerup, but make it explicit to remind folks to do this when
       * other references are used. */
      cal = ADC0->CAL & ~(_ADC_CAL_SINGLEOFFSET_MASK | _ADC_CAL_SINGLEGAIN_MASK);
      cal |= ((DEVINFO->ADC0CAL0 & _DEVINFO_ADC0CAL0_1V25_GAIN_MASK) >> _DEVINFO_ADC0CAL0_1V25_GAIN_SHIFT) << _ADC_CAL_SINGLEGAIN_SHIFT;
      cal |= ((DEVINFO->ADC0CAL0 & _DEVINFO_ADC0CAL0_1V25_OFFSET_MASK) >> _DEVINFO_ADC0CAL0_1V25_OFFSET_SHIFT) << _ADC_CAL_SINGLEOFFSET_SHIFT;
      ADC0->CAL = cal;
    }

    {
      unsigned int hfper_Hz = CMU_ClockFreqGet(cmuClock_ADC0);
      unsigned int prescale = (ADC0->CTRL & _ADC_CTRL_PRESC_MASK) >> _ADC_CTRL_PRESC_SHIFT;
      unsigned int adc_Hz = hfper_Hz / (1 + prescale);

      printf("ADC CTRL %lx; ADC prescale %u clock %u Hz\n", ADC0->CTRL, prescale, adc_Hz);
    }

    /* Read device temperature calibration constants, and adjust if
     * necessary for errata. */
    adcth_base_Cel = ((DEVINFO->CAL & _DEVINFO_CAL_TEMP_MASK) >> _DEVINFO_CAL_TEMP_SHIFT);
    adcth_base_adc = ((DEVINFO->ADC0CAL2 & _DEVINFO_ADC0CAL2_TEMP1V25_MASK) >> _DEVINFO_ADC0CAL2_TEMP1V25_SHIFT);
#if defined(_EFM32_GIANT_FAMILY)
    {
      /* Work around ADC_E116 */
      uint8_t chip_rev = SYSTEM_GetProdRev();
      if ((16 <= chip_rev) && (chip_rev <= 17)) {
        printf("NB: Correcting for calibration error\n");
        adcth_base_adc -= 112;
      }
    }
#endif

    BSPACM_CORE_ENABLE_CYCCNT();
    while (1) {
      int temp_adc;
      int vdddiv3_adc;
      uint32_t c0, c1, c2;

      ADC0->SINGLECTRL = ADC_SINGLECTRL_INPUTSEL_TEMP | singlectrl;
      c0 = BSPACM_CORE_CYCCNT();
      ADC0->CMD = ADC_CMD_SINGLESTART;
      do {
      } while (ADC0->STATUS & ADC_STATUS_SINGLEACT);
      temp_adc = ADC0->SINGLEDATA;
      ADC0->SINGLECTRL = ADC_SINGLECTRL_INPUTSEL_VDDDIV3 | singlectrl;
      c1 = BSPACM_CORE_CYCCNT();
      ADC0->CMD = ADC_CMD_SINGLESTART;
      do {
      } while (ADC0->STATUS & ADC_STATUS_SINGLEACT);
      vdddiv3_adc = ADC0->SINGLEDATA;
      c2 = BSPACM_CORE_CYCCNT();

      {
        int delta_adc = adcth_base_adc - temp_adc;
        /* Convert 12-bit ADC count with 1.25V reference to microvolts:
         * uV = 10^6 * 1.25 * c / 4096
         *    = 10^6 * (5 / 4) * c / 2^12
         *    = 10^6 * 5 * c / 2^14
         *    = 5^6 * 5 * c / 2^8
         *    = (78125 * c) / 256 */
        int delta_uV = (78125 * delta_adc) / 256;
        int temp_cCel = (100 * adcth_base_Cel) - ((10 * delta_uV) / THERMOMETER_GRADIENT_uV_dCel);

        /* Convert 12-bit ADC count with 1.25V reference to millivolts
         * mV = 10^3 * 1.25 * c / 4096
         *    = 10^3 * (5 / 4) * c / 2^12
         *    = 10^3 * 5 * c / 2^14
         *    = 5^3 * 5 * c / 2^11
         *    = (625 * c) / 2048 */
        int vdddiv3_mV = (625 * vdddiv3_adc) / 2048;

        printf("temp %d cCel [%lu cy]; %u mV [%lu cy]; \n",
               temp_cCel, c1-c0, 3 * vdddiv3_mV, c2-c1);
      }

      /* Interesting effect: if we wait in EM0 the measured voltage is
       * about 20 mV lower than it is if we wait in EM2. */
      BSPACM_CORE_DELAY_CYCLES(10 * SystemCoreClock);
    }
  } while (0);
  fflush(stdout);
  ioctl(1, BSPACM_IOCTL_FLUSH, O_WRONLY);
  BSPACM_CORE_DEEP_SLEEP();
}
