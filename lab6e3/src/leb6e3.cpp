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
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <mutex>
#include "Fmutex.h"
#include "user_vcom.h"

volatile uint32_t RIT_count;
xSemaphoreHandle sbRIT;
xSemaphoreHandle endTest;
int pps = 1000;
volatile bool state = true;
volatile bool direction = false;
volatile int average = 0;
volatile int stepNumber = 0;
bool counting = true;

DigitalIoPin *direct;
DigitalIoPin *step;
DigitalIoPin *sw1;
DigitalIoPin *sw2;

static void prvSetupHardware(void)
{
	SystemCoreClockUpdate();
	Board_Init();

	/* Initial LED0 state is off */
	Board_LED_Set(0, false);

	// initialize RIT (= enable clocking etc.)
	Chip_RIT_Init(LPC_RITIMER);
	// set the priority level of the interrupt
	// The level must be equal or lower than the maximum priority specified in FreeRTOS config
	// Note that in a Cortex-M3 a higher number indicates lower interrupt priority
	NVIC_SetPriority( RITIMER_IRQn, configLIBRARY_MAX_SYSCALL_INTERRUPT_PRIORITY + 1 );
}

void RIT_start(int count, int us)
{
	uint64_t cmp_value;
	// Determine approximate compare value based on clock rate and passed interval
	cmp_value = (uint64_t) Chip_Clock_GetSystemClockRate() * (uint64_t) us / 1000000;
	// disable timer during configuration
	Chip_RIT_Disable(LPC_RITIMER);
	RIT_count = count;
	// enable automatic clear on when compare value==timer value
	// this makes interrupts trigger periodically
	Chip_RIT_EnableCompClear(LPC_RITIMER);
	// reset the counter
	Chip_RIT_SetCounter(LPC_RITIMER, cmp_value);
	Chip_RIT_SetCompareValue(LPC_RITIMER, cmp_value);
	// start counting
	Chip_RIT_Enable(LPC_RITIMER);
	// Enable the interrupt signal in NVIC (the interrupt controller)
	NVIC_EnableIRQ(RITIMER_IRQn);
	// wait for ISR to tell that we're done
	if(xSemaphoreTake(sbRIT, portMAX_DELAY) == pdTRUE) {
		// Disable the interrupt signal in NVIC (the interrupt controller)
		NVIC_DisableIRQ(RITIMER_IRQn);
	}
	else {
		// unexpected error
	}
}

void task1(void *pvParameter){
	while(1){
		if(sw1->read()){
			Board_LED_Set(0, true);
			vTaskDelay(500);
		}else if(sw2->read()){
			Board_LED_Set(1, true);
			vTaskDelay(500);
		}else{
			Board_LED_Set(0, false);
			Board_LED_Set(1, false);
		}
	}
}

void task2(void *pvParameter){
	char buffer[80];
	if(counting == true){
		int times = 0;
		while(times < 5){
			RIT_start(100000, 500);
			if(times == 0){
				average = 0;
			}
			times++;
		}
		average = average/4 + 4;
		if(average % 2 != 0){
			average = average + 1;
		}
		counting = false;
		vTaskDelay(500);
	}
	snprintf(buffer, 80, "\r\nAverage step: %d, speed test starts\r\n", average);
	USB_send((uint8_t *) buffer, strlen(buffer));
	while(1){
		TickType_t startTime = xTaskGetTickCount();
		if(pps >= 2500){
			int multi = 1;
			for(int i = 0; i < 5; i++){
				RIT_start(4, 1000000 / (multi * pps));
				multi += 0.2;
			}
			RIT_start(average-20, 1000000 / (2 * pps));
		}else{
			RIT_start(average, 1000000 / (2 * pps));
		}
		TickType_t endTime = xTaskGetTickCount();
		vTaskDelay(50);
		if((direction == true && !sw1->read()) || (direction == false && !sw2->read())){
			snprintf(buffer, 80, "\r\nSpeed test ends, max pps %d, max rpm %d", pps, (60*pps/400));
			USB_send((uint8_t *) buffer, strlen(buffer));
			step->write(false);
			break;
		}
		snprintf(buffer, 80, "\r\nSpeed test pps: %d, time: %.4fs, step number: %d", pps, (float)(endTime-startTime)/1000.0, stepNumber);
		USB_send((uint8_t *) buffer, strlen(buffer));
		stepNumber = 0;
		direction = !direction;
		pps += 100;
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

extern "C" {
void RIT_IRQHandler(void)
{
	// This used to check if a context switch is required
	portBASE_TYPE xHigherPriorityWoken = pdFALSE;
	// Tell timer that we have processed the interrupt.
	// Timer then removes the IRQ until next match occurs
	Chip_RIT_ClearIntStatus(LPC_RITIMER); // clear IRQ flag
	bool left = (sw1->read() && direction == true);
	bool right = (sw2->read() && direction == false);
	if(RIT_count > 0) {
		RIT_count--;
		direct->write(direction);
		if(counting == true){
			if(left||right){
				direction = !direction;
				average += stepNumber;
				stepNumber = 0;
				step->write(false);
				Chip_RIT_Disable(LPC_RITIMER); // disable timer
				// Give semaphore and set context switch flag if a higher priority task was woken up
				xSemaphoreGiveFromISR(sbRIT, &xHigherPriorityWoken);
			}else{
				step->write(state);
				state = !state;
				stepNumber++;
			}
		}else{
			step->write(state);
			state = !state;
			stepNumber++;
		}
	}
	else {
		Chip_RIT_Disable(LPC_RITIMER); // disable timer
		// Give semaphore and set context switch flag if a higher priority task was woken up
		xSemaphoreGiveFromISR(sbRIT, &xHigherPriorityWoken);
	}
	// End the ISR and (possibly) do a context switch
	portEND_SWITCHING_ISR(xHigherPriorityWoken);
}
}

int main(void) {

	prvSetupHardware();
	sbRIT = xSemaphoreCreateBinary();
	endTest = xSemaphoreCreateBinary();
	direct = new DigitalIoPin(1, 0, DigitalIoPin::output, true);
	step = new DigitalIoPin(0, 24, DigitalIoPin::output, true);
	sw1 = new DigitalIoPin(0, 27, DigitalIoPin::pullup, true);
	sw2 = new DigitalIoPin(0, 28, DigitalIoPin::pullup, true);

	xTaskCreate(task1, "LED task", configMINIMAL_STACK_SIZE*3, NULL, (tskIDLE_PRIORITY + 1UL), (TaskHandle_t *) NULL);
	xTaskCreate(task2, "Move step", configMINIMAL_STACK_SIZE*4, NULL, (tskIDLE_PRIORITY + 1UL), (TaskHandle_t *) NULL);
	xTaskCreate(cdc_task, "CDC", configMINIMAL_STACK_SIZE*2, NULL, (tskIDLE_PRIORITY + 1UL), (TaskHandle_t *) NULL);
	vTaskStartScheduler();

    return 0 ;
}
