/*
 * StateMachine.h
 *
 *  Created on: Dec 6, 2016
 *      Author: Administrator
 */

#ifndef STATEMACHINE_H_
#define STATEMACHINE_H_

#include "DigitalIoPin.h"
#include "Event.h"

class StateMachine {
public:
	StateMachine();
	virtual ~StateMachine();
	void HandleState(Event &e);
	void conft(DigitalIoPin &car, DigitalIoPin &pedestrian, DigitalIoPin &redlightcar,
			DigitalIoPin &yellowlightcar, DigitalIoPin &greenlightcar, DigitalIoPin &redlightped,
			DigitalIoPin &greenlightped, DigitalIoPin &pedbuttonlight);
private:
	DigitalIoPin car;
	DigitalIoPin pedestrian;
	DigitalIoPin redlightcar;
	DigitalIoPin yellowlightcar;
	DigitalIoPin greenlightcar;
	DigitalIoPin redlightped;
	DigitalIoPin greenlightped;
	DigitalIoPin pedbuttonlight;

	enum state { Sred, Sprered, Scar, Sprecar, Sped, Spreped, Spedbut };
	state currentState;
	void setState(state newState);

	void HandleStateCar(Event &e);
	void HandleStateRed(Event &e);
	void HandleStatePrered(Event &e);
	void HandleStatePrecar(Event &e);
	void HandleStatePreped(Event &e);
	void HandleStatePed(Event &e);
	void HandleStatePedbut(Event &e);

	bool ifcar;
	bool ifped;
	int timer;
};

#endif /* STATEMACHINE_H_ */
