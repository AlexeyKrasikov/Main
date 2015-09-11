/*===========================================================*/
/*====================*/ //#define DEBUG /*====================*/
/*===========================================================*/

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Подключение библиотек
#include "Debug.h"
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
#include <CRC.h>
#include <MatrixKeyboard.h>
#include <RingBuffer.h>

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Макросы
#define ROWS 			4
#define COLUMNS			3
#define NUMBER_BUTTON 	(ROWS * COLUMNS)
#define DEBOUNCE 		20
#define SLAVE_ADDRESS 	119
#define NUMBER_BYTE_MESSAGE ( NUMBER_BUTTON/8 + ((bool)NUMBER_BUTTON%8) )
#define MAX_BYTE_MESSAGE 4

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Глобальные переменные
RingBuffer<uint8_t> gloBuf(192 * MAX_BYTE_MESSAGE);

uint8_t numberBytes;
bool 	InitFlag = false;
volatile bool AnswerFlag = false;

uint8_t* arrayReceive = new uint8_t[22];
uint8_t* rowsPtr = new uint8_t;
uint8_t* columnsPtr = new uint8_t;
uint8_t* rowPinsPtr = new uint8_t[18];
uint8_t* columnPinsPtr = new uint8_t[18];
uint8_t* debouncePtr = new uint8_t;


//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Прототипы и объявления

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Начальная настройка
void setup() 
{
	uint8_t CRC;

	assertInit(115200);
	assertlnF("BEGIN DEBUG");

	Wire.begin(SLAVE_ADDRESS);
	Wire.onReceive(onReceiveSlave);
	Wire.onRequest(onRequestSlaveInit);

	while(1) {
		while(!Wire.available()) {
			assertlnF("wait...");
			ass(delay(1000));
		}

		uint8_t numberReceiveBytes = Wire.available();

		assertlnF("Пришло сообщение по I2C");
		assertF("Число принятых байт = ");
		assertln(numberReceiveBytes);
		assertlnF("Начинаем обработку...");

		if (numberReceiveBytes > 22) {
			continue;
		}

		uint8_t lastElement = numberReceiveBytes - 1;

		for (uint8_t numByte = 0; numByte < numberReceiveBytes; numByte++) {
			arrayReceive[numByte] = Wire.read();
			ass(
				if (numByte != lastElement) {
					assert(arrayReceive[numByte]); assertF(" ");
				}
				else {
					assert2(arrayReceive[numByte], HEX);
				}
				);
		}
		assertln();

		assertlnF("Обработали все принятые байты, инициализируем переменные...");

		CRC = computeTableCRC8(arrayReceive, lastElement);

		if(CRC == arrayReceive[lastElement]) {
			InitFlag = true;
			break;
		}
	}

	*rowsPtr = arrayReceive[0];

	for (uint8_t i = 0, j = 1; i < *rowsPtr; i++, j++) {
		rowPinsPtr[i] = arrayReceive[j];
	}

	*columnsPtr = arrayReceive[*rowsPtr + 1];

	for (uint8_t i = 0, j = *rowsPtr + 2; i < *columnsPtr; i++, j++) {
		columnPinsPtr[i] = arrayReceive[j];
	}

	*debouncePtr = arrayReceive[*rowsPtr + *columnsPtr + 2];

	numberBytes = (*rowsPtr * *columnsPtr) /8 + ((bool)(*rowsPtr * *columnsPtr)%8);

	while(!AnswerFlag);

	Wire.onRequest(onRequestSlave);

	delete[] arrayReceive;
	assertlnF("Поехали! )))");
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Основной цикл
void loop()
{
	static RCPins 		Rows (*rowsPtr, rowPinsPtr);
	static RCPins 		Columns (*columnsPtr, columnPinsPtr);
	static MatrixKB 	MKB(Rows, Columns, *debouncePtr);

	if (InitFlag) {
		delete rowsPtr;
		delete columnsPtr;
		delete debouncePtr;
		delete[] rowPinsPtr;
		delete[] columnPinsPtr;
		delete[] arrayReceive;

		InitFlag = false;
	}

	MKB.loop();

	cli();

	static uint8_t tempBuffer[MAX_BYTE_MESSAGE];
	bool tempFlag = false;

	for (uint8_t i = 0; i < NUMBER_BYTE_MESSAGE; i++) {
		if (tempBuffer[i] != MKB.getDStruct()[i]) {
			tempBuffer[i] = MKB.getDStruct()[i];
			tempFlag = true;
		} 
	}

	if (tempFlag) {
		for (uint8_t i = 0; i < MAX_BYTE_MESSAGE; i++) {
			gloBuf.put(tempBuffer[i]);
		}
	}

	sei();
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Обработка прерываний

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Функции 

void onReceiveSlave(int howMany) {}

void onRequestSlaveInit() 
{
	if(InitFlag) { Wire.write("OK", 2); AnswerFlag = true;}
	else { Wire.write("KO", 2); }
}

void onRequestSlave()
{
	uint8_t buffer[MAX_BYTE_MESSAGE + 2];

	uint8_t temp = gloBuf.getElements() >> 2;

	for (uint8_t i = 0; i < MAX_BYTE_MESSAGE; i++) {
		buffer[i] = gloBuf.get();
	}

	buffer[numberBytes] = temp;

	buffer[numberBytes + 1] = computeTableCRC8(buffer, numberBytes + 1);

	Wire.write(buffer, numberBytes + 2);
}
