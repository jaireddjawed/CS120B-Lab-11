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

int getRandomNum() {
	return rand();
}


enum Vertical_Ball_States {ShiftUp, ShiftDown};
static unsigned char row = 0xFE;
static unsigned char col = 0x10;

unsigned char rows[2] = {0xE3, 0xF8};
unsigned char player1Row;
unsigned char player2Row;


int ballSpeed=7000;

int Vertical_Ball_SM_Tick(int state) {
	unsigned char op = ~row;
	// get 0 bit of ball row
	op = (log(op & -op)/log(2)) + 1;
			
	// hits player1 side or player1 paddle
	int topPaddle = (player1Row & (1 << ((op - 1) - 1))) >> ((op - 1) - 1);
	int centerPaddle = (player1Row & (1 << (op - 1))) >> (op - 1);
	int bottomPaddle = (player1Row & (1 << ((op + 1) - 1))) >> ((op + 1) - 1);

	// hits ai side or ai paddle
	int aiTopPaddle = (rows[1] & (1 << ((op - 1) - 1))) >> ((op - 1) - 1);
	int aiCenterPaddle = (rows[1] & (1 << (op - 1))) >> (op - 1);
	int aiBottomPaddle = (rows[1] & (1 << (op + 1) - 1)) >> ((op + 1) - 1);


	switch (state) {
		case ShiftUp:
			if (row == 0xFE || (col == 0x40 && centerPaddle == 0 && bottomPaddle == 0) || (col == 0x02 && aiCenterPaddle == 0 && aiTopPaddle == 0)) {
				state = ShiftDown;
			}
			break;
		case ShiftDown:
			if (row == 0xEF || (col == 0x40 && topPaddle == 0 && centerPaddle == 0) || (col == 0x02 && aiBottomPaddle == 0 && aiCenterPaddle == 0)) {
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

unsigned char player1Score = 0x00;
unsigned char player2Score = 0x00;

int Horizontal_Ball_SM_Tick(int state) {
	unsigned char op = ~row;
	// get 0 bit of ball row
	op = (log(op & -op)/log(2)) + 1;
			

	// hits player1 side or player1 paddle
	int topPaddle = (player1Row & (1 << ((op - 1) - 1))) >> ((op - 1) - 1);
	int centerPaddle = (player1Row & (1 << (op - 1))) >> (op - 1);
	int bottomPaddle = (player1Row & (1 << ((op + 1) - 1))) >> ((op + 1) - 1);

	// hits ai side or ai paddle
	int aiTopPaddle = (rows[1] & (1 << ((op - 1) - 1))) >> ((op - 1) - 1);
	int aiCenterPaddle = (rows[1] & (1 << (op - 1))) >> (op - 1);
	int aiBottomPaddle = (rows[1] & (1 << (op + 1) - 1)) >> ((op + 1) - 1);

	switch (state) {
		case ShiftLeft:		
			// make the ball's x direction go in the opposite direction if it hits the center paddle
			if (col == 0x80 || (col == 0x40 && topPaddle == 0 && centerPaddle == 0 && bottomPaddle == 0)) {
				state = ShiftRight;
			}
			break;
		case ShiftRight:
			if (col == 0x01 || (col == 0x02 && aiTopPaddle == 0 && aiCenterPaddle == 0 && aiBottomPaddle == 0)) {
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

enum State {Player1Start, Player1UpKeyPress, Player1UpKeyRelease, Player1DownKeyPress, Player1DownKeyRelease} player1_state;
unsigned char topRow = 0xF1;
unsigned char bottomRow = 0xf1;

int Player_SM_Tick(int state) {
	player1Row = rows[0];

	switch (state) {
		case Player1Start:
			if (~PINB & 0x01) {
				state = Player1UpKeyPress;
			}
			if (~PINB & 0x02) {
				state = Player1DownKeyPress;
			}
			break;
		case Player1UpKeyPress:
			state = Player1UpKeyRelease;
			break;
		case Player1UpKeyRelease:
			if (!(~PINB & 0x01)) {
				state = Player1Start;
			}
			else {
				state = Player1UpKeyRelease;
			}
			break;
		case Player1DownKeyPress:
			state = Player1DownKeyRelease;
			break;
		case Player1DownKeyRelease:
			if (!(~PINB & 0x02)) {
				state = Player1Start;
			}
			else {
				state = Player1DownKeyRelease;
			}
			break;
		default:
			state = Player1Start;
			break;
	}

	switch (state) {
		case Player1UpKeyPress:
			if (player1Row <= topRow) {
				player1Row = (0x01 << 7) | (player1Row >> 0x01);
			}
			break;
		case Player1DownKeyPress:
			if (player1Row >= bottomRow) {
				player1Row ^= 0x80;
				player1Row = (player1Row << 1) | 0x01;
			}
			break;
		case Player1Start:
		case Player1UpKeyRelease:
		case Player1DownKeyRelease:
		default:
			break;
	}


	rows[0] = player1Row;

	return state;
}

enum {S1} ai_state;
int AI_SM_Tick(int state) {
	// if the ball is in the column and the player is not there
	// collision detection elsewhere
	// if the ball is one column ahead and in the same row
	// as one of the piecies of the paddle
	
	if (col == 0x80) {
		if (player2Score == 0x07) {
			player1Score = 0x00;
			player2Score = 0x00;
			PORTB = 0x03;
		}
		player2Score = (player2Score << 1) | 0x01;
	}
	if (col == 0x01) {
		if (player1Score == 0x07) {
			player1Score = 0x00;
			player2Score = 0x00;
			PORTB = 0x03;
		}
		player1Score = (player1Score << 1) | 0x01;
	}

	
	int move = getRandomNum() % 2;
	if (col == 0x08 && move == 0 && rows[1] <= topRow) {
		rows[1] = (0x01 << 7) | (rows[1] >> 1);
	}
	if(col == 0x08 && move == 1 && rows[1] >= bottomRow) {
		unsigned char test = rows[1];
		test ^= 0x80;
		rows[1] = (test << 1) | 0x01;
	}

	PORTB = (player2Score << 5) | (player1Score << 2) | PORTB;

	return state;
}

enum States {Row1, Row2, Row3} my_state;
int Combine_SM_Tick(int state) {
	unsigned char ballCol = 0x08;
	unsigned char ballRow = 0xFE;

	switch (state) {
		case Row1:
			PORTD = rows[0];
			PORTC = 0x80;
			state = Row2;
			break;
		case Row2:
			PORTD = rows[1];
			PORTC = 0x01;
			state = Row3;
			break;
		case Row3:
			PORTD = row;
			PORTC = col;
			state = Row1;
			break;
		default:
			break;
	}

	return state;
}

enum ResetStates {ResetButtonWait, ResetButtonPress, ResetButtonRelease} reset_state;
int Reset_SM_Tick(int state) {
	switch (state) {
		case ResetButtonWait:
			if (~PIND & 0x80) {
				state = ResetButtonPress;
			}
			break;
		case ResetButtonPress:
			state = ResetButtonRelease;
			break;
		case ResetButtonRelease:
			if (!(~PIND & 0x80)) {
				state = ResetButtonWait;
			}
			break;
		default:
			break;
	}

	switch (state) {
		// reset everything to default states
		case ResetButtonPress:
			PORTB = 0x03;
			
			player1Score = 0x00;
			player2Score = 0x00;
			
			row = 0xFE;
			col = 0x10;
			
			rows[0] = 0xF1;
			rows[1] = 0xF1;
			break;
		case ResetButtonWait:
		case ResetButtonRelease:
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

	static task task1, task2, task3, task4, task5, task6;
	task *tasks[] = { &task1, &task2, &task3, &task4, &task5, &task6 };
	unsigned short numTasks = sizeof(tasks)/sizeof(task*);

	task1.state = ShiftDown;
	task1.period = ballSpeed;
	task1.elapsedTime = task1.period;
	task1.TickFct = &Vertical_Ball_SM_Tick;

	task2.state = ShiftRight;
	task2.period = ballSpeed;
	task2.elapsedTime = task2.period;
	task2.TickFct = &Horizontal_Ball_SM_Tick;

	
	task3.state = Player1Start;
	task3.period = 100;
	task3.elapsedTime = task3.period;
	task3.TickFct = &Player_SM_Tick;

	task4.state = Row1;
	task4.period = 50;
	task4.elapsedTime = task4.period;
	task4.TickFct = &Combine_SM_Tick;

	task5.state = S1;
	task5.period = 7000;
	task5.elapsedTime = task5.period;
	task5.TickFct = &AI_SM_Tick;

	task6.state = ResetButtonWait;
	task6.period = 7000;
	task6.elapsedTime = task6.period;
	task6.TickFct = &Reset_SM_Tick;

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
