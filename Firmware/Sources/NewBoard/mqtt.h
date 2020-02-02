#ifndef MQTT_H

#define MQTT_H

#include "processes.h"
#include "connectwifi.h"
#include "settings.h"

#define MQTT_OK 0
#define MQTT_OFF 1
#define MQTT_STARTING 2
#define MQTT_ERROR_NO_WIFI -1
#define MQTT_ERROR_BAD_PROTOCOL -2
#define MQTT_ERROR_BAD_CLIENT_ID -3
#define MQTT_ERROR_CONNECT_UNAVAILABLE -4
#define MQTT_ERROR_BAD_CREDENTIALS -5
#define MQTT_ERROR_CONNECT_UNAUTHORIZED -6
#define MQTT_ERROR_CONNECT_FAILED -7
#define MQTT_ERROR_CONNECT_ERROR -8
#define MQTT_ERROR_CONNECT_MESSAGE_FAILED -9
#define MQTT_ERROR_LOOP_FAILED -10

#define MQTT_CONNECT_RETRY_INTERVAL_MSECS 5000

#define MQTT_USER_NAME_LENGTH 100
#define MQTT_PASSWORD_LENGTH 200
#define MQTT_TOPIC_LENGTH 150

#define MQTT_NO_OF_RETRIES 3

#define MQTT_STATUS_TRANSMIT_OK_MESSAGE_NUMBER 21
#define MQTT_STATUS_TRANSMIT_OK_MESSAGE_TEXT "Mqtt transmit OK"

#define MQTT_STATUS_TRANSMIT_TIMOUT_MESSAGE_NUMBER 22
#define MQTT_STATUS_TRANSMIT_TIMEOUT_MESSAGE_TEXT "Mqtt transmit timeout"

struct MqttSettings
{
	char mqttDeviceName[DEVICE_NAME_LENGTH];
	char mqttServer[SERVER_NAME_LENGTH];
	boolean mqttSecureSockets;
	int mqttPort;
	char mqttUser[MQTT_USER_NAME_LENGTH];
	char mqttPassword[MQTT_PASSWORD_LENGTH];
	char mqttPublishTopic[MQTT_TOPIC_LENGTH];
	char mqttSubscribeTopic[MQTT_TOPIC_LENGTH];
	char mqttReportTopic[MQTT_TOPIC_LENGTH];

	int mqttSecsPerUpdate;
	int seconds_per_mqtt_retry;
	boolean mqtt_enabled;
};

extern struct MqttSettings mqttSettings;

extern struct SettingItemCollection mqttSettingItems;

boolean publishBufferToMQTT(char * buffer);
extern struct process * activeMQTTProcess;

int startMQTT(struct process * mqttProcess);
int stopMQTT(struct process * mqttProcess);
int updateMQTT(struct process * mqttProcess);
void mqttStatusMessage(struct process * mqttProcess, char * buffer, int bufferLength);

#endif