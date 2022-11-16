#include "centipede.h"
#include <math.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <inttypes.h>

// the programming rules for this virtual machine are set with consideration of an eventual future hardware implementation:
// 1. no local variables, only global and minimum in number and size
// 2. no dynamic memory allocation
// 3. no third-party libraries and only standard data types

// definition of a single VM core structure
typedef struct {
	uint32_t instr_pointer;		// instruction address pointer
	uint8_t dst_index;			// current index of the data stack
	data_t dst[DST_DEPTH];		// data stack array
	uint8_t rst_index;			// current index of the return stack
	uint32_t rst[RST_DEPTH];	// return stack array
	data_t V[VARIABLES];		// variables data block
    uint32_t string_pointer;    // memory pointer for string transfers
    uint8_t str_buffer[STR_BUF_S];  // internal buffer for string conversions
} vm_core_t;

data_t acc1, acc2, acc3,
       acc4, acc5, acc6;        // internal data accumulators; shared among all cores
vm_core_t C[CORES];				// virtual machine core array
uint8_t number_cores;			// number of running cores
uint8_t core;					// currently maintained core
uint8_t *code;	                // only needed for emulation; points to the start of the program code

const double NaN = ((double) (0.0 / 0.0));  // NaN constant

// instruction codes
// the index in this array corresponds with the instruction code
void (*const instruction[256])(void) = {
/* 00 */	i_RESET,    i_,         i_ACTIVE,   i_CURRENT,  i_RETURN,   i_RUN,      i_STOP,     i_ARUN,
/* 08 */	i_CLEAR,    i_DEPTH,    i_DUP,      i_DROP,     i_OVER,     i_SWAP,     i_,         i_,
/* 10 */	i_CINT0,    i_CINT1,    i_CINT8,    i_CINT16,   i_CINT32,   i_CINT64,   i_CFLOAT,   i_,
/* 18 */	i_WHERE,    i_,         i_DATAI8,   i_DATAI16,  i_DATAI32,  i_DATAI64,  i_DATAF,    i_,
/* 20 */	i_FREE,     i_VINT1,    i_VINT8,    i_VINT16,   i_VINT32,   i_VInt64,   i_VFLOAT,   i_,
/* 28 */	i_GOTO,     i_CALL,     i_AGOTO,    i_ACALL,    i_GOTOIF,   i_CALLIF,   i_AGOTOIF,  i_ACALLIF,
/* 30 */	i_EQ,       i_NEQ,      i_SM,       i_SMEQ,     i_GR,       i_GREQ,     i_SET,      i_GET,
/* 38 */	i_SHIFTL,   i_SHIFTR,   i_OR,       i_EXOR,     i_AND,      i_INVERT,   i_,         i_NOT,
/* 40 */	i_SUB,      i_ADD,      i_MUL,      i_DIV,      i_IDIV,     i_MOD,      i_,         i_,
/* 48 */	i_DEC,      i_INC,      i_VDEC,     i_VINC,     i_,         i_,         i_,         i_,
/* 50 */	i_NINF,     i_PINF,     i_,         i_NAN,      i_,         i_,         i_,         i_,
/* 58 */	i_PI,       i_E,        i_,         i_,         i_,         i_,         i_,         i_,
/* 60 */	i_RAND,     i_,         i_,         i_ABS,      i_ROUND,    i_TRUNC,    i_FRACT,    i_,
/* 68 */	i_LOGD,     i_LOGN,     i_EXP,      i_POWER,    i_ROOT2,    i_ROOT3,    i_,         i_,
/* 70 */	i_SIN,      i_COS,      i_ASIN,     i_ACOS,     i_HSIN,     i_HCOS,     i_,         i_,
/* 78 */	i_TAN,      i_COTAN,    i_ATAN,     i_,         i_HTAN,     i_,         i_,         i_,
/* 80 */	i_DBASE,    i_PBASE,    i_DSIZE,    i_PSIZE,    i_,         i_,         i_,         i_,
/* 88 */	i_MEMINIT,  i_STRLEN,   i_MEMCOPY,  i_STRCOPY,  i_MEMCOMP,  i_STRCOMP,  i_,         i_,
/* 90 */	i_,         i_,         i_,         i_,         i_,         i_,         i_,         i_,
/* 98 */	i_,         i_,         i_,         i_,         i_,         i_,         i_,         i_,
/* A0 */	i_,         i_,         i_,         i_,         i_,         i_,         i_,         i_,
/* A8 */	i_,         i_,         i_,         i_,         i_,         i_,         i_,         i_,
/* B0 */	i_,         i_,         i_,         i_,         i_,         i_,         i_,         i_,
/* B8 */	i_,         i_,         i_,         i_,         i_,         i_,         i_,         i_,
/* C0 */	i_,         i_,         i_,         i_,         i_,         i_,         i_,         i_,
/* C8 */	i_,         i_,         i_,         i_,         i_,         i_,         i_,         i_,
/* D0 */	i_,         i_,         i_,         i_,         i_,         i_,         i_,         i_,
/* D8 */	i_,         i_,         i_,         i_,         i_,         i_,         i_,         i_,
/* E0 */	i_,         i_,         i_,         i_,         i_,         i_,         i_,         i_,
/* E8 */	i_,         i_,         i_,         i_,         i_,         i_,         i_,         i_,
/* F0 */	i_NOP,      i_PADNOP,   i_SETBIN,   i_SETOCT,   i_SETHEX,   i_SETCHR,   i_SETSTR,   i_COMMENT,
/* F8 */	i_GETCHR,   i_PEEKCHR,  i_PUTCHR,   i_PUTSTR,   i_PUTVAL,   i_NAME,     i_LABEL,    i_END,
};

// instruction words as text
// each instruction must not be longer than 7 characters (the 8th is reserved for a closing 0)
// the index in this array corresponds with the instruction code
const char instr_text[256][8] = {
/* 00 */    "reset",    "",         "active",   "current",  "return",   "run",      "stop",     "arun",
/* 08 */    "clear",    "depth",    "dup",      "drop",     "over",     "swap",     "",         "",
/* 10 */    "const0",   "const1",   "cint8",    "cint16",   "cint32",   "cint64",	"cfloat",   "",
/* 18 */    "where",    "",         "datai8",   "datai16",  "datai32",  "datai64",  "dataf",    "",
/* 20 */    "free",     "vint1",    "vint8",    "vint16",   "vint32",   "vint64",   "vfloat",   "",
/* 28 */    "goto",     "call",     "agoto",    "acall",    "gotoif",   "callif",   "agotoif",  "acallif",
/* 30 */    "==",       "<>",       "<",        "<=",       ">",        ">=",       "=",        "?",
/* 38 */    "shiftl",   "shiftr",   "or",       "exor",     "and",      "invert",   "",         "not",
/* 40 */    "-",        "+",        "*",        "/",        "idiv",     "mod",      "",         "",
/* 48 */    "--",       "++",       "v--",      "v++",      "",         "",         "",         "",
/* 50 */    "-INF",     "+INF",     "",         "NaN",      "",         "",         "",         "",
/* 58 */    "_pi",      "_e",       "",         "",         "",         "",         "",         "",
/* 60 */    "rand",     "",         "",         "abs",      "round",    "trunc",    "fract",    "",
/* 68 */    "logd",     "logn",     "exp",      "power",    "root2",    "root3",    "",         "",
/* 70 */    "sin",      "cos",      "asin",     "acos",     "hsin",     "hcos",     "",         "",
/* 78 */    "tan",      "cotan",    "atan",     "",         "htan",     "",         "",         "",
/* 80 */    "Dbase",    "Pbase",    "Dsize",    "Psize",    "",         "",         "",         "",
/* 88 */    "meminit",  "strlen",   "memcopy",  "strcopy",  "memcomp",  "strcomp",  "",         "",
/* 90 */    "",         "",         "",         "",         "",         "",         "",         "",
/* 98 */    "",         "",         "",         "",         "",         "",         "",         "",
/* A0 */    "",         "",         "",         "",         "",         "",         "",         "",
/* A8 */    "",         "",         "",         "",         "",         "",         "",         "",
/* B0 */    "",         "",         "",         "",         "",         "",         "",         "",
/* B8 */    "",         "",         "",         "",         "",         "",         "",         "",
/* C0 */    "",         "",         "",         "",         "",         "",         "",         "",
/* C8 */    "",         "",         "",         "",         "",         "",         "",         "",
/* D0 */    "",         "",         "",         "",         "",         "",         "",         "",
/* D8 */    "",         "",         "",         "",         "",         "",         "",         "",
/* E0 */    "",         "",         "",         "",         "",         "",         "",         "",
/* E8 */    "",         "",         "",         "",         "",         "",         "",         "",
//          nop       padding nop   setbin      setoct      sethex      setchr      setstr     comment
/* F0 */    "nop",      ".",        "setbin",   "setoct",   "sethex",   "setchr",   "setstr",   "#",
/* F8 */    "getchr",   "peekchr",  "putchr",   "putstr",   "putval",   "!",        "@",        "END",
};


// main VM function
__attribute__ ((optimize("-O3")))
void vm_run(uint8_t *Code) {
    con_rx_in = con_rx_out = 0;
    con_tx_in = con_tx_out = 0;
    memset((uint8_t *) con_rx_buf, 0, CONSOLE_RX_BUF_S);
    memset((uint8_t *) con_tx_buf, 0, CONSOLE_TX_BUF_S);
	for(code = Code, core = 0, i_RESET();
		#ifndef PIC32MX1
			instruction[code[C[core].instr_pointer]] != i_END	// allow exit from this loop in generic mode
		#endif
		; instruction[code[C[core].instr_pointer++]]()) {		// enter the current instruction pointing the first byte after

        #if CONSOLE_BLOCKING == 0   // non-blocking output to the console
            if(con_tx_out != con_tx_in) {
                putchar((char) con_tx_buf[con_tx_out]);     	// output to the console
                if(++con_tx_out >= CONSOLE_TX_BUF_S) con_tx_out = 0;
            }
            if(C[core].string_pointer) {    // strings are output without holding up the execution of other threads
                if(*((uint8_t *) (uintptr_t) &code[C[core].string_pointer])) {
                    if(((con_tx_in + 1) % CONSOLE_TX_BUF_S) != con_tx_out) {    // act only if there is room in the buffer
                        con_tx_buf[con_tx_in] = *((uint8_t *) (uintptr_t) &code[C[core].string_pointer++]);
                        if(++con_tx_in >= CONSOLE_TX_BUF_S) con_tx_in = 0;
                    }
                }
                else C[core].string_pointer = 0;
            }
        #endif

        #if CONSOLE_BREAK_CODE > 0          // execution break key
            if(con_rx_out != con_rx_in && con_rx_buf[con_rx_out] == CONSOLE_BREAK_CODE) {
                i_RESET();
                monitor(1);
                while(1);   // halt execution indefinitely
            }
        #endif

        #if CORES > 1   // multi-core execution
            core = ((++core < number_cores) ? core : 0);    // switch to executing the next parallel core (if any)
        #endif

	}
	instruction[code[C[core].instr_pointer]]();	// this is always the i_END() instruction
}


// make copy of a data structure
//#define copyData(dd, ds) memcpy(dd, ds, sizeof(data_t))
__inline__ __attribute__ ((optimize("-O3")))
static void copyData(data_t *dd, data_t *ds) {
    dd->params._all = ds->params._all;
    dd->val.f = ds->val.f;  // since (.val) is a union, just copying the largest element from it
}


// push data into the data stack
__inline__ __attribute__ ((optimize("-O3")))
static void push(data_t *d) {
	if(C[core].dst_index >= DST_DEPTH) {
		#ifdef ENABLE_STOFL_INT
			exception(INT_STOFL);
			return;
		#else
			C[core].dst_index = DST_DEPTH - 1;
			memmove(&C[core].dst[0], &C[core].dst[1], C[core].dst_index * sizeof(data_t));
		#endif
	}
	copyData(&C[core].dst[C[core].dst_index++], d);
}


/*
NOTE: call table for conversions did not yield higher execution speed

// data conversion functions

void cvt_VOID(data_t *d) {}
void cvt_EXTFP(data_t *d) {}

void cvt_INT1(data_t *d) {
    if(d->params.type == T_FLOAT) d->val.i = ((d->val.f != 0.0) ? 1 : 0);
    else d->val.i = ((d->val.i != 0) ? 1 : 0);
}

void cvt_INT8(data_t *d) {
    if(d->params.type == T_FLOAT) d->val.i = (int8_t) d->val.f;
    else d->val.i = (int8_t) d->val.i;
}

void cvt_INT16(data_t *d) {
    if(d->params.type == T_FLOAT) d->val.i = (int16_t) d->val.f;
    else d->val.i = (int16_t) d->val.i;
}

void cvt_INT32(data_t *d) {
    if(d->params.type == T_FLOAT) d->val.i = (int32_t) d->val.f;
    else d->val.i = (int32_t) d->val.i;
}

void cvt_INT64(data_t *d) {
    if(d->params.type == T_FLOAT) d->val.i = (Int64) d->val.f;
    else d->val.i = (Int64) d->val.i;
}

void cvt_FLOAT(data_t *d) {
    if(d->params.type == T_FLOAT) d->val.f = (double) d->val.f;
    else d->val.f = (double) d->val.i;
}


// call references to the data conversion functions
// NOTE: must match the exact order of the data types as per the T_xxx constants
void (*const convert[8])(data_t *d) = {
    cvt_VOID, cvt_INT1, cvt_INT8, cvt_INT16, cvt_INT32, cvt_INT64, cvt_FLOAT, cvt_EXTFP
};
*/


// pull data from the data stack
// data type as defined in the T_xxx constants (use T_VOID for no conversion)
__inline__ __attribute__ ((optimize("-O3")))
static void pull(data_t *d, uint8_t type) {
	if(C[core].dst_index == 0) {
		#ifdef ENABLE_STUFL_INT
			exception(INT_STUFL);
			return;
		#else
			C[core].dst_index = 1;
		#endif
	}
	copyData(d, &C[core].dst[--C[core].dst_index]);
    //convert[type](d);   // perform data type conversion through a call table
	switch(type) {      // perform data type conversion in the traditional way
		case T_INT1:
			if(d->params.type == T_FLOAT) d->val.i = ((d->val.f != 0.0) ? 1 : 0);
			else d->val.i = ((d->val.i != 0) ? 1 : 0);
			break;
		case T_INT8:
			if(d->params.type == T_FLOAT) d->val.i = (int8_t) d->val.f;
			else d->val.i = (int8_t) d->val.i;
			break;
		case T_INT16:
			if(d->params.type == T_FLOAT) d->val.i = (int16_t) d->val.f;
			else d->val.i = (int16_t) d->val.i;
			break;
		case T_INT32:
			if(d->params.type == T_FLOAT) d->val.i = (int32_t) d->val.f;
			else d->val.i = (int32_t) d->val.i;
			break;
		case T_INT64:
			if(d->params.type == T_FLOAT) d->val.i = (Int64) d->val.f;
			else d->val.i = (Int64) d->val.i;
			break;
		case T_FLOAT:
			if(d->params.type == T_FLOAT) d->val.f = (double) d->val.f;
			else d->val.f = (double) d->val.i;
			break;
		default: break;
	}
	if(type != T_VOID) d->params._all = type;
}


// create a system exception
// push (xcode) into the stack and reset the current execution core
void exception(uint32_t xcode) {
	memset(&C[core], 0, sizeof(C[core]));
	acc1.params._all = T_INT32;
	acc1.val.i = xcode;
	push(&acc1);
}


// invalid instruction
void i_(void) {
	exception(INT_INVALID);
}


// no operation
void i_NOP(void) { }
void i_PADNOP(void) { }
void i_SETBIN(void) { }
void i_SETOCT(void) { }
void i_SETHEX(void) { }
void i_SETCHR(void) { }
void i_SETSTR(void) { }


// return the physical address of start of the data memory (RAM)
void i_DBASE(void) {
	acc1.val.i = (uint32_t) (uintptr_t) Centipede_RAM;
	acc1.params._all = T_INT32;
	push(&acc1);
}


// return the physical address of start of the program memory (ROM)
void i_PBASE(void) {
	acc1.val.i = (uint32_t) (uintptr_t) Centipede_ROM;
	acc1.params._all = T_INT32;
	push(&acc1);
}


// return the total size the data memory (RAM) in bytes
void i_DSIZE(void) {
    acc1.val.i = 1024ul * RAM_KB;
	acc1.params._all = T_INT64;
	push(&acc1);
}


// return the total size the program memory (ROM) in bytes
void i_PSIZE(void) {
    acc1.val.i = 1024ul * ROM_KB;
	acc1.params._all = T_INT64;
	push(&acc1);
}


// mark end of program code
void i_END(void) {
    acc2.val.i = 0;
    for(acc1.val.i = 0; acc1.val.i < number_cores; acc1.val.i++) {
        if(C[acc1.val.i].string_pointer) acc2.val.i++;
    }
    if(acc2.val.i) { C[core].instr_pointer--; return; } // stay on this instruction until all string transfers completes
    i_RESET();
    exception(INT_END);
    #ifdef PIC32MX1
        monitor(0); // the Monitor exits only by resetting the system
    #endif
}


// full VM reset
void i_RESET(void) {
	if(core == 0) {
		number_cores = 1;
		memset(&C, 0, sizeof(C));	// reset all cores
	}
	exception(INT_RESET);
}


// skip number of bytes set in the following two bytes
void i_COMMENT(void) {
    acc1.val.i = *((uint16_t *) &code[C[core].instr_pointer]);
    C[core].instr_pointer += (2 + acc1.val.i);
}


// skip number of bytes set in the following two bytes
void i_NAME(void) {
	C[core].instr_pointer += 8;	// skip the "nonce" field
    acc1.val.i = *((uint16_t *) &code[C[core].instr_pointer]);
    C[core].instr_pointer += (2 + acc1.val.i);
}


// skip number of bytes set in the following one byte
void i_LABEL(void) {
    acc1.val.i = *((uint8_t *) &code[C[core].instr_pointer]);
    C[core].instr_pointer += (1 + acc1.val.i);
}


// return the current instruction address
void i_WHERE(void) {
	acc1.params._all = T_INT32;
	acc1.val.i = C[core].instr_pointer;
	push(&acc1);
}


// return the total number of execution cores
void i_CORES(void) {
	acc1.params._all = T_INT64;
	acc1.val.i = CORES;
	push(&acc1);
}


// return the number of currently running cores
void i_ACTIVE(void) {
	acc1.params._all = T_INT64;
	acc1.val.i = number_cores;
	push(&acc1);
}


// return the number of the current core
void i_CURRENT(void) {
	acc1.params._all = T_INT64;
	acc1.val.i = core;
	push(&acc1);
}


// clear the data stack and reset the stack pointer
void i_CLEAR(void) {
	C[core].dst_index = 0;
	memset(&C[core].dst, 0, sizeof(C[core].dst));
}


// int1 constant (with value 0)
void i_CINT0(void) {
	acc1.params._all = T_INT1;
	acc1.val.i = 0;
	push(&acc1);
}


// int1 constant (with value 1)
void i_CINT1(void) {
	acc1.params._all = T_INT1;
	acc1.val.i = 1;
	push(&acc1);
}


// int8 constant
void i_CINT8(void) {
	acc1.params._all = T_INT8;
    acc1.val.i = *((int8_t *) &code[C[core].instr_pointer]);
	C[core].instr_pointer += 1;
	push(&acc1);
}


// int16 constant
void i_CINT16(void) {
	acc1.params._all = T_INT16;
    acc1.val.i = *((int16_t *) (uintptr_t) &code[C[core].instr_pointer]);
	C[core].instr_pointer += 2;
	push(&acc1);
}


// int32 constant
void i_CINT32(void) {
	acc1.params._all = T_INT32;
    acc1.val.i = *((int32_t *) (uintptr_t) &code[C[core].instr_pointer]);
	C[core].instr_pointer += 4;
	push(&acc1);
}


// int64 constant
void i_CINT64(void) {
	acc1.params._all = T_INT64;
	acc1.val.i = *((Int64 *) (uintptr_t) &code[C[core].instr_pointer]);
	C[core].instr_pointer += 8;
	push(&acc1);
}


// floating point constant
void i_CFLOAT(void) {
	acc1.params._all = T_FLOAT;
    acc1.val.f = *((double *) (uintptr_t) &code[C[core].instr_pointer]);
	C[core].instr_pointer += 8;
	push(&acc1);
}


// int8 data constants
void i_DATAI8(void) {
    acc2.val.i = *((uint32_t *) &code[C[core].instr_pointer]);  // number of elements
 	C[core].instr_pointer += 4;	// skip the number of elements
	i_WHERE();
	C[core].instr_pointer += acc2.val.i;
}


// int16 data constants
void i_DATAI16(void) {
 acc2.val.i = *((uint32_t *) (uintptr_t) &code[C[core].instr_pointer]);  // number of elements
	C[core].instr_pointer += 4;	// skip the number of elements
	i_WHERE();
	C[core].instr_pointer += (2 * acc2.val.i);
}


// int32 data constants
void i_DATAI32(void) {
 acc2.val.i = *((uint32_t *) (uintptr_t) &code[C[core].instr_pointer]);  // number of elements
	C[core].instr_pointer += 4;	// skip the number of elements
	i_WHERE();
	C[core].instr_pointer += (4 * acc2.val.i);
}


// int64 data constants
void i_DATAI64(void) {
    acc2.val.i = *((uint32_t *) (uintptr_t) &code[C[core].instr_pointer]);  // number of elements
	C[core].instr_pointer += 4;	// skip the number of elements
	i_WHERE();
	C[core].instr_pointer += (8 * acc2.val.i);
}


// fp data constants
void i_DATAF(void) {
    acc2.val.i = *((uint32_t *) (uintptr_t) &code[C[core].instr_pointer]);  // number of elements
	C[core].instr_pointer += 4;	// skip the number of elements
	i_WHERE();
	C[core].instr_pointer += (8 * acc2.val.i);
}


// free up a variable
void i_FREE(void) {
	pull(&acc1, T_INT32);	// get the var id
	if((acc1.val.i < 0) || (acc1.val.i >= VARIABLES)) { exception(INT_INVVID); return; }
	C[core].V[(uint32_t) acc1.val.i].params._all = T_VOID;
}


// new generic variable
__inline__ __attribute__ ((optimize("-O3")))
static void newvar(uint8_t type) {
	pull(&acc1, T_INT32); // variable id
	if((acc1.val.i < 0) || (acc1.val.i >= VARIABLES)) { exception(INT_INVVID); return; }
	pull(&acc2, T_INT32); // dimension
	if((acc2.val.i < 0) || (acc2.val.i >= (1ul << (32 - TYPE_BITS)))) { exception(INT_INVDIM); return; }
	#ifdef ENABLE_REDEF_INT
		if(C[core].V[acc1.val.i].params.type != T_VOID) { exception(INT_REDEF); return; }
	#endif
	memset(&C[core].V[acc1.val.i], 0, sizeof(data_t));
    C[core].V[acc1.val.i].params.type = type;
	C[core].V[acc1.val.i].params.dim = acc2.val.i;
}


// new var1 variable
void i_VINT1(void) {
	newvar(T_INT1);
}


// new var8 variable
void i_VINT8(void) {
	newvar(T_INT8);
}


// new var16 variable
void i_VINT16(void) {
	newvar(T_INT16);
}


// new var32 variable
void i_VINT32(void) {
	newvar(T_INT32);
}


// new var64 variable
void i_VInt64(void) {
	newvar(T_INT64);
}


// new float variable
void i_VFLOAT(void) {
	newvar(T_FLOAT);
}


// get value from variable
__attribute__ ((optimize("-O3")))
void i_GET(void) {
	pull(&acc1, T_INT32);	// get the variable id
	if((acc1.val.i < 0) || (acc1.val.i >= VARIABLES)) { exception(INT_INVVID); return; }
	if(C[core].V[acc1.val.i].params.dim) {	// is it an array variable?
		pull(&acc2, T_INT32);	// get the index
		if((acc2.val.i < 0) || (acc2.val.i > C[core].V[acc1.val.i].params.dim)) {   // out of range
			#if INDEX_BEHAVIOUR == 2
				acc2.val.i = (acc2.val.i % C[core].V[acc1.val.i].params.dim) + 1;
			#elif INDEX_BEHAVIOUR == 1
				exception(INT_INVDIM);
				return;
			#endif
		}
		if(acc2.val.i) {    // is it an index in the array?
			acc2.val.i--;	// indexes start from 1 so the value needs to be adjusted to start from 0
			switch(C[core].V[acc1.val.i].params.type) {
				case T_INT1:
					acc2.val.i = !!(*((int8_t *) (uintptr_t) C[core].V[acc1.val.i].val.i
                                        + (acc2.val.i / 8)) & BIT(acc2.val.i % 8));
					break;
				case T_INT8:
					acc2.val.i = *((int8_t *) (uintptr_t) C[core].V[acc1.val.i].val.i + acc2.val.i);
					break;
				case T_INT16:
                    acc2.val.i = *((int16_t *) (uintptr_t) C[core].V[acc1.val.i].val.i + (2 * acc2.val.i));
					break;
				case T_INT32:
					acc2.val.i = *((int32_t *) (uintptr_t) C[core].V[acc1.val.i].val.i + (4 * acc2.val.i));
					break;
				case T_INT64:
					acc2.val.i = *((Int64 *) (uintptr_t) C[core].V[acc1.val.i].val.i + (8 * acc2.val.i));
					break;
				case T_FLOAT:
                    acc2.val.f = *((double *) (uintptr_t) C[core].V[acc1.val.i].val.i + (8 * acc2.val.i));
					break;
				default: break;	// nothing to do with unknown types
			}
			acc2.params._all = C[core].V[acc1.val.i].params.type;
		}
		else {  // index 0 is the core variable of the array
			acc2.val.i = C[core].V[acc1.val.i].val.i;	// read from the core variable (array base address)
			acc2.params._all = T_INT64;
		}
	}
	else copyData(&acc2, &C[core].V[acc1.val.i]);       // read from a single variable
	push(&acc2);
}


// set value into variable
__attribute__ ((optimize("-O3")))
void i_SET(void) {
	pull(&acc1, T_INT32);	// get the variable id
	if((acc1.val.i < 0) || (acc1.val.i >= VARIABLES)) { exception(INT_INVVID); return; }
	if(C[core].V[acc1.val.i].params.dim) {  // is it an array variable?
		pull(&acc2, T_INT32);	// get the index
		if((acc2.val.i < 0) || (acc2.val.i > (C[core].V[acc1.val.i].params.dim))) { // out of range
			#if INDEX_BEHAVIOUR == 2
				acc2.val.i = (acc2.val.i % C[core].V[acc1.val.i].params.dim) + 1;
			#elif INDEX_BEHAVIOUR == 1
				exception(INT_INVDIM);
				return;
			#endif
		}
		if(acc2.val.i) {    // is it an index in the array?
			acc2.val.i--;	// indexes start from 1 so the value needs to be adjusted to start from 0
			switch(C[core].V[acc1.val.i].params.type) {
				case T_INT1:
					pull(&acc3, T_INT1);
					if(acc3.val.i)
						*((int8_t *) (uintptr_t) C[core].V[acc1.val.i].val.i + (acc2.val.i / 8)) |= BIT(acc2.val.i % 8);
					else
						*((int8_t *) (uintptr_t) C[core].V[acc1.val.i].val.i + (acc2.val.i / 8)) &= ~BIT(acc2.val.i % 8);
					break;
				case T_INT8:
					pull(&acc3, T_INT8);
					*((int8_t *) (uintptr_t) C[core].V[acc1.val.i].val.i + acc2.val.i) = acc3.val.i;
					break;
				case T_INT16:
					pull(&acc3, T_INT16);
                    *((int16_t *) (uintptr_t) C[core].V[acc1.val.i].val.i + (2 * acc2.val.i)) = acc3.val.i;
					break;
				case T_INT32:
					pull(&acc3, T_INT32);
					*((int32_t *) (uintptr_t) C[core].V[acc1.val.i].val.i + (4 * acc2.val.i)) = acc3.val.i;
					break;
				case T_INT64:
					pull(&acc3, T_INT64);
                    *((Int64 *) (uintptr_t) C[core].V[acc1.val.i].val.i + (8 * acc2.val.i)) = acc3.val.i;
					break;
				case T_FLOAT:
					pull(&acc3, T_FLOAT);
                    *((double *) (uintptr_t) C[core].V[acc1.val.i].val.i + (8 * acc2.val.i)) = acc3.val.f;
					break;
				default: 	// nothing to do with unknown types but still need to pull the parameter
					pull(&acc3, T_VOID);
					break;
			}
		}
		else {  // index 0 is the core variable of the array
			pull(&acc3, T_INT64);
			C[core].V[acc1.val.i].val.i = acc3.val.i;	// write into the core variable (array base address)
		}
	}
	else {  // single variable
		switch(C[core].V[acc1.val.i].params.type) {     // write into a single variable
			case T_INT1:
				pull(&acc3, T_INT1);
				C[core].V[acc1.val.i].val.i = !!acc3.val.i;
				break;
			case T_INT8:
				pull(&acc3, T_INT8);
				C[core].V[acc1.val.i].val.i = acc3.val.i;
				break;
			case T_INT16:
				pull(&acc3, T_INT16);
				C[core].V[acc1.val.i].val.i = acc3.val.i;
				break;
			case T_INT32:
				pull(&acc3, T_INT32);
				C[core].V[acc1.val.i].val.i = acc3.val.i;
				break;
			case T_INT64:
				pull(&acc3, T_INT64);
				C[core].V[acc1.val.i].val.i = acc3.val.i;
				break;
			case T_FLOAT:
				pull(&acc3, T_FLOAT);
				C[core].V[acc1.val.i].val.f = acc3.val.f;
				break;
			default: 	// nothing to do with unknown types but still need to pull the parameter
				pull(&acc3, T_VOID);
				break;
		}
	}
}


// increase the value in variable
void i_VINC(void) {
    pull(&acc5, T_VOID);    // store the variable id
    if(C[core].V[acc5.val.i].params.dim) {  // is it an array variable?
        pull(&acc6, T_VOID);
        push(&acc6);        // pull the index and store it, then push back
    }
    push(&acc5);            // push back the variable id
    i_GET();
    i_INC();
    if(C[core].V[acc5.val.i].params.dim) push(&acc6);   // restore the index for arrays
    push(&acc5);            // push again the variable id
    i_SET();
}


// decrease the value in variable
void i_VDEC(void) {
    pull(&acc5, T_VOID);    // store the variable id
    if(C[core].V[acc5.val.i].params.dim) {  // is it an array variable?
        pull(&acc6, T_VOID);
        push(&acc6);        // pull the index and store it, then push back
    }
    push(&acc5);            // push back the variable id
    i_GET();
    i_DEC();
    if(C[core].V[acc5.val.i].params.dim) push(&acc6);   // restore the index for arrays
    push(&acc5);            // push again the variable id
    i_SET();
}


// return current stack depth
void i_DEPTH(void) {
	acc1.params._all = T_INT32;
	acc1.val.i = C[core].dst_index;
	push(&acc1);
}


// duplicate the top element
void i_DUP(void) {
	pull(&acc1, T_VOID);
	push(&acc1);
	push(&acc1);
}


// remove the top element
void i_DROP(void) {
	pull(&acc1, T_VOID);
}


// copy on top an element from a greater depth
void i_OVER(void) {
	pull(&acc1, T_INT32);
	acc1.val.i = (C[core].dst_index - 1) - acc1.val.i;
	if((acc1.val.i < 0) || (acc1.val.i > (C[core].dst_index - 1))) {
		#if INVDEPTH_BEHAVIOUR == 0
			if(acc1.val.i > (C[core].dst_index - 1)) acc1.val.i = (C[core].dst_index - 1);
			if(acc1.val.i < 0) acc1.val.i = 0;
		#else
			exception(INT_INVSTX);
			return;
		#endif
	}
	copyData(&acc1, &C[core].dst[acc1.val.i]);
	push(&acc1);
}


// swap the top of the stack with an element from a greater depth
void i_SWAP(void) {
	pull(&acc1, T_INT32);
	acc1.val.i = (C[core].dst_index - 1) - acc1.val.i;
	if((acc1.val.i < 0) || (acc1.val.i > (C[core].dst_index - 1))) {
		#if INVDEPTH_BEHAVIOUR == 0
			if(acc1.val.i > (C[core].dst_index - 1)) acc1.val.i = (C[core].dst_index - 1);
			if(acc1.val.i < 0) acc1.val.i = 0;
		#else
			exception(INT_INVSTX);
			return;
		#endif
	}
	copyData(&acc2, &C[core].dst[acc1.val.i]);
	pull(&acc3, T_VOID);
	push(&acc2);
	copyData(&C[core].dst[acc1.val.i], &acc2);
}


// jump to a relative address
void i_GOTO(void) {
	pull(&acc1, T_INT32);
	C[core].instr_pointer += acc1.val.i;
}


// call a function at a relative address
void i_CALL(void) {
	pull(&acc1, T_INT32);
	if(C[core].rst_index >= RST_DEPTH) { exception(INT_RETSTOFL); return; }
	C[core].rst[C[core].rst_index++] = C[core].instr_pointer;
	C[core].instr_pointer += acc1.val.i;
}


// jump to an absolute address
void i_AGOTO(void) {
	pull(&acc1, T_INT64);
	C[core].instr_pointer = (uint32_t) acc1.val.i;
}


// call a function at an absolute address
void i_ACALL(void) {
	pull(&acc1, T_INT64);
	if(C[core].rst_index >= RST_DEPTH) { exception(INT_RETSTOFL); return; }
	C[core].rst[C[core].rst_index++] = C[core].instr_pointer;
	C[core].instr_pointer = (uint32_t) acc1.val.i;
}


// conditional jump to a relative address
void i_GOTOIF(void) {
	pull(&acc1, T_INT32);
	pull(&acc2, T_VOID);
	switch(acc2.params.type) {
		case T_INT1:
		case T_INT8:
		case T_INT16:
		case T_INT32:
		case T_INT64:
			if(acc2.val.i != 0) C[core].instr_pointer += acc1.val.i;
			break;
		case T_FLOAT:
			if(acc2.val.f != 0.0) C[core].instr_pointer += acc1.val.i;
			break;
		default: break;
	}
}


// conditional call a function at a relative address
void i_CALLIF(void) {
	pull(&acc1, T_INT32);
	pull(&acc2, T_VOID);
	if(C[core].rst_index >= RST_DEPTH) { exception(INT_RETSTOFL); return; }
	switch(acc2.params.type) {
		case T_INT1:
		case T_INT8:
		case T_INT16:
		case T_INT32:
		case T_INT64:
			if(acc2.val.i != 0) {
				C[core].rst[C[core].rst_index++] = C[core].instr_pointer;
				C[core].instr_pointer += acc1.val.i;
			}
			break;
		case T_FLOAT:
			if(acc2.val.f != 0.0) {
				C[core].rst[C[core].rst_index++] = C[core].instr_pointer;
				C[core].instr_pointer += acc1.val.i;
			}
			break;
		default: break;
	}
}


// conditional jump to an absolute address
void i_AGOTOIF(void) {
	pull(&acc1, T_INT64);
	pull(&acc2, T_VOID);
	switch(acc2.params.type) {
		case T_INT1:
		case T_INT8:
		case T_INT16:
		case T_INT32:
		case T_INT64:
			if(acc2.val.i != 0) C[core].instr_pointer = (uint32_t) acc1.val.i;
			break;
		case T_FLOAT:
			if(acc2.val.f != 0.0) C[core].instr_pointer = (uint32_t) acc1.val.i;
			break;
		default: break;
	}
}


// conditional call a function at an absolute address
void i_ACALLIF(void) {
	pull(&acc1, T_INT64);
	pull(&acc2, T_VOID);
	if(C[core].rst_index >= RST_DEPTH) { exception(INT_RETSTOFL); return; }
	switch(acc2.params.type) {
		case T_INT1:
		case T_INT8:
		case T_INT16:
		case T_INT32:
		case T_INT64:
			if(acc2.val.i != 0) {
				C[core].rst[C[core].rst_index++] = C[core].instr_pointer;
				C[core].instr_pointer = (uint32_t) acc1.val.i;
			}
			break;
		case T_FLOAT:
			if(acc2.val.f != 0.0) {
				C[core].rst[C[core].rst_index++] = C[core].instr_pointer;
				C[core].instr_pointer = (uint32_t) acc1.val.i;
			}
			break;
		default: break;
	}
}


// return from a subroutine
void i_RETURN(void) {
	if(C[core].rst_index == 0) { exception(INT_RETSTUFL); return; }
	C[core].instr_pointer = C[core].rst[--C[core].rst_index];
}


// stop the current core
void i_STOP(void) {
    if(C[core].string_pointer) { C[core].instr_pointer--; return; } // stay on this instruction until all string transfers completes
    if(number_cores < 2) {  // a single core is just halted
        i_RESET();
        while(1);   // halt execution indefinitely
    }
    if(core != (number_cores - 1)) memcpy(&C[core], &C[number_cores - 1], sizeof(vm_core_t));
    number_cores--;
}


// jump to a relative address on a new core
void i_RUN(void) {
    pull(&acc1, T_INT32);
    if(number_cores >= CORES) { exception(INT_NO_CORES); return; }
    memcpy(&C[number_cores], &C[core], sizeof(vm_core_t));  // clone the current core
    C[number_cores].instr_pointer += acc2.val.i;
    number_cores++;
}


// jump to a absolute address on a new core
void i_ARUN(void) {
    pull(&acc1, T_INT64);
    if(number_cores >= CORES) { exception(INT_NO_CORES); return; }
    memcpy(&C[number_cores], &C[core], sizeof(vm_core_t));  // clone the current core
	C[number_cores].instr_pointer = (uint32_t) acc2.val.i;
    number_cores++;
}


// equalise v1 and v2 types by promoting the lower to the higher type
// void < int1 < int8 < int16 < int32 < int64 < float < extfp
static void promote(data_t *v1, data_t *v2) {
	if(v2->params.type < v1->params.type) { // aiming to have (v2) type greater or equal than (v1)
		data_t *vt = v1;
		v1 = v2; v2 = vt;
	}
	if(v2->params.type > v1->params.type) { // promote (v1) to (v2)
		switch(v2->params.type) {
			case T_INT1: v1->val.i = !!v1->val.i; break;			// (v1) is void
			case T_INT8: v1->val.i = (int8_t) v1->val.i; break;		// (v1) is void or int1
			case T_INT16: v1->val.i = (int16_t) v1->val.i; break;	// (v1) is void, int1 or int8
			case T_INT32: v1->val.i = (int32_t) v1->val.i; break;	// (v1) is void, int1, int8 or int16
			case T_INT64: v1->val.i = (Int64) v1->val.i; break;     // (v1) is void, int1, int8, int16, or int32
			case T_FLOAT: v1->val.f = (double) v1->val.i; break;	// (v1) is void, int1, int8, int16, int32, or int64
			default: break;
		}
		v1->params.type = v2->params.type;
	}
}


// ==
void i_EQ(void) {
	pull(&acc2, T_VOID);
	pull(&acc1, T_VOID);
	promote(&acc1, &acc2);
	switch(acc1.params.type) {
		case T_INT1:
		case T_INT8:
		case T_INT16:
		case T_INT32:
		case T_INT64: acc1.val.i = ((acc1.val.i == acc2.val.i) ? 1 : 0); break;
		case T_FLOAT: acc1.val.i = ((acc1.val.f == acc2.val.f) ? 1 : 0); break;
		default: acc1.val.i = 0; break;
	}
	acc1.params._all = T_INT1;
	push(&acc1);
}


// <>
void i_NEQ(void) {
	pull(&acc2, T_VOID);
	pull(&acc1, T_VOID);
	promote(&acc1, &acc2);
	switch(acc1.params.type) {
		case T_INT1:
		case T_INT8:
		case T_INT16:
		case T_INT32:
		case T_INT64: acc1.val.i = ((acc1.val.i != acc2.val.i) ? 1 : 0); break;
		case T_FLOAT: acc1.val.i = ((acc1.val.f != acc2.val.f) ? 1 : 0); break;
		default: acc1.val.i = 0; break;
	}
	acc1.params._all = T_INT1;
	push(&acc1);
}


// <
void i_SM(void) {
	pull(&acc2, T_VOID);
	pull(&acc1, T_VOID);
	promote(&acc1, &acc2);
	switch(acc1.params.type) {
		case T_INT1:
		case T_INT8:
		case T_INT16:
		case T_INT32:
		case T_INT64: acc1.val.i = ((acc1.val.i < acc2.val.i) ? 1 : 0); break;
		case T_FLOAT: acc1.val.i = ((acc1.val.f < acc2.val.f) ? 1 : 0); break;
		default: acc1.val.i = 0; break;
	}
	acc1.params._all = T_INT1;
	push(&acc1);
}


// <=
void i_SMEQ(void) {
	pull(&acc2, T_VOID);
	pull(&acc1, T_VOID);
	promote(&acc1, &acc2);
	switch(acc1.params.type) {
		case T_INT1:
		case T_INT8:
		case T_INT16:
		case T_INT32:
		case T_INT64: acc1.val.i = ((acc1.val.i <= acc2.val.i) ? 1 : 0); break;
		case T_FLOAT: acc1.val.i = ((acc1.val.f <= acc2.val.f) ? 1 : 0); break;
		default: acc1.val.i = 0; break;
	}
	acc1.params._all = T_INT1;
	push(&acc1);
}


// >
void i_GR(void) {
	pull(&acc2, T_VOID);
	pull(&acc1, T_VOID);
	promote(&acc1, &acc2);
	switch(acc1.params.type) {
		case T_INT1:
		case T_INT8:
		case T_INT16:
		case T_INT32:
		case T_INT64: acc1.val.i = ((acc1.val.i > acc2.val.i) ? 1 : 0); break;
		case T_FLOAT: acc1.val.i = ((acc1.val.f > acc2.val.f) ? 1 : 0); break;
		default: acc1.val.i = 0; break;
	}
	acc1.params._all = T_INT1;
	push(&acc1);
}


// >=
void i_GREQ(void) {
	pull(&acc2, T_VOID);
	pull(&acc1, T_VOID);
	promote(&acc1, &acc2);
	switch(acc1.params.type) {
		case T_INT1:
		case T_INT8:
		case T_INT16:
		case T_INT32:
		case T_INT64: acc1.val.i = ((acc1.val.i >= acc2.val.i) ? 1 : 0); break;
		case T_FLOAT: acc1.val.i = ((acc1.val.f >= acc2.val.f) ? 1 : 0); break;
		default: acc1.val.i = 0; break;
	}
	acc1.params._all = T_INT1;
	push(&acc1);
}


// bitwise or
void i_OR(void) {
	pull(&acc2, T_INT64);
	pull(&acc1, T_INT64);
	acc1.val.i |= acc2.val.i;
	acc1.params._all = T_INT64;
	push(&acc1);
}


// bitwise exclusive or
void i_EXOR(void) {
	pull(&acc2, T_INT64);
	pull(&acc1, T_INT64);
	acc1.val.i ^= acc2.val.i;
	acc1.params._all = T_INT64;
	push(&acc1);
}


// bitwise and
void i_AND(void) {
	pull(&acc2, T_INT64);
	pull(&acc1, T_INT64);
	acc1.val.i &= acc2.val.i;
	acc1.params._all = T_INT64;
	push(&acc1);
}


// bitwise inversion
void i_INVERT(void) {
	pull(&acc1, T_INT64);
	acc1.val.i ^= (UInt64) -1;
	acc1.params._all = T_INT64;
	push(&acc1);
}


// logical not
void i_NOT(void) {
	pull(&acc1, T_INT1);
	acc1.val.i = !acc1.val.i;
	acc1.params._all = T_INT1;
	push(&acc1);
}


// bitwise shift left
void i_SHIFTL(void) {
	pull(&acc2, T_INT64);
	pull(&acc1, T_INT64);
	if(acc2.val.i > 0) {
		if(acc2.val.i < 64) {
			while(acc2.val.i--) acc1.val.i <<= 1;
		}
		else acc1.val.i = 0;
	}
	acc1.params._all = T_INT64;
	push(&acc1);
}


// bitwise shift right
void i_SHIFTR(void) {
	pull(&acc2, T_INT64);
	pull(&acc1, T_INT64);
	if(acc2.val.i > 0) {
		if(acc2.val.i < 64) {
			while(acc2.val.i--) acc1.val.i >>= 1;
		}
		else acc1.val.i = 0;
	}
	acc1.params._all = T_INT64;
	push(&acc1);
}


// arithmetic addition
void i_ADD(void) {
	pull(&acc2, T_VOID);
	pull(&acc1, T_VOID);
	promote(&acc1, &acc2);
	switch(acc1.params.type) {
		case T_INT1:
		case T_INT8:
		case T_INT16:
		case T_INT32:
		case T_INT64: acc1.val.i = acc1.val.i + acc2.val.i; break;
		case T_FLOAT: acc1.val.f = acc1.val.f + acc2.val.f; break;
		default: break;
	}
	push(&acc1);
}


// arithmetic increment of the value in the data stack
void i_INC(void) {
	pull(&acc1, T_VOID);
	switch(acc1.params.type) {
		case T_INT1:
		case T_INT8:
		case T_INT16:
		case T_INT32:
		case T_INT64: acc1.val.i++; break;
		case T_FLOAT: acc1.val.f += 1.0; break;
		default: break;
	}
	push(&acc1);
}


// arithmetic subtraction
void i_SUB(void) {
	pull(&acc2, T_VOID);
	pull(&acc1, T_VOID);
	promote(&acc1, &acc2);
	switch(acc1.params.type) {
		case T_INT1:
		case T_INT8:
		case T_INT16:
		case T_INT32:
		case T_INT64: acc1.val.i = acc1.val.i - acc2.val.i; break;
		case T_FLOAT: acc1.val.f = acc1.val.f - acc2.val.f; break;
		default: break;
	}
	push(&acc1);
}


// arithmetic decrement of the value in the data stack
void i_DEC(void) {
	pull(&acc1, T_VOID);
	switch(acc1.params.type) {
		case T_INT1:
		case T_INT8:
		case T_INT16:
		case T_INT32:
		case T_INT64: acc1.val.i--; break;
		case T_FLOAT: acc1.val.f -= 1.0; break;
		default: break;
	}
	push(&acc1);
}


// arithmetic multiplication
void i_MUL(void) {
	pull(&acc2, T_VOID);
	pull(&acc1, T_VOID);
	promote(&acc1, &acc2);
	switch(acc1.params.type) {
		case T_INT1:
		case T_INT8:
		case T_INT16:
		case T_INT32:
		case T_INT64: acc1.val.i = acc1.val.i * acc2.val.i; break;
		case T_FLOAT: acc1.val.f = acc1.val.f * acc2.val.f; break;
		default: break;
	}
	push(&acc1);
}


// arithmetic division
void i_DIV(void) {
	pull(&acc2, T_VOID);
	pull(&acc1, T_VOID);
	promote(&acc1, &acc2);
	switch(acc1.params.type) {
		case T_INT1:
		case T_INT8:
		case T_INT16:
		case T_INT32:
		case T_INT64: {
			if(acc2.val.i == 0) {
				#if DIV0_BEHAVIOUR == 1
					exception(INT_DIV0);
					return;
				#elif DIV0_BEHAVIOUR == 2
					exception(INT_DIV0);
					return;
				#else
					acc1.val.f = NaN;
					acc1.td = T_FLOAT;
				#endif
			}
			else acc1.val.i = acc1.val.i / acc2.val.i;
			break;
		}
		case T_FLOAT: {
			if(acc2.val.f == 0.0) {
				#if DIV0_BEHAVIOUR == 1
					acc1.val.f = NaN;
				#elif DIV0_BEHAVIOUR == 2
					exception(INT_DIV0);
					return;
				#else
					acc1.val.f = NaN;
				#endif
			}
			else acc1.val.f = acc1.val.f / acc2.val.f;
			break;
		}
		default: break;
	}
	push(&acc1);
}


// integer division
void i_IDIV(void) {
	pull(&acc2, T_INT64);
	pull(&acc1, T_INT64);
	if(acc2.val.i == 0) {
		#if DIV0_BEHAVIOUR == 1
			exception(INT_DIV0);
			return;
		#elif DIV0_BEHAVIOUR == 2
			exception(INT_DIV0);
			return;
		#else
			acc1.val.f = NaN;
			acc1.td = T_FLOAT;
		#endif
	}
	else acc1.val.i = acc1.val.i / acc2.val.i;
	push(&acc1);
}


// modulo
void i_MOD(void) {
	pull(&acc2, T_INT64);
	pull(&acc1, T_INT64);
	if(acc2.val.i == 0) {
		#if DIV0_BEHAVIOUR == 1
			exception(INT_DIV0);
			return;
		#elif DIV0_BEHAVIOUR == 2
			exception(INT_DIV0);
			return;
		#else
			acc1.val.f = NaN;
			acc1.td = T_FLOAT;
		#endif
	}
	else acc1.val.i = acc1.val.i % acc2.val.i;
	push(&acc1);
}


// NaN constant
void i_NAN(void) {
    acc1.val.f = NaN;
    acc1.params._all = T_FLOAT;
    push(&acc1);
}


// +Infinity constant
void i_PINF(void) {
    acc1.val.f = (1.0 / 0.0);
    acc1.params._all = T_FLOAT;
    push(&acc1);
}


// -Infinity constant
void i_NINF(void) {
    acc1.val.f = (-1.0 / 0.0);
    acc1.params._all = T_FLOAT;
    push(&acc1);
}


// pi constant
void i_PI(void) {
    acc1.val.f = 3.14159265358979323846264338327950288L;
    acc1.params._all = T_FLOAT;
    push(&acc1);
}


// e constant
void i_E(void) {
    acc1.val.f = 2.71828182845904523536028747135266249L;
    acc1.params._all = T_FLOAT;
    push(&acc1);
}


// random number 0.0 <= number < 1
void i_RAND(void) {
    acc1.val.f = (double) rand() / ((double) RAND_MAX + 1.0);
    acc1.params._all = T_FLOAT;
    push(&acc1);
}


// absolute value
void i_ABS(void) {
    pull(&acc1, T_VOID);
	switch(acc1.params.type) {
		case T_INT1:
		case T_INT8:
		case T_INT16:
		case T_INT32:
		case T_INT64: acc1.val.i = ((acc1.val.i < 0) ? -acc1.val.i : acc1.val.i); break;
		case T_FLOAT: acc1.val.f = ((acc1.val.f < 0.0) ? -acc1.val.f : acc1.val.f); break;
		default: break;
	}
	push(&acc1);
}


// rounding of a floating-point number
void i_ROUND(void) {
    pull(&acc1, T_FLOAT);
    if(acc1.val.f >= 0.0) acc1.val.f = (double) ((Int64) acc1.val.f + ((acc1.val.f - (double) ((Int64) acc1.val.f)) >= 0.5) ? 1 : 0);
    else acc1.val.f = (double) ((Int64) acc1.val.f - ((acc1.val.f + (double) ((Int64) acc1.val.f)) <= -0.5) ? 1 : 0);
	push(&acc1);
}


// floating-point truncation to integer part only
void i_TRUNC(void) {
    pull(&acc1, T_FLOAT);
    acc1.val.f = (double) ((Int64) acc1.val.f);
	push(&acc1);
}


// floating-point fraction only
void i_FRACT(void) {
    pull(&acc1, T_FLOAT);
    acc1.val.f = acc1.val.f - (double) ((Int64) acc1.val.f);
	push(&acc1);
}


// sine
void i_SIN(void) {
    pull(&acc1, T_FLOAT);
    acc1.val.f = sin(acc1.val.f);
	push(&acc1);
}


// arc sine
void i_ASIN(void) {
    pull(&acc1, T_FLOAT);
    acc1.val.f = asin(acc1.val.f);
	push(&acc1);
}


// hyperbolic sine
void i_HSIN(void) {
    pull(&acc1, T_FLOAT);
    acc1.val.f = sinh(acc1.val.f);
	push(&acc1);
}


// cosine
void i_COS(void) {
    pull(&acc1, T_FLOAT);
    acc1.val.f = cos(acc1.val.f);
	push(&acc1);
}


// arc cosine
void i_ACOS(void) {
    pull(&acc1, T_FLOAT);
    acc1.val.f = acos(acc1.val.f);
	push(&acc1);
}


// hyperbolic cosine
void i_HCOS(void) {
    pull(&acc1, T_FLOAT);
    acc1.val.f = cosh(acc1.val.f);
	push(&acc1);
}


// tangent
void i_TAN(void) {
    pull(&acc1, T_FLOAT);
    acc1.val.f = tan(acc1.val.f);
	push(&acc1);
}


// arc tangent
void i_ATAN(void) {
    pull(&acc1, T_FLOAT);
    acc1.val.f = atan(acc1.val.f);
	push(&acc1);
}


// hyperbolic tangent
void i_HTAN(void) {
    pull(&acc1, T_FLOAT);
    acc1.val.f = tanh(acc1.val.f);
	push(&acc1);
}


// cotangent
void i_COTAN(void) {
    pull(&acc1, T_FLOAT);
    acc1.val.f = 1.0 / tan(acc1.val.f);
	push(&acc1);
}


// decimal logarithm with base 10
void i_LOGD(void) {
    pull(&acc1, T_FLOAT);
    acc1.val.f = log10(acc1.val.f);
	push(&acc1);
}


// natural logarithm with base 'e'
void i_LOGN(void) {
    pull(&acc1, T_FLOAT);
    acc1.val.f = log(acc1.val.f);
	push(&acc1);
}


// exponent e^x
void i_EXP(void) {
    pull(&acc1, T_FLOAT);
    acc1.val.f = exp(acc1.val.f);
	push(&acc1);
}


// power function (value1 ^ value2)
void i_POWER(void) {
    pull(&acc2, T_FLOAT);
    pull(&acc1, T_FLOAT);
    acc1.val.f = pow(acc1.val.f, acc2.val.f);
	push(&acc1);
}


// square root
void i_ROOT2(void) {
    pull(&acc1, T_FLOAT);
    acc1.val.f = sqrt(acc1.val.f);
	push(&acc1);
}


// cube root
void i_ROOT3(void) {
    pull(&acc1, T_FLOAT);
    acc1.val.f = cbrt(acc1.val.f);
	push(&acc1);
}


// put character to the console
void i_PUTCHR(void) {
    if(((con_tx_in + 1) % CONSOLE_TX_BUF_S) != con_tx_out) {    // act only if there is room in the buffer
        pull(&acc1, T_INT8);
        con_tx_buf[con_tx_in] = (uint8_t) acc1.val.i;
        if(++con_tx_in >= CONSOLE_TX_BUF_S) con_tx_in = 0;
    }
    else C[core].instr_pointer--;   // stay on this instruction until there is room in the console output buffer
}


#if CONSOLE_BLOCKING != 0
// blocking output of a string to the console
static void internal_outstr(void) {
    while(C[core].string_pointer) {    // strings are output without holding up the execution of other threads
        if(*((uint8_t *) (uintptr_t) &code[C[core].string_pointer]))
            putchar(*((uint8_t *) (uintptr_t) &code[C[core].string_pointer++]));
        else C[core].string_pointer = 0;
    }
}
#endif


// put zero-terminated string to the console
void i_PUTSTR(void) {
    if(C[core].string_pointer) { C[core].instr_pointer--; return; } // stay on this instruction until a previous transfer completes
    pull(&acc1, T_INT64);
    C[core].string_pointer = (uint32_t) acc1.val.i;
    #if CONSOLE_BLOCKING != 0
        internal_outstr();
    #endif
}


// put decimal value to the console
void i_PUTVAL(void) {
    if(C[core].string_pointer) { C[core].instr_pointer--; return; } // stay on this instruction until a previous transfer completes
    pull(&acc1, T_VOID);
	switch(acc1.params.type) {
		case T_INT1: sprintf((char *) C[core].str_buffer, "%i", (int) !!acc1.val.i); break;
		case T_INT8: sprintf((char *) C[core].str_buffer, "%i", (int) acc1.val.i); break;
		case T_INT16: sprintf((char *) C[core].str_buffer, "%i", (int) acc1.val.i); break;
		case T_INT32: sprintf((char *) C[core].str_buffer, "%li", (long) acc1.val.i); break;
        case T_INT64:
            #if INT64_SUPPORTED == 1
                sprintf((char *) C[core].str_buffer, "%" PRId64, (int64_t) acc1.val.i);
            #else
                sprintf((char *) C[core].str_buffer, "%li", (long) acc1.val.i);
            #endif
            break;
		case T_FLOAT: sprintf((char *) C[core].str_buffer, "%G", (double) acc1.val.f); break;
		default: break;
	}
    C[core].string_pointer = (uint32_t) (uintptr_t) C[core].str_buffer - (uint32_t) (uintptr_t) code;
    #if CONSOLE_BLOCKING != 0
        internal_outstr();
    #endif
}


// get character from the console
void i_GETCHR(void) {
    if(con_rx_out == con_rx_in) { C[core].instr_pointer--; return; } // stay on this instruction until there is something in the buffer
    acc1.params._all = T_INT16;
    acc1.val.i = con_rx_buf[con_rx_out];
    if(++con_rx_out >= CONSOLE_RX_BUF_S) con_rx_out = 0;
    push(&acc1);
}


// peek into the console input buffer
void i_PEEKCHR(void) {
    acc1.params._all = T_INT16;
    acc1.val.i = ((con_rx_out != con_rx_in) ? con_rx_buf[con_rx_out] : -1);
    push(&acc1);
}


// internal
// put an incoming from the console character into the console input buffer
// this function needs to be called from an outside interrupt when a character is received on the console post
void console_chr_in(uint8_t incoming_character) {
    if(((con_rx_in + 1) % CONSOLE_RX_BUF_S) != con_rx_out) {    // act only if there is room in the buffer
        con_rx_buf[con_rx_in] = incoming_character;
        if(++con_rx_in >= CONSOLE_RX_BUF_S) con_rx_in = 0;
    }
}


// initialise memory area with a byte value
void i_MEMINIT(void) {
    pull(&acc3, T_INT8);    // value
    pull(&acc2, T_INT64);   // size
    pull(&acc1, T_INT64);   // start address
    while(acc2.val.i-- > 0) *((uint8_t *) (uintptr_t) acc1.val.i++) = acc3.val.i;
}


// return the length of a zero-terminated string
void i_STRLEN(void) {
    pull(&acc1, T_INT64);   // start address
    acc2.val.i = 0;
    while(*((uint8_t *) (uintptr_t) acc1.val.i++) != 0) acc2.val.i++;
    push(&acc2);
}


// copy memory area from one address to another
void i_MEMCOPY(void) {
    pull(&acc3, T_INT64);   // size
    pull(&acc2, T_INT64);   // source
    pull(&acc1, T_INT64);   // destination
    if(acc1.val.i < acc2.val.i) {
        while(acc3.val.i-- > 0) *((uint8_t *) (uintptr_t) acc1.val.i++) = *((uint8_t *) (uintptr_t) acc2.val.i++);
    }
    else if(acc1.val.i > acc2.val.i) {
        acc1.val.i += acc3.val.i;
        acc2.val.i += acc3.val.i;
        while(acc3.val.i-- > 0) *((uint8_t *) (uintptr_t) --acc1.val.i) = *((uint8_t *) (uintptr_t) --acc2.val.i);
    }
}


// copy zero-terminated string from one address to another
void i_STRCOPY(void) {
    pull(&acc2, T_INT64);   // source
    pull(&acc1, T_INT64);   // destination
    if(acc1.val.i < acc2.val.i) {
        while(*((uint8_t *) (uintptr_t) acc2.val.i) != 0) {
            *((uint8_t *) (uintptr_t) acc1.val.i++) = *((uint8_t *) (uintptr_t) acc2.val.i++);
        }
        *((uint8_t *) (uintptr_t) acc1.val.i) = 0;  // terminating 0 byte
    }
    else if(acc1.val.i > acc2.val.i) {
        acc3.val.i = 1;
        acc4.val.i = acc2.val.i;
        while(*((uint8_t *) (uintptr_t) acc4.val.i++) != 0) acc3.val.i++;
        acc1.val.i += acc3.val.i;
        acc2.val.i += acc3.val.i;
        while(acc3.val.i-- > 0) *((uint8_t *) (uintptr_t) --acc1.val.i) = *((uint8_t *) (uintptr_t) --acc2.val.i);
    }
}


// compare two areas and return 0 if they are equal, or the result of the comparison from the first difference
void i_MEMCOMP(void) {
    pull(&acc3, T_INT64);   // size
    pull(&acc2, T_INT64);   // area 2
    pull(&acc1, T_INT64);   // area 1
    acc4.val.i = 0;
    while(acc3.val.i-- > 0 && acc4.val.i == 0) {
        acc4.val.i = *((uint8_t *) (uintptr_t) acc1.val.i++) - *((uint8_t *) (uintptr_t) acc2.val.i++);
    }
    push(&acc4);
}


// compare two strings and return 0 if they are equal, or the result of the comparison from the first difference
void i_STRCOMP(void) {
    pull(&acc2, T_INT64);   // area 2
    pull(&acc1, T_INT64);   // area 1
    acc4.val.i = 0;
    while(acc4.val.i == 0 &&
            (*((uint8_t *) (uintptr_t) acc1.val.i) > 0 || *((uint8_t *) (uintptr_t) acc2.val.i) > 0)) {
        acc4.val.i = *((uint8_t *) (uintptr_t) acc1.val.i++) - *((uint8_t *) (uintptr_t) acc2.val.i++);
    }
    push(&acc4);
}
