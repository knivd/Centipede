/*
 * --------------------------------------------------------------------------------------
 *
 * Centipede Monitor
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

#ifndef MONITOR_H
#define	MONITOR_H

#ifdef	__cplusplus
extern "C" {
#endif

#include <stdint.h>

#define CTRL_C  0x03

// built-in Monitor
// the (clrscr) parameter is flag to clear the screen (non-zero), or not (zero)
void monitor(uint8_t clrscr);

#ifdef	__cplusplus
}
#endif

#endif
