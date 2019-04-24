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
#include "Syslock.h"

Syslock mutex = Syslock();
SemaphoreHandle_t counting;

static void prvSetupHardware(void)
{
	SystemCoreClockUpdate();
	Board_Init();

	/* Initial LED0 state is off */
	Board_LED_Set(0, false);
}

void task1(void *pvParameter){
	int QmaxLength = 60;
	int index = 0;
	bool question = false;
	while(1){
		int c = mutex.read();
		if((index < QmaxLength) && (c != '\n') && (c != '\r') && (c != EOF)){
			if (index == 0){
				mutex.writeString("      ");
			}
			if(c == '?'){
				question = true;
			}
			mutex.write(&c);
			index++;
		}else if((index >= QmaxLength) || (c == '\n') || (c == '\r')){
			if (index == 0) {
				mutex.writeString("\r\n");
				continue;
			}
			mutex.writeString("\r");
			mutex.writeString("[You] ");
			mutex.writeString("\r\n");
			index = 0;
			if(question){
				xSemaphoreGive(counting);
			}
			question = false;
		}
	}
}

void task2(void *pvParameter){
	char *answer[6]{
			"All you need it love. And your laptop.\r\n",
			"Buy it in every color.\r\n",
			"It's the best in the world.\r\n",
			"It seemed like the right thing to do.\r\n",
			"I'm gone.\r\n",
			"Yours truly.\r\n"
	};
	while(1){
		if(xSemaphoreTake(counting, portMAX_DELAY) == pdTRUE){
			mutex.writeString("\r\n[Oracle] Hmmm...\r\n");
			vTaskDelay(configTICK_RATE_HZ * 3);
			mutex.writeString("\r\n[Oracle] ");
			mutex.writeString(answer[(xTaskGetTickCount() % 6)]);
			vTaskDelay(configTICK_RATE_HZ * 2);
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
	counting = xSemaphoreCreateCounting(10, 0);
	prvSetupHardware();
	xTaskCreate(task1, "Task 1", 128, NULL, (tskIDLE_PRIORITY + 1UL), (TaskHandle_t *) NULL);
	xTaskCreate(task2, "Task 2", 128, NULL, (tskIDLE_PRIORITY + 1UL), (TaskHandle_t *) NULL);
	vTaskStartScheduler();
    return 0 ;
}
