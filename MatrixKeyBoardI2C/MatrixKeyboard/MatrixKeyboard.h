#ifndef MATRIX_KEYBOARD_H
#define MATRIX_KEYBOARD_H

#include <arduino.h>
#include <dBool.h>

// Не более 18 ног!!!

#define MAX_PINS  18

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

#endif // MATRIX_KEYBOARD_H