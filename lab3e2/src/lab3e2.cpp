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
#include "queue.h"
#include <stdio.h>      /* printf, scanf, puts, NULL */
#include <stdlib.h>     /* srand, rand */
#include <time.h>       /* time */

QueueHandle_t queue = xQueueCreate(20, sizeof(int));
static void prvSetupHardware(void)
{
	SystemCoreClockUpdate();
	Board_Init();

	/* Initial LED0 state is off */
	Board_LED_Set(0, false);
}

void task1(void *pvParameter){
	while(1){
		int number = rand() % 500 + 1;
		xQueueSendToBack(queue, &number, portMAX_DELAY);
		vTaskDelay(500);
	}
}

void task2(void *pvParameter){
	DigitalIoPin button(0, 17, DigitalIoPin::pullup, true);
	int buttonPressed = 112;
	bool pressed = false;
	while(1){
		if(button.read() && pressed == false){
			vTaskDelay(50);
			if(button.read()){
				xQueueSendToBack(queue, &buttonPressed, portMAX_DELAY);
				pressed = true;
			}else if(button.read() && pressed == true){

			}
		}else if(button.read() == false){
			pressed = false;
		}
	}
}

void task3(void *pvParameter){
	while(1){
		int value;
		while(1){
			xQueueReceive(queue, &value, portMAX_DELAY);

			if(value == 112){
				DEBUGOUT("\r\n%d Help me!\r\n", value);
				vTaskDelay(300);
			}else{
				DEBUGOUT("\r\n%d\r\n", value);
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

int main(void) {

	srand (time(NULL));
	prvSetupHardware();
	xTaskCreate(task1, "Task 1", configMINIMAL_STACK_SIZE, NULL, (tskIDLE_PRIORITY + 1UL), (TaskHandle_t *) NULL);
	xTaskCreate(task2, "Task 2", configMINIMAL_STACK_SIZE, NULL, (tskIDLE_PRIORITY + 1UL), (TaskHandle_t *) NULL);
	xTaskCreate(task3, "Task 3", configMINIMAL_STACK_SIZE + 128, NULL, (tskIDLE_PRIORITY + 1UL), (TaskHandle_t *) NULL);
	vTaskStartScheduler();

    return 0 ;
}
