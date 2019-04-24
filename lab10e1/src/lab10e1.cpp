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

QueueHandle_t queue;
SemaphoreHandle_t binary;
TimerHandle_t oneShot;
TimerHandle_t autoReload;

static void prvSetupHardware(void)
{
	SystemCoreClockUpdate();
	Board_Init();

	/* Initial LED0 state is off */
	Board_LED_Set(0, false);
}

void task1(void *pvParameter){
	char buffer[20];
	while(1){
		xQueueReceive(queue, &buffer, portMAX_DELAY);
		Board_UARTPutSTR(buffer);
	}
}

void task2(void *pvParameter){
	while(1){
		if(xSemaphoreTake(binary, portMAX_DELAY) == pdPASS){
			char buffer[20];
			snprintf(buffer, 20, "aargh\r\n");
			xQueueSendToBack(queue, &buffer, portMAX_DELAY);
		}
	}
}

static void oneShotCallBack(TimerHandle_t timer){
	xSemaphoreGive(binary);
}

static void autoReloadCallBack(TimerHandle_t timer){
	char buffer[20];
	snprintf(buffer, 20, "hello\r\n");
	xQueueSendToBack(queue, &buffer, portMAX_DELAY);
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
	oneShot = xTimerCreate("one shot", 20000, pdFALSE, 0, oneShotCallBack);
	autoReload = xTimerCreate("auto reload", 5000, pdTRUE, 0, autoReloadCallBack);
	xTimerStart(autoReload, 0);
	xTimerStart(oneShot, 0);
	xTaskCreate(task1, "printing task", configMINIMAL_STACK_SIZE*4, NULL, (tskIDLE_PRIORITY + 1UL), (TaskHandle_t *) NULL);
	xTaskCreate(task2, "sending command", configMINIMAL_STACK_SIZE*4, NULL, (tskIDLE_PRIORITY + 1UL), (TaskHandle_t *) NULL);
	vTaskStartScheduler();
    return 0 ;
}
