/* BSPACM - tm4c/display_periphs demonstration application
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
#include <driverlib/sysctl.h>
#include <stdio.h>
#include <fcntl.h>

typedef struct sBSPACMdeviceTM4CperiphDescriptor {
  uint8_t periph_id;
  uint8_t max_instances;
  const char * tag;
} sBSPACMdeviceTM4CperiphDescriptor;

#define ID_TO_PERIPH_INDEX(pid_) ((0xFF00 & (pid_)) >> 10)

#define DESCRIPTOR_RECORD(pid_, tag_) {      \
    .periph_id = ID_TO_PERIPH_INDEX(pid_),   \
    .max_instances = 0x1F & (pid_),          \
    .tag = tag_                              \
  }

const sBSPACMdeviceTM4CperiphDescriptor periphs[] = {
  DESCRIPTOR_RECORD(SYSCTL_PERIPH_ADC1, "ADC"),
  DESCRIPTOR_RECORD(SYSCTL_PERIPH_CAN1, "CAN"),
  DESCRIPTOR_RECORD(SYSCTL_PERIPH_CCM0, "CCM"),
  DESCRIPTOR_RECORD(SYSCTL_PERIPH_COMP0, "COMP"),
  DESCRIPTOR_RECORD(SYSCTL_PERIPH_EEPROM0, "EEPROM"),
  DESCRIPTOR_RECORD(SYSCTL_PERIPH_EMAC0, "EMAC"),
  DESCRIPTOR_RECORD(SYSCTL_PERIPH_EPHY0, "EPHY"),
  DESCRIPTOR_RECORD(SYSCTL_PERIPH_EPI0, "EPI"),
  DESCRIPTOR_RECORD(SYSCTL_PERIPH_FAN1, "FAN"),
  DESCRIPTOR_RECORD(SYSCTL_PERIPH_GPIOT, "GPIO"),
  DESCRIPTOR_RECORD(SYSCTL_PERIPH_HIBERNATE, "HIBERNATE"),
  DESCRIPTOR_RECORD(SYSCTL_PERIPH_I2C9, "I2C"),
  DESCRIPTOR_RECORD(SYSCTL_PERIPH_LCD0, "LCD"),
  DESCRIPTOR_RECORD(SYSCTL_PERIPH_ONEWIRE0, "ONEWIRE"),
  DESCRIPTOR_RECORD(SYSCTL_PERIPH_PWM1, "PWM"),
  DESCRIPTOR_RECORD(SYSCTL_PERIPH_QEI1, "QEI"),
  DESCRIPTOR_RECORD(SYSCTL_PERIPH_SSI3, "SSI"),
  DESCRIPTOR_RECORD(SYSCTL_PERIPH_TIMER7, "TIMER"),
  DESCRIPTOR_RECORD(SYSCTL_PERIPH_UART7, "UART"),
  DESCRIPTOR_RECORD(SYSCTL_PERIPH_UDMA, "UDMA"),
  DESCRIPTOR_RECORD(SYSCTL_PERIPH_USB0, "USB"),
  DESCRIPTOR_RECORD(SYSCTL_PERIPH_WDOG1, "WDOG"),
  DESCRIPTOR_RECORD(SYSCTL_PERIPH_WTIMER5, "WTIMER")
};

void
display_periphs (void)
{
  static const char * const sr_str[] = { "reset", "RESET" };
  static const char * const rcgc_str[] = { "rcgc", "RCGC" };
  static const char * const scgc_str[] = { "scgc", "SCGC" };
  static const char * const dcgc_str[] = { "dcgc", "DCGC" };
  static const char * const pr_str[] = { "rdy", "RDY" };
  volatile uint32_t * const pp_data = &SYSCTL->PPWD;
  volatile uint32_t * const sr_data = &SYSCTL->SRWD;
  volatile uint32_t * const rcgc_data = &SYSCTL->RCGCWD;
  volatile uint32_t * const scgc_data = &SYSCTL->SCGCWD;
  volatile uint32_t * const dcgc_data = &SYSCTL->DCGCWD;
  volatile uint32_t * const pr_data = &SYSCTL->PRWD;
  int i;

  for (i = 0; i < sizeof(periphs)/sizeof(*periphs); ++i) {
    const sBSPACMdeviceTM4CperiphDescriptor * dp = periphs + i;
    unsigned int instance = 0;
    while (instance <= dp->max_instances) {
      const unsigned int mask = (1U << instance);
      if (pp_data[dp->periph_id] & (1U << instance)) {
        char buf[16];
        if (ID_TO_PERIPH_INDEX(SYSCTL_PERIPH_GPIOA) == dp->periph_id) {
          int charid = 'A' + instance;
          if ('O'-2 < charid) {
            charid += 2;
          } else if ('I'-1 < charid) {
            charid += 1;
          }
          snprintf(buf, sizeof(buf), "%s%c", dp->tag, charid);
        } else {
          snprintf(buf, sizeof(buf), "%s%u", dp->tag, instance);
        }
        printf("%-16s %s %s %s %s %s\n", buf,
               sr_str[!!(mask & sr_data[dp->periph_id])],
               rcgc_str[!!(mask & rcgc_data[dp->periph_id])],
               scgc_str[!!(mask & scgc_data[dp->periph_id])],
               dcgc_str[!!(mask & dcgc_data[dp->periph_id])],
               pr_str[!!(mask & pr_data[dp->periph_id])]);
      }
      ++instance;
    }
  }
}

void main ()
{
  vBSPACMledConfigure();

  BSPACM_CORE_ENABLE_INTERRUPT();

  printf("\n" __DATE__ " " __TIME__ "\n");
  printf("System clock %lu Hz\n", SystemCoreClock);
  display_periphs();
  fflush(stdout);
  ioctl(1, BSPACM_IOCTL_FLUSH, O_WRONLY);
  BSPACM_CORE_DEEP_SLEEP();
}
