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
#include <cstdio>
#include <cstring>
#include <ctype.h>
#include <stdlib.h>
#include "queue.h"

void setupPin();
QueueHandle_t queue;

static void prvSetupHardware(void)
{
	SystemCoreClockUpdate();
	Board_Init();
	setupPin();

	/* Initial LED0 state is off */
	Board_LED_Set(0, false);
}

void task1(void *pvParameter){
	uint8_t n;
	int index = 0;
	int button;
	while(1){
		xQueueReceive(queue, &n, portMAX_DELAY);
		if(index == 0){
			button = n;
			index++;
		}else{
			if(button == n){
				index++;
			}else if(button != n){
                char buffer[40];
                snprintf(buffer, 40, "Button %d pressed %d times.\r\n", button, index);
                Board_UARTPutSTR(buffer);
                index = 0;
                button = n;
			}
		}
	}
}

/*****************************************************************************
 * Public functions
 ****************************************************************************/

/* the following is required if runtime statistics are to be collected */
extern "C" {

void PIN_INT1_IRQHandler(void) {
    // This used to check if a context switch is required
    BaseType_t xHigherPriorityWoken = pdFALSE;

    // Clear interrupt status
    LPC_GPIO_PIN_INT->IST = (1 << 1);

    uint8_t n = 1;
    xQueueSendToBackFromISR(queue, &n, &xHigherPriorityWoken);

    // End the ISR and (possibly) do a context switch
    portEND_SWITCHING_ISR(xHigherPriorityWoken);
}

void PIN_INT2_IRQHandler(void) {
    // This used to check if a context switch is required
    BaseType_t xHigherPriorityWoken = pdFALSE;

    // Clear interrupt status
    LPC_GPIO_PIN_INT->IST = (1 << 2);

    uint8_t n = 2;
    xQueueSendToBackFromISR(queue, &n, &xHigherPriorityWoken);

    // End the ISR and (possibly) do a context switch
    portEND_SWITCHING_ISR(xHigherPriorityWoken);
}

void PIN_INT3_IRQHandler(void) {
    // This used to check if a context switch is required
    BaseType_t xHigherPriorityWoken = pdFALSE;

    // Clear interrupt status
    LPC_GPIO_PIN_INT->IST = (1 << 3);

    uint8_t n = 3;
    xQueueSendToBackFromISR(queue, &n, &xHigherPriorityWoken);

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

/**
 * @brief	main routine for FreeRTOS blinky example
 * @return	Nothing, function should not exit
 */
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
	queue = xQueueCreate(10, sizeof(uint8_t));
	prvSetupHardware();
	xTaskCreate(task1, "button read", configMINIMAL_STACK_SIZE + 256, NULL, (tskIDLE_PRIORITY + 1UL), (TaskHandle_t *) NULL);
	vTaskStartScheduler();

    return 0 ;
}
