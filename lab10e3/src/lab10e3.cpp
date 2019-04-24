/*
===============================================================================
 Name        : main.c
 Author      : $(author)
 Version     :
 Copyright   : $(copyright)
 Description : main definition
===============================================================================
*/

#if defined (__USE_LPCOPEN)
#if defined(NO_BOARD_LIB)
#include "chip.h"
#else
#include "board.h"
#endif
#endif

#include <cr_section_macros.h>
#include "FreeRTOS.h"
#include "task.h"
#include "semphr.h"
#include "queue.h"
#include "timers.h"
#include <cstdio>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>

void commandHandle();
QueueHandle_t queue;
SemaphoreHandle_t binary;
TimerHandle_t oneShot;
TimerHandle_t autoReload;
TickType_t lastReceive = 0;
char command[60];
bool ledOn = true;

static void prvSetupHardware(void)
{
	SystemCoreClockUpdate();
	Board_Init();

	/* Initial LED0 state is off */
	Board_LED_Set(0, false);
}

void task1(void *pvParameter){
	int wordNumber = 0;
	while(1){
		int c = Board_UARTGetChar();
		if(c != EOF){
			xTimerReset(oneShot, 0);
			Board_UARTPutChar(c);
			command[wordNumber] = c;
			wordNumber++;
			if((c == '\r') || (wordNumber >= 60)){
				commandHandle();
				wordNumber = 0;
				command[0] = '\0';
			}
		}
		if(ledOn == true){
			Board_LED_Set(1, true);
		}else{
			Board_LED_Set(1, false);
		}
	}
}

void commandHandle(){
	TickType_t now = xTaskGetTickCount();
	if(strncmp(command, "help", 4) == 0){
		Board_UARTPutSTR("\r\nIf no characters are received in 30 seconds all the characters received so far are discarded.\n");
		Board_UARTPutSTR("\rThe commands are:\n");
		Board_UARTPutSTR("\r\tinterval <number> - set the led toggle interval (default is 5 seconds)\n");
		Board_UARTPutSTR("\r\ttime – show elapsed time since the last led toggle\r\n");
	}else if(strncmp(command, "interval ", 9) == 0){
		int length = strtol(command+9, NULL, 10);
		if(length <= 0){
			length = 5;
		}
		xTimerChangePeriod(autoReload, length*configTICK_RATE_HZ, 0);
	}else if(strncmp(command, "time", 4) == 0){
		float elapsedTime = (now - lastReceive)/1000.0;
		if(lastReceive > 0){
			char buffer[80];
			snprintf(buffer, 80, "\r\nElapsed time since the last led toggle: %.1fs\r\n", elapsedTime);
			Board_UARTPutSTR(buffer);
		}
	}
	Board_UARTPutSTR("\r\n");
}

static void oneShotCallBack(TimerHandle_t timer){
	command[0] = '\0';
	Board_UARTPutSTR("[Inactive]\r\n");
}

static void autoReloadCallBack(TimerHandle_t timer){
	ledOn = !ledOn;
	if(ledOn == false){
		lastReceive = xTaskGetTickCount();
	}
}

/*****************************************************************************
 * Public functions
 ****************************************************************************/

/* the following is required if runtime statistics are to be collected */
extern "C" {

void vConfigureTimerForRunTimeStats( void ) {
	Chip_SCT_Init(LPC_SCTSMALL1);
	LPC_SCTSMALL1->CONFIG = SCT_CONFIG_32BIT_COUNTER;
	LPC_SCTSMALL1->CTRL_U = SCT_CTRL_PRE_L(255) | SCT_CTRL_CLRCTR_L; // set prescaler to 256 (255 + 1), and start timer
}

}
/* end runtime statictics collection */

/**
 * @brief	main routine for FreeRTOS blinky example
 * @return	Nothing, function should not exit
 */

int main(void) {

	prvSetupHardware();
	queue = xQueueCreate(2, sizeof(char[20]));
	binary = xSemaphoreCreateBinary();
	oneShot = xTimerCreate("one shot", 30000, pdFALSE, 0, oneShotCallBack);
	autoReload = xTimerCreate("auto reload", 5000, pdTRUE, 0, autoReloadCallBack);
	xTimerStart(oneShot, 0);
	xTimerStart(autoReload, 0);
	xTaskCreate(task1, "command task", configMINIMAL_STACK_SIZE*4, NULL, (tskIDLE_PRIORITY + 1UL), (TaskHandle_t *) NULL);
	vTaskStartScheduler();
    return 0 ;
}
