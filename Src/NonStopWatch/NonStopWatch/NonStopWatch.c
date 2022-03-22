/*
 * NonStopWatch.c
 *
 * Created: 17.01.2017 11:46:30
 *  Author: -
 */ 

#define F_CPU 8000000

#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/delay.h>

#include "Indicator.h"

#define enable()		{asm("sei");}
#define disable()		{asm("cli");}

volatile char text[5] = "0000";

ISR(TIMER0_OVF_vect)
{
	TCNT0 += (256-125);

	text[3]++;
	if(text[3] > '9')
	{
		text[3] = '0';
		text[2]++;
	}
	if(text[2] > '9')
	{
		text[2] = '0';
		text[1]++;
	}
	if(text[1] > '9')
	{
		text[1] = '0';
		text[0]++;
	}
	if(text[0] > '9')
		text[0] = '0';
}


int main(void)
{
	// Setup timer

	TCCR0 =  (1<<CS01) | (1<<CS00) ;// 011: CK/64, 8 MHz -> 125 kHz

	TIMSK |= 1<<TOIE0;

	InitIndicator();
	enable();

	char PrevMs = text[3];
    while(1)
    {
		// Update string only on change
		if(PrevMs != text[3])
		{
			PrevMs = text[3];
			StringToIndicator((const char*)text);
		}
        Update(1); // We do not need moving the string
		_delay_us(60); // Some light from LED is needed
    }
}