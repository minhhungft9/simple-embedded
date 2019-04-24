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

#define PWM_FREQ 1000
#define PWM_CYCLE 5

#include <cr_section_macros.h>
#include "FreeRTOS.h"
#include "task.h"
#include "DigitalIoPin.h"
#include "ITM_write.h"
#include <stdio.h>
#include <string.h>
void SCT_Init();

static void prvSetupHardware(void)
{
	SystemCoreClockUpdate();
	Board_Init();
	Chip_SCT_Init(LPC_SCT0);
	SCT_Init();
	ITM_init();
	Chip_SWM_Init();
	Chip_SWM_MovablePortPinAssign(SWM_SCT0_OUT0_O, 0, 3);
	Board_LED_Set(0, false);
	Board_LED_Set(1, false);
}

void task1(void *pvParameter){
	DigitalIoPin sw1(0, 17, DigitalIoPin::pullup, true);
	DigitalIoPin sw2(1, 11, DigitalIoPin::pullup, true);
	DigitalIoPin sw3(1, 9, DigitalIoPin::pullup, true);
	int dutyCycle = PWM_CYCLE;
	char debug[60];
	bool write = true;

	while (1) {
		if (sw1.read() && !sw2.read()) {
			dutyCycle += 5;
			write = true;
		} else if (sw3.read() && !sw2.read()) {
			dutyCycle -= 5;
			write = true;
		} else if (sw1.read() && sw2.read()) {
			dutyCycle += 20;
			write = true;
		} else if (sw3.read() && sw2.read()) {
			dutyCycle -= 20;
			write = true;
		}else if((sw1.read() && sw3.read()) || sw2.read()){
			continue;
		}

		if (dutyCycle > 100) {
			dutyCycle = 100;
		} else if (dutyCycle < 5) {
			dutyCycle = 5;
		}

		LPC_SCT0->MATCHREL[1].L = dutyCycle*10;
		if(write == true){
			sprintf(debug, "Duty cycle: %d\n", dutyCycle);
			ITM_write(debug);
			write = false;
		}
		vTaskDelay(200);
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
void SCT_Init()
{
	LPC_SCT0->CONFIG |= (1 << 17); // two 16-bit timers, auto limit
	LPC_SCT0->CTRL_L |= (72-1) << 5; // set prescaler, SCTimer/PWM clock = 1 MHz
	LPC_SCT0->MATCHREL[0].L = PWM_FREQ-1; // match 0 @ 10/1MHz = 10 usec (100 kHz PWM freq)
	LPC_SCT0->MATCHREL[1].L = PWM_CYCLE*10; // match 1 used for duty cycle (in 10 steps)
	LPC_SCT0->EVENT[0].STATE = 0xFFFFFFFF; // event 0 happens in all states
	LPC_SCT0->EVENT[0].CTRL = (1 << 12); // match 0 condition only
	LPC_SCT0->EVENT[1].STATE = 0xFFFFFFFF; // event 1 happens in all states
	LPC_SCT0->EVENT[1].CTRL = (1 << 0) | (1 << 12); // match 1 condition only
	LPC_SCT0->OUT[0].SET = (1 << 0); // event 0 will set SCTx_OUT0
	LPC_SCT0->OUT[0].CLR = (1 << 1); // event 1 will clear SCTx_OUT0
	LPC_SCT0->CTRL_L &= ~(1 << 2); // unhalt it by clearing bit 2 of CTRL reg
}

int main(void) {

	prvSetupHardware();
	xTaskCreate(task1, "led + button", configMINIMAL_STACK_SIZE + 256, NULL, (tskIDLE_PRIORITY + 1UL), (TaskHandle_t *) NULL);
	vTaskStartScheduler();

    return 0 ;
}
