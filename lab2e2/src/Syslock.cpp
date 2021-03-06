/*
 * Syslock.cpp
 *
 *  Created on: Aug 29, 2017
 *      Author: Administrator
 */

#include "Syslock.h"

Syslock::Syslock() {
	// TODO Auto-generated constructor stub
	this->mutex = xSemaphoreCreateMutex();
}

Syslock::~Syslock() {
	// TODO Auto-generated destructor stub
	vSemaphoreDelete(mutex);
}

void Syslock::write(int *description){
	if(mutex != NULL){
		if(xSemaphoreTake(mutex, portMAX_DELAY) == pdTRUE){
			Board_UARTPutChar(*description);
			xSemaphoreGive(mutex);
		}
	}
}
