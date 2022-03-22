/*
 * UART.h
 *
 * Created: 18.01.2019 17:05:29
 *  Author: -
 */ 

////////////////////////////////////    UART   /////////////////////////////////////////////////////
#define UART_BUF_LEN 32     // Must be < 256!
#define TX_UART_BUF_LEN 255 // Must be < 256!

extern char RxBuf[UART_BUF_LEN];
extern volatile unsigned char PosWRxBuf;
extern volatile unsigned char PosRRxBuf;
extern volatile unsigned char bOverflow;

extern char TxBuf[TX_UART_BUF_LEN];
extern volatile unsigned char PosWTxBuf;
extern volatile unsigned char PosRTxBuf;

#define UART_UDREMPTYINT_ENABLE 	(UCSRB |= (1<<UDRIE))
#define UART_UDREMPTYINT_DISABLE 	(UCSRB &= ~(1<<UDRIE))


void UART_Init(unsigned char ubrrl);

//void USART_Transmit( unsigned char data );
void USART_Transmit_async( unsigned char data );
void USART_Transmit_sync( unsigned char data );

uint8_t USART_Bytes_in_Tx_buffer();

void UART_Putstring_P (const char *stringg);	/* Puts a String to UART */
void UART_Putstring_P_sync (const char *stringg);	/* Puts a String to UART - FOR LONG STRINGS, sync sendout*/


void UART_Putstring (char *stringg);	/* Puts a String to UART */

void UART_Putstring_itoa (unsigned long value);	/* Puts a String to UART */

void UART_Putstring_ftoa (float value);	/* Puts a String to UART */

void UART_Putstring_htoa (unsigned long value);	/* Puts a String to UART */


unsigned char UART_rxready(void);

unsigned char  UART_recv(unsigned char * buf, unsigned char len);

unsigned char  UART_recv_char();


