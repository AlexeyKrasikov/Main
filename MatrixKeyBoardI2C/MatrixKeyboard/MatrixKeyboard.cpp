#include <arduino.h>
#include <MatrixKeyboard.h>
#include <dBool.h>
#include <CRC.h>
#include <Wire.h>

MatrixKB::MatrixKB(RCPins &row, RCPins &col, uint8_t debounce) :
				_row(row), _col(col), _debounce(debounce), DButtons( (_row.getNum() * _col.getNum()), _debounce, _buttons )
{
	while (_row.getNum() * _col.getNum() > MAX_PINS) ;
	_buttons = new bool[_row.getNum() * _col.getNum()];	
}

MatrixKB::~MatrixKB()
{
	delete[] _buttons;
}

void MatrixKB::loop()
{
	for (uint8_t i = 0; i < _row.getNum(); i++) {
		digitalWrite(_row.getNumOfPins()[i], false);
		pinMode(_row.getNumOfPins()[i], OUTPUT);

		for (uint8_t j = 0; j < _col.getNum(); j++) {
			_buttons[(i * _col.getNum()) + j] = digitalRead(_col.getNumOfPins()[j]);
		}

		pinMode(_row.getNumOfPins()[i], INPUT_PULLUP);
	}

	DButtons(); 
}

bool InitSlaveProcess(int slaveAddress) 
{
	int temp;

	char buffer[3];

	temp = Wire.requestFrom(slaveAddress, 2);	

	if (temp == 2) {
		for (uint8_t i = 0; i < temp; i++) {
			buffer[i] = Wire.read();
		}

		buffer[2] = '\0';

		if (!strcmp(buffer, "OK")) { return true; }
		if (!strcmp(buffer, "KO")) { return false; }
	}

	for (uint8_t i = 0; i < temp; i++) {
		Wire.read();
	}
	return false;
}

void slaveReset(int slaveAddress)
{
	Wire.beginTransmission(slaveAddress);
	Wire.write("RESET", 5);
	Wire.endTransmission();
}

void doInitSlave(int slaveAddress, InitKeyboard &ArrayInitSlave)
{
	Wire.begin();
	slaveReset(slaveAddress);
	do {
		delay(1000);
		Wire.beginTransmission(slaveAddress);
		Wire.write(ArrayInitSlave.getArray(), ArrayInitSlave.getSize());
		Wire.endTransmission();
	}
	while (!InitSlaveProcess(slaveAddress));

	delay(10);
}

char keyboadProcess(int slaveAddress, uint8_t numberButton, bool* arrayOfButtons)
{
	uint8_t numberByteMessage = numberButton/8 + ((bool)numberButton%8);
	uint8_t buffer[numberByteMessage + 2];
	int temp;	

	temp = Wire.requestFrom(slaveAddress, numberByteMessage + 2);	

	if (temp == numberByteMessage + 2) {
		for (uint8_t i = 0; i < temp; i++) {
			buffer[i] = Wire.read();
		}
	
		uint8_t CRC = computeTableCRC8(buffer, numberByteMessage + 1);
		if (CRC == buffer[numberByteMessage + 1]) {
			if (buffer[numberByteMessage]) {
				for (uint8_t i = 0; i < numberButton; i++) {
					arrayOfButtons[i] = (bool)(buffer[i >> 3] & _BV(i & 0b111));
				}
				return 1;
			}
			return 0;
		}
		else { return (-1); }
	}
	else { 
		for (uint8_t i = 0; i < temp; i++) {
			Wire.read();
		}
		return (-2); 
	}
}