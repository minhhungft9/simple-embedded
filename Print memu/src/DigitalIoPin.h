/*
 * DigitalIoPin.h
 *
 *  Created on: Nov 9, 2016
 *      Author: Administrator
 */

#ifndef DIGITALIOPIN_H_
#define DIGITALIOPIN_H_

class DigitalIoPin {
public:
	DigitalIoPin(int pin, int port, bool input = true, bool pullup = true, bool invert = false);
	virtual ~DigitalIoPin();
	bool read();
	void write(bool value);
private:
	int pinn;
	int portt;
};

#endif /* DIGITALIOPIN_H_ */


