/*
 * shell-core.c
 *
 *  Created on: 1 мая 2018 г.
 *      Author: ichiro
 */

#include "FreeRTOS.h"
#include "FreeRTOS_CLI.h"
#include "queue.h"
#include "shell.h"
#include "serial.h"
#include "microrl.h"
#include "string.h"

/*----------------------------------------------------------------------------*/
/* Private functions definitions. */
void prvOutputString(const char* const pcMessage);
char prvGetChar(void);
int prvExecute(const char* const arg);
int prvComplete(const char* const cmdline, char const ** comp_arr);

/*----------------------------------------------------------------------------*/


/* Create microrl object and pointer on it. */
microrl_t rl;
microrl_t * prl = &rl;

/* Const messages output by the command console. */
static const char * const pcWelcomeMessage =
cmdVT100_CLEAR_SCREEN
cmdVT100_CURSOR_HOME
cmdVT100_CURSOR_BOTTOM
".-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-.\r\n"
"|  _______        _        _______      __     __ |\r\n"
"| |__   __|      (_)      |__   __|     \\ \\   / / |\r\n"
"!    | |_ __ __ _ _  ___  ___| | ___  _ _\\ \\_/ /  !\r\n"
":    | | '__/ _` | |/ _ \\/ __| |/ _ \\| '__\\   /   :\r\n"
".    | | | | (_| | |  __/ (__| | (_) | |   | |    .\r\n"
":    |_|_|  \\__,_| |\\___|\\___|_|\\___/|_|   |_|    :\r\n"
"!               _/ |                              !\r\n"
"|    ver. 0.1  |__/        Trajectory CNC         |\r\n"
"`-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-'\r\n"
"\r\n"
" Type \"help\" to view a list of available commands.\r\n"
"\r\n";


void shell_start(void)
{
	serial_init();
	prvOutputString(pcWelcomeMessage);

	/* Call init with ptr to microrl instance and print callback. */
	microrl_init(prl, prvOutputString);
	/* Set callback for execute. */
	microrl_set_execute_callback(prl, prvExecute);
#ifdef _USE_COMPLETE
	/* Set callback for completion. */
	microrl_set_complete_callback(prl, prvComplete);
#endif
	/* Set callback for Ctrl+C. */
	microrl_set_sigint_callback(prl, NULL);

	for (;;) {
		/* Put received char from stdin to microrl lib. */
		microrl_insert_char(prl, prvGetChar());
	}
}

/*----------------------------------------------------------------------------*/

void prvOutputString(const char * const pcMessage)
{
	vSerialPutString(
			(signed char *) pcMessage,
			(unsigned short) strlen(pcMessage));
}
/*----------------------------------------------------------------------------*/

char prvGetChar(void)
{
	signed char cRxedChar;
	while (xSerialGetChar(&cRxedChar, portMAX_DELAY) != pdPASS)
		;
	return (char) cRxedChar;
}

/*----------------------------------------------------------------------------*/

int prvComplete(const char* const cmdline, char const ** comp_arr)
{
	return 0;
}

/*----------------------------------------------------------------------------*/

int prvExecute(const char * const cInputString)
{
	char *pcOutputString;
	BaseType_t xReturned;

	pcOutputString = FreeRTOS_CLIGetOutputBuffer();

	/* Pass the received command to the command interpreter.  The
	 * command interpreter is called repeatedly until it returns
	 * pdFALSE	(indicating there is no more output) as it might
	 * generate more than one string. */
	do {
		/* Get the next output string from the command
		 * interpreter. */
		xReturned = FreeRTOS_CLIProcessCommand(
				cInputString,
				pcOutputString,
				configCOMMAND_INT_MAX_OUTPUT_SIZE);

		/* Write the generated string to the UART. */
		prvOutputString(pcOutputString);

	} while (xReturned != pdFALSE);

	return 0;
}
/*----------------------------------------------------------------------------*/
