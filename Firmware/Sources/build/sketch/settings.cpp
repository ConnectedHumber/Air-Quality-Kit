#include <EEPROM.h>
#include <Arduino.h>
#include "string.h"
#include "errors.h"
#include "settings.h"
#include "debug.h"
#include "ArduinoJson-v5.13.2.h"
#include "utils.h"
#include "lora.h"

struct Device_Settings settings;

void setEmptyString(void *dest)
{
	strcpy((char *)dest, "");
}

void setFalse(void *dest)
{
	boolean *destBool = (boolean *)dest;
	*destBool = false;
}

void setTrue(void *dest)
{
	boolean *destBool = (boolean *)dest;
	*destBool = true;
}

boolean validateOnOff(void *dest, const char *newValueStr)
{
	boolean *destBool = (boolean *)dest;

	if (strcasecmp(newValueStr, "on") == 0)
	{
		*destBool = true;
		return true;
	}

	if (strcasecmp(newValueStr, "off") == 0)
	{
		*destBool = false;
		return true;
	}

	return false;
}

boolean validateYesNo(void *dest, const char *newValueStr)
{
	boolean *destBool = (boolean *)dest;

	if (strcasecmp(newValueStr, "yes") == 0)
	{
		*destBool = true;
		return true;
	}

	if (strcasecmp(newValueStr, "no") == 0)
	{
		*destBool = false;
		return true;
	}

	return false;
}

boolean validateString(char *dest, const char *source, int maxLength)
{
	if (strlen(source) > (maxLength - 1))
		return false;

	strcpy(dest, source);

	return true;
}

boolean validateInt(void *dest, const char *newValueStr)
{
	int value;

	if (sscanf(newValueStr, "%d", &value) == 1)
	{
		*(int *)dest = value;
		return true;
	}

	return false;
}

boolean validateDouble(void *dest, const char *newValueStr)
{
	double value;

	if (sscanf(newValueStr, "%lf", &value) == 1)
	{
		*(double *)dest = value;
		return true;
	}

	return false;
}

void dump_hex(uint8_t *pos, int length)
{
	while (length > 0)
	{
		// handle leading zeroes
		if (*pos < 0x10)
		{
			TRACE("0");
		}
		TRACE_HEX(*pos);
		pos++;
		length--;
	}
	TRACELN();
}

char hex_digit(int val)
{
	if (val < 10)
	{
		return '0' + val;
	}
	else
	{
		return 'A' + (val - 10);
	}
}

void dumpHexString(char *dest, uint8_t *pos, int length)
{
	while (length > 0)
	{
		// handle leading zeroes

		*dest = hex_digit(*pos / 16);
		dest++;
		*dest = hex_digit(*pos % 16);
		dest++;
		pos++;
		length--;
	}
	*dest = 0;
}

void dumpUnsignedLong(char *dest, uint32_t value)
{
	// Write backwards to put least significant values in
	// the right place

	// move to the end of the string
	int pos = 8;
	// put the terminator in position
	dest[pos] = 0;
	pos--;

	while (pos > 0)
	{
		byte b = value & 0xff;
		dest[pos] = hex_digit(b % 16);
		pos--;
		dest[pos] = hex_digit(b / 16);
		pos--;
		value = value >> 8;
	}
}

int hexFromChar(char c, int *dest)
{
	if (c >= '0' && c <= '9')
	{
		*dest = (int)(c - '0');
		return WORKED_OK;
	}
	else
	{
		if (c >= 'A' && c <= 'F')
		{
			*dest = (int)(c - 'A' + 10);
			return WORKED_OK;
		}
		else
		{
			if (c >= 'a' && c <= 'f')
			{
				*dest = (int)(c - 'a' + 10);
				return WORKED_OK;
			}
		}
	}
	return INVALID_HEX_DIGIT_IN_VALUE;
}

#define MAX_DECODE_BUFFER_LENGTH 20

int decodeHexValueIntoBytes(uint8_t *dest, const char *newVal, int length)
{
	if (length > MAX_DECODE_BUFFER_LENGTH)
	{
		TRACELN("Incoming hex value will not fit in the buffer");
		return INCOMING_HEX_VALUE_TOO_BIG_FOR_BUFFER;
	}

	// Each hex value is in two bytes - make sure the incoming text is the right length

	int inputLength = strlen(newVal);

	if (inputLength != length * 2)
	{
		TRACELN("Incoming hex value is the wrong length");
		return INCOMING_HEX_VALUE_IS_THE_WRONG_LENGTH;
	}

	int pos = 0;

	u1_t buffer[MAX_DECODE_BUFFER_LENGTH];
	u1_t *bpos = buffer;

	while (pos < inputLength)
	{
		int d1, d2, reply;

		reply = hexFromChar(newVal[pos], &d1);
		if (reply != WORKED_OK)
			return reply;
		pos++;
		reply = hexFromChar(newVal[pos], &d2);
		if (reply != WORKED_OK)
			return reply;
		pos++;

		*bpos = (u1_t)(d1 * 16 + d2);
		bpos++;
	}

	// If we get here the buffer has been filled OK

	memcpy_P(dest, buffer, length);
	return WORKED_OK;
}

int decodeHexValueIntoUnsignedLong(u4_t *dest, const char *newVal)
{

	int inputLength = strlen(newVal);

	if (inputLength != 8)
	{
		TRACELN("Incoming hex value is the wrong length");
		return INCOMING_HEX_VALUE_IS_THE_WRONG_LENGTH;
	}

	int pos = 0;

	u4_t result = 0;

	while (pos < inputLength)
	{
		int d1, d2, reply;

		reply = hexFromChar(newVal[pos], &d1);
		if (reply != WORKED_OK)
			return reply;
		pos++;
		reply = hexFromChar(newVal[pos], &d2);
		if (reply != WORKED_OK)
			return reply;
		pos++;

		u4_t v = d1 * 16 + d2;
		result = result * 256 + v;
	}

	// If we get here the value has been received OK

	*dest = result;
	return WORKED_OK;
}

void setDefaultDevname(void *dest)
{
	char *destStr = (char *)dest;
	snprintf(destStr, DEVICE_NAME_LENGTH, "CHASW-%06x-1", ESP.getEfuseMac());
}

boolean validateDevName(void *dest, const char *newValueStr)
{
	return (validateString((char *)dest, newValueStr, DEVICE_NAME_LENGTH));
}

boolean validateWifiSSID(void *dest, const char *newValueStr)
{
	return (validateString((char *)dest, newValueStr, WIFI_SSID_LENGTH));
}

boolean validateWifiPWD(void *dest, const char *newValueStr)
{
	return (validateString((char *)dest, newValueStr, WIFI_PASSWORD_LENGTH));
}

struct SettingItem wifiSettingItems[] =
	{
		"Device name", "devname", settings.deviceName, DEVICE_NAME_LENGTH, text, setDefaultDevname, validateDevName,
		"WiFiSSID1", "wifissid1", settings.wifi1SSID, WIFI_SSID_LENGTH, text, setEmptyString, validateWifiSSID,
		"WiFiPassword1", "wifipwd1", settings.wifi1PWD, WIFI_PASSWORD_LENGTH, password, setEmptyString, validateWifiPWD,
		"WiFiSSID2", "wifissid2", settings.wifi2SSID, WIFI_SSID_LENGTH, text, setEmptyString, validateWifiSSID,
		"WiFiPassword2", "wifipwd2", settings.wifi2PWD, WIFI_PASSWORD_LENGTH, password, setEmptyString, validateWifiPWD,
		"WiFiSSID3", "wifissid3", settings.wifi3SSID, WIFI_SSID_LENGTH, text, setEmptyString, validateWifiSSID,
		"WiFiPassword3", "wifipwd3", settings.wifi3PWD, WIFI_PASSWORD_LENGTH, password, setEmptyString, validateWifiPWD,
		"WiFiSSID4", "wifissid4", settings.wifi4SSID, WIFI_SSID_LENGTH, text, setEmptyString, validateWifiSSID,
		"WiFiPassword4", "wifipwd4", settings.wifi4PWD, WIFI_PASSWORD_LENGTH, password, setEmptyString, validateWifiPWD,
		"WiFiSSID5", "wifissid5", settings.wifi5SSID, WIFI_SSID_LENGTH, text, setEmptyString, validateWifiSSID,
		"WiFiPassword5", "wifipwd5", settings.wifi4PWD, WIFI_PASSWORD_LENGTH, password, setEmptyString, validateWifiPWD,
		"WiFi On", "wifion", &settings.wiFiOn, ONOFF_INPUT_LENGTH, onOff, setFalse, validateOnOff};

boolean validateServerName(void *dest, const char *newValueStr)
{
	return (validateString((char *)dest, newValueStr, SERVER_NAME_LENGTH));
}

struct SettingItem autoUpdateSettingItems[] =
	{
		"Auto update image server", "autoupdimage", settings.autoUpdateImageServer, SERVER_NAME_LENGTH, text, setEmptyString, validateServerName,
		"Auto update status server", "autoupdstatus", settings.autoUpdateStatusServer, SERVER_NAME_LENGTH, text, setEmptyString, validateServerName,
		"Auto update on", "autoupdon", &settings.autoUpdateEnabled, ONOFF_INPUT_LENGTH, onOff, setFalse, validateOnOff};

char *defaultMQTTName = "NewMQTTDevice";
char *defaultMQTTHost = "mqtt.connectedhumber.org";

void setDefaultMQTTTname(void *dest)
{
	char *destStr = (char *)dest;
	snprintf(destStr, DEVICE_NAME_LENGTH, "Sensor-%06x", ESP.getEfuseMac());
}

void setDefaultMQTThost(void *dest)
{
	strcpy((char *)dest, "mqtt.connectedhumber.org");
}

boolean validateMQTThost(void *dest, const char *newValueStr)
{
	return (validateString((char *)dest, newValueStr, SERVER_NAME_LENGTH));
}

void setDefaultMQTTport(void *dest)
{
	int *destInt = (int *)dest;
	*destInt = 1883; // use 8883 for secure connection to Azure IoT hub
}

boolean validateMQTTtopic(void *dest, const char *newValueStr)
{
	return (validateString((char *)dest, newValueStr, MQTT_TOPIC_LENGTH));
}

void setDefaultMQTTusername(void *dest)
{
	strcpy((char *)dest, "connectedhumber");
}

boolean validateMQTTusername(void *dest, const char *newValueStr)
{
	return (validateString((char *)dest, newValueStr, MQTT_USER_NAME_LENGTH));
}

boolean validateMQTTPWD(void *dest, const char *newValueStr)
{
	return (validateString((char *)dest, newValueStr, MQTT_PASSWORD_LENGTH));
}

void setDefaultMQTTpublishTopic(void *dest)
{
	snprintf((char *)dest, MQTT_TOPIC_LENGTH, "airquality/data/Monitair-%06x", ESP.getEfuseMac());
}

void setDefaultMQTTsubscribeTopic(void *dest)
{
	snprintf((char *)dest, MQTT_TOPIC_LENGTH, "airquality/command/Monitair-%06x", ESP.getEfuseMac());
}

void setDefaultMQTTreportTopic(void *dest)
{
	snprintf((char *)dest, MQTT_TOPIC_LENGTH, "airquality/report/Monitair-%06x", ESP.getEfuseMac());
}

void setDefaultMQTTsecsPerUpdate(void *dest)
{
	int *destInt = (int *)dest;
	*destInt = 360;
}

void setDefaultMQTTsecsPerRetry(void *dest)
{
	int *destInt = (int *)dest;
	*destInt = 10;
}

struct SettingItem mqtttSettingItems[] =
	{
		"MQTT Host", "mqtthost", settings.mqttServer, SERVER_NAME_LENGTH, text, setDefaultMQTThost, validateServerName,
		"MQTT Port number", "mqttport", &settings.mqttPort, NUMBER_INPUT_LENGTH, integerValue, setDefaultMQTTport, validateInt,
		"MQTT Secure sockets (on or off)", "mqttsecure", &settings.mqttSecureSockets, ONOFF_INPUT_LENGTH, onOff, setFalse, validateOnOff,
		"MQTT Active (yes or no)", "mqttactive", &settings.mqtt_enabled, ONOFF_INPUT_LENGTH, yesNo, setFalse, validateYesNo,
		"MQTT UserName", "mqttuser", settings.mqttUser, MQTT_USER_NAME_LENGTH, text, setDefaultMQTTusername, validateMQTTusername,
		"MQTT Password", "mqttpwd", settings.mqttPassword, MQTT_PASSWORD_LENGTH, password, setEmptyString, validateMQTTPWD,
		"MQTT Publish topic", "mqttpub", settings.mqttPublishTopic, MQTT_TOPIC_LENGTH, text, setDefaultMQTTpublishTopic, validateMQTTtopic,
		"MQTT Subscribe topic", "mqttsub", settings.mqttSubscribeTopic, MQTT_TOPIC_LENGTH, text, setDefaultMQTTsubscribeTopic, validateMQTTtopic,
		"MQTT Reporting topic", "mqttreport", settings.mqttReportTopic, MQTT_TOPIC_LENGTH, text, setDefaultMQTTreportTopic, validateMQTTtopic,
		"MQTT Seconds per update", "mqttsecsperupdate", &settings.mqttSecsPerUpdate, NUMBER_INPUT_LENGTH, integerValue, setDefaultMQTTsecsPerUpdate, validateInt,
		"MQTT Seconds per retry", "mqttsecsperretry", &settings.seconds_per_mqtt_retry, NUMBER_INPUT_LENGTH, integerValue, setDefaultMQTTsecsPerRetry, validateInt};

void setDefaultPixelRed(void *dest)
{
	int *destInt = (int *)dest;
	*destInt = 0;
}

void setDefaultPixelGreen(void *dest)
{
	int *destInt = (int *)dest;
	*destInt = 255;
}

void setDefaultPixelBlue(void *dest)
{
	int *destInt = (int *)dest;
	*destInt = 0;
}

boolean validateColour(void *dest, const char *newValueStr)
{
	int value;

	if (sscanf(newValueStr, "%d", &value) == 1)
	{
		*(int *)dest = value;
		return true;
	}

	if (value < 0)
		return false;

	if (value > 255)
		return false;

	return true;
}

void setDefaultAirqLowLimit(void *dest)
{
	int *destInt = (int *)dest;
	*destInt = 15;
}

void setDefaultAirqLowWarnLimit(void *dest)
{
	int *destInt = (int *)dest;
	*destInt = 40;
}

void setDefaultAirqMidWarnLimit(void *dest)
{
	int *destInt = (int *)dest;
	*destInt = 65;
}

void setDefaultAirqHighWarnLimit(void *dest)
{
	int *destInt = (int *)dest;
	*destInt = 150;
}

void setDefaultAirqHighAlertLimit(void *dest)
{
	int *destInt = (int *)dest;
	*destInt = 250;
}

void setDefault(void *dest)
{
	int *destInt = (int *)dest;
	*destInt = 0;
}

struct SettingItem pixelSettingItems[] =
	{
		"Pixel red (0-255)",
		"pixelred",
		&settings.pixelRed,
		NUMBER_INPUT_LENGTH,
		integerValue,
		setDefaultPixelRed,
		validateColour,
		"Pixel green (0-255)",
		"pixelgreen",
		&settings.pixelGreen,
		NUMBER_INPUT_LENGTH,
		integerValue,
		setDefaultPixelGreen,
		validateColour,
		"Pixel blue (0-255)",
		"pixelblue",
		&settings.pixelBlue,
		NUMBER_INPUT_LENGTH,
		integerValue,
		setDefaultPixelBlue,
		validateColour,
		"AirQ Low Limit",
		"airqlowlimit",
		&settings.airqLowLimit,
		NUMBER_INPUT_LENGTH,
		integerValue,
		setDefaultAirqLowLimit,
		validateInt,
		"AirQ Low Warning Limit",
		"airqlowwarnlimit",
		&settings.airqLowWarnLimit,
		NUMBER_INPUT_LENGTH,
		integerValue,
		setDefaultAirqLowWarnLimit,
		validateInt,
		"AirQ Mid Warning Limit",
		"airqmidwarnlimit",
		&settings.airqMidWarnLimit,
		NUMBER_INPUT_LENGTH,
		integerValue,
		setDefaultAirqMidWarnLimit,
		validateInt,
		"AirQ High Warning Limit",
		"airqhighwarnlimit",
		&settings.airqHighWarnLimit,
		NUMBER_INPUT_LENGTH,
		integerValue,
		setDefaultAirqHighWarnLimit,
		validateInt,
		"AirQ High Alert Limit",
		"airqhighalertlimit",
		&settings.airqHighAlertLimit,
		NUMBER_INPUT_LENGTH,
		integerValue,
		setDefaultAirqHighAlertLimit,
		validateInt,
};

void setDefaultAirQSensorType(void *dest)
{
	int *destInt = (int *)dest;
	*destInt = UNKNOWN_SENSOR;
}

void setDefaultAirQWarmupTime(void *dest)
{
	int *destInt = (int *)dest;
	*destInt = 30;
}

void setDefaultAirqRXpin(void *dest)
{
	int *destInt = (int *)dest;
	*destInt = 17;
}

void setDefaultAirqTXpin(void *dest)
{
	int *destInt = (int *)dest;
	*destInt = 13;
}

void setDefaultGpsPinNo(void *dest)
{
	int *destInt = (int *)dest;
	*destInt = 22;
}

void setDefaultPowerControlPinNo(void *dest)
{
	int *destInt = (int *)dest;
	*destInt = 21;
}

void setDefaultControlInputPin(void *dest)
{
	int *destInt = (int *)dest;
	*destInt = 36;
}

void setDefaultPixelControlPinNo(void *dest)
{
	int *destInt = (int *)dest;
	*destInt = 25;
}

void setDefaultNoOfPixels(void *dest)
{
	int *destInt = (int *)dest;
	*destInt = 12;
}

void setDefaultAirqnoOfAverages(void *dest)
{
	int *destInt = (int *)dest;
	*destInt = 25;
}

void setDefaultEnvnoOfAverages(void *dest)
{
	int *destInt = (int *)dest;
	*destInt = 25;
}

struct SettingItem hardwareSettingItems[] =
	{
		"AirQ Sensor type (0 = not fitted 1=SDS011, 2=ZPH01)",
		"airqsensortype",
		&settings.airqSensorType,
		NUMBER_INPUT_LENGTH,
		integerValue,
		setDefaultAirQSensorType,
		validateInt,
		"AirQ Seconds for sensor warmup",
		"airqsensorwarmup",
		&settings.airqSecnondsSensorWarmupTime,
		NUMBER_INPUT_LENGTH,
		integerValue,
		setDefaultAirQWarmupTime,
		validateInt,
		"AirQ RX Pin",
		"airqrxpinno",
		&settings.airqRXPinNo,
		NUMBER_INPUT_LENGTH,
		integerValue,
		setDefaultAirqRXpin,
		validateInt,
		"AirQ TX Pin",
		"airqtxpinno",
		&settings.airqTXPinNo,
		NUMBER_INPUT_LENGTH,
		integerValue,
		setDefaultAirqTXpin,
		validateInt,
		"BME 280 fitted (yes or no)",
		"bme280fitted",
		&settings.bme280Fitted,
		ONOFF_INPUT_LENGTH,
		yesNo,
		setTrue,
		validateYesNo,
		"Power Control fitted (yes or no)",
		"powercontrolfitted",
		&settings.powerControlFitted,
		ONOFF_INPUT_LENGTH,
		yesNo,
		setFalse,
		validateYesNo,
		"Power Control Pin",
		"powercontrolpin",
		&settings.powerControlPin,
		NUMBER_INPUT_LENGTH,
		integerValue,
		setDefaultPowerControlPinNo,
		validateInt,
		"Control Input Pin",
		"controlinputpin",
		&settings.controlInputPin,
		NUMBER_INPUT_LENGTH,
		integerValue,
		setDefaultControlInputPin,
		validateInt,
		"Control Input Active Low",
		"controlinputlow",
		&settings.controlInputPinActiveLow,
		ONOFF_INPUT_LENGTH,
		yesNo,
		setFalse,
		validateYesNo,
		"GPS fitted (yes or no)",
		"gpsfitted",
		&settings.gpsFitted,
		ONOFF_INPUT_LENGTH,
		yesNo,
		setFalse,
		validateYesNo,
		"GPS RX Pin",
		"gpsrxpin",
		&settings.gpsRXPinNo,
		NUMBER_INPUT_LENGTH,
		integerValue,
		setDefaultGpsPinNo,
		validateInt,
		"Number of pixels (0 for pixels not fitted)",
		"noofpixels",
		&settings.noOfPixels,
		NUMBER_INPUT_LENGTH,
		integerValue,
		setDefaultNoOfPixels,
		validateInt,
		"Pixel Control Pin",
		"pixelcontrolpin",
		&settings.pixelControlPinNo,
		NUMBER_INPUT_LENGTH,
		integerValue,
		setDefaultPixelControlPinNo,
		validateInt,
		"AirQ Number of averages",
		"airqnoOfAverages",
		&settings.airqNoOfAverages,
		NUMBER_INPUT_LENGTH,
		integerValue,
		setDefaultAirqnoOfAverages,
		validateInt,
		"Environment Number of averages",
		"envnoOfAverages",
		&settings.envNoOfAverages,
		NUMBER_INPUT_LENGTH,
		integerValue,
		setDefaultEnvnoOfAverages,
		validateInt,
};

void setDefaultPositionValue(void *dest)
{
	double *destDouble = (double *)dest;
	*destDouble = -1000;
}

struct SettingItem locationSettingItems[] =
	{
		"Fixed location", "fixedlocation", &settings.fixedLocation, ONOFF_INPUT_LENGTH, yesNo, setTrue, validateYesNo,
		"Device lattitude", "lattitude", &settings.lattitude, NUMBER_INPUT_LENGTH, doubleValue, setDefaultPositionValue, validateDouble,
		"Device longitude", "longitude", &settings.longitude, NUMBER_INPUT_LENGTH, doubleValue, setDefaultPositionValue, validateDouble,
		"Device indoors", "indoorDevice", &settings.indoorDevice, ONOFF_INPUT_LENGTH, yesNo, setFalse, validateYesNo};

boolean validateSplashScreen(void *dest, const char *newValueStr)
{
	return (validateString((char *)dest, newValueStr, SPLASH_LINE_LENGTH));
}

void setDefaultSplashTopLine(void *dest)
{
	snprintf((char *)dest, SPLASH_LINE_LENGTH, "Connected");
}

void setDefaultSplashBottomLine(void *dest)
{
	snprintf((char *)dest, SPLASH_LINE_LENGTH, "Humber");
}

boolean validateLoggingState(void *dest, const char *newValueStr)
{
	int value;

	if (sscanf(newValueStr, "%d", &value) != 1)
	{
		return false;
	}

	if (value < 0)
	{
		return false;
	}

	if (value > 5)
	{
		return false;
	}

	*(Logging_State *)dest = (Logging_State)value;
	return true;
}

void setDefaultLoggingState(void *dest)
{
	*(Logging_State *)dest = loggingOff;
}

struct SettingItem displaySettingItems[] =
	{
		"Splash screen top line", "splashTop", settings.splash_screen_top_line, SPLASH_LINE_LENGTH, text, setDefaultSplashTopLine, validateSplashScreen,
		"Splash screen bottom line", "splashBtm", settings.splash_screen_bottom_line, SPLASH_LINE_LENGTH, text, setDefaultSplashBottomLine, validateSplashScreen,
		"Logging (0=off,1=air,2=temp,3=pres,4=hum,5=all)", "logging", &settings.logging, NUMBER_INPUT_LENGTH, integerValue, setDefaultLoggingState, validateLoggingState};

struct SettingItem quickSettingItems[] =
	{
		"WiFi access point name", "wifissid1", settings.wifi1SSID, WIFI_SSID_LENGTH, text, setEmptyString, validateWifiSSID,
		"WiFi password", "wifipwd1", settings.wifi1PWD, WIFI_PASSWORD_LENGTH, password, setEmptyString, validateWifiPWD,
		"MQTT Password", "mqttpwd", settings.mqttPassword, MQTT_PASSWORD_LENGTH, password, setEmptyString, validateMQTTPWD,
		"Device lattitude", "lattitude", &settings.lattitude, NUMBER_INPUT_LENGTH, doubleValue, setDefaultPositionValue, validateDouble,
		"Device longitude", "longitude", &settings.longitude, NUMBER_INPUT_LENGTH, doubleValue, setDefaultPositionValue, validateDouble,
		"Device indoors", "indoorDevice", &settings.indoorDevice, ONOFF_INPUT_LENGTH, yesNo, setFalse, validateYesNo};

SettingItemCollection allSettings[] = {
	{"Quick", "Just the settings to get you started", quickSettingItems, sizeof(quickSettingItems) / sizeof(SettingItem)},
	{"Output", "Display and logging settings", displaySettingItems, sizeof(displaySettingItems) / sizeof(SettingItem)},
	{"Wifi", "Set the SSID and password for wifi connections", wifiSettingItems, sizeof(wifiSettingItems) / sizeof(SettingItem)},
	{"MQTT", "Set the device, user, site, password and topic for MQTT", mqtttSettingItems, sizeof(mqtttSettingItems) / sizeof(SettingItem)},
	{"LoRa", "Set the authentication and data rate for LoRa", loraSettingItems, noOfLoraSettingItems},
	{"Pixel", "Set the pixel colours and display levels", pixelSettingItems, sizeof(pixelSettingItems) / sizeof(SettingItem)},
	{"Hardware", "Set the hardware pins and configuration", hardwareSettingItems, sizeof(hardwareSettingItems) / sizeof(SettingItem)},
	{"Location", "Set the fixed location of the device", locationSettingItems, sizeof(locationSettingItems) / sizeof(SettingItem)}};

SettingItem *FindSetting(char *settingName)
{
	// Start the search at setting collection 1 so that the quick settings are not used in the search
	for (int collectionNo = 1; collectionNo < sizeof(allSettings) / sizeof(SettingItemCollection); collectionNo++)
	{
		for (int settingNo = 0; settingNo < allSettings[collectionNo].noOfSettings; settingNo++)
		{
			if (strcasecmp(settingName, allSettings[collectionNo].settings[settingNo].formName))
			{
				return &allSettings[collectionNo].settings[settingNo];
			}
		}
	}
	return NULL;
}

void printSetting(SettingItem *item)
{
	int *intValuePointer;
	boolean *boolValuePointer;
	double *doubleValuePointer;
	u4_t *loraIDValuePointer;

	char loraKeyBuffer[LORA_KEY_LENGTH * 2 + 1];

	Serial.printf("    %s [%s]: ", item->prompt, item->formName);

	switch (item->settingType)
	{

	case text:
		Serial.println((char *)item->value);
		break;

	case password:
		//		Serial.println((char *)item->value);
		Serial.println("******");
		break;

	case integerValue:
		intValuePointer = (int *)item->value;
		Serial.println(*intValuePointer);
		break;

	case doubleValue:
		doubleValuePointer = (double *)item->value;
		Serial.println(*doubleValuePointer);
		break;

	case loraKey:
		dumpHexString(loraKeyBuffer, (uint8_t *)item->value, LORA_KEY_LENGTH);
		Serial.println(loraKeyBuffer);
		break;

	case loraID:
		loraIDValuePointer = (u4_t *)item->value;
		dumpUnsignedLong(loraKeyBuffer, *loraIDValuePointer);
		Serial.println(loraKeyBuffer);
		break;

	case onOff:
		boolValuePointer = (boolean *)item->value;
		if (*boolValuePointer)
		{
			Serial.println("on");
		}
		else
		{
			Serial.println("off");
		}
		break;

	case yesNo:
		boolValuePointer = (boolean *)item->value;
		if (*boolValuePointer)
		{
			Serial.println("yes");
		}
		else
		{
			Serial.println("no");
		}
		break;
	}
}

void appendSettingJSON(SettingItem *item, char *jsonBuffer, int bufferLength)
{
	int *intValuePointer;
	boolean *boolValuePointer;
	double *doubleValuePointer;
	u4_t *loraIDValuePointer;

	char loraKeyBuffer[LORA_KEY_LENGTH * 2 + 1];

	snprintf(jsonBuffer, bufferLength,
			 "%s,\"%s\":",
			 jsonBuffer,
			 item->formName);

	switch (item->settingType)
	{

	case text:
		snprintf(jsonBuffer, bufferLength,
				 "%s\"%s\"",
				 jsonBuffer,
				 (char *)item->value);
		break;

	case password:
		Serial.println("******");
		snprintf(jsonBuffer, bufferLength,
				 "%s\"******\"",
				 jsonBuffer);
		break;

	case integerValue:
		intValuePointer = (int *)item->value;
		Serial.println(*intValuePointer);
		break;

	case doubleValue:
		doubleValuePointer = (double *)item->value;
		Serial.println(*doubleValuePointer);
		break;

	case loraKey:
		dumpHexString(loraKeyBuffer, (uint8_t *)item->value, LORA_KEY_LENGTH);
		Serial.println(loraKeyBuffer);
		break;

	case loraID:
		loraIDValuePointer = (u4_t *)item->value;
		dumpUnsignedLong(loraKeyBuffer, *loraIDValuePointer);
		Serial.println(loraKeyBuffer);
		break;

	case onOff:
		boolValuePointer = (boolean *)item->value;
		if (*boolValuePointer)
		{
			Serial.println("on");
		}
		else
		{
			Serial.println("off");
		}
		break;

	case yesNo:
		boolValuePointer = (boolean *)item->value;
		if (*boolValuePointer)
		{
			Serial.println("yes");
		}
		else
		{
			Serial.println("no");
		}
		break;
	}
}

void resetSetting(SettingItem *setting)
{
	setting->setDefault(setting->value);
}

void resetSettingCollection(SettingItemCollection *settingCollection)
{
	for (int settingNo = 0; settingNo < settingCollection->noOfSettings; settingNo++)
	{
		resetSetting(&settingCollection->settings[settingNo]);
	}
}

void resetSettings()
{
	settings.majorVersion = MAJOR_VERSION;
	settings.minorVersion = MINOR_VERSION;

	for (int collectionNo = 0; collectionNo < sizeof(allSettings) / sizeof(SettingItemCollection); collectionNo++)
	{
		resetSettingCollection(&allSettings[collectionNo]);
	}

	PrintAllSettings();
	settings.checkByte1 = CHECK_BYTE_O1;
	settings.checkByte2 = CHECK_BYTE_O2;
}

void PrintSettingCollection(SettingItemCollection settingCollection)
{
	Serial.printf("\n%s\n", settingCollection.collectionName);
	for (int settingNo = 0; settingNo < settingCollection.noOfSettings; settingNo++)
	{
		printSetting(&settingCollection.settings[settingNo]);
	}
}

void PrintAllSettings()
{
	// Start the search at setting collection 1 so that the quick settings are not printed
	for (int collectionNo = 1; collectionNo < sizeof(allSettings) / sizeof(SettingItemCollection); collectionNo++)
	{
		PrintSettingCollection(allSettings[collectionNo]);
	}
}

void writeBytesToEEPROM(byte *bytesToStore, int address, int length)
{
	int endAddress = address + length;
	for (int i = address; i < endAddress; i++)
	{
		EEPROM.write(i, *bytesToStore);
		bytesToStore++;
	}
	EEPROM.commit();
}

void readBytesFromEEPROM(byte *destination, int address, int length)
{
	int endAddress = address + length;

	for (int i = address; i < endAddress; i++)
	{
		*destination = EEPROM.read(i);

		destination++;
	}
}

void saveSettings()
{
	int addr = SETTINGS_EEPROM_OFFSET;

	byte *settingPtr = (byte *)&settings;

	writeBytesToEEPROM(settingPtr, SETTINGS_EEPROM_OFFSET, sizeof(Device_Settings));
}

void loadSettings()
{
	byte *settingPtr = (byte *)&settings;
	readBytesFromEEPROM(settingPtr, SETTINGS_EEPROM_OFFSET, sizeof(Device_Settings));
}

boolean validStoredSettings()
{
	return (settings.checkByte1 == CHECK_BYTE_O1 && settings.checkByte2 == CHECK_BYTE_O2);
}

boolean matchSettingCollectionName(SettingItemCollection *settingCollection, const char *name)
{
	int settingNameLength = strlen(settingCollection->collectionName);

	for (int i = 0; i < settingNameLength; i++)
	{
		if (tolower(name[i] != settingCollection->collectionName[i]))
			return false;
	}

	// reached the end of the name, character that follows should be either zero (end of the string)
	// or = (we are assigning a value to the setting)

	if (name[settingNameLength] == 0)
		return true;

	return false;
}

SettingItemCollection *findSettingItemCollectionByName(const char *name)
{
	for (int collectionNo = 0; collectionNo < sizeof(allSettings) / sizeof(SettingItemCollection); collectionNo++)
	{
		if (matchSettingCollectionName(&allSettings[collectionNo], name))
		{
			return &allSettings[collectionNo];
		}
	}
	return NULL;
}

boolean matchSettingName(SettingItem *setting, const char *name)
{
	int settingNameLength = strlen(setting->formName);

	for (int i = 0; i < settingNameLength; i++)
	{
		if (tolower(name[i] != setting->formName[i]))
			return false;
	}

	// reached the end of the name, character that follows should be either zero (end of the string)
	// or = (we are assigning a value to the setting)

	if (name[settingNameLength] == 0)
		return true;

	if (name[settingNameLength] == '=')
		return true;

	return false;
}

SettingItem *findSettingByNameInCollection(SettingItemCollection settingCollection, const char *name)
{
	for (int settingNo = 0; settingNo < settingCollection.noOfSettings; settingNo++)
	{
		if (matchSettingName(&settingCollection.settings[settingNo], name))
			return &settingCollection.settings[settingNo];
	}
	return NULL;
}

SettingItem *findSettingByName(const char *name)
{
	for (int collectionNo = 0; collectionNo < sizeof(allSettings) / sizeof(SettingItemCollection); collectionNo++)
	{
		SettingItem *result;
		result = findSettingByNameInCollection(allSettings[collectionNo], name);
		if (result != NULL)
			return result;
	}
	return NULL;
}

struct AllSystemSettings allSystemSettings = {
	allSettings,
	sizeof(allSettings) / sizeof(SettingItemCollection)};

AllSystemSettings *getAllSystemSettings()
{
	return &allSystemSettings;
}

processSettingCommandResult processSettingCommand(char *command)
{
	SettingItem *setting = findSettingByName(command);

	if (setting != NULL)
	{
		// found a setting - get the length of the setting name:
		int settingNameLength = strlen(setting->formName);

		if (command[settingNameLength] == 0)
		{
			// Settting is on it's own on the line
			// Just print the value
			printSetting(setting);
			return displayedOK;
		}

		if (command[settingNameLength] == '=')
		{
			// Setting is being assigned a value
			// move down the input to the new value
			char *startOfSettingInfo = command + settingNameLength + 1;
			if (setting->validateValue(setting->value, startOfSettingInfo))
			{
				saveSettings();
				return setOK;
			}
			return settingValueInvalid;
		}
	}
	return settingNotFound;
}

void sendSettingItemToJSONString(struct SettingItem *item, char *buffer, int bufferSize)
{
	int *valuePointer;
	double *doublePointer;
	boolean *boolPointer;
	u4_t *loraIDValuePointer;
	char loraKeyBuffer[LORA_KEY_LENGTH * 2 + 1];

	switch (item->settingType)
	{
	case text:
		snprintf(buffer, bufferSize, "\"%s\"", item->value);
		break;
	case password:
		snprintf(buffer, bufferSize, "\"%s\"", item->value);
		break;
	case integerValue:
		valuePointer = (int *)item->value;
		snprintf(buffer, bufferSize, "%d", *valuePointer);
		break;
	case doubleValue:
		doublePointer = (double *)item->value;
		snprintf(buffer, bufferSize, "%lf", *doublePointer);
		break;
	case onOff:
		boolPointer = (boolean *)item->value;
		if (*boolPointer)
		{
			snprintf(buffer, bufferSize, "\"on\"");
		}
		else
		{
			snprintf(buffer, bufferSize, "\"off\"");
		}
		break;
	case yesNo:
		boolPointer = (boolean *)item->value;
		if (*boolPointer)
		{
			snprintf(buffer, bufferSize, "\"yes\"");
		}
		else
		{
			snprintf(buffer, bufferSize, "\"no\"");
		}
		break;
	case loraKey:
		dumpHexString(loraKeyBuffer, (uint8_t *)item->value, LORA_KEY_LENGTH);
		snprintf(buffer, bufferSize, "\"%s\"", loraKeyBuffer);
		break;

	case loraID:
		loraIDValuePointer = (u4_t *)item->value;
		dumpUnsignedLong(loraKeyBuffer, *loraIDValuePointer);
		snprintf(buffer, bufferSize, "\"%s\"", loraKeyBuffer);
		break;
	}
}

void testSettingsStorage()
{
	Serial.println("Testing Settings Storage");
	Serial.println("Resetting Settings");
	resetSettings();
	PrintAllSettings();
	Serial.println("Storing Settings");
	saveSettings();
	Serial.println("Loading Settings");
	loadSettings();
	PrintAllSettings();
	if (validStoredSettings())
		Serial.println("Settings storage restored OK");
	else
		Serial.println("Something wrong with setting storage");
}

#define COMMAND_REPLY_BUFFER_SIZE 240
#define REPLY_ELEMENT_SIZE 100

char command_reply_buffer[COMMAND_REPLY_BUFFER_SIZE];

StaticJsonBuffer<300> jsonBuffer;

void build_command_reply(int errorNo, JsonObject &root, char *resultBuffer)
{
	char replyBuffer[REPLY_ELEMENT_SIZE];

	const char *sequence = root["seq"];

	if (sequence)
	{
		// Got a sequence number in the command - must return the same number
		// so that the sender can identify the command that was sent
		int sequenceNo = root["seq"];
		sprintf(replyBuffer, "\"error\":%d,\"seq\":%d", errorNo, sequenceNo);
	}
	else
	{
		sprintf(replyBuffer, "\"error\":%d", errorNo);
	}
	strcat(resultBuffer, replyBuffer);
}

void build_text_value_command_reply(int errorNo, const char * result, JsonObject &root, char *resultBuffer)
{
	char replyBuffer[REPLY_ELEMENT_SIZE];

	const char *sequence = root["seq"];

	if (sequence)
	{
		// Got a sequence number in the command - must return the same number
		// so that the sender can identify the command that was sent
		int sequenceNo = root["seq"];
		sprintf(replyBuffer, "\"val\":%s\",\"error\":%d,\"seq\":%d", result, errorNo, sequenceNo);
	}
	else
	{
		sprintf(replyBuffer, "\"val\":%s,\"error\":%d", result, errorNo);
	}

	strcat(resultBuffer, replyBuffer);
}

void abort_json_command(int error, JsonObject &root, void (*deliverResult)(char *resultText))
{
	build_command_reply(error, root, command_reply_buffer);
	// append the version number to the invalid command message
	strcat(command_reply_buffer, "}");
	deliverResult(command_reply_buffer);
}

void act_onJson_command(const char *json, void (*deliverResult)(char *resultText))
{
	TRACE("Received message:");
	TRACELN(json);

	command_reply_buffer[0] = 0;

	strcat(command_reply_buffer, "{");

	// Clear any previous elements from the buffer

	jsonBuffer.clear();

	JsonObject &root = jsonBuffer.parseObject(json);

	if (!root.success())
	{
		TRACELN("JSON could not be parsed");
		abort_json_command(JSON_MESSAGE_COULD_NOT_BE_PARSED, root, deliverResult);
		return;
	}

	const char *setting = root["setting"];

	if (!setting)
	{
		TRACELN("Missing setting");
		abort_json_command(JSON_MESSAGE_MISSING_COMMAND_NAME, root, deliverResult);
		return;
	}

	TRACE("Received setting: ");
	TRACELN(setting);

	SettingItem *item = findSettingByName(setting);

	if (item == NULL)
	{
		build_command_reply(JSON_MESSAGE_COMMAND_NAME_INVALID, root, command_reply_buffer);
	}
	else
	{
		char buffer[120];

		if (!root["value"])
		{
			sendSettingItemToJSONString(item, buffer, 120);
			// no value - just a status request
			build_text_value_command_reply(WORKED_OK, buffer, root, command_reply_buffer);
		}
		else
		{
			const char *inputSource = NULL;

			if (!root["value"].is<int>())
			{
				// need to convert the input value into a string
				// as our value parser uses strings as inputs
				snprintf(buffer, 120, "%d", root["value"]);
				inputSource = buffer;
			}
			else
			{
				if (!root["value"].is<char *>())
					inputSource = root["value"];
			}

			if (inputSource == NULL)
			{
				build_command_reply(JSON_MESSAGE_INVALID_DATA_TYPE, root, command_reply_buffer);
			}
			else
			{
				if (item->validateValue(item->value, inputSource))
				{
					saveSettings();
					build_command_reply(JSON_COMMAND_OK, root, command_reply_buffer);
				}
				else
				{
					build_command_reply(JSON_MESSAGE_INVALID_DATA_VALUE, root, command_reply_buffer);
				}
			}
		}
	}

	strcat(command_reply_buffer, "}");
	deliverResult(command_reply_buffer);
}

void setupSettings()
{
	EEPROM.begin(EEPROM_SIZE);

	loadSettings();

	if (!validStoredSettings())
	{
		Serial.println("Stored settings reset");
		resetSettings();
		saveSettings();
	}

	// always clear down the sensor type to force auto discovery
	// can be used for manual setting during testing
	setDefaultAirQSensorType(&settings.airqSensorType);

	PrintAllSettings();
}

void createSettingsJson(char *jsonBuffer, int length)
{
	snprintf(jsonBuffer, length, "{ \"dev\":\"%s\",\"plat\":\WEMOS\"",
			 settings.deviceName);
}

void testFindSettingByName()
{
	resetSettings();

	SettingItem *result = findSettingByName("pixelControlPinNo");
	if (result != NULL)
		Serial.println("Setting find works OK");
	else
		Serial.println("Setting find failed");

	result = findSettingByName("xpixelControlPinNo");
	if (result == NULL)
		Serial.println("Setting not find works OK");
	else
		Serial.println("Setting not find failed");

	result = findSettingByName("pixelControlPinNox");
	if (result == NULL)
		Serial.println("Setting not find works OK");
	else
		Serial.println("Setting not find failed");

	if (processSettingCommand("fred"))
		Serial.println("Should not have processed setting fred");
	else
		Serial.println("Setting not found OK");

	if (processSettingCommand("devname"))
		Serial.println("Found setting devname - should have printed it");
	else
		Serial.println("Setting devname not found");

	if (processSettingCommand("devname=test"))
		Serial.println("Found setting devname - should have changed it to test");
	else
		Serial.println("Setting devname not found");

	if (processSettingCommand("devname"))
		Serial.println("Found setting devname - should have printed it");
	else
		Serial.println("Setting devname not found");
}