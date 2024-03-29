#include <avr/io.h>
#include <avr/interrupt.h>
#include <avr/pgmspace.h>
#include <avr/wdt.h>
#include <string.h>
#include <stdlib.h>

/*
Code for ATMega8. For 88/168 replace UDR -> UDR0, UCSRB -> UCSR0B etc
*/


#include "Uart.h"

/*
/ * UART Buffer Defines * /
#define UART_RX_BUFFER_SIZE 64      / *  2,4,8,16,32,64,128 or 256 bytes * /
#define UART_TX_BUFFER_SIZE 128


#define UART_RX_BUFFER_MASK ( UART_RX_BUFFER_SIZE - 1 )
#if ( UART_RX_BUFFER_SIZE & UART_RX_BUFFER_MASK )
	#error RX buffer size is not a power of 2
#endif

#define UART_TX_BUFFER_MASK ( UART_TX_BUFFER_SIZE - 1 )
#if ( UART_TX_BUFFER_SIZE & UART_TX_BUFFER_MASK )
	#error TX buffer size is not a power of 2
#endif


/ * Static Variables * /
static unsigned char UART_RxBuf[UART_RX_BUFFER_SIZE];
static volatile unsigned char UART_RxHead;
static volatile unsigned char UART_RxTail;
static unsigned char UART_TxBuf[UART_TX_BUFFER_SIZE];
static volatile unsigned char UART_TxHead;
static volatile unsigned char UART_TxTail;
*/


char RxBuf[UART_BUF_LEN];
volatile unsigned char PosWRxBuf = 0;
volatile unsigned char PosRRxBuf = 0;
volatile unsigned char bOverflow = 0;

char TxBuf[TX_UART_BUF_LEN];
volatile unsigned char PosWTxBuf = 0;
volatile unsigned char PosRTxBuf = 0;


/* Initialize UART */
void UART_Init( unsigned char ubrrl )
{
	DDRD |= (1<<DDD1); // TX pin = out

// ATMega8
	UBRRH = 0;
	UBRRL = ubrrl; 	/* Set the baud rate */
	/* Enable UART receiver and transmitter, and receive interrupt */
//	UCSRB = ( (1<<RXCIE) | (1<<RXEN) | (1<<TXEN) );
	UCSRB = (1<<RXCIE) | (1<<TXCIE) | /*(1<<UDRIE0) |*/ (1<<RXEN) | (1<<TXEN) ;

	UCSRC = (1<<URSEL) | (1<<UCSZ1) | (1<<UCSZ0); 
//	UCSRC = (1<<UCSZ1) | (1<<UCSZ0); // 8-bit FOR MEGA88!

	PosWRxBuf = 0;
	PosRRxBuf = 0;
	PosWRxBuf = 0;
	PosWTxBuf = 0;
}

uint8_t USART_Bytes_in_Tx_buffer()
{
	unsigned char w,r;
	cli();
	w = PosWTxBuf;
	r = PosRTxBuf;
	sei();
	
	if(w == r)
	return 0;
	
	if(w > r)
	return w - r;
	else return w + TX_UART_BUF_LEN - r;
}

void USART_Transmit_sync( unsigned char data )
{
	/* Wait for empty transmit buffer */
	while ( !( UCSRA & (1<<UDRE)) );
	/* Put data into buffer, sends the data */
	UDR = data;
}

void USART_Transmit_async( unsigned char data )
{
	if( (UCSRB & (1<<UDRIE) ) == 0)  // Если мы не передаём сейчас, прерывание запрещено
	//	if ( ( UCSR0A & (1<<UDRE0)) ) // если буфер пустой
	{
		cli();
		UDR = data;
		UART_UDREMPTYINT_ENABLE;
		sei();
	}
	else  // если не пустой, кладём в очередь.
	{
		cli();
		TxBuf[PosWTxBuf] = data;
		PosWTxBuf++;
		if(PosWTxBuf >= TX_UART_BUF_LEN)
		PosWTxBuf = 0;
		sei();
	}
}


void UART_Putstring_P (const char *stringg)	/* Puts a String to UART */
{
	uint8_t* p = (uint8_t*)stringg;
	char c = pgm_read_byte(p);
	p++;
	
	while(c != 0) 	/* go through string */
	{
		USART_Transmit_async(c);
		c = pgm_read_byte(p);
		p++;
	}
}

void UART_Putstring_P_sync (const char *stringg)	/* Puts a String to UART */
{
	uint8_t* p = (uint8_t*)stringg;
	char c = pgm_read_byte(p);
	p++;
	
	while(c != 0) 	/* go through string */
	{
		USART_Transmit_sync(c);
		c = pgm_read_byte(p);
		p++;
		
		wdt_reset();
	}
}


void UART_Putstring (char *stringg)	/* Puts a String to UART */
{
	int i = 0;
	while(stringg[i]) 	/* go through string */
	{
		
		USART_Transmit_async(stringg[i]);
		i++;
	}
}

void UART_Putstring_itoa (unsigned long value)	/* Puts a String to UART */
{
	char s[16];
	itoa(value, s, 10);
	UART_Putstring(s);
}

void UART_Putstring_ftoa (float value)	/* Puts a String to UART */
{
	char s[16];
	int16_t i = (int16_t)value;
	itoa(i, s, 10);
	UART_Putstring(s);
	UART_Putstring(".");
	value -= i;
	value *= 1000;
	if(value < 0) value = -value;
	if(value < 100)
	UART_Putstring("0");
	if(value < 10)
	UART_Putstring("0");
	itoa(value, s, 10);
	UART_Putstring(s);
}

void UART_Putstring_htoa (unsigned long value)	/* Puts a String to UART */
{
	char s[16];
	itoa(value, s, 16);
	UART_Putstring(s);
}

// Returns number of bytes in RX buffer
unsigned char UART_rxready(void)
{
	if(PosWRxBuf >= PosRRxBuf)
	return (PosWRxBuf-PosRRxBuf);
	else
	return (PosWRxBuf - PosRRxBuf + sizeof(RxBuf) );
}

unsigned char  UART_recv(unsigned char * buf, unsigned char len)
{
	unsigned char n = UART_rxready();
	unsigned char i;
	if(len < n) n = len;
	for(i=0; i<n; i++)
	{
		//		DBG_PRINT_H(*r_ptr);
		*buf++ = RxBuf[PosRRxBuf++];
		if(PosRRxBuf >= sizeof(RxBuf) )
		PosRRxBuf = 0;
	}
	return n;
}

unsigned char  UART_recv_char()
{
	unsigned char n = UART_rxready();
	
	if(n == 0)
	return 0;
	
	n = RxBuf[PosRRxBuf++];
	if(PosRRxBuf >= sizeof(RxBuf) )
	PosRRxBuf = 0;

	return n;
}


ISR(USART_RXC_vect)
{
	//UART_data_handler(UDR0);
	RxBuf[PosWRxBuf++] = UDR;
	if(PosWRxBuf >= UART_BUF_LEN)
	PosWRxBuf = 0;
	if(PosWRxBuf == PosRRxBuf)
	bOverflow = 1;
	
	//UDR0 = PosWRxBuf + '0'; //DELME
}


ISR(USART_TXC_vect) // TX complete, pushed out
{
}

// UART_UDREMPTYINT_DISABLE UART_UDREMPTYINT_ENABLE
ISR(USART_UDRE_vect) // UDR empty, needs more data
{
	if(PosWTxBuf != PosRTxBuf)
	{
		UDR = TxBuf[PosRTxBuf];
		PosRTxBuf++;
		if(PosRTxBuf >= TX_UART_BUF_LEN)
		PosRTxBuf = 0;
	}

	if(PosWTxBuf == PosRTxBuf)
	{
		// Мы закончили передавать, всё ушло
		UART_UDREMPTYINT_DISABLE;
	}
}















/* Interrupt handlers * /
//#pragma vector=UART_RX_vect
//__interrupt void UART_RX_interrupt( void )
//ISR(SIG_UART_RECV)
ISR(USART_RXC_vect)
{
	unsigned char data;
	unsigned char tmphead;

	data = UDR;                 / * Read the received data * /
	/ * Calculate buffer index * /
	tmphead = ( UART_RxHead + 1 ) & UART_RX_BUFFER_MASK;
	UART_RxHead = tmphead;      / * Store new index * /

	if ( tmphead == UART_RxTail )
	{
		 / * ERROR! Receive buffer overflow * /
	}
	
	UART_RxBuf[tmphead] = data; / * Store received data in buffer * /
}

//#pragma vector=UART_UDRE_vect
//__interrupt void UART_TX_interrupt( void )
ISR(USART_TXC_vect)
{
	unsigned char tmptail;

	/ * Check if all data is transmitted * /
	if ( UART_TxHead != UART_TxTail )
	{
		/ * Calculate buffer index * /
		tmptail = ( UART_TxTail + 1 ) & UART_TX_BUFFER_MASK;
		UART_TxTail = tmptail;      / * Store new index * /
	
		UDR = UART_TxBuf[tmptail];  / * Start transmition * /
	}
	else
	{
		UCSRB &= ~(1<<UDRIE);          / * Disable UDRE interrupt * /
	}
}

/ * Read and write functions * /
unsigned char UART_ReceiveByte( void )
{
	unsigned char tmptail;
	
	while ( UART_RxHead == UART_RxTail )  / * Wait for incomming data * /
		;
	tmptail = ( UART_RxTail + 1 ) & UART_RX_BUFFER_MASK;/ * Calculate buffer index * /
	
	UART_RxTail = tmptail;                / * Store new index * /
	
	return UART_RxBuf[tmptail];           / * Return data * /
}

void UART_TransmitByte( unsigned char data )
{
	unsigned char tmphead;
	/ * Calculate buffer index * /
	tmphead = ( UART_TxHead + 1 ) & UART_TX_BUFFER_MASK; / * Wait for free space in buffer * /
	while ( tmphead == UART_TxTail );

	UART_TxBuf[tmphead] = data;           / * Store data in buffer * /
	UART_TxHead = tmphead;                / * Store new index * /

	UCSRB |= (1<<UDRIE);                    / * Enable UDRE interrupt * /
}

unsigned char UART_DataInReceiveBuffer( void )
{
	return ( UART_RxHead != UART_RxTail ); / * Return 0 (FALSE) if the receive buffer is empty * /
}

void UART_puts( const char *s )
{
	while ( *s ) {
		UART_TransmitByte( *s++ );
	}
}

void UART_puts_p( const char *prg_s )
{
	char c;

	while ( ( c = pgm_read_byte( prg_s++ ) ) ) {
		UART_TransmitByte(c);
	}
}
*/