/*
 * MorseSender.cpp
 *
 *  Created on: Nov 15, 2016
 *      Author: Administrator
 */

#include "MorseSender.h"
#include <ctype.h>
#include "board.h"
#include <cstdio>
#include "math.h"
void Sleep(int ms);

MorseSender::MorseSender(DigitalIoPin *led, DigitalIoPin *decoder){
	this->led = new DigitalIoPin(*led);
	this->decoder = new DigitalIoPin(*decoder);
}

MorseSender::~MorseSender() {

}

bool MorseSender::command(char *text){
	//send command
	if(text[0] == 's' && text[1] == 'e' && text[2] == 't'){
		char setting[30];
		snprintf(setting, 30, "\n\rwpm: %d\n\rdot length: %d\n\r", this->wpm, 1000/this->wpm);
		for(int i = 0; i < 30; i++){
			if(setting[i] == '\0' ){
				break;
			}
			Board_UARTPutChar(setting[i]);
		}
		text[0] = '\0';
		return false;
	}
	//wpm command
	if(text[0] == 'w' && text[1] == 'p' && text[2] == 'm'){
		this->wpm = 0;
		char number[4];
		int index = 0;
		while(index < 3){
			if(text[index+4] < 0x30 || text[index+4] > 0x39){
				break;
			}
			number[index] = text[index+4];
			index++;
		}
		for(int i = 0; i < index; i++){
			this->wpm += (static_cast<int>(number[i]) - 48)*pow(10,index-i-1);
		}
		if(this->wpm == 0){
			this->wpm = 20;
		}
		text[0] = '\0';
		return false;
	}
	//send command
	if(text[0] == 's' && text[1] == 'e' && text[2] == 'n' && text[3] == 'd'){
		int index = 0;
		while(text[index+5] != '\0'){
			text[index] = text[index+5];
			index++;
		}
		text[index] = text[index+5];
		return true;
	}
	return false;
}

void MorseSender::delay(int number, int dotlength){
	if(number == 1){
		Board_UARTPutChar('.');
		this->decoder->write(true);
		this->led->write(false);
		Sleep(dotlength*1);
		this->decoder->write(false);
		this->led->write(true);
		Sleep(dotlength*1);
	}else if(number == 3){
		Board_UARTPutChar('=');
		this->decoder->write(true);
		this->led->write(false);
		Sleep(dotlength*3);
		this->decoder->write(false);
		this->led->write(true);
		Sleep(dotlength*1);
	}else{
		this->decoder->write(false);
		this->led->write(true);
		Sleep(dotlength*3);
	}
	this->led->write(true);
	this->decoder->write(false);
}

void MorseSender::sendCode(char *text){

	int index = 0;
	int dot = 1000/this->wpm;
	int result[8];

	while(text[index+1] != '\0'){
		char c = toupper(text[index]);
		if(c == ' '){
			delay(2, dot);
			index++;
			continue;
		}
		unsigned int k = 0;
		while(ITU_morse[k].symbol != '0'){
			if(c == ITU_morse[k].symbol){
				for(unsigned int n = 0; n < 7; n++){
					result[n] = ITU_morse[k].code[n];
					if(result[n] == 1){
						delay(1, dot);
					}else if(result[n] == 3){
						delay(3, dot);
					}
				}
				break;
			}else{
				k++;
			}
		}
		if(ITU_morse[k].symbol == '0'){
			delay(3, dot);
			delay(1, dot);
			delay(1, dot);
			delay(3, dot);
		}
		delay(2, dot);
		index++;
	}
	for(int i = 0; i < 80; i++){
		text[i] = '\0';
	}
}

