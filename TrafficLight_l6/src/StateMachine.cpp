/*
 * StateMachine.cpp
 *
 *  Created on: Dec 6, 2016
 *      Author: Administrator
 */
#include "StateMachine.h"
#include "board.h"

StateMachine::StateMachine(){
}

StateMachine::~StateMachine() {
}

void StateMachine::conft(DigitalIoPin &car, DigitalIoPin &pedestrian, DigitalIoPin &redlightcar,
		DigitalIoPin &yellowlightcar, DigitalIoPin &greenlightcar, DigitalIoPin &redlightped,
		DigitalIoPin &greenlightped, DigitalIoPin &pedbuttonlight){
	this->car = car;
	this->pedestrian = pedestrian;
	this->redlightcar = redlightcar;
	this->yellowlightcar = yellowlightcar;
	this->greenlightcar = greenlightcar;
	this->redlightped = redlightped;
	this->greenlightped = greenlightped;
	this->pedbuttonlight = pedbuttonlight;
	this->ifcar = false;
	this->timer = 0;
	this->currentState = Scar;
}

void StateMachine::setState(state newState){
	this->currentState = newState;
}

void StateMachine::HandleStateCar(Event &e){
	switch(e.type){
	case Event::Ecar:
		ifcar = true;
		timer = 0;
		break;
	case Event::Enocar:
		ifcar = false;
		break;
	case Event::Eped:
		ifped = true;
		if(ifcar == true){
			timer = 0;
			pedbuttonlight.write(false);
			setState(Spedbut);
		}else{
			timer = 0;
			setState(Spreped);
		}
		break;
	case Event::Etick:
		if(timer == 600){
			timer = 0;
			setState(Sprered);
		}else{
			timer++;
			greenlightcar.write(true);
			redlightped.write(true);
		}
		break;
	}
}

void StateMachine::HandleStateRed(Event &e){
	switch(e.type){
	case Event::Ecar:
		ifcar = true;
		timer = 0;
		setState(Sprecar);
		break;
	case Event::Enocar:
		ifcar = false;
		break;
	case Event::Eped:
		ifped = true;
		timer = 30;
		setState(Spreped);
		break;
	case Event::Etick:
		break;
	}
}

void StateMachine::HandleStatePrered(Event &e){
	switch(e.type){
	case Event::Ecar:
		ifcar = true;
		break;
	case Event::Enocar:
		ifcar = false;
		break;
	case Event::Eped:
		ifped = true;
		break;
	case Event::Etick:
		if(timer == 0){
			yellowlightcar.write(true);
			greenlightcar.write(false);
			timer++;
		}else if(timer == 30){
			timer = 0;
			yellowlightcar.write(false);
			redlightcar.write(true);
			setState(Sred);
		}else{
			timer++;
		}
		break;
	}
}

void StateMachine::HandleStatePrecar(Event &e){
	switch(e.type){
	case Event::Ecar:
		ifcar = true;
		break;
	case Event::Enocar:
		ifcar = false;
		break;
	case Event::Eped:
		ifped =  true;
		break;
	case Event::Etick:
		if(timer == 0){
			yellowlightcar.write(true);
			timer++;
		}else if(timer == 30){
			yellowlightcar.write(false);
			redlightcar.write(false);
			greenlightcar.write(true);
			greenlightped.write(false);
			redlightped.write(true);
			setState(Scar);
		}else{
			timer++;
		}
		break;
	}
}

void StateMachine::HandleStatePreped(Event &e){
	switch(e.type){
	case Event::Ecar:
		ifcar = true;
		break;
	case Event::Enocar:
		ifcar = false;
		break;
	case Event::Eped:
		ifped = true;
		break;
	case Event::Etick:
		if(timer == 0){
			greenlightcar.write(false);
			yellowlightcar.write(true);
			timer++;
		}else if(timer == 30){
			timer = 0;
			yellowlightcar.write(false);
			redlightcar.write(true);
			redlightped.write(false);
			greenlightped.write(true);
			setState(Sped);
		}else{
			timer++;
		}
		break;
	}
}

void StateMachine::HandleStatePed(Event &e){
	switch(e.type){
	case Event::Ecar:
		ifcar = true;
		break;
	case Event::Enocar:
		ifcar = false;
		break;
	case Event::Eped:
		ifped = true;
		break;
	case Event::Etick:
		if(timer == 150){
			timer = 0;
			setState(Sprecar);
		}else{
			timer++;
		}
		break;
	}
}

void StateMachine::HandleStatePedbut(Event &e){
	switch(e.type){
	case Event::Ecar:
		ifcar = true;
		break;
	case Event::Enocar:
		ifcar = false;
		break;
	case Event::Eped:
		ifped = true;
		break;
	case Event::Etick:
		if(ifcar == false || timer == 600){
			timer = 0;
			pedbuttonlight.write(true);
			setState(Spreped);
		}else{
			timer++;
		}
		break;
	}
}

void StateMachine::HandleState(Event &e){
	switch(currentState){
	case Scar: HandleStateCar(e); break;
	case Sprecar: HandleStatePrecar(e); break;
	case Sred: HandleStateRed(e); break;
	case Sprered: HandleStatePrered(e); break;
	case Sped: HandleStatePed(e); break;
	case Spreped: HandleStatePreped(e); break;
	case Spedbut: HandleStatePedbut(e); break;
	}
}


