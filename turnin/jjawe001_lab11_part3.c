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
#include "bit.h"
#include "scheduler.h"
#include "timer.h"
#include "keypad.h"
#endif

int getRandomNum() {
	return rand();
}


enum Vertical_Ball_States {VBallStart, ShiftUp, ShiftDown} vball_state;
static unsigned char row = 0xFE;
static unsigned char col = 0x10;

unsigned char rows[2] = {0xF1, 0xF1};
unsigned char player1Row;
unsigned char player2Row;


int ballSpeed=7000;
int AIEnabled = 1;

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
		case VBallStart:
			if (~PIND & 0x80) {
				state = ShiftDown;
			}
			break;
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


enum Horizontal_Ball_States {HBallStart, ShiftLeft, ShiftRight} hball_state;

// player1Score is 1 here because it acts as an indicator for which mode the second player is in (ai or human)
// before the start of the game
// once the game starts, it's set to 0

unsigned char player1Score = 0x01;
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
		case HBallStart:
			if (~PIND & 0x80) {
				state = ShiftRight;
			}
			break;
		case ShiftLeft:		
			// make the ball's x direction go in the opposite direction if it hits the center paddle
			if (col == 0x80 || (col == 0x40 && topPaddle == 0 && centerPaddle == 0 && bottomPaddle == 0)) {
				state = ShiftRight;
				ballSpeed=50;
			}
			break;
		case ShiftRight:
			if (col == 0x01 || (col == 0x02 && aiTopPaddle == 0 && aiCenterPaddle == 0 && aiBottomPaddle == 0)) {
				state = ShiftLeft;
				ballSpeed=50;
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

enum Player2States {Player2Start, Player2UpKeyPress, Player2UpKeyRelease, Player2DownKeyPress, Player2DownKeyRelease} player2_state;
unsigned char Player2_SM_Tick(int state) {
	unsigned char x = GetKeypadKey();

	
	if (AIEnabled == 0) {
		switch(state) {
			case Player2Start:
				if (x == '1') {
					state = Player2UpKeyPress;
				}
				if (x == '2') {
					state = Player2DownKeyPress;
				}
				break;
			case Player2UpKeyPress:
				state = Player2UpKeyRelease;
				break;
			case Player2UpKeyRelease:
				if (x != '1') {
				       state = Player2Start;
				}
		 		break;
			case Player2DownKeyPress:
				state = Player2DownKeyRelease;
				break;
			case Player2DownKeyRelease:
				if (x != '2') {
					state = Player2Start;
				}
			default:
				break;		
		}

		switch (state) {
			case Player2UpKeyPress:
				if (rows[1] <= topRow){
					rows[1] = (0x01 << 7) | (rows[1] >> 0x01);
				}
				break;
			case Player2DownKeyPress:
				if (rows[1] >= bottomRow) {
					unsigned char tmp = rows[1];
					rows[1] = (tmp << 1) | 0x01;
				}
			default:
				break;
		}
	}

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


	if (AIEnabled == 1) {	
		int move = getRandomNum() % 2;
		if (col == 0x08 && move == 0 && rows[1] <= topRow) {
			rows[1] = (0x01 << 7) | (rows[1] >> 1);
		}
		if(col == 0x08 && move == 1 && rows[1] >= bottomRow) {
			unsigned char test = rows[1];
			test ^= 0x80;
			rows[1] = (test << 1) | 0x01;
		}
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

// allows the choice of either the AI or Human Player for player 2
enum AI_Player2_States{ ChoiceStart, SinglePlayer, SinglePlayerPress, SinglePlayerRelease, MultiPlayer, MultiPlayerPress, MultiPlayerRelease, StopChoice} ai_player2_state;

// This function will briefly take over player1Score before the game starts to show whether
// the game is in single or multiplayer mode
// this function will no longer take action once the game has started
int AI_Player2_SM_Tick(int state) {
	switch (state) {
		case ChoiceStart:
			state = SinglePlayer;
			break;
		case SinglePlayer:
			if (~PIND & 0x40) {
				state = MultiPlayerPress;
			}
			// the game has now started in singleplayer mode
			if (~PIND & 0x80) {
				state = StopChoice;
			}
			break;
		case SinglePlayerPress:
			state = SinglePlayerRelease;
			break;
		case SinglePlayerRelease:
			if (!(~PIND & 0x40)) {
				state = SinglePlayer;
			}
			
			// if start button is pressed
			// start game in singleplayer mode
			// then StopChoice
			if (~PIND & 0x80) {
				state=StopChoice;
			}
			break;
		case MultiPlayer:
			if (~PIND & 0x40) {
				state = SinglePlayerPress;
			}
			// the game has now started in mutliplayer mode
			if (~PIND & 0x80) {
				state = StopChoice;
			}
			break;
		case MultiPlayerPress:
			state = MultiPlayerRelease;
			break;
		case MultiPlayerRelease:
			if (!(~PIND & 0x40)) {
				state = MultiPlayer;
			}

			if (~PIND & 0x80) {
				state = StopChoice;
			}
			break;
		case StopChoice:
			break;
		default:
			state = ChoiceStart;
			break;
	}


	switch (state) {
		case ChoiceStart:
		case SinglePlayerPress:
			player1Score = 0x01;
			AIEnabled = 1;
			break;
		case MultiPlayerPress:
			player1Score = 0x03;
			AIEnabled = 0;
			break;
		default:
			break;
	}

	// start
	// sets player1Score to 0x01
	// press button
	// moves to multiplayer state
	return state;
}

//

int main(void) {
    /* Insert DDR and PORT initializations */
	DDRA = 0xF0; PORTA = 0x0F;
	// B0, B1 inputs, the rest of B is output
	DDRB = 0xFC; PORTB = 0x03;
	DDRC = 0xFF; PORTC = 0x00;
	DDRD = 0x3F; PORTD = 0xC0;

	srand(time(NULL));

	static task task1, task2, task3, task4, task5, task6, task7, task8;
	task *tasks[] = { &task1, &task2, &task3, &task4, &task5, &task6, &task7, &task8};
	unsigned short numTasks = sizeof(tasks)/sizeof(task*);

	task1.state = VBallStart;
	task1.period = ballSpeed;
	task1.elapsedTime = task1.period;
	task1.TickFct = &Vertical_Ball_SM_Tick;

	task2.state = HBallStart;
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

	task7.state = Player2Start;
	task7.period = 100;
	task7.elapsedTime = task7.period;
	task7.TickFct = &Player2_SM_Tick;

	task8.state = ChoiceStart;
	task8.period = 50;
	task8.elapsedTime = task8.period;
	task8.TickFct = &AI_Player2_SM_Tick;

	TimerSet(1);
	TimerOn();

//	ballSpeed = 6000;
//	task1.period = ballSpeed;
//	task2.period = ballSpeed;
//
	unsigned short i;
    /* Insert your solution below */
    while (1) {
	for (i = 0; i < numTasks; i++) {
		//if (i == 0 || i == 1) {
		//	tasks[i]->period = 4000;
		//	tasks[1]->elapsedTime = 4000;
		//}



		if (tasks[i]->elapsedTime == tasks[i]->period) {
			tasks[i]->state = tasks[i]->TickFct(tasks[i]->state);
				//	if (i == 0 || i == 1)
				//tasks[i]->period = ballSpeed;
			tasks[i]->elapsedTime = 0;
		}
		tasks[i]->elapsedTime += 50;
	}

	while(!TimerFlag);
	TimerFlag = 0;
	  // tasks[0]->period = ballSpeed;
	  // tasks[1]->period = ballSpeed;


    }
    return 1;
}
