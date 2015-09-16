#ifndef DBOOL_H
#define DBOOL_H

// Debounced bool

#include <arduino.h>
#include <artl.h>

class DebounceBase
{   
public:
   explicit DebounceBase(bool & watchState) : 
                  _watchState(watchState), _oldState(watchState)
   {
      _saveTime = millis();
   };
   ~DebounceBase() {};

protected:
   uint32_t       _saveTime;
   bool &         _watchState;
   bool           _oldState;
};

//////////////////////////////////////////////////////////////////////////////////////////////////

class dBool : public DebounceBase
{   
public:
   dBool(bool & watchState, uint8_t debounce = 20) : 
                  DebounceBase(watchState), _debounce(debounce) {};
   ~dBool() {};

   bool operator() (void)
   {
      if (_watchState != _oldState) {
         uint32_t tempTime = millis();
         if ((tempTime - _saveTime) >= _debounce) {
            _saveTime = tempTime;
            _oldState = _watchState;
         }
      }

      return _oldState;
   }

private:
   const uint8_t  _debounce;
};

////////////////////////////////////////////////////////////////////////////////////////////////////////

class dCommonBool : public DebounceBase
{   
public:
   explicit dCommonBool(bool & watchState) : 
                  DebounceBase(watchState) {};
   ~dCommonBool() {};

   bool operator() (void)
   {
      if (_watchState != _oldState) {
         uint32_t tempTime = millis();
         if ((tempTime - _saveTime) >= _debounce) {
            _saveTime = tempTime;
            _oldState = _watchState;
         }
      }

      return _oldState;
   }

private:
   static const uint8_t  _debounce;
};

////////////////////////////////////////////////////////////////////////////////////////////////////////

template <bool INVERTED = false>
class dStructBool
{
public:
   // В качестве параметров с переменным числом аргументов передаются адреса отслеживаемых булевых переменных
   dStructBool(uint8_t i, uint8_t debounce, ...) :    
               _i(i), _debounce(debounce)
   {
      _saveTime = new uint32_t[_i];
      uint32_t tempTime = millis();

      _watchState = new bool*[_i];
      va_list args;
      va_start(args, debounce);

      uint8_t num_byte = _i / 8;
      if ((_i & 0b111) != 0) { num_byte++; }
      _oldState = new uint8_t[num_byte];

      for (uint8_t i = 0; i < num_byte; i ++) { INVERTED ? _oldState[i] = 0xFF : _oldState[i] = 0; }

      for (uint8_t i = 0; i < _i; i++) {
         _saveTime[i] = tempTime;
         _watchState[i] = va_arg(args, bool*);
         bool tempBool = INVERTED ? !*_watchState[i] : *_watchState[i];
         if (tempBool) { _oldState[i >> 3] |= _BV(i & 0b111); }
      }

      va_end(args);
   }

   ~dStructBool() 
   {
      delete[] _saveTime;
      delete[] _watchState;
      delete[] _oldState;
   };

   uint8_t* operator() (void)
   {
      uint32_t tempTime = millis();
      for (uint8_t i = 0; i < _i; i++) {
         bool tempBool = INVERTED ? !*_watchState[i] : *_watchState[i];
         if ( tempBool != (bool)(_oldState[i >> 3] & _BV(i & 0b111)) ) {
            if ((tempTime - _saveTime[i]) >= _debounce) {
               _saveTime[i] = tempTime;
               tempBool ? _oldState[i >> 3] |= _BV(i & 0b111) : _oldState[i >> 3] &= ~_BV(i & 0b111);
            }
         }
      }

      return _oldState;
   }

   uint8_t* getDState() { return _oldState; }

private:
   const uint8_t  _i,
                  _debounce;

   uint32_t *     _saveTime;
   bool **        _watchState;
   uint8_t *      _oldState;
};

////////////////////////////////////////////////////////////////////////////////////////////////////////

template <bool INVERTED = false>
class dStructArrayBool
{
public:
   // В качестве параметров с переменным числом аргументов передаются адреса отслеживаемых булевых переменных
   dStructArrayBool(uint8_t j, uint8_t debounce, bool* (&watchState)) :    
               _i(j), _debounce(debounce), _watchState(watchState)
   {
      _saveTime = new uint32_t[_i];

      uint8_t num_byte = _i / 8;
      if ((_i & 0b111) != 0) { num_byte++; }
      _oldState = new uint8_t[num_byte];
      initOldState(_watchState);
   }

   ~dStructArrayBool() 
   {
      delete[] _saveTime;
      delete[] _oldState;
   };

   void initOldState(bool* &watchState)
   {
      uint32_t tempTime = millis();

      _watchState = watchState;

      uint8_t num_byte = _i / 8;
      if ((_i & 0b111) != 0) { num_byte++; }

      for (uint8_t i = 0; i < num_byte; i ++) { _oldState[i] = 0; }

      for (uint8_t i = 0; i < _i; i++) {
         _saveTime[i] = tempTime;
         bool tempBool = INVERTED ? !watchState[i] : watchState[i];
         if (tempBool) { _oldState[i >> 3] |= _BV(i & 0b111); }
      }
   }

   uint8_t* operator() (void)
   {
      uint32_t tempTime = millis();
      for (uint8_t i = 0; i < _i; i++) {
         bool tempBool = INVERTED ? !_watchState[i] : _watchState[i];
         if ( tempBool != (bool)(_oldState[i >> 3] & _BV(i & 0b111)) ) {
            if ((tempTime - _saveTime[i]) >= _debounce) {
               _saveTime[i] = tempTime;               
               tempBool ? _oldState[i >> 3] |= _BV(i & 0b111) : _oldState[i >> 3] &= ~_BV(i & 0b111);
            }
         }
      }

      return _oldState;
   }

   uint8_t* getDState() { return _oldState; }

private:
   const uint8_t  _i,
                  _debounce;

   uint32_t *     _saveTime;
   bool *&        _watchState;
   uint8_t *      _oldState;
};

///////////////////////////////////////////////////////////////////////////////////////////////////
template <bool INVERTED = false>
class dStructPin
{
public:
   // В качестве параметров с переменным числом аргументов передаются номера пинов Ардуины
   dStructPin(uint8_t i, uint8_t debounce, ...) :    
               _i(i), _debounce(debounce)
   {
      _saveTime = new uint32_t[_i];
      uint32_t tempTime = millis();

      _watchState = new uint8_t[_i];
      va_list args;
      va_start(args, debounce);

      uint8_t num_byte = _i / 8;
      if ((_i & 0b111) != 0) { num_byte++; }
      _oldState = new uint8_t[num_byte];
      _dOldState = new uint8_t[num_byte];

      for (uint8_t i = 0; i < num_byte; i ++) { _oldState[i] = 0; _dOldState[i] = 0; }

      for (uint8_t i = 0; i < _i; i++) {
         _saveTime[i] = tempTime;
         _watchState[i] = va_arg(args, uint16_t);

         bool bTemp = digitalRead(_watchState[i]);
         if (INVERTED) { bTemp = !bTemp; }
         if (bTemp) { _oldState[i >> 3] |= _BV(i & 0b111); _dOldState[i >> 3] |= _BV(i & 0b111); }
      }

      va_end(args);
   }

   ~dStructPin() 
   {
      delete[] _saveTime;
      delete[] _watchState;
      delete[] _oldState;
      delete[] _dOldState;
   };

   void loop()
   {
      uint32_t tempTime = millis();
      for (uint8_t i = 0; i < _i; i++) {
         bool bTemp = digitalRead(_watchState[i]);
         if (INVERTED) { bTemp = !bTemp; }
         bTemp ? _oldState[i >> 3] |= _BV(i & 0b111) : _oldState[i >> 3] &= ~_BV(i & 0b111);

         if ( (_oldState[i >> 3] & _BV(i & 0b111)) != (_dOldState[i >> 3] & _BV(i & 0b111)) ) {
            if ((tempTime - _saveTime[i]) >= _debounce) {
               _saveTime[i] = tempTime;
               (_oldState[i >> 3] & _BV(i & 0b111)) ? _dOldState[i >> 3] |= _BV(i & 0b111) : _dOldState[i >> 3] &= ~_BV(i & 0b111);
            }
         }
      }
   }

   uint8_t* getState() { return _oldState; }
   uint8_t* getDState() { return _dOldState; }

private:
   const uint8_t  _i,
                  _debounce;

   uint32_t *     _saveTime;
   uint8_t *      _watchState;
   uint8_t *      _oldState,
           *      _dOldState;
};

#endif // DBOOL_H