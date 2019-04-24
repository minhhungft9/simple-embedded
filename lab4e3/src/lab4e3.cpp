/*
 * main.cpp
 *
 *  Created on: Sep 4, 2017
 *      Author: Administrator
 */
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
#include "semphr.h"
#include <cstdio>
#include <cstring>
#include <ctype.h>
#include <stdlib.h>
#include "ITM_write.h"
#define STEP_REDUCION 3

SemaphoreHandle_t binary = xSemaphoreCreateBinary();

static void prvSetupHardware(void)
{
	SystemCoreClockUpdate();
	Board_Init();

	/* Initial LED0 state is off */
	Board_LED_Set(0, false);
}

void task1(void *pvParameter){
	DigitalIoPin sw1(0, 27, DigitalIoPin::pullup, true);
	DigitalIoPin sw2(0, 28, DigitalIoPin::pullup, true);
	bool run = false;

	while(1){
		if(run == true){
			if(sw1.read() && sw2.read()){
				Board_LED_Set(0, false);
				Board_LED_Set(1, false);
				run = false;
			}else if(sw1.read()){
				Board_LED_Set(0, true);
			}else if(sw2.read()){
				Board_LED_Set(1, true);
			}else{
				Board_LED_Set(0, false);
				Board_LED_Set(1, false);
			}
		}else{
			if(sw1.read() && sw2.read()){
				bool state = true;
				Board_LED_Set(2, state);
				state = !state;
				vTaskDelay(100);
			}else{
				Board_LED_Set(2, false);
				run = true;
				xSemaphoreGive(binary);
			}
		}
	}
}

void task2(void *pvParameter){
	DigitalIoPin sw1(0, 27, DigitalIoPin::pullup, true);
	DigitalIoPin sw2(0, 28, DigitalIoPin::pullup, true);

	DigitalIoPin direct(1, 0, DigitalIoPin::output, true);
	DigitalIoPin step(0, 24, DigitalIoPin::output, true);
	bool direction = false;
	int stepNumber = 0;
	char buffer[50];
	int average = 0;
	int index = 0;
	xSemaphoreTake(binary, portMAX_DELAY);
	while(index < 5){
		direct.write(direction);
		step.write(false);
		vTaskDelay(1);
		step.write(true);
		vTaskDelay(1);
		stepNumber++;
		if(sw1.read() && direction == true){
			if(index == 0){
				direction = !direction;
				index++;
				stepNumber = 0;
				continue;
			}
			average += stepNumber;
			index++;
			direction = !direction;
			snprintf(buffer, 50, "Stepper hit switch 1: %d steps\r\n", stepNumber);
			ITM_write(buffer);
			stepNumber = 0;
		}else if(sw2.read() && direction == false){
			if(index == 0){
				direction = !direction;
				index++;
				stepNumber = 0;
				continue;
			}
			average += stepNumber;
			index++;
			direction = !direction;
			snprintf(buffer, 50, "Stepper hit switch 2: %d steps\r\n", stepNumber);
			ITM_write(buffer);
			stepNumber = 0;
		}
	}
	average = average / 4;
	snprintf(buffer, 50, "\r\nAverage step number: %d\r\n", average);
	ITM_write(buffer);
	Board_LED_Set(2, true);
	vTaskDelay(2000);
	Board_LED_Set(2, false);
	direction = true;
	ITM_write("Start...\r\n");
	while(1){
		direct.write(direction);
		step.write(false);
		vTaskDelay(1);
		step.write(true);
		vTaskDelay(1);
		stepNumber++;
		if((stepNumber == average - STEP_REDUCION) && direction == true){
			direction = !direction;
			stepNumber = STEP_REDUCION;
		}else if((stepNumber == average - STEP_REDUCION) && direction == false){
			direction = !direction;
			stepNumber = STEP_REDUCION;
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

	ITM_init();
	prvSetupHardware();
	xTaskCreate(task1, "Read switch", configMINIMAL_STACK_SIZE, NULL, (tskIDLE_PRIORITY + 1UL), (TaskHandle_t *) NULL);
	xTaskCreate(task2, "Move step", configMINIMAL_STACK_SIZE + 256, NULL, (tskIDLE_PRIORITY + 1UL), (TaskHandle_t *) NULL);
	vTaskStartScheduler();

    return 0 ;
}
