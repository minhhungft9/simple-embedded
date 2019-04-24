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
#include "math.h"
#include <cstdio>

volatile uint32_t RIT_count;
xSemaphoreHandle sbRIT;
void commandHandle(char *text);
void RIT_start(int count, int us);
int pps = 400;
volatile bool state = false;
volatile bool direction = false;

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

void task1(void *pvParameter){
	int wordNumber = -1;
	char command[60];
	while(1){
		if(wordNumber < 0){
			Board_UARTPutSTR("\r\nEnter command:\r\n");
			wordNumber++;
		}
		int c = Board_UARTGetChar();
		if(c != EOF){
			Board_UARTPutChar(c);
			command[wordNumber] = c;
			wordNumber++;
			if((c == '\r') || (wordNumber >= 60)){
				commandHandle(command);
				wordNumber = -1;
			}
		}
	}
}

void commandHandle(char *text){
	if(text[0] == 'l' && text[1] == 'e' && text[2] == 'f' && text[3] == 't'){
		direction = true;
		char number[5];
		int stepNumber = 0;
		int index = 0;
		bool valid = true;
		for(int i = 0; i < 4; i++){
			if(text[index+5] < 0x30 || text[index+5] > 0x39){
				valid = false;
			}
		}
		if(valid){
			while(index < 4){
				if(text[index+5] < 0x30 || text[index+5] > 0x39){
					break;
				}
				number[index] = text[index+5];
				index++;
			}
			for(int i = 0; i < index; i++){
				stepNumber += (static_cast<int>(number[i])-48)*pow(10,index-i-1);
			}
			text[0] = '\0';
			Board_UARTPutSTR("\r\nMoving to the left...\r\n");
			RIT_start(2*stepNumber, 1000000 / (2 * pps));
		}else{
			Board_UARTPutSTR("\r\nInvalid input\r\n");
		}
	}else if(text[0] == 'r' && text[1] == 'i' && text[2] == 'g' && text[3] == 'h' && text[4] == 't'){
		direction = false;
		char number[5];
		int stepNumber = 0;
		int index = 0;
		bool valid = true;
		for(int i = 0; i < 4; i++){
			if(text[index+6] < 0x30 || text[index+6] > 0x39){
				valid = false;
			}
		}
		if(valid){
			while(index < 4){
				if(text[index+6] < 0x30 || text[index+6] > 0x39){
					break;
				}
				number[index] = text[index+6];
				index++;
			}
			for(int i = 0; i < index; i++){
				stepNumber += (static_cast<int>(number[i])-48)*pow(10,index-i-1);
			}
			text[0] = '\0';
			Board_UARTPutSTR("\r\nMoving to the right...\r\n");
			RIT_start(2*stepNumber, 1000000 / (2 * pps));
		}else{
			Board_UARTPutSTR("\r\nInvalid input\r\n");
		}
	}else if(text[0] == 'p' && text[1] == 'p' && text[2] == 's'){
		pps = 0;
		char number[5];
		int index = 0;
		bool valid = true;
		for(int i = 0; i < 4; i++){
			if(text[index+4] < 0x30 || text[index+4] > 0x39){
				valid = false;
			}
		}
		if(valid){
			while(index < 4){
				if(text[index+4] < 0x30 || text[index+4] > 0x39){
					break;
				}
				number[index] = text[index+4];
				index++;
			}
			for(int i = 0; i < index; i++){
				pps += (static_cast<int>(number[i])-48)*pow(10,index-i-1);
			}
			if(pps == 0){
				pps = 400;
			}
			Board_UARTPutSTR("\r\nPPS set\r\n");
			text[0] = '\0';
		}else{
			Board_UARTPutSTR("\r\nInvalid input\r\n");
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
			step->write(false);
			Chip_RIT_Disable(LPC_RITIMER); // disable timer
			// Give semaphore and set context switch flag if a higher priority task was woken up
			xSemaphoreGiveFromISR(sbRIT, &xHigherPriorityWoken);
		}else{
			step->write(state);
			state = !state;
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

int main(void) {

	sbRIT = xSemaphoreCreateBinary();
	prvSetupHardware();
	direct = new DigitalIoPin(1, 0, DigitalIoPin::output, true);
	step = new DigitalIoPin(0, 24, DigitalIoPin::output, true);
	sw1 = new DigitalIoPin(0, 27, DigitalIoPin::pullup, true);
	sw2 = new DigitalIoPin(0, 28, DigitalIoPin::pullup, true);
	xTaskCreate(task1, "Move step", configMINIMAL_STACK_SIZE*3, NULL, (tskIDLE_PRIORITY + 1UL), (TaskHandle_t *) NULL);
	vTaskStartScheduler();

    return 0 ;
}
