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
#include "ITM_write.h"
#include "DigitalIoPin.h"
#include <mutex>
#include "Fmutex.h"
#include "user_vcom.h"
#include "semphr.h"
#include <cstdio>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>

const long FILTER = 200;
const long TIME_OUT = 15000;
QueueHandle_t queue;
bool learning = false;
int passWord[8] = {1, 1, 1, 1, 0, 1, 1, 0};
int input[8];
void setupPin();

struct pinTime{
	int pin;
	TickType_t time;
};

bool unlock(){
	bool unlock = true;
	for(int i = 0; i < 8; i++){
		if(input[i] != passWord[i]){
			unlock = false;
			break;
		}
	}
	return unlock;
}

void resetInput(){
	for(int i = 0; i < 8; i++){
		input[i] = 2;
	}
}

void shiftAndAdd(int n){
	for(int i = 0; i < 8; i++){
		input[i] = input[i+1];
	}
	input[7] = n;
}

/* the following is required if runtime statistics are to be collected */
extern "C" {

void PIN_INT1_IRQHandler(void) {
    // This used to check if a context switch is required
    BaseType_t xHigherPriorityWoken = pdFALSE;

    // Clear interrupt status
    LPC_GPIO_PIN_INT->IST = (1 << 1);

    pinTime p;
    p.pin = 0;
    p.time = xTaskGetTickCountFromISR();
    xQueueSendToBackFromISR(queue, &p, &xHigherPriorityWoken);

    // End the ISR and (possibly) do a context switch
    portEND_SWITCHING_ISR(xHigherPriorityWoken);
}

void PIN_INT2_IRQHandler(void) {
    // This used to check if a context switch is required
    BaseType_t xHigherPriorityWoken = pdFALSE;

    // Clear interrupt status
    LPC_GPIO_PIN_INT->IST = (1 << 2);

    pinTime p;
    p.pin = 1;
    p.time = xTaskGetTickCountFromISR();
    xQueueSendToBackFromISR(queue, &p, &xHigherPriorityWoken);

    // End the ISR and (possibly) do a context switch
    portEND_SWITCHING_ISR(xHigherPriorityWoken);
}

void vConfigureTimerForRunTimeStats( void ) {
	Chip_SCT_Init(LPC_SCTSMALL1);
	LPC_SCTSMALL1->CONFIG = SCT_CONFIG_32BIT_COUNTER;
	LPC_SCTSMALL1->CTRL_U = SCT_CTRL_PRE_L(255) | SCT_CTRL_CLRCTR_L; // set prescaler to 256 (255 + 1), and start timer
}

}

void LED(bool red, bool green, bool blue){
	Board_LED_Set(0, red);
	Board_LED_Set(1, green);
	Board_LED_Set(2, blue);
}
/* end runtime statictics collection */

/* Sets up system hardware */
static void prvSetupHardware(void)
{
	SystemCoreClockUpdate();
	Board_Init();
	setupPin();

	/* Initial LED0 state is off */
	Board_LED_Set(0, false);
	Board_LED_Set(1, false);
	Board_LED_Set(2, false);
}

void receive_task(void *pvParameter){
	DigitalIoPin sw3(1, 9, DigitalIoPin::pullup, true);
	TickType_t pressTime = 0;
	while(1){
		if(sw3.read()){
			pressTime++;
			if(pressTime == 3000){
				pinTime p;
				p.pin = 3;
				p.time = xTaskGetTickCount();
				xQueueSendToBack(queue, &p, portMAX_DELAY);
				pressTime = 0;
			}
		}else if(!sw3.read()){
				pressTime = 0;
		}
		vTaskDelay(1);
	}
}

void lock_task(void *pvParameter){
	pinTime p;
	pinTime lastP;
	lastP.pin = 0;
	lastP.time = 0;
	int index = 0;
	TickType_t now;
	resetInput();
	Board_UARTPutSTR("\r\nCurrent password: ");
	for(int i = 0; i < 8; i++){
		char buffer[10];
		snprintf(buffer, 10, "%d", passWord[i]);
		Board_UARTPutSTR(buffer);
	}
	Board_UARTPutSTR("\r\n");
	while(1){
		if(learning == false){
			LED(true, false, false);
		}else{
			LED(false, false, true);
		}

		BaseType_t status = xQueueReceive(queue, &p, TIME_OUT);
		if(status == errQUEUE_EMPTY){
			Board_UARTPutSTR("\r\nTime out, all previous key presses are ignored.\r\n");
			resetInput();
			index = 0;
		}else{
			if((p.time - lastP.time) > FILTER){
				if(p.pin == 3){
					learning = true;
					continue;
				}
				lastP.time = p.time;
				if(learning == true){
					input[index] = p.pin;
					index++;
					if(index == 8){
						for(int i = 0; i < index; i++){
							passWord[i] = input[i];
						}
						Board_UARTPutSTR("\r\nNew password: ");
						for(int i = 0; i < index; i++){
							char buffer[10];
							snprintf(buffer, 10, "%d", passWord[i]);
							Board_UARTPutSTR(buffer);
						}
						Board_UARTPutSTR("\r\n");
						resetInput();
						index = 0;
						learning = false;
					}
				}
				else if(learning == false){
					char buffer[24];
					snprintf(buffer, 24, "%d", p.pin);
					Board_UARTPutSTR(buffer);
					shiftAndAdd(p.pin);
					if(unlock()){
						resetInput();
						LED(false, true, false);
						vTaskDelay(configTICK_RATE_HZ*5);
					}
				}
			}
		}
	}
}

void setupPin(){
	Chip_IOCON_PinMuxSet(LPC_IOCON, 0, 17, IOCON_DIGMODE_EN | IOCON_MODE_INACT | IOCON_MODE_PULLUP);
	Chip_IOCON_PinMuxSet(LPC_IOCON, 1, 11, IOCON_DIGMODE_EN | IOCON_MODE_INACT | IOCON_MODE_PULLUP);
	//Chip_IOCON_PinMuxSet(LPC_IOCON, 1, 9, IOCON_DIGMODE_EN | IOCON_MODE_INACT | IOCON_MODE_PULLUP);

	Chip_GPIO_SetPinDIRInput(LPC_GPIO, 0, 17);
	Chip_GPIO_SetPinDIRInput(LPC_GPIO, 1, 11);
	//Chip_GPIO_SetPinDIRInput(LPC_GPIO, 1, 9);

	 /* Enable PININT clock */
	Chip_Clock_EnablePeriphClock(SYSCTL_CLOCK_PININT);

	/* Reset the PININT block */
	Chip_SYSCTL_PeriphReset(RESET_PININT);

	/* Configure interrupt channel for the GPIO pin in INMUX block */
	Chip_INMUX_PinIntSel(1, 0, 17);
	Chip_INMUX_PinIntSel(2, 1, 11);
	//Chip_INMUX_PinIntSel(3, 1, 9);

	/* Configure channel interrupt as edge sensitive and falling edge interrupt */
	//LPC_GPIO_PIN_INT->IST = (1 << 1) | (1 << 2) | (1 << 3); // Clear interrupt status
	//LPC_GPIO_PIN_INT->ISEL &= ~(1 << 1) | ~(1 << 2) | ~(1 << 3); // Configure the pins as edge sensitive
	//LPC_GPIO_PIN_INT->SIENF = (1 << 1) | (1 << 2) | (1 << 3); // Enable low edge PININT interrupts
	Chip_PININT_ClearIntStatus(LPC_GPIO_PIN_INT, PININTCH1);
	Chip_PININT_SetPinModeEdge(LPC_GPIO_PIN_INT, PININTCH1);
	Chip_PININT_EnableIntLow(LPC_GPIO_PIN_INT, PININTCH1);

	Chip_PININT_ClearIntStatus(LPC_GPIO_PIN_INT, PININTCH2);
	Chip_PININT_SetPinModeEdge(LPC_GPIO_PIN_INT, PININTCH2);
	Chip_PININT_EnableIntLow(LPC_GPIO_PIN_INT, PININTCH2);

	//Chip_PININT_ClearIntStatus(LPC_GPIO_PIN_INT, PININTCH3);
	//Chip_PININT_SetPinModeEdge(LPC_GPIO_PIN_INT, PININTCH3);
	//Chip_PININT_EnableIntLow(LPC_GPIO_PIN_INT, PININTCH3);

	/* Enable interrupt in the NVIC */
	NVIC_ClearPendingIRQ(PIN_INT1_IRQn);
	NVIC_ClearPendingIRQ(PIN_INT2_IRQn);
	//NVIC_ClearPendingIRQ(PIN_INT3_IRQn);
	NVIC_EnableIRQ(PIN_INT1_IRQn);
	NVIC_EnableIRQ(PIN_INT2_IRQn);
	//NVIC_EnableIRQ(PIN_INT3_IRQn);
}

int main(void) {
	queue = xQueueCreate(10, sizeof(pinTime));
	prvSetupHardware();

	xTaskCreate(receive_task, "Rx", configMINIMAL_STACK_SIZE*3, NULL, (tskIDLE_PRIORITY + 1UL), (TaskHandle_t *) NULL);
	xTaskCreate(lock_task, "Lx", configMINIMAL_STACK_SIZE*3, NULL, (tskIDLE_PRIORITY + 1UL), (TaskHandle_t *) NULL);
	vTaskStartScheduler();
	return 1;
}
