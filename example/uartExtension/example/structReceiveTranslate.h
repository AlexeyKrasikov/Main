#ifndef STRUCT_RECEIVE_TRANSLATE_H
#define STRUCT_RECEIVE_TRANSLATE_H

struct SlaveInfo { // данные, отправляемые от мастера слейву
public:
  uint8_t info1 : 1; // битовое поле для уплотнения структуры данных
  uint8_t info2 : 1;
  uint8_t info3;
  int   info4;
};

struct MasterInfo { // данные, отправляемые от слейва мастеру
public:
enum MotorCommand {xNO_COMMANDS, xON, xOFF, xSTOP};
  MotorCommand command;
  uint8_t info1;
  uint8_t info2 : 1;
  uint8_t info3 : 1;
  uint32_t  info4;
};

#endif // STRUCT_RECEIVE_TRANSLATE_H

