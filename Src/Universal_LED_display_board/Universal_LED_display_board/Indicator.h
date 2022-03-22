/*
 * Indicator.h
 *
 * Created: 12.12.2016 17:53:17
 *  Author: https://github.com/dkorobkov
 
 Code is for ATMega8 and pin compatible devices.
 
 For SMA420564 (4*7-seg, common cathode):
 You need to drive one cathode and 0 to 8 anodes to turn on a single digit.
 Current flows from ATMega pin through LED to another pin(s).
 +-----+-------+-----------+------------+
 | LED | Descr |  MCU pin  | Level for  |
 | pin |       | (ATMega8) | lighting   |
 +-----+-------+-----------+------------+
 | 1   |  e    |  PD4   2  |     1      |
 | 2   |  d    |  PC3   26 |     1      |
 | 3   |  .    |  PD7   11 |     1      |
 | 4   |  c    |  PC2   25 |     1      |
 | 5   |  g    |  PD6   10 |     1      |
 | 6   |  C4   |  PB0   12 |     1      |
 | 7   |  b    |  PC1   24 |     1      |
 | 8   |  C3   |  PB1   13 |     1      |
 | 9   |  C2   |  PB2   14 |     1      |
 | 10  |  f    |  PD5   9  |     1      |
 | 11  |  a    |  PC0   23 |     1      |
 | 12  |  C1   |  PB3   15 |     1      |
 +-----+-------+-----------+------------+
 
 For 2*FJ5101BH (7-seg, common anode):
 All segments are driven simultaneously (16 MCU outputs are used)
 
 +-----+-------+-----------+------------+
 | LED1| Descr |  MCU pin  | Level for  |
 | pin |       | (ATMega8) | lighting   |
 +-----+-------+-----------+------------+
 | 1   |  e    |  PD4   2  |     0      |
 | 2   |  d    |  PC3   26 |     0      |
 | 3   | Anode |  +5V      |     1      |
 | 4   |  c    |  PC2   25 |     0      |
 | 5   |  dp   |  PD7   11 |     0      |
 | 6   |  b    |  PC1   24 |     0      |
 | 7   |  a    |  PC0   23 |     0      |
 | 8   | Anode |  +5V      |     1      |
 | 9   |  f    |  PD5   9  |     0      |
 | 10  |  g    |  PD6   10 |     0      |
 +-----+-------+-----------+------------+
 +-----+-------+-----------+------------+
 | LED2| Descr |  MCU pin  | Level for  |
 | pin |       | (ATMega8) | lighting   |
 +-----+-------+-----------+------------+
 | 1   |  e    |  PB4   16 |     0      |
 | 2   |  d    |  PB3   15 |     0      |
 | 3   | Anode |  +5V      |     1      |
 | 4   |  c    |  PB2   14 |     0      |
 | 5   |  dp   |  PB7   8  |     0      | USES_CRYSTAL=0
 | 5   |  dp   |  PC5   28 |     0      | USES_CRYSTAL=1
 | 6   |  b    |  PB1   13 |     0      |
 | 7   |  a    |  PB0   12 |     0      |
 | 8   | Anode |  +5V      |     1      |
 | 9   |  f    |  PB5   17 |     0      |
 | 10  |  g    |  PB6   7  |     0      | USES_CRYSTAL=0
 | 10  |  g    |  PC4   27 |     0      | USES_CRYSTAL=1
 +-----+-------+-----------+------------+
 
 This code was tested with:
  NUM_DIGITS=4 and COMMON_PIN=COMMON_CATHODE
  NUM_DIGITS=2 and COMMON_PIN=COMMON_ANODE and USES_CRYSTAL=0
 
 */ 


#ifndef INDICATOR_H_
#define INDICATOR_H_
#define COMMON_CATHODE 0
#define COMMON_ANODE 1

// User settable defines.

// Set type of your indicator.
#define COMMON_PIN COMMON_ANODE

// Set number of digits on your indicator. Can be 2 or 4
// 4 is for SMA420564 and similar.
// 2 is for 2*FJ5101BH and similar.
#define NUM_DIGITS 2

// If 1-digit LEDs are installed there are two options. 
// If you need crystal (i.e. Atmel's internal calibrated RC oscillator
// is not sufficient) you will short R30 and R31 "w/xtal". If you need SDA and SCL
// you will have to work __without__ crystal and short R17, R18 "no xtal".
#if NUM_DIGITS == 2
	#define USES_CRYSTAL 0 // LED connected to PB6, PB7
//	#define USES_CRYSTAL 1 // LED connected to PC4, PC5
#endif

// Set maximum string length for display. Affects free RAM.
// (Of course, for some applications you will want to use PROGMEM
// for string storage and display, rewriting the code is up to you)
#define MAX_STRING_LEN 32

// When displaying moving string the constants below are used.
#define MOVING_START_TIME 1500 // Show beginning of string during this milliseconds
#define MOVING_MID_TIME 500    // Milliseconds between 1-char moves
#define MOVING_END_TIME	1500   // Show end of string during this milliseconds

// *************** No user settable variables below this line *****************

// For dynamic display, call me as often as possible
// (but about once in millisecond).
// Tell me number of milliseconds passed from previous update
// so I could move the string being displayed.
void Update(unsigned char ms);

// Fills indicator from PROGMEM
void StringToIndicator_P(const char /*PROGMEM*/ * pStr);

// Fills indicator from RAM
void StringToIndicator(const char * pStr);

// Sets up MCU I/O. Call before usage.
void InitIndicator();




#endif /* INDICATOR_H_ */