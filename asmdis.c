#include "centipede.h"
#include <math.h>
#include <string.h>
#include <stdio.h>
#include <ctype.h>
#include <inttypes.h>

#define isdigitC(c) ((c) >= '0' && (c) <= '9')
#define isXdigitC(c) (isdigitC(c) || ((c) >= 'a' && (c) <= 'f') || ((c) >= 'A' && (c) <= 'F'))
#define isidchr1C(c) (((c) >= 'a' && (c) <= 'z') || ((c) >= 'A' && (c) <= 'Z') || (c) == '_')
#define isidchrC(c) (isidchr1C(c) || isdigitC(c))

typedef enum { DEC, BIN, OCT, HEX, CHR, STR } num_base_t;
num_base_t num_base = DEC;
int32_t errcode = 0;    // error code during assembly process


// ======================================================================================
// common functions used by both the Assembler and the Disassembler

// return instruction code based on an instruction handler, or -1 if not found
static int16_t instr_code(void (*i_func)(void)) {
    int16_t t = 0xFF;
    do {
        if(instruction[t] == i_func) break;
    } while(--t >= 0);
    return t;
}


// ======================================================================================
// Disassembler

// disassembler
// if (*output) is NULL the output is on the standard system console
// the parameter (num_instr) specifies the number source lines to list
// (*pcode) gets modified after execution
// the function will return the difference between the listed and the specified number of lines for listing
#define INVALID_INSTR "UNKNOWN"
uint32_t vm_disassembler(uint8_t **pcode, char *output, uint32_t num_ln) {
    num_base = DEC;
    uint8_t *vcode;
    uint32_t instr_cnt = 0; // instruction counter within the line
    for(vcode = *pcode; vcode && num_ln > 0; vcode++) {

        if(*vcode == instr_code(i_END))  {
            if(output) sprintf(output, "\r\nEND\r\n");
            else printf("\r\nEND\r\n");
            break;
        }
        if(*vcode == instr_code(i_PADNOP)) continue;    // padding NOPs are not listed
        if(*vcode == instr_code(i_NOP)) {               // normal NOPs are listed just as a new line
            if(output) sprintf(output, "\r\n"); else printf("\r\n");
            num_ln--;
            instr_cnt = 0;
            continue;
        }

        if(*vcode == instr_code(i_SETBIN)) { num_base = BIN; continue; }
        if(*vcode == instr_code(i_SETOCT)) { num_base = OCT; continue;}
        if(*vcode == instr_code(i_SETHEX)) { num_base = HEX; continue;}
        if(*vcode == instr_code(i_SETCHR)) { num_base = CHR; continue;}
        if(*vcode == instr_code(i_SETSTR)) { num_base = STR; continue;}

        if(instr_cnt == 0) printf("\r%04lX:  ", (unsigned long) (uintptr_t) vcode);

        if(*vcode == instr_code(i_COMMENT) || *vcode == instr_code(i_NAME)) {
            uint8_t vc = *(vcode++);
            if(vc == instr_code(i_NAME)) vcode += 8;    // skip the "nonce" field in name definitions
            uint16_t len = *((uint16_t *) vcode);
            vcode += 2;
            if(vc == instr_code(i_COMMENT) && instr_cnt)  {
                if(output) sprintf(output, "   "); else printf("   ");
            }
            if(output) sprintf(output, "%s", instr_text[vc]);
            else printf("%s", instr_text[vc]);
            if(vc == instr_code(i_COMMENT)) printf(" ");
            while(len--) {
                if(output) sprintf(output, "%c", *(vcode++));
                printf("%c", *vcode);
                if(len) vcode++;
            }
            if(output) sprintf(output, " "); else printf(" ");
            if(vc == instr_code(i_COMMENT)) {
                if(output) sprintf(output, "\r\n"); else printf("\r\n");
                num_ln--;
                instr_cnt = 0;
            }
            else instr_cnt++;
        }

        else if(*vcode == instr_code(i_LABEL)) {
            uint8_t vc = *(vcode++);
            uint8_t len = *(vcode++);
            if(output) sprintf(output, "%s", instr_text[vc]);
            else printf("%s", instr_text[vc]);
            while(len--) {
                if(output) sprintf(output, "%c", *(vcode++));
                printf("%c", *vcode);
                if(len) vcode++;
            }
            num_ln--;
            instr_cnt = 0;
        }

        else if(*vcode == instr_code(i_CINT0)) {
            if(output) sprintf(output, "0"); else printf("0");
        }

        else if(*vcode == instr_code(i_CINT1)) {
            if(output) sprintf(output, "1"); else printf("1");
        }

        else if(*vcode == instr_code(i_CINT8)) {
            vcode++;
            switch(num_base) {
                case STR:
                case CHR:
                    if(output) sprintf(output, "\'%c\'", *(vcode++));
                    else printf("\'%c\'", *(vcode++));
                    break;
                case BIN:
                case OCT:
                case HEX:
                case DEC:
                default:
                    if(output) sprintf(output, "%i", (int) *(vcode++));
                    else printf("%i", (int) *(vcode++));
                    break;
            }
            vcode--;
        }

        else if(*vcode == instr_code(i_CINT16)) {
            vcode++;
            switch(num_base) {
                case STR:
                case CHR:
                case BIN:
                case OCT:
                case HEX:
                case DEC:
                default:
                    if(output) sprintf(output, "%i", (int) *((int16_t *) vcode));
                    else printf("%i", (int) *((int16_t *) vcode));
                    vcode += 2;
                    break;
            }
            vcode--;
        }

        else if(*vcode == instr_code(i_CINT32)) {
            vcode++;
            switch(num_base) {
                case STR:
                case CHR:
                case BIN:
                case OCT:
                case HEX:
                case DEC:
                default:
                    if(output) sprintf(output, "%li", (long) *((int32_t *) vcode));
                    else printf("%li", (long) *((int32_t *) vcode));
                    vcode += 4;
                    break;
            }
            vcode--;
        }

        else if(*vcode == instr_code(i_CINT64)) {
            vcode++;
            switch(num_base) {
                case STR:
                case CHR:
                case BIN:
                case OCT:
                case HEX:
                case DEC:
                default:
                    #if INT64_SUPPORTED == 1
                        if(output) sprintf(output, "%" PRId64, *((int64_t *) vcode));
                        else printf("%" PRId64, *((int64_t *) vcode));
                    #else
                        if(output) sprintf(output, "%li", *((int32_t *) vcode));
                        else printf("%li", *((int32_t *) vcode));
                    #endif
                    vcode += 8;
                    break;

            }
            vcode--;
        }

        else if(*vcode == instr_code(i_CFLOAT)) {
            vcode++;
            if(output) sprintf(output, "%G", *((double *) vcode));
            else printf("%G", *((double *) vcode));
            vcode += 8;
            vcode--;
        }

        else if(*vcode == instr_code(i_DATAI8)) {
            vcode++;
            uint32_t cnt = *((uint32_t *) vcode);
            vcode += 4;
            if(num_base != STR) {
                while(cnt--) {
                    if(output) sprintf(output, "%i", (int) *(vcode++));
                    else printf("%i", (int) *(vcode++));
                    if(cnt) {
                        if(output) sprintf(output, ", "); else printf(", ");
                    }
                }
            }
            else {
                if(output) sprintf(output, "\""); else printf("\"");
                char ss[3];
                while(cnt--) {
                    memset(ss, 0, sizeof(ss));
                    ss[0] = *(vcode++);
                    if(cnt == 0) break; // do not need to include the final 0 character in the output
                    if(ss[0] == 0) strcpy(ss, "\0");
                    else if(ss[0] == '\n') strcpy(ss, "\\n");
                    else if(ss[0] == '\r') strcpy(ss, "\\r");
                    else if(ss[0] == '\t') strcpy(ss, "\\t");
                    else if(ss[0] == '\b') strcpy(ss, "\\b");
                    else if(ss[0] == '\a') strcpy(ss, "\\a");
                    else if(ss[0] == '\f') strcpy(ss, "\\f");
                    else if(ss[0] == '\e') strcpy(ss, "\\e");
                    else if(ss[0] == '\\') strcpy(ss, "\\\\");
                    else if(ss[0] == '\'') strcpy(ss, "\\\'");
                    else if(ss[0] == '\"') strcpy(ss, "\\\"");
                    if(ss[0] < ' ' || ss[0] == 0x7F) ss[0] = '.';
                    if(output) sprintf(output, "%s", ss); else printf("%s", ss);
                }
                if(output) sprintf(output, "\""); else printf("\"");
            }
            vcode--;
        }

        else if(*vcode == instr_code(i_DATAI16)) {
            vcode++;
            uint32_t cnt = *((uint32_t *) vcode);
            vcode += 4;
            while(cnt--) {
                int16_t v = *((int16_t *) vcode);
                vcode += 2;
                if(output) sprintf(output, "%i", (int) v);
                else printf("%i", (int) v);
                if(cnt) {
                    if(output) sprintf(output, ", "); else printf(", ");
                }
            }
            vcode--;
        }

        else if(*vcode == instr_code(i_DATAI32)) {
            vcode++;
            uint32_t cnt = *((uint32_t *) vcode);
            vcode += 4;
            while(cnt--) {
                int32_t v = *((int32_t *) vcode);
                vcode += 4;
                if(output) sprintf(output, "%li", (long) v);
                else printf("%li", (long) v);
                if(cnt) {
                    if(output) sprintf(output, ", "); else printf(", ");
                }
            }
            vcode--;
        }

        else if(*vcode == instr_code(i_DATAI64)) {
            vcode++;
            uint32_t cnt = *((uint32_t *) vcode);
            vcode += 4;
            while(cnt--) {
                Int64 v = *((Int64 *) vcode);
                vcode += 8;
                #if INT64_SUPPORTED == 1
                    if(output) sprintf(output, "%" PRId64, v);
                    else printf("%" PRId64, v);
                #else
                    if(output) sprintf(output, "%li", v);
                    else printf("%li", v);
                #endif
                if(cnt) {
                    if(output) sprintf(output, ", "); else printf(", ");
                }
            }
            vcode--;
        }

        else if(*vcode == instr_code(i_DATAF)) {
            vcode++;
            uint32_t cnt = *((uint32_t *) vcode);
            vcode += 4;
            while(cnt--) {
                double v = *((double *) vcode);
                vcode += 8;
                if(output) sprintf(output, "%G", v);
                else printf("%G", v);
                if(cnt) {
                    if(output) sprintf(output, ", "); else printf(", ");
                }
            }
            vcode--;
        }

        else {
            if(output) sprintf(output, "%s", (instr_text[*vcode][0] ? instr_text[*vcode] : INVALID_INSTR));
            else printf("%s", (instr_text[*vcode][0] ? instr_text[*vcode] : INVALID_INSTR));
        }
        if(output) sprintf(output, " "); else printf(" ");
        num_base = DEC;
        instr_cnt++;
    }
    *pcode = vcode;
    return num_ln;
}
#undef INVALID_INSTR


// ======================================================================================
// Assembler

#ifndef PIC32MX1

// skip whitespaces
static void skip_spaces(char **source) {
	char *src = *source;
	while(strchr(" \r\t\v", *src)) src++;
	*source = src;
}


// get from the source and return a 4-bit (single digit) hexadecimal number
static uint8_t get_hex_digit(char **src) {
    char *s = *src;
    char c = *(s++);
    if(c >= '0' && c <= '9') c = (c - '0');
    else if(c >= 'a' && c <= 'f') c = (c - 'a' + 10);
    else if(c >= 'A' && c <= 'F') c = (c - 'A' + 10);
    else s--;
    *src = s;
    return (uint8_t) c;
}


// return the length of a valid identifier word pointed by (*s)
static uint32_t id_len(char *s) {
    uint32_t l;
    for(l = 0; isidchrC(*s); s++, l++);
    return l;
}


// jump to the next instruction and modify the instruction pointer
// most instructions are just a single byte, however there are a few which include parameters to follow
static void instr_next(uint8_t **code) {
    uint32_t n = 0;
    uint8_t *p = *code;
    uint8_t i = *(p++); // skip the instruction code here
    if(i == instr_code(i_CINT8)) p++;
    else if(i == instr_code(i_CINT16)) p += 2;
    else if(i == instr_code(i_CINT32)) p += 4;
    else if(i == instr_code(i_CINT64)) p += 8;
    else if(i == instr_code(i_CFLOAT)) p += 8;
    else if(i == instr_code(i_DATAI8)) { n = *((uint32_t *) p); p += (4 + n); }
    else if(i == instr_code(i_DATAI16)) { n = *((uint32_t *) p); p += (4 + (n * 2)); }
    else if(i == instr_code(i_DATAI32)) { n = *((uint32_t *) p); p += (4 + (n * 4)); }
    else if(i == instr_code(i_DATAI64)) { n = *((uint32_t *) p); p += (4 + (n * 8)); }
    else if(i == instr_code(i_DATAF)) { n = *((uint32_t *) p); p += (4 + (n * 8)); }
    else if(i == instr_code(i_COMMENT)) { n = *((uint16_t *) p); p += (2 + n); }        // comments have 2 bytes length
    else if(i == instr_code(i_NAME)) { n = *((uint16_t *) (p + 8)); p += (10 + n); }    // names have 8 bytes nonce + 2 bytes length
    else if(i == instr_code(i_LABEL)) p += (1 + *p);                                    // labels have 1 byte length
    *code = p;
}


// helper function used to determine the exact type of integer numbers
static void int_type(char **src, data_t *acc) {
    skip_spaces(src);
    char *s = *src;
    if(!strncmp(":1", s, 2) && *(s + 2) <= ' ') {
        s += 2; acc->params._all = T_INT1;
        acc->val.i = !!acc->val.i;  // convert the value to single bit
    }
    else if(!strncmp(":8", s, 2) && *(s + 2) <= ' ') { s += 2; acc->params._all = T_INT8; }
    else if(!strncmp(":16", s, 3) && *(s + 3) <= ' ') { s += 3; acc->params._all = T_INT16; }
    else if(!strncmp(":32", s, 3) && *(s + 3) <= ' ') { s += 3; acc->params._all = T_INT32; }
    else if(!strncmp(":64", s, 3) && *(s + 3) <= ' ') { s += 3; acc->params._all = T_INT64; }
    else {  // automatically determine the type
        acc->params._all = T_INT64;
        if(acc->val.i >= INT32_MIN && acc->val.i <= INT32_MAX) acc->params._all = T_INT32;
        if(acc->val.i >= INT16_MIN && acc->val.i <= INT16_MAX) acc->params._all = T_INT16;
        if(acc->val.i >= INT8_MIN && acc->val.i <= INT8_MAX) acc->params._all = T_INT8;
        if(acc->val.i >= 0 && acc->val.i <= 1) acc->params._all = T_INT1;
    }
    *src = s;
}


// get a single character from a source constant
// observes the pre-defined character constants and numeric codes
// will modify the global error code in case of a need
static char get_char(char **src) {
    char c = 0;
    char *s = *src;
    while(*s == '\\' && *(s + 1) <= ' ') {	// a single '\' character allows continuing the string on the next line
        s++;	// skip the '\'
        while(*s <= ' ') s++;
    }
    if(*s == '\"') c = '\0';    // a closing double quote will return 0
    else if(*s == '\\') {       // special character constants
        s++;	                // skip the '\' character
        if(*s == '0') c = '\0';
        else if(*s == 'n') c = '\n';
        else if(*s == 'r') c = '\r';
        else if(*s == 't') c = '\t';
        else if(*s == 'b') c = '\b';
        else if(*s == 'a') c = '\a';
        else if(*s == 'f') c = '\f';
        else if(*s == 'e') c = 27;
        else if(*s == '\\') c = '\\';
        else if(*s == '\'') c = '\'';
        else if(*s == '\"') c = '\"';
        else if(*s == '\?') c = '\?';
        else if(*s == 'x') {    // up to 2-digit hexadecimal ASCII code
            s++;
            c = (char) ((get_hex_digit(&s) << 4) + get_hex_digit(&s));
            s--;
        }
        else if(isdigitC(*s)) {  // up to 3-digit decimal ASCII code
            uint8_t t = (uint8_t) (*(s++) - '0');
            if(isdigitC(*s)) t = (t * 10) + (uint8_t) (*(s++) - '0');
            if(isdigitC(*s)) t = (t * 10) + (uint8_t) (*(s++) - '0');
            c = (char) t;
            s--;
        }
        else errcode = ERR_INV_CHAR;
        if(errcode == 0) s++;
    }
	else if(*s < ' ') errcode = ERR_INV_CHAR;
	else c = *(s++);
    *src = s;
    return c;
}


// get a numeric constant from text source and store it in (acc)
// will return an error code or 0 if successful
// will modify the source pointer accordingly
static int32_t get_number(char **source, data_t *acc, char brk_code) {
    char *src = *source;
    UInt64 i = 0;
    acc->params._all = T_VOID;   // this will serve as an invalid number indicator
    num_base = DEC;

    // hexadecimal unsigned constants staring with '0x'
    if(*src == '0' && (*(src + 1) == 'x' || *(src + 1) == 'X')) {
        num_base = HEX;
        uint8_t t = 16; // maximum number of hexadecimal digits
        src += 2;
        while(t-- && isXdigitC(*src)) i = (i << 4) + get_hex_digit(&src);
        if(t == 0) return ERR_INV_NUMBER;
        acc->val.i = i;
        int_type(&src, acc);
    }

    // octal unsigned constants staring with '0'
    else if(*src == '0' && isdigitC(*(src + 1))) {
        num_base = OCT;
        uint8_t t = 21; // maximum number of octal digits
        src++;  // skip the '0'
        if(*src > '7') return ERR_INV_NUMBER;
        while(t-- && *src >= '0' && *src <= '7') i = (i << 3) + (*(src++) - '0');
        if(t == 0) return ERR_INV_NUMBER;
        acc->val.i = i;
        int_type(&src, acc);
    }

    // binary unsigned constants staring with '0b'
    else if(*src == '0' && (*(src + 1) == 'b' || *(src + 1) == 'B')) {
        num_base = BIN;
        uint8_t t = 64; // maximum number of binary digits
        src += 2;
        while(t-- && (*src == '0' || *src == '1')) i = (i << 1) + (*(src++) - '0');
        if(t == 0) return ERR_INV_NUMBER;
        acc->val.i = i;
        int_type(&src, acc);
    }

    // decimal constants (optionally starting with '0d')
	// supported format: [sign] nnn [.nnn [ e/E [sign] nnn [.nnn] ] ]
    else {
        double v = 0.0; // fully constructed floating point value
        double f = 0.0; // factor
        double e = 0.0; // exponent
        int8_t dp = 0;  // 1:'decimal point found' flag
        int8_t ef = 0;  // 1:'E found' flag
        int8_t es = 1;  // 'E' sign 1:positive, -1:negative
        uint8_t t = 60; // maximum number of characters in the number
        if(*src == '0' && (*(src + 1) == 'd' || *(src + 1) == 'D')) src += 2;   // just skip the '0d' part
        if(!isdigitC(*src)) return ERR_INV_NUMBER;
        while(t-- && *src != 0 && *src != brk_code) {
            if(isdigitC(*src)) {
                if(!ef) {
                    if(!dp) i = (10 * i) + (*src - '0');
                    else { v += (f * (*src - '0')); f *= 0.1; }
                }
                else {
                    if(!dp) e = (10 * e) + (*src - '0');
                    else { e += (f * (*src - '0')); f *= 0.1; }
                }
                src++;
            }
            else {
                if(*src == '.') {
                    if(!dp) {
                        if(!ef) v = (double) i;
                        dp = 1; f = 0.1;
                        src++;
                    }
                    else return ERR_INV_NUMBER;
                }
                else if(*src == 'e' || *src == 'E') {
                    if(!ef) {
                        if(!dp) v = (double) i;
                        dp = 0; ef = 1;
                        src++;
                        if(*src == '-' || *src == '+') {
                            if(*(src++) == '-') { es = -1; f = 0.1; }
                        }
                    }
                    else return ERR_INV_NUMBER;
                }
                else break;
            }
        }
        if(t == 0) return ERR_INV_NUMBER;
        if(dp || ef) {  // floating point number
            v *= pow(10, (e * es));
            acc->val.f = v;
            acc->params._all = T_FLOAT;
        }
        else {          // integer number
            i *= (Int64) pow(10, (e * es));
            acc->val.i = i;
            int_type(&src, acc);
        }
    }

    *source = src;
    return 0;
}


// assembler
// the assembly process produces relocatable code
// the parameter (brk_code) must be set 0 to compile a full source text, or set 10 (\n) to compile a single line
// the input and output pointers are modified accordingly
// on a successful compilation, the compile code length in bytes is returned
// a negative result refers to an exit code as per the ERR_xxx constants
int32_t vm_assembler(char **source, uint8_t **destination, char brk_code) {
	char *src = *source;
	uint8_t *dst = *destination;
    uint8_t *start = *destination;
    uint8_t *end = *destination;
    uint8_t stage = 0;  // stage 0 will compile everything with void jump addresses
                        // stage 1 will recompile and fill the jump addresses
    uint8_t varf = 0;
    int32_t offs = 0;
    uint32_t idlen;
    errcode = 0;

    #define ND_DEPTH   10   // named definition maximum nesting depth
    uint8_t ndidx = 0;
    char *ndsrc[ND_DEPTH];

    // these codes are only a single character in the source
    char COMMENT = instr_text[instr_code(i_COMMENT)][0];
    char LABEL = instr_text[instr_code(i_LABEL)][0];
    char NAME = instr_text[instr_code(i_NAME)][0];

    #define next_stage() { stage++; end = dst; dst = start; if(!errcode) { src = *source; } continue; }

	while(errcode == 0 && stage < 2) {
        if(*src == '\0' || *src == brk_code) next_stage();    // end of source - change stage or finish
		skip_spaces(&src);
        if(*src == '\n') {  // store the new lines from the source as "nop" instructions
            if(ndidx) src = ndsrc[--ndidx];
            else { src++; *(dst++) = instr_code(i_NOP); }
            continue;
        }

        // source commentaries or name definitions
        if(*src == COMMENT) {
            uint16_t len;
            while((((uintptr_t) dst) & 1) != 1) *(dst++) = instr_code(i_PADNOP);
            for(len = 0, ++src; (*src >= ' ' || *src < 0); src++, len++) *(dst + 3 + len) = *src;
            *(dst++) = instr_code(i_COMMENT);
            *((uint16_t *) dst) = len;
            dst += (2 + len);
            continue;
        }

        // name definitions
        if(*src == NAME && isidchr1C(*(src + 1))) {
            char *ss = src + 1;
            uint16_t len;
            while((((uintptr_t) dst) & 7) != 7) *(dst++) = instr_code(i_PADNOP);
            for(len = 0, ++src; (*src >= ' ' || *src < 0); src++, len++) *(dst + 11 + len) = *src;
            *(dst++) = instr_code(i_NAME);
            *((uint64_t *) dst) = (uintptr_t) ss; // this is the "nonce" field
            *((uint16_t *) (dst + 8)) = len;
            dst += (10 + len);
            continue;
        }

        // jump labels
        if(*src == LABEL && isidchr1C(*(src + 1))) {
            uint8_t len;
            for(len = 0, ++src; isidchrC(*src); src++, len++) *(dst + 2 + len) = *src;
            *(dst++) = instr_code(i_LABEL);
            *(dst++) = len;
            dst += len;
            continue;
        }

        // predefined variable names V0 ... Vx
        // no 'continue' in this block as it prepares for the following numeric constant
        varf = 0;
        if((*src == 'V' || *src == 'v') && isdigitC(*(src + 1))) { src++; varf = 1; }

        // numeric constants
        // get the number and its type in (acc1)
        // bit length of integer numbers can be specified by adding :n immediately after the number
        // (eg. 0x05:8 defines an 8-bit integer, -22917:32 defines a 32-bit integer)
        // supported bit lengths are :1, :8, :16, :32, :64
        // if not explicitly specified, the bit length of integer numbers is automatically determined as per the value
        if(isdigitC(*src)) {
            data_t acc;
    		errcode = get_number(&src, &acc, brk_code);
            if(errcode < 0) next_stage();
            if(varf && (acc.val.i < 0 || acc.val.i >= VARIABLES)) { errcode = ERR_INV_VARN; next_stage(); }
            if(num_base == BIN) *(dst++) = instr_code(i_SETBIN);
            else if(num_base == OCT) *(dst++) = instr_code(i_SETOCT);
            else if(num_base == HEX) *(dst++) = instr_code(i_SETHEX);
            num_base = 0;   // reset the base after every constant
            switch(acc.params.type) {
                case T_INT1:
                    *(dst++) = (acc.val.i ? instr_code(i_CINT1) : instr_code(i_CINT0));
                    break;
                case T_INT8:
                    *(dst++) = instr_code(i_CINT8);
                    *((int8_t *) dst++) = (int8_t) acc.val.i;
                    break;
                case T_INT16:
                    while((((uintptr_t) dst) & 1) != 1) *(dst++) = instr_code(i_PADNOP);
                    *(dst++) = instr_code(i_CINT16);
                    *((int16_t *) dst) = (int16_t) acc.val.i;
                    dst += 2;
                    break;
                case T_INT32:
                    while((((uintptr_t) dst) & 3) != 3) *(dst++) = instr_code(i_PADNOP);
                    *(dst++) = instr_code(i_CINT32);
                    *((int32_t *) dst) = (int32_t) acc.val.i;
                    dst += 4;
                    break;
                case T_INT64:
                    while((((uintptr_t) dst) & 7) != 7) *(dst++) = instr_code(i_PADNOP);
                    *(dst++) = instr_code(i_CINT64);
                    //memset(dst, 0, 8);
                    *((Int64 *) dst) = (Int64) acc.val.i;
                    dst += 8;
                    break;
                case T_FLOAT:
                    if(varf) { errcode = ERR_INV_VARN; next_stage(); }
                    while((((uintptr_t) dst) & 7) != 7) *(dst++) = instr_code(i_PADNOP);
                    *(dst++) = instr_code(i_CFLOAT);
                    //memset(dst, 0, 8);
                    *((double *) dst) = (double) acc.val.f;
                    dst += 8;
                    break;
                default:    // expected to never come here
                    errcode = ERR_INV_NUMBER;
                    break;
            }
            varf = 0;
            continue;
        }
        if(varf) { errcode = ERR_INV_NUMBER; next_stage(); }

        // single quote character constants (8-bit integer)
        if(*src == '\'') {
            num_base = CHR;
            src++;  // skip the opening single quote
            *(dst++) = instr_code(i_SETCHR);
            *(dst++) = instr_code(i_CINT8);
            *(dst++) = get_char(&src);
            if(errcode == 0) {
                if(*src != '\'') { errcode = ERR_INV_CHAR; next_stage(); }
                src++;  // skip the closing single quote
            }
            num_base = DEC;
            continue;
        }

        // character strings
        if(*src == '\"') {
            num_base = STR;
            src++;  // skip the opening double quote
            *(dst++) = instr_code(i_SETSTR);
            while((((uintptr_t) dst) & 3) != 3) *(dst++) = instr_code(i_PADNOP);
            *(dst++) = instr_code(i_DATAI8);
            uint8_t *dlen = dst;
            dst += 4;
            uint32_t len = 0;
            char c;
            do {
                c = get_char(&src);
                *(dst++) = c;       // the trailing 0 will be stored and counted in the length as well
                len++;
            } while(c && errcode == 0);
            *((uint32_t *) dlen) = len; // store the recorded string length
            if(errcode == 0) {
                if(*src != '\"') errcode = ERR_INV_CHAR;
                src++;  // skip the closing double quote
            }
            num_base = DEC;
            continue;
        }

        // check in the reserved words
        {
            uint16_t ic;
            for(ic = 0; ic <= 0xFF; ic++) {
                if(*instr_text[ic] && !strncmp(src, instr_text[ic], strlen(instr_text[ic])) &&
                    *(src + strlen(instr_text[ic])) <= ' ') break;  // found a match
            }
            if(ic <= 0xFF) {
                if(ic == instr_code(i_DATAI8) || ic == instr_code(i_DATAI16) ||
                        ic == instr_code(i_DATAI32) || ic == instr_code(i_DATAI64) ||
                        ic == instr_code(i_DATAF)) {
                    while((((uintptr_t) dst) & 3) != 3) *(dst++) = instr_code(i_PADNOP);
                }
                *(dst++) = (uint8_t) ic;
                src += strlen(instr_text[ic]);
                uint8_t *dcnt = dst;
                uint32_t num = 0;
                data_t acc;

                // data statements
                if(ic == instr_code(i_DATAI8)) {
                    dst += 4;
                    while(1) {
                        skip_spaces(&src);
                        errcode = get_number(&src, &acc, brk_code);
                        if(errcode < 0) next_stage();
                        if(acc.params.type != T_INT1 && acc.params.type != T_INT8) { errcode = ERR_INV_RANGE; next_stage(); }
                        *(dst++) = (int8_t) acc.val.i;
                        num++;
                        skip_spaces(&src);
                        if(*src == ',') src++; else break;
                    }
                    if(errcode == 0) *((uint32_t *) dcnt) = num;
                }

                else if(ic == instr_code(i_DATAI16)) {
                    dst += 4;
                    while(1) {
                        skip_spaces(&src);
                        errcode = get_number(&src, &acc, brk_code);
                        if(errcode < 0) break;
                        if(acc.params.type != T_INT1 && acc.params.type != T_INT8 &&
                            acc.params.type != T_INT16) { errcode = ERR_INV_RANGE; next_stage(); }
                        acc.val.i = (int16_t) acc.val.i;
                        *((uint16_t *) dst) = acc.val.i;
                        dst += 2;
                        num++;
                        skip_spaces(&src);
                        if(*src == ',') src++; else break;
                    }
                    if(errcode == 0) *((uint32_t *) dcnt) = num;
                }

                else if(ic == instr_code(i_DATAI32)) {
                    dst += 4;
                    while(1) {
                        skip_spaces(&src);
                        errcode = get_number(&src, &acc, brk_code);
                        if(errcode < 0) next_stage();
                        if(acc.params.type != T_INT1 && acc.params.type != T_INT8 && acc.params.type != T_INT16 &&
                            acc.params.type != T_INT32) { errcode = ERR_INV_RANGE; next_stage(); }
                        acc.val.i = (int32_t) acc.val.i;
                        *((uint32_t *) dst) = acc.val.i;
                        dst += 4;
                        num++;
                        skip_spaces(&src);
                        if(*src == ',') src++; else break;
                    }
                    if(errcode == 0) *((uint32_t *) dcnt) = num;
                }

                else if(ic == instr_code(i_DATAI64)) {
                    dst += 4;
                    while(1) {
                        skip_spaces(&src);
                        errcode = get_number(&src, &acc, brk_code);
                        if(errcode < 0) next_stage();
                        if(acc.params.type != T_INT1 && acc.params.type != T_INT8 && acc.params.type != T_INT16 &&
                            acc.params.type != T_INT32 && acc.params.type != T_INT64) { errcode = ERR_INV_RANGE; next_stage(); }
                        acc.val.i = (Int64) acc.val.i;
                        memset(dst, 0, 8);
                        *((Int64 *) dst) = acc.val.i;
                        dst += 8;
                        num++;
                        skip_spaces(&src);
                        if(*src == ',') src++; else break;
                    }
                    if(errcode == 0) *((uint32_t *) dcnt) = num;
                }

                else if(ic == instr_code(i_DATAF)) {
                    dst += 4;
                    while(1) {
                        skip_spaces(&src);
                        errcode = get_number(&src, &acc, brk_code);
                        if(errcode < 0) next_stage();
                        if(acc.params.type != T_FLOAT) { errcode = ERR_INV_RANGE; next_stage(); }
                        acc.val.f = (double) acc.val.f;
                        memset(dst, 0, 8);
                        *((double *) dst) = acc.val.f;
                        dst += 8;
                        num++;
                        skip_spaces(&src);
                        if(*src == ',') src++; else break;
                    }
                    if(errcode == 0) *((uint32_t *) dcnt) = num;
                }

                continue;
            }
        }

        idlen = id_len(src);    // get the length of the identifier

        // check in the named definitions
        {
            uint8_t *p = start;
            uint8_t *found = NULL;
            uint8_t ff;
            uint16_t pl = 0;
            while(p < dst && found == NULL) {
                ff = 0;
                if(*p == instr_code(i_NAME)) {
                    pl = id_len((char *) p + 11);
                    if(idlen == pl && !strncmp(src, (char *) (p + 11), pl)) ff = 1;
                }
                if(ff) found = *((uint8_t **) (p + 1));
                instr_next(&p);
            }
            if(found) {
                if(ndidx >= ND_DEPTH) { errcode = ERR_NESTING_DEPTH; next_stage(); }
                ndsrc[ndidx++] = src + pl;
                src = (char *) found + pl;
                continue;
            }
        }

        // check in the defined jump labels
        if(stage) { // the second compilation stage has the labels already known
            uint8_t *p = start;
            uint8_t pl = 0;
            while(p < end) {
                if(*p == instr_code(i_LABEL)) {
                    pl = *(p + 1);
                    if(idlen == pl && !strncmp(src, (char *) (p + 2), pl)) break;
                }
                instr_next(&p);
            }
            if(p >= end) { errcode = ERR_UNKNOWN_ID; next_stage(); }
            p += (2 + pl);  // skip the label in the code
            while((((uintptr_t) dst) & 3) != 3) *(dst++) = instr_code(i_PADNOP);
            *(dst++) = instr_code(i_CINT32);
            offs = p - dst - 5; // the extra 5 bytes are coming from the CIN32 instruction
        }
        else {  // in the first compilation stage do the padding only
            while((((uintptr_t) dst) & 3) != 3) *(dst++) = instr_code(i_PADNOP);
            *(dst++) = instr_code(i_CINT32);
            offs = 0;
        }
        src += idlen;   // skip the identifier in the source
        *((int32_t *) dst) = offs;
        dst += 4;
        continue;

        errcode = ERR_SYNTAX;
	}

    *(end++) = instr_code(i_END);   // ensure proper program code termination with an END instruction
	*source = src;
	*destination = end;
	return (errcode ? errcode : (intptr_t) (end - start));
}

#endif
