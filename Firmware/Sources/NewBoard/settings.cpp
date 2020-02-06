#include <EEPROM.h>
#include <Arduino.h>
#include "string.h"
#include "errors.h"
#include "settings.h"
#include "sensors.h"
#include "processes.h"
#include "debug.h"
#include "ArduinoJson-v5.13.2.h"
#include "utils.h"
#include "pixels.h"
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

boolean validateServerName(void *dest, const char *newValueStr)
{
	return (validateString((char *)dest, newValueStr, SERVER_NAME_LENGTH));
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

	case loraKey:
		dumpHexString(loraKeyBuffer, (uint8_t*)item->value, LORA_KEY_LENGTH);
		Serial.println(loraKeyBuffer);
		break;

	case loraID:
		loraIDValuePointer = (u4_t*)item->value;
		dumpUnsignedLong(loraKeyBuffer, *loraIDValuePointer);
		Serial.println(loraKeyBuffer);
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

	case loraKey:
		dumpHexString(loraKeyBuffer, (uint8_t*)item->value, LORA_KEY_LENGTH);
		Serial.println(loraKeyBuffer);
		break;

	case loraID:
		loraIDValuePointer = (u4_t*)item->value;
		dumpUnsignedLong(loraKeyBuffer, *loraIDValuePointer);
		Serial.println(loraKeyBuffer);
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
		resetSetting(settingCollection->settings[settingNo]);
	}
}

void PrintSettingCollection(SettingItemCollection * settingCollection)
{
	Serial.printf("\n%s\n", settingCollection->collectionName);
	for (int settingNo = 0; settingNo < settingCollection->noOfSettings; settingNo++)
	{
		printSetting(settingCollection->settings[settingNo]);
	}
}

void PrintSystemDetails()
{
	Serial.printf("%s\nVersion %d.%d\n\n", settings.deviceName, settings.majorVersion, settings.minorVersion);
}


void PrintAllSettings()
{
	PrintSystemDetails();
	Serial.println("Sensors");
	iterateThroughSensorSettingCollections(PrintSettingCollection);
	Serial.println("Processes");
	iterateThroughProcessSettingCollections(PrintSettingCollection);
}

void resetSettings()
{
	snprintf(settings.deviceName, DEVICE_NAME_LENGTH, "Sensor-%06x", (unsigned long) ESP.getEfuseMac());
	settings.majorVersion = MAJOR_VERSION;
	settings.minorVersion = MINOR_VERSION;

	resetProcessesToDefaultSettings();
	resetSensorsToDefaultSettings();
}

unsigned char checksum;
int saveAddr;
int loadAddr;

void writeByteToEEPROM(byte b, int address)
{
	checksum = checksum + b;

	unsigned char rb = EEPROM.read(address);

	if (EEPROM.read(address) != b)
	{
		EEPROM.write(address, b);
	}
}

void writeBytesToEEPROM(unsigned char *bytesToStore, int address, int length)
{
	int endAddress = address + length;

	for (int i = address; i < endAddress; i++)
	{
		byte b = *bytesToStore;

		writeByteToEEPROM(b, i);

		bytesToStore++;
	}
}

byte readByteFromEEPROM(int address)
{
	byte result;
	result = EEPROM.read(address);
	checksum += result;
	return result;
}

void readBytesFromEEPROM(byte *destination, int address, int length)
{
	int endAddress = address + length;

	for (int i = address; i < endAddress; i++)
	{
		*destination = readByteFromEEPROM(i);
		destination++;
	}
}

void readSettingsBLockFromEEPROM(unsigned char* block, int size)
{
	readBytesFromEEPROM(block, loadAddr, size);
	loadAddr = loadAddr + size;
}

void saveSettingsBLockToEEPROM(unsigned char * block, int size)
{
	writeBytesToEEPROM(block, saveAddr,size);
	saveAddr = saveAddr + size;
}

void saveSettings()
{
	saveAddr = SETTINGS_EEPROM_OFFSET;

	checksum = 0;

	saveSettingsBLockToEEPROM((unsigned char *) & settings, sizeof(struct Device_Settings));

	iterateThroughProcessSecttings(saveSettingsBLockToEEPROM);

	iterateThroughSensorSecttings(saveSettingsBLockToEEPROM);

	writeByteToEEPROM(checksum, saveAddr);

	EEPROM.commit();
}

void loadSettings()
{
	loadAddr = SETTINGS_EEPROM_OFFSET;

	checksum = 0;

	readSettingsBLockFromEEPROM((unsigned char*)&settings, sizeof(struct Device_Settings));

	iterateThroughProcessSecttings(readSettingsBLockFromEEPROM);

	iterateThroughSensorSecttings(readSettingsBLockFromEEPROM);

	unsigned char readChecksum = readByteFromEEPROM(loadAddr);
}

void checksumBytesFromEEPROM(byte* destination, int address, int length)
{
	int endAddress = address + length;

	for (int i = address; i < endAddress; i++)
	{
		readByteFromEEPROM(i);
	}
}

void checkSettingsBLockFromEEPROM(unsigned char* block, int size)
{
	checksumBytesFromEEPROM(block, loadAddr, size);
	loadAddr = loadAddr + size;
}

boolean validStoredSettings()
{
	loadAddr = SETTINGS_EEPROM_OFFSET;

	checksum = 0;

	checkSettingsBLockFromEEPROM((unsigned char*)&settings, sizeof(struct Device_Settings));

	iterateThroughProcessSecttings(checkSettingsBLockFromEEPROM);

	iterateThroughSensorSecttings(checkSettingsBLockFromEEPROM);

	unsigned char calcChecksum = checksum;

	unsigned char readChecksum = readByteFromEEPROM(loadAddr);

	return (calcChecksum == readChecksum);
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
	sensor * s = findSensorSettingCollectionByName(name);
	if (s != NULL)
		return s->settingItems;

	process* p = findProcessSettingCollectionByName(name);
	if (p != NULL)
		return p->settingItems;

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
		if (matchSettingName(settingCollection.settings[settingNo], name))
			return settingCollection.settings[settingNo];
	}
	return NULL;
}



SettingItem* findSettingByName(const char* settingName)
{
	SettingItem* result;

	result = FindSensorSettingByFormName(settingName);

	if (result != NULL)
		return result;

	result = FindProcesSettingByFormName(settingName);
	if (result != NULL)
		return result;

	return NULL;
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

	switch (item->settingType)
	{
	case text:
		snprintf(buffer, bufferSize, "\"%s\"", item->value);
		break;
	case password:
		snprintf(buffer, bufferSize, "\"******\"");
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
	Serial.println("Restoring setings");
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

		if (!root.containsKey("value"))
		{
			// no value - just a status request
			Serial.println("  No value part");
			sendSettingItemToJSONString(item, buffer, 120);
			build_text_value_command_reply(WORKED_OK, buffer, root, command_reply_buffer);
		}
		else
		{
			// got a value part
			const char *inputSource = NULL;

			if (root["value"].is<int>())
			{
				//Serial.println("Got an int");
				// need to convert the input value into a string
				// as our value parser uses strings as inputs
				int v = root["value"];
				Serial.printf("  Setting int %d\n", v);
				snprintf(buffer, 120, "%d", v);
				inputSource = buffer;
			}
			else
			{
				if (root["value"].is<char*>())
				{
					inputSource = root["value"];
					Serial.printf("  Setting string %s\n", inputSource);
				}
				else
				{
					Serial.println("  Unrecognised setting");
				}
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

	if (validStoredSettings())
	{
		loadSettings();
		PrintSystemDetails();
		Serial.println("  Settings loaded OK");
	}
	else
	{
		resetSettings();
		saveSettings();
		PrintSystemDetails();
		Serial.println("  Settings Reset");
	}

	PrintAllSettings();
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
