/*===========================================================*/
/*====================*/ //#define DEBUG /*====================*/
/*===========================================================*/

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Подключение библиотек
#include <SPI.h>
#include <Ethernet.h>										
#include <PubSubClient.h>
#include <Wire.h>	
#include <ExtPins.h>										
#include <artl.h>
#include <MFRC522.h>
#include <MsTimer2.h>
#include <avr/pgmspace.h>
#include <RFIDuart.h>
#include <avr/wdt.h>
#include <WiznetWatchdog.h>
#include <dBool.h>
#include <MatrixKeyboard.h>
#include <CRC.h>

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Макросы
#define ROWS 			4
// #define COLUMNS			3
#define COLUMNS			4
#define NUMBER_BUTTON 	(ROWS * COLUMNS)
#define DEBOUNCE 		20
#define SLAVE_ADDRESS 	119

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Глобальные переменные
bool arrayOfButtons[NUMBER_BUTTON];
// const char arrayOfChar[NUMBER_BUTTON] = { 	'1', '2', '3', 
// 											'4', '5', '6',
// 											'7', '8', '9',
// 											'*', '0', '#' };
const char* arrayOfChar[NUMBER_BUTTON] = { 	"1", "2", "3", "Отмена",
											"4", "5", "6", "Корректировка",
											"7", "8", "9", "Пробел",
											".", "0", "00", "Ввод" };

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Прототипы и объявления

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Начальная настройка
void setup() 
{
	Serial.begin(115200);
	Serial.println("Begin");

	// RCPins 		Rows (ROWS, 4, 5, 6, 7);
 // 	RCPins 		Columns (COLUMNS, 8, 9, 10);
	RCPins 		Rows (ROWS, 2, 3, 4, 5);
 	RCPins 		Columns (COLUMNS, 6, 7, 8, 9);
	InitKeyboard 	ArrayInitSlave(Rows, Columns, DEBOUNCE);
	doInitSlave(SLAVE_ADDRESS, ArrayInitSlave);

	MsTimer2::set(STD_UPDATE_PERIOD, callback);
	MsTimer2::start();
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Основной цикл
void loop()
{
	
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Обработка прерываний
void callback()
{
	interrupts();
	ext::process();
	extMiniprocess();	
}
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Функции 

void extMiniprocess()
{
	char temp = keyboadProcess(SLAVE_ADDRESS, NUMBER_BUTTON, arrayOfButtons);
	switch(temp) {
		case 1: 
				printPushButtons();
				break;
		case (-1): 
				Serial.println(F("Error CRC..."));
				break;
		case (-2):
				Serial.println(F("Bad message."));
		case 0:
		default:
				break;
	}
}

void printPushButtons()
{
	bool temp = false;

	for (uint8_t i = 0; i < NUMBER_BUTTON; i++) {
		if (arrayOfButtons[i]) {
			Serial.print(arrayOfChar[i]); 
			Serial.print(" ");
			temp = true;
		}
	}

	if (!temp) {
		Serial.print(F("- - - - -"));
	}

	Serial.println();
}
