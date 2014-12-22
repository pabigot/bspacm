/* Linker script for Nordic Semiconductor nRF51422 variant XXAC
 * 256 kiB flash at 0x0000.0000
 *  16 kiB SRAM at 0x2000.0000 */
MEMORY
{
  FLASH (rx) : ORIGIN = 0x00000000, LENGTH = 0x40000   /* 256k */
  RAM (rwx)  : ORIGIN = 0x20000000, LENGTH = 0x04000   /*  16k */
}
