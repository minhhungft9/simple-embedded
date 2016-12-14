/*
 * DigitalIoPin.h
 *
 *  Created on: Nov 9, 2016
 *      Author: Tuan
 */

#ifndef DIGITALIOPIN_H_
#define DIGITALIOPIN_H_

class DigitalIoPin {
public:
	DigitalIoPin(int pin, int port, bool input = true, bool pullup = true, bool invert = false);
	DigitalIoPin(const DigitalIoPin &io);
	virtual ~DigitalIoPin();
	bool read();
	void write(bool value);
private:
	int pin;
	int port;
	bool input, pullup, invert;
};

#endif /* DIGITALIOPIN_H_ */
