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
SemaphoreHandle_t go;

int pps = 400;
volatile bool state = false;
volatile bool direction = false;
volatile int average = 0;
volatile int stepNumber = 0;

DigitalIoPin *direct;
DigitalIoPin *step;
DigitalIoPin *sw1;
DigitalIoPin *sw2;

QueueHandle_t queue;

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

struct command {
	int type;
	int data;
};

void commandHandle(char *text){
	command c;
	if(strncmp(text, "left", 4) == 0){
		c.type = 1;
		c.data = strtol(text+4, NULL, 10);
		BaseType_t status = xQueueSendToBack(queue, &c, 0);
		if (status == errQUEUE_FULL) {
			char err[] = "Error: command queue full\r\n";
			USB_send((uint8_t *) err, strlen(err));
		}
	}else if(strncmp(text, "right", 5) == 0){
		c.type = 2;
		c.data = strtol(text+5, NULL, 10);
		BaseType_t status = xQueueSendToBack(queue, &c, 0);
		if (status == errQUEUE_FULL) {
			char err[] = "Error: command queue full\r\n";
			USB_send((uint8_t *) err, strlen(err));
		}
	}else if(strncmp(text, "pps", 3) == 0){
		c.type = 3;
		c.data = strtol(text+3, NULL, 10);
		BaseType_t status = xQueueSendToBack(queue, &c, 0);
		if (status == errQUEUE_FULL) {
			char err[] = "Error: command queue full\r\n";
			USB_send((uint8_t *) err, strlen(err));
		}
	}else if(strncmp(text, "stop", 4) == 0){
		c.type = 4;
		c.data = 0;
		BaseType_t status = xQueueSendToFront(queue, &c, 0);
		if (status == errQUEUE_FULL) {
			char err[] = "Error: command queue full\r\n";
			USB_send((uint8_t *) err, strlen(err));
		}
	}else if(strncmp(text, "go", 2) == 0){
		xSemaphoreGive(go);
	}
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
	Chip_RIT_SetCounter(LPC_RITIMER, 0);
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
	int times = 0;
	while(times < 3){
		RIT_start(100000, 250);
		if(times == 0){
			average = 0;
		}
		times++;
	}
	average = average/2;
	RIT_start(average/2, 250);
	while(1){
		char buffer[RCV_BUFFERS+1] = "";
		int index = 0;
		while(index < 20){
			int len = USB_receive((uint8_t *) (buffer + index), RCV_BUFFERS);
			char *pos = strstr((buffer + index), "\r");
			if (pos != NULL) {
				USB_send((uint8_t *) (buffer + index), pos - (buffer + index) + 1);
				USB_send((uint8_t *) "\n", 1);
				break;
			} else {
				USB_send((uint8_t *) (buffer + index), len);
			}
			index += len;
		}
		commandHandle(buffer);
	}
}

void task2(void *pvParameter){
	command c;
	bool run = false;
	while(1){
		if(xSemaphoreTake(go, portMAX_DELAY) == pdPASS){
			run = true;
		}
		while(run){
			if(xQueueReceive(queue, &c, portMAX_DELAY) == pdPASS){
				if(c.type == 1){
					direction = true;
					RIT_start(c.data, 1000000 / (2 * pps));
				}else if(c.type == 2){
					direction = false;
					RIT_start(c.data, 1000000 / (2 * pps));
				}else if(c.type == 3){
					pps = c.data;
				}else if(c.type == 4){
					step->write(false);
					run = false;
					break;
				}
			}
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

extern "C" {
void RIT_IRQHandler(void)
{
	// This used to check if a context switch is required
	portBASE_TYPE xHigherPriorityWoken = pdFALSE;
	// Tell timer that we have processed the interrupt.
	// Timer then removes the IRQ until next match occurs
	Chip_RIT_ClearIntStatus(LPC_RITIMER); // clear IRQ flag
	if(RIT_count > 0) {
		RIT_count--;
		// do something useful here...
		direct->write(direction);
		if((sw1->read() && direction == true)||(sw2->read() && direction == false)){
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
	queue = xQueueCreate(5, sizeof(command));
	go = xSemaphoreCreateBinary();
	direct = new DigitalIoPin(1, 0, DigitalIoPin::output, true);
	step = new DigitalIoPin(0, 24, DigitalIoPin::output, true);
	sw1 = new DigitalIoPin(0, 27, DigitalIoPin::pullup, true);
	sw2 = new DigitalIoPin(0, 28, DigitalIoPin::pullup, true);

	xTaskCreate(task1, "Read command", configMINIMAL_STACK_SIZE*3, NULL, (tskIDLE_PRIORITY + 1UL), (TaskHandle_t *) NULL);
	xTaskCreate(task2, "Move step", configMINIMAL_STACK_SIZE*3, NULL, (tskIDLE_PRIORITY + 1UL), (TaskHandle_t *) NULL);
	xTaskCreate(cdc_task, "CDC", configMINIMAL_STACK_SIZE*3, NULL, (tskIDLE_PRIORITY + 1UL), (TaskHandle_t *) NULL);
	vTaskStartScheduler();

    return 0 ;
}
