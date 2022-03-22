#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>
#include <avr/io.h>
#include <avr/pgmspace.h>
//#include "f_cpu.h"
#include <util/delay.h>

//#include "Serial.h"
#include "Uart.h"

//#include "Globals.h"
#include "Menu.h"
/*
Возвращаемые значения:
 0: ничего не нажато
 1: что-то нажато
 2: работа с меню окончена
 */

unsigned char CurrMenuItem = 0;			// текущий выбранный пункт меню
unsigned char InputType = TYPE_NONE;	// что мы сейчас вводим
void* pInputValue = NULL;				// куда вводим
unsigned char bEditingItem = 0;			// сейчас редактируем?
unsigned char szInputString[MAX_INPUT+1];
unsigned char InputPos;					// Индекс ввода в массив

// test variables
/*int Value1 = 0;
int Value11 = 0;
int Value12 = 0;
int Value2 = 0;
int Value3 = 0;
char String1[20];*/

// Декларация функций для вызовов типа TYPE_EXEC. Должны быть void fn(void).
void DisplaySettings(void);
extern void WriteSettingsToEEPROM(void);

typedef struct MENUITEM
{
	unsigned char		Item;
	unsigned char		DataType;	// INPUT_TYPES или номер пункта меню для перехода при выборе
	unsigned char		Key;		// клавиша для выбора этого пункта меню
	PGM_P				pText;		// текст, который показать
	PGM_P				VarName;	// имя этой переменной или настройки
	void*				pData;		// указатель на место, куда класть
	unsigned char		nBytes;		// Длина строки
	struct MENUITEM*	pParent;	
	struct MENUITEM*	pNextSibling;
	struct MENUITEM*	pFirstChild;
} MENUITEM; 

/*

ROOT--Main menu--Set values 11,12--Set value 11
                |                 +Set value 12
                +Set value 2                               
                +Set value 3                               
                +Set string 1                               
*/

//const char szpText0[] PROGMEM = "Main menu";
//const char szpText1[] PROGMEM = "Set values 11,12";
//const char szpText2[] PROGMEM = "Set bool value 2";
//const char szpText3[] PROGMEM = "Set int value 3";
//const char szpText4[] PROGMEM = "Check settings";
//const char szpText5[] PROGMEM = "Set string 1";
//const char szpText6[] PROGMEM = "Set bool value 11";
//const char szpText7[] PROGMEM = "Set int value 12";
//const char szpText8[] PROGMEM = "Exit program";

// const char szpVarName0[] PROGMEM = "";
// const char szpVarName1[] PROGMEM = "";
// const char szpVarName2[] PROGMEM = "Val2";
// const char szpVarName3[] PROGMEM = "Val3";
// const char szpVarName4[] PROGMEM = "";
// const char szpVarName5[] PROGMEM = "Str1";
// const char szpVarName6[] PROGMEM = "Val11";
// const char szpVarName7[] PROGMEM = "Val12";
// const char szpVarName8[] PROGMEM = "";

// MENUITEM MenuItem[] = 
// {
// //idx  Item DataType		Key	     pText		Varname		pData		nBytes					pParent;		pNextSibling;	pFirstChild;
// /* 0*/	{1,	TYPE_MENU,		0,   szpText0,	szpVarName0,	NULL,		0,						NULL,			NULL,			&MenuItem[1]	},
// /* 1*/	{2,	TYPE_SUBMENU,	'1', szpText1,	szpVarName1,	NULL,		0,						&MenuItem[0],	&MenuItem[2],	&MenuItem[6]	},
// /* 2*/	{3,	TYPE_BOOL,		'2', szpText2,	szpVarName2,	&Value2,	0,						&MenuItem[0],	&MenuItem[3],   NULL            },
// /* 3*/  {4, TYPE_INT,       '3', szpText3,	szpVarName3,	&Value3,    0,                      &MenuItem[0],   &MenuItem[4],   NULL            },
// /* 4*/	{5,	TYPE_EXEC,		'C', szpText4,	szpVarName4,	DisplaySettings,0,					&MenuItem[0],   &MenuItem[5],   NULL            },
// /* 5*/	{6,	TYPE_STRING,	'S', szpText5,	szpVarName5,	String1,	sizeof(String1) - 1,    &MenuItem[0],   NULL,           NULL            },
// /* 6*/	{7,	TYPE_BOOL,		'1', szpText6,	szpVarName6,	&Value11,	0,                      &MenuItem[1],   &MenuItem[7],   NULL            },
// /* 7*/	{8,	TYPE_INT,		'2', szpText7,	szpVarName7,	&Value12,	0,                      &MenuItem[1],   NULL,			NULL            },
// /* 8*/	{9,	TYPE_INT,		'X', szpText8,	szpVarName8,	NULL,		0,                      &MenuItem[1],   NULL,			NULL            },
// /* 9*/	{0,	TYPE_NONE,		0,		NULL,		NULL,		NULL,		0,						NULL,			NULL,			NULL} // Последний в списке
// };


/*

ROOT--Main menu-+-Set delay after PPS-+-Set delay for ch0
                |                    +-Set delay for ch1
                |                    +-...
                |                    +-Set delay for ch7
				+-Set period-+-Set period for ch0
                |            +-Set period for ch1
                |            +-...
                |            +-Set period for ch7
				+-Set pulse length-+-Set pulse length for ch0
				|                  +-Set pulse length for ch1
				|                  +-...
				|                  +-Set pulse length for ch7
                +Invert all outputs                               
                +Check settings                               
*/

const char szpText0[] PROGMEM = "Main menu";
const char szpText1[] PROGMEM = "Set (D)elay after PPS, us";
const char szpText10[] PROGMEM = "Delay for ch0";
const char szpText11[] PROGMEM = "Delay for ch1";
const char szpText12[] PROGMEM = "Delay for ch2";
const char szpText13[] PROGMEM = "Delay for ch3";
const char szpText14[] PROGMEM = "Delay for ch4";
const char szpText15[] PROGMEM = "Delay for ch5";
const char szpText16[] PROGMEM = "Delay for ch6";
const char szpText17[] PROGMEM = "Delay for ch7";
const char szpText2[] PROGMEM = "Set (P)eriod, us";
const char szpText20[] PROGMEM = "Period for ch0";
const char szpText21[] PROGMEM = "Period for ch1";
const char szpText22[] PROGMEM = "Period for ch2";
const char szpText23[] PROGMEM = "Period for ch3";
const char szpText24[] PROGMEM = "Period for ch4";
const char szpText25[] PROGMEM = "Period for ch5";
const char szpText26[] PROGMEM = "Period for ch6";
const char szpText27[] PROGMEM = "Period for ch7";
const char szpText3[] PROGMEM = "Set pulse (L)ength, us";
const char szpText30[] PROGMEM = "Pulse length for ch0";
const char szpText31[] PROGMEM = "Pulse length for ch1";
const char szpText32[] PROGMEM = "Pulse length for ch2";
const char szpText33[] PROGMEM = "Pulse length for ch3";
const char szpText34[] PROGMEM = "Pulse length for ch4";
const char szpText35[] PROGMEM = "Pulse length for ch5";
const char szpText36[] PROGMEM = "Pulse length for ch6";
const char szpText37[] PROGMEM = "Pulse length for ch7";
const char szpText4[] PROGMEM = "(W)rite settings to EEPROM";
const char szpText5[] PROGMEM = "(C)heck settings";

const char szpVarNameEmpty[] PROGMEM = "";
const char szpVarName10[] PROGMEM = "Delay[0], us";
const char szpVarName11[] PROGMEM = "Delay[1], us";
const char szpVarName12[] PROGMEM = "Delay[2], us";
const char szpVarName13[] PROGMEM = "Delay[3], us";
const char szpVarName14[] PROGMEM = "Delay[4], us";
const char szpVarName15[] PROGMEM = "Delay[5], us";
const char szpVarName16[] PROGMEM = "Delay[6], us";
const char szpVarName17[] PROGMEM = "Delay[7], us";
const char szpVarName20[] PROGMEM = "Period[0], us";
const char szpVarName21[] PROGMEM = "Period[1], us";
const char szpVarName22[] PROGMEM = "Period[2], us";
const char szpVarName23[] PROGMEM = "Period[3], us";
const char szpVarName24[] PROGMEM = "Period[4], us";
const char szpVarName25[] PROGMEM = "Period[5], us";
const char szpVarName26[] PROGMEM = "Period[6], us";
const char szpVarName27[] PROGMEM = "Period[7], us";
const char szpVarName30[] PROGMEM = "Pulse length[0], us";
const char szpVarName31[] PROGMEM = "Pulse length[1], us";
const char szpVarName32[] PROGMEM = "Pulse length[2], us";
const char szpVarName33[] PROGMEM = "Pulse length[3], us";
const char szpVarName34[] PROGMEM = "Pulse length[4], us";
const char szpVarName35[] PROGMEM = "Pulse length[5], us";
const char szpVarName36[] PROGMEM = "Pulse length[6], us";
const char szpVarName37[] PROGMEM = "Pulse length[7], us";

MENUITEM MenuItem[] =
{
//idx  Item		DataType		Key	     pText		Varname			pData								nBytes	pParent;		pNextSibling;	pFirstChild;
/* 0*/	{1,		TYPE_MENU,		0,   szpText0,	szpVarNameEmpty,	NULL,								0,		NULL,			NULL,			&MenuItem[1]	},
/* 1*/	{2,		TYPE_SUBMENU,	'D', szpText1,	szpVarNameEmpty,	NULL,								0,		&MenuItem[0],	&MenuItem[2],	&MenuItem[6]	},
/* 2*/	{3,		TYPE_SUBMENU,	'P', szpText2,	szpVarNameEmpty,	NULL,								0,		&MenuItem[0],	&MenuItem[3],	&MenuItem[14]	},
/* 3*/	{4,		TYPE_SUBMENU,	'L', szpText3,	szpVarNameEmpty,	NULL,								0,		&MenuItem[0],	&MenuItem[4],	&MenuItem[22]	},
/* 4*/	{5,		TYPE_EXEC,		'W', szpText4,	szpVarNameEmpty,	WriteSettingsToEEPROM,				0,		&MenuItem[0],	&MenuItem[5],   NULL            },
/* 5*/	{6,		TYPE_EXEC,		'C', szpText5,	szpVarNameEmpty,	DisplaySettings,					0,		&MenuItem[0],   NULL,			NULL            },

/* 6*/	{7,		TYPE_INT32,		'0', szpText10,	szpVarName10,		(void*)&Outputs[0].DelayFromPps,	0,      &MenuItem[1],   &MenuItem[7],	NULL            },
/* 7*/	{8,		TYPE_INT32,		'1', szpText11,	szpVarName11,		(void*)&Outputs[1].DelayFromPps,	0,      &MenuItem[1],   &MenuItem[8],	NULL            },
/* 8*/	{9,		TYPE_INT32,		'2', szpText12,	szpVarName12,		(void*)&Outputs[2].DelayFromPps,	0,      &MenuItem[1],   &MenuItem[9],	NULL            },
/* 9*/	{10,	TYPE_INT32,		'3', szpText13,	szpVarName13,		(void*)&Outputs[3].DelayFromPps,	0,      &MenuItem[1],   &MenuItem[10],	NULL            },
/*10*/	{11,	TYPE_INT32,		'4', szpText14,	szpVarName14,		(void*)&Outputs[4].DelayFromPps,	0,      &MenuItem[1],   &MenuItem[11],	NULL            },
/*11*/	{12,	TYPE_INT32,		'5', szpText15,	szpVarName15,		(void*)&Outputs[5].DelayFromPps,	0,      &MenuItem[1],   &MenuItem[12],	NULL            },
/*12*/	{13,	TYPE_INT32,		'6', szpText16,	szpVarName16,		(void*)&Outputs[6].DelayFromPps,	0,      &MenuItem[1],   &MenuItem[13],	NULL            },
/*13*/	{14,	TYPE_INT32,		'7', szpText17,	szpVarName17,		(void*)&Outputs[7].DelayFromPps,	0,      &MenuItem[1],   NULL,			NULL            },

/*14*/	{15,	TYPE_INT32,		'0', szpText20,	szpVarName20,		(void*)&Outputs[0].Period,			0,      &MenuItem[2],   &MenuItem[15],	NULL            },
/*15*/	{16,	TYPE_INT32,		'1', szpText21,	szpVarName21,		(void*)&Outputs[1].Period,			0,      &MenuItem[2],   &MenuItem[16],	NULL            },
/*16*/	{17,	TYPE_INT32,		'2', szpText22,	szpVarName22,		(void*)&Outputs[2].Period,			0,      &MenuItem[2],   &MenuItem[17],	NULL            },
/*17*/	{18,	TYPE_INT32,		'3', szpText23,	szpVarName23,		(void*)&Outputs[3].Period,			0,      &MenuItem[2],   &MenuItem[18],	NULL            },
/*18*/	{19,	TYPE_INT32,		'4', szpText24,	szpVarName24,		(void*)&Outputs[4].Period,			0,      &MenuItem[2],   &MenuItem[19],	NULL            },
/*19*/	{20,	TYPE_INT32,		'5', szpText25,	szpVarName25,		(void*)&Outputs[5].Period,			0,      &MenuItem[2],   &MenuItem[20],	NULL            },
/*20*/	{21,	TYPE_INT32,		'6', szpText26,	szpVarName26,		(void*)&Outputs[6].Period,			0,      &MenuItem[2],   &MenuItem[21],	NULL            },
/*21*/	{22,	TYPE_INT32,		'7', szpText27,	szpVarName27,		(void*)&Outputs[7].Period,			0,      &MenuItem[2],   NULL,			NULL            },

/*22*/	{23,	TYPE_INT32,		'0', szpText30,	szpVarName30,		(void*)&Outputs[0].PulseLen,		0,		&MenuItem[3],   &MenuItem[23],	NULL            },
/*23*/	{24,	TYPE_INT32,		'1', szpText31,	szpVarName31,		(void*)&Outputs[1].PulseLen,		0,      &MenuItem[3],   &MenuItem[24],	NULL            },
/*24*/	{25,	TYPE_INT32,		'2', szpText32,	szpVarName32,		(void*)&Outputs[2].PulseLen,		0,      &MenuItem[3],   &MenuItem[25],	NULL            },
/*25*/	{26,	TYPE_INT32,		'3', szpText33,	szpVarName33,		(void*)&Outputs[3].PulseLen,		0,      &MenuItem[3],   &MenuItem[26],	NULL            },
/*26*/	{27,	TYPE_INT32,		'4', szpText34,	szpVarName34,		(void*)&Outputs[4].PulseLen,		0,      &MenuItem[3],   &MenuItem[27],	NULL            },
/*27*/	{28,	TYPE_INT32,		'5', szpText35,	szpVarName35,		(void*)&Outputs[5].PulseLen,		0,      &MenuItem[3],   &MenuItem[28],	NULL            },
/*28*/	{29,	TYPE_INT32,		'6', szpText36,	szpVarName36,		(void*)&Outputs[6].PulseLen,		0,      &MenuItem[3],   &MenuItem[29],	NULL            },
/*29*/	{30,	TYPE_INT32,		'7', szpText37,	szpVarName37,		(void*)&Outputs[7].PulseLen,		0,      &MenuItem[3],   NULL,			NULL            },

/*30*/	{0,		TYPE_NONE,		0,		NULL,		NULL,			NULL,								0,		NULL,			NULL,			NULL} // Последний в списке
};

MENUITEM* pCurrMenuItem = NULL;

//Строим дерево меню
void InitMenu(void)
{
	//TODO
}

const char PROGMEM szpCrlf[] = "\r\n";

unsigned char IsVariable(INPUT_TYPES Type)
{
	if(Type == TYPE_BOOL ||
			Type == TYPE_INT16 ||
			Type == TYPE_INT32 ||
			Type == TYPE_STRING)
		return TRUE;
	return FALSE;
}

void ShowMenuItems(void)
{
	if(pCurrMenuItem == NULL)
		return;

	// Покажем заголовок
	unsigned char HeadingLen = 0;

	if(pCurrMenuItem->pText != NULL)
	{
		UART0_SendString_P(szpCrlf);
		UART0_SendString_P(pCurrMenuItem->pText);
		HeadingLen = strlen_P(pCurrMenuItem->pText); // подечркнём cтроку
		// подчёркиваем заголовок
		unsigned char i;
		UART0_SendString_P(szpCrlf);
		for(i=0; i<HeadingLen; i++)
			UART0_SendString_P(PSTR("-"));
		UART0_SendString_P(szpCrlf);
	}

	// ПОкажем пункты меню, если они есть
	if(pCurrMenuItem->pFirstChild != NULL)
	{
		MENUITEM* pChild = pCurrMenuItem->pFirstChild;
		// печатаем все тксты меню с одинаковым родителем
		while(pChild != NULL)
		{
			UART0_SendByte(pChild->Key);
			UART0_SendString_P(PSTR("\t"));
			// printf("%c\t", pChild->Key);
			UART0_SendString_P(pChild->pText);
			UART0_SendString_P(szpCrlf);
			pChild = pChild->pNextSibling;
		}
		if(pCurrMenuItem->pParent == NULL)
			UART0_SendString_P(PSTR("ESC\tExit menu\r\n"));
		else
			UART0_SendString_P(PSTR("ESC\tBack\r\n"));
	}
	
}

void ShowCurrentValue(void)
{
	switch(pCurrMenuItem->DataType)
	{
	case TYPE_BOOL:
		if((*(unsigned char*)pCurrMenuItem->pData) == TRUE)
			UART0_SendString_P(PSTR("\"yes\""));
		else UART0_SendString_P(PSTR("\"no\""));
		break;
	case TYPE_INT16:
		{
			// printf("%d", *(int*)pCurrMenuItem->pData);
			char ar[12];
			itoa(*(int*)pCurrMenuItem->pData, ar, 10);
			ar[11] = 0;
			UART0_SendString(ar);
		}
		
		break;
	case TYPE_INT32:
		{
			// printf("%d", *(int*)pCurrMenuItem->pData);
			char ar[16];
			ltoa(*(long*)pCurrMenuItem->pData, ar, 10);
			ar[15] = 0;
			UART0_SendString(ar);
		}
	
		break;
	case TYPE_STRING:
		//printf("\"%s\"",(char*)pCurrMenuItem->pData);
		UART0_SendString((char*)pCurrMenuItem->pData);
		break;
	default: 
		UART0_SendString_P(PSTR("ERROR! You have bad DataType value in this menu item"));
	}
}

// обрабатываем пункт меню в зависимости от того, что там надо сделать.
char ProcessItem(unsigned char c)
{
/*
typedef enum
{
	TYPE_NONE = 0,
	TYPE_BOOL = 1,		// вводим yes/no
	TYPE_INT16 = 2,		// вводим int
	TYPE_INT32 = 3,		// вводим int
	TYPE_STRING = 4,
	TYPE_SUBMENU = 5,	// переходим в подменю и показываем все пункты, у которых текущий родитель
	TYPE_EXEC = 6,		// вызываем обработчик, который должен что-то сделать (например, показать список параметров)
	TYPE_EXITPROGRAM = 7, // по нажатию надо выйти из программы
	TYPE_MENU = 8,		// Это меню, чисто показать пункты
}INPUT_TYPES;
*/
	// Показываем текущее значение и предлагаем редактировать
	if(bEditingItem == FALSE)
	{
		const char* pHelp = "";
		switch(pCurrMenuItem->DataType)
		{
		case TYPE_BOOL: pHelp = "y/n, 1/0"; break;
		case TYPE_INT16:	pHelp = "number"; break;
		case TYPE_INT32:	pHelp = "32-bit number"; break;
		case TYPE_STRING:	pHelp = "string"; break;
		case TYPE_EXEC: // Вызываем функцию
			if(pCurrMenuItem->pData != NULL)
			{
				void (*pFn)() = (void (*)())pCurrMenuItem->pData; 
				(*pFn)();
			}
			else UART0_SendString_P(PSTR("ERROR: No ptr to function!\r\n"));
			// Выполнили и сразу вощвращаемся на уровень выше
			pCurrMenuItem = pCurrMenuItem->pParent;
			ShowMenuItems();
			return MR_WORKING; // Отработали и выходим.
		default: UART0_SendString_P(PSTR("ERROR: Data type not processed!\r\n"));
		}

		bEditingItem = TRUE;
		//printf("Current %s is ", pCurrMenuItem->VarName);
		UART0_SendString_P(PSTR("Current "));
		UART0_SendString_P(pCurrMenuItem->VarName);
		UART0_SendString_P(PSTR(" is "));
		ShowCurrentValue();
		//printf(".\r\nPlease enter new value (%s, ESC to cancel): ", pHelp);
		UART0_SendString_P(PSTR(".\r\nPlease enter new value ("));
		UART0_SendString(pHelp);
		UART0_SendString_P(PSTR(", ESC to cancel): "));
		
		// Готовим массив к редактированию
		memset(szInputString, 0, sizeof(szInputString));
		InputPos = 0;
	}
	else // Собственно, редактируем
	{
		if(c == 27) // ESC
		{
			// Закончили редактирование или хотим выйти с уровня
			if(bEditingItem == TRUE)
			{
				//printf("\r\n%s = ", pCurrMenuItem->VarName);
				UART0_SendString_P(szpCrlf);
				UART0_SendString_P(pCurrMenuItem->VarName);
				UART0_SendString_P(PSTR(" = "));
				ShowCurrentValue(); // Покажем, что значение не изменилось
				UART0_SendString_P(szpCrlf);
			}
			bEditingItem = FALSE;
			return MR_EXITSUBMENU;
		}
		else if(c == 13)
		{
			// ENTER, закончили редактирование
			switch(pCurrMenuItem->DataType)
			{
			case TYPE_BOOL:
				for(c=0; c<MAX_INPUT; c++)
					szInputString[c] = tolower(szInputString[c]);
				if(strchr((char*)szInputString, 'y') != NULL)
					*((unsigned char*)pCurrMenuItem->pData) = TRUE;
				if(strchr((char*)szInputString, '1') != NULL)
					*((unsigned char*)pCurrMenuItem->pData) = TRUE;
				if(strchr((char*)szInputString, 'n') != NULL)
					*((unsigned char*)pCurrMenuItem->pData) = FALSE;
				if(strchr((char*)szInputString, '0') != NULL)
					*((unsigned char*)pCurrMenuItem->pData) = FALSE;
				break;
// not enough memory!
//			case TYPE_INT16:
//				if(szInputString[0] >= '0' && szInputString[0] <= '9')
//					*((int*)pCurrMenuItem->pData) = atoi((char*)szInputString);
//				break;
			case TYPE_INT32:
				if(szInputString[0] >= '0' && szInputString[0] <= '9')
					*((long*)pCurrMenuItem->pData) = atol((char*)szInputString);
				break;
// not enough memory!
//			case TYPE_STRING:
//				strncpy((char*)pCurrMenuItem->pData, (char*)szInputString, pCurrMenuItem->nBytes);
//				break;
			}
			//printf("\r\n%s = ", pCurrMenuItem->VarName);
			UART0_SendString_P(szpCrlf);
			UART0_SendString_P(pCurrMenuItem->VarName);
			UART0_SendString_P(PSTR(" = "));
			ShowCurrentValue();
			UART0_SendString_P(szpCrlf);
			bEditingItem = FALSE;
			return MR_EXITSUBMENU;
		}
		else if(c == 8) // BACKSPACE
		{
			if(InputPos > 0)
			{
				szInputString[InputPos] = 0;
				InputPos--;
			}
		}
		else if(c >= ' ' && c <= 'z')
		{
			if(InputPos < MAX_INPUT)
			{
				szInputString[InputPos++] = c;
				putch(c);
			}
		}
	}

	return MR_WORKING; // FIXME
}

char ProcessMenu(void)
{
	if(!kbhit())
	{
		if(pCurrMenuItem == NULL)
			return MR_NOTHINGPRESSED;
		else return MR_WORKING;
	}

	char c = getch();

	if(!c)
	{
		getch(); //Fn key was pressed
		return 0;
	}

	if(pCurrMenuItem == NULL)
	{
		if(c != 0x0d) 
			return MR_NOTHINGPRESSED;
		else
		{
			pCurrMenuItem = MenuItem;
			ShowMenuItems();
			return MR_WORKING;
		}
	}
	else
	{
//		unsigned char done = 0;
//		while(!done)
		{
//			done = 1;
			if(bEditingItem == FALSE)
			{
				if(c == 27)// переходим на уровень выше
				{
					if(pCurrMenuItem->pParent != NULL)
					{
						pCurrMenuItem = pCurrMenuItem->pParent;
						ShowMenuItems();
						c = 0; // Чтобы этот ESC не попал в редактор 
					}
					else 
						return MR_EXITMENU;
				}
				if(c == 13)// ENTER - покажем пункты меню, если не редактируем, иначе отдаём в редактор
				{
					ShowMenuItems();
					return MR_WORKING;
				}
			}
			
			// мы в меню, обрабатываем его
			MENUITEM* pItem = pCurrMenuItem->pFirstChild;
			// Ксли у пункта меню есть дети, то проверяем, какому из них соответствует
			// нажатая клавиша. 
			// Если у пункта меню нет детей, то делаем то, что там написано
			if(pItem == NULL) // нет детей
			{
				switch(ProcessItem(c))
				{
				case MR_EXITSUBMENU:
					pCurrMenuItem = pCurrMenuItem->pParent;
					ShowMenuItems();
					break;
				}
				return MR_WORKING;
			}
			else
			{
				while(pItem != NULL)
				{
					// перед сравнением приводим к одному и тому же строчному виду
					if( (pItem->Key | 0x20) == (c | 0x20) )
					{
						// выбран этот пункт - нажата кнопка, соответствующая пункту
						// делаем его текущим
						pCurrMenuItem = pItem;
						ShowMenuItems();
						// Ксли ниже этого пункта ничего нет, начинаем обрабатывать пункт
						if(pCurrMenuItem->pFirstChild == NULL)
							if(ProcessItem(c) == MR_EXITSUBMENU)
							{
								pCurrMenuItem = pCurrMenuItem->pParent;
								ShowMenuItems();
								return MR_WORKING;
							}
						return ProcessMenu();
					}
					pItem = pItem->pNextSibling;
				}
				if(pItem == NULL)
				{
					ShowMenuItems();
					return MR_WORKING;
				}
			}
		} // while (!done)
	}

	return MR_WORKING;
}

// Вызываемые функции
void DisplaySettings(void)
{
	// Сохраняем текущий пункт меню
	MENUITEM* pCurr = pCurrMenuItem;
	pCurrMenuItem = MenuItem;
	while(pCurrMenuItem->DataType != TYPE_NONE)
	{
		if(pCurrMenuItem->pData != NULL && pCurrMenuItem->VarName != NULL && 
			IsVariable((INPUT_TYPES)pCurrMenuItem->DataType))
		{
			//printf("%s = ", pCurrMenuItem->VarName);
			UART0_SendString_P(pCurrMenuItem->VarName);
			UART0_SendString_P(PSTR(" = "));
			ShowCurrentValue();
			UART0_SendString_P(szpCrlf);

			// поскольку очень много данных, делаем задержку
//			_delay_ms(50);
		}
		pCurrMenuItem++;
	}

	// Возвращаем взад
	pCurrMenuItem = pCurr;
}

