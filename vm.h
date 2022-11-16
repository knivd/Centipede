/*
 * --------------------------------------------------------------------------------------
 *
 * Virtual Machine
 * written by Konstantin Dimitrov, (C) 2021
 * NOTE: this module is abstract code
 *
 * --------------------------------------------------------------------------------------
 *  LICENSE INFORMATION
 *   1. This code is "linkware". It is free for use, modification, and distribution
 *      without any limitations. The only requirement is to link with the author on
 *      LinkedIn: https://www.linkedin.com/in/knivd/ or follow on Twitter: @knivd
 *   2. The author assumes no responsibility for any loss or damage caused by this code
 * --------------------------------------------------------------------------------------
*/

// ======================================================================================
// Written and debugged with Code::Blocks 20.03
// NOTE: for best source readability, please use a monospace font (eg. Consolas)

#ifndef VM_H
#define	VM_H

#ifdef	__cplusplus
extern "C" {
#endif

#include <stdint.h>

// reset the virtual machine and start executing instructions from the given memory block (assumed virtual address 0)
void vm_run(uint8_t *Code);

#define CORES           	4   // maximum number of simultaneously running cores
#define DST_DEPTH       	32  // depth of the internal data stack per individual core (maximum 256)
#define RST_DEPTH       	32  // depth of the call/return stack per individual core   (maximum 256)
#define VARIABLES       	160 // number of allocated variables per individual core    (maximum 256)
#define STR_BUF_S           64  // size in bytes of the internal string buffer per individual core
#define INDEX_BEHAVIOUR		1	// 0: ignore; 1: exception on invalid index; 2: wrap around within the range
#define INVDEPTH_BEHAVIOUR	1	// 0: ignore and take the edge element; 1: exception on invalid depth
#define DIV0_BEHAVIOUR		1	// 0: always return a NaN value; 1: exception on integer, Nan on float, 2: always exception
#define INT64_SUPPORTED     1   // set 0 for compilers that don't support 64-bit integers

#define SERIAL_BAUDRATE     115200  // serial console baudrate; protocol 8N1
#define CONSOLE_RX_BUF_S    144     // size in bytes of the console reception buffer
#define CONSOLE_TX_BUF_S    64      // size in bytes of the console transmission buffer
#define CONSOLE_BLOCKING    0       // if enabled, the console output is blocking execution
                                    // this increases the overall VM execution speed by about 10%
                                    // but everything is stopped while sending something to the console
#define CONSOLE_BREAK_CODE  3       // if not zero, defines an ASCII code for execution break key and
                                    // entry in the Monitor

#define ADDR_INTR           0ul     // reset/exception entry address in the virtul machine

#define ENABLE_STOFL_INT    // if defined, a stack overflow situation will trigger the STOFL exception
#define ENABLE_STUFL_INT    // if defined, a stack underflow situation will trigger the STUFL exception
#define ENABLE_REDEF_INT    // if defined, an attempt to redefine a variable will trigger the REDEF exception

#if INT64_SUPPORTED == 1
    typedef int64_t  Int64;
    typedef uint64_t UInt64;
#else
    typedef int32_t  Int64;
    typedef uint32_t UInt64;
#endif

// exception type loaded into the data stack on start of execution
#define INT_RESET       0x00	// reset event
#define INT_END			0x01	// end of program reached
#define INT_INVALID     0x03	// invalid instruction
#define INT_STOFL       0x04	// data stack overflow
#define INT_STUFL       0x05	// data stack underflow
#define INT_INVSTX      0x0C	// invalid stack index
#define INT_NO_CORES    0x10	// no more available cores
#define INT_INVVID      0x20	// invalid variable id
#define INT_INVDIM      0x23	// invalid variable dimension or index
#define INT_REDEF       0x24	// redefinition of a variable
#define INT_UNALLOC     0x25	// accessing an unallocated variable
#define INT_RETSTOFL    0x34	// return stack overflow ("call" with a full return stack)
#define INT_RETSTUFL    0x35	// return stack underflow ("return" with an empty return stack)
#define INT_DIV0		0x40	// division by zero

#define T_VOID          0       // not a real data type; used to mark free variables
#define T_INT1          1       // 1-bit data type
#define T_INT8          2       // 8-bit signed integer
#define T_INT16         3       // 16-bit signed integer
#define T_INT32         4       // 32-bit signed integer
#define T_INT64         5       // 64-bit signed integer
#define T_FLOAT         6       // 64-bit floating point number (double precision)
#define T_EXTFP         7       // reserved for extended precision floating point numbers

#ifndef BIT
#define BIT(b) (1ull << (b))
#endif

#define TYPE_BITS   3

typedef struct {
	union {
		Int64 i;        // integer value
		double f;       // floating point value
	} val;              // value register
    union {
        struct {
            uint32_t type : TYPE_BITS;          // bits [2:0] data type (see the T_xxx definitions)
            uint32_t dim  : (32 - TYPE_BITS);   // bits [31:3] dimension (0 for single variables)
        };
        struct {
            uint32_t _all;
        };
    } params;
} data_t;

extern void (*const instruction[256])(void);    // instruction codes
extern const char instr_text[256][8];           // instruction words as text

void i_(void);          // invalid instruction
void i_NOP(void);       // no operation
void i_PADNOP(void);    // no operation
void i_SETBIN(void);    // no operation
void i_SETOCT(void);    // no operation
void i_SETHEX(void);    // no operation
void i_SETCHR(void);    // no operation
void i_SETSTR(void);    // no operation
void i_DBASE(void);		// return the physical address of start of the data memory (RAM)
void i_PBASE(void);		// return the physical address of start of the program memory (ROM)
void i_DSIZE(void);		// return the total size the data memory (RAM) in bytes
void i_PSIZE(void);		// return the total size the program memory (ROM) in bytes
void i_END(void);       // mark end of program code
void i_RESET(void);     // full VM reset
void i_WHERE(void);     // return the current instruction address
void i_CORES(void);     // return the total number of execution cores
void i_ACTIVE(void);    // return the number of currently running cores
void i_CURRENT(void);   // return the number of the current core
void i_CLEAR(void);     // clear the data stack and reset the stack pointer
void i_DEPTH(void);     // return current stack depth
void i_DUP(void);       // duplicate the top element
void i_DROP(void);      // remove the top element
void i_OVER(void);      // copy on top an element from a greater depth
void i_SWAP(void);      // swap the top of the stack with an element from a greater depth
void i_CINT0(void);     // int1 data constant with value 0
void i_CINT1(void);     // int1 data constant with value 1
void i_CINT8(void);     // int8 constant
void i_CINT16(void);    // int16 constant
void i_CINT32(void);    // int32 constant
void i_CINT64(void);    // int64 constant
void i_CFLOAT(void);    // float constant
void i_DATAI8(void);    // int8 data constants
void i_DATAI16(void);   // int16 data constants
void i_DATAI32(void);   // int32 data constants
void i_DATAI64(void);   // int64 data constants
void i_DATAF(void);     // float data constants
void i_FREE(void);      // free a variable    (id free)
void i_VINT1(void);     // new int1 variable  (dimension id vint8)
void i_VINT8(void);     // new int8 variable  (dimension id vint8)
void i_VINT16(void);    // new int16 variable (dimension id vint16)
void i_VINT32(void);    // new int32 variable (dimension id vint32)
void i_VInt64(void);    // new int64 variable (dimension id vint64)
void i_VFLOAT(void);    // new float variable (dimension id vfloat)
void i_SET(void);       // set value into variable ([index] id =)
void i_GET(void);       // get value from variable ([index] id ?)
void i_VINC(void);      // increase the value in variable
void i_VDEC(void);      // decrease the value in variable
void i_GOTO(void);      // jump to a relative address
void i_CALL(void);      // call a function at a relative address
void i_AGOTO(void);     // jump to an absolute address
void i_ACALL(void);     // call a function at an absolute address
void i_GOTOIF(void);    // conditional jump to a relative address (value addr gotoif)
void i_CALLIF(void);    // conditional call a function at a relative address (value addr callif)
void i_AGOTOIF(void);   // conditional jump to an absolute address (value addr agotoif)
void i_ACALLIF(void);   // conditional call a function at an absolute address (value addr acallif)
void i_RETURN(void);    // return from a subroutine
void i_RUN(void);       // jump to a relative address on a new core
void i_ARUN(void);      // jump to a absolute address on a new core
void i_STOP(void);      // stop the current core
void i_EQ(void);        // ==
void i_NEQ(void);       // <>
void i_SM(void);        // <
void i_SMEQ(void);      // <=
void i_GR(void);        // >
void i_GREQ(void);      // >=
void i_SHIFTL(void);    // bitwise shift left
void i_SHIFTR(void);    // bitwise shift right
void i_OR(void);        // bitwise or
void i_EXOR(void);      // bitwise exclusive or
void i_AND(void);       // bitwise and
void i_INVERT(void);    // bitwise inversion
void i_NOT(void);       // logical not
void i_ADD(void);       // arithmetic addition
void i_INC(void);       // arithmetic increment of the value in the data stack
void i_SUB(void);       // arithmetic subtraction
void i_DEC(void);       // arithmetic decrement of the value in the data stack
void i_MUL(void);       // arithmetic multiplication
void i_DIV(void);       // arithmetic division
void i_IDIV(void);      // integer division
void i_MOD(void);       // modulo
void i_NAN(void);       // NaN constant
void i_PINF(void);      // +Infinity constant
void i_NINF(void);      // -Infinity constant
void i_PI(void);        // pi constant
void i_E(void);         // e constant
void i_RAND(void);      // random number 0.0 <= number < 1
void i_ABS(void);       // absolute value
void i_ROUND(void);     // rounding of a floating-point number
void i_TRUNC(void);     // floating-point truncation to integer part only
void i_FRACT(void);     // floating-point fraction only
void i_SIN(void);       // sine
void i_ASIN(void);      // arc sine
void i_HSIN(void);      // hyperbolic sine
void i_COS(void);       // cosine
void i_ACOS(void);      // arc cosine
void i_HCOS(void);      // hyperbolic cosine
void i_TAN(void);       // tangent
void i_ATAN(void);      // arc tangent
void i_HTAN(void);      // hyperbolic tangent
void i_COTAN(void);     // cotangent
void i_LOGD(void);      // decimal logarithm with base 10
void i_LOGN(void);      // natural logarithm with base 'e'
void i_EXP(void);       // exponent e^x
void i_POWER(void);     // power function (value1 ^ value2)
void i_ROOT2(void);     // square root
void i_ROOT3(void);     // cube root
void i_COMMENT(void);   // text commentaries in source
void i_LABEL(void);     // text labels in source
void i_NAME(void);      // named constant in source
void i_GETCHR(void);    // get character from the console
void i_PEEKCHR(void);   // peek into the console input buffer
void i_PUTCHR(void);    // put character to the console
void i_PUTSTR(void);    // put zero-terminated string to the console
void i_PUTVAL(void);    // put decimal value to the console
void i_MEMINIT(void);   // initialise memory area with a byte value
void i_STRLEN(void);    // return the length of a zero-terminated string
void i_MEMCOPY(void);   // copy memory area from one address to another
void i_STRCOPY(void);   // copy zero-terminated string from one address to another
void i_MEMCOMP(void);   // compare two areas and return 0 if they are equal, or the result of the comparison from the first difference
void i_STRCOMP(void);   // compare two strings and return 0 if they are equal, or the result of the comparison from the first difference

volatile uint8_t con_rx_in;
volatile uint8_t con_rx_out;
uint8_t con_rx_buf[CONSOLE_RX_BUF_S];
volatile uint8_t con_tx_in;
volatile uint8_t con_tx_out;
uint8_t con_tx_buf[CONSOLE_TX_BUF_S];

// create a system exception
void exception(uint32_t xcode);

// internal
// put an incoming from the console character into the console input buffer
// this function needs to be called from an outside interrupt when a character is received on the console post
void console_chr_in(uint8_t incoming_character);

#ifdef	__cplusplus
}
#endif

#endif
