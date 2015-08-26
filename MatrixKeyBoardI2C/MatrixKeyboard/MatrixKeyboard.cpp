#include <arduino.h>
#include <MatrixKeyboard.h>
#include <dBool.h>

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