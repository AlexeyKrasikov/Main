#include "uartExtension.h"

#ifdef DEBUG
#define TIMEOUT_ANSWER              5000000UL  // Максимальное ожидание ответа от слейва
#define TIMEOUT_RECEIVE				3000000UL   // Максимальное время в мкс на прием между байтами информации для мастера.
#else
#define TIMEOUT_ANSWER              5000UL     	// Максимальное время ожидание ответа
#define TIMEOUT_RECEIVE				1000UL      // Максимальное время в мкс на прием между байтами информации
#endif
uartBase::uartBase(uint8_t size, void(*funcStructHandler)()) :
	_counterRead(0), _counterNeadRead(0), _newDataPresent(false), _bufSize(size + 3),
	_startReceiveFlag(false), _funcStructHandler(funcStructHandler)
{
	_buf = new uint8_t[_bufSize];
}

uartBase::~uartBase() { delete[] _buf; }

void uartBase::setup(uint32_t speed) {
	Serial.begin(speed);
	debugPrintlnF("BEGIN");
}

void uartBase::sendMessage(const void* mess, uint8_t size) {
	Serial.write((const uint8_t*)mess, size);
	Serial.flush(); // без этого память освободиться до того момента, как передастся сообщение
}

int uartBase::structHandler() {
	uint8_t temp = SREG;
	int returnValue = (-1);
	cli();					// запрещаем прерывания
	if (_newDataPresent) {
		_funcStructHandler();
		_newDataPresent = false;
		returnValue = 0;
	}
	SREG |= temp & _BV(7);	// разрешаем прерывания. флаг I 7-й по счету
	return returnValue;
}

//---------------------------------------------------------------------------

UExt::UExt(uint8_t pinTransmit, uint8_t size, void(*funcStructHandler)()) : 
	_pinTransmit(pinTransmit), uartBase(size, funcStructHandler) {}

void UExt::setup(uint32_t speed) {
	uartBase::setup();
	pinMode(_pinTransmit, OUTPUT);
	digitalWrite(_pinTransmit, false);
}

void UExt::sendMessage(const void* mess, uint8_t size) {
	digitalWrite(_pinTransmit, true);
	Serial.write((const uint8_t*)mess, size);
	Serial.flush(); // без этого память освободиться до того момента, как передастся сообщение
	digitalWrite(_pinTransmit, false);
}

//-------------------------------------------------------------------------------------------------

UExtSlave::UExtSlave(uint8_t pinTransmit, uint8_t address, uint8_t size, void(*funcStructHandler)()) :
	UExt(pinTransmit, size, funcStructHandler), _address(address), _counterOff(0)
{}

void UExtSlave::sendMessage(const void* mess, uint8_t size) {
	uint8_t* buf = new uint8_t[size + 3];
	buf[0] = _address;
	buf[1] = size + 3;
	memcpy(buf + 2, mess, size);
	buf[size + 2] = computeTableCRC8(buf, size + 2);
	uartBase::sendMessage(buf, size + 3);
	delete[] buf;
}

const uint8_t* UExtSlave::messageHandler()
{
	uint8_t counterWork = 0;
	int temp;

	while (1) {        // если пришел байт
		if (++counterWork > 20) { debugPrintlnF("20 byte processed"); return 0; }                // Если обработано 20 байт и более - выйти.
													 // сделано, чтобы дать возможность работать другим функциям в loop-е и не забирать все ресурсы при очень
													 // большом размере передававемых данных.

		if ((temp = Serial.read()) == (-1)) {
			if (_startReceiveFlag == true) {				   // Не пришло новых байт за период вызова функции.
				_counterOff++;
				if (_counterOff > 1) {
					_startReceiveFlag = false;				   // завершить обработку.
					_counterRead = 0;
					_counterOff = 0;
					debugPrintlnF("lose mess");
				}
			}
			return 0;
		}
		else { if(!_counterRead) { debugPrintlnF("getMess"); }}
		_counterOff = 0;
		_counterRead++;

		if (_counterRead == 1) {								// если пришедший байт адрес   
			if (temp != _address) {							 	// если адрес устройства не совпадает, то не анализировать
				debugPrintlnF("alen address");
				_startReceiveFlag = false;
				_counterRead = 0;                              	// завершить обработку предыдущей команды
			}
			else { 
				_startReceiveFlag = true; 
				debugPrintlnF("good address");
				_buf[0] = temp;
			}			
			continue;
		}

		if (_counterRead == 2) {
			_counterNeadRead = temp;
			_buf[1] = temp;
			debugPrintF("byte to read = ");
			debugPrintln(temp);
			if ((temp > _bufSize) || (temp < 3)) { 
				debugPrintlnF("bad num byte");
				_startReceiveFlag = false;
				_counterRead = 0;                              // завершить обработку предыдущей команды
			}			
			continue;
		}

		_buf[_counterRead - 1] = temp;
		debugPrintF("byte read = ");
		debugPrintln(_counterRead);
		if (_counterRead >= _bufSize) {						 // прочитано байт == макс размеру буфера.
			_startReceiveFlag = false;
			debugPrintlnF("bufer overflow");
		}
		if (_counterRead == _counterNeadRead) {
			_startReceiveFlag = false;
			_counterRead = 0;
			debugPrintF("CRC must be ");
			debugPrintln2arg(computeTableCRC8(_buf, _counterNeadRead - 1), HEX);
			if (!computeTableCRC8(_buf, _counterNeadRead)) {
				_newDataPresent = true;				
				debugPrintlnF("CRC OK");
				return (_buf + 2);
			}
			else {  
				debugPrintlnF("bad CRC"); 
				return 0; 
			}
		} 
		if (!_startReceiveFlag) {
			_counterRead = 0; 
			return 0; 
		}
	}
}

//-----------------------------------------------------------------------------------------

UExtMaster::UExtMaster(uint8_t pinTransmit, uint8_t size, void(*funcStructHandler)()) :
	UExt(pinTransmit, size, funcStructHandler), _answerWait(false) {}

bool UExtMaster::sendMessage(uint8_t address, const void* mess, uint8_t size) {
	if (!_answerWait) { 
		while (Serial.read() != (-1)) {} // Очищаем буфер приемника перед отправкой нового сообщения
		_saveTime = micros();
		_answerWait = true;
		uint8_t* buf = new uint8_t[size + 3];
		buf[0] = address;
		buf[1] = size + 3;
		memcpy(buf + 2, mess, size);
		buf[size + 2] = computeTableCRC8(buf, size + 2);
		uartBase::sendMessage(buf, size + 3);
		delete[] buf;
		return true;
	}
	return false;
}

const uint8_t* UExtMaster::messageHandler(uint8_t address)
{
	uint8_t counterWork = 0;
	int temp;

	while (_answerWait) {        // если пришел байт
		if (++counterWork > 20) { debugPrintlnF("20 byte processed"); return 0; }                
															// Если обработано 20 байт и более - выйти.
															// сделано, чтобы дать возможность работать другим функциям в loop-е и не забирать все ресурсы при очень
															// большом размере передававемых данных.

		if ((temp = Serial.read()) == (-1)) {
			if (_startReceiveFlag == true) {
		        if ( (micros() - _saveTime) < TIMEOUT_RECEIVE) {
		            debugPrintlnF("NO_RECEIVE_TO");
		            continue;
	         	}
	         	else { 
	         		_answerWait = false;
		            _startReceiveFlag = false;
		            _counterRead = 0;                                // завершить обработку.
		            debugPrintlnF("RECEIVE_TO"); 
		            return 0;
	         	}
      		} else {
				if ( (micros() - _saveTime) >= TIMEOUT_ANSWER ) {
					_answerWait = false;
					debugPrintlnF("ANSWER_TO");
				}
				return 0;
			}
		}

		_counterRead++;
		_saveTime = micros();

		if (_counterRead == 1) {								// если пришедший байт адрес   
			if (temp != address) {							 	// если адрес устройства не совпадает, то не анализировать
				debugPrintlnF("alen address");
				_counterRead = 0;                              	// завершить обработку предыдущей команды
			}
			else { 
				debugPrintlnF("good address");
				_startReceiveFlag = true;
				_buf[0] = temp; 
			}
			continue;
		}

		if (_counterRead == 2) {
			_counterNeadRead = temp;
			_buf[1] = temp;
			debugPrintF("byte to read = ");
			debugPrintln(temp);
			if ((temp > _bufSize) || (temp < 3)) { 
				debugPrintlnF("bad num byte");
				_startReceiveFlag = false;
				_counterRead = 0;                              // завершить обработку предыдущей команды
				_answerWait = false;
			}
			continue;
		}

		_buf[_counterRead - 1] = temp;
		debugPrintF("byte read = ");
		debugPrintln(_counterRead);
		if (_counterRead >= _bufSize) {						 // прочитано байт == макс размеру буфера.
			_startReceiveFlag = false;
			debugPrintlnF("bufer overflow");
		}
		if (_counterRead == _counterNeadRead) {
			_answerWait = false;
			_startReceiveFlag = false;
			_counterRead = 0;
			debugPrintF("CRC must be ");
			debugPrintln2arg(computeTableCRC8(_buf, _counterNeadRead - 1), HEX);
			if (!computeTableCRC8(_buf, _counterNeadRead)) {
				_newDataPresent = true;				
				debugPrintlnF("CRC OK");
				return (_buf + 2);
			}
			else {  
				debugPrintlnF("bad CRC"); 
				return 0; 
			}
		} 
		if (!_startReceiveFlag) {
			_counterRead = 0; 
			_answerWait = false;
			return 0; 
		}
	}
	return 0;
}