/*
 * DigitalIoPin.cpp
 *
 *  Created on: Nov 9, 2016
 *      Author: Tuan
 */

#include "DigitalIoPin.h"
#include "chip.h"

DigitalIoPin::DigitalIoPin(int pin, int port, bool input, bool pullup, bool invert) {
	this->pin = pin;
	this->port = port;
	this->input = input;
	this->pullup = pullup;
	this->invert = invert;

	Chip_IOCON_PinMuxSet(LPC_IOCON, port, pin, (IOCON_DIGMODE_EN | (pullup == true ? IOCON_MODE_PULLUP : 0x0) | (invert == true ? IOCON_INV_EN : 0x0)));
	if (input == true)
		Chip_GPIO_SetPinDIRInput(LPC_GPIO, port, pin);
	else
		Chip_GPIO_SetPinDIROutput(LPC_GPIO, port, pin);
}

DigitalIoPin::DigitalIoPin(const DigitalIoPin &io) {
	pin = io.pin;
	port = io.port;
	input = io.input;
	pullup = io.pullup;
	invert = io.invert;
}

DigitalIoPin::~DigitalIoPin(){

}

bool DigitalIoPin::read() {
	return Chip_GPIO_GetPinState(LPC_GPIO, port, pin);
}

void DigitalIoPin::write(bool value) {
	Chip_GPIO_SetPinState(LPC_GPIO, this->port, this->pin, value);
}

