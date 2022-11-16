#include "centipede.h"
#include "xmodem/xmodem.h"
#include <string.h>
#include <stdio.h>
#include <ctype.h>

#ifdef PIC32MX1
#include <plib.h>
#endif

// clear the console reception buffer
#define clear_kbd() { con_rx_out = con_rx_in; }

#define MAX_PARAMS      2       // maximum number of command parameters
#define CON_BUFF_SIZE   36      // command buffer size in bytes

uint8_t cbuf[CON_BUFF_SIZE];    // command buffer


// get a command line from the console
static uint8_t *get_line(void) {
    uint8_t ch = 0;
    do {
        memset(cbuf, 0, CON_BUFF_SIZE);
        while(ch != '\x1B') {
            ch = (uint8_t) getch();
            if(ch == '\r') break;                   // Enter
            else if(ch == '\x1B') printf("\\");     // Esc to cancel the line
            else if((ch == '\b' || ch == 0x7F) && strlen((char *) cbuf) > 0) {  // Backspace
                cbuf[strlen((char *) cbuf) - 1] = 0;
                printf("\b \b");
            }
            else if(ch >= ' ' && ch < 127) {
                if(strlen((char *) cbuf) >= (CON_BUFF_SIZE - 2)) cbuf[strlen((char *) cbuf) - 2] = 0;
                cbuf[strlen((char *) cbuf)] = ch;
            }
        }
    } while(ch == '\x1B');
    if(cbuf[0]) printf("\r\n");
    return cbuf;
}


// built-in Monitor
void monitor(uint8_t clrscr) {
    uint8_t *cmd, pnum, user_cmd = 0;
    uint32_t par[MAX_PARAMS];
    uint32_t view_addr = (uint32_t) (uintptr_t) Centipede_ROM;
    uint32_t list_addr = (uint32_t) (uintptr_t) Centipede_ROM;
    uint32_t par_count = 16;

    prog_addr = (uint8_t *) Centipede_ROM;
    // cbuf = (uint8_t *) malloc(CON_BUFF_SIZE);  // allocate memory for the console buffer
    if(cbuf == NULL) return;    // cannot allocate buffer memory
    for(pnum = 0; pnum < 1 + 200 * !!clrscr; pnum++) printf("\r\n");
    printf("\r\n\n\nCentipede Monitor\r\n");
    while(1) {

        clear_kbd();
        cmd = get_line();
        while(cbuf[0] && cbuf[strlen((char *) cbuf) - 1] == ' ') cbuf[strlen((char *)cbuf) - 1] = 0; // trim trailing spaces
        if(cbuf[0] == 0) cbuf[0] = user_cmd;
        if(cbuf[0] == 0) continue;      // don't do anything with an empty line

        memset(par, 0, sizeof(par));
        for(pnum = 0; pnum < MAX_PARAMS; pnum++) {
            while(*cmd == ' ') cmd++;   // skip spaces
            if(!strncmp((char *) cmd, ". ", 2)) {   // a '.' character yields parameter 0xFFFFFFFF
                cmd += 2;
                par[pnum] = (uint32_t) -1;
            }
            else if(!strncmp((char *) cmd, "ROM ", 4) || !strncmp((char *) cmd, "rom ", 4)) {   // parameter "ROM" is a predefined constant for the start of ROM
                cmd += 4;
                par[pnum] = (uint32_t) (uintptr_t) Centipede_ROM;
            }
            else if(!strncmp((char *) cmd, "RAM ", 4) || !strncmp((char *) cmd, "ram ", 4)) {   // parameter "RAM" is a predefined constant for the start of RAM
                cmd += 4;
                par[pnum] = (uint32_t) (uintptr_t) Centipede_RAM;
            }
            else {  // get a parameter
                if(!isxdigit(*cmd)) break;  // not a parameter so continue with command processing
                for(par[pnum] = 0; isxdigit(*cmd); cmd++) {
                    par[pnum] <<= 4;
                    par[pnum] += ((toupper(*cmd) >= 'A') ? (toupper(*cmd) - 'A' + 10) : (*cmd - '0'));
                }
            }
        }
        while(*cmd == ' ') cmd++;   // skip spaces

        if((toupper(*cmd) == 'H' || *cmd == '?') /* && strlen((char *) cmd) == 1 && pnum == 0 */) {
            printf("\r\n");
            printf("ROM start address %04lX, length %04lX\r\n", (unsigned long) (uintptr_t) Centipede_ROM, (unsigned long) (ROM_KB * 1024ul));
            printf("RAM start address %04lX, length %04lX\r\n", (unsigned long) (uintptr_t) Centipede_RAM, (unsigned long) (RAM_KB * 1024ul));
            printf("\r\n");
            printf("[address]    Z  erase the entire program memory\r\n");
            printf("                the parameter must match the ROM start address\r\n");
            printf("[[address]]  U  upload program code\r\n");
            printf("                no parameter defaults to the ROM start address\r\n");
            printf("[[address]] [count]  V  view memory\r\n");
            printf("[[address]] [count]  L  list code\r\n");
            printf("H or ?  this help    X  exit and reset\r\n");
            printf("\r\n");
            printf("Predefined constants: \'.\' last value, \'RAM\', \'ROM\'\r\n");
            printf("\r\n");
            user_cmd = 0;
        }

        else if(toupper(*cmd) == 'X' && strlen((char *) cmd) == 1 && pnum == 0) break;

        else if(toupper(*cmd) == 'Z' && strlen((char *) cmd) == 1 && pnum == 1) {
            if(par[0] == (uint32_t) (uintptr_t) Centipede_ROM) {
                printf("Erasing program memory... ");
                if(!memory_erase()) printf(" done\r\n");
                else printf(" ERROR\r\n");
            }
            else printf("ERROR: Invalid start address\r\n");
            user_cmd = 0;
        }

        else if(toupper(*cmd) == 'U' && strlen((char *) cmd) == 1 && pnum < 2) {
            prog_addr = (uint8_t *) Centipede_ROM;
            if(pnum > 0) prog_addr = (uint8_t *) (uintptr_t) par[0];
            uint8_t *a = prog_addr;
            uint8_t *e = (uint8_t *) Centipede_ROM + (ROM_KB * 1024ul);
            uint8_t ef = 1;
            while(a >= (uint8_t *) Centipede_ROM && a < e && ef) ef = (*(a++) == 0xFF); // blank check
            if(prog_addr >= (uint8_t *) Centipede_ROM && prog_addr < e) {
                if(ef) {
                    printf("Receiving via XMODEM to start address %04lX\r\n", (unsigned long) (uintptr_t) prog_addr);
                    int r = XmodemReceive(storeBlock, NULL, (ROM_KB * 1024ul), 0, 0);
                    if(r >= 0) printf("OK");
                    else switch(r) {
                        case -1: printf("Cancelled"); break;
                        case -2: printf("Sync Error"); break;
                        case -3: printf("Failed Retry"); break;
                        case -4: printf("Storage Error"); break;
                        default: printf("ERROR"); break;
                    }
                }
                else printf("Blank check failed at %04lX", (unsigned long) (uintptr_t) a);
            }
            else printf("Invalid memory address");
            printf("\r\n");
            user_cmd = 0;
        }

        else if(toupper(*cmd) == 'V' && strlen((char *) cmd) == 1 && pnum < 3) {
            if(pnum > 0 && par[0] != (uint32_t) -1) view_addr = (par[0] >> 4) << 4;
            uint32_t count = par_count;
            if(pnum > 1 && par[1] != (uint32_t) -1) count = par_count = (par[1] >> 4) << 4;
            while(count--) {
                if((view_addr & 0x0F) == 0) printf("%08lX: ", (unsigned long) view_addr);
                printf("%02X", (int) *((uint8_t *) (uintptr_t) view_addr));
                if((view_addr & 0x0F) < 0x0F) printf(" ");
                else printf("\r\n");
                view_addr++;
                if(kbhit() && getch() == CTRL_C) {
                    printf("^C\r\n");
                    break;
                }
            }
            user_cmd = 'V';
        }

        else if(toupper(*cmd) == 'L' && strlen((char *) cmd) == 1 && pnum < 3) {
            if(pnum > 0 && par[0] != (uint32_t) -1) list_addr = par[0];
            uint32_t count = par_count;
            if(pnum > 1 && par[1] != (uint32_t) -1) count = par_count = par[1];
            while(count--) {
                uint8_t *la = (uint8_t *) (uintptr_t) list_addr;
                if(vm_disassembler(&la, NULL, 1)) break;
                list_addr = (uint32_t) (uintptr_t) la;
                printf("  ");
                if(kbhit() && getch() == CTRL_C) {
                    printf("^C\r\n");
                    break;
                }
            }
            user_cmd = 'L';
        }

        else printf("ERROR: invalid command line syntax\r\n");

    }
    // free(cbuf);
    resetPlatform();
}
