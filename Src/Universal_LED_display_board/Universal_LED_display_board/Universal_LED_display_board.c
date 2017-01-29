/*
 * Universal_LED_display_board.c
 *
 * Created: 07.12.2016 22:03:44
 *  Author: github.com/dkorobkov
 
 An example C code for Universal LED display board 
 (Gerber available at github.com/dkorobkov).
 Works with various 7-segment LEDs.
 
  */ 

#define F_CPU 8000000

#include <avr/pgmspace.h>
#include <util/delay.h>
#include "Indicator.h"


const char PROGMEM String[] = "HELLO Github";

int main(void)
{
	InitIndicator();
	
	StringToIndicator_P(String);
	
	unsigned short int count = 0;	
	
    while(1)
    {
		Update(1);
		_delay_ms(1);
		count++;

		// Example of loading string from PROGMEM
		if(count == 20000)
			StringToIndicator_P(PSTR("-Uniboard-"));
		
		// Example of loading string from RAM
		if(count == 30000)
			StringToIndicator(" Hi");
		if(count == 40000)
		{
			// Another example of loading string from PROGMEM
			StringToIndicator_P(String);
			count = 0;
		}
    }
}