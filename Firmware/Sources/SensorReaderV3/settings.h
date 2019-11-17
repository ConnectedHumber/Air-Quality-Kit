#ifndef SETTINGS_H

#define SETTINGS_H

#include <Arduino.h>

#include "utils.h"

#include "timing.h" // contains the definition for Logging_State

#define MAJOR_VERSION 3
#define MINOR_VERSION 0

// Sensor settings
#define UNKNOWN_SENSOR 0
#define SDS011_SENSOR 1
#define ZPH01_SENSOR 2
#define PMS5003_SENSOR 3

#define DEVICE_NAME_LENGTH 20

#define WIFI_SSID_LENGTH 30
#define WIFI_PASSWORD_LENGTH 30

#define SERVER_NAME_LENGTH 200
#define MQTT_USER_NAME_LENGTH 100
#define MQTT_PASSWORD_LENGTH 200
#define MQTT_TOPIC_LENGTH 150
#define NUMBER_INPUT_LENGTH 20
#define YESNO_INPUT_LENGTH 0
#define ONOFF_INPUT_LENGTH 0
#define SETTING_ERROR_MESSAGE_LENGTH 120

#define SPLASH_LINE_LENGTH 15

#define MAX_SETTING_LENGTH 300

#define EEPROM_SIZE 5000
#define SETTINGS_EEPROM_OFFSET 0
#define CHECK_BYTE_O1 0x55
#define CHECK_BYTE_O2 0xAA

struct Device_Settings
{
	int majorVersion;
	int minorVersion;

	byte checkByte1;

	char deviceName[DEVICE_NAME_LENGTH];

	boolean indoorDevice;

	// WiFi settings

	// Auto update settings

	char autoUpdateImageServer[SERVER_NAME_LENGTH];
	char autoUpdateStatusServer[SERVER_NAME_LENGTH];
	boolean autoUpdateEnabled;

	// MQTT settings

	// Hardware settings

	int airqSensorType;
	int airqSecnondsSensorWarmupTime;
	int airqRXPinNo;
	int airqTXPinNo;
	boolean bme280Fitted;

	boolean powerControlFitted;
	int powerControlPin;
	int controlInputPin;
	boolean controlInputPinActiveLow;
	
	boolean gpsFitted;
	int gpsRXPinNo;

	boolean fixedLocation;
	double lattitude;
	double longitude;

	int airqNoOfAverages;

	int envNoOfAverages;

	Logging_State logging;

	char splash_screen_top_line[SPLASH_LINE_LENGTH];
	char splash_screen_bottom_line[SPLASH_LINE_LENGTH];

	byte checkByte2;
};

extern struct Device_Settings settings;

enum Setting_Type { text, password, integerValue, doubleValue, loraKey, loraID, onOff, yesNo };

struct SettingItem {
	char * prompt;
	char * formName;
	void * value;
	int maxLength;
	Setting_Type settingType;
	void (*setDefault)(void * destination);
	boolean (*validateValue)(void * dest, const char * newValueStr);
};

struct SettingItemCollection
{
	char * collectionName;
	char * collectionDescription;
	SettingItem * settings;
	int noOfSettings;
};

struct AllSystemSettings
{
	SettingItemCollection * collections;
	int noOfCollections;
};

enum processSettingCommandResult { displayedOK, setOK, settingNotFound, settingValueInvalid };

void saveSettings();
void loadSettings();
void resetSettings();
void PrintAllSettings();

SettingItemCollection * findSettingItemCollectionByName(const char * name);
AllSystemSettings * getAllSystemSettings();

processSettingCommandResult processSettingCommand(char * command);

void setupSettings();

void dumpHexString(char *dest, uint8_t *pos, int length);
void dumpUnsignedLong(char *dest, uint32_t value);
int decodeHexValueIntoBytes(uint8_t *dest, const char *newVal, int length);
int decodeHexValueIntoUnsignedLong(u4_t *dest, const char *newVal);

void sendSettingItemToString(struct SettingItem * item, char * bufffer, int bufferLength);

void act_onJson_command(const char *json, void (*deliverResult)(char *resultText));

void setEmptyString(void *dest);

void setFalse(void *dest);

void setTrue(void *dest);

boolean validateOnOff(void *dest, const char *newValueStr);

boolean validateYesNo(void *dest, const char *newValueStr);

boolean validateString(char *dest, const char *source, int maxLength);

boolean validateInt(void *dest, const char *newValueStr);

boolean validateDouble(void *dest, const char *newValueStr);

boolean validateColour(void* dest, const char* newValueStr);

void setDefaultDevname(void* dest);

boolean validateDevName(void* dest, const char* newValueStr);

boolean validateServerName(void* dest, const char* newValueStr);

boolean validateWifiSSID(void* dest, const char* newValueStr);

boolean validateWifiPWD(void* dest, const char* newValueStr);

#endif