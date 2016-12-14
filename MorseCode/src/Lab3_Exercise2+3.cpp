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
#include "MorseSender.h"
#include "DigitalIoPin.h"


#define TICKRATE_HZ1 (1000)

static volatile int counter;

#ifdef __cplusplus
extern "C" {
#endif
/*
 * @brief Handle interrupt from SysTick timer
 * @return Nothing
*/
void SysTick_Handler(void)
{
			if(counter > 0) counter--;
}
#ifdef __cplusplus
}
#endif

void Sleep(int ms)
{
			counter = ms;
			while(counter > 0) {
					__WFI();
			}
}

int main(void) {

#if defined (__USE_LPCOPEN)
    SystemCoreClockUpdate();
#if !defined(NO_BOARD_LIB)
    Board_Init();
    Board_LED_Set(0, true);
#endif
#endif

    Chip_Clock_SetSysTickClockDiv(1);
    uint32_t sysTickRate = Chip_Clock_GetSysTickClockRate();
    SysTick_Config(sysTickRate / TICKRATE_HZ1);

    DigitalIoPin led(25, 0, false, true, false);
    DigitalIoPin decoder(8, 0, false, true, false);
    MorseSender morse(&led, &decoder);

    int wordNumber = -1;
    int c;
    char text[86];

    while(1) {
    	if(wordNumber < 0){
    		Board_UARTPutSTR("\r\nEnter command:\r\n");
    		wordNumber++;
    	}
		c = Board_UARTGetChar();
		if((c != EOF)){
			Board_UARTPutChar(c);
			text[wordNumber] = c;
			wordNumber++;
			if((c == '\r')|(wordNumber >= 86)){
				if(morse.command(text)){
					Board_UARTPutChar('\n');
					morse.sendCode(text);
				}
				wordNumber = -1;
			}
		}
    }
    return 0 ;
}

