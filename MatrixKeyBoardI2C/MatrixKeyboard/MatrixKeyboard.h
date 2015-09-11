#ifndef MATRIX_KEYBOARD_H
#define MATRIX_KEYBOARD_H

#include <arduino.h>
#include <dBool.h>
#include <CRC.h>
#include <Wire.h>

// Не более 18 ног!!!

#define MAX_PINS  18

class InitKeyboard;

bool InitSlaveProcess(int slaveAddress);
void doInitSlave(int slaveAddress, InitKeyboard &ArrayInitSlave);
uint8_t keyboadProcess(int slaveAddress, uint8_t numberButton, bool* arrayOfButtons);

class RCPins
{
public:
   RCPins(uint8_t rc, ...) : _rc(rc)
   {
      while ( (_rc == 0) || (_rc > MAX_PINS) ) ;

      _numRC = new uint8_t[_rc];

      va_list args;
      va_start(args, rc);

      for (uint8_t i = 0; i < _rc; i++) {
         _numRC[i] = va_arg(args, uint16_t);
         pinMode(_numRC[i], INPUT_PULLUP);
      }

      va_end(args);
   }

   RCPins(uint8_t rc, uint8_t* numRC) : _rc(rc)
   {
      while ( (_rc == 0) || (_rc > MAX_PINS) ) ;

      _numRC = new uint8_t[_rc];

      for (uint8_t i = 0; i < _rc; i++) {
         _numRC[i] = numRC[i];
         pinMode(_numRC[i], INPUT_PULLUP);
      }
   }

   ~RCPins() { delete[] _numRC; }

   uint8_t  getNum() { return _rc; }
   uint8_t* getNumOfPins() { return _numRC; }

protected:
   const uint8_t  _rc;
         uint8_t  *_numRC;
};

class MatrixKB
{   
public:
   MatrixKB(RCPins &row, RCPins &col, uint8_t debounce = 20);
   ~MatrixKB();

   void loop();

   uint8_t* getDStruct() { return DButtons.getDState(); }

protected:
   const uint8_t                    _debounce;
         RCPins &                   _row, _col;
         bool                       *_buttons;
         dStructArrayBool<INVERT>   DButtons;
};

class InitKeyboard
{
public:
   InitKeyboard(RCPins &row, RCPins &col, uint8_t debounce = 20)
   {
      uint8_t Index = 0;

      _size = row.getNum() + col.getNum() + 4;

      _ArrayTransmitt = new uint8_t[_size];

      _ArrayTransmitt[Index] = row.getNum();
      Index++;

      for (uint8_t i = 0, j = Index; i < row.getNum(); i++, j++) {
         _ArrayTransmitt[j] = row.getNumOfPins()[i];
      }
      Index += row.getNum();

      _ArrayTransmitt[Index] = col.getNum();
      Index++;

      for (uint8_t i = 0, j = Index; i < col.getNum(); i++, j++) {
         _ArrayTransmitt[j] = col.getNumOfPins()[i];
      }
      Index += col.getNum();

      _ArrayTransmitt[Index] = debounce;
      Index++;

      _ArrayTransmitt[Index] = computeTableCRC8(_ArrayTransmitt, Index);
   }
   ~InitKeyboard()
   {
      delete[] _ArrayTransmitt;
   }

   uint8_t* getArray() { return _ArrayTransmitt; }
   uint8_t getSize() { return _size; }

protected:
   uint8_t* _ArrayTransmitt;
   uint8_t  _size;
};

#endif // MATRIX_KEYBOARD_H