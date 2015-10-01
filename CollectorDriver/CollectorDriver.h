#ifndef __CollectorDriver_H__
#define __CollectorDriver_H__

/*-------------------------------------------------------------------------------------------------------
Типы возвращаемых значений методов loop()
return -1	- 	функция loop не задействована
return 0 	- 	удачное завершение. Сработал концевик ожидаемого направления.
return 1 	- 	информационное. Двигатель находится в неопределенном положении между концевиками 
return 2 	- 	ошибка. Не отжался концевик исходного состояния двигателя.
return 3 	- 	ошибка. Не правильное направление вращения. Сработал противоположный концевик.
return 4 	- 	ошибка. Зажаты оба концевика. 
return 5 	- 	ошибка. Ток двигателя превысил максимальное значение.
return 6 	- 	ошибка. Таймаут. Вышло время максимальной работы двигателя. Ни один концевик не сработал.
-------------------------------------------------------------------------------------------------------*/
#include <artl.h>
#include <DEBUG.H>

#define CD CollectorDriver
#define SCD SwitchCollectorDriver

#define CUR_SENS true

class CD // чисто по таймеру
{
public:
	typedef enum NextState {OPEN, CLOSE} NextState;
	typedef enum CurrentState {OPENED, CLOSED, UNDEFINED, ERROR} CurrentState;

	CD (ILogicOut &turnOpen, ILogicOut &turnClose, uint32_t actionTimeOpen = 10000, uint32_t actionTimeClose = 10000)
		: _turnOpen(turnOpen), _turnClose(turnClose), _actionTimeOpen(actionTimeOpen), _actionTimeClose(actionTimeClose)
		{
			_turnOpen.setState(OFF);
			_turnClose.setState(OFF);

			_turnFlag = OFF;
			_currentTime = 0;
			_nextState = OPEN;
		}
	~CD() {}

	NextState getTurnState ()
	{
		return _nextState;
	}

	void turnOn (NextState nextState);

	void turnOff ()
	{
		_turnFlag = OFF;
		_turnOpen.setState(OFF);
		_turnClose.setState(OFF);
	}

	uint16_t getCurrentTime ()
	{
		return _currentTime;
	}

	void freezeTime ()
	{
		_turnFlag = OFF;
	}

	bool getTurnFlag ()
	{
		return _turnFlag;
	}

	char loop ()
	{
		if (_turnFlag == ON) {
			_currentTime = millis() - _saveTime;
			if (_currentTime < _actionTime) return 1;
			setCurrentTime(0);
			return 6;
		}
		return (-1);
	}
	
protected:
	NextState 		_nextState;
	bool 			_turnFlag;
	const uint32_t 	_actionTimeOpen,
					_actionTimeClose;
	uint32_t 		_actionTime,
					_currentTime;
	uint32_t 		_saveTime;
	ILogicOut 		&_turnOpen,
					&_turnClose;

	void setCurrentTime (uint16_t value)
	{
		_currentTime = value;
	}

	void reSaveTime () 
	{
		_saveTime = millis();
	}

	void setActionTime (NextState nextState) 
	{
		if (_nextState == nextState) {
			if (_currentTime <= 1) {
				if (_nextState == OPEN) { _actionTime = _actionTimeOpen; }
				else { _actionTime = _actionTimeClose; }
			}
			else { _actionTime = _actionTime - _currentTime; }
		}
		else {
			if (_currentTime <= 1) {
				if (_nextState == OPEN) { _actionTime = _actionTimeClose; }
				else { _actionTime = _actionTimeOpen; }
			}
			else {
				uint32_t tempAction;
				_currentTime = _actionTime - _currentTime;
				if (_nextState == OPEN) { 
					tempAction = (uint32_t)_actionTimeClose * _currentTime / _actionTimeOpen; 
					_actionTime = _actionTimeClose - (uint16_t)tempAction;
				}
				else { 
					tempAction = (uint32_t)_actionTimeOpen * _currentTime / _actionTimeClose; 
					_actionTime = _actionTimeOpen - (uint16_t)tempAction;
				} 
			} 
		}
	}
};

class SCD : public CD 	// по таймеру + концевики
{
public:
	SCD (	ILogicOut &turnOpen, ILogicOut &turnClose, 
			ILogicIn &switchOpen, ILogicIn &switchClose, 
			uint32_t actionTimeOpen = 10000, uint32_t actionTimeClose = 10000, 
			uint16_t offSwitchTime = 500)
		: CD (	turnOpen, turnClose, actionTimeOpen, actionTimeClose),
				_switchOpen(switchOpen), _switchClose(switchClose), _offSwitchTime(offSwitchTime) 
	{
		_turnOn = 0;
		_turnOff = 0;
		_switchWait = 0;
		_switchError = 0;
		_currentState = UNDEFINED;
		_switchOpen.setState(_switchOpen.readState());
		_switchOpen.setState(_switchClose.readState());
	}
	~SCD() {}

	void turnOn (NextState nextState);

	char loop ();

	CurrentState getCurrentState ()
	{
		return _currentState;
	}

protected:
	CurrentState _currentState;
	bool 		_undefinedSwitches;
	const uint16_t 	_offSwitchTime;
	ILogicOut 	*_turnOn,
				*_turnOff;
	ILogicIn 	&_switchOpen,
				&_switchClose,
				*_switchWait,
				*_switchError;
};

#endif