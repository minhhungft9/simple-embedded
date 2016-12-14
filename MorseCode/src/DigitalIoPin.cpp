/*
 * DigitalIoPin.cpp
 *
 *  Created on: Nov 9, 2016
 *      Author: Tuan
 */

#include "DigitalIoPin.h"
#include "chip.h"

DigitalIoPin::~DigitalIoPin() {
}

DigitalIoPin::DigitalIoPin(int pin, int port, bool input, bool pullup, bool invert) {
	this->pin = pin;
	this->port = port;
	this->input = input;
	this->pullup = pullup;
	this->invert = invert;

	Chip_IOCON_PinMuxSet(LPC_IOCON, port, pin,
			(IOCON_DIGMODE_EN | (pullup ? IOCON_MODE_PULLUP : 0x0) | (invert ? IOCON_INV_EN : 0x0)));
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

bool DigitalIoPin::read() {
	bool i = Chip_GPIO_GetPinState(LPC_GPIO, this->port, this->pin);
	return i;
}

void DigitalIoPin::write(bool value) {
	Chip_GPIO_SetPinState(LPC_GPIO, this->port, this->pin, value);
}

