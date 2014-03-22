/* Linker script for Texas Instruments TM4C1294NCPDT
 * 1024 kiB flash at 0x0000.0000
 *  256 kiB SRAM at 0x2000.0000 */
MEMORY
{
  FLASH (rx) : ORIGIN = 0x00000000, LENGTH = 0x100000   /* 1M */
  RAM (rwx)  : ORIGIN = 0x20000000, LENGTH = 0x40000   /* 256k */
}
