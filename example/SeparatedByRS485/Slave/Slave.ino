#include <arduino.h>
#include <SPI.h>
#include <Ethernet.h> 
#include <PubSubClient.h>
//#include <MFRC522.h>
#include <Wire.h>  
#include <MsTimer2.h>                                 
//#include <ExtPins.h> 
//#include <artl.h>
#include <dBool.h>
#include <CRC.h>
#include <IO_boards.h>
#include <IO_motor.h>
#include <avr/pgmspace.h>
#include <uartExtension.h>
#include <avr/wdt.h>
#include "../structReceiveTranslate.h"

////////////////////////////////////////////////////////////////////////////////////////////
// Макросы
#define IN_FLASH    const PROGMEM char

#define DEBOUNCE 		50
#define NUMBER_BUTTON 	16
#define DRIVER_TIMEOUT 	3000

using namespace IO;

////////////////////////////////////////////////////////////////////////////////////////////
toMaster MyTranslateInfo;
toSlave MyReceiveInfo;

void extOutCmdHandler(IOut& extOut, CommonCmd command);
void ReceiveStructHandler();
void resetReceiveInfo();
void readButtons();
void timer_ev();
////////////////////////////////////////////////////////////////////////////////////////////
// порядок пинов на передатчике сверху вниз 10, 13, 11, 12, A0, A1(pinDir)
Modules modules(3, 4, 10, 13, 11, 12, A0);
Modules &IExt::_modules = modules;

ExtIn buttons[] = {	ExtIn(2, 0, INVERT), ExtIn(2, 1, INVERT),	                
	                ExtIn(2, 2, INVERT), ExtIn(2, 3, INVERT),	                
	                ExtIn(2, 4, INVERT), ExtIn(2, 5, INVERT),	                
	                ExtIn(2, 6, INVERT), ExtIn(2, 7, INVERT),
	                ExtIn(3, 0, INVERT), ExtIn(3, 1, INVERT),
	                ExtIn(3, 2, INVERT), ExtIn(3, 3, INVERT),	                
	                ExtIn(3, 4, INVERT), ExtIn(3, 5, INVERT),	                
	                ExtIn(3, 6, INVERT), ExtIn(3, 7, INVERT) };	                

bool buttonArray[NUMBER_BUTTON];	// сюда будут считываться кнопки
dStructArrayBool<> dButton(NUMBER_BUTTON, DEBOUNCE); // класс, формирующий упакованную структуру Дебоунс кнопок

ExtIn 	swStart(0, 7, INVERT),
		swEnd(0, 6, INVERT);

ExtOut 	lockStart(2, 0),
		lockEnd(2, 1),
		anubisEyes(2, 2);

ExtOut 	motorIN1[] = { 	ExtOut(1, 6),
						ExtOut(1, 3),
						ExtOut(1, 0),
						ExtOut(0, 5) },

		motorIN2[] = { 	ExtOut(1, 5),
						ExtOut(1, 2),
						ExtOut(0, 7),
						ExtOut(0, 4) },

		motorEN[] = { 	ExtOut(1, 7),
						ExtOut(1, 4),
						ExtOut(1, 1),
						ExtOut(0, 6) };

ExtIn 	motorFwdSw[] = {ExtIn(1, 6, INVERT),
						ExtIn(1, 4, INVERT),
						ExtIn(1, 2, INVERT),
						ExtIn(1, 0, INVERT) },

 		motorRevSw[] = {ExtIn(1, 7, INVERT),
						ExtIn(1, 5, INVERT),
						ExtIn(1, 3, INVERT),
						ExtIn(1, 1, INVERT) };

MotorSw grips[] = { MotorSw(motorIN1[0], motorIN2[0], motorEN[0], motorFwdSw[0], motorRevSw[0], DRIVER_TIMEOUT),
					MotorSw(motorIN1[1], motorIN2[1], motorEN[1], motorFwdSw[1], motorRevSw[1], DRIVER_TIMEOUT),
					MotorSw(motorIN1[2], motorIN2[2], motorEN[2], motorFwdSw[2], motorRevSw[2], DRIVER_TIMEOUT),
					MotorSw(motorIN1[3], motorIN2[3], motorEN[3], motorFwdSw[3], motorRevSw[3], DRIVER_TIMEOUT) };

RefreshModules dummy(modules);

UExtSlave messager(A1, SLAVE_ADDRESS, sizeof(MyReceiveInfo), ReceiveStructHandler);
///////////////////////////////////////////SETUP///////////////////////////////////////////////
void setup()
{
    messager.setup(RS485_BAUDRATE); // без параметров по умолчанию скорость 115200
    readButtons();
    dButton.initOldState(buttonArray); 
    resetReceiveInfo();
    MsTimer2::set(TIME_UPDATE_SLAVE, timer_ev);
    MsTimer2::start();
}
////////////////////////////////////////////////////////////////////////////////////////////////

void loop()
{
	modules.refresh();
    readButtons();	// считываем кнопки
    dButton();		// пакуем состояния в структуру
    for(uint8_t i = 0; i < 4; i++) {
    	grips[i].loop();    	
    }
    
    if(!messager.structHandler()) { // если пришел запрос
        messager.sendMessage(&MyTranslateInfo, sizeof(MyTranslateInfo)); // не вызывать внутри structHandler()!!!!
    }
}

void readButtons()
{
	for (uint8_t i = 0; i < NUMBER_BUTTON; i++) {
		buttonArray[i] = buttons[i].getState();
	}
}

void resetReceiveInfo() {
    uint8_t i;
    uint8_t* temp = (uint8_t*) (&MyReceiveInfo);   
    for (i = 0; i < sizeof(MyReceiveInfo); i ++) {
        temp[i] = 0;
    } 
}

void extOutCmdHandler(IOut& extOut, CommonCmd command) {
	switch(command) {
		case xNO_CMD:
		default: 		break;
		case xON:		extOut.setState(ON);
						break;
		case xOFF: 		extOut.setState(OFF);
						break;
	}
}

void ReceiveStructHandler() { 
	// обрабатываем команды
	for(uint8_t i = 0; i < 4; i++) {
		switch(MyReceiveInfo.motorCmd[i]) {
			case xNO_COMMANDS: 
			default: 			break;
			case xFWD: 			grips[i].turn(ON);
								break;
			case xREV:			grips[i].turn(OFF);
								break;
			case xSTOP: 		grips[i].stop();
								break;
		}
	}
	extOutCmdHandler(lockStart, MyReceiveInfo.lockStart);
	extOutCmdHandler(lockEnd, MyReceiveInfo.lockEnd);
	extOutCmdHandler(anubisEyes, MyReceiveInfo.eyesLed);
	resetReceiveInfo(); // очищаем приемную структуру после обработки команд
	// заполняем структуру отправки данных
	for(uint8_t i = 0; i < 4; i++) {
		MyTranslateInfo.motor[i].state = grips[i].getState();
		MyTranslateInfo.motor[i].swFwd = motorFwdSw[i].getState();
		MyTranslateInfo.motor[i].swRev = motorRevSw[i].getState();
	}
	MyTranslateInfo.buttons = (((uint16_t)(dButton.getDState())[1]) << 8) + (dButton.getDState())[0];
	MyTranslateInfo.lockStart = lockStart.getState();
	MyTranslateInfo.lockEnd = lockEnd.getState();
	MyTranslateInfo.swStart = swStart.getState();
	MyTranslateInfo.swEnd = swEnd.getState();
	MyTranslateInfo.eyesLed = anubisEyes.getState();
}

void timer_ev() {
    interrupts();
    const uint8_t* buf = messager.messageHandler();
    if (buf) { memcpy(&MyReceiveInfo, buf, sizeof(MyReceiveInfo)); }    // копируем полученные данные в нашу приемную структуру.
}
