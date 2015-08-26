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
#define ROWS 			4
#define COLUMNS			3
#define NUMBER_BUTTON 	(ROWS * COLUMNS)
#define SLAVE_ADDRESS 	119
#define NUMBER_BYTE_MESSAGE ( NUMBER_BUTTON/8 + ((bool)NUMBER_BUTTON%8) )

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

	Wire.begin();

	// MsTimer2::set(STD_UPDATE_PERIOD, callback);
	// MsTimer2::start();
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Основной цикл
void loop()
{
	extMiniprocess();
	delay(2000);
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
	uint8_t buffer[NUMBER_BYTE_MESSAGE + 1];

	temp = Wire.requestFrom(SLAVE_ADDRESS, NUMBER_BYTE_MESSAGE + 1);	

	if (temp == NUMBER_BYTE_MESSAGE + 1) {
		for (uint8_t i = 0; i <= NUMBER_BYTE_MESSAGE; i++) {
			if (Wire.available()) {
				buffer[i] = Wire.read();
			}
		}
	
		if (calculateCRC(buffer)) {
			for (uint8_t i = 0; i < NUMBER_BUTTON; i++) {
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
	uint32_t temp = 0;
	for (uint8_t i = 0; i < NUMBER_BYTE_MESSAGE; i++) {
		temp += ((uint32_t)buffer[i]) << ((i * 8) + 7);
	}

	return (temp % 181) == buffer[NUMBER_BYTE_MESSAGE];
}