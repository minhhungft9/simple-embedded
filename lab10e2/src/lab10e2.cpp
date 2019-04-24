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
#include "DigitalIoPin.h"

QueueHandle_t queue;
SemaphoreHandle_t binary;
TimerHandle_t oneShot;
bool ledOn = false;

static void prvSetupHardware(void)
{
	SystemCoreClockUpdate();
	Board_Init();

	/* Initial LED0 state is off */
	Board_LED_Set(0, false);
	Board_LED_Set(1, false);
	Board_LED_Set(2, false);
}

void task1(void *pvParameter){
	DigitalIoPin sw1(0, 17, DigitalIoPin::pullup, true);
	DigitalIoPin sw3(1, 9, DigitalIoPin::pullup, true);
	while(1){
		if(sw1.read() || sw3.read()){
			if(ledOn == false){
				ledOn = true;
				Board_UARTPutSTR("Button pressed, turn LED back on\r\n");
			}else{
				Board_UARTPutSTR("Button pressed, timer resetted\r\n");
			}
			xTimerReset(oneShot, 0);
		}

		if(ledOn == true){
			Board_LED_Set(1, true);
		}else{
			Board_LED_Set(1, false);
		}
	}
}

static void oneShotCallBack(TimerHandle_t timer){
	if(ledOn == true)
		ledOn = false;
		Board_UARTPutSTR("Time out, turn LED off\r\n");
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
	oneShot = xTimerCreate("one shot", 5000, pdFALSE, 0, oneShotCallBack);
	xTimerStart(oneShot, 0);
	xTaskCreate(task1, "LED task", configMINIMAL_STACK_SIZE*4, NULL, (tskIDLE_PRIORITY + 1UL), (TaskHandle_t *) NULL);
	vTaskStartScheduler();
    return 0 ;
}
