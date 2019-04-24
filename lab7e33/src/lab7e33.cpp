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
#define PWM_FREQ 1000
#define PWM_CYCLE 255
// TODO: insert other definitions and declarations here
void SCT_Init();

/* the following is required if runtime statistics are to be collected */
extern "C" {

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
	Chip_SCT_Init(LPC_SCT0);
	Chip_SCT_Init(LPC_SCT1);
	Chip_SCT_Init(LPC_SCT2);
	SCT_Init();
	Chip_SWM_Init();
	Chip_SWM_MovablePortPinAssign(SWM_SCT0_OUT0_O, 0, 25);
	Chip_SWM_MovablePortPinAssign(SWM_SCT1_OUT0_O, 0, 3);
	Chip_SWM_MovablePortPinAssign(SWM_SCT2_OUT0_O, 1, 1);
	/* Initial LED0 state is off */
	Board_LED_Set(0, false);
	Board_LED_Set(1, false);
	Board_LED_Set(2, false);
}

struct color{
	int rawNumber;
	int red;
	int green;
	int blue;
};

void commandHandle(char *text){
	color c;
	if(strncmp(text, "rgb ", 4) == 0){
		int raw = strtol(text+4, NULL, 16);
		int red = raw/(256*256);
		int green = (raw - red*(256*256))/256;
		int blue = raw - red*(256*256) - green*256;
		c.rawNumber = raw;
		c.red = red;
		c.green = green;
		c.blue = blue;
		xQueueSendToBack(queue, &c, portMAX_DELAY);
	}
}

/* send data and toggle thread */
static void send_task(void *pvParameters) {
	color c;
	while(1){
		char buffer[80];
		if(xQueueReceive(queue, &c, portMAX_DELAY) == pdPASS){
			int8_t len = snprintf(buffer, 80, "Raw number: %d, red: %d, green: %d, blue: %d\r\n", c.rawNumber, c.red, c.green, c.blue);
			USB_send((uint8_t *) buffer, len);
			LPC_SCT0->MATCHREL[1].L = (PWM_CYCLE-c.red)/PWM_CYCLE*PWM_FREQ;
			LPC_SCT1->MATCHREL[1].L = (PWM_CYCLE-c.green)/PWM_CYCLE*PWM_FREQ;
			LPC_SCT2->MATCHREL[1].L = (PWM_CYCLE-c.blue)/PWM_CYCLE*PWM_FREQ;
			vTaskDelay(300);
		}
	}
}

/* LED1 toggle thread */
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

void SCT_Init()
{
	LPC_SCT0->CONFIG |= (1 << 17); // two 16-bit timers, auto limit
	LPC_SCT0->CTRL_L |= (72-1) << 5; // set prescaler, SCTimer/PWM clock = 1 MHz
	LPC_SCT0->MATCHREL[0].L = (1000000/PWM_FREQ)-1; // match 0 @ 10/1MHz = 10 usec (100 kHz PWM freq)
	LPC_SCT0->MATCHREL[1].L = PWM_CYCLE*PWM_FREQ/PWM_CYCLE; // match 1 used for duty cycle (in 10 steps)
	LPC_SCT0->EVENT[0].STATE = 0xFFFFFFFF; // event 0 happens in all states
	LPC_SCT0->EVENT[0].CTRL = (1 << 12); // match 0 condition only
	LPC_SCT0->EVENT[1].STATE = 0xFFFFFFFF; // event 1 happens in all states
	LPC_SCT0->EVENT[1].CTRL = (1 << 0) | (1 << 12); // match 1 condition only
	LPC_SCT0->OUT[0].SET = (1 << 0); // event 0 will set SCTx_OUT0
	LPC_SCT0->OUT[0].CLR = (1 << 1); // event 1 will clear SCTx_OUT0
	LPC_SCT0->CTRL_L &= ~(1 << 2); // unhalt it by clearing bit 2 of CTRL reg

	LPC_SCT1->CONFIG |= (1 << 17); // two 16-bit timers, auto limit
	LPC_SCT1->CTRL_L |= (72-1) << 5; // set prescaler, SCTimer/PWM clock = 1 MHz
	LPC_SCT1->MATCHREL[0].L = (1000000/PWM_FREQ)-1; // match 0 @ 10/1MHz = 10 usec (100 kHz PWM freq)
	LPC_SCT1->MATCHREL[1].L = PWM_CYCLE*PWM_FREQ/PWM_CYCLE; // match 1 used for duty cycle (in 10 steps)
	LPC_SCT1->EVENT[0].STATE = 0xFFFFFFFF; // event 0 happens in all states
	LPC_SCT1->EVENT[0].CTRL = (1 << 12); // match 0 condition only
	LPC_SCT1->EVENT[1].STATE = 0xFFFFFFFF; // event 1 happens in all states
	LPC_SCT1->EVENT[1].CTRL = (1 << 0) | (1 << 12); // match 1 condition only
	LPC_SCT1->OUT[0].SET = (1 << 0); // event 0 will set SCTx_OUT0
	LPC_SCT1->OUT[0].CLR = (1 << 1); // event 1 will clear SCTx_OUT0
	LPC_SCT1->CTRL_L &= ~(1 << 2); // unhalt it by clearing bit 2 of CTRL reg

	LPC_SCT2->CONFIG |= (1 << 17); // two 16-bit timers, auto limit
	LPC_SCT2->CTRL_L |= (72-1) << 5; // set prescaler, SCTimer/PWM clock = 1 MHz
	LPC_SCT2->MATCHREL[0].L = (1000000/PWM_FREQ)-1; // match 0 @ 10/1MHz = 10 usec (100 kHz PWM freq)
	LPC_SCT2->MATCHREL[1].L = PWM_CYCLE*PWM_FREQ/PWM_CYCLE; // match 1 used for duty cycle (in 10 steps)
	LPC_SCT2->EVENT[0].STATE = 0xFFFFFFFF; // event 0 happens in all states
	LPC_SCT2->EVENT[0].CTRL = (1 << 12); // match 0 condition only
	LPC_SCT2->EVENT[1].STATE = 0xFFFFFFFF; // event 1 happens in all states
	LPC_SCT2->EVENT[1].CTRL = (1 << 0) | (1 << 12); // match 1 condition only
	LPC_SCT2->OUT[0].SET = (1 << 0); // event 0 will set SCTx_OUT0
	LPC_SCT2->OUT[0].CLR = (1 << 1); // event 1 will clear SCTx_OUT0
	LPC_SCT2->CTRL_L &= ~(1 << 2); // unhalt it by clearing bit 2 of CTRL reg
}


int main(void) {
	queue = xQueueCreate(1, sizeof(color));
	prvSetupHardware();
	ITM_init();

	/* LED1 toggle thread */
	xTaskCreate(send_task, "Tx",
				configMINIMAL_STACK_SIZE*3, NULL, (tskIDLE_PRIORITY + 1UL),
				(TaskHandle_t *) NULL);

	/* LED1 toggle thread */
	xTaskCreate(receive_task, "Rx",
				configMINIMAL_STACK_SIZE*3, NULL, (tskIDLE_PRIORITY + 1UL),
				(TaskHandle_t *) NULL);

	/* LED2 toggle thread */
	xTaskCreate(cdc_task, "CDC",
				configMINIMAL_STACK_SIZE*2, NULL, (tskIDLE_PRIORITY + 1UL),
				(TaskHandle_t *) NULL);


	/* Start the scheduler */
	vTaskStartScheduler();

	/* Should never arrive here */
	return 1;
}
