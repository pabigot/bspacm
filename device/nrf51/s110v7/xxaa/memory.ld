/* Linker script for Nordic Semiconductor nRF51 variant XXAA
 * 256 kiB flash at 0x0000.0000
 *  16 kiB SRAM  at 0x2000.0000
 *
 * S110 v7.0.0 (SDS 1.3) reserves 88 KiBy FLASH, 8 KiBy RAM
 */
MEMORY
{
  FLASH (rx) : ORIGIN = 0x00016000, LENGTH = 168k /* 256 - 88 */
  RAM (rwx)  : ORIGIN = 0x20002000, LENGTH = 8k   /* 16 - 8 */
}
