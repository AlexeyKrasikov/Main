#include <SPI.h>
#include <Ethernet.h>
#include <PubSubClient.h>

#include <avr/wdt.h>
#include <WiznetWatchdogStaticIP.h>

#include <IO_mqttShell.h>

#include "../structReceiveTranslate.h"
#include "IO_motor.h"

EthernetClient ethClient;

byte mqttServer[] = { 10, 1, 10, 15 }; // egypt
void onTopicUpdate(char * topic, byte * payload, unsigned int length);
PubSubClient mqttClient(mqttServer, 1883, onTopicUpdate, ethClient);

using namespace IO;
PubSubClient &MqttShell::_mqttClient = mqttClient;

char mqttClientName[] = "StatueOfAnubisDevice";
byte mac[] = { 0xC0, 0x03, 0x07, 0x07, 0x07, 0x01 };
byte ip[] = { 10, 1, 20, 6 };
byte dnsServer[] = { 0, 0, 0, 0 };
byte gateway[] = { 10, 1, 10, 2 };
byte subnet[] = { 255, 255, 0, 0 };
const PROGMEM char DEVICE_TS[] = "egypt/StatueOfAnubisDevice/state";
const PROGMEM char DEVICE_TC[] = "egypt/StatueOfAnubisDevice/commands";
WiznetWatchdogStaticIP wiznetWatchdog(A0, mac, ip, dnsServer, gateway, subnet, mqttClient, mqttClientName, DEVICE_TS, DEVICE_TC);

toSlave   rxData;
toMaster  txData;

void setup()
{
  rxData.motorCmd[0] = xFWD;
  txData.motorSt[0] = SWITCH_ERR;
  Serial.begin(9600);
}

void onTopicUpdate(char * topic, byte * payload, unsigned int length)
{
  char message[MQTT_MAX_PACKET_SIZE + 1];
  memcpy(message, payload, length);
  message[length] = '\0';
  MqttShell::forEachTopicUpdate(topic, message);
}

void mqttReconnect()
{
  
  if (mqttClient.loop()) {
    wdt_reset();
    return;
  }
  wiznetWatchdog.onMQTTReconnect();
  wdt_reset();
  MqttShell::forEachOnReconnect();
  wdt_reset();
}

void loop()
{
  mqttReconnect();
  wiznetWatchdog.loop();
  wdt_reset();
  MqttShell::forEachLoop();
  wdt_reset();
}