/*
 * shell-core.c
 *
 *  Created on: 1 ??? 2018 ?.
 *      Author: ichiro
 *      modified by: nikolai (2021)
 */

#include "shell.h"

#include <stdlib.h>
#include <string.h>

#include "FreeRTOS.h"
#include "FreeRTOS_CLI.h"
#include "queue.h"
#include "serial.h"
#include "../Middlewares/Third_Party/microrl/src/microrl.h"
#include "tools.h"

#ifndef  configINCLUDE_TRACE_RELATED_CLI_COMMANDS
#define configINCLUDE_TRACE_RELATED_CLI_COMMANDS 0
#endif

#ifndef configINCLUDE_QUERY_HEAP_COMMAND
#define configINCLUDE_QUERY_HEAP_COMMAND 0
#endif

/*----------------------------------------------------------------------------*/
/* Private functions definitions. */
void prvOutputString(const char *pcMessage);

char prvGetChar(void);

int prvExecute(const char *cInputString);
// const char* prvComplete(const char*, const struct dfa*);

/*----------------------------------------------------------------------------*/

#ifdef _USE_COMPLETE
static const char *const kCompletableCommands[] = { "led red on",
																										"led red off",
																										"led red toggle",
																										"led blue on",
																										"led blue off",
																										"led blue toggle",
																										"led green on",
																										"led green off",
																										"led green toggle",
																										"man",
																										"start",
																										"task-stats",
																										"run-time-stats",
																										"echo-three-parameters",
																										"echo-parameters",

#if (configINCLUDE_QUERY_HEAP_COMMAND == 1)
		"query heap",
#endif

#if (configINCLUDE_TRACE_RELATED_CLI_COMMANDS == 1)
		"trace start",
		"trace stop"
#endif
};

const size_t kCompletableCommandsCount = sizeof(kCompletableCommands) / sizeof(char *);
#endif

/* Create microrl object and pointer on it. */
microrl_t rl;
microrl_t *prl = &rl;

/* Const messages output by the command console. */
static const char *const pcWelcomeMessage =
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
		"|    ver. 0.2  |__/        Trajectory CNC         |\r\n"
		"`-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-'\r\n"
		"\r\n"
		" Type \"help\" to view a list of available commands.\r\n"
		"\r\n";

void shell_start(void) {
#ifdef _USE_COMPLETE
	const size_t kCompletableCommandsIncludingManCount = kCompletableCommandsCount * 2;  // { command } U { man command}
	char **commands = calloc(kCompletableCommandsIncludingManCount, sizeof(char *));
	if (commands == NULL) {
		abort();
	}

	size_t max_command_len = 0;
	for (size_t i = 0; i < kCompletableCommandsCount; ++i) {
		const size_t kCurrCommandLen = strlen(kCompletableCommands[i]);
		if (kCurrCommandLen > max_command_len) {
			max_command_len = kCurrCommandLen;
		}

		commands[i] = calloc(kCurrCommandLen + 1, sizeof(char));
		if (commands[i] == NULL) {
			abort();
		}
		strcpy(commands[i], kCompletableCommands[i]);
	}

	const size_t kManCommandLen = max_command_len + 5;
	char *const man_command = calloc(kManCommandLen, sizeof(char));  // "man " + command + '\0'
	if (man_command == NULL) {
		abort();
	}

	man_command[0] = 'm';
	man_command[1] = 'a';
	man_command[2] = 'n';
	man_command[3] = ' ';
	for (size_t i = 0; i < kCompletableCommandsCount; ++i) {
		man_command[4] = '\0';
		char *const curr_command = calloc(strlen(kCompletableCommands[i]) + 1, sizeof(char));
		if (curr_command == NULL) {
			abort();
		}

		strcpy(curr_command, kCompletableCommands[i]);
		char *const kCommandName = strtok(curr_command, " ");
		strcat(man_command, kCommandName);

		commands[kCompletableCommandsCount + i] = calloc(kManCommandLen + 1, sizeof(char));
		if (commands[i] == NULL) {
			abort();
		}
		strcpy(commands[kCompletableCommandsCount + i], man_command);
		free(curr_command);
	}

	struct dfa autocompletion_dfa = autocompletion_dfa_create(commands, kCompletableCommandsIncludingManCount);

	for (size_t i = 0; i < kCompletableCommandsIncludingManCount; ++i) {
		free(commands[i]);
	}
	free(commands);
#endif
	serial_init();
	prvOutputString(pcWelcomeMessage);

	/* Call init with ptr to microrl instance and print callback. */
	microrl_init(prl, prvOutputString);
	/* Set callback for execute. */
	microrl_set_execute_callback(prl, prvExecute);
#ifdef _USE_COMPLETE
	prl->autocompleter = &autocompletion_dfa;
	/* Set callback for completion. */
	//	microrl_set_complete_callback(prl, prvComplete);
#endif
	/* Set callback for Ctrl+C. */
	microrl_set_sigint_callback(prl, NULL);

	for (;;) {
		/* Put received char from stdin to microrl lib. */
		microrl_insert_char(prl, prvGetChar());
	}
}

/*----------------------------------------------------------------------------*/

void prvOutputString(const char *const pcMessage) {
	vSerialPutString((signed char *)pcMessage,
									 (unsigned short)strlen(pcMessage));
}

/*----------------------------------------------------------------------------*/

char prvGetChar(void) {
	signed char cRxedChar;
	while (xSerialGetChar(&cRxedChar, portMAX_DELAY) != pdPASS) {
	}
	return (char)cRxedChar;
}

/*----------------------------------------------------------------------------*/

// const char* prvComplete(const char *const cmdline,
// 		const struct dfa *const autocompletion_dfa) {
// 	return dfa_get_next_autocompletion_variant(autocompletion_dfa, cmdline);
// }
/*----------------------------------------------------------------------------*/

int prvExecute(const char *const cInputString) {
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
		xReturned = FreeRTOS_CLIProcessCommand(cInputString, pcOutputString,
																					 configCOMMAND_INT_MAX_OUTPUT_SIZE);

		/* Write the generated string to the UART. */
		prvOutputString(pcOutputString);

	} while (xReturned != pdFALSE);

	return 0;
}
/*----------------------------------------------------------------------------*/