#include <MsTimer2.h>                                   
#include <avr/wdt.h>
#include <CRC.h>
#include <uartExtension.h>
#include "../structReceiveTranslate.h"

SlaveInfo MyReceiveInfo;
MasterInfo MyTranslateInfo;

void ReceiveStructHandler() { //функция обработки данных полей структуры.
	// обрабатываем данные нашей структуры 
}

void initReceiveInfo() {
	// Инициализируем стартовые данные нашей структуры
}

#define ADDRESS 22
#define TRANSMIT_PIN 2 // пин, управляющий направлением приема/передачи (DIR) платы-удлинителя.
UExtSlave messager(ADDRESS, TRANSMIT_PIN, sizeof(MyReceiveInfo), ReceiveStructHandler);

void timer_ev() {
	interrupts();
	const uint8_t* buf = messager.messageHandler();
	if (!buf) { memcpy(&MyReceiveInfo, buf, sizeof(MyReceiveInfo));  } // копируем полученные данные в нашу приемную структуру.
}

void setup() 
{ 
	initReceiveInfo();
	messager.setup();
	MsTimer2::set(TIME_UPDATE_SLAVE, timer_ev);
	MsTimer2::start();
} 
 
void loop() 
{ 
	messager.structHandler();
	// что-то делаем с нашими полученными данными, 
	// формируем данные в структуре отправки
	messager.sendMessage(&MyTranslateInfo, sizeof(MyTranslateInfo)); // и отправляем их запрашивающему
	// можно отправлять сразу после запроса, а новые данные формировать после отправки старых.
} 

