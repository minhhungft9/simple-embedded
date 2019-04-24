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

// TODO: insert other include files here
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

QueueHandle_t queue;
void setupPin();
void commandHandle(char *text);
long filter = 50;

struct pinTime{
	int pin;
	TickType_t time;
};
/* the following is required if runtime statistics are to be collected */
extern "C" {

void PIN_INT1_IRQHandler(void) {
	pinTime p;
    // This used to check if a context switch is required
    BaseType_t xHigherPriorityWoken = pdFALSE;

    // Clear interrupt status
    LPC_GPIO_PIN_INT->IST = (1 << 1);

    p.pin = 1;
    p.time = xTaskGetTickCountFromISR();
    xQueueSendToBackFromISR(queue, &p, &xHigherPriorityWoken);

    // End the ISR and (possibly) do a context switch
    portEND_SWITCHING_ISR(xHigherPriorityWoken);
}

void PIN_INT2_IRQHandler(void) {
	pinTime p;
    // This used to check if a context switch is required
    BaseType_t xHigherPriorityWoken = pdFALSE;

    // Clear interrupt status
    LPC_GPIO_PIN_INT->IST = (1 << 2);

    p.pin = 2;
    p.time = xTaskGetTickCountFromISR();
    xQueueSendToBackFromISR(queue, &p, &xHigherPriorityWoken);

    // End the ISR and (possibly) do a context switch
    portEND_SWITCHING_ISR(xHigherPriorityWoken);
}

void PIN_INT3_IRQHandler(void) {
	pinTime p;
    // This used to check if a context switch is required
    BaseType_t xHigherPriorityWoken = pdFALSE;

    // Clear interrupt status
    LPC_GPIO_PIN_INT->IST = (1 << 3);

    p.pin = 3;
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
/* end runtime statictics collection */

/* Sets up system hardware */
static void prvSetupHardware(void)
{
	SystemCoreClockUpdate();
	Board_Init();
	setupPin();

	/* Initial LED0 state is off */
	Board_LED_Set(0, false);
}
/*
void commandHandle(char *text){
	if(strncmp(text, "filter ", 7) == 0){
		long raw = strtol(text+7, NULL, 10);
		if(raw > 0){
			filter = raw;
			char buffer[30];
			snprintf(buffer, 30, "Filter time: %ld\r\n", filter);
			USB_send((uint8_t *) buffer, strlen(buffer));
		}
	}
}
*/
static void send_task(void *pvParameters) {
	//USB_send((uint8_t *) "Start waiting: \r\n", 20);
	Board_UARTPutSTR("Start waiting: \r\n");
	pinTime p;
	pinTime lastP;
	lastP.pin = 0;
	lastP.time = 0;
	while(1){
		xQueueReceive(queue, &p, portMAX_DELAY);
		long delayTime = (p.time - lastP.time)*1000/configTICK_RATE_HZ;
		if(delayTime > filter){
			char buffer[24];
			//int len;
			if (delayTime > 1000) {
				//len = snprintf(buffer, 24, "%.2fs Button %d\r\n", (float) delayTime/1000, p.pin);
				snprintf(buffer, 24, "%.2fs Button %d\r\n", (float) delayTime/1000, p.pin);
			} else {
				//len = snprintf(buffer, 24, "%ldms Button %d\r\n", delayTime, p.pin);
				snprintf(buffer, 24, "%ldms Button %d\r\n", delayTime, p.pin);
			}
			//USB_send((uint8_t *) buffer, len);
			Board_UARTPutSTR(buffer);
			lastP = p;
		}
	}
}

/*
static void receive_task(void *pvParameters) {
	while(1){
		char buffer[RCV_BUFFERS+1] = "";
		int index = 0;
		while(index < RCV_BUFFERS+1){
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
*/
void receive_task(void *pvParameter){
	int wordNumber = 0;
	char command[60];
	while(1){
		int c = Board_UARTGetChar();
		if(c != EOF){
			Board_UARTPutChar(c);
			command[wordNumber] = c;
			wordNumber++;
			if((c == '\r') || (wordNumber >= 60)){
				commandHandle(command);
				wordNumber = 0;
				command[0] = '\0';
			}
		}
	}
}

void commandHandle(char *text){
	if(strncmp(text, "filter ", 7) == 0){
		long raw = strtol(text+7, NULL, 10);
		if(raw > 0){
			filter = raw;
			char buffer[30];
			snprintf(buffer, 30, "\r\nFilter time: %ld\r\n", filter);
			Board_UARTPutSTR(buffer);
		}
	}
}

void setupPin(){
	Chip_IOCON_PinMuxSet(LPC_IOCON, 0, 17, IOCON_DIGMODE_EN | IOCON_MODE_INACT | IOCON_MODE_PULLUP);
	Chip_IOCON_PinMuxSet(LPC_IOCON, 1, 11, IOCON_DIGMODE_EN | IOCON_MODE_INACT | IOCON_MODE_PULLUP);
	Chip_IOCON_PinMuxSet(LPC_IOCON, 1, 9, IOCON_DIGMODE_EN | IOCON_MODE_INACT | IOCON_MODE_PULLUP);

	Chip_GPIO_SetPinDIRInput(LPC_GPIO, 0, 17);
	Chip_GPIO_SetPinDIRInput(LPC_GPIO, 1, 11);
	Chip_GPIO_SetPinDIRInput(LPC_GPIO, 1, 9);

	 /* Enable PININT clock */
	Chip_Clock_EnablePeriphClock(SYSCTL_CLOCK_PININT);

	/* Reset the PININT block */
	Chip_SYSCTL_PeriphReset(RESET_PININT);

	/* Configure interrupt channel for the GPIO pin in INMUX block */
	Chip_INMUX_PinIntSel(1, 0, 17);
	Chip_INMUX_PinIntSel(2, 1, 11);
	Chip_INMUX_PinIntSel(3, 1, 9);

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

	Chip_PININT_ClearIntStatus(LPC_GPIO_PIN_INT, PININTCH3);
	Chip_PININT_SetPinModeEdge(LPC_GPIO_PIN_INT, PININTCH3);
	Chip_PININT_EnableIntLow(LPC_GPIO_PIN_INT, PININTCH3);

	/* Enable interrupt in the NVIC */
	NVIC_ClearPendingIRQ(PIN_INT1_IRQn);
	NVIC_ClearPendingIRQ(PIN_INT2_IRQn);
	NVIC_ClearPendingIRQ(PIN_INT3_IRQn);
	NVIC_EnableIRQ(PIN_INT1_IRQn);
	NVIC_EnableIRQ(PIN_INT2_IRQn);
	NVIC_EnableIRQ(PIN_INT3_IRQn);
}

int main(void) {
	queue = xQueueCreate(2, sizeof(pinTime));
	prvSetupHardware();

	xTaskCreate(send_task, "Tx",
				configMINIMAL_STACK_SIZE*4, NULL, (tskIDLE_PRIORITY + 1UL),
				(TaskHandle_t *) NULL);

	xTaskCreate(receive_task, "Rx",
				configMINIMAL_STACK_SIZE*3, NULL, (tskIDLE_PRIORITY + 1UL),
				(TaskHandle_t *) NULL);

	/*xTaskCreate(cdc_task, "CDC",
				configMINIMAL_STACK_SIZE*2, NULL, (tskIDLE_PRIORITY + 1UL),
				(TaskHandle_t *) NULL);*/


	/* Start the scheduler */
	vTaskStartScheduler();

	/* Should never arrive here */
	return 1;
}
