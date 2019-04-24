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
#include "semphr.h"
#include "Syslock.h"
#include "event_groups.h"

Syslock mutex = Syslock();
EventGroupHandle_t eventBit;
const EventBits_t waitBit = (1 << 1) | (1 << 2) | (1 << 3);

static void prvSetupHardware(void)
{
	SystemCoreClockUpdate();
	Board_Init();

	/* Initial LED0 state is off */
	Board_LED_Set(0, false);
}

void task1(void *pvParameter){
	DigitalIoPin sw1(0, 17, DigitalIoPin::pullup, true);
	int times = 1;
	bool pressed = false;
	while(1){
		if(times == 0){
			xEventGroupSetBits(eventBit, ( 1 << 1 ));
			break;
		}

		if(sw1.read() && pressed == false){
			vTaskDelay(100);
			if(sw1.read()){
				times--;
				pressed = true;
			}else if(sw1.read() && pressed == true){

			}
		}else if(sw1.read() == false){
			pressed = false;
		}
	}

	TickType_t elapsedTick = xTaskGetTickCount();
	xEventGroupWaitBits(eventBit, waitBit, pdFALSE, pdTRUE, portMAX_DELAY);
	char buffer[40];
	snprintf(buffer, 40, "Task 1, elapsed tick: %lu \r\n", elapsedTick);
	mutex.write(buffer);

	while(1);
}

void task2(void *pvParameter){
	DigitalIoPin sw2(1, 11, DigitalIoPin::pullup, true);
	int times = 2;
	bool pressed = false;
	while(1){
		if(times == 0){
			xEventGroupSetBits(eventBit, ( 1 << 2 ));
			break;
		}

		if(sw2.read() && pressed == false){
			vTaskDelay(100);
			if(sw2.read()){
				times--;
				pressed = true;
			}else if(sw2.read() && pressed == true){

			}
		}else if(sw2.read() == false){
			pressed = false;
		}
	}

	TickType_t elapsedTick = xTaskGetTickCount();
	xEventGroupWaitBits(eventBit, waitBit, pdFALSE, pdTRUE, portMAX_DELAY);
	char buffer[40];
	snprintf(buffer, 40, "Task 2, elapsed tick: %lu \r\n", elapsedTick);
	mutex.write(buffer);

	while(1);
}

void task3(void *pvParameter){
	DigitalIoPin sw3(1, 9, DigitalIoPin::pullup, true);
	int times = 3;
	bool pressed = false;
	while(1){
		if(times == 0){
			xEventGroupSetBits(eventBit, ( 1 << 3 ));
			break;
		}

		if(sw3.read() && pressed == false){
			vTaskDelay(100);
			if(sw3.read()){
				times--;
				pressed = true;
			}else if(sw3.read() && pressed == true){

			}
		}else if(sw3.read() == false){
			pressed = false;
		}
	}

	TickType_t elapsedTick = xTaskGetTickCount();
	xEventGroupWaitBits(eventBit, waitBit, pdFALSE, pdTRUE, portMAX_DELAY);
	char buffer[40];
	snprintf(buffer, 40, "Task 3, elapsed tick: %lu \r\n", elapsedTick);
	mutex.write(buffer);

	while(1);
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
	eventBit = xEventGroupCreate();
	xTaskCreate(task1, "sw1", configMINIMAL_STACK_SIZE*4, NULL, (tskIDLE_PRIORITY + 1UL), (TaskHandle_t *) NULL);
	xTaskCreate(task2, "sw2", configMINIMAL_STACK_SIZE*4, NULL, (tskIDLE_PRIORITY + 1UL), (TaskHandle_t *) NULL);
	xTaskCreate(task3, "sw3", configMINIMAL_STACK_SIZE*4, NULL, (tskIDLE_PRIORITY + 1UL), (TaskHandle_t *) NULL);
	vTaskStartScheduler();

    return 0 ;
}
