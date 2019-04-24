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

static void prvSetupHardware(void)
{
	SystemCoreClockUpdate();
	Board_Init();

	/* Initial LED0 state is off */
	Board_LED_Set(0, false);
}

void task1(void *pvParameter){
	DigitalIoPin sw1(0, 17, DigitalIoPin::pullup, true);
	bool pressed = false;
	while(1){
		if(sw1.read() && pressed == false){
			vTaskDelay(100);
			if(sw1.read()){
				pressed = true;
			}
		}else if(!sw1.read() && pressed == true){
			xEventGroupSetBits(eventBit, ( 1 << 1 ));
			pressed = false;
		}
	}
}

void task2(void *pvParameter){
	DigitalIoPin sw2(1, 11, DigitalIoPin::pullup, true);
	bool pressed = false;
	while(1){
		if(sw2.read() && pressed == false){
			vTaskDelay(100);
			if(sw2.read()){
				pressed = true;
			}
		}else if(!sw2.read() && pressed == true){
			xEventGroupSetBits(eventBit, ( 1 << 2 ));
			pressed = false;
		}
	}
}

void task3(void *pvParameter){
	DigitalIoPin sw3(1, 9, DigitalIoPin::pullup, true);
	bool pressed = false;
	while(1){
		if(sw3.read() && pressed == false){
			vTaskDelay(100);
			if(sw3.read()){
				pressed = true;
			}
		}else if(!sw3.read() && pressed == true){
			xEventGroupSetBits(eventBit, ( 1 << 3 ));
			pressed = false;
		}
	}
}

void task4(void *pvParameter){
	TickType_t TIME_OUT = 10*configTICK_RATE_HZ;
	TickType_t b1 = 0;
	TickType_t b2 = 0;
	TickType_t b3 = 0;
	TickType_t lastReceive = 0;
	EventBits_t receive;
	const EventBits_t waitBit = (1 << 1) | (1 << 2) | (1 << 3);
	while(1){
		receive = xEventGroupWaitBits(eventBit, waitBit, pdTRUE, pdTRUE, TIME_OUT);
		TickType_t now = xTaskGetTickCount();
		if(receive == waitBit){
			char buffer[30];
			snprintf(buffer, 30, "OK, elapsed tick: %lu \r\n", now - lastReceive);
			mutex.write(buffer);
			b1 = b2 = b3 = lastReceive = now;
		}else{
			char buffer[60];
			if ((receive & (1 << 1)) == 0) {
				snprintf(buffer, 60, "Fail at task 1, elapsed tick: %lu \r\n", now - b1);
				mutex.write(buffer);
			}else{
				b1 = now;
			}
			if ((receive & (1 << 2)) == 0) {
				snprintf(buffer, 60, "Fail at task 2, elapsed tick: %lu \r\n", now - b2);
				mutex.write(buffer);
			}else{
				b2 = now;
			}
			if ((receive & (1 << 3)) == 0) {
				snprintf(buffer, 60, "Fail at task 3, elapsed tick: %lu \r\n", now - b3);
				mutex.write(buffer);
			}else{
				b3 = now;
			}
			xEventGroupClearBits(eventBit, waitBit);
		}
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
	eventBit = xEventGroupCreate();
	xTaskCreate(task1, "sw1", configMINIMAL_STACK_SIZE*4, NULL, (tskIDLE_PRIORITY + 1UL), (TaskHandle_t *) NULL);
	xTaskCreate(task2, "sw2", configMINIMAL_STACK_SIZE*4, NULL, (tskIDLE_PRIORITY + 1UL), (TaskHandle_t *) NULL);
	xTaskCreate(task3, "sw3", configMINIMAL_STACK_SIZE*4, NULL, (tskIDLE_PRIORITY + 1UL), (TaskHandle_t *) NULL);
	xTaskCreate(task4, "watchdog", configMINIMAL_STACK_SIZE*4, NULL, (tskIDLE_PRIORITY + 1UL), (TaskHandle_t *) NULL);
	vTaskStartScheduler();

    return 0 ;
}
