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

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Макросы
#define NUMBER_INPUT 	18
#define DEBOUNCE 		20
#define SLAVE_ADDRESS 	119
#define NUMBER_BYTE_MESSAGE ( NUMBER_INPUT/8 + ((bool)NUMBER_INPUT%8) )

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Глобальные переменные

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Прототипы и объявления

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Начальная настройка
void setup() 
{
	Serial.begin(115200);
	Serial.println("Begin");

	for (uint8_t i = A4; i <= A5; i++) {
		pinMode(i, INPUT_PULLUP);
		digitalWrite(i, true);
	}

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
}
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Функции 

void extMiniprocess()
{
	int temp;
	uint8_t buffer[NUMBER_BYTE_MESSAGE];

	temp = Wire.requestFrom(SLAVE_ADDRESS, NUMBER_BYTE_MESSAGE);	

	if (temp == NUMBER_BYTE_MESSAGE) {
		for (uint8_t i = 0; i < NUMBER_BYTE_MESSAGE; i++) {
			if (Wire.available()) {
				buffer[i] = Wire.read();
			}
			else { break; }
		}
	
		if (calculateCRC(buffer)) {
			for (uint8_t i = 0; i < NUMBER_INPUT; i++) {
				Serial.print( (bool)(buffer[i >> 3] & _BV(i & 0b111)) ); Serial.print(" ");
			}
			Serial.println();
		}
		else { Serial.println("Error CRC"); }
	}
	else { Serial.println("Bad message"); }
}

bool calculateCRC(uint8_t* buffer)
{
	uint32_t temp;
	temp = 	( (((uint32_t)buffer[2]) & (_BV(0) | _BV(1))) << 21 ) + 
			( ((uint32_t)buffer[1]) << 13 ) +
			( ((uint32_t)buffer[0]) << 5);

	temp %= 61;

	return (buffer[2] >> 2) == (uint8_t)temp ? true : false;
}