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
#include "LiquidCrystal.h"
#include <ctime>
#include <stdio.h>
#include "RealTimeClock.h"
#include "DigitalIoPin.h"

#include <cr_section_macros.h>

void delayMicroseconds(unsigned int us);

#define TICKRATE_HZ1 (1000)

static RealTimeClock *pointer;
static volatile int counter;

#ifdef __cplusplus
extern "C" {
#endif
/**
 * @brief Handle interrupt from SysTick timer
 * @return Nothing
 */
void SysTick_Handler(void)
{
	if(pointer != NULL) pointer->tick();
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
    Chip_RIT_Init(LPC_RITIMER);

    DigitalIoPin d4(8,1, false, true, false);
	DigitalIoPin d5(5,0, false, true, false);
	DigitalIoPin d6(6,0, false, true, false);
	DigitalIoPin d7(7,0, false, true, false);
	DigitalIoPin en(6,1, false, true, false);
	DigitalIoPin rs(8,0, false, true, false);

	DigitalIoPin sw1(17,0,true,true,false);
	DigitalIoPin sw2(11,1,true,true,false);
	DigitalIoPin sw3(9,1,true,true,false);

	LiquidCrystal lcd(&rs, &en, &d4, &d5, &d6, &d7);

	char buffer[32];
	char alarm[32];

	time_t t = time(0);
	struct tm * now = localtime(&t);
	now->tm_hour = 0;
	now->tm_min = 0;
	now->tm_sec = 0;

	int houralarm = 0;
	int minalarm = 0;
	bool alarmturn = false;

	static RealTimeClock clockk(TICKRATE_HZ1, now);
	pointer = &clockk;

	while(1) {
		clockk.gettime(now);

		sniprintf(buffer, 32, "%s%d:%s%d:%s%d", (now->tm_hour < 10 ? " " : ""), now->tm_hour, (now->tm_min < 10 ? "0" : ""), now->tm_min, (now->tm_sec < 10 ? "0" : ""), now->tm_sec);
		lcd.print(buffer, 0);
		snprintf(alarm, 32, "Alarm: %s%d:%s%d(%s)", (houralarm < 10 ? " ":""), houralarm, (minalarm < 10 ? "0":""), minalarm, (alarmturn == true ? "e":"d"));
		lcd.print(alarm,1);

		if(alarmturn == true){
			if(houralarm == clockk.gethour() && minalarm == clockk.getmin()){
				Board_LED_Set(0, true);
				Sleep(200);
				Board_LED_Set(0, false);
				Sleep(200);
			}
		}
		//set time
		if(sw1.read()==false){
			lcd.clear();
			int hourset = now->tm_hour;
			int minset = now->tm_min;
			int secset = 0;
			while(1){
				lcd.print("Set time", 0);
				sniprintf(buffer, 32, "%s%d:%s%d:%s%d", (hourset < 10 ? " " : ""), hourset, (minset < 10 ? "0" : ""), minset, (secset < 10 ? "0" : ""), secset);
				lcd.print(buffer, 1);
				if (!sw1.read()) {
					hourset = (hourset == 23) ? 0 : (hourset + 1);
					delayMicroseconds(500000);
					continue;
				}
				if (!sw2.read()) {
					minset = (minset == 59) ? 0 : (minset + 1);
					delayMicroseconds(500000);
					continue;
				}
				if (!sw3.read()) {
					clockk.sethour(hourset);
					clockk.setmin(minset);
					clockk.setsec(0);
					lcd.clear();
					break;
				}
			}
		}
		//set alarm
		if(sw2.read()==false){
			lcd.clear();
			while(1){
				lcd.print("Set alarm", 0);
				snprintf(alarm, 32, "%s%d:%s%d", (houralarm < 10 ? " ":""), houralarm, (minalarm < 10 ? "0":""), minalarm);
				lcd.print(alarm, 1);
				if (!sw1.read()) {
					houralarm = (houralarm == 23) ? 0 : (houralarm + 1);
					delayMicroseconds(500000);
					continue;
				}
				if (!sw2.read()) {
					minalarm = (minalarm == 59) ? 0 : (minalarm + 1);
					delayMicroseconds(500000);
					continue;
				}
				if (!sw3.read()){
					lcd.clear();
					break;
				}
			}
		}
		//turn on/off alarm
		if(sw3.read()==false){
			alarmturn = !alarmturn;
			delayMicroseconds(500000);
		}
    }

    return 0 ;
}
