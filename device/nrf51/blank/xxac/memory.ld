/* Linker script for Nordic Semiconductor nRF51 variant XXAC
 * 256 kiB flash at 0x0000.0000
 *  32 kiB SRAM  at 0x2000.0000 */
MEMORY
{
  FLASH (rx) : ORIGIN = 0x00000000, LENGTH = 256k
  RAM (rwx)  : ORIGIN = 0x20000000, LENGTH = 32k
}
