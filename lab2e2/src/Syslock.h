/*
 * Syslock.h
 *
 *  Created on: Aug 29, 2017
 *      Author: Administrator
 */

#ifndef SYSLOCK_H_
#define SYSLOCK_H_

#include "FreeRTOS.h"
#include "semphr.h"

class Syslock {
	public:
		Syslock();
		virtual ~Syslock();
		void write(int *description);
	private:
		SemaphoreHandle_t mutex;
};

#endif /* SYSLOCK_H_ */
