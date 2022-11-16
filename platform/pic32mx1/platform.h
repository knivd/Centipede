#ifndef PLATFORM_H
#define	PLATFORM_H

#ifndef _SUPPRESS_PLIB_WARNING
#define _SUPPRESS_PLIB_WARNING
#endif

#ifdef	__cplusplus
extern "C" {
#endif

#include <xc.h>
#include <plib.h>

#ifndef BIT
#define BIT(b) (1ull << (b))
#endif

#define _XTAL_FREQ      48000000ul  // oscillator frequency as defined in the configuration bits
#define _PB_FREQ        48000000ul  // peripheral bus frequency as defined in the configuration bits

// SYSTEM DEFINITIONS ===========================================================================

void _general_exception_handler(void);
void initPlatform(void);
void resetPlatform(void);
void uSec(unsigned long us);

#define ROM_KB      110 // how many kB of non-volatile program memory will be reserved for the Centipede
                        // this value must be divisible by (BYTE_PAGE_SIZE) without a remainder
#define ROM_ADDR    0x9D000000

#define RAM_KB      48  // how many kB is the Centipede RAM
#define RAM_ADDR    0xA0004000

extern const uint8_t Centipede_ROM[];   // starts from (ROM_ADDR) and has size (1024*ROM_KB) bytes
extern uint8_t Centipede_RAM[];         // starts from (RAM_ADDR) and has size (1024*RAM_KB) bytes

uint8_t *prog_addr;		// current flash upload address in Monitor

int memory_erase(void);

// SPECIAL VERSION OF NVMProgram() ADAPTED FOR PIC32MX1xx/2xx ===================================

#ifdef PAGE_SIZE
#undef PAGE_SIZE
#undef BYTE_PAGE_SIZE
#undef ROW_SIZE
#undef BYTE_ROW_SIZE
#undef NUM_ROWS_PAGE
#endif

#define PAGE_SIZE       256             // # of 32-bit Instructions per Page
#define BYTE_PAGE_SIZE  (4 * PAGE_SIZE) // Page size in Bytes
#define ROW_SIZE        32              // # of 32-bit Instructions per Row
#define BYTE_ROW_SIZE   (4 * ROW_SIZE)  // # Row size in Bytes
#define NUM_ROWS_PAGE   8               // Number of Rows per Page

unsigned int NVMProgramMX1(unsigned char *address, unsigned char *data, unsigned int size, unsigned char *pagebuff);
int storeBlock(void *funcCtx, void *xmodemBuffer, int xmodemSize);

// CONSOLE ======================================================================================
// NOTE: the system console uses UART1 on pins 11:Tx and 12:Rx (on 28-pin package PIC32MX1xx/2xx)

#include "../../vm.h"       // needed for the console definitions

#define CON_LOCAL_ECHO  1   // if not 0, the local echo in the console will be enabled

int kbhit(void);
char getch(void);

#ifdef	__cplusplus
}
#endif

#endif
