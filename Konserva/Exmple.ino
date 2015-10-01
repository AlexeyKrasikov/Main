#include "Debug.h"
#include <SPI.h>
#include <Ethernet.h>										
#include <PubSubClient.h>
#include <Wire.h>	
#include <ExtPins.h>										
#include <artl.h>
#include <MFRC522.h>
#include <MsTimer2.h>
#include <avr/pgmspace.h>
#include <CollectorDriver.h>
#include <Konserva.h>

#include <avr/wdt.h>
#include <WiznetWatchdog.h>

void onTopicUpdate(char* topic, byte* payload, unsigned int length);

uint8_t		mac[]    		= 	{0xXX, 0xXX, 0xXX, 0xXX, 0xXX, 0xXX},
			serverName[]	= 	{..., ..., ..., ...};

const PROGMEM char topicGeneralState[] 		= "konserva/GeneralDevice/state";
const PROGMEM char topicGeneralCommands[] 	= "konserva/GeneralDevice/commands";

char		* const clientName 				= "KONSERVA";

EthernetClient ethClient;
PubSubClient client(serverName, 1883, onTopicUpdate, ethClient);
WiznetWatchdog wiznetWatchdog(A0, mac, client, clientName, 
                             topicGeneralState, topicGeneralCommands);


Konserva<PIN_T, PIN_T, PIN_T, PIN_T> konserva(client, PIN_2, PIN_3, PIN_6, PIN_5, 5000, 5000);

void setup() {}

void loop() 
{
	wiznetWatchdog.loop(); 		wdt_reset();
	// Проверка сообщений с сервера
	mqttLoop();					wdt_reset();

	konserva.loop();			wdt_reset();
}

void mqttLoop()
{
	wdt_reset();
	if(client.loop()) { return; }

	wiznetWatchdog.onMQTTReconnect();	

	konserva.onMQTTReconnect();	
}

void onTopicUpdate(char* topic, byte* payload, unsigned int length)
{
	wdt_reset();
	char* message = (char*)malloc(length+1);

	memcpy(message, payload, length);				

	konserva.onTopicUpdate(topic, message);

	free(message);
}
