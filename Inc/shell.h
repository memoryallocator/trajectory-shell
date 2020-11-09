/*
 * shell.h
 *
 *  Created on: 1 мая 2018 г.
 *      Author: ichiro
 */

#ifndef SHELL_H_
#define SHELL_H_

/* Dimensions the buffer into which input characters are placed. */
#define cmdMAX_INPUT_SIZE		1024

/* Dimensions a buffer to be used by the UART driver, if the UART driver uses a
 * buffer at all. */
#define cmdQUEUE_LENGTH			2048

/* DEL acts as a backspace. */
#define cmdASCII_DEL			(0x7F)
#define cmdASCII_ESC			(0x1B)
#define cmdASCII_BELL			(0x07)

/* VT100 terminal commands */
#define cmdVT100_CURSOR_BACKWARD		"\033[D"
#define cmdVT100_CURSOR_FORWARD			"\033[C"
#define cmdVT100_CLEAR_SCREEN			"\033[2J"
#define cmdVT100_CURSOR_HOME			"\033[0;0H"
#define cmdVT100_CURSOR_BOTTOM_2_LINES	"\033[2B"
#define cmdVT100_CURSOR_BOTTOM			"\033[B"
#define cmdVT100_CURSOR_UP				"\033[A"



void shell_init(void);
void vRegisterSampleCLICommands(void);


#endif /* SHELL_H_ */
