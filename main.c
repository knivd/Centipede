#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include "centipede.h"


int _inbyte(unsigned short timeout_ms) {
	int ch = EOF;

	#ifdef PIC32MX1

		uint32_t t = timeout_ms * 10;
		do {
			ch = _mon_getc(0);              // test for received character
			if(ch >= 0) ch = _mon_getc(-1); // get the character and remove it from the buffer (no echo!)
			else uSec(100);
		} while(t-- && ch == EOF);

	#else	// generic platform
		// todo _inbyte() for Xmodem

	#endif

	return ch;
}


int main(int argc, char *argv[]) {
    int r = 0;

    #ifdef PIC32MX1

        initPlatform();
        r = 50;
        while(r--) {    // wait one second for Esc to force entry into the Monitor
            uSec(20000);
            if(kbhit() && getch() == 0x1B) monitor(1);
        }
        uint32_t *p = (uint32_t *) Centipede_ROM;
        if(*p != 0xFFFFFFFF) vm_run((uint8_t *) Centipede_ROM);
        monitor(1); // the Monitor exits with reset

    #else	// generic platform

		if(argc > 1) {

			if(!strcmp(argv[1], "-m") || !strcmp(argv[1], "-r")) {
				if(argc > 2) {
					FILE *f = fopen(argv[2], "rb");
					if(f) {
						long int flen;
						fseek(f, 0, SEEK_END);
						flen = ftell(f);
						if(flen > 0) {
							uint8_t *srcaddr = Centipede_ROM;
							rewind(f);
							fread(srcaddr, 1, flen, f);
							fclose(f);
							if(!strcmp(argv[1], "-m")) monitor(1);
							else if(!strcmp(argv[1], "-r")) vm_run(Centipede_ROM);
						}
					}
					else printf("  Unable to open input file \"%s\"\r\n", argv[1]);
				}
				else monitor(1);
			}

			else {
				printf("Compiling \"%s\"", argv[1]);
				if(argc > 2) printf(" into \"%s\"", argv[2]);
				printf("\r\n");
				FILE *f = fopen(argv[1], "rb");
				if(f) {
					long int flen;
					fseek(f, 0, SEEK_END);
					flen = ftell(f);
					if(flen > 0) {
						uint8_t *srcaddr = Centipede_RAM;
						rewind(f);
						fread(srcaddr, 1, flen, f);
						fclose(f);
						*(srcaddr + flen) = '\0';	// ensure proper source termination
						uint8_t *src = srcaddr;
						uint8_t *dst = Centipede_ROM;
						memory_erase();
						r = vm_assembler((char **) &src, &dst, 0);
						if(r < 0) {
							while(src > srcaddr && *src != '\n') src--;
							if(*src == '\n') src++;
							while(*src == ' ') src++;
							printf("  ");
							while(*src && *src >= ' ') printf("%c", *(src++));
							unsigned long line = 1;
							uint8_t *p = srcaddr;
							while(p < src) {
								if(*(p++) == '\n') line++;
							}
							printf("\r\n  Line %lu, error code %i\r\n", line, (int) r);
						}
						else if(argc > 2) {
							FILE *fo = fopen(argv[2], "wb");
							if(fo) {
								fwrite(Centipede_ROM, 1, r, f);
								fclose(f);
							}
							else printf("  Unable to create output file \"%s\"\r\n", argv[2]);
						}
						if(r >= 0) printf("  Output %i bytes\r\n", r);
						if(argc < 3) vm_run(Centipede_ROM);	// run if only the source has been specified
					}
					fclose(f);
				}
				else printf("  Unable to open input file \"%s\"\r\n", argv[1]);
			}

        }

        else {
			printf("Usage:\r\n");
			printf("[src_file] [bin_file] - compile and store\r\n");
			printf("[src_file]            - compile and run\r\n");
			printf("-r [bin_file]         - load binary file and run\r\n");
			printf("-m [[bin_file]]       - load binary file and start the Monitor\r\n");
			printf("\r\n");
        }

    #endif
    return r;
}
