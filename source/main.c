/*	Author: lab
 *  Partner(s) Name: Jaired Jawed
 *	Lab Section:
 *	Assignment: Lab #  Exercise #
 *	Exercise Description: [optional - include for your own benefit]
 *
 *	I acknowledge all content contained herein, excluding template or example
 *	code, is my own original work.
 */
#include <avr/io.h>
#ifdef _SIMULATE_
#include "simAVRHeader.h"
#include "scheduler.h"
#include "timer.h"
#endif

/*
enum Demo_States {shiftRight, shiftLeft};
int Demo_Tick(int state) {
	static unsigned char pattern = 0x40;
	static unsigned char row = 0xFE;

	switch (state) {
		case shiftRight:
			pattern >>= 1;
			row = (row << 1) | 0x01;
			if (pattern == 0x01) {
				state = shiftLeft;
			}
			break;
		case shiftLeft:
			pattern <<= 1;
			if (pattern == 0x80) {
				state = shiftRight;
			}
			break;
		default:
			state = shiftRight;
			break;
	}


	// 
	//
	//if (row == 0xFE) {
	//	row = (row << 1) | 0x01;
	//}

	//if (row == 0xEF) {
		
	//}

	PORTC = pattern;
	PORTD = row;
	return state;
}
*/

enum Vertical_Ball_States {ShiftUp, ShiftDown};
int Vertical_Ball_SM_Tick(int state) {
	static unsigned char row = 0xFE;

	switch (state) {
		case ShiftUp:
			if (row == 0xFE) {
				state = ShiftDown;
			}
			break;
		case ShiftDown:
			if (row == 0xEF) {
				state = ShiftUp;
			}
			break;
		default:
			state = ShiftDown;
			break;
	}

	switch (state) {
		case ShiftUp:
			row = (0x01 << 7) | (row >> 1);
			break;
		case ShiftDown:
			row = (row << 1) | 0x01;
			break;
		default:
			break;
	}

	PORTD = row;

	return state;
}


enum Horizontal_Ball_States {ShiftLeft, ShiftRight};
int Horizontal_Ball_SM_Tick(int state) {
	static unsigned char col = 0x80;
	switch (state) {
		case ShiftLeft:
			if (col == 0x80) {
				state = ShiftRight;
			}
			break;
		case ShiftRight:
			if (col == 0x01) {
				state = ShiftLeft;
			}
			break;
		default:
			state = ShiftRight;
			break;
	}

	switch (state) {
		case ShiftLeft:
			col <<= 1;
			break;
		case ShiftRight:
			col >>= 1;
			break;
		default:
			break;
	}

	PORTC = col;

	return state;
}


int main(void) {
    /* Insert DDR and PORT initializations */
	DDRC = 0xFF; PORTC = 0x00;
	DDRD = 0xFF; PORTD = 0x00;

	static task task1, task2;
	task *tasks[] = { &task1, &task2 };
	unsigned short numTasks = sizeof(tasks)/sizeof(task*);

	task1.state = ShiftDown;
	task1.period = 100;
	task1.elapsedTime = task1.period;
	task1.TickFct = &Vertical_Ball_SM_Tick;

	task2.state = ShiftRight;
	task2.period = 100;
	task2.elapsedTime = task2.period;
	task2.TickFct = &Horizontal_Ball_SM_Tick;

	TimerSet(50);
	TimerOn();

	unsigned short i;
    /* Insert your solution below */
    while (1) {
	for (i = 0; i < numTasks; i++) {
		if (tasks[i]->elapsedTime == tasks[i]->period) {
			tasks[i]->state = tasks[i]->TickFct(tasks[i]->state);
			tasks[i]->elapsedTime = 0;
		}
		tasks[i]->elapsedTime += 50;
	}
	while(!TimerFlag);
	TimerFlag = 0;
    }
    return 1;
}
