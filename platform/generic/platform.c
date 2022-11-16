#include "platform.h"
#include <stdio.h>
#include <string.h>
#include <time.h>

// reserved Centipede ROM
uint8_t Centipede_ROM[ROM_KB * 1024ul] = { [0 ... (ROM_KB * 1024ul)-1] = 0xFF };

// reserved Centipede RAM
uint8_t Centipede_RAM[RAM_KB * 1024ul] = { [0 ... (RAM_KB * 1024ul)-1] = 0 };


// initialise the system
void initPlatform(void) {
    // initialise system serial console
    memset((char *) con_rx_buf, 0, sizeof(CONSOLE_RX_BUF_S));
    con_rx_in = con_rx_out = 0;
}


void resetPlatform(void) {
    printf("Not supported on this platform\r\n");
}


// generic platform implementation only allows millisecond accuracy
void uSec(unsigned long us) {
    unsigned long ms = us / 1000;
    unsigned long ce = clock() + (ms * (CLOCKS_PER_SEC / 1000));
	while((unsigned long) clock() > ce) continue;
	while((unsigned long) clock() < ce) continue;
}


// store single received xmodem block
// this function is required by XmodemReceive()
int storeBlock(void *funcCtx, void *xmodemBuffer, int xmodemSize) {
    memcpy(prog_addr, xmodemBuffer, xmodemSize);
    prog_addr += xmodemSize;
    return xmodemSize;
}


// erase the entire allocated program memory
int memory_erase(void) {
    memset(Centipede_ROM, 0xFF, sizeof(Centipede_ROM));
    return 0;
}
