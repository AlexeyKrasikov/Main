#ifndef UART_EXTENSION_H
#define UART_EXTENSION_H

/*===========================================================*/
/*====================*/ //#define DEBUG /*====================*/
/*===========================================================*/

// классы работают с данными размером до 255 байт. (-3 байта на адрес, размер и CRC)

#include <Arduino.h>
#include "DEBUG.h"                             
#include <avr/pgmspace.h>
#include <avr/wdt.h>
#include <MsTimer2.h>  
#include <CRC.h>

// при изменении следующего параметра, соответствующим образом подправить параметр 
// TIMEOUT_RECEIVE в файле *.cpp 
#define TIME_UPDATE_SLAVE 3  // Период вызова функции проверки сообщений slave в мс

/* 
Протокол. 
Первый байт - адрес устройства (от 0)
Второй байт - полное количество передаваемых байт, с CRC, 
				байтом адреса и байтом количества
Последний байт - CRC посылки.
*/

class uartBase
{
public:
	void setup(uint32_t speed = 115200UL);
	void sendMessage(const void* mess, uint8_t size);
	int structHandler();
protected:
	uartBase(uint8_t size, void(*funcStructHandler)());
	~uartBase();
	bool			_startReceiveFlag;	
	uint8_t			_counterRead,
					_counterNeadRead;
	uint8_t			*_buf;
	const uint8_t 	_bufSize;
	void(*_funcStructHandler)();
	volatile bool 	_newDataPresent;
};

//--------------------------------------------------------------------------

class UExt : public uartBase
{
public:
	void setup(uint32_t speed = 115200UL);
	void sendMessage(const void* mess, uint8_t size);
protected:
	UExt(uint8_t pinTransmit, uint8_t size, void(*funcStructHandler)());
	~UExt() {};
	const uint8_t	_pinTransmit;
};

//---------------------------------------------------------------------------

class UExtSlave :  public UExt
{   
public:
	UExtSlave(uint8_t pinTransmit, uint8_t address, uint8_t size, void(*funcStructHandler)());

	const uint8_t* messageHandler();
	void sendMessage(const void* mess, uint8_t size);

private:
	const uint8_t	_address;
	volatile uint8_t _counterOff;
};

//------------------------------------------------------------------------------

class UExtMaster :  public UExt
{
public:
	UExtMaster(uint8_t pinTransmit, uint8_t size, void(*funcStructHandler)());
	const uint8_t* messageHandler(uint8_t address);
	bool sendMessage(uint8_t address, const void* mess, uint8_t size);
	bool isAnswer() { return _answerWait; }
private:
	uint32_t _saveTime;
	bool _answerWait;
};

#endif // UART_EXTENSION_H