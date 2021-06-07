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
#include <stdlib.h>
#include <time.h>
#include <math.h>

#ifdef _SIMULATE_
#include "simAVRHeader.h"
#include "scheduler.h"
#include "timer.h"
#endif

enum Vertical_Ball_States {ShiftUp, ShiftDown} vertical_ball_state;

unsigned char row = 0xFE;
unsigned char col = 0x10;

unsigned char player1Score = 0x00;
unsigned char player2Score = 0x00;

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

enum Horizontal_Ball_States {ShiftLeft, ShiftRight} hball_state;

int Horizontal_Ball_SM_Tick(int state) {
	switch (state) {
		case ShiftLeft:		
			// make the ball's x direction go in the opposite direction if it hits the center paddle
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

enum States {Row1, Row2, Row3} my_state;
int Combine_SM_Tick(int state) {
	switch (state) {
		case Row3:
			PORTD = row;
			PORTC = col;
			break;
		default:
			break;
	}

	return state;
}

int main(void) {
    /* Insert DDR and PORT initializations */
	// B0, B1 inputs, the rest of B is output
	DDRB = 0xFC; PORTB = 0x03;
	DDRC = 0xFF; PORTC = 0x00;
	DDRD = 0x7F; PORTD = 0x80;

	srand(time(NULL));

	static task task1, task2, task3;
	task *tasks[] = { &task1, &task2, &task3 };
	unsigned short numTasks = sizeof(tasks)/sizeof(task*);

  task1.state = ShiftDown;
	task1.period = 7000;
	task1.elapsedTime = task1.period;
	task1.TickFct = &Vertical_Ball_SM_Tick;

	task2.state = ShiftRight;
	task2.period = 7000;
	task2.elapsedTime = task2.period;
	task2.TickFct = &Horizontal_Ball_SM_Tick;

  task3.state = Row1;
	task3.period = 7000;
	task3.elapsedTime = task4.period;
	task3.TickFct = &Combine_SM_Tick;

	TimerSet(1);
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
