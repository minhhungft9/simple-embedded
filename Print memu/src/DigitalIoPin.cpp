/*
 * DigitalIoPin.cpp
 *
 *  Created on: Nov 9, 2016
 *      Author: Administrator
 */

#include "DigitalIoPin.h"
#include <iostream>
#include "chip.h"

DigitalIoPin::DigitalIoPin(int pin, int port, bool input, bool pullup, bool invert) {
	// TODO Auto-generated constructor stub
	this->pinn = pin;
	this->portt = port;
	Chip_IOCON_PinMuxSet(LPC_IOCON, pin, port, (IOCON_MODE_PULLUP | IOCON_DIGMODE_EN | IOCON_INV_EN));
	Chip_GPIO_SetPinDIRInput(LPC_GPIO, pin, port);
}

DigitalIoPin::~DigitalIoPin() {
	// TODO Auto-generated destructor stub
}

bool DigitalIoPin::read(){
	return Chip_GPIO_GetPinState(LPC_GPIO, pinn, portt);
}

void DigitalIoPin::write(bool value){
	Chip_GPIO_SetPinState(LPC_GPIO, pinn, portt, value);
}

