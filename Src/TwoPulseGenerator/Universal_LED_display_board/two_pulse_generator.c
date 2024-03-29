/*
 
 Генератор двух импульсов, один запускающий, другой сдвигается относительно 
 первого вперёд-назад и его длина тоже настраивается.
 
 Контроллер - ATMega8L
  
  */ 

#define F_CPU 8000000

#include <avr/pgmspace.h>
#include <avr/eeprom.h>
#include <avr/interrupt.h>
#include <util/delay.h>
#include <stdlib.h>
#include <stdio.h>
#include "Indicator.h"
#include "Uart.h"
#include "Menu.h"

//const char PROGMEM String[] = "HELLO Github";
void Timer1Init()
{
	TCNT1 = 0;
	TCCR1A = (0<<WGM11) | (0 << WGM10); // CTC mode
	TCCR1B = (1 << WGM12) |(0 << CS12) | (1 << CS01) | (0 << CS00); // CLK/8, 1 tick/us
	OCR1A = 100; // will interrupt once in 100 us
	
	TIMSK  = (1<<OCIE1A); // Interrupt on reach OCR0A
}

volatile uint16_t ticks = 0;

volatile uint16_t ticksalways = 0;

ISR(TIMER1_COMPA_vect)
{
	static uint8_t localticks = 0;
	
	ticksalways++;
	if(ticksalways > 1000) // 100 ms
		ticksalways = 0;
	
	Update(1);
	
	if(PIND & (1<<PIND0)) // Секундомер работает только когда ему на вход дают 1
	{
		if(++localticks > 10)
		{
			ticks++;
			if(ticks > 9999)
			ticks = 0;
			localticks = 0;
		}	
	}
}

	// Loads OSCCAL value from last EEPROM byte
void LoadOsccal()
{
	OSCCAL = eeprom_read_byte((uint8_t*)E2END);
}

void DelayUs(uint16_t us)
{
	if(us & 0x8000) // Underflow, delay<0
		return;
		
	us += (us>>1);
	while(us-- > 0)
		{asm volatile ("nop");}
}

int main(void)
{
	LoadOsccal();
	
	DelayUs(10);
	DelayUs(50);
	DelayUs(200);
	
	DDRB |= (1<<DDB4) | (1<<DDB5) | (1<<DDB6) | (1<<DDB7); // выходы "открытый коллектор"
	PORTB &= ~((1<<PB4)|(1<<PB5));

	int i=0;
/*	for(i=0; i<10000; i++)
	{
		_delay_us(50);
		PORTB |= (1<<PB4) | (1<<PB6) ; // похоже, битый порт?
		_delay_us(50); // делаем импульс 5 мкс
		PORTB &= ~( (1<<PB4) | (1<<PB6) );
	}
*/

	UART_Init(51); // 51 = 9600 @ 8 MHz
	
/*	uint8_t c = 0;
	while(1)
	{
		USART_Transmit_sync(c++);
		_delay_ms(10);
	}*/
	
	InitIndicator();

	DDRB |= (1<<DDB4) | (1<<DDB5) | (1<<DDB6) | (1<<DDB7); // выходы "открытый коллектор"
	PORTB &= ~((1<<PB4)|(1<<PB5));
	
//	StringToIndicator_P(PSTR("Triggered StoPWAtch-pulsegen"));
	
	StringToIndicator_P(PSTR("TWwO Pulse generator. Uart 9600 ")); // Ww will show W in two chars

	for(i=0; i<15000; i++)
	{
		_delay_us(500);
		Update(1);
	}

	Timer1Init();
	
	sei();		
	
	UART_Putstring_P(PSTR("Two pulse generator. Uart 9600\r\nBuild: " __TIMESTAMP__ "\r\n"));

	
	uint16_t t = ticks;
	
	uint8_t WhatToChange = 0;
	uint8_t PulseLength = 10;
	int16_t Offset = 50;
	
	char s[8];
	itoa(PulseLength, s, 10);
	StringToIndicator(s); // Ww will show W in two chars

	uint8_t bPermanent = 0;
	
    while(1)
    {
		
		//if(ticksalways < 10 ) // any ticks in first 10 ms
		if(ticks - t > 500)
		{			
			cli(); // ЧТобы никто не вмешался в точную генерацию импульса
			
			//ticksalways = 10; // чтобы мы в следующий раз ввалились сбда только через 1 с
			t = ticks;
/*                               +----------------------+                                                                       
                                 |                      |                                                                       
                                 |                      |                                                                       
                                 |                      |                                                                       
              +---+  offset > 5  |                      |                                                                            
Light         |   |              |                      |                                                                         
---------------------------------+                      +--------------------------------------                                   
              |   |                                                                                                               
              |   |                                                                                                               
              |   |                                                                                                               
Trig          |   |                                                                                                               
--------------+   +---------------------------------------------------------------------------- */
#define TRIG_WIDTH 5
			if(Offset > TRIG_WIDTH)	
			{
				// trigger
				PORTB |= (1<<PB5) | (1<<PB7) ; // похоже, битый порт?
				_delay_us(TRIG_WIDTH); // делаем импульс 5 мкс
				PORTB &= ~( (1<<PB5) | (1<<PB7) );

				DelayUs(Offset-TRIG_WIDTH); 
			
				// light
				PORTB |= (1<<PB4) | (1<<PB6) ; // похоже, битый порт?
				DelayUs(PulseLength); // делаем импульс 5 мкс 
				if(bPermanent == 0)
					PORTB &= ~( (1<<PB4) | (1<<PB6) );
			}
/*                               +----------------------+                                                                       
                                 |                      |                                                                       
                                 |                      |                                                                       
                                 |                      |                                                                       
              0 < offset < 5   +---+                    |                                                                                                  
Light                          | | |                    |                                                                                              
---------------------------------+ |                    +--------------------------------------                                   
                               |   |                                                                                                               
                               |   |                                                                                                               
                               |   |                                                                                                               
Trig                           |   |                                                                                                               
-------------------------------+   +---------------------------------------------------------------------------- */
			else if(Offset > 0)	
			{
				// trigger
				PORTB |= (1<<PB5) | (1<<PB7) ; // похоже, битый порт?
				DelayUs(Offset); // делаем импульс 5 мкс

				// light
				PORTB |= (1<<PB4) | (1<<PB6) ; // похоже, битый порт?

				DelayUs(TRIG_WIDTH - Offset); // делаем остаток импульса
				PORTB &= ~( (1<<PB5) | (1<<PB7) ); // trigger off

				DelayUs(PulseLength - TRIG_WIDTH + Offset); // остаток импульса света
				
				if(bPermanent == 0)
					PORTB &= ~( (1<<PB4) | (1<<PB6) );
			}
/*                           +----------------------+                                                                       
                             |                      |                                                                       
                             |                      |                                                                       
                             |                      |                                                                       
                  offset < 0 |  o  +---+            |                                                                                                  
Light                        |     |   |            |                                                                                              
-----------------------------+     |   |            +--------------------------------------                                   
                                   |   |                                                                                                               
                                   |   |                                                                                                               
                                   |   |                                                                                                               
Trig                               |   |                                                                                                               
-----------------------------------+   +---------------------------------------------------------------------------- */
			else if(Offset > 0)	
			{
				// light
				PORTB |= (1<<PB4) | (1<<PB6) ; // похоже, битый порт?

				DelayUs(0 - Offset); // делаем остаток импульса

				// trigger
				PORTB |= (1<<PB5) | (1<<PB7) ; // похоже, битый порт?
				DelayUs(TRIG_WIDTH); // делаем импульс 5 мкс

				PORTB &= ~( (1<<PB5) | (1<<PB7) ); // trigger off

				DelayUs(PulseLength - TRIG_WIDTH + Offset); // остаток импульса света
				
				if(bPermanent == 0)
					PORTB &= ~( (1<<PB4) | (1<<PB6) );
			}
			
			sei();
		}
		
/*		if(t != ticks)
		{
			t = ticks;
			
			char str[10];
			sprintf(str, "%04d", ticks);
			StringToIndicator(str);			
		}*/
		
		
		if(WhatToChange == 'L')
		{
			s[0]='L'; s[1] = ' ';
			itoa(PulseLength, s+1, 10);
			StringToIndicator(s); // Ww will show W in two chars
		}
		else if(WhatToChange == 'O')
		{
			s[0]='o'; s[1] = ' ';
			itoa(Offset, s+1, 10);
			StringToIndicator(s); 
		}
		else StringToIndicator_P(PSTR("L-O")); 
		
		if(UART_rxready() > 0)
		{
			char c = UART_recv_char();
			switch(c)
			{
				case 'P': bPermanent = 1; break;
				case 'p': bPermanent = 0; break;
				case 'L': case 'l': 
					WhatToChange = 'L'; 
					UART_Putstring_P(PSTR("Length = "));
					UART_Putstring_itoa(PulseLength);
					UART_Putstring_P(PSTR("\r\n"));
					break;
				case 'O': case 'o':
					WhatToChange = 'O';
					UART_Putstring_P(PSTR("Offset = "));
					UART_Putstring_itoa(Offset);
					UART_Putstring_P(PSTR("\r\n"));
					break;
				case '+': 
					if(WhatToChange == 'L')
					{
						PulseLength++;
						UART_Putstring_P(PSTR("Length = "));
						UART_Putstring_itoa(PulseLength);
						UART_Putstring_P(PSTR("\r\n"));
					}
					if(WhatToChange == 'O')
					{
						Offset++;
						UART_Putstring_P(PSTR("Offset = "));
						UART_Putstring_itoa(Offset);
						UART_Putstring_P(PSTR("\r\n"));
					}
					break;
				case '-':
					if(WhatToChange == 'L' && PulseLength > 5)
					{
						PulseLength--;
						UART_Putstring_P(PSTR("Length = "));
						UART_Putstring_itoa(PulseLength);
						UART_Putstring_P(PSTR("\r\n"));
					}
					if(WhatToChange == 'O')
					{
						Offset--;
						UART_Putstring_P(PSTR("Offset = "));
						UART_Putstring_itoa(Offset);
						UART_Putstring_P(PSTR("\r\n"));
					}
					break;
				default: UART_Putstring_P(PSTR("L - set pulse length, O - set offset from trigger to LED pulse, +- - change selected value, Pp - LED permanent on/off\r\n")); break;
			}
			
		}
		
//		Menu();
    }
}
