/*
 * --------------------------------------------------------------------------------------
 *
 * Centipede
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

#ifndef CENTIPEDE_H
#define	CENTIPEDE_H

#ifdef	__cplusplus
extern "C" {
#endif

#include <stdint.h>

#ifdef PIC32MX1
	#include "platform/pic32mx1/platform.h"

#else
	#include "platform/generic/platform.h"

#endif

#include "vm.h"
#include "asmdis.h"
#include "monitor.h"

int _inbyte(unsigned short t);  // read byte from the console with timeout in ms

#ifdef	__cplusplus
}
#endif

#endif
