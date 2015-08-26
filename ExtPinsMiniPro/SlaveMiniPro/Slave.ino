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

dStructPin<> dP(NUMBER_INPUT, DEBOUNCE, 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, A0, A1, A2, A3);

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Начальная настройка
void setup() 
{
	for (uint8_t i = 0; i < NUMBER_INPUT + 2; i++) {
		pinMode(i, INPUT_PULLUP);
		digitalWrite(i, true);
	}

	Wire.begin(SLAVE_ADDRESS);
	//Wire.onReceive(onReceiveSlave);
	Wire.onRequest(onRequestSlave);
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Основной цикл
void loop()
{
	dP.loop();
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
void onReceiveSlave(int numBytes) 
{
	while(Wire.available()) { Wire.read(); }
}

void onRequestSlave()
{
	dP.getDState()[NUMBER_BYTE_MESSAGE - 1] &= (_BV(1) | _BV(0)); 
	dP.getDState()[NUMBER_BYTE_MESSAGE - 1] += calculateCRC() << 2;
	Wire.write(dP.getDState(), NUMBER_BYTE_MESSAGE);
}

uint8_t calculateCRC()
{
	uint32_t temp;
	temp = 	( ((uint32_t)dP.getDState()[2]) << 21 ) + 
			( ((uint32_t)dP.getDState()[1]) << 13 ) +
			( ((uint32_t)dP.getDState()[0]) << 5);

	return temp % 61;
}
