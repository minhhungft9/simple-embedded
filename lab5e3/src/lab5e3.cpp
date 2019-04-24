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

#include <mutex>
#include "Fmutex.h"
#include "user_vcom.h"
#include "semphr.h"
#include <stdio.h>
#include <string.h>

SemaphoreHandle_t counting;

// TODO: insert other definitions and declarations here


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

	/* Initial LED0 state is off */
	Board_LED_Set(0, false);

}

/* send data and toggle thread */
static void send_task(void *pvParameters) {
	char *answer[6]{
		"All you need it love. And your laptop.\r\n",
		"Buy it in every color.\r\n",
		"It's the best in the world.\r\n",
		"It seemed like the right thing to do.\r\n",
		"I'm gone.\r\n",
		"Yours truly.\r\n"
	};
	while(1){
		char thinking[] = "[Oracle] I find your lack of faith disturbing\r\n";
		char buffer[80];
		if(xSemaphoreTake(counting, portMAX_DELAY) == pdPASS){
			int8_t len = snprintf(buffer, 80, "[Oracle]  %s", answer[(xTaskGetTickCount() % 6)]);
			USB_send((uint8_t *) thinking, strlen(thinking));
			vTaskDelay(configTICK_RATE_HZ * 3);
			USB_send((uint8_t *) buffer, len);
			vTaskDelay(configTICK_RATE_HZ * 2);
		}
	}
}


/* LED1 toggle thread */
static void receive_task(void *pvParameters) {
	bool LedState = false;
	bool question = false;
	char buffer[80];
	strcpy(buffer, "\r[You]  ");

	while (1) {
		char str[20];
		uint32_t len = USB_receive((uint8_t *)str, 19);
		str[len] = 0;
		for (uint8_t i = 0; i < len; i++) {
			if (str[i] == '?') {
				question = true;
			}
		}
		USB_send((uint8_t *)str, 1);
		if(str[0] != '\r' && str[0] != '\n' && str[0] != EOF){
			strcat(buffer, str);
		}else if(str[0] == '\r' || str[0] == '\n'){
			if(question == true){
				xSemaphoreGive(counting);
			}
			strcat(buffer, "\r\n");
			USB_send((uint8_t *)buffer, strlen(buffer));
			question = false;
			str[0] = '\0';
			buffer[8] = '\0';
		}
		Board_LED_Set(1, LedState);
		LedState = (bool) !LedState;
	}
}


int main(void) {
	counting = xSemaphoreCreateCounting(10, 0);
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
