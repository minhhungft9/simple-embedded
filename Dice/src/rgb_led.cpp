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
#define MAXLEDS 7
#define TICKRATE_HZ1 (100)
static const uint8_t diceport[] = { 0, 1, 0, 0, 0, 0, 1 };
static const uint8_t dicepins[] = { 0, 3, 16, 10, 9, 29, 9 };
// TODO: insert other definitions and declarations here
volatile int counter;

#ifdef __cplusplus
extern "C" {
#endif
void SysTick_Handler(void)
	{
		if(counter > 7){
			counter = 0;
		}
		counter++;
	}
#ifdef __cplusplus
}
#endif

//Turn on all the LEDs
void allOn(){
	int idx;
	for (idx = 0; idx < MAXLEDS; idx++) {
		    Chip_GPIO_SetPinDIROutput(LPC_GPIO, diceport[idx], dicepins[idx]);
		    Chip_GPIO_SetPinState(LPC_GPIO, diceport[idx], dicepins[idx], true);
	}
}

//Turn off all the LEDs
void allOff(){
	int idx;
	for (idx = 0; idx < MAXLEDS; idx++) {
		    Chip_GPIO_SetPinDIROutput(LPC_GPIO, diceport[idx], dicepins[idx]);
		    Chip_GPIO_SetPinState(LPC_GPIO, diceport[idx], dicepins[idx], false);
	}
}

class Dice {
public:
			Dice();
			//virtual ~Dice();
			void Show(int number);
};
Dice::Dice(){
	Chip_IOCON_PinMuxSet(LPC_IOCON, 0, 8, (IOCON_MODE_PULLUP | IOCON_DIGMODE_EN | IOCON_INV_EN));
	Chip_GPIO_SetPinDIRInput(LPC_GPIO, 0, 8);
	Chip_IOCON_PinMuxSet(LPC_IOCON, 1, 6, (IOCON_MODE_PULLUP | IOCON_DIGMODE_EN | IOCON_INV_EN));
	Chip_GPIO_SetPinDIRInput(LPC_GPIO, 1, 6);
	allOff();
}
void Dice::Show(int number){
	if(number == 0){
		allOff();
	}else if(number == 1){
		Chip_GPIO_SetPinDIROutput(LPC_GPIO, 0, 10);	//PD4
		Chip_GPIO_SetPinState(LPC_GPIO, 0, 10, true);
	}else if(number == 2){
		Chip_GPIO_SetPinDIROutput(LPC_GPIO, 0, 0);	//PD7
		Chip_GPIO_SetPinState(LPC_GPIO, 0, 0, true);
		Chip_GPIO_SetPinDIROutput(LPC_GPIO, 0, 9);	//PD3
		Chip_GPIO_SetPinState(LPC_GPIO, 0, 9, true);
	}else if(number == 3){
		Chip_GPIO_SetPinDIROutput(LPC_GPIO, 0, 10);	//PD4
		Chip_GPIO_SetPinState(LPC_GPIO, 0, 10, true);
		Chip_GPIO_SetPinDIROutput(LPC_GPIO, 0, 0);	//PD7
		Chip_GPIO_SetPinState(LPC_GPIO, 0, 0, true);
		Chip_GPIO_SetPinDIROutput(LPC_GPIO, 0, 9);	//PD3
		Chip_GPIO_SetPinState(LPC_GPIO, 0, 9, true);
	}else if(number == 4){
		Chip_GPIO_SetPinDIROutput(LPC_GPIO, 0, 0);	//PD7
		Chip_GPIO_SetPinState(LPC_GPIO, 0, 0, true);
		Chip_GPIO_SetPinDIROutput(LPC_GPIO, 1, 9);	//PD1
		Chip_GPIO_SetPinState(LPC_GPIO, 1, 9, true);
		Chip_GPIO_SetPinDIROutput(LPC_GPIO, 0, 9);	//PD3
		Chip_GPIO_SetPinState(LPC_GPIO, 0, 9, true);
		Chip_GPIO_SetPinDIROutput(LPC_GPIO, 0, 16);	//PD5
		Chip_GPIO_SetPinState(LPC_GPIO, 0, 16, true);
	}else if(number == 5){
		Chip_GPIO_SetPinDIROutput(LPC_GPIO, 0, 0);	//PD7
		Chip_GPIO_SetPinState(LPC_GPIO, 0, 0, true);
		Chip_GPIO_SetPinDIROutput(LPC_GPIO, 1, 9);	//PD1
		Chip_GPIO_SetPinState(LPC_GPIO, 1, 9, true);
		Chip_GPIO_SetPinDIROutput(LPC_GPIO, 0, 9);	//PD3
		Chip_GPIO_SetPinState(LPC_GPIO, 0, 9, true);
		Chip_GPIO_SetPinDIROutput(LPC_GPIO, 0, 16);	//PD5
		Chip_GPIO_SetPinState(LPC_GPIO, 0, 16, true);
		Chip_GPIO_SetPinDIROutput(LPC_GPIO, 0, 10);	//PD4
		Chip_GPIO_SetPinState(LPC_GPIO, 0, 10, true);
	}else if(number == 6){
		Chip_GPIO_SetPinDIROutput(LPC_GPIO, 0, 0);	//PD7
		Chip_GPIO_SetPinState(LPC_GPIO, 0, 0, true);
		Chip_GPIO_SetPinDIROutput(LPC_GPIO, 1, 9);	//PD1
		Chip_GPIO_SetPinState(LPC_GPIO, 1, 9, true);
		Chip_GPIO_SetPinDIROutput(LPC_GPIO, 0, 9);	//PD3
		Chip_GPIO_SetPinState(LPC_GPIO, 0, 9, true);
		Chip_GPIO_SetPinDIROutput(LPC_GPIO, 0, 16);	//PD5
		Chip_GPIO_SetPinState(LPC_GPIO, 0, 16, true);
		Chip_GPIO_SetPinDIROutput(LPC_GPIO, 1, 3);	//PD6
		Chip_GPIO_SetPinState(LPC_GPIO, 1, 3, true);
		Chip_GPIO_SetPinDIROutput(LPC_GPIO, 0, 29);	//PD2
		Chip_GPIO_SetPinState(LPC_GPIO, 0, 29, true);
	}else{
		allOn();
	}
}

int main(void) {

#if defined (__USE_LPCOPEN)
    // Read clock settings and update SystemCoreClock variable
    SystemCoreClockUpdate();
#if !defined(NO_BOARD_LIB)
    // Set up and initialize all required blocks and
    // functions related to the board hardware
    Board_Init();
    // Set the LED to the state of "On"
    Board_LED_Set(0, true);
#endif
#endif
    Chip_Clock_SetSysTickClockDiv(1);
    uint32_t sysTickRate = Chip_Clock_GetSysTickClockRate();
    SysTick_Config(sysTickRate / TICKRATE_HZ1);

    Dice dice;
    volatile int n;
    while(true){
    	if((Chip_GPIO_GetPinState(LPC_GPIO, 0, 8))&&(Chip_GPIO_GetPinState(LPC_GPIO, 1, 6)==false)){
    		n = 0;
    		allOn();
    	}else if(Chip_GPIO_GetPinState(LPC_GPIO, 0, 8) == false){
    		allOff();
    		if(Chip_GPIO_GetPinState(LPC_GPIO, 1, 6)){
    			n = counter;
    		}else if(Chip_GPIO_GetPinState(LPC_GPIO, 1, 6)==false){
    			dice.Show(n);
    		}
    	}
    }

    // Force the counter to be placed into memory
    volatile static int i = 0 ;
    // Enter an infinite loop, just incrementing a counter
    while(1) {
        i++ ;
    }
    return 0 ;
}
