/*
 FreeRTOS V9.0.0 - Copyright (C) 2016 Real Time Engineers Ltd.
 All rights reserved

 VISIT http://www.FreeRTOS.org TO ENSURE YOU ARE USING THE LATEST VERSION.

 This file is part of the FreeRTOS distribution.

 FreeRTOS is free software; you can redistribute it and/or modify it under
 the terms of the GNU General Public License (version 2) as published by the
 Free Software Foundation >>>> AND MODIFIED BY <<<< the FreeRTOS exception.

 ***************************************************************************
 >>!   NOTE: The modification to the GPL is included to allow you to     !<<
 >>!   distribute a combined work that includes FreeRTOS without being   !<<
 >>!   obliged to provide the source code for proprietary components     !<<
 >>!   outside of the FreeRTOS kernel.                                   !<<
 ***************************************************************************

 FreeRTOS is distributed in the hope that it will be useful, but WITHOUT ANY
 WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 FOR A PARTICULAR PURPOSE.  Full license text is available on the following
 link: http://www.freertos.org/a00114.html

 ***************************************************************************
 *                                                                       *
 *    FreeRTOS provides completely free yet professionally developed,    *
 *    robust, strictly quality controlled, supported, and cross          *
 *    platform software that is more than just the market leader, it     *
 *    is the industry's de facto standard.                               *
 *                                                                       *
 *    Help yourself get started quickly while simultaneously helping     *
 *    to support the FreeRTOS project by purchasing a FreeRTOS           *
 *    tutorial book, reference manual, or both:                          *
 *    http://www.FreeRTOS.org/Documentation                              *
 *                                                                       *
 ***************************************************************************

 http://www.FreeRTOS.org/FAQHelp.html - Having a problem?  Start by reading
 the FAQ page "My application does not run, what could be wrong?".  Have you
 defined configASSERT()?

 http://www.FreeRTOS.org/support - In return for receiving this top quality
 embedded software for free we request you assist our global community by
 participating in the support forum.

 http://www.FreeRTOS.org/training - Investing in training allows your team to
 be as productive as possible as early as possible.  Now you can receive
 FreeRTOS training directly from Richard Barry, CEO of Real Time Engineers
 Ltd, and the world's leading authority on the world's leading RTOS.

 http://www.FreeRTOS.org/plus - A selection of FreeRTOS ecosystem products,
 including FreeRTOS+Trace - an indispensable productivity tool, a DOS
 compatible FAT file system, and our tiny thread aware UDP/IP stack.

 http://www.FreeRTOS.org/labs - Where new FreeRTOS products go to incubate.
 Come and try FreeRTOS+TCP, our new open source TCP/IP stack for FreeRTOS.

 http://www.OpenRTOS.com - Real Time Engineers ltd. license FreeRTOS to High
 Integrity Systems ltd. to sell under the OpenRTOS brand.  Low cost OpenRTOS
 licenses offer ticketed support, indemnification and commercial middleware.

 http://www.SafeRTOS.com - High Integrity Systems also provide a safety
 engineered and independently SIL3 certified version for use in safety and
 mission critical applications that require provable dependability.

 1 tab == 4 spaces!
 */

/******************************************************************************
 *
 * http://www.FreeRTOS.org/cli
 *
 ******************************************************************************/

/* Standard includes. */
#include <stdlib.h>
#include <string.h>
#include "shell.h"
#include "gcode.h"


/* HAL includes. */
#include "stm32f4xx_hal.h"

/* FreeRTOS+CLI includes. */
#include "FreeRTOS.h"
#include "FreeRTOS_CLI.h"

#include "task.h"

#ifndef  configINCLUDE_TRACE_RELATED_CLI_COMMANDS
#define configINCLUDE_TRACE_RELATED_CLI_COMMANDS 0
#endif

#ifndef configINCLUDE_QUERY_HEAP_COMMAND
#define configINCLUDE_QUERY_HEAP_COMMAND 0
#endif

#define PRV_COMMAND_SIGNATURE \
char *pcWriteBuffer, \
size_t xWriteBufferLen, \
const char *pcCommandString

/*----------------------------------------------------------------------------*/

/*
 * Implements the "!" command (execute g-code).
 */
static BaseType_t
prvExecCommand(PRV_COMMAND_SIGNATURE);

/*
 * Implements the "led" command.
 */
static BaseType_t
prvLedCommand(PRV_COMMAND_SIGNATURE);

/*
 * Implements the "man" command.
 */
static BaseType_t
prvManCommand(PRV_COMMAND_SIGNATURE);

/*
 * Implements the "start" command.
 */
static BaseType_t
prvStartCommand(PRV_COMMAND_SIGNATURE);


/*
 * Implements the "task-stats" command.
 */
static BaseType_t
prvTaskStatsCommand(PRV_COMMAND_SIGNATURE);

/*
 * Implements the "run-time-stats" command.
 */
#if(configGENERATE_RUN_TIME_STATS == 1)
static BaseType_t
prvRunTimeStatsCommand PRV_COMMAND;
#endif /* configGENERATE_RUN_TIME_STATS */

/*
 * Implements the echo-three-parameters command.
 */
static BaseType_t
prvThreeParameterEchoCommand(PRV_COMMAND_SIGNATURE);

/*
 * Implements the echo-parameters command.
 */
static BaseType_t
prvParameterEchoCommand(PRV_COMMAND_SIGNATURE);

/*
 * Implements the "query heap" command.
 */
#if(configINCLUDE_QUERY_HEAP_COMMAND == 1)
static BaseType_t
prvQueryHeapCommand PRV_COMMAND;
#endif

/*
 * Implements the "trace start" and "trace stop" commands;
 */
#if(configINCLUDE_TRACE_RELATED_CLI_COMMANDS == 1)
static BaseType_t
prvStartStopTraceCommand PRV_COMMAND;
#endif

/*----------------------------------------------------------------------------*/



/* Structure that defines the "!" command line command.  This executes g-code provided. */
static const CLI_Command_Definition_t xExecCommand = {
  "!",
  "\r\n"
  "! <G-code string>:\r\n"
  "  Execute single line g-code.\r\n\r\n",
  prvExecCommand,
  1
};

/* Structure that defines the "led" command line command.  This switch on/off
 * and toggle the LEDs. */
static const CLI_Command_Definition_t xLedCommand = {
  "led",
  "\r\n"
  "led [red | green | blue] [on | off | toggle]:\r\n"
  "  Switch on/off and toggle the LEDs.\r\n\r\n",
  prvLedCommand,
  2
};

/* Structure that defines the "start" command line command.  This displays the
 * manual page of the command. */
static const CLI_Command_Definition_t xManCommand = {
  "man",
  "\r\n"
  "man <command>:\r\n"
  "  Displays the manual page of the command.\r\n",
  prvManCommand,
  1
};

/* Structure that defines the "start" command line command.  This generates
 * an answer "ready" only. */
static const CLI_Command_Definition_t xStart = {
  "start", /* The command string to type. */
  "\r\n"
  "start:\r\n"
  "  Displays an answer \"ready\" only nya.\r\n",
  prvStartCommand, /* The function to run. */
  0 /* No parameters are expected. */
};


/* Structure that defines the "task-stats" command line command.  This generates
 * a table that gives information on each task in the system. */
static const CLI_Command_Definition_t xTaskStats = {
  "task-stats", /* The command string to type. */
  "\r\n"
  "task-stats:\r\n"
  "  Displays a table showing the state of each FreeRTOS task.\r\n",
  prvTaskStatsCommand, /* The function to run. */
  0 /* No parameters are expected. */
};

/* Structure that defines the "echo_3_parameters" command line command.  This
 * takes exactly three parameters that the command simply echos back one at a
 * time. */
static const CLI_Command_Definition_t xThreeParameterEcho = {
  "echo-3-parameters",
  "\r\n"
  "echo-3-parameters <param1> <param2> <param3>:\r\n"
  "  Expects three parameters, echos each in turn.\r\n",
  prvThreeParameterEchoCommand, /* The function to run. */
  3 /* Three parameters are expected, which can take any value. */
};

/* Structure that defines the "echo_parameters" command line command.  This
 * takes a variable number of parameters that the command simply echos back one
 *  at a time. */
static const CLI_Command_Definition_t xParameterEcho = {
  "echo-parameters",
  "\r\n"
  "echo-parameters <...>:\r\n"
  "  Take variable number of parameters, echos each in turn.\r\n",
  prvParameterEchoCommand, /* The function to run. */
  -1 /* The user can enter any number of commands. */
};

#if(configGENERATE_RUN_TIME_STATS == 1)
/* Structure that defines the "run-time-stats" command line command.  This
 * generates a table that shows how much run time each task has */
static const CLI_Command_Definition_t xRunTimeStats = {
  "run-time-stats", /* The command string to type. */
  "\r\n"
  "run-time-stats:\r\n"
  "  Displays a table showing how much processing time each FreeRTOS task has"
  "used.\r\n",
  prvRunTimeStatsCommand, /* The function to run. */
  0 /* No parameters are expected. */
};
#endif /* configGENERATE_RUN_TIME_STATS */

#if(configINCLUDE_QUERY_HEAP_COMMAND == 1)
/* Structure that defines the "query_heap" command line command. */
static const CLI_Command_Definition_t xQueryHeap = {
  "query-heap",
  "\r\n"
  "query-heap:\r\n"
  "Displays the free heap space, and minimum ever free heap space.\r\n",
  prvQueryHeapCommand, /* The function to run. */
  0 /* The user can enter any number of commands. */
};
#endif /* configQUERY_HEAP_COMMAND */

#if configINCLUDE_TRACE_RELATED_CLI_COMMANDS == 1
/* Structure that defines the "trace" command line command.  This takes a single
 * parameter, which can be either "start" or "stop". */
static const CLI_Command_Definition_t xStartStopTrace = {
  "trace",
  "\r\n"
  "trace [start | stop]:\r\n"
  "Starts or stops a trace recording for viewing in FreeRTOS+Trace.\r\n",
  prvStartStopTraceCommand, /* The function to run. */
  1 /* One parameter is expected.  Valid values are "start" and "stop". */
};
#endif /* configINCLUDE_TRACE_RELATED_CLI_COMMANDS */

/*-----------------------------------------------------------*/

void vRegisterSampleCLICommands(void) {
  /* Register all the command line commands defined immediately above. */
  FreeRTOS_CLIRegisterCommand(&xManCommand);
  FreeRTOS_CLIRegisterCommand(&xStart);
  FreeRTOS_CLIRegisterCommand(&xLedCommand);

  FreeRTOS_CLIRegisterCommand(&xTaskStats);
  FreeRTOS_CLIRegisterCommand(&xThreeParameterEcho);
  FreeRTOS_CLIRegisterCommand(&xParameterEcho);
  FreeRTOS_CLIRegisterCommand(&xExecCommand);

#if(configGENERATE_RUN_TIME_STATS == 1)
  {
    FreeRTOS_CLIRegisterCommand(&xRunTimeStats);
  }
#endif

#if(configINCLUDE_QUERY_HEAP_COMMAND == 1)
  {
    FreeRTOS_CLIRegisterCommand(&xQueryHeap);
  }
#endif

#if(configINCLUDE_TRACE_RELATED_CLI_COMMANDS == 1)
  {
    FreeRTOS_CLIRegisterCommand(&xStartStopTrace);
  }
#endif
}

/*----------------------------------------------------------------------------*/

static BaseType_t
prvExecCommand(
  char* pcWriteBuffer,
  size_t xWriteBufferLen,
  const char* pcCommandString
) {
  //	BaseType_t xParametersListValid = pdTRUE;

  BaseType_t xGcodeLength;
  char* pcGcode;

  //	parser_block_t gc_block;

  configASSERT(pcWriteBuffer);
  memset(pcWriteBuffer, 0x00, xWriteBufferLen);

  /* Obtain the led color. */
  pcGcode = FreeRTOS_CLIGetParameter(
    pcCommandString, /* The command string itself. */
    1, /* Return the first parameter. */
    &xGcodeLength /* Store the parameter string length. */
  );

  uint8_t res = gc_execute_line(pcGcode);

  /* Generate an answer. */
  sprintf(
    pcWriteBuffer,
    "\r\n Return code is %d \r\n",
    res
  );
  //	itoa(res, pcWriteBuffer, 10);
  //	strcpy(pcWriteBuffer, "\r\nready\r\n");

  /* There is no more data to return after this single string, so return
   * pdFALSE. */
  return pdFALSE;
}

/*----------------------------------------------------------------------------*/

static BaseType_t
prvLedCommand(
  char* pcWriteBuffer,
  size_t xWriteBufferLen,
  const char* pcCommandString
) {
  BaseType_t xParametersListValid = pdTRUE;

  BaseType_t xLedColorCmdStringLength;
  BaseType_t xOnOffCmdStringLength;
  const char* pcLedColorCmd;
  const char* pcOnOffCmd;

  uint16_t usLedColor = LD3_Pin;

  uint16_t usLedState;
  static const uint16_t usLedOn = 1, usLedOff = 0, usLedToggle = 2;
  static const char* const pcInvalidColorMessage =
    "\r\n"
    "Invalid led color. You should enter \"red\", \"green\" "
    "or \"blue\".\r\n";
  static const char* const pcInvalidCommandMessage =
    "\r\n"
    "Invalid command. You should enter \"on\", \"off\" "
    "or \"toggle\".\r\n";
  static const char* const pcLedAlreadySwitchedOnMessage =
    "\r\n"
    "The LED has already been switched on.\r\n";
  static const char* const pcLedAlreadySwitchedOffMessage =
    "\r\n"
    "The LED has already been switched off.\r\n";

  configASSERT(pcWriteBuffer);
  memset(pcWriteBuffer, 0x00, xWriteBufferLen);

  /* Obtain the led color. */
  pcLedColorCmd = FreeRTOS_CLIGetParameter(
    pcCommandString,         /* The command string itself. */
    1,                 /* Return the first parameter. */
    &xLedColorCmdStringLength /* Store the parameter string length. */
  );

  /* Obtain the command: on or off. */
  pcOnOffCmd = FreeRTOS_CLIGetParameter(
    pcCommandString,      /* The command string itself. */
    2,              /* Return the first parameter. */
    &xOnOffCmdStringLength /* Store the parameter string length. */
  );

  /* Determine color of the led. */
  if (strncmp(pcLedColorCmd, "red", xLedColorCmdStringLength) == 0) {
    usLedColor = LD5_Pin;
  } else if (strncmp(pcLedColorCmd, "green", xLedColorCmdStringLength) == 0) {
    usLedColor = LD4_Pin;
  } else if (strncmp(pcLedColorCmd, "blue", xLedColorCmdStringLength) == 0) {
    usLedColor = LD6_Pin;
  } else {
    strcpy(pcWriteBuffer, pcInvalidColorMessage);
    xParametersListValid &= pdFALSE;
  }

  /* Determine command */
  if (strncmp(pcOnOffCmd, "on", xOnOffCmdStringLength) == 0) {
    usLedState = usLedOn;
  } else if (strncmp(pcOnOffCmd, "off", xOnOffCmdStringLength) == 0) {
    usLedState = usLedOff;
  } else if (strncmp(pcOnOffCmd, "toggle", xOnOffCmdStringLength) == 0) {
    usLedState = usLedToggle;
  } else {
    strncat(
      pcWriteBuffer,
      pcInvalidCommandMessage,
      (size_t) pcInvalidCommandMessage
    );
    xParametersListValid &= pdFALSE;
  }

  if (xParametersListValid == pdTRUE) {
    if (usLedState == usLedOn) {
      if (HAL_GPIO_ReadPin(GPIOD, usLedColor) == GPIO_PIN_SET) {
        strcpy(pcWriteBuffer, pcLedAlreadySwitchedOnMessage);
      } else {
        HAL_GPIO_WritePin(GPIOD, usLedColor, GPIO_PIN_SET);
        strcpy(pcWriteBuffer, "\r\ndone\r\n");
      }
    } else if (usLedState == usLedOff) {
      if (HAL_GPIO_ReadPin(GPIOD, usLedColor) == GPIO_PIN_RESET) {
        strcpy(pcWriteBuffer, pcLedAlreadySwitchedOffMessage);
      } else {
        HAL_GPIO_WritePin(GPIOD, usLedColor, GPIO_PIN_RESET);
        strcpy(pcWriteBuffer, "\r\ndone\r\n");
      }
    } else {
      HAL_GPIO_TogglePin(GPIOD, usLedColor);
      strcpy(pcWriteBuffer, "\r\ndone\r\n");
    }
  }

  /* There is no more data to return after this single string, so return
   * pdFALSE. */
  return pdFALSE;
}

/*----------------------------------------------------------------------------*/

static BaseType_t
prvManCommand(
  char* pcWriteBuffer,
  //		size_t xWriteBufferLen __attribute__((unused)),
  size_t xWriteBufferLen,
  const char* pcCommandString
) {
  const CLI_Definition_List_Item_t* pxCommand = NULL;
  const char* pcRegisteredCommandString;
  const char* pcParameter;
  BaseType_t xParameterStringLength;
  size_t xCommandStringLength;

  /* Obtain the parameter string. */
  pcParameter = FreeRTOS_CLIGetParameter(
    pcCommandString,      /* The command string itself. */
    1,              /* Return the first parameter. */
    &xParameterStringLength  /* Store the parameter string length. */
  );

  /* Sanity check something was returned. */
  configASSERT(pcParameter);
  memset(pcWriteBuffer, 0x00, xWriteBufferLen);

  /* Search for the command string in the list of registered commands. */
  for (pxCommand = FreeRTOS_CLIGetRegisteredCommands();
       pxCommand != NULL;
       pxCommand = pxCommand->pxNext) {
    pcRegisteredCommandString =
      pxCommand->pxCommandLineDefinition->pcCommand;
    xCommandStringLength = strlen(pcRegisteredCommandString);

    if (strncmp(
      pcParameter,
      pcRegisteredCommandString,
      xCommandStringLength) == 0
      ) {
      strcpy(
        pcWriteBuffer,
        pxCommand->pxCommandLineDefinition->pcHelpString
      );
      return pdFALSE;
    }
  }

  if (strncmp(pcParameter, "nya", strlen("nya")) == 0) {
    strcpy(
      pcWriteBuffer,
      "\r\n"
      " _\r\n"
      " \\`*-.\r\n"
      "  )  _`-.\r\n"
      " .  : `. .        MEOW\r\n"
      " : _   '  \\\r\n"
      " ; *` _.   `*-._\r\n"
      " `-.-'          `-.\r\n"
      "   ;       `       `.\r\n"
      "   :.       .        \\\r\n"
      "   . \\  .   :   .-'   .\r\n"
      "   '  `+.;  ;  '      :\r\n"
      "   :  '  |    ;       ;-.\r\n"
      "   ; '   : :`-:     _.`* ;\r\n"
      ".*' /  .*' ; .*`- +'  `*'\r\n"
      "`*-*   `*-*  `*-*'\r\n"
    );
  } else {
    strcpy(
      pcWriteBuffer,
      "\r\n"
      "Command not recognised. "
      "Type \"help\" to view a list of available commands.\r\n"
    );
  }
  return pdFALSE;
}

/*----------------------------------------------------------------------------*/

static BaseType_t
prvStartCommand(
  char* pcWriteBuffer,
  size_t xWriteBufferLen __attribute__((unused)),
  const char* pcCommandString __attribute__((unused))
) {
  configASSERT(pcWriteBuffer);

  /* Generate an answer. */
  strcpy(pcWriteBuffer, "\r\nready\r\n");

  /* There is no more data to return after this single string, so return
   * pdFALSE. */
  return pdFALSE;
}


/*----------------------------------------------------------------------------*/

static BaseType_t
prvTaskStatsCommand(
  char* pcWriteBuffer,
  size_t xWriteBufferLen,
  const char* pcCommandString
) {
  const char* const pcHeader =
    "     State   Priority  Stack    #\r\n"
    "************************************************\r\n";
  BaseType_t xSpacePadding;

  /* Remove compile time warnings about unused parameters, and check the
   * write buffer is not NULL.  NOTE - for simplicity, this example assumes
   * the write buffer length is adequate, so does not check for buffer
   * overflows. */
  (void) pcCommandString;
  (void) xWriteBufferLen;
  configASSERT(pcWriteBuffer);

  /* Generate a table of task stats. */
  strcpy(pcWriteBuffer, "\r\nTask");
  pcWriteBuffer += strlen(pcWriteBuffer);

  /* Minus three for the null terminator and half the number of characters in
   * "Task" so the column lines up with the centre of the heading. */
  configASSERT(configMAX_TASK_NAME_LEN > 3);
  for (xSpacePadding = strlen("Task");
       xSpacePadding < (configMAX_TASK_NAME_LEN - 3);
       xSpacePadding++
    ) {
    /* Add a space to align columns after the task's name. */
    *pcWriteBuffer = ' ';
    pcWriteBuffer++;

    /* Ensure always terminated. */
    *pcWriteBuffer = 0x00;
  }
  strcpy(pcWriteBuffer, pcHeader);
  vTaskList(pcWriteBuffer + strlen(pcHeader));

  /* There is no more data to return after this single string, so return
   * pdFALSE. */
  return pdFALSE;
}
/*-----------------------------------------------------------*/

#if(configINCLUDE_QUERY_HEAP_COMMAND == 1)

static BaseType_t
prvQueryHeapCommand(
    char *pcWriteBuffer,
    size_t xWriteBufferLen,
    const char *pcCommandString
) {
  /* Remove compile time warnings about unused parameters, and check the
   * write buffer is not NULL.  NOTE - for simplicity, this example assumes
   * the write buffer length is adequate, so does not check for buffer
   * overflows. */
  (void) pcCommandString;
  (void) xWriteBufferLen;
  configASSERT(pcWriteBuffer);

  sprintf(
      pcWriteBuffer,
      "\r\n"
      "Current free heap %d bytes, "
      "minimum ever free heap %d bytes\r\n",
      (int) xPortGetFreeHeapSize(),
      (int) xPortGetMinimumEverFreeHeapSize()
  );

  /* There is no more data to return after this single string, so return
   * pdFALSE. */
  return pdFALSE;
}

#endif /* configINCLUDE_QUERY_HEAP */
/*----------------------------------------------------------------------------*/

#if(configGENERATE_RUN_TIME_STATS == 1)

static BaseType_t
prvRunTimeStatsCommand(
    char *pcWriteBuffer,
    size_t xWriteBufferLen,
    const char *pcCommandString
) {
  const char * const pcHeader =
      "\r\n"
      "  Abs Time      % Time\r\n"
      "****************************************\r\n";
  BaseType_t xSpacePadding;

  /* Remove compile time warnings about unused parameters, and check the
   * write buffer is not NULL.  NOTE - for simplicity, this example assumesthe
   * write buffer length is adequate, so does not check for buffer
   * overflows. */
  (void) pcCommandString;
  (void) xWriteBufferLen;
  configASSERT(pcWriteBuffer);

  /* Generate a table of task stats. */
  strcpy(pcWriteBuffer, "Task");
  pcWriteBuffer += strlen(pcWriteBuffer);

  /* Pad the string "task" with however many bytes necessary to make it the
   * length of a task name.  Minus three for the null terminator and half the
   * number of characters in	"Task" so the column lines up with the centre of
   * the heading. */
  for(xSpacePadding = strlen("Task");
    xSpacePadding < (configMAX_TASK_NAME_LEN - 3);
    xSpacePadding++
  ) {
    /* Add a space to align columns after the task's name. */
    *pcWriteBuffer = ' ';
    pcWriteBuffer++;

    /* Ensure always terminated. */
    *pcWriteBuffer = 0x00;
  }

  strcpy(pcWriteBuffer, pcHeader);
  vTaskGetRunTimeStats(pcWriteBuffer + strlen(pcHeader));

  /* There is no more data to return after this single string, so return
   * pdFALSE. */
  return pdFALSE;
}

#endif /* configGENERATE_RUN_TIME_STATS */

/*----------------------------------------------------------------------------*/

static BaseType_t
prvThreeParameterEchoCommand(
  char* pcWriteBuffer,
  size_t xWriteBufferLen,
  const char* pcCommandString
) {
  const char* pcParameter;
  BaseType_t xParameterStringLength, xReturn;
  static UBaseType_t uxParameterNumber = 0;

  /* Remove compile time warnings about unused parameters, and check the
   * write buffer is not NULL.  NOTE - for simplicity, this example assumes
   * the write buffer length is adequate, so does not check for buffer
   * overflows. */
  (void) pcCommandString;
  (void) xWriteBufferLen;
  configASSERT(pcWriteBuffer);

  if (uxParameterNumber == 0) {
    /* The first time the function is called after the command has been
     * entered just a header string is returned. */
    sprintf(pcWriteBuffer, "\r\nThe three parameters were:\r\n");

    /* Next time the function is called the first parameter will be echoed
     * back. */
    uxParameterNumber = 1U;

    /* There is more data to be returned as no parameters have been echoed
     back yet. */
    xReturn = pdPASS;
  } else {
    /* Obtain the parameter string. */
    pcParameter = FreeRTOS_CLIGetParameter(
      pcCommandString, /* The command string itself. */
      uxParameterNumber, /* Return the next parameter. */
      &xParameterStringLength /* Store the parameter string length. */
    );

    /* Sanity check something was returned. */
    configASSERT(pcParameter);

    /* Return the parameter string. */
    memset(pcWriteBuffer, 0x00, xWriteBufferLen);
    sprintf(pcWriteBuffer, "%d: ", (int) uxParameterNumber);
    strncat(pcWriteBuffer, pcParameter, (size_t) xParameterStringLength);
    strncat(pcWriteBuffer, "\r\n", strlen("\r\n"));

    /* If this is the last of the three parameters then there are no more
     * strings to return after this one. */
    if (uxParameterNumber == 3U) {
      /* If this is the last of the three parameters then there are no
       * more strings to return after this one. */
      xReturn = pdFALSE;
      uxParameterNumber = 0;
    } else {
      /* There are more parameters to return after this one. */
      xReturn = pdTRUE;
      uxParameterNumber++;
    }
  }

  return xReturn;
}

/*----------------------------------------------------------------------------*/

static BaseType_t
prvParameterEchoCommand(
  char* pcWriteBuffer,
  size_t xWriteBufferLen,
  const char* pcCommandString
) {
  const char* pcParameter;
  BaseType_t xParameterStringLength, xReturn;
  static UBaseType_t uxParameterNumber = 0;

  /* Remove compile time warnings about unused parameters, and check the
   * write buffer is not NULL.  NOTE - for simplicity, this example assumes
   * the write buffer length is adequate, so does not check for buffer
   * overflows. */
  (void) pcCommandString;
  (void) xWriteBufferLen;

  configASSERT(pcWriteBuffer);

  if (uxParameterNumber == 0) {
    /* The first time the function is called after the command has been
     * entered just a header string is returned. */
    sprintf(pcWriteBuffer, "\r\nThe parameters were:\r\n");

    /* Next time the function is called the first parameter will be echoed
     * back. */
    uxParameterNumber = 1U;

    /* There is more data to be returned as no parameters have been echoed
     * back yet. */
    xReturn = pdPASS;
  } else {
    /* Obtain the parameter string. */
    pcParameter = FreeRTOS_CLIGetParameter(
      pcCommandString, /* The command string itself. */
      uxParameterNumber, /* Return the next parameter. */
      &xParameterStringLength /* Store the parameter string length. */
    );

    if (pcParameter != NULL) {
      /* Return the parameter string. */
      memset(pcWriteBuffer, 0x00, xWriteBufferLen);
      sprintf(pcWriteBuffer, "%d: ", (int) uxParameterNumber);
      strncat(
        pcWriteBuffer,
        (char*) pcParameter,
        (size_t) xParameterStringLength
      );
      strncat(pcWriteBuffer, "\r\n", strlen("\r\n"));

      /* There might be more parameters to return after this one. */
      xReturn = pdTRUE;
      uxParameterNumber++;
    } else {
      /* No more parameters were found.  Make sure the write buffer does
       * not contain a valid string. */
      pcWriteBuffer[0] = 0x00;

      /* No more data to return. */
      xReturn = pdFALSE;

      /* Start over the next time this command is executed. */
      uxParameterNumber = 0;
    }
  }

  return xReturn;
}
/*----------------------------------------------------------------------------*/

#if configINCLUDE_TRACE_RELATED_CLI_COMMANDS == 1

static BaseType_t
prvStartStopTraceCommand(
    char *pcWriteBuffer,
    size_t xWriteBufferLen,
    const char *pcCommandString
) {
  const char *pcParameter;
  BaseType_t lParameterStringLength;

  /* Remove compile time warnings about unused parameters, and check the
   * write buffer is not NULL.  NOTE - for simplicity, this example assumes
   * the write buffer length is adequate, so does not check for buffer
   * overflows. */
  (void) pcCommandString;
  (void) xWriteBufferLen;
  configASSERT(pcWriteBuffer);

  /* Obtain the parameter string. */
  pcParameter = FreeRTOS_CLIGetParameter(
      pcCommandString, /* The command string itself. */
      1, /* Return the first parameter. */
      &lParameterStringLength /* Store the parameter string length. */
  );

  /* Sanity check something was returned. */
  configASSERT(pcParameter);

  /* There are only two valid parameter values. */
  if(strncmp(pcParameter, "start", strlen("start")) == 0) {
    /* Start or restart the trace. */
    vTraceStop();
    vTraceClear();
    vTraceStart();

    sprintf(pcWriteBuffer, "\r\nTrace recording (re)started.\r\n");
  } else if(strncmp(pcParameter, "stop", strlen("stop")) == 0) {
    /* End the trace, if one is running. */
    vTraceStop();
    sprintf(pcWriteBuffer, "\r\nStopping trace recording.\r\n");
  } else {
    sprintf(pcWriteBuffer, "\r\nValid parameters are 'start' and 'stop'.\r\n");
  }

  /* There is no more data to return after this single string, so return
   pdFALSE. */
  return pdFALSE;
}

#endif /* configINCLUDE_TRACE_RELATED_CLI_COMMANDS */
