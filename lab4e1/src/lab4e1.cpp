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
	while(1){
		if(sw1.read()){
			Board_LED_Set(0, true);
		}else{
			Board_LED_Set(0, false);
		}

		if(sw2.read()){
			Board_LED_Set(1, true);
		}else{
			Board_LED_Set(1, false);
		}
	}
}

void task2(void *pvParameter){
	DigitalIoPin b1(0, 17, DigitalIoPin::pullup, true);
	DigitalIoPin b3(1, 9, DigitalIoPin::pullup, true);
	DigitalIoPin sw1(0, 27, DigitalIoPin::pullup, true);
	DigitalIoPin sw2(0, 28, DigitalIoPin::pullup, true);

	DigitalIoPin direct(1, 0, DigitalIoPin::output, true);
	DigitalIoPin step(0, 24, DigitalIoPin::output, true);
	while(1){
		 if((b1.read() && b3.read()) || sw1.read() || sw2.read()){
			 continue;
		 }else if(b1.read()){
			 direct.write(false);
			 step.write(true);
			 vTaskDelay(1);
			 step.write(false);
			 vTaskDelay(1);
		 }else if(b3.read()){
			 direct.write(true);
			 step.write(true);
			 vTaskDelay(1);
			 step.write(false);
			 vTaskDelay(1);
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

	prvSetupHardware();
	xTaskCreate(task1, "Led task", configMINIMAL_STACK_SIZE, NULL, (tskIDLE_PRIORITY + 1UL), (TaskHandle_t *) NULL);
	xTaskCreate(task2, "Move step", configMINIMAL_STACK_SIZE, NULL, (tskIDLE_PRIORITY + 1UL), (TaskHandle_t *) NULL);
	vTaskStartScheduler();

    return 0 ;
}
