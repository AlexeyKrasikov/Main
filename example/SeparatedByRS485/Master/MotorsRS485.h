#ifndef _MOTOR_RS485_H_
#define _MOTOR_RS485_H_


#include <avr/pgmspace.h>
#include "IO_mqttShell.h"
#include "IO_motor.h"

#include "../structReceiveTranslate.h"

namespace IO {
	class MotorRS485 : public IMotor {
	public:
		MotorRS485(MotorStateField * motorSt, MotorCommand * motorCmd);
		void loop();
		void turn(bool dir);
		void stop();
		motorState getState();
	protected:
		MotorStateField * _motorSt;
		MotorCommand * _motorCmd;
	};
}

#endif  /* _MOTOR_RS485_H_ */