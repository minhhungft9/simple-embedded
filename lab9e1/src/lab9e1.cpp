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
#include "DigitalIoPin.h"
#include "ITM_write.h"
#include "time.h"
#include <stdlib.h>
#include "semphr.h"
#include "event_groups.h"

SemaphoreHandle_t mutex;
EventGroupHandle_t eventBit;

static void prvSetupHardware(void)
{
	SystemCoreClockUpdate();
	Board_Init();

	/* Initial LED0 state is off */
	Board_LED_Set(0, false);
}

void task1(void *pvParameter){
	DigitalIoPin sw1(0, 17, DigitalIoPin::pullup, true);
	while(1){
		if(sw1.read()){
			Board_UARTPutSTR("\r\nSW1 pressed\r\n");
			xSemaphoreGive(mutex);
			xEventGroupSetBits(eventBit, ( 1 << 0 ));
		}
	}
}

void task2(void *pvParameter){
	xEventGroupWaitBits(eventBit, ( 1 << 0 ), pdTRUE, pdTRUE, portMAX_DELAY);
	while(1){
		xSemaphoreTake(mutex, portMAX_DELAY);
		char buffer[40];
		float k = rand();
		while (k > 1)
			k = k / 10.0;
		int delayTime = (k+1)*configTICK_RATE_HZ;
		snprintf(buffer, 40, "\r\nTask 2, elapsed ticks: %lu", xTaskGetTickCount());
		Board_UARTPutSTR(buffer);
		xSemaphoreGive(mutex);
		vTaskDelay(delayTime);
	}
}

void task3(void *pvParameter){
	xEventGroupWaitBits(eventBit, ( 1 << 0 ), pdTRUE, pdTRUE, portMAX_DELAY);
	while(1){
		xSemaphoreTake(mutex, portMAX_DELAY);
		char buffer[40];
		float k = rand();
		while (k > 1)
			k = k / 10.0;
		int delayTime = (k+1)*configTICK_RATE_HZ;
		snprintf(buffer, 40, "\r\nTask 3, elapsed ticks: %lu", xTaskGetTickCount());
		Board_UARTPutSTR(buffer);
		xSemaphoreGive(mutex);
		vTaskDelay(delayTime);
	}
}

void task4(void *pvParameter){
	xEventGroupWaitBits(eventBit, ( 1 << 0 ), pdTRUE, pdTRUE, portMAX_DELAY);
	while(1){
		xSemaphoreTake(mutex, portMAX_DELAY);
		char buffer[40];
		float k = rand();
		while (k > 1)
			k = k / 10.0;
		int delayTime = (k+1)*configTICK_RATE_HZ;
		snprintf(buffer, 40, "\r\nTask 4, elapsed ticks: %lu", xTaskGetTickCount());
		Board_UARTPutSTR(buffer);
		xSemaphoreGive(mutex);
		vTaskDelay(delayTime);
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
	srand(time(NULL));
	mutex = xSemaphoreCreateMutex();
	eventBit = xEventGroupCreate();
	xTaskCreate(task1, "send bit", configMINIMAL_STACK_SIZE, NULL, (tskIDLE_PRIORITY + 1UL), (TaskHandle_t *) NULL);
	xTaskCreate(task3, "wait for bit", configMINIMAL_STACK_SIZE*4, NULL, (tskIDLE_PRIORITY + 1UL), (TaskHandle_t *) NULL);
	xTaskCreate(task4, "wait for bit", configMINIMAL_STACK_SIZE*4, NULL, (tskIDLE_PRIORITY + 1UL), (TaskHandle_t *) NULL);
	xTaskCreate(task2, "wait for bit", configMINIMAL_STACK_SIZE*4, NULL, (tskIDLE_PRIORITY + 1UL), (TaskHandle_t *) NULL);
	vTaskStartScheduler();

    return 0 ;
}
