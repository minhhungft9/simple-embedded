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
#include "ITM_write.h"
#endif
#endif

#include <cr_section_macros.h>
#include <cstdio>
#include "DigitalIoPin.h"

void turn(int l1, int l2, int l3){
	if(l1 == 1){
		Board_LED_Toggle(0);
	}else{
		Board_LED_Set(0, false);
	}

	if(l2 == 1){
		Board_LED_Toggle(1);
	}else{
		Board_LED_Set(1, false);
	}

	if(l3 == 1){
		Board_LED_Toggle(2);
	}else{
		Board_LED_Set(2, false);
	}
}

void menu(char *buffer, int l1, int l2, int l3, int led){
	snprintf(buffer, 50,"Select led:\nRed\t%s%s\nGreen\t%s%s\nBlue\t%s%s\n\n", (l1 == 0 ? "OFF" : "ON"), (led == 1 ? " <--" : ""), (l2 == 0 ? "OFF" : "ON"), (led == 2 ? " <--" : ""), (l3 == 0 ? "OFF" : "ON"), (led == 3 ? " <--" : ""));
	ITM_write(buffer);
}

int main(void) {

#if defined (__USE_LPCOPEN)
    SystemCoreClockUpdate();
#if !defined(NO_BOARD_LIB)
    Board_Init();
    ITM_init();
    Board_LED_Set(0, true);
#endif
#endif

    DigitalIoPin sw1(0,17);
    DigitalIoPin sw2(1,11);
    DigitalIoPin sw3(1,9);

    int n = 0;
    int led = 1;
    int l1 = 0;
    int l2 = 0;
    int l3 = 0;
    char buffer[50];

    menu(buffer, l1, l2, l3, led);
    ITM_write(buffer);
    while(true){
    	if(sw1.read()&&n==0){
    	 	n = 1;
    	 	if(led == 1){
    	 		led = 3;
    	 	}else{
    	 		led--;
    	 	}
    	}else if(sw2.read()&&n==0){
    	    n = 2;
    	    if(led == 1){
    	    	l1 = (l1 == 0)? 1 : 0;
    	    }else if(led == 2){
    	    	l2 = (l2 == 0)? 1 : 0;
    	    }else if(led == 3){
    	        l3 = (l3 == 0)? 1 : 0;
    	    }
    	}else if(sw3.read()&&n==0){
    	    n = 3;
    	    if (led == 3){
    	    	led = 1;
    	    }else{
    	    	led++;
    	    }
    	}else if(!sw1.read()&&!sw2.read()&&!sw3.read()){
    	    if(n == 1){
    	    	menu(buffer, l1, l2, l3, led);
    	    }else if(n == 2){
    	    	menu(buffer, l1, l2, l3, led);
    	    	turn(l1,l2,l3);
    	    }else if(n == 3){
    	    	menu(buffer, l1, l2, l3, led);
    	    }
    	    n = 0;
    	    buffer[0] = '\0';
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
