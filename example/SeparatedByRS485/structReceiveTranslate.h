#ifndef STRUCT_RECEIVE_TRANSLATE_H
#define STRUCT_RECEIVE_TRANSLATE_H

#include <IO_motor.h>
#define SLAVE_ADDRESS   (123)
#define RS485_BAUDRATE  (115200)

enum MotorCommand {xNO_COMMANDS, xFWD, xREV, xSTOP};
enum CommonCmd {xNO_CMD, xON, xOFF};

struct toSlave { // данные, отправляемые от мастера слейву
public:
  MotorCommand motorCmd[4];
  CommonCmd lockStart;
  CommonCmd lockEnd;
  CommonCmd eyesLed;
};

struct MotorStateField {
  IO::motorState state;
  uint8_t swFwd:1,
          swRev:1;
};

struct toMaster { // данные, отправляемые от слейва мастеру
public:
  MotorStateField motor[4];
  uint16_t buttons;
  uint8_t lockStart :1,
          lockEnd   :1,
          swStart   :1,
          swEnd     :1,
          eyesLed   :1;
};

#endif // STRUCT_RECEIVE_TRANSLATE_H