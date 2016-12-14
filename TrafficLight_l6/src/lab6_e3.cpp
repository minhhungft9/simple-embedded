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
#include "DigitalIoPin.h"
#include "Event.h"
#include "StateMachine.h"
#define TICKRATE_HZ1 (1000)

// TODO: insert other include files here

// TODO: insert other definitions and declarations here
volatile int counter;
extern "C" {
void SysTick_Handler(void)
{
	counter++;
}
}

bool enter_critical(void)
{
	uint32_t pm = __get_PRIMASK();
	__disable_irq();
	return (pm & 1) == 0;
}

// restore interrupt enable state
void leave_critical(bool enable)
{
	if(enable) __enable_irq();
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
    Board_LED_Set(0, false);
    Board_LED_Set(1, false);
    Board_LED_Set(2, false);
#endif
#endif
    Chip_Clock_SetSysTickClockDiv(1);
    uint32_t sysTickRate = Chip_Clock_GetSysTickClockRate();
    SysTick_Config(sysTickRate / TICKRATE_HZ1);
    // TODO: insert code here
    StateMachine traffic;

    DigitalIoPin car(0,0,true,true,true);				//car sensor
    DigitalIoPin pedestrian(1,3,true,true,true);		//pedestrian button

    DigitalIoPin greenlightcar(0,12,false,true,false);	//car green light
    DigitalIoPin yellowlightcar(0,28,false,true,false);	//car yellow light
    DigitalIoPin redlightcar(0,27,false,true,false);	//car red light
    DigitalIoPin greenlightped(1,0,false,true,false);	//pedestrian green light
    DigitalIoPin redlightped(0,24,false,true,false);	//pedestrian red light
    DigitalIoPin pedbuttonlight(1,1,false,false,false);	//pedestrian button light

    traffic.conft(car,pedestrian,redlightcar,yellowlightcar,greenlightcar,redlightped,greenlightped,pedbuttonlight);

    Event carr(Event::Ecar);
    Event nocar(Event::Enocar);
    Event ped(Event::Eped);
    Event tick(Event::Etick);

    while(1) {
    	if(car.read() == true){
    		traffic.HandleState(carr);
    	}else if(car.read() == false){
    		traffic.HandleState(nocar);
    	}

    	if(pedestrian.read() == true){
    		traffic.HandleState(ped);
    	}

    	if(counter == 100){
    		bool irq = enter_critical();
    		counter = 0;
    		traffic.HandleState(tick);
    		leave_critical(irq);
    	}
    }
    return 0 ;
}
