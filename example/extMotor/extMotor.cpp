#include "extMotor.h"

using namespace IO;
using namespace ExtMotorSpace;

namespace ExtMotorSpace { 
	StateMotorHandler stdMotorHandler;
}

bool StateMotorHandler::handler(motorState state)
{
	switch(state) {
		case TIME_OUT: 			return _bitFlag.h_TIME_OUT;
		case DIRECTION_ERR: 	return _bitFlag.h_DIRECTION_ERR;
		case SWITCH_TIME_OUT: 	return _bitFlag.h_SWITCH_TIME_OUT;
		case SWITCH_ERR: 		return _bitFlag.h_SWITCH_ERR;
		case COMMON_ERR: 		return _bitFlag.h_COMMON_ERR;
		case FWD: 				return _bitFlag.h_FWD;
		case REV: 				return _bitFlag.h_REV;
		case UNDEF: 			return _bitFlag.h_UNDEF;					
		case OVERCURRENT: 		return _bitFlag.h_OVERCURRENT;					
		case T_FWD: 			return _bitFlag.h_T_FWD;
		case T_REV: 			return _bitFlag.h_T_REV;
	}
}

//-------------------------------------------------------------------------

void MotorOutPins::turn(bool dir)
{
	stop();
	_turnFlag = true;
	_outFWD.setState(dir);
	_outREV.setState(!dir);
	_outREV._modules.refresh();		
}

void MotorOutPins::stop()
{
 	_outFWD.setState(OFF);
 	_outREV.setState(OFF);
 	_outREV._modules.refresh();
 	delay(10);
 	_turnFlag = false;
}

motorState MotorOutPins::getState()
{
	motorState state;

	if (_turnFlag) {
		if (_outFWD.getState() && !_outREV.getState()) {
			state = T_FWD; 
		}
		else if (!_outFWD.getState() && _outREV.getState()) {
			state = T_REV; 
		}
		else { state = COMMON_ERR; }
	}
	else { state = UNDEF; }

	return state;
}

//-------------------------------------------------------------------------

motorState MotorT::getState()
{
	_state = MotorOutPins::getState();

	if ( (_state == T_FWD) || (_state == T_REV) ) {
		if ( (millis() - _turningStart) >= _turnTimeOut) { 
			_state = TIME_OUT;
		}		
	}
	return _state;
}

void MotorT::turn(bool dir)
{
	MotorOutPins::turn(dir);
	_turningStart = millis();
}

//-------------------------------------------------------------------------

// void MotorS::turn(bool dir)
// {
// 	if (dir) {
// 		if (_fwdS.getState()) { return; }
// 	}
// 	else {
// 		if (_revS.getState()) { return; }
// 	}
// 	MotorLogicOutPins::turn(dir);	
// }

// motorState MotorS::getState()
// {
// 	_state = MotorLogicOutPins::getState();

// 	if (_state != COMMON_ERR) {
// 		if (_fwdS.getState() && _revS.getState()) { _state = SWITCH_ERR; }
// 		else if (_state == T_FWD) {
// 			if (_fwdS.getState()) { _state = FWD; }
// 			else if (_revS.getState()) { _state = DIRECTION_ERR; }
// 		}
// 		else if (_state == T_REV) {
// 			if (_revS.getState()) { _state = REV; }
// 			else if (_fwdS.getState()) { _state = DIRECTION_ERR; }
// 		}
// 	}
// 	return _state;
// }

//-------------------------------------------------------------------------

// motorState MotorST::getState()
// {
// 	MotorS::getState();

// 	if ( (_state != COMMON_ERR) && (_state != SWITCH_ERR) && 
// 		(_state != DIRECTION_ERR) ) {
// 		if ( (_state == T_FWD) || (_state == T_REV) ) {
// 			if ( (millis() - _turningStart) >= _turnTimeOut) { 
// 				_state = TIME_OUT;
// 			}		
// 		}
// 		else if ( (_outFWD.getState() && _revS.getState()) || (_outREV.getState() && _fwdS.getState()) ) {
// 			if ( (millis() - _turningStart) >= _turnTimeOut) { 
// 				_state = SWITCH_TIME_OUT;
// 			}
// 		}
// 	}

// 	return _state;
// }
