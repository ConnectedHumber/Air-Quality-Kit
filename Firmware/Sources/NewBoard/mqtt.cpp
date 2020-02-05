#include <Arduino.h>
#include <DNSServer.h>
#include <WiFiUdp.h>
#include <WiFiServer.h>
#include <WiFiClient.h>
#include <WiFiClientSecure.h>

#include "utils.h"
#include "processes.h"
#include "settings.h"
#include "messages.h"

#include "mqtt.h"

#include <PubSubClient.h>

struct MqttSettings mqttSettings;

char* defaultMQTTName = "NewMQTTDevice";
char* defaultMQTTHost = "mqtt.connectedhumber.org";

void setDefaultMQTTTname(void* dest)
{
	char* destStr = (char*)dest;
	snprintf(destStr, DEVICE_NAME_LENGTH, "Sensor-%06x", (unsigned long) ESP.getEfuseMac());
}

void setDefaultMQTThost(void* dest)
{
	strcpy((char*)dest, "mqtt.connectedhumber.org");
}

boolean validateMQTThost(void* dest, const char* newValueStr)
{
	return (validateString((char*)dest, newValueStr, SERVER_NAME_LENGTH));
}

void setDefaultMQTTport(void* dest)
{
	int* destInt = (int*)dest;
	*destInt = 1883; // use 8883 for secure connection to Azure IoT hub
}

boolean validateMQTTtopic(void* dest, const char* newValueStr)
{
	return (validateString((char*)dest, newValueStr, MQTT_TOPIC_LENGTH));
}

void setDefaultMQTTusername(void* dest)
{
	strcpy((char*)dest, "connectedhumber");
}

boolean validateMQTTusername(void* dest, const char* newValueStr)
{
	return (validateString((char*)dest, newValueStr, MQTT_USER_NAME_LENGTH));
}

boolean validateMQTTPWD(void* dest, const char* newValueStr)
{
	return (validateString((char*)dest, newValueStr, MQTT_PASSWORD_LENGTH));
}

void setDefaultMQTTpublishTopic(void* dest)
{
	snprintf((char*)dest, MQTT_TOPIC_LENGTH, "airquality/data/Sensor-%06x", (unsigned long) ESP.getEfuseMac());
}

void setDefaultMQTTsubscribeTopic(void* dest)
{
	snprintf((char*)dest, MQTT_TOPIC_LENGTH, "airquality/command/Sensor-%06x", (unsigned long)ESP.getEfuseMac());
}

void setDefaultMQTTreportTopic(void* dest)
{
	snprintf((char*)dest, MQTT_TOPIC_LENGTH, "airquality/report/Sensor-%06x", (unsigned long)ESP.getEfuseMac());
}

void setDefaultMQTTsecsPerUpdate(void* dest)
{
	int* destInt = (int*)dest;
	*destInt = 360;
}

void setDefaultMQTTsecsPerRetry(void* dest)
{
	int* destInt = (int*)dest;
	*destInt = 10;
}

void setDefaultMQTTDeviceName(void* dest)
{
	char* destStr = (char*)dest;
	snprintf(destStr, DEVICE_NAME_LENGTH, "Sensor-%06x", ESP.getEfuseMac());
}

boolean validateMQTTDeviceName(void* dest, const char* newValueStr)
{
	return (validateString((char*)dest, newValueStr, DEVICE_NAME_LENGTH));
}

struct SettingItem mqttDeviceNameSetting = {
	"MQTT Device name", "mqttdevicename", mqttSettings.mqttDeviceName, DEVICE_NAME_LENGTH, text, setDefaultMQTTDeviceName , validateMQTTDeviceName };

struct SettingItem mqttOnOffSetting = {
	"MQTT Active (yes or no)", "mqttactive",&mqttSettings.mqtt_enabled, ONOFF_INPUT_LENGTH, yesNo, setFalse, validateYesNo };

struct SettingItem mqttServerSetting = {
	"MQTT Host", "mqtthost", mqttSettings.mqttServer, SERVER_NAME_LENGTH, text, setDefaultMQTThost, validateServerName };

struct SettingItem mqttPortSetting = {
	"MQTT Port number", "mqttport", &mqttSettings.mqttPort, NUMBER_INPUT_LENGTH, integerValue, setDefaultMQTTport, validateInt };

struct SettingItem mqttSecureSocketsSetting = {
	"MQTT Secure sockets active (yes or no)", "mqttsecure", &mqttSettings.mqttSecureSockets, YESNO_INPUT_LENGTH, onOff, setFalse, validateYesNo };

struct SettingItem mqttUserSetting = {
	"MQTT UserName", "mqttuser", mqttSettings.mqttUser, MQTT_USER_NAME_LENGTH, text, setDefaultMQTTusername, validateMQTTusername };

struct SettingItem mqttPasswordSetting = {
	"MQTT Password", "mqttpwd", mqttSettings.mqttPassword, MQTT_PASSWORD_LENGTH, password, setEmptyString, validateMQTTPWD };

struct SettingItem mqttPublishTopicSetting = {
	"MQTT Publish topic", "mqttpub", mqttSettings.mqttPublishTopic, MQTT_TOPIC_LENGTH, text, setDefaultMQTTpublishTopic, validateMQTTtopic };

struct SettingItem mqttSubscribeTopicSetting = {
	"MQTT Subscribe topic", "mqttsub", mqttSettings.mqttSubscribeTopic, MQTT_TOPIC_LENGTH, text, setDefaultMQTTsubscribeTopic, validateMQTTtopic };

struct SettingItem mqttReportTopicSetting = {
	"MQTT Reporting topic", "mqttreport", mqttSettings.mqttReportTopic, MQTT_TOPIC_LENGTH, text, setDefaultMQTTreportTopic, validateMQTTtopic };

struct SettingItem mqttSecsPerUpdateSetting = {
	"MQTT Seconds per update", "mqttsecsperupdate", &mqttSettings.mqttSecsPerUpdate, NUMBER_INPUT_LENGTH, integerValue, setDefaultMQTTsecsPerUpdate, validateInt };

struct SettingItem seconds_per_mqtt_retrySetting = {
"MQTT Seconds per retry", "mqttsecsperretry", &mqttSettings.seconds_per_mqtt_retry, NUMBER_INPUT_LENGTH, integerValue, setDefaultMQTTsecsPerRetry, validateInt };


struct SettingItem* mqttSettingItemPointers[] =
{
	&mqttDeviceNameSetting,
	&mqttOnOffSetting,
	&mqttServerSetting,
	&mqttPortSetting,
	&mqttSecureSocketsSetting,
	&mqttUserSetting,
	&mqttPasswordSetting,
	&mqttPublishTopicSetting,
	&mqttSubscribeTopicSetting,
	&mqttReportTopicSetting,
	&mqttSecsPerUpdateSetting,
	&seconds_per_mqtt_retrySetting
};

struct SettingItemCollection mqttSettingItems = {
	"MQTT",
	"MQTT host, username, password and connection topics",
	mqttSettingItemPointers,
	sizeof(mqttSettingItemPointers) / sizeof(struct SettingItem*)
};


unsigned long mqtt_timer_start;

Client* espClient = NULL;

PubSubClient* mqttPubSubClient = NULL;

#define MQTT_RECEIVE_BUFFER_SIZE 240
char mqtt_receive_buffer[MQTT_RECEIVE_BUFFER_SIZE];

#define MQTT_SEND_BUFFER_SIZE 240

char mqtt_send_buffer[MQTT_SEND_BUFFER_SIZE];

boolean first_mqtt_message = true;

int messagesSent;
int messagesReceived;

void mqtt_deliver_command_result(char* result)
{
	publishBufferToMQTT(result);
}


void callback(char* topic, byte* payload, unsigned int length)
{
	int i;
	for (i = 0; i < length; i++) {
		mqtt_receive_buffer[i] = (char)payload[i];
	}

	// Put the terminator on the string
	mqtt_receive_buffer[i] = 0;

	act_onJson_command(mqtt_receive_buffer, mqtt_deliver_command_result);

	Serial.printf("Received from MQTT: %s\n", mqtt_receive_buffer);

	messagesReceived++;
}

int mqttConnectErrorNumber;

struct process* mqttWiFiProcess = NULL;
struct process* activeMQTTProcess;

int startMQTT(struct process* mqttProcess)
{
	activeMQTTProcess = mqttProcess;

	if (mqttWiFiProcess == NULL)
	{
		mqttWiFiProcess = findProcessByName("WiFi");
	}

	mqttProcess->status = MQTT_STARTING;

	return mqttProcess->status;
}

int restartMQTT()
{
	messagesReceived = 0;
	messagesSent = 0;

	if (mqttWiFiProcess->status != WIFI_OK)
	{
		activeMQTTProcess->status = MQTT_ERROR_NO_WIFI;
		return MQTT_ERROR_NO_WIFI;
	}

	if (espClient == NULL)
	{
		if (mqttSettings.mqttSecureSockets)
		{
			espClient = new WiFiClientSecure();
		}
		else
		{
			espClient = new WiFiClient();
		}

		mqttPubSubClient = new PubSubClient(*espClient);

		mqttPubSubClient->setServer(mqttSettings.mqttServer, mqttSettings.mqttPort);
		mqttPubSubClient->setCallback(callback);
	}

	if (!mqttPubSubClient->connect(mqttSettings.mqttDeviceName, mqttSettings.mqttUser, mqttSettings.mqttPassword))
	{
		switch (mqttPubSubClient->state())
		{
		case MQTT_CONNECT_BAD_PROTOCOL:
			activeMQTTProcess->status = MQTT_ERROR_BAD_PROTOCOL;
			break;
		case MQTT_CONNECT_BAD_CLIENT_ID:
			activeMQTTProcess->status = MQTT_ERROR_BAD_CLIENT_ID;
			break;
		case MQTT_CONNECT_UNAVAILABLE:
			activeMQTTProcess->status = MQTT_ERROR_CONNECT_UNAVAILABLE;
			break;
		case MQTT_CONNECT_BAD_CREDENTIALS:
			activeMQTTProcess->status = MQTT_ERROR_BAD_CREDENTIALS;
			break;
		case MQTT_CONNECT_UNAUTHORIZED:
			activeMQTTProcess->status = MQTT_ERROR_CONNECT_UNAUTHORIZED;
			break;
		case MQTT_CONNECTION_TIMEOUT:
			activeMQTTProcess->status = MQTT_ERROR_BAD_PROTOCOL;
			break;
		case MQTT_CONNECT_FAILED:
			activeMQTTProcess->status = MQTT_ERROR_CONNECT_FAILED;
			break;
		default:
			mqttConnectErrorNumber = mqttPubSubClient->state();
			activeMQTTProcess->status = MQTT_ERROR_CONNECT_ERROR;
			break;
		}
		return activeMQTTProcess->status;
	}

	mqttPubSubClient->subscribe(mqttSettings.mqttSubscribeTopic);

	//snprintf(mqtt_send_buffer, MQTT_SEND_BUFFER_SIZE,
	//	"{\"dev\":\"%s\", \"status\":\"starting\"}",
	//	mqttSettings.deviceName);

	//if (!mqttPubSubClient->publish(mqttSettings.mqttReportTopic, mqtt_send_buffer))
	//{
	//	Serial.println("publish failed");
	//	mqttProcess->status = MQTT_ERROR_CONNECT_MESSAGE_FAILED;
	//	return MQTT_ERROR_CONNECT_MESSAGE_FAILED;
	//}

	activeMQTTProcess->status = MQTT_OK;
	return MQTT_OK;
}

int mqttRetries = 0;

boolean publishBufferToMQTT(char* buffer)
{
	if (activeMQTTProcess->status == MQTT_OK)
	{
		messagesSent++;
		mqttRetries = 0;
		Serial.println("Publishing message");
		displayMessage(MQTT_STATUS_TRANSMIT_OK_MESSAGE_NUMBER, MQTT_STATUS_TRANSMIT_OK_MESSAGE_TEXT);

		return mqttPubSubClient->publish(mqttSettings.mqttPublishTopic, buffer);
	}

	Serial.println("Not publishing message");

	mqttRetries++;

	if (mqttRetries >= MQTT_NO_OF_RETRIES)
	{
		displayMessage(MQTT_STATUS_TRANSMIT_TIMOUT_MESSAGE_NUMBER, MQTT_STATUS_TRANSMIT_TIMEOUT_MESSAGE_TEXT);
		forceSensorShutdown();
		// if we get here we have not been shut down - need to do some more retries
		mqttRetries = 0;
	}

	return false;
}

int stopMQTT(struct process* mqttProcess)
{
	// don't do anything because we are all going to die anyway
	return MQTT_OFF;
}

unsigned long timeOfLastMQTTsuccess = 0;


int updateMQTT(struct process* mqttProcess)
{
	switch (mqttProcess->status)
	{

	case MQTT_OK:

		if (WiFi.status() != WL_CONNECTED)
		{
			mqttProcess->status = MQTT_ERROR_NO_WIFI;
			mqttPubSubClient->disconnect();
		}

		timeOfLastMQTTsuccess = offsetMillis();

		if (!mqttPubSubClient->loop())
		{
			mqttPubSubClient->disconnect();
			mqttProcess->status = MQTT_ERROR_LOOP_FAILED;
		}

		break;

	case MQTT_OFF:
		break;

	case MQTT_STARTING:
		mqttProcess->status = restartMQTT();
		break;

	case MQTT_ERROR_NO_WIFI:
		if (mqttWiFiProcess->status == WIFI_OK)
		{
			mqttProcess->status = restartMQTT();
		}
		break;

	case MQTT_ERROR_BAD_PROTOCOL:
	case MQTT_ERROR_BAD_CLIENT_ID:
	case MQTT_ERROR_CONNECT_UNAVAILABLE:
	case MQTT_ERROR_BAD_CREDENTIALS:
	case MQTT_ERROR_CONNECT_UNAUTHORIZED:
	case MQTT_ERROR_CONNECT_FAILED:
	case MQTT_ERROR_CONNECT_ERROR:
	case MQTT_ERROR_CONNECT_MESSAGE_FAILED:
	case MQTT_ERROR_LOOP_FAILED:
		if (ulongDiff(offsetMillis(), timeOfLastMQTTsuccess) > MQTT_CONNECT_RETRY_INTERVAL_MSECS)
		{
			mqttProcess->status = restartMQTT();
			timeOfLastMQTTsuccess = offsetMillis();
		}
		break;

	default:
		break;
	}

	return mqttProcess->status;
}

void mqttStatusMessage(struct process* mqttProcess, char* buffer, int bufferLength)
{
	switch (mqttProcess->status)
	{
	case MQTT_OK:
		snprintf(buffer, bufferLength, "MQTT OK sent: %d rec: %d", messagesSent, messagesReceived);
		break;
	case MQTT_STARTING:
		snprintf(buffer, bufferLength, "MQTT Starting");
		break;
	case MQTT_OFF:
		snprintf(buffer, bufferLength, "MQTT OFF");
		break;
	case MQTT_ERROR_NO_WIFI:
		snprintf(buffer, bufferLength, "MQTT error no WiFi");
		break;
	case MQTT_ERROR_BAD_PROTOCOL:
		snprintf(buffer, bufferLength, "MQTT error bad protocol");
		break;
	case MQTT_ERROR_BAD_CLIENT_ID:
		snprintf(buffer, bufferLength, "MQTT error bad client ID");
		break;
	case MQTT_ERROR_CONNECT_UNAVAILABLE:
		snprintf(buffer, bufferLength, "MQTT error connect unavailable");
		break;
	case MQTT_ERROR_BAD_CREDENTIALS:
		snprintf(buffer, bufferLength, "MQTT error bad credentials");
		break;
	case MQTT_ERROR_CONNECT_UNAUTHORIZED:
		snprintf(buffer, bufferLength, "MQTT error connect unauthorized");
		break;
	case MQTT_ERROR_CONNECT_FAILED:
		snprintf(buffer, bufferLength, "MQTT error connect failed");
		break;
	case MQTT_ERROR_CONNECT_ERROR:
		snprintf(buffer, bufferLength, "MQTT error connect error %d", mqttConnectErrorNumber);
		break;
	case MQTT_ERROR_CONNECT_MESSAGE_FAILED:
		snprintf(buffer, bufferLength, "MQTT error connect message failed");
		break;
	case MQTT_ERROR_LOOP_FAILED:
		snprintf(buffer, bufferLength, "MQTT error loop failed");
		break;
	default:
		snprintf(buffer, bufferLength, "MQTT failed but I'm not sure why");
		break;
	}
}

