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

// ball row and column
unsigned char row = 0xFE;
unsigned char col = 0x10;

// default speed of ball
int ballSpeed = 7000;

unsigned char player1Score = 0x00;
unsigned char player2Score = 0x00;

// row of player 1 and 2
unsigned char player1Row = 0xF1;
unsigned char player2Row = 0xF1;

int getRandomNum() {
	return rand();
}

enum Vertical_Ball_States {VBallStart, ShiftUp, ShiftDown} vertical_ball_state;
int Vertical_Ball_SM_Tick(int state) {
	unsigned char tmp = ~row;

	switch (state) {
		case ShiftUp:
			// side paddle collision collision detection
			if (col == 0x40 && ((player1Row >> 1) & tmp) != 0 /*|| (col == 0x02 && ((player2Row << 1) & tmp) != 0)*/) {
				state = ShiftDown;

				// speed up ball once a side paddle hits it
				if (ballSpeed >= 6000) {
					ballSpeed -= 1000;
				}
			}
			else if (row == 0xFE) {
				state = ShiftDown;
			}
			break;
		case ShiftDown:
			// side paddle collision detection
			if ((col == 0x40 && ((player1Row << 1) & tmp) != 0) /*|| (col == 0x02 && ((player2Row << 1) & tmp) != 0) */) {
				state = ShiftUp;

				// speed up side paddle once 
				if (ballSpeed >= 6000) {
					ballSpeed -= 100;
				}
			}
			else if (row == 0xEF) {
				state = ShiftUp;
			}
			break;
		default:
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

enum Horizontal_Ball_States {ShiftLeft, ShiftRight} horizontal_ball_state;
int Horizontal_Ball_SM_Tick(int state) {
	unsigned char tmp1 = ~player1Row;
	unsigned char tmp2 = ~player2Row;
	unsigned char tmp3 = ~row;

	unsigned char player1Count = 0;
	unsigned char player2Count = 0;



	while (tmp1 && tmp3) {
		player1Count += tmp1 & tmp3 & 1;
		tmp1>>=1;
		tmp3>>=1;
	}

	tmp3 = ~row;

	while (tmp2 && tmp3) {
		player2Count += tmp2 & tmp3 & 1;
		tmp2>>=1;
		tmp3>>=1;
	}

	switch (state) {
		case ShiftLeft:		
			// make the ball's x direction go in the opposite direction if it hits the center paddle
			
			// horizontal ball collision detection
			if (col == 0x40 && player1Count == 1) {
				state = ShiftRight;

				// slow down ball
				if (ballSpeed <= 8000) {
					ballSpeed += 100;
				}
			}
			else if (col == 0x80) {
				state = ShiftRight;
			}
			break;
		case ShiftRight:
			// horizontal ball collision detection
			if (col == 0x02 && player2Count == 1) {
				state = ShiftLeft;

				// slow down ball
				if (ballSpeed <= 8000) {
					ballSpeed += 100;
				}
			}
			else if (col == 0x01) {
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

enum State {
	P1Start,
	P1UpKeyPress,
	P1UpKeyRelease,
	P1DownKeyPress,
	P1DownKeyRelease
} player1_state;

int Player1_SM_Tick(int state) {
	switch (state) {
		case P1Start:
			if (~PINB & 0x01) {
				state = P1UpKeyPress;
			}
			if (~PINB & 0x02) {
				state = P1DownKeyPress;
			}
			break;
		case P1UpKeyPress:
			state = P1UpKeyRelease;
			break;
		case P1UpKeyRelease:
			if (!(~PINB & 0x01)) {
				state = P1Start;
			}
			break;
		case P1DownKeyPress:
			state = P1DownKeyRelease;
			break;
		case P1DownKeyRelease:
			if (!(~PINB & 0x02)) {
				state = P1Start;
			}
			break;
		default:
			break;
	}

	switch (state) {
		case P1UpKeyPress:
			if (player1Row <= 0xF1) {
				player1Row = (0x01 << 7) | (player1Row >> 0x01);
			}
			break;
		case P1DownKeyPress:
			if (player1Row >= 0xF1) {
				unsigned char tmp = player1Row;
				tmp^=0x80;
				player1Row = (tmp << 1) | 0x01;
			}
		default:
			break;
	}

	return state;
}

enum ResetStates {
	ResetButtonWait,
	ResetButtonPress,
  ResetButtonRelease
} reset_state;

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
		  // reset scores
			PORTB = 0x03;
			player1Score = 0x00;
			player2Score = 0x00;
			
			// reset ball row and column
			row = 0xFE;
			col = 0x10;
			
			// reset player 1 and 2 to default row
			player1Row = 0xF1;
			player2Row = 0xF1;

			ballSpeed = 7000;
			break;
		case ResetButtonWait:
		case ResetButtonRelease:
		default:
			break;
	}

	return state;
}

enum {AIInit, AIWait} ai_state;
int AI_SM_Tick(int state) {
	// if the ball is in the column and the player is not there
	// collision detection elsewhere
	// if the ball is one column ahead and in the same row
	// as one of the piecies of the paddle
	

	switch (state) {
		case AIInit:
			state = AIWait;
			break;
		case AIWait:
		default:
			break;
	}


	int move;

	switch (state) {
		case AIWait:

			move = getRandomNum() % 2;

			if (col == 0x08 && move == 0 && player2Row <= 0xF1) {
				player2Row = (0x01 << 7) | (player2Row >> 1);
			}
			if(col == 0x08 && move == 1 && player2Row >= 0xF1) {
				unsigned char tmp = player2Row;
				tmp ^= 0x80;
				player2Row = (tmp << 1) | 0x01;
			}
			break;
		case AIInit:
		default:
			break;
	}


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

	

	PORTB = (player2Score << 5) | (player1Score << 2) | PORTB;

	return state;
}

enum States {P1Row, P2Row, BallRow} combine_state;
int Combine_SM_Tick(int state) {
	switch (state) {
		case P1Row:
			PORTD = player1Row;
			PORTC = 0x80;
			state = P2Row;
			break;
		case P2Row:
			PORTD = player2Row;
			PORTC = 0x01;
			state = BallRow;
			break;
		case BallRow:
			PORTD = row;
			PORTC = col;
			state = P1Row;
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

  	task3.state = P1Row;
	task3.period = 50;
	task3.elapsedTime = task3.period;
	task3.TickFct = &Combine_SM_Tick;

	task4.state = P1Start;
	task4.period = 100;
	task4.elapsedTime = task4.period;
	task4.TickFct = &Player1_SM_Tick;

	// will be changed to score SM and separate AI
	task5.state = AIInit;
	task5.period = ballSpeed;
	task5.elapsedTime = task5.period;
	task5.TickFct = &AI_SM_Tick;

  	task6.state = ResetButtonWait;
	task6.period = ballSpeed;
	task6.elapsedTime = task5.period;
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
					
				  if (i == 0 || i == 1) {  
				 	 tasks[i]->period = ballSpeed;
				  }
        		}
        		tasks[i]->elapsedTime += 50;
      		}

	    while(!TimerFlag);
	    TimerFlag = 0;
    	}

    return 1;
}
