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
#include <cstdio>
#include <cstring>
#include <ctype.h>
#include <stdlib.h>
#include "queue.h"

QueueHandle_t queue;

static void prvSetupHardware(void)
{
	SystemCoreClockUpdate();
	Board_Init();

	/* Initial LED0 state is off */
	Board_LED_Set(0, false);
}

struct debugEvent {
	char *format;
	uint32_t data[3];
};

void debug(char *format, uint32_t d1, uint32_t d2, uint32_t d3){
	debugEvent e;
	e.format = format;
	e.data[0] = d1;
	e.data[1] = d2;
	e.data[2] = d3;
	xQueueSendToBack(queue, &e, portMAX_DELAY);
}

void task1(void *pvParameter){
	debugEvent e;
	char buffer[60];
	while (1) {
		xQueueReceive(queue, &e, portMAX_DELAY);
		if(e.data[0] == 2){
			snprintf(buffer, 64, e.format, e.data[1], e.data[2]);
			ITM_write("Received from serial read: ");
			ITM_write(buffer);
		}else if(e.data[0] == 3){
			snprintf(buffer, 64, e.format, e.data[1], e.data[2]);
			ITM_write("Received from button read: ");
			ITM_write(buffer);
		}
		ITM_write("\r\n");
	}
}

void task2(void *pvParameter){
	int index = 0;
	int t = 0;
	int taskNumber = 2;
	char mess[] = "Word length: %d, at tick %d";

	while(1){
		int c = Board_UARTGetChar();
		if(c != EOF){
			if(!isspace(c)){
				Board_UARTPutChar(c);
				index++;
			}else if(isspace(c)){
				Board_UARTPutChar(c);
				if(index > 0){
					t = xTaskGetTickCount();
					debug(mess, taskNumber, index, t);
					index = 0;
				}
			}
		}else{
			vTaskDelay(3);
		}
	}
}

void task3(void *pvParameter){
	DigitalIoPin button(0, 17, DigitalIoPin::pullup, true);
	int t = 0;
	int taskNumber = 3;
	bool pressed = false;
	char mess[] = "SW1 pressed for %d ticks at %d";

	while(1){
		if(button.read() && pressed == false){
			vTaskDelay(1);
			if(button.read()){
				t = xTaskGetTickCount();
				pressed = true;
			}else if(button.read() && pressed == true){

			}
		}else if(button.read() == false && pressed == true){
			int pressLength = xTaskGetTickCount() - t;
			debug(mess, taskNumber, pressLength, t);
			pressed = false;
		}
		vTaskDelay(3);
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
	ITM_write("Hello World!\r\n");
	queue = xQueueCreate(3, sizeof(debugEvent));

	prvSetupHardware();
	xTaskCreate(task1, "debug task", configMINIMAL_STACK_SIZE + 256, NULL, (tskIDLE_PRIORITY + 1UL), (TaskHandle_t *) NULL);
	xTaskCreate(task2, "serial read", configMINIMAL_STACK_SIZE + 256, NULL, (tskIDLE_PRIORITY + 2UL), (TaskHandle_t *) NULL);
	xTaskCreate(task3, "button read", configMINIMAL_STACK_SIZE + 256, NULL, (tskIDLE_PRIORITY + 2UL), (TaskHandle_t *) NULL);
	vTaskStartScheduler();

    return 0 ;
}
