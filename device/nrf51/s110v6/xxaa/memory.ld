/* Linker script for Nordic Semiconductor nRF51 variant XXAA
 * 256 kiB flash at 0x0000.0000
 *  16 kiB SRAM  at 0x2000.0000
 *
 * S110 v5.2.1 (SDS 1.1) reserves 80 KiBy FLASH, 8 KiBy RAM
 * S110 v6.0.0 (SDS 1.2) reserves 80 KiBy FLASH, 8 KiBy RAM
 */
MEMORY
{
  FLASH (rx) : ORIGIN = 0x00014000, LENGTH = 176k /* 256 - 80 */
  RAM (rwx)  : ORIGIN = 0x20002000, LENGTH = 8k   /* 16 - 8 */
}
