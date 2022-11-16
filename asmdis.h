/*
 * --------------------------------------------------------------------------------------
 *
 * Assembler and Disassembler
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

#ifndef ASMDIS_H
#define	ASMDIS_H

#ifdef	__cplusplus
extern "C" {
#endif

#include <stdint.h>

// error codes returned by the assembler
#define ERR_SYNTAX          -1
#define ERR_INV_NUMBER      -2
#define ERR_INV_RANGE       -3
#define ERR_INV_CHAR        -4
#define ERR_INV_VARN        -5
#define ERR_UNKNOWN_ID      -6
#define ERR_NESTING_DEPTH   -7

// assembler
// the assembly process produces relocatable code
// the parameter (brk_code) must be set 0 to compile a full source text, or set 10 (\n) to compile a single line
// the input and output pointers are modified accordingly
// on a successful compilation, the compile code length in bytes is returned
// a negative result refers to an exit code as per the ERR_xxx constants
int32_t vm_assembler(char **source, uint8_t **destination, char brk_code);

// disassembler
// if (*output) is NULL the output is on the standard system console
// the parameter (num_instr) specifies the number of source lines to list
// (*pcode) gets modified after execution
// the function will return the difference between the listed and the specified number of lines for listing
uint32_t vm_disassembler(uint8_t **pcode, char *output, uint32_t num_ln);

#ifdef	__cplusplus
}
#endif

#endif
