#ifndef PLATFORM_H
#define	PLATFORM_H

#ifdef	__cplusplus
extern "C" {
#endif

#ifndef BIT
#define BIT(b) (1ul << (b))
#endif

#include <stdint.h>

// SYSTEM DEFINITIONS ===========================================================================

void initPlatform(void);
void resetPlatform(void);
void uSec(unsigned long us);

#define ROM_KB      128 // how many kB of non-volatile program memory will be reserved for the Centipede
                        // this value must be divisible by (BYTE_PAGE_SIZE) without a remainder

#define RAM_KB      256 // how many kB is the Centipede RAM

extern uint8_t Centipede_ROM[];		// starts from (ROM_ADDR) and has size (1024*ROM_KB) bytes
extern uint8_t Centipede_RAM[];     // starts from (RAM_ADDR) and has size (1024*RAM_KB) bytes

uint8_t *prog_addr;		// current flash upload address in Monitor

int storeBlock(void *funcCtx, void *xmodemBuffer, int xmodemSize);
int memory_erase(void);

// CONSOLE ======================================================================================

#include "../../vm.h"       // needed for the console definitions

#define CON_LOCAL_ECHO  1   // if not 0, the local echo in the console will be enabled

int kbhit(void);
char getch(void);

#ifdef	__cplusplus
}
#endif

#endif
