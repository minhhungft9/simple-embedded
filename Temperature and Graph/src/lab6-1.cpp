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
#include <cmath>
#endif
#endif

#include <cr_section_macros.h>
#include "DigitalIoPin.h"
#include "LiquidCrystal.h"
#include "BarGraph.h"

// TODO: insert other include files here

// TODO: insert other definitions and declarations here
#define TICKRATE_HZ (1000)	/* 100 ticks per second */
static volatile uint32_t ticks;
static volatile uint32_t counter;
static volatile int diff;
static volatile unsigned int blinktime;
static volatile float frequency;
static const int green = 40;
volatile uint32_t trim1;
volatile uint32_t trim2;
volatile uint32_t trim3;
volatile uint32_t temp1;
volatile uint32_t temp2;
volatile uint32_t temp3;


extern "C" {

void SysTick_Handler(void)
{
	static int unsigned count = 0;
	//static int count2 = 0;

//	if (count2 < 100) {
//		count2++;
//	} else {
//		Chip_ADC_StartSequencer(LPC_ADC0, ADC_SEQA_IDX);
//		count2 = 0;
//	}

	if(count < blinktime){
		count++;
	}

	if(count > blinktime){
		count = 0;
	}

	if(diff < - green){
		if(count == blinktime){
			Board_LED_Toggle(2);
			count = 0;
		}
		Board_LED_Set(0, false);
		Board_LED_Set(1, false);
	}else if(diff <= green && diff >= - green){
		Board_LED_Set(0, false);
		Board_LED_Set(2, false);
		Board_LED_Toggle(1);
		count = 0;
	}else if(diff > green){
		if(count == blinktime){
			Board_LED_Toggle(0);
			count = 0;
		}
		Board_LED_Set(1, false);
		Board_LED_Set(2, false);
	}

	ticks++;

	if(counter > 0) counter--;
}
//void ADC0A_IRQHandler(void)
//{
//	uint32_t pending;
//	volatile int counter = 0;
//	/* Get pending interrupts */
//	pending = Chip_ADC_GetFlags(LPC_ADC0);
//	/* Sequence A completion interrupt */
//	if (pending & ADC_FLAGS_SEQA_INT_MASK) {
//		/* Read the ADC values here */
//	if(counter == 0){
//		trim1 = Chip_ADC_GetDataReg(LPC_ADC0, 8);
//		temp1 = Chip_ADC_GetDataReg(LPC_ADC0, 10);
//		counter++;
//	}
//	if(counter == 1){
//		trim2 = Chip_ADC_GetDataReg(LPC_ADC0, 8);
//		temp2 = Chip_ADC_GetDataReg(LPC_ADC0, 10);
//		counter++;
//	}else if(counter == 2)
//		trim3 = Chip_ADC_GetDataReg(LPC_ADC0, 8);
//		temp3 = Chip_ADC_GetDataReg(LPC_ADC0, 10);
//		counter = 0;
//	}
//	/* Clear any pending interrupts */
//	Chip_ADC_ClearFlags(LPC_ADC0, pending);
//}

}
// extern "C"


// returns the interrupt enable state before entering critical section
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

// Example:
// bool irq = enter_critical();
// Change variables that are shared with an ISR
// leave_critical(irq);



void Sleep(uint32_t time)
{
	counter = time;
	while(counter > 0) {
		__WFI();
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
    ITM_init();
    // Set the LED to the state of "On"
    Board_LED_Set(0, true);
#endif
#endif

    // TODO: insert code here
    Chip_RIT_Init(LPC_RITIMER);
	/* Setup ADC for 12-bit mode and normal power */
	Chip_ADC_Init(LPC_ADC0, 0);

	/* Setup for maximum ADC clock rate */
	Chip_ADC_SetClockRate(LPC_ADC0, ADC_MAX_SAMPLE_RATE);

	/* For ADC0, sequencer A will be used without threshold events.
	   It will be triggered manually, convert CH8 and CH10 in the sequence  */
	Chip_ADC_SetupSequencer(LPC_ADC0, ADC_SEQA_IDX, (ADC_SEQ_CTRL_CHANSEL(8) | ADC_SEQ_CTRL_CHANSEL(10) | ADC_SEQ_CTRL_MODE_EOS));

	/* For ADC0, select analog input pin for channel 0 on ADC0 */
	Chip_ADC_SetADC0Input(LPC_ADC0, 0);

	/* Use higher voltage trim for both ADC */
	Chip_ADC_SetTrim(LPC_ADC0, ADC_TRIM_VRANGE_HIGHV);

	/* Assign ADC0_8 to PIO1_0 via SWM (fixed pin) and ADC0_10 to PIO0_0 */
	Chip_SWM_EnableFixedPin(SWM_FIXED_ADC0_8);
	Chip_SWM_EnableFixedPin(SWM_FIXED_ADC0_10);

	/* Need to do a calibration after initialization and trim */
	Chip_ADC_StartCalibration(LPC_ADC0);
	while (!(Chip_ADC_IsCalibrationDone(LPC_ADC0)));

	/* Clear all pending interrupts and status flags */
	Chip_ADC_ClearFlags(LPC_ADC0, Chip_ADC_GetFlags(LPC_ADC0));

	/* Enable sequence A completion interrupts for ADC0 */
	Chip_ADC_EnableInt(LPC_ADC0, ADC_INTEN_SEQA_ENABLE);
	/* We don't enable the corresponding interrupt in NVIC so the flag is set but no interrupt is triggered */

	/* Enable related ADC NVIC interrupts */
	//NVIC_EnableIRQ(ADC0_SEQA_IRQn);

	/* Enable sequencer */
	Chip_ADC_EnableSequencer(LPC_ADC0, ADC_SEQA_IDX);

	/* Configure systick timer */
	SysTick_Config(Chip_Clock_GetSysTickClockRate() / TICKRATE_HZ);

	uint32_t a0;
	uint32_t d0;
	uint32_t a10;
	uint32_t d10;
	char str[80];
	char value1[5];
	char value2[5];

	DigitalIoPin d4(8,1, false, true, false);
	DigitalIoPin d5(5,0, false, true, false);
	DigitalIoPin d6(6,0, false, true, false);
	DigitalIoPin d7(7,0, false, true, false);
	DigitalIoPin en(6,1, false, true, false);
	DigitalIoPin rs(8,0, false, true, false);

	LiquidCrystal lcd(&rs, &en, &d4, &d5, &d6, &d7);
	BarGraph Tempgraph(lcd, 50, false, 5, 0);
	BarGraph Trimgraph(lcd, 50, false, 5, 1);

	while(1) {
		// get data from ADC channels
//		d0 = (ADC_DR_RESULT(trim1) + ADC_DR_RESULT(trim2) + ADC_DR_RESULT(trim3))/3;
//		d10 = (ADC_DR_RESULT(temp1) + ADC_DR_RESULT(temp2) + ADC_DR_RESULT(temp3))/3;

		Chip_ADC_StartSequencer(LPC_ADC0, ADC_SEQA_IDX);

		// poll sequence complete flag
		while(!(Chip_ADC_GetFlags(LPC_ADC0) & ADC_FLAGS_SEQA_INT_MASK));
		// clear the flags
		Chip_ADC_ClearFlags(LPC_ADC0, Chip_ADC_GetFlags(LPC_ADC0));

		// get data from ADC channels
		a0 = Chip_ADC_GetDataReg(LPC_ADC0, 8); // raw value
		d0 = ADC_DR_RESULT(a0); // ADC result with status bits masked to zero and shifted to start from zero
		a10 = Chip_ADC_GetDataReg(LPC_ADC0, 10);
		d10 = ADC_DR_RESULT(a10);

		bool irq = enter_critical();
		diff = d0 - d10;
		blinktime = (std::abs(diff))/4;
		frequency = 1000/blinktime;
		leave_critical(irq);

		sprintf(value2, "%s%lu", (d10>999 ? "" : (d10>99 ? " " :(d10>9 ? "  " : "   "))), d10);
		lcd.print(value2, 0);

		Tempgraph.draw((d10 - 2450)/2);

		sprintf(value1, "%s%lu", (d0>999 ? "" : (d0>99 ? " " :(d0>9 ? "  " : "   "))), d0);
		lcd.print(value1, 1);

		Trimgraph.draw(d0/82);

		sprintf(str, "Trimmer value: %lu, temperature sensor value: %lu\nDifference = %ld, led frequency: %f\n", d0, d10, d0 - d10, frequency);
		Board_UARTPutSTR(str);
		ITM_write(str);

		Sleep(300);
	}

    return 0 ;
}
