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
#include "Syslock.h"

// TODO: insert other include files here

// TODO: insert other definitions and declarations here

DigitalIoPin sw1(0, 17, DigitalIoPin::pullup);
DigitalIoPin sw2(1, 11, DigitalIoPin::pullup);
DigitalIoPin sw3(1, 9, DigitalIoPin::pullup);
Syslock mutex = Syslock();

static void prvSetupHardware(void)
{
	SystemCoreClockUpdate();
	Board_Init();

	/* Initial LED0 state is off */
	Board_LED_Set(0, false);
}

static void sw1Task(void *pvParameter){
	while(1){
		if(sw1.read()==false){
			mutex.write("SW1 pressed \r\n");
		}
	}
}

static void sw2Task(void *pvParameter){
	while(1){
		if(sw2.read()==false){
			mutex.write("SW2 pressed \r\n");
		}
	}
}

static void sw3Task(void *pvParameter){
	while(1){
		if(sw3.read()==false){
			mutex.write("SW3 pressed \r\n");
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

    xTaskCreate(sw1Task, "SW1 task", configMINIMAL_STACK_SIZE, NULL, (tskIDLE_PRIORITY + 1UL), (TaskHandle_t *) NULL);
    xTaskCreate(sw2Task, "SW2 task", configMINIMAL_STACK_SIZE, NULL, (tskIDLE_PRIORITY + 1UL), (TaskHandle_t *) NULL);
    xTaskCreate(sw3Task, "SW3 task", configMINIMAL_STACK_SIZE, NULL, (tskIDLE_PRIORITY + 1UL), (TaskHandle_t *) NULL);

    vTaskStartScheduler();
    return 0 ;
}
