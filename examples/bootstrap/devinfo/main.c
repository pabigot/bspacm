/* BSPACM - bootstrap/devinfo
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

/* This program displays the device identifying information over the
 * console.  It also serves as an example of how to detect such
 * information within an application. */

#include <bspacm/utility/led.h>
#include <inttypes.h>
#include <ctype.h>
#include <stdio.h>
#if (BSPACM_DEVICE_SERIES_EFM32 - 0)
#include <em_system.h>
#endif /* BSPACM_DEVICE_SERIES_EFM32 */

void dumpMemory (const uint8_t * dp,
                 size_t len,
                 unsigned long base)
{
  const uint8_t * const edp = dp + len;
  const uint8_t * adp = dp;

  while (dp < edp) {
    if (0 == (base & 0x0F)) {
      if (adp < dp) {
        printf("  ");
        while (adp < dp) {
          putchar(isprint(*adp) ? *adp : '.');
          ++adp;
        }
      }
      adp = dp;
      printf("\n%08lx ", base);
    } else if (0 == (base & 0x07)) {
      putchar(' ');
    }
    printf(" %02x", *dp++);
    ++base;
  }
  if (adp < dp) {
    while (base & 0x0F) {
      if (0 == (base & 0x07)) {
        putchar(' ');
      }
      printf("   ");
      ++base;
    }
    printf("  ");
    while (adp < dp) {
      putchar(isprint(*adp) ? *adp : '.');
      ++adp;
    }
  }
  putchar('\n');
}

void main ()
{
  uint32_t scc_Hz;

  scc_Hz = SystemCoreClock;
  SystemCoreClockUpdate();

  vBSPACMledConfigure();
  BSPACM_CORE_ENABLE_INTERRUPT();

  printf("\n" __DATE__ " " __TIME__ "\n");
  printf("System clock %lu Hz was %lu Hz\n", SystemCoreClock, scc_Hz);

#if (BSPACM_DEVICE_SERIES_TM4C - 0)
  printf("TM4C series device:\n");
  printf("DID0 %08" PRIx32 " ; DID1 %08" PRIx32 "\n",
         SYSCTL->DID0, SYSCTL->DID1);
  if (1 == (0x07 & (SYSCTL->DID0 >> 28))) {
    uint8_t class = 0xFF & (SYSCTL->DID0 >> 16);
    uint8_t major = 0xFF & (SYSCTL->DID0 >> 8);
    uint8_t minor = 0xFF & (SYSCTL->DID0 >> 0);
    /* Only know how to decode the second version format.  Magic
     * numbers because TI doesn't have quality CMSIS headers. */
    switch (class) {
      case 0x05:
        printf("TM4C123");
        break;
      case 0x0A:
        printf("TM4C129"); /* "Snowflake" */
        break;
      default:
        printf("(Unknown line %u)", class);
        break;
    }
    printf(" die rev %c%u\n", 'A' + major, minor);
  }
  {
    uint8_t ver = 0x0F & (SYSCTL->DID1 >> 28);
    uint8_t fam = 0x0F & (SYSCTL->DID1 >> 24);
    uint8_t partno = 0xFF & (SYSCTL->DID1 >> 16);
    uint8_t pincount = 0x07 & (SYSCTL->DID1 >> 13);
    uint8_t temp = 0x07 & (SYSCTL->DID1 >> 5);
    uint8_t pkg = 0x03 & (SYSCTL->DID1 >> 3);
    uint8_t rohs = 0x01 & (SYSCTL->DID1 >> 2);
    uint8_t qual = 0x03 & (SYSCTL->DID1 >> 0);
    printf("Version: ");
    switch (ver) {
      case 0x00:
        printf("LM3S");
        break;
      case 0x01:
        printf("TM4C");
        break;
      default:
        printf("%u", ver);
        break;
    }
    printf("\nFamily: ");
    switch (fam) {
      case 0x00:
        printf("TM4C/LM4F/LM3S");
        break;
      default:
        printf("%u", fam);
        break;
    }
    printf("\nPart: 0x%x", partno);
    {
      static const char * pincount_str[] = {
        "resv0", "resv1",
#if (BSPACM_DEVICE_LINE_TM4C123 - 0)
        "100-pin", "64-pin",
        "144-pin", "157-pin",
        "168-pin", "resv7"
#elif (BSPACM_DEVICE_LINE_TM4C129 - 0)
        "100-pin LQFP", "64-pin LQFP",
        "144-pin LQFP", "157-pin BGA",
        "128-pin TQFP", "212-pin BGA"
#else /* line */
        "resv2", "resv3"
        "resv4", "resv5"
        "resv6", "resv7"
#endif /* line */
      };
      static const char * pkg_str[] = {
#if (BSPACM_DEVICE_LINE_TM4C123 - 0)
        "resv0", "LQFP", "BGA", "resv3"
#elif (BSPACM_DEVICE_LINE_TM4C129 - 0)
        "resv0", "QFP", "BGA", "resv3"
#else /* line */
        "resv0", "resv1", "resv2", "resv3"
#endif /* line */
      };
      printf("\nPackage: %s, type %s, %sROHS", pincount_str[pincount], pkg_str[pkg], rohs ? "" : "NOT ");
    }
    {
      static const char * temp_str[] = {
        "commercial", "industrial", "extended", "resv3"
      };
      printf("\nTemperature Range: %s\n", temp_str[temp]);
    }
    {
      static const char * qual_str[] = {
        "Engineering Sample",
        "Pilot Production",
        "Fully Qualified",
        "resv3"
      };
      printf("Qualification: %s\n", qual_str[qual]);
    }
#if (BSPACM_DEVICE_LINE_TM4C123 - 0)
    /* TI didn't explain how to decode this; just said 0x7f means 256
     * kiB FLASH and 32 kiB SRAM. */
    printf("Flash size: %u kiB (FSIZE = 0x%x)\n",
           (unsigned int)((1 + FLASH_CTRL->FSIZE) << 1),
           (unsigned int)FLASH_CTRL->FSIZE);
#elif (BSPACM_DEVICE_LINE_TM4C129 - 0)
    {
      unsigned int fsize = FLASH_CTRL->PP & ((1U << 16) - 1);
      printf("Flash size: %u kiB (PP = 0x%08" PRIx32 ")\n",
             (unsigned int)((1 + fsize) << 1),
             FLASH_CTRL->PP);
    }
#else
    printf("Unable to determine flash size\n");
#endif
    printf("SRAM size: %u kiB (SSIZE = 0x%x)\n",
           (unsigned int)((1 + FLASH_CTRL->SSIZE) >> 2),
           (unsigned int)FLASH_CTRL->SSIZE);

  }
#endif /* BSPACM_DEVICE_SERIES_TM4C */

#if (BSPACM_DEVICE_SERIES_EFM32 - 0)
  printf("EFM32 series device:\n");
  {
    SYSTEM_ChipRevision_TypeDef chiprev;
    const char * fp = "unknown";
    /* No SYSTEM_ function for device family extraction */
    switch ((DEVINFO->PART & _DEVINFO_PART_DEVICE_FAMILY_MASK) >> _DEVINFO_PART_DEVICE_FAMILY_SHIFT) {
      case _DEVINFO_PART_DEVICE_FAMILY_G:
        fp = "";
        break;
      case _DEVINFO_PART_DEVICE_FAMILY_GG:
        fp = "Giant ";
        break;
      case _DEVINFO_PART_DEVICE_FAMILY_TG:
        fp = "Tiny ";
        break;
      case _DEVINFO_PART_DEVICE_FAMILY_LG:
        fp = "Leopard ";
        break;
      case _DEVINFO_PART_DEVICE_FAMILY_WG:
        fp = "Wonder ";
        break;
      case _DEVINFO_PART_DEVICE_FAMILY_ZG:
        fp = "Zero ";
        break;
    }
    SYSTEM_ChipRevisionGet(&chiprev);
    printf("\n%sGecko %u product rev %u, chip revision %c (%u.%u)\n",
           fp, SYSTEM_GetPartNumber(), SYSTEM_GetProdRev(),
           'A' + chiprev.minor, chiprev.major, chiprev.minor);
  }
  printf("%u kiB flash, page size %u B\n",
         SYSTEM_GetFlashSize(), (unsigned int)SYSTEM_GetFlashPageSize());
  printf("%u kiB SRAM\n", SYSTEM_GetSRAMSize());

  /* UNIQUEL is a POSIX timestamp; UNIQUEH is a facility id.  See
   * http://community.silabs.com/t5/32-Bit-Discussion/Unique-ID/m-p/98373 */
  printf("DUN %08" PRIx32 "%08" PRIx32 "\n", DEVINFO->UNIQUEH, DEVINFO->UNIQUEL);

#if 0
  /* Calibration data is documented in the Device Information section
   * of the Memory and Bus System chapter of the line reference
   * manual.  Values are pairs of a register address and the value
   * representing the device-specific configuration.  Where the
   * power-up value does not match the calibrated value (ADC, DAC,
   * ACMP, USB, ...), this is because the corresponding module has not
   * been enabled: once it is supplied a clock the values will be
   * correct (or so it appears). */
  {
    unsigned int i;

    printf("\nCalibration data at %p:\n", CALIBRATE);
    for (i = 0; i < CALIBRATE_MAX_REGISTERS; ++i) {
      if (~0 != CALIBRATE[i].ADDRESS) {
        uint32_t actual = *(uint32_t*)CALIBRATE[i].ADDRESS;
        printf("%08" PRIxPTR " @%08" PRIx32 " := %08" PRIx32 " [%08" PRIx32 "]%c\n",
               (uintptr_t)(CALIBRATE + i), CALIBRATE[i].ADDRESS,
               CALIBRATE[i].VALUE,
               actual, (CALIBRATE[i].VALUE == actual) ? ' ' : '*');
      }
    }
  }
#endif /* CALIBRATE */
#if 0
  {
    const uint32_t * dip = (const uint32_t *)DEVINFO;
    const uint32_t * const dipe = dip + sizeof(*DEVINFO) / sizeof(*dip);
    printf("\nDevice info words:");
    while (dip < dipe) {
      if (0x0 == (0x0F & (uintptr_t)dip)) {
        printf("\n%08" PRIxPTR " ", (uintptr_t)dip);
      }
      printf(" %08" PRIx32, *dip);
      ++dip;
    }
    printf("\nDevice info bytes:\n");
    dumpMemory((const uint8_t *)DEVINFO,
               sizeof(*DEVINFO),
               (uintptr_t)DEVINFO);
  }
#endif /* DEVINFO */
#if 0
  {
    printf("\nUser Data page:\n");
    dumpMemory((const uint8_t *)USERDATA_BASE,
               FLASH_PAGE_SIZE,
               USERDATA_BASE);
  }
#endif /* USERDATA */

#endif /* BSPACM_DEVICE_SERIES_EFM32 */

#if (BSPACM_DEVICE_SERIES_NRF51 - 0)
  printf("nRF51 series device:\n");
  printf("FICR block:");
  dumpMemory((const uint8_t *)NRF_FICR_BASE, sizeof(*NRF_FICR), (uintptr_t)NRF_FICR_BASE);
  printf("PART: %lx\n", NRF_FICR->INFO.PART);
  printf("VARIANT: %lx\n", NRF_FICR->INFO.VARIANT);
  printf("RAM: %lu KiBy\n", NRF_FICR->INFO.RAM);
  printf("FLASH: %lu KiBy\n", NRF_FICR->INFO.FLASH);
#endif /* BSPACM_DEVICE_SERIES_NRF51 */
  while(1);
}
