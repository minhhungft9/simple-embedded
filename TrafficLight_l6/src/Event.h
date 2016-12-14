/*
 * Event.h
 *
 *  Created on: Dec 6, 2016
 *      Author: Administrator
 */

#ifndef EVENT_H_
#define EVENT_H_

class Event {
public:
	enum eventType { Ecar, Enocar, Eped, Etick };
	Event(eventType e);
	eventType type;
	virtual ~Event();
};

#endif /* EVENT_H_ */
