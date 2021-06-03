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

enum Vertical_Ball_States {ShiftUp, ShiftDown};
static unsigned char row = 0xFE;

int Vertical_Ball_SM_Tick(int state) {
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

	return state;
}


enum Horizontal_Ball_States {ShiftLeft, ShiftRight};
static unsigned char col = 0x80;

int Horizontal_Ball_SM_Tick(int state) {
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

	return state;
}

enum State {State1};
//static unsigned char startCol = 0x40;
//static unsigned char startRow = 0xF1;

int Player_SM_Tick(int state) {
	if (~PINB & 0x01 && PIND <= 0xF9) {
		PORTD = (0x01 << 7) | (PORTD >> 1);
	}

	if (~PINB & 0x02 && PIND >= 0xF3) {
		unsigned char tmp = PIND;
		tmp = tmp^0x80;
		PORTD = (tmp << 1) | 0x01;
	}

	return state;
}

int main(void) {
    /* Insert DDR and PORT initializations */
	DDRB = 0x00; PORTB = 0xFF;
	DDRC = 0xFF; PORTC = 0x00;
	DDRD = 0xFF; PORTD = 0x00;

	static task task1, task2, task3;
	task *tasks[] = { &task1, &task2, &task3 };
	unsigned short numTasks = sizeof(tasks)/sizeof(task*);

	task1.state = ShiftDown;
	task1.period = 100;
	task1.elapsedTime = task1.period;
	task1.TickFct = &Vertical_Ball_SM_Tick;

	task2.state = ShiftRight;
	task2.period = 100;
	task2.elapsedTime = task2.period;
	task2.TickFct = &Horizontal_Ball_SM_Tick;

	task3.state = State1;
	task3.period = 50;
	task3.elapsedTime = task3.period;
	task3.TickFct = &Player_SM_Tick;

	TimerSet(50);
	TimerOn();
	
	PORTC = 0x40;
	PORTD = 0xF9;

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
