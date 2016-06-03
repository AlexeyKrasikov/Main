#include <SPI.h>                                            
#include <Ethernet.h>                                       
#include <PubSubClient.h>                                   
#include <MsTimer2.h>                                   
#include <avr/pgmspace.h>
#include <avr/wdt.h>
#include <WiznetWatchdog.h> 
#include <WiznetWatchdogStaticIP.h>
#include <CRC.h>
#include <uartExtension.h>
#include "../structReceiveTranslate.h"

#define IN_FLASH    const PROGMEM char

SlaveInfo MyTranslateInfo;
MasterInfo MyReceiveInfo; 

void 	onTopicUpdate(char *, byte *, unsigned int);

uint8_t 	mac[] = { 0xC0, 0x03, 0xBB, 0x4F, 0xD0, 0xB2 },
server[] = { 10, 1, 10, 8 };
byte		ip[] = { 10, 1, 47, 2 }; // ip адрес устройства
byte		mydns[] = { 0, 0, 0, 0 };	// dns сервер в сети если не используется то byte mydns[] = {0,0,0,0};
byte		gateway[] = { 10, 1, 10, 2 };	// ip адрес основного шлюза
byte		subnet[] = { 255, 255, 0 ,0 };	// маска подсети
char		* const clientName = "TEST";

EthernetClient ethClient;
PubSubClient mqttClient(server, 1883, onTopicUpdate, ethClient);

IN_FLASH	topicGeneralState[] = "test/TEST_Device/state";
IN_FLASH	topicGeneralCommands[] = "test/TEST_Device/commands";

WiznetWatchdogStaticIP  wiznetWatchdog(A0,
	mac, ip, mydns, gateway, subnet,
	mqttClient, clientName,
	topicGeneralState, topicGeneralCommands);

void ReceiveStructHandler() { //функция обработки данных полей структуры.
	// обрабатываем данные нашей структуры 
}

void initReceiveInfo() {
	// Инициализируем стартовые данные нашей структуры
}

#define ADDRESS_SLAVE 22
#define TRANSMIT_PIN 2 // пин, управляющий направлением приема/передачи (DIR) платы-удлинителя.
UExtMaster messager(TRANSMIT_PIN, sizeof(MyReceiveInfo), ReceiveStructHandler);

void setup() 
{ 
	initReceiveInfo();
	messager.setup();
} 
 
void loop() 
{ 
	mqttLoop();                     wdt_reset();    // Проверка сообщений с сервера
	wiznetWatchdog.loop();          wdt_reset();
	// формируем данные в структуре отправки
	if (!messager.isAnswer() { // если не ожидаем ответа от слейва
		messager.sendMessage(ADDRESS_SLAVE, &MyTranslateInfo, sizeof(MyTranslateInfo)); // то отправляем данные-команды
	}

	const uint8_t* buf = messager.messageHandler(ADDRESS_SLAVE); // обрабатываем сообщения от устройства с заданным адресом
	// if (buf != 0), то значит, что-то пришло.
	if (!buf) { 
		memcpy(&MyReceiveInfo, buf, sizeof(MyReceiveInfo));  // копируем полученные данные в нашу приемную структуру.
		messager.structHandler();	// что-то делаем с полученными в ответ данными 	
	}
} 

void mqttLoop()
{
	if (mqttClient.loop()) { return; }
	wiznetWatchdog.onMQTTReconnect();               wdt_reset();
}

void onTopicUpdate(char* topic, byte* payload, unsigned int length)
{
	char* message = (char*)malloc(length + 1);
	memcpy(message, payload, length);
	message[length] = '\0';
	wiznetWatchdog.onTopicUpdate(topic, message);  	wdt_reset();
	free(message);
}

