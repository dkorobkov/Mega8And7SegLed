/*
 * Indicator.c
 *
 * Created: 12.12.2016 17:53:01
 *  Author: https://github.com/dkorobkov
 */ 

#include <avr/pgmspace.h>
#include <avr/io.h>
#include <string.h>

#include "Indicator.h"

// Check for errors
#ifndef NUM_DIGITS 
	#error You must define NUM_DIGITS
#endif
	
#if NUM_DIGITS != 2 && NUM_DIGITS != 4
	#error NUM_DIGITS must be 2 or 4
#endif

#ifndef COMMON_PIN
#error You must define COMMON_PIN
#endif

#if COMMON_PIN != COMMON_CATHODE && COMMON_PIN != COMMON_ANODE
#error COMMON_PIN must be COMMON_CATHODE or COMMON_ANODE
#endif


#if COMMON_PIN == COMMON_CATHODE
	#define DIGIT_1_ON  (PORTB |=  (1<<PB3))
	#define DIGIT_1_OFF (PORTB &= ~(1<<PB3))
	#define DIGIT_2_ON  (PORTB |=  (1<<PB2))
	#define DIGIT_2_OFF (PORTB &= ~(1<<PB2))
	#define DIGIT_3_ON  (PORTB |=  (1<<PB1))
	#define DIGIT_3_OFF (PORTB &= ~(1<<PB1))
	#define DIGIT_4_ON  (PORTB |=  (1<<PB0))
	#define DIGIT_4_OFF (PORTB &= ~(1<<PB0))
	#define DIGITS_OFF  (PORTB &= ~( (1<<PB0) | (1<<PB1) | (1<<PB2) | (1<<PB3)))
#endif

#if COMMON_PIN == COMMON_ANODE
#define DIGIT_1_OFF  (PORTB |=  (1<<PB3))
#define DIGIT_1_ON (PORTB &= ~(1<<PB3))
#define DIGIT_2_OFF  (PORTB |=  (1<<PB2))
#define DIGIT_2_ON (PORTB &= ~(1<<PB2))
#define DIGIT_3_OFF  (PORTB |=  (1<<PB1))
#define DIGIT_3_ON (PORTB &= ~(1<<PB1))
#define DIGIT_4_OFF  (PORTB |=  (1<<PB0))
#define DIGIT_4_ON (PORTB &= ~(1<<PB0))
#define DIGITS_OFF  (PORTB |= ( (1<<PB0) | (1<<PB1) | (1<<PB2) | (1<<PB3)))
#endif


/*
	__________________________________________________
	     |   segment values              |            |
	Char |dp | G | F | E | D | C | B | A | hex code   |
	-----|---|---|---|---|---|---|---|---|------------|
	  0  | 0 | 0 | 1 | 1 | 1 | 1 | 1 | 1 |    3f      |
	  1  | 0 | 0 | 0 | 0 | 0 | 1 | 1 | 0 |    06      |
	  2  | 0 | 1 | 0 | 1 | 1 | 0 | 1 | 1 |    5b      |
	  3  | 0 | 1 | 0 | 0 | 1 | 1 | 1 | 1 |    4f      |
	  4  | 0 | 1 | 1 | 0 | 0 | 1 | 1 | 0 |    66      |
	  5  | 0 | 1 | 1 | 0 | 1 | 1 | 0 | 1 |    6d      |
	  6  | 0 | 1 | 1 | 1 | 1 | 1 | 0 | 1 |    7d      |
	  7  | 0 | 0 | 0 | 0 | 0 | 1 | 1 | 1 |    07      |
	  8  | 0 | 1 | 1 | 1 | 1 | 1 | 1 | 1 |    7f      |
	  9  | 0 | 1 | 1 | 0 | 1 | 1 | 1 | 1 |    6f      |
	  A  | 0 | 1 | 1 | 1 | 0 | 1 | 1 | 1 |    77      |
	  b  | 0 | 1 | 1 | 1 | 1 | 1 | 0 | 0 |    7c      |
	  C  | 0 | 0 | 1 | 1 | 1 | 0 | 0 | 1 |    39      |
	  d  | 0 | 1 | 0 | 1 | 1 | 1 | 1 | 0 |    5e      |
	  E  | 0 | 1 | 1 | 1 | 1 | 0 | 0 | 1 |    79      |
	  F  | 0 | 1 | 1 | 1 | 0 | 0 | 0 | 1 |    71      |
	 .   | 1 | 0 | 0 | 0 | 0 | 0 | 0 | 0 |    80      |
	-----|---|---|---|---|---|---|---|---|------------|
	  a  | 0 | 1 | 0 | 1 | 1 | 1 | 1 | 1 |    5f      |
	  c  | 0 | 1 | 0 | 1 | 1 | 0 | 0 | 0 |    58      |
	  e  | 0 | 1 | 1 | 1 | 1 | 0 | 1 | 1 |    7b      |
	  G  | 0 | 1 | 1 | 1 | 1 | 1 | 0 | 1 |    7d      |
	  H  | 0 | 1 | 1 | 1 | 0 | 1 | 1 | 0 |    75      |
	  h  | 0 | 1 | 1 | 1 | 0 | 1 | 0 | 0 |    74      |
	  I  | 0 | 0 | 0 | 0 | 0 | 1 | 1 | 0 |    06      |
	  i  | 0 | 0 | 0 | 0 | 0 | 1 | 0 | 0 |    04      |
	  J  | 0 | 0 | 0 | 1 | 1 | 1 | 1 | 0 |    1e      |
	  L  | 0 | 0 | 1 | 1 | 1 | 0 | 0 | 0 |    38      |
	  l  | 0 | 0 | 1 | 1 | 0 | 0 | 0 | 0 |    30      |
	  n  | 0 | 1 | 0 | 1 | 0 | 1 | 0 | 0 |    54      |
	  o  | 0 | 1 | 0 | 1 | 1 | 1 | 0 | 0 |    5c      |
	  P  | 0 | 1 | 1 | 1 | 0 | 0 | 1 | 1 |    73      |
	  q  | 0 | 1 | 1 | 0 | 0 | 1 | 1 | 1 |    67      |
	  r  | 0 | 1 | 0 | 1 | 0 | 0 | 0 | 0 |    50      |
	  t  | 0 | 1 | 1 | 1 | 1 | 0 | 0 | 0 |    78      |
	  U  | 0 | 0 | 1 | 1 | 1 | 1 | 1 | 0 |    3e      |
	  u  | 0 | 0 | 0 | 1 | 1 | 1 | 0 | 0 |    1c      |
	  Y  | 0 | 1 | 1 | 0 | 1 | 1 | 1 | 0 |    6e      |
	  °  | 0 | 1 | 1 | 0 | 0 | 0 | 1 | 1 |    63      |
	  -  | 0 | 1 | 0 | 0 | 0 | 0 | 0 | 0 |    40      |
	  =  | 0 | 1 | 0 | 0 | 1 | 0 | 0 | 0 |    48      |
	  _  | 0 | 0 | 0 | 0 | 1 | 0 | 0 | 0 |    08      |
	 .   | 1 | 0 | 0 | 0 | 0 | 0 | 0 | 0 |    80      |
	  !  | 1 | 0 | 0 | 0 | 0 | 1 | 1 | 0 |    86      |
	  "  | 0 | 0 | 1 | 0 | 0 | 0 | 0 | 1 |    21      |
	  %  | 0 | 0 | 1 | 0 | 0 | 0 | 1 | 0 |            |
	-----*---*---*---*---*---*---*---*---*------------*
*/

// Starting from ' ', ASCII below 0x20 are non printable
unsigned char PROGMEM const ToHexDigit[128-32] = 
{
0x00, 0x86, 0x22, 0x08, 0x08, 0x08, 0x08, 0x02, //  !"#$%&'
0x39, 0x0f, 0x08, 0x46, 0x80, 0x40, 0x80, 0x08, // ()*+,-./
0x3f, 0x06, 0x5b, 0x4f, 0x66, 0x6d, 0x7d, 0x07,	//	01234567
0x7f, 0x6f, 0x08, 0x08, 0x08, 0x48, 0x08, 0x53,	//  89:;<=>?
0x08, 0x77, 0x7c, 0x39, 0x5e, 0x79, 0x71, 0x3d,	//  @ABCDEFG
0x76, 0x06, 0x0E, 0x08, 0x38, 0x54, 0x54, 0x3f,	//  HIJKLMNO
0x73, 0x67, 0x31, 0x6d, 0x78, 0x3e, 0x08, 0x08,	//  PQRSTUVW
0x08, 0x6e, 0x49, 0x39, 0x08, 0x0f, 0x01, 0x08,	//  XYZ[\]^_
0x20, 0x5f, 0x7c, 0x58, 0x5e, 0x7b, 0x71, 0x3d,	//  `abcdefg
0x74, 0x04, 0x0c, 0x08, 0x30, 0x08, 0x54, 0x5c,	//  hijklmno   
0x73, 0x67, 0x50, 0x6d, 0x78, 0x1c, 0x08, 0x08,	//  pqrstuvw
0x08, 0x6e, 0x49, 0x39, 0x30, 0x0f, 0x40, 0x5d	//  xyz{|}~?
};

#define DECIMAL_POINT 0x80

// if next char is '.' we'll fit it into the same segment.
static unsigned char CharTo7Seg(unsigned char c, unsigned char NextValue)
{
	unsigned char mask = 0x0;
	
	// add decimal point
	if( NextValue == '.' || NextValue == ',')
		mask = 0x80; // Turn on '.'
	
	if(c == '°') // Useful special character
		return mask | 0x63;	
	
	if(c == 0)
		return mask;
	
	if(c < ' ' || c > 127)
		mask |= 0x08; // Underscore for non-printables
	else 
		mask |= pgm_read_byte(&(ToHexDigit[c-' '])); 
		
	return mask;
}

static void Segments(char Value, char NextValue)
{
	// Convert Value to segments on/off	
	unsigned char c = CharTo7Seg(Value, NextValue);
#if COMMON_PIN == COMMON_ANODE
	c = ~c; // Invert output
#endif

#if NUM_DIGITS == 4
	if(c & 0x01) PORTC |= (1<<PC0); else PORTC &= ~(1<<PC0);
	if(c & 0x02) PORTC |= (1<<PC1); else PORTC &= ~(1<<PC1);
	if(c & 0x04) PORTC |= (1<<PC2); else PORTC &= ~(1<<PC2);
	if(c & 0x08) PORTC |= (1<<PC3); else PORTC &= ~(1<<PC3);
	if(c & 0x10) PORTD |= (1<<PD4); else PORTD &= ~(1<<PD4);
	if(c & 0x20) PORTD |= (1<<PD5); else PORTD &= ~(1<<PD5);
	if(c & 0x40) PORTD |= (1<<PD6); else PORTD &= ~(1<<PD6);
	if(c & 0x80) PORTD |= (1<<PD7); else PORTD &= ~(1<<PD7);
#endif
#if NUM_DIGITS == 2
	if(c & 0x01) PORTC |= (1<<PC0); else PORTC &= ~(1<<PC0);
	if(c & 0x02) PORTC |= (1<<PC1); else PORTC &= ~(1<<PC1);
	if(c & 0x04) PORTC |= (1<<PC2); else PORTC &= ~(1<<PC2);
	if(c & 0x08) PORTC |= (1<<PC3); else PORTC &= ~(1<<PC3);
	if(c & 0x10) PORTD |= (1<<PD4); else PORTD &= ~(1<<PD4);
	if(c & 0x20) PORTD |= (1<<PD5); else PORTD &= ~(1<<PD5);
	if(c & 0x40) PORTD |= (1<<PD6); else PORTD &= ~(1<<PD6);
	if(c & 0x80) PORTD |= (1<<PD7); else PORTD &= ~(1<<PD7);
#endif

}

// Turns on segments of Digit defined by Value. 
// NextValue is used to show decimal point in place, together with character.
static void ShowDigit(unsigned char Digit, char Value, char bShowPoint)
{
	DIGITS_OFF;
	
	Segments(Value, bShowPoint);
	
	switch(Digit)
	{
		case 0: DIGIT_1_ON; return;
		case 1: DIGIT_2_ON; return;
		case 2: DIGIT_3_ON; return;
		case 3: DIGIT_4_ON; return;
	}	
}

static unsigned short int Time = 0; // Time for moving a string being displayed 
static unsigned char szStringToDisplay[MAX_STRING_LEN] = {0,0,0,0,0};
static unsigned char Indicator[NUM_DIGITS<<1]; // Actual string for indicator
unsigned char idxStart = 0; // First char that we are displaying now
unsigned char bEndOfLine = 0; // Flag that we are at end of line
unsigned char NowLighting = 0; // Number of currently highlighted digit (0-3)

static void ResetDisplay()
{
	idxStart = 0;
	bEndOfLine = 0;
	Time = 0;
}

static void CopyStringToIndicator()
{
	char* p = (char*)szStringToDisplay + idxStart;
	char PrevChar = 0;
	if(idxStart > 0)
	{
		PrevChar = (char)szStringToDisplay[idxStart-1];
		if(PrevChar != '.' && PrevChar != ',')
			PrevChar = 0;
	}
	// Now copy data into Indicator[], considering decimal points
	for(unsigned char i=0; i<NUM_DIGITS; i++)
	{
		Indicator[(i<<1) + 1] = 0; // no trailing decimal point for this char
		if(bEndOfLine != 0) // Fill with spaces thru end of line
		{
			Indicator[i<<1] = 0;
			continue;
		}
			
		if(*p == 0) // EOL
			bEndOfLine = 1;
		else if(*p == '.' || *p == ',')  // this is '.'
			{
				if(PrevChar == '.' || PrevChar == ',') // ".."
					Indicator[i<<1] = Indicator[(i<<1) + 1] = '.'; // Also mark we have '.' here
				else // "F."
				{
					Indicator[i<<1] = *(p+1); // can be EOL, it is OK - will be empty
					if(*(p+1) != 0) // Is it last char in line?
						p++; // Skip '.'
				}
			}
		else if( *(p+1) == '.' || *(p+1) == ',') // "F." - next is '.'
		{
			Indicator[i<<1] = *p; // Char
			Indicator[(i<<1) + 1] = '.'; // followed by '.'
			p++; // Skip following '.'
		}	
		else  // Ordinary character
			Indicator[i<<1] = *p;	
		
		PrevChar = Indicator[(i<<1)+1]; // Remember if there was '.'
		p++;
	}
	
	// Next char after all can be EOL
	if(*p == 0)
		bEndOfLine = 1;
}

// For dynamic display, call me as often as possible 
// (but about once in millisecond).
// Tell me number of milliseconds passed from previous update
// so I could move the string being displayed.
void Update(unsigned char ms)
{
	// Turn on next digit (0, 1, 2, 3)
	ShowDigit(NowLighting>>1, Indicator[NowLighting],
			Indicator[NowLighting + 1]);

	NowLighting += 2;

	if(NowLighting >= (NUM_DIGITS * 2) )
	{
		NowLighting = 0;
	}
	
	Time += ms;
	
	unsigned short int MoveString = 0;

	// Process moving string

	// Beginning and end of line are displayed longer
	if( (bEndOfLine != 0 ) && (Time >= MOVING_END_TIME) ) // End of line
		MoveString = MOVING_END_TIME;
	else if( (idxStart == 0) && Time >= MOVING_START_TIME )// Start of line				
		MoveString = MOVING_START_TIME; // Time to show
	else if( (idxStart != 0) && (Time >= MOVING_MID_TIME) && bEndOfLine == 0 )	// Otherwise
		MoveString = MOVING_MID_TIME; // shorter time
	
	if(MoveString > 0) // Yes, we need to move string on display
	{
		Time -= MoveString;	// Avoid additive error
		
		// If we are not at EOL
		if(bEndOfLine == 0)
			idxStart++;
		else 
		{
			idxStart = 0;
			bEndOfLine = 0;
		}
		// Special case to skip starting '.' if it is from "F."
		if(idxStart > 0 && 
				(szStringToDisplay[idxStart] == '.' || szStringToDisplay[idxStart] == ',' )
				&& szStringToDisplay[idxStart - 1] != '.' 
				&& szStringToDisplay[idxStart - 1] != ',' ) 
			idxStart++;
			
		CopyStringToIndicator();		
	}
}

// Fills indicator from PROGMEM
void StringToIndicator_P(const char /*PROGMEM*/ * pStr)
{
	// Clear whole string (will have many zeros behind the string)
	memset(szStringToDisplay, 0, sizeof(szStringToDisplay));
	// Copy data, ensuring at least one trailing zero
	strncpy_P((char*)szStringToDisplay, pStr, sizeof(szStringToDisplay));
	ResetDisplay();
	CopyStringToIndicator();
	Update(0);
}

// Fills indicator from RAM
void StringToIndicator(const char * pStr)
{
	// Clear whole string (will have many zeros behind the string)
	memset(szStringToDisplay, 0, sizeof(szStringToDisplay));
	// Copy data, ensuring at least one trailing zero
	strncpy((char*)szStringToDisplay, pStr, sizeof(szStringToDisplay));
	ResetDisplay();
	memset(Indicator, 0, sizeof(Indicator));
	CopyStringToIndicator();
	Update(0);
}

// Sets up MCU I/O
void InitIndicator()
{
#if NUM_DIGITS == 4 // 4-digit dynamic display SMA420564
	DDRB = (1<<DDB0) | (1<<DDB1) | (1<<DDB2) | (1<<DDB3);
	DDRC = (1<<DDC0) | (1<<DDC1) | (1<<DDC2) | (1<<DDC3);
	DDRD = (1<<DDD4) | (1<<DDD5) | (1<<DDD6) | (1<<DDD7);
#endif
#if NUM_DIGITS == 2 // 2 * 1-digit display FJ5101BH
DDRB = (1<<DDB0) | (1<<DDB1) | (1<<DDB2) | (1<<DDB3) | (1<<DDB4) | (1<<DDB5);
DDRC = (1<<DDC0) | (1<<DDC1) | (1<<DDC2) | (1<<DDC3);
DDRD = (1<<DDD4) | (1<<DDD5) | (1<<DDD6) | (1<<DDD7);
#if USES_CRYSTAL == 1
	DDRC |= (1<<DDC4) | (1<<DDC5);
#else
	DDRB |= (1<<DDB6) | (1<<DDB7);
#endif

#endif
}
