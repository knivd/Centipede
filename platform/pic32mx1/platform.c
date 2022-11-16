#include "cfg_bits.h"
#include "platform.h"
#include <stdio.h>
#include <string.h>

// reserved Centipede ROM
// NOTE: from physical address (ROM_ADDR)
__attribute__ ((address(ROM_ADDR)))
const uint8_t Centipede_ROM[(((ROM_KB * 1024ul) / BYTE_ROW_SIZE) + !!((ROM_KB * 1024ul) % BYTE_ROW_SIZE)) * BYTE_ROW_SIZE] \
                                = { [0 ... (ROM_KB * 1024ul)-1] = 0xFF };

// reserved Centipede RAM
// NOTE: from physical address (RAM_ADDR)
__attribute__ ((address(RAM_ADDR)))
uint8_t Centipede_RAM[RAM_KB * 1024ul] = { [0 ... (RAM_KB * 1024ul)-1] = 0 };


// initialise the system
void initPlatform(void) {
	DisableWDT();
    TRISA = TRISB = 0xFFFFFFFF;
    ANSELA = ANSELB = 0;
    LATA = LATB = 0;
    CNENA = CNENB = CNCONA = CNCONB = 0;
    CNPUA = CNPUB = CNPDA = CNPDB = 0;

    // initialise system serial console on UART1 and pins RA4(RX) and RB4(TX)
    memset((char *) con_rx_buf, 0, sizeof(CONSOLE_RX_BUF_S));
    con_rx_in = con_rx_out = 0;
    PPSInput(3, U1RX, RPA4);
    PPSOutput(1, RPB4, U1TX);
    UARTEnable(UART1, 0);
    UARTConfigure(UART1, (UART_ENABLE_HIGH_SPEED | UART_ENABLE_PINS_TX_RX_ONLY));
    UARTSetLineControl(UART1, (UART_DATA_SIZE_8_BITS | UART_PARITY_NONE | UART_STOP_BITS_1));
    UARTSetDataRate(UART1, _PB_FREQ, SERIAL_BAUDRATE);
    UARTSetFifoMode(UART1, UART_INTERRUPT_ON_RX_NOT_EMPTY);
    UARTEnable(UART1, (UART_ENABLE | UART_PERIPHERAL | UART_TX | UART_RX));
    INTSetVectorPriority(INT_UART_1_VECTOR, INT_PRIORITY_LEVEL_3);
    INTClearFlag(INT_U1RX);
    INTEnable(INT_U1RX, INT_ENABLED);

    INTConfigureSystem(INT_SYSTEM_CONFIG_MULT_VECTOR);
    INTEnableSystemMultiVectoredInt();  // allow vectored interrupts
    INTEnableInterrupts();
}


// UART console interrupt handler
void __ISR(_UART_1_VECTOR, IPL3AUTO) ConsoleRxHandler(void) {
    while(UARTReceivedDataIsAvailable(UART1)) {
        int e = UARTGetLineStatus(UART1) & (UART_PARITY_ERROR | UART_FRAMING_ERROR | UART_OVERRUN_ERROR);
        int c = UARTGetDataByte(UART1);
        U1STACLR = e;
        if(!e) {
            con_rx_buf[con_rx_in] = c;
            if(++con_rx_in >= CONSOLE_RX_BUF_S) con_rx_in = 0;
            if(con_rx_in == con_rx_out) {   // drop the oldest character if the buffer gets full
                if(++con_rx_out >= CONSOLE_RX_BUF_S) con_rx_out = 0;
            }
        }
    }
    INTClearFlag(INT_U1RX);
}


// put a character on the system console
void _mon_putc(char ch) {
    while(UARTTransmitterIsReady(UART1) == 0) continue;
    UART_DATA ud;
    ud.__data = (UINT16) ch;
    UARTSendData(UART1, ud);
    while(UARTTransmissionHasCompleted(UART1) == 0) continue;
}


// get a character from the system console buffer
int _mon_getc(int blocking) {
    while(con_rx_in == con_rx_out) {    // wait for a character
        if(!blocking) return EOF;       // there is nothing in the buffer
    }
    int c = (int) con_rx_buf[con_rx_out];
    if(blocking) {
        if(++con_rx_out >= CONSOLE_RX_BUF_S) con_rx_out = 0;    // move the output pointer to the next character
        #if CON_LOCAL_ECHO != 0
            if(blocking != -1) putchar(c);  // parameter -1 is a special case to disable echo even when it is enabled
        #endif
    }
    return c;
}


// get character from the console without removing it from the buffer
// return the current character from the input buffer, or EOF in case the buffer is empty
int kbhit(void) {
    int ch = _mon_getc(0);
    return ((ch != EOF) ? ch : 0);  // character with code 0 cannot trigger kbhit()
}


// getch() hook
char getch(void) {
    return _mon_getc(1);
}


// CPU exceptions handler
__attribute__ ((nomips16, optimize("-O0")))
void _general_exception_handler(void)	{
    const static char *szException[] = {
        "Interrupt",                        // 0
        "Unknown",                          // 1
        "Unknown",                          // 2
        "Unknown",                          // 3
        "Address error (load or ifetch)",   // 4
        "Address error (store)",            // 5
        "Bus error (ifetch)",               // 6
        "Bus error (load/store)",           // 7
        "SysCall",                          // 8
        "Breakpoint",                       // 9
        "Reserved instruction",             // 10
        "Coprocessor unusable",             // 11
        "Arithmetic overflow",              // 12
        "Trap (possible divide by zero)",   // 13
        "Unknown",                          // 14
        "Unknown",                          // 15
        "Implementation specific 1",        // 16
        "CorExtend Unusable",               // 17
        "Coprocessor 2"                     // 18
    };
    volatile static unsigned long codeException;
    volatile static unsigned long addressException;
    const char *pszExcept;
    asm volatile ("mfc0 %0,$13" : "=r" (codeException));
    asm volatile ("mfc0 %0,$14" : "=r" (addressException));
    codeException = (codeException & 0x7C) >> 2;
    if(codeException < 19) {
        pszExcept = szException[codeException];
        printf("\r\n\nCPU EXCEPTION: '%s' at address $%04lx\r\nRestarting...\r\n\n\n", pszExcept, addressException);
        uSec(3000000);
    }
    SoftReset();
}


void resetPlatform(void) {
    SoftReset();
}


void uSec(unsigned long us) {
    unsigned long i = ((((unsigned long) (us) * 1000) - 600) / (2000000000 / _XTAL_FREQ));
    unsigned int z;
    if(us <= 100) z = INTDisableInterrupts();
    WriteCoreTimer(0);
    while(ReadCoreTimer() < i);
    if(us <= 100) INTRestoreInterrupts(z);
}


// SPECIAL VERSION OF NVMProgram() ADAPTED FOR PIC32MX1xx/2xx ===================================

unsigned int NVMProgramMX1(unsigned char *address, unsigned char *data, unsigned int size, unsigned char *pagebuff) {
    uintptr_t pageStartAddr;
    unsigned int numBefore, numAfter, numLeftInPage;
    unsigned int index;

	if((size & 3) || ((uintptr_t) pagebuff & 3)) return 1;          // 1. make sure that the size and pagebuff are word aligned
	if(size == 0) return 0;     // nothing to program

    pageStartAddr = (uintptr_t) address & (~(BYTE_PAGE_SIZE - 1));  // 2. calculate Page Start address
	numBefore = (uintptr_t) address & (BYTE_PAGE_SIZE - 1);         // 3. calculate the number of bytes that need to be copied from Flash.
    memcpy(pagebuff, (unsigned char *) pageStartAddr, numBefore);   // 4. make a copy of original data, if necessary

    while(size) {
        numLeftInPage = BYTE_PAGE_SIZE - numBefore;                 // 5. determine how many to copy from Source data
        if(size <= numLeftInPage) {
            memcpy((pagebuff + numBefore), data, size);             // copy all of it
            numAfter = numLeftInPage - size;                        // calculate the number of bytes that need to be stored after the address.
    	    if(numAfter) memcpy((pagebuff + numBefore + size), (unsigned char *) ((uintptr_t) address + size), numAfter); // copy whats left
            size = 0;
        }
        else {
            memcpy((pagebuff + numBefore), data, numLeftInPage);    // copy numLeft of it
            size -= numLeftInPage;                                  // decrement size
            address += numLeftInPage; data += numLeftInPage;        // increment addresses
        }
        NVMErasePage((void *) pageStartAddr);                       // erase the Page
        for(index = 0; index < NUM_ROWS_PAGE; index++) {            // program the Page
            NVMWriteRow((unsigned char *) (pageStartAddr + (index * BYTE_ROW_SIZE)), (pagebuff + (index * BYTE_ROW_SIZE)));
        }
		numBefore = 0;                                              // done with partial page, move to page boundary
		pageStartAddr = (uintptr_t) address;
    }
    return 0;
}


// store single received xmodem block
// this function is required by XmodemReceive()
int storeBlock(void *funcCtx, void *xmodemBuffer, int xmodemSize) {
    // using the beginning of the RAM as buffer
    if(NVMProgramMX1(prog_addr, xmodemBuffer, xmodemSize, Centipede_RAM) == 0) prog_addr += xmodemSize;
    else return -1;
    return xmodemSize;
}


// erase the entire allocated program memory
int memory_erase(void) {
    uint8_t *a = (uint8_t *) Centipede_ROM;
    uint32_t c = 0;
    int r = 0;
    while(c < (1024ul * ROM_KB)) {
        r = NVMErasePage(a);
        if(r) break;
        a += BYTE_PAGE_SIZE;
        c += BYTE_PAGE_SIZE;
    }
    return r;
}
