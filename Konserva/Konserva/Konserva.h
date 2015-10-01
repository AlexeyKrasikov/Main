#ifndef __KONSERVA_H__
#define __KONSERVA_H__

/*-------------------------------------------------------------------------------------------------------
Типы возвращаемых значений методов loop()
return -1	- 	функция loop не задействована
return 0 	- 	удачное завершение. Сработал концевик ожидаемого направления.
return 1 	- 	информационное. Двигатель находится в неопределенном положении между концевиками 
return 2 	- 	ошибка. Не отжался концевик исходного состояния двигателя.
return 3 	- 	ошибка. Не правильное направление вращения. Сработал противоположный концевик.
return 4 	- 	ошибка. Зажаты оба концевика. 
return 5 	- 	ошибка. Ток двигателя превысил максимальное значение.
return 6 	- 	ошибка. Таймаут. Вышло время максимальной работы двигателя. Ни один концевик не сработал.
-------------------------------------------------------------------------------------------------------*/
#include <DEBUG.H>
#include <SPI.h>
#include <Ethernet.h>										
#include <PubSubClient.h>
#include <ExtPins.h>
#include <artl.h>
#include <avr/pgmspace.h>
#include <avr/wdt.h>
#include <CollectorDriver.h>

#define KONSERVA_MAX_TOPIC_LENTH 64

const PROGMEM char topicKonservaState[] 			= "timemashine/Present_Past_Konserva/state";
const PROGMEM char topicKonservaCommands[]			= "timemashine/Present_Past_Konserva/commands";
const PROGMEM char topicKonservaSwitchesState[] 	= "timemashine/Present_Past_Konserva_Switches/state";
const PROGMEM char topicKonservaSwitchesCommands[]	= "timemashine/Present_Past_Konserva_Switches/commands";
const PROGMEM char topicKonservaErrorState[] 		= "timemashine/Present_Past_Konserva_Error/state";
const PROGMEM char topicKonservaErrorCommands[]		= "timemashine/Present_Past_Konserva_Error/commands";

const PROGMEM char commandKonservaForce[]			= "FORCE";

const PROGMEM char commandKonservaOpen[]			= "OPEN";
const PROGMEM char commandKonservaClose[]			= "CLOSE";
const PROGMEM char commandKonservaStop[]			= "STOP";

const PROGMEM char stateKonservaMoveToOpen[] 		= "MOVE TO OPEN";
const PROGMEM char stateKonservaMoveToClose[] 		= "MOVE TO CLOSE";
const PROGMEM char stateKonservaStop[] 				= "STOP";

const PROGMEM char stateSwitchesOpen[] 				= "OPEN";
const PROGMEM char stateSwitchesClosed[] 			= "CLOSED";
const PROGMEM char stateSwitchesUndefined[] 		= "UNDEFINED";
const PROGMEM char stateSwitchesError[] 			= "ERROR";

const PROGMEM char stateErrorTimeoutOpenSwitchReleased[] 	= "ERROR TIMEOUT. OPEN SWITCH NOT RELEASED";
const PROGMEM char stateErrorTimeoutCloseSwitchReleased[] 	= "ERROR TIMEOUT. CLOSE SWITCH NOT RELEASED";
const PROGMEM char stateErrorDirection[] 					= "ERROR DIRECTION";
const PROGMEM char stateErrorBlockedBothSwitches[] 			= "ERROR BLOCKED BOTH SWITCHES";
const PROGMEM char stateErrorTimeoutMoveToOpen[] 			= "ERROR TIMEOUT MOVE TO OPEN";
const PROGMEM char stateErrorTimeoutMoveToClose[] 			= "ERROR TIMEOUT MOVE TO CLOSE";


template<	typename TOpen, typename TClose, typename TSensorOpen, typename TSensorClose, 
			bool invOpen = !INVERT, bool invClose = !INVERT, 
			bool invSensorOpen = INVERT, bool invSensorClose = INVERT >
class Konserva
{
public:
	Konserva(	PubSubClient &client, TOpen pinOpen, TClose pinClose, 
				TSensorOpen pinSensorOpen, TSensorClose pinSensorClose, 
				uint32_t timeOpen, uint32_t timeClose, uint16_t offSwitchTime = 500 );
	~Konserva() {};

	void 	loop(),
			onMQTTReconnect(),
			onTopicUpdate(char* topic, char* message),
			sendKonservaState(),
			sendSwithesState(),
			sendErrorState();

protected:
	PubSubClient &m_client;

	LogicOut<TOpen, invOpen> m_Open;
	LogicOut<TClose, invClose> m_Close;
	DebouncedLogicIn<TSensorOpen, invSensorOpen> m_SensorOpen;
	DebouncedLogicIn<TSensorClose, invSensorClose> m_SensorClose;

	SCD m_konserva;

	char m_returnedValue;
	char m_lastError;
};


template<	typename TOpen, typename TClose, typename TSensorOpen, typename TSensorClose,
			bool invOpen, bool invClose, bool invSensorOpen, bool invSensorClose >
inline
		Konserva<	TOpen, TClose, TSensorOpen, TSensorClose, 
					invOpen, invClose, invSensorOpen, invSensorClose>::
		Konserva(	PubSubClient &client, TOpen pinOpen, TClose pinClose, 
					TSensorOpen pinSensorOpen, TSensorClose pinSensorClose, 
					uint32_t timeOpen, uint32_t timeClose, uint16_t offSwitchTime )

				: 	m_client(client), m_Open(pinOpen), m_Close(pinClose), 
					m_SensorOpen(pinSensorOpen, PIN_INPUT_PULLUP, 50), 
					m_SensorClose(pinSensorClose, PIN_INPUT_PULLUP, 50),
					m_konserva(	m_Open, m_Close, m_SensorOpen, m_SensorClose, 
								timeOpen, timeClose, offSwitchTime),
					m_returnedValue(-1), m_lastError(0)
{}


template<	typename TOpen, typename TClose, typename TSensorOpen, typename TSensorClose,
			bool invOpen, bool invClose, bool invSensorOpen, bool invSensorClose >
inline
void 
		Konserva<	TOpen, TClose, TSensorOpen, TSensorClose,
					invOpen, invClose, invSensorOpen, invSensorClose>::
		loop()
{
	static CD::CurrentState 	oldSwitchesState = m_konserva.getCurrentState();
	static char 				oldReturnedLoopValue = m_returnedValue;
	// static char 				oldErrorValue = m_lastError;
	char value;

	value = m_konserva.loop();
	assert2ln(value, DEC);

	if ((value == 0) || (value > 1)) { m_konserva.turnOff(); }

	if (value > 1) 			{ m_lastError = value; sendErrorState();}
	else /* (value <= 1) */ { m_returnedValue = value; }

	if (m_returnedValue != oldReturnedLoopValue) { 
		oldReturnedLoopValue = m_returnedValue; 
		sendKonservaState(); 
	}
	if (m_konserva.getCurrentState() != oldSwitchesState) { 
		oldSwitchesState = m_konserva.getCurrentState();
		sendSwithesState(); 
	}
	// if (m_lastError != oldErrorValue) { 
	// 	oldErrorValue = m_lastError;
	// 	sendErrorState(); 
	// }
}


template<	typename TOpen, typename TClose, typename TSensorOpen, typename TSensorClose,
			bool invOpen, bool invClose, bool invSensorOpen, bool invSensorClose >
inline
void 
		Konserva<	TOpen, TClose, TSensorOpen, TSensorClose,
					invOpen, invClose, invSensorOpen, invSensorClose>::
		onMQTTReconnect()
{
	char topic[KONSERVA_MAX_TOPIC_LENTH];

	m_client.subscribe(strcpy_P(topic, topicKonservaCommands));
	m_client.subscribe(strcpy_P(topic, topicKonservaSwitchesCommands));
	m_client.subscribe(strcpy_P(topic, topicKonservaErrorCommands));

	sendKonservaState();
	sendSwithesState();
	sendErrorState();
}


template<	typename TOpen, typename TClose, typename TSensorOpen, typename TSensorClose,
			bool invOpen, bool invClose, bool invSensorOpen, bool invSensorClose >
inline
void 
		Konserva<	TOpen, TClose, TSensorOpen, TSensorClose,
					invOpen, invClose, invSensorOpen, invSensorClose>::
		onTopicUpdate(char* topic, char* message)
{
	if (!strcmp_P(topic, topicKonservaCommands)) {
		if (!strcmp_P(message, commandKonservaOpen)) {
			m_konserva.turnOn(CD::OPEN);
		}

		if (!strcmp_P(message, commandKonservaClose)) {
			m_konserva.turnOn(CD::CLOSE);
		}

		if (!strcmp_P(message, commandKonservaStop)) {
			m_konserva.turnOff();
		}

		if (!strcmp_P(message, commandKonservaForce)) {
			sendKonservaState();
		}
	}

	if (!strcmp_P(topic, topicKonservaSwitchesCommands)) {
		if (!strcmp_P(message, commandKonservaForce)) {
			sendSwithesState();
		}
	} 

	if (!strcmp_P(topic, topicKonservaErrorCommands)) {
		if (!strcmp_P(message, commandKonservaForce)) {
			sendErrorState();
		}
	}
}


template<	typename TOpen, typename TClose, typename TSensorOpen, typename TSensorClose,
			bool invOpen, bool invClose, bool invSensorOpen, bool invSensorClose >
void 
		Konserva<	TOpen, TClose, TSensorOpen, TSensorClose,
					invOpen, invClose, invSensorOpen, invSensorClose>::
		sendKonservaState()
{
	char 	topic[KONSERVA_MAX_TOPIC_LENTH],
			message[KONSERVA_MAX_TOPIC_LENTH];

	strcpy_P(topic, topicKonservaState);

	switch (m_returnedValue) {
	case (-1): 
		m_client.publish(topic,	strcpy_P(message, stateKonservaStop));
		break;
	case 1:
		if (m_konserva.getTurnState() == CD::OPEN) {
			m_client.publish(topic, strcpy_P(message, stateKonservaMoveToOpen)); }
		if (m_konserva.getTurnState() == CD::CLOSE) {
			m_client.publish(topic, strcpy_P(message, stateKonservaMoveToClose)); }
		break;
	}
}


template<	typename TOpen, typename TClose, typename TSensorOpen, typename TSensorClose,
			bool invOpen, bool invClose, bool invSensorOpen, bool invSensorClose >
void 
		Konserva<	TOpen, TClose, TSensorOpen, TSensorClose,
					invOpen, invClose, invSensorOpen, invSensorClose>::
		sendSwithesState()
{
	char 	topic[KONSERVA_MAX_TOPIC_LENTH],
			message[KONSERVA_MAX_TOPIC_LENTH];

	strcpy_P(topic, topicKonservaSwitchesState);

	switch (m_konserva.getCurrentState()) {
	case CD::OPENED: 
		m_client.publish(topic, strcpy_P(message, stateSwitchesOpen));
		break;
	case CD::CLOSED:
		m_client.publish(topic, strcpy_P(message, stateSwitchesClosed));
		break;
	case CD::UNDEFINED: 
		m_client.publish(topic, strcpy_P(message, stateSwitchesUndefined));
		break;
	case CD::ERROR: 
		m_client.publish(topic, strcpy_P(message, stateSwitchesError));
		break;
	}
}

template<	typename TOpen, typename TClose, typename TSensorOpen, typename TSensorClose,
			bool invOpen, bool invClose, bool invSensorOpen, bool invSensorClose >
void 
		Konserva<	TOpen, TClose, TSensorOpen, TSensorClose,
					invOpen, invClose, invSensorOpen, invSensorClose>::
		sendErrorState()
{
	char 	topic[KONSERVA_MAX_TOPIC_LENTH],
			message[KONSERVA_MAX_TOPIC_LENTH];

	strcpy_P(topic, topicKonservaErrorState);

	switch (m_lastError) {
	case (2):
		if (m_konserva.getTurnState() == CD::OPEN) { 
			m_client.publish(topic,	strcpy_P(message, stateErrorTimeoutCloseSwitchReleased)); }
		if (m_konserva.getTurnState() == CD::CLOSE) { 
			m_client.publish(topic,	strcpy_P(message, stateErrorTimeoutOpenSwitchReleased)); }
		break;
	case (3):
		m_client.publish(topic, strcpy_P(message, stateErrorDirection));
		break;
	case (4):
		m_client.publish(topic, strcpy_P(message, stateErrorBlockedBothSwitches));
		break;
	case (6):
		if (m_konserva.getTurnState() == CD::OPEN) { 
			m_client.publish(topic,	strcpy_P(message, stateErrorTimeoutMoveToOpen)); }
		if (m_konserva.getTurnState() == CD::CLOSE) { 
			m_client.publish(topic,	strcpy_P(message, stateErrorTimeoutMoveToClose)); }
		break;
	}
}

#endif