#include <CollectorDriver.h>

void CD::turnOn (NextState nextState)
{
	_turnFlag = ON;
	reSaveTime();
	setActionTime (nextState);	
	_nextState = nextState;
	
	if (nextState == OPEN) {
		_turnOpen.setState(ON);
		_turnClose.setState(OFF);
	}
	else {
		_turnOpen.setState(OFF);
		_turnClose.setState(ON);
	}
}

void SCD::turnOn (NextState nextState)
{
	_undefinedSwitches = OFF;
	_turnFlag = ON;
	setActionTime (nextState);
	reSaveTime();
	_nextState = nextState;
	
	if (nextState == OPEN) {
		_turnOn = &_turnOpen;
		_turnOff = &_turnClose;
		_switchWait = &_switchOpen;
		_switchError = &_switchClose;
	}
	else {
		_turnOn = &_turnClose;
		_turnOff = &_turnOpen;
		_switchWait = &_switchClose;
		_switchError = &_switchOpen;
	}

	if (_switchWait->getState() == OFF) { 						// Если нужное положение еще не достигнуто.
		_turnOn->setState(ON);
		_turnOff->setState(OFF);
	}	
}

char SCD::loop ()
{
	_switchOpen.stateChanged();
	_switchClose.stateChanged();

	if (_switchOpen.getState() == ON) {
		if (_switchClose.getState() == ON) _currentState = ERROR;
		else _currentState = OPENED;
	}
	else {
		if (_switchClose.getState() == ON) _currentState = CLOSED;
		else _currentState = UNDEFINED;
	}

	if (_turnFlag == ON) {
		if (_switchWait->getState() == ON) { 								// Достигнут концевик
			setCurrentTime(0);
			assertln(_currentTime);
			return 0; 
		}						

		_currentTime = millis() - _saveTime;
		assertln(_currentTime);

		bool tempSwitchError = _switchError->getState();
		bool tempSwitchWait = _switchWait->getState();

		if (_currentTime >= _offSwitchTime) {
			if (tempSwitchError == ON) {
				if (_undefinedSwitches == OFF) { 							// Не отжался противоположный концевик
					return 2;
				}
				else { 														// Неверное направление		 																	 
					setCurrentTime(0);	
					assertln(_currentTime);
					return 3;
				}
			}	
		}

		if (_currentTime >= _actionTime) {		 							// Вышло время открытия	
			setCurrentTime(0);
			assertln(_currentTime);
			return 6;
		}

		if (_currentState == UNDEFINED) {									// Двигатель в неопределенном положении. Ни один концевик не замкнут.
			_undefinedSwitches = ON;				
			return 1;
		}							 
	}
	return (-1);
}
