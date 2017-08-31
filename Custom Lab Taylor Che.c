/*
 * Custom.c
 *
 * Created: 8/24/2017 11:00:31 PM
 * Author : tche0
 */ 

#include <avr/io.h>
#include "io.c"
#include <avr/io.h>
#include <avr/interrupt.h>
#include <stdlib.h>
#include <string.h>

//String Concatonation Function Source:https://stackoverflow.com/questions/8465006/how-do-i-concatenate-two-strings-in-c


char* concat(const char *s1, const char *s2)
{
	char *result = malloc(strlen(s1)+strlen(s2)+1);//+1 for the zero-terminator
	//in real code you would check for errors in malloc here
	strcpy(result, s1);
	strcat(result, s2);
	return result;
}

//Global Functions

//PWM Functions
void set_PWM(double frequency) {
	
	// Keeps track of the currently set frequency
	// Will only update the registers when the frequency
	// changes, plays music uninterrupted.
	static double current_frequency;
	if (frequency != current_frequency) {

		if (!frequency) TCCR3B &= 0x08; //stops timer/counter
		else TCCR3B |= 0x03; // resumes/continues timer/counter
		
		// prevents OCR3A from overflowing, using prescaler 64
		// 0.954 is smallest frequency that will not result in overflow
		if (frequency < 0.954) OCR3A = 0xFFFF;
		
		// prevents OCR3A from underflowing, using prescaler 64					// 31250 is largest frequency that will not result in underflow
		else if (frequency > 31250) OCR3A = 0x0000;
		
		// set OCR3A based on desired frequency
		else OCR3A = (short)(8000000 / (128 * frequency)) - 1;

		TCNT3 = 0; // resets counter
		current_frequency = frequency;
	}
}

void PWM_on() {
	TCCR3A = (1 << COM3A0);
	// COM3A0: Toggle PB6 on compare match between counter and OCR3A
	TCCR3B = (1 << WGM32) | (1 << CS31) | (1 << CS30);
	// WGM32: When counter (TCNT3) matches OCR3A, reset counter
	// CS31 & CS30: Set a prescaler of 64
	set_PWM(0);
}

void PWM_off() {
	TCCR3A = 0x00;
	TCCR3B = 0x00;
}

//Tick Function
volatile unsigned char TimerFlag = 0; // TimerISR() sets this to 1. C programmer should clear to 0.

// Internal variables for mapping AVR's ISR to our cleaner TimerISR model.
unsigned long _avr_timer_M = 1; // Start count from here, down to 0. Default 1 ms.
unsigned long _avr_timer_cntcurr = 0; // Current internal count of 1ms ticks

unsigned char periodFlag=0x00;

void TimerOn() {
	// AVR timer/counter controller register TCCR1
	TCCR1B = 0x0B;// bit3 = 0: CTC mode (clear timer on compare)
	// bit2bit1bit0=011: pre-scaler /64
	// 00001011: 0x0B
	// SO, 8 MHz clock or 8,000,000 /64 = 125,000 ticks/s
	// Thus, TCNT1 register will count at 125,000 ticks/s

	// AVR output compare register OCR1A.
	OCR1A = 125;	// Timer interrupt will be generated when TCNT1==OCR1A
	// We want a 1 ms tick. 0.001 s * 125,000 ticks/s = 125
	// So when TCNT1 register equals 125,
	// 1 ms has passed. Thus, we compare to 125.
	// AVR timer interrupt mask register
	TIMSK1 = 0x02; // bit1: OCIE1A -- enables compare match interrupt

	//Initialize avr counter
	TCNT1=0;

	_avr_timer_cntcurr = _avr_timer_M;
	// TimerISR will be called every _avr_timer_cntcurr milliseconds

	//Enable global interrupts
	SREG |= 0x80; // 0x80: 1000000
}

void TimerOff() {
	TCCR1B = 0x00; // bit3bit1bit0=000: timer off
}


void TimerISR() {
	TimerFlag = 1;
	periodFlag++;
}

// In our approach, the C programmer does not touch this ISR, but rather TimerISR()
ISR(TIMER1_COMPA_vect) {
	// CPU automatically calls when TCNT1 == OCR1 (every 1 ms per TimerOn settings)
	_avr_timer_cntcurr--; // Count down to 0 rather than up to TOP
	if (_avr_timer_cntcurr == 0) { // results in a more efficient compare
		TimerISR(); // Call the ISR that the user uses
		_avr_timer_cntcurr = _avr_timer_M;
	}
}

// Set TimerISR() to tick every M ms
void TimerSet(unsigned long M) {
	_avr_timer_M = M;
	_avr_timer_cntcurr = _avr_timer_M;
}

//Global Variables
unsigned char hour, minute; //Clock Variables
unsigned char tempA, tempB;

volatile unsigned char PauseFlag = 0;


double TETRIS[40] = { _E5, _B4, _C5, _D5, _C5,
					_B4, _A4, _A4, _C5, _E5,
					_D5, _C5, _B4, _B4, _C5,
					_D5, _E5, _C5, _A4, _A4,
					_R, _D5, _F5, _A5, _G5,
					_F5, _E5, _C5, _E5, _D5,
					_C5, _B4, _B4, _C5, _D5,
					_E5, _C5, _A4, _A4, _R,};

double FUR[35] = {_E5, _DS5, _E5, _DS5, _E5,
				_B4, _D5, _C5, _A4, _C4,
				_E4, _A4, _B4, _E4, _GS4,
				_B4, _C5, _E4, _E5, _DS5,
				_E5, _DS5, _E5, _B4, _D5,
				_C5, _A4, _C4, _E4, _A4,
				_B4, _E4, _C5, _B4, _A4
				 };

double MARIO[80] = {_E7, _E7, 0, _E7,
					0, _C7, _E7, 0,
					_G7, 0, 0,  0,
					_G6, 0, 0, 0,
					_C7, 0, 0, _G6,
					0, 0, _E6, 0,
					0, _A6, 0, _B6,
					0, _AS6, _A6, 0,
					_G6, _E7, _G7,
					_A7, 0, _F7, _G7,
					0, _E7, 0, _C7,
					_D7, _B6, 0, 0,
					_C7, 0, 0, _G6,
					0, 0, _E6, 0,
					0, _A6, 0, _B6,
					0, _AS6, _A6, 0,
					_G6, _E7, _G7,
					_A7, 0, _F7, _G7,
					0, _E7, 0, _C7,
					_D7, _B6, 0, 0
					};

double STAR[30] = {_C4, _C4, _C4, _G4, _G4,
					_G4, _F4, _E4, _D4, _C5,
					_C5, _C5, _G4, _G4, _F4,
					_E4, _D4, _C5, _C5, _C5,
					_G4, _G4, _F4, _E4, _F4,
					_D4, _D4, _D4, _D4, _D4};

double *ACCESS[] = { NULL, TETRIS, FUR, MARIO, STAR };

char *MENU_STR[] = {"SELECT SONG", "TETRIS", "FUR ELISE", "MARIO", "STAR WARS"};
unsigned char numMenu;

unsigned int note_num;

unsigned int period;

//Play Music SM

int val,l;

int length(int numMenu){
	
	if (numMenu==1)
	{
		val = 40;
	}
	else if (numMenu==2)
	{
		val = 35;
	}
	else if (numMenu==3)
	{
		val = 80;
	}
	else if (numMenu==4)
	{
		val = 30;
	}
	
	return val;
}

enum SM1_States { SM1_INIT, SM1_MENU, SM1_PLAY, SM1_PAUSE,
				  SM1_NEXT, SM1_RESET, SM1_BACK } SM1_State;

void TickFct_State_machine_1() {
	
	switch(SM1_State) { // Transitions

		//INITIAL STATE "LOADING"
		case SM1_INIT:
		if (1) {
			SM1_State = SM1_MENU;
		}
		break;
		
		// MENU
		case SM1_MENU:

		if (tempA==0x08) {
			if(numMenu>=4){
				numMenu=1;
			}
			else{
				numMenu++;
			}
			SM1_State = SM1_NEXT;
		}
		else if(tempA==0x10){
			if (numMenu<=1)
			{
				numMenu=4;
			}
			else{
				numMenu--;
			}
			SM1_State = SM1_BACK;
		}
		else if (tempA==0x01) {
			if (!(numMenu==0))
			{
				SM1_State = SM1_PLAY;
				l = length(numMenu);
			}
			else
			{
				SM1_State = SM1_MENU;
			}
		}
		else{
			SM1_State=SM1_MENU;
		}
		break;
		
		
		case SM1_PLAY:
		
		if (tempA==0x08) {
			if(numMenu>=4){
				numMenu=1;
			}
			else{
				numMenu++;
			}
			SM1_State = SM1_NEXT;
		}
		else if(tempA==0x10){
			if (numMenu<=1)
			{
				numMenu=4;
			}
			else{
				numMenu--;
			}
			SM1_State = SM1_BACK;
		}
		else if (tempA==0x02) {
			SM1_State = SM1_PAUSE;
		}
		
		else if (note_num<l) {
			SM1_State = SM1_PLAY;
		}
		else if ((note_num>=l)) {
			SM1_State = SM1_MENU;
		}
		else{
			SM1_State = SM1_PLAY;
		}
		break;
		
		case SM1_PAUSE:
		if (tempA==0x08) {
			if(numMenu>=4){
				numMenu=1;
			}
			else{
				numMenu++;
			}
			SM1_State = SM1_NEXT;
		}
		else if(tempA==0x10){
			if (numMenu<=1)
			{
				numMenu=4;
			}
			else{
				numMenu--;
			}
			SM1_State = SM1_BACK;
		}
		else if (tempA==0x01) {
			SM1_State = SM1_PLAY;
			LCD_DisplayString(1, MENU_STR[numMenu]);
		}
		else if (tempA==0x02) {
			SM1_State = SM1_MENU;
			numMenu=0;
			LCD_DisplayString(1, MENU_STR[numMenu]);
		}
		else if (tempA==0x04) {
			SM1_State = SM1_RESET;
		}
		else{
			SM1_State=SM1_PAUSE;
		}
		break;
		
		case SM1_NEXT:
		l = length(numMenu);
		if(note_num>0){
			note_num = 0;
			SM1_State=SM1_PLAY;
		}
		else{
			SM1_State = SM1_MENU;
		}
		break;
		
		case SM1_RESET:
		if (1) {
			SM1_State = SM1_PAUSE;
		}
		break;
		
		case SM1_BACK:
		l = length(numMenu);
		if(note_num>0){
			note_num = 0;
			SM1_State=SM1_PLAY;
		}
		else{
			SM1_State = SM1_MENU;
		}
		break;
		
		default:
		SM1_State = SM1_INIT;
	} // Transitions



	switch(SM1_State) { // State actions
		case SM1_INIT:
			LCD_DisplayString(1, "Starting...");
			numMenu=0;
			set_PWM(0);
			l=0;
		break;
		
		case SM1_MENU:
			LCD_DisplayString(1, MENU_STR[numMenu]);
			set_PWM(0);
			note_num = 0;
		break;
		
		case SM1_PLAY:
			if(note_num<(l)){
				set_PWM(ACCESS[numMenu][note_num]);
				note_num++;
			}
			else{
				note_num = 0;
				SM1_State=SM1_MENU;
			}
			LCD_DisplayString(1, concat("PLAY ",MENU_STR[numMenu]));
			
		break;
		
		case SM1_PAUSE:
			LCD_DisplayString(1,concat("PAUSED ",MENU_STR[numMenu]));
			set_PWM(0);
		break;

		case SM1_NEXT:	
		break;
		
		case SM1_RESET:
			LCD_DisplayString(1, "RESETTING");
			note_num=0;
		break;
		
		case SM1_BACK:
		break;
		
		default: // ADD default behaviour below
		break;
	} // State actions

}

unsigned char waitFlag=0x00;

int main(void)
{
   	DDRA = 0x00; PORTA = 0XFF;
	DDRB = 0xFF; PORTB = 0x00;
   	DDRC = 0xFF; PORTC = 0x00;
   	DDRD = 0xFF; PORTD = 0x00;
	
	period = 200;
	
	SM1_State = SM1_INIT;
	
	LCD_init();
	
	TimerOn();
	TimerSet(period);
	PWM_on();
    while (1) 
    {
			tempA = ~PINA;

				TickFct_State_machine_1();

			
			while(!TimerFlag);
			TimerFlag = 0;
    }
}

