#ifndef __EXT_MOTOR_H__
#define __EXT_MOTOR_H__

#include <Arduino.h>
#include <IO_boards.h>

#define INVERT (true)

#define ON (true)
#define OFF (false)

namespace ExtMotorSpace {
	enum motorState { FWD, T_FWD, UNDEF, T_REV, REV, TIME_OUT, SWITCH_TIME_OUT, 
							DIRECTION_ERR, SWITCH_ERR, COMMON_ERR, OVERCURRENT };

	class StateMotorHandler {
	public:
		StateMotorHandler(bool h_TIME_OUT = true,
						bool h_DIRECTION_ERR = true,
						bool h_SWITCH_TIME_OUT = true,
						bool h_SWITCH_ERR = true,
						bool h_COMMON_ERR = true,
						bool h_FWD = true,
						bool h_REV = true,
						bool h_UNDEF = true,					
						bool h_OVERCURRENT = true,					
						bool h_T_FWD = false,
						bool h_T_REV = false) 
		{
		 	_bitFlag.h_TIME_OUT = h_TIME_OUT;
			_bitFlag.h_DIRECTION_ERR = h_DIRECTION_ERR;
			_bitFlag.h_SWITCH_TIME_OUT = h_SWITCH_TIME_OUT;
			_bitFlag.h_SWITCH_ERR = h_SWITCH_ERR;
			_bitFlag.h_COMMON_ERR = h_COMMON_ERR;
			_bitFlag.h_FWD = h_FWD;
			_bitFlag.h_REV = h_REV;
			_bitFlag.h_UNDEF = h_UNDEF;					
			_bitFlag.h_OVERCURRENT = h_OVERCURRENT;					
			_bitFlag.h_T_FWD = h_T_FWD;
			_bitFlag.h_T_REV = h_T_REV; 
		};

		bool handler(ExtMotorSpace::motorState state);
	protected:
		struct  BitFlag {
			bool h_TIME_OUT 		: 1;
			bool h_DIRECTION_ERR 	: 1;
			bool h_SWITCH_TIME_OUT 	: 1;
			bool h_SWITCH_ERR 		: 1;
			bool h_COMMON_ERR 		: 1;
			bool h_FWD 				: 1;
			bool h_REV 				: 1;
			bool h_UNDEF 			: 1;					
			bool h_OVERCURRENT 		: 1;					
			bool h_T_FWD 			: 1;
			bool h_T_REV 			: 1;
		} _bitFlag;
	};

	extern ExtMotorSpace::StateMotorHandler stdMotorHandler;

	class IMotor {
	public:
		virtual motorState getState() = 0;
		virtual void turn(bool dir) = 0;
		virtual void stop() = 0;
		virtual motorState loop() = 0;
	};

	class MotorOutPins : public IMotor { 
	public: 
		void turn(bool dir);
		void stop();
		motorState getState();
	protected:
		MotorOutPins(IO::ExtOut& outFWD, IO::ExtOut& outREV, StateMotorHandler& handl = stdMotorHandler)
			: _outFWD(outFWD), _outREV(outREV), _turnFlag(false), _handl(stdMotorHandler)
		{
			_outFWD.setState(OFF);
		 	_outREV.setState(OFF);
		 	_outREV._modules.refresh();
		};
		motorState _state;
		StateMotorHandler& _handl;
		bool _turnFlag;
		IO::ExtOut& 	_outFWD;
		IO::ExtOut& 	_outREV;
	};

	class MotorInPins {
	protected:
		MotorInPins(IO::ExtIn& fwdS, IO::ExtIn& revS)
						: _fwdS(fwdS), _revS(revS) {};

		IO::ExtIn& 	_fwdS;
		IO::ExtIn& 	_revS;
	};

	// class MotorAnalogInPins {
	// protected:
	// 	MotorAnalogInPins(IAnalogIn& current)
	// 	: _current(current) {};

	// 	IAnalogIn& 	_current;
	// };

	class MotorTimeOuts {
	protected:
		MotorTimeOuts(uint32_t turnTimeOut) //, uint16_t switchTimeOut)
					: /*_switchTimeOut(switchTimeOut), */_turnTimeOut(turnTimeOut) {};

		//uint16_t _switchTimeOut;
		uint32_t _turningStart, _turnTimeOut;
	};

	class MotorT : 	public virtual MotorOutPins,
					public virtual MotorTimeOuts {
	public:
		MotorT(IO::ExtOut& outFWD, IO::ExtOut& outREV, uint32_t turnTimeOut,
				StateMotorHandler& handl = stdMotorHandler) 
				: MotorOutPins(outFWD, outREV, stdMotorHandler), 
				MotorTimeOuts(turnTimeOut) {};

		void turn(bool dir);
		motorState getState();
		motorState loop() { 
			motorState temp = getState();
			if (_handl.handler(temp)) { stop();} 
			//_outFWD._modules.refresh();
			return temp;
		}
	};

	// class MotorS : 	public virtual Motor, public virtual MotorOutPins,
	// 				public virtual MotorInPins {
	// public:
	// 	MotorS(ILogicOut& outFWD, ILogicOut& outREV, ILogicOut& outEN,
	// 			ILogicIn& fwdS, ILogicIn& revS, StateMotorHandler& handl = stdMotorHandler) 
	// 			: MotorLogicOutPins(outFWD, outREV, outEN), 
	// 			MotorLogicInPins(fwdS, revS), Motor(stdMotorHandler) {};			

	// 	void turn(bool dir);
	// 	void loop() { if (_handl.handler(getState())) { stop(); }}
	// 	ExtMotorSpace::motorState getState();
	// };

	// class MotorST : public MotorS, public MotorT {
	// public:
	// 	MotorST(ILogicOut& outFWD, ILogicOut& outREV, ILogicOut& outEN,
	// 			ILogicIn& fwdS, ILogicIn& revS, uint32_t turnTimeOut, 
	// 			uint16_t switchTimeOut = 500, StateMotorHandler& handl = stdMotorHandler)
	// 			: MotorLogicOutPins(outFWD, outREV, outEN), 
	// 			MotorLogicInPins(fwdS, revS), Motor(stdMotorHandler),
	// 			MotorTimeOuts(turnTimeOut, switchTimeOut), 
	// 			MotorS(outFWD, outREV, outEN, fwdS, revS),
	// 			MotorT(outFWD, outREV, outEN, turnTimeOut, switchTimeOut) {};

	// 	void loop() { MotorT::loop(); }
	// 	void turn(bool dir) { MotorS::turn(dir); } 
	// 	ExtMotorSpace::motorState getState();
	// };

} // namespace ExtMotorSpace

#endif // __EXT_MOTOR_H__