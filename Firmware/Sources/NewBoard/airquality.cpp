#include <Arduino.h>

#include "utils.h"
#include "sensors.h"
#include "airquality.h"
#include "settings.h"
#include "processes.h"
#include "debug.h"

#include <HardwareSerial.h>

#define AIRQ_SERIAL_DATA_TIMEOUT_MILLIS 2000

struct AirqualitySettings airqualitySettings;

void setDefaultAirQSensorType(void* dest)
{
	int* destInt = (int*)dest;
	*destInt = UNKNOWN_SENSOR;
}

void setDefaultAirQWarmupTime(void* dest)
{
	int* destInt = (int*)dest;
	*destInt = 30;
}

void setDefaultAirqRXpin(void* dest)
{
	int* destInt = (int*)dest;
	*destInt = 16;
}

void setDefaultAirqTXpin(void* dest)
{
	int* destInt = (int*)dest;
	*destInt = 17;
}

void setDefaultAirqnoOfAverages(void* dest)
{
	int* destInt = (int*)dest;
	*destInt = 25;
}


struct SettingItem airqSensorTypeSetting = {
"AirQ Sensor type (0 = not fitted 1=SDS011, 2=ZPH01)",
"airqsensortype",
&airqualitySettings.airqSensorType,
NUMBER_INPUT_LENGTH,
integerValue,
setDefaultAirQSensorType,
validateInt };

struct SettingItem airqSecondsSensorWarmupTimeSetting = {
"AirQ Seconds for sensor warmup",
"airqsensorwarmup",
&airqualitySettings.airqSecnondsSensorWarmupTime,
NUMBER_INPUT_LENGTH,
integerValue,
setDefaultAirQWarmupTime,
validateInt
};

struct SettingItem airqRXPinNoSetting = {
"AirQ RX Pin",
"airqrxpinno",
&airqualitySettings.airqRXPinNo,
NUMBER_INPUT_LENGTH,
integerValue,
setDefaultAirqRXpin,
validateInt
};

struct SettingItem airqTXPinNoSetting = {
"AirQ TX Pin",
"airqtxpinno",
&airqualitySettings.airqTXPinNo,
NUMBER_INPUT_LENGTH,
integerValue,
setDefaultAirqTXpin,
validateInt };

struct SettingItem airqNoOfAveragesSetting = {
"AirQ Number of averages",
"airqnoOfAverages",
&airqualitySettings.airqNoOfAverages,
NUMBER_INPUT_LENGTH,
integerValue,
setDefaultAirqnoOfAverages,
validateInt };

struct SettingItem* airQualitySettingItemPointers[] =
{
	&airqSensorTypeSetting,
	&airqRXPinNoSetting,
	&airqTXPinNoSetting,
	&airqNoOfAveragesSetting
};

struct SettingItemCollection airQualitySettingItems = {
	"airq",
	"Set pin assignments and number of averages for the air quality sensor",
	airQualitySettingItemPointers,
	sizeof(airQualitySettingItemPointers) / sizeof(struct SettingItem*)
};


HardwareSerial* airqSensorSerial = NULL;

unsigned long lastAirqSerialDataReceived;

#define AIRQ_SENSOR_START_SEQUENCE_LENGTH 2

struct airqSensorStartSequence
{
	int sensorType;
	char sequence[AIRQ_SENSOR_START_SEQUENCE_LENGTH];
	int pos;
};

struct airqSensorStartSequence airqStartSequences[] =
{
	{ SDS011_SENSOR, {170,192}, 0 },
	{ ZPH01_SENSOR, {0xFF,0x18}, 0 },
	{ PMS5003_SENSOR, {0x42, 0x4D}, 0}
};


void resetAirqAverages(airqualityReading* reading)
{
	reading->pm10AvgTotal = 0;
	reading->pm25AvgTotal = 0;
	reading->averageCount = 0;
}

void updateAirqAverages(airqualityReading* reading)
{

	reading->pm10AvgTotal += reading->pm10;
	reading->pm25AvgTotal += reading->pm25;

	reading->averageCount++;

	if (reading->averageCount == airqualitySettings.airqNoOfAverages)
	{
		reading->pm10Average = reading->pm10AvgTotal / airqualitySettings.airqNoOfAverages;
		reading->pm25Average = reading->pm25AvgTotal / airqualitySettings.airqNoOfAverages;
		reading->lastAirqAverageMillis = offsetMillis();
		reading->airNoOfAveragesCalculated++;
		resetAirqAverages(reading);
	}
}

int getSensorType(int* sensorType)
{
	byte buffer;
	int value;
	boolean gotSerialData = false;
	unsigned long airqReadingStartTime;

	airqReadingStartTime = offsetMillis();

	for (int i = 0; i < sizeof(airqStartSequences) / sizeof(struct airqSensorStartSequence); i++)
	{
		airqStartSequences[i].pos = 0;
	}

	while (true)
	{
		if (ulongDiff(offsetMillis(), airqReadingStartTime) > AIRQ_READING_TIMEOUT_MSECS)
		{
			if (gotSerialData)
				return AIRQ_SENSOR_NOT_RECOGNIZED_AT_START;
			else
				return AIRQ_NO_SENSOR_DETECTED_AT_START;
		}

		if (airqSensorSerial->available() == 0)
		{
			delay(1);
			continue;
		}

		gotSerialData = true;

		buffer = airqSensorSerial->read();

		value = int(buffer);

		for (int i = 0; i < sizeof(airqStartSequences) / sizeof(struct airqSensorStartSequence); i++)
		{
			if (value == airqStartSequences[i].sequence[airqStartSequences[i].pos])
			{
				airqStartSequences[i].pos++;
				if (airqStartSequences[i].pos == AIRQ_SENSOR_START_SEQUENCE_LENGTH)
				{
					// found a match to the sensor
					*sensorType = airqStartSequences[i].sensorType;
					return SENSOR_OK;
				}
			}
			else
			{
				airqStartSequences[i].pos = 0;
			}
		}
	}
}

int startAirq(struct sensor* airqSensor)
{
	struct airqualityReading* airqualityActiveReading;

	if (airqSensor->activeReading == NULL)
	{
		airqualityActiveReading = new airqualityReading();
		airqSensor->activeReading = airqualityActiveReading;
	}
	else
	{
		airqualityActiveReading =
			(struct airqualityReading*) airqSensor->activeReading;
	}

	airqualityActiveReading->errors = 0;
	airqualityActiveReading->readings = 0;

	// Open the serial port
	if (airqSensorSerial == NULL)
	{
		airqSensorSerial = new HardwareSerial(1);

		// airqSensorSerial->begin(9600, SERIAL_8N1, 17, 13);  // badud, mode, rx, tx
		airqSensorSerial->begin(9600, SERIAL_8N1, airqualitySettings.airqRXPinNo, airqualitySettings.airqTXPinNo);  // badud, mode, rx, tx
	}

	int sensorType;

	int getSensorTypeResult = getSensorType(&sensorType);

	if (getSensorTypeResult != SENSOR_OK)
	{
		return getSensorTypeResult;
	}

	if (sensorType != airqualitySettings.airqSensorType)
	{
		airqualitySettings.airqSensorType = sensorType;
		saveSettings();
	}

	lastAirqSerialDataReceived = offsetMillis();

	airqSensor->status = AIRQ_NO_DATA_RECEIVED;
	return AIRQ_NO_DATA_RECEIVED;
}

int sds011Len = 0;
int sds011Pm10Serial = 0;
int sds011PM25Serial = 0;
byte sds011CalcChecksum;
byte sds011RecChecksum;
int sds011ChecksumOK = 0;

boolean pumpSDS011Byte(airqualityReading* result, byte sds011Value)
{
	//Serial.print(sds011Value, HEX);
	//Serial.print(' ');

	switch (sds011Len) {
	case (0): if (sds011Value != 170) { sds011Len = -1; }; break;
	case (1): if (sds011Value != 192) { sds011Len = -1; }; break;
	case (2): sds011PM25Serial = sds011Value; sds011CalcChecksum = sds011Value; break;
	case (3): sds011PM25Serial += (sds011Value << 8); sds011CalcChecksum += sds011Value; break;
	case (4): sds011Pm10Serial = sds011Value; sds011CalcChecksum += sds011Value; break;
	case (5): sds011Pm10Serial += (sds011Value << 8); sds011CalcChecksum += sds011Value; break;
	case (6): sds011CalcChecksum += sds011Value; break;
	case (7): sds011CalcChecksum += sds011Value; break;
	case (8): sds011RecChecksum = sds011Value; if (sds011Value == (sds011CalcChecksum % 256)) { sds011ChecksumOK = 1; }
			else { sds011Len = -1; }; break;
	case (9): if (sds011Value != 171) { sds011Len = -1; }; break;
	}

	sds011Len++;

	if (sds011Len == 10)
	{
		sds011Len = 0;
		if (sds011ChecksumOK == 1)
		{
			float pm10 = sds011Pm10Serial / 10.0;
			float pm25 = sds011PM25Serial / 10.0;

			result->pm10 = pm10;
			result->pm25 = pm25;

			result->readings++;

			updateAirqAverages(result);
			return true;
		}
		else
		{
			result->errors++;
		}
	}

	return false;
}

int zph01Len = 0;
byte zph01CalcChecksum;
byte zph01RecChecksum;
float zph01Pm25Serial = 0;

boolean pumpZPH01Byte(airqualityReading* result, byte zph01Value)
{
	boolean gotResult = false;

	switch (zph01Len) {
	case (0): if (zph01Value != 0xFF) { zph01Len = -1; }; break;
	case (1): if (zph01Value != 0x18) { zph01Len = -1; }
			else { zph01CalcChecksum = zph01Value; }; break;
	case (2): if (zph01Value != 0) { zph01Len = -1; }; break;
	case (3): zph01Pm25Serial = zph01Value; zph01CalcChecksum += zph01Value; break;
	case (4): zph01Pm25Serial += (zph01Value / 100.0); zph01CalcChecksum += zph01Value; break;
	case (5): zph01CalcChecksum += zph01Value; break;
	case (6): if (zph01Value != 0x01) { zph01Len = -1; }
			else { zph01CalcChecksum += zph01Value; }; break;
	case (7): zph01CalcChecksum += zph01Value; break;
	case (8):  break;
	}

	zph01Len++;

	if (zph01Len == 9)
	{
		zph01CalcChecksum = (~zph01CalcChecksum) + 1;

		if (zph01Value == zph01CalcChecksum)
		{
			float pm25 = zph01Pm25Serial * 20;

			result->pm25 = pm25;
			result->pm10 = -1;

			updateAirqAverages(result);

			result->readings++;

			gotResult = true;
		}
		else
		{
			result->errors++;
		}
		zph01Len = 0;
	}

	return gotResult;
}

int pms5003Len = 0;
int pms5003Pm10Serial = 0;
int pms5003Pm25Serial = 0;
unsigned int pms5003CalcChecksum;
unsigned int pms5003RecChecksum;

boolean pumppms5003Byte(airqualityReading* result, byte pms5003Value)
{
	switch (pms5003Len) {
	case (0): if (pms5003Value != 0x42) { pms5003Len = -1; }; pms5003CalcChecksum = 0x42; break;
	case (1): if (pms5003Value != 0x4d) { pms5003Len = -1; }; pms5003CalcChecksum += pms5003Value;  break;
	case (2): /* Frame Length High:*/  if (pms5003Value != 0) { pms5003Len = -1; }; pms5003CalcChecksum += pms5003Value; break;
	case (3): /* Frame Length Low:*/  if (pms5003Value != 0x1c) { pms5003Len = -1; }; pms5003CalcChecksum += pms5003Value; break;
	case (4): /* PM1 standard particle High:*/  pms5003CalcChecksum += pms5003Value; break;
	case (5): /* PM1 standard particle Low:*/  pms5003CalcChecksum += pms5003Value; break;
	case (6): /* PM25 standard particle High:*/  pms5003CalcChecksum += pms5003Value; break;
	case (7): /* PM25 standard particle Low:*/  pms5003CalcChecksum += pms5003Value; break;
	case (8): /* PM10 standard particle High:*/  pms5003CalcChecksum += pms5003Value; break;
	case (9): /* PM10 standard particle Low:*/  pms5003CalcChecksum += pms5003Value; break;
	case (10): /* PM10 mgram/m3 High:*/ pms5003Pm10Serial = pms5003Value; pms5003CalcChecksum += pms5003Value; break;
	case (11): /* PM10 mgram/m3 Low:*/ pms5003Pm10Serial = (pms5003Pm10Serial << 8) + pms5003Value; pms5003CalcChecksum += pms5003Value; break;
	case (12): /* PM25 mgram/m3 High:*/  pms5003Pm25Serial = pms5003Value;  pms5003CalcChecksum += pms5003Value; break;
	case (13): /* PM25 mgram/m3 Low:*/  pms5003Pm25Serial = (pms5003Pm25Serial << 8) + pms5003Value;  pms5003CalcChecksum += pms5003Value; break;
	case (14): /* Cocentration Unit High:*/  pms5003CalcChecksum += pms5003Value; break;
	case (15): /* Concentration Unit Low:*/  pms5003CalcChecksum += pms5003Value; break;
	case (16): /* Particles>0.3um in 1 li High:*/  pms5003CalcChecksum += pms5003Value; break;
	case (17): /* Particles>0.3um in 1 li Low:*/  pms5003CalcChecksum += pms5003Value; break;
	case (18): /* Particles>0.5um in 1 li High:*/  pms5003CalcChecksum += pms5003Value; break;
	case (19): /* Particles>0.5um in 1 li Low:*/  pms5003CalcChecksum += pms5003Value; break;
	case (20): /* Particles>1.0um in 1 li High:*/  pms5003CalcChecksum += pms5003Value; break;
	case (21): /* Particles>1.0um in 1 li Low:*/  pms5003CalcChecksum += pms5003Value; break;
	case (22): /* Particles>2.5um in 1 li High:*/  pms5003CalcChecksum += pms5003Value; break;
	case (23): /* Particles>2.5um in 1 li Low:*/  pms5003CalcChecksum += pms5003Value; break;
	case (24): /* Particles>5.0um in 1 li High:*/  pms5003CalcChecksum += pms5003Value; break;
	case (25): /* Particles>5.0um in 1 li Low:*/  pms5003CalcChecksum += pms5003Value; break;
	case (26): /* Particles>10.0um in 1 li High:*/  pms5003CalcChecksum += pms5003Value; break;
	case (27): /* Particles>10.0um in 1 li Low:*/  pms5003CalcChecksum += pms5003Value; break;
	case (28): /* Reserved High:*/  pms5003CalcChecksum += pms5003Value; break;
	case (29): /* Reserved Low:*/  pms5003CalcChecksum += pms5003Value; break;
	case (30): /* Check High:*/  pms5003RecChecksum = pms5003Value; break;
	case (31): /* Check Low:*/
		pms5003RecChecksum = (pms5003RecChecksum << 8) + pms5003Value; break;
	}

	pms5003Len++;

	if (pms5003Len == 32)
	{
		pms5003Len = 0;

		if (pms5003RecChecksum == pms5003CalcChecksum & 0xFFFF)
		{
			result->pm10 = pms5003Pm10Serial;
			result->pm25 = pms5003Pm25Serial;
			result->readings++;
			updateAirqAverages(result);
			return true;
		}
		else
		{
			result->errors++;
		}
	}
	return false;
}

uint8_t get_checksum(uint8_t* byte_buffer, int byte_buffer_length)
{
	uint8_t check_sum = 0;

	// TRACE("Check:");
	for (int i = 2; i < byte_buffer_length; i++)
	{
		// TRACE_HEX(byte_buffer[i]);
		// TRACE(" ");
		check_sum = check_sum + byte_buffer[i];
	}
	// TRACELN();
	return check_sum;
}

uint8_t reporting_mode_command[] = {
  0xAA, //  00  start of command
  0xB4, //  01  command ID - 0xb4 means read - 0xc5 means write
  0x02, //  02  data byte 1 always 2
  0x00, //  03  data byte 2 - 0 means query 1 means set
  0x00, //  04  data byte 3 - 0 means report active 1 means report query
  0x00, //  05  data byte 4- 0 (reserved) 
  0x00, //  06  data byte 5 - 0 (reserved) or Device ID byte 1
  0x00, //  07  data byte 6 - 0 (reserved) or Device ID byte 2
  0x00, //  08  data byte 7 - 0
  0x00, //  09  data byte 8 - 0
  0x00, //  10  data byte 9 - 0
  0x00, //  11  data byte 10 - 0
  0x00, //  12  data byte 11 - 0
  0x00, //  13  data byte 12 - 0
  0x00, //  14  data byte 13 - 0
  0xFF, //  15  0xFF means all sensors or device ID byte 1
  0xFF, //  16  0xFF means all sensors or device ID byte 2
  0x00, //  17  checksum
  0xAB  //  18  tail
};

void send_block(uint8_t* block_base, int block_length)
{
	int checksum_pos = block_length - 2;
	uint8_t checksum = get_checksum(reporting_mode_command, checksum_pos);

	// TRACE("Calculated checksum:");
	// TRACE_HEX(checksum);

	reporting_mode_command[checksum_pos] = checksum;

	for (int i = 0; i < block_length; i++)
	{

		// TRACE("Writing byte:");
		// TRACE_HEXLN(reporting_mode_command[i]);

		airqSensorSerial->write(reporting_mode_command[i]);
	}
}

void set_sds011_working(bool working)
{
	reporting_mode_command[1] = 0xB4;
	reporting_mode_command[2] = 6;
	reporting_mode_command[3] = 1;
	if (working)
		reporting_mode_command[4] = 1;
	else
		reporting_mode_command[4] = 0;
	Serial.println("working command being sent");
	send_block(reporting_mode_command, sizeof(reporting_mode_command) / sizeof(uint8_t));
}

void set_sensor_working(bool working)
{
	if (working)
	{
		TRACELN("Setting sensor working");
	}
	else
	{
		TRACELN("Setting sensor asleep");
	}

	switch (airqualitySettings.airqSensorType)
	{
	case UNKNOWN_SENSOR:
		break;

	case SDS011_SENSOR:
		set_sds011_working(working);
		break;

	case PMS5003_SENSOR:
		break;

	case ZPH01_SENSOR:
		break;
	}
}

int updateAirqReading(struct sensor* airqSensor)
{
	unsigned long updateMillis = offsetMillis();

	struct airqualityReading* airqualityActiveReading =
		(airqualityReading*)airqSensor->activeReading;

	int ch;
	boolean gotReading;

	if (airqSensorSerial->available() == 0)
	{
		if (airqSensor->status != AIRQ_NO_DATA_RECEIVED)
		{
			unsigned long timeSinceLastSerialData = ulongDiff(updateMillis, lastAirqSerialDataReceived);

			// if the data timeout has expired, change the state
			if (timeSinceLastSerialData > AIRQ_SERIAL_DATA_TIMEOUT_MILLIS)
				airqSensor->status = AIRQ_NO_DATA_RECEIVED;
		}
	}
	else
	{
		lastAirqSerialDataReceived = updateMillis;

		while (airqSensorSerial->available())
		{
			switch (airqSensor->status)
			{
			case AIRQ_NO_DATA_RECEIVED:
				airqSensor->status = AIRQ_NO_READING_DECODED;
				break;

			case AIRQ_NO_READING_DECODED:
			case SENSOR_OK:

				ch = airqSensorSerial->read();

				switch (airqualitySettings.airqSensorType)
				{
				case SDS011_SENSOR:
					gotReading = pumpSDS011Byte(airqualityActiveReading, (byte)ch);
					break;
				case ZPH01_SENSOR:
					gotReading = pumpZPH01Byte(airqualityActiveReading, (byte)ch);
					break;
				case PMS5003_SENSOR:
					gotReading = pumppms5003Byte(airqualityActiveReading, (byte)ch);
					break;
				}

				if (gotReading)
				{
					airqSensor->status = SENSOR_OK;
					airqSensor->millisAtLastReading = offsetMillis();
					airqSensor->readingNumber++;
					updateAirqAverages(airqualityActiveReading);
				}
				break;

			case AIRQ_NO_SENSOR_DETECTED_AT_START:
			case AIRQ_SENSOR_NOT_RECOGNIZED_AT_START:
				startAirq(airqSensor);
				break;
			}
		}
	}
	return airqSensor->status;
}

void startAirqReading(struct sensor* airqSensor)
{
	airqualityReading* airq = (airqualityReading*)airqSensor->activeReading;

	resetAirqAverages(airq);
}

int addAirqReading(struct sensor* airqSensor, char* jsonBuffer, int jsonBufferSize)
{
	struct airqualityReading* airqualityActiveReading =
		(airqualityReading*)airqSensor->activeReading;

	if (ulongDiff(offsetMillis(), airqSensor->millisAtLastReading) < AIRQ_READING_LIFETIME_MSECS)
	{
		struct airqualityReading* airqualityActiveReading =
			(airqualityReading*)airqSensor->activeReading;

		switch (airqualitySettings.airqSensorType)
		{
		case SDS011_SENSOR:
		case PMS5003_SENSOR:
			snprintf(jsonBuffer, jsonBufferSize, "%s,\"PM10\":%.2f,\"PM25\":%.2f",
				jsonBuffer,
				airqualityActiveReading->pm10Average, airqualityActiveReading->pm25Average);
			return SENSOR_OK;

		case ZPH01_SENSOR:
			snprintf(jsonBuffer, jsonBufferSize, "%s,\"PM25\":%.2f",
				jsonBuffer,
				airqualityActiveReading->pm25Average);
			return SENSOR_OK;
		}
	}

	return airqSensor->status;
}

void airqStatusMessage(struct sensor* airqSensor, char* buffer, int bufferLength)
{
	struct airqualityReading* airqualityActiveReading =
		(airqualityReading*)airqSensor->activeReading;

	switch (airqualitySettings.airqSensorType)
	{
	case ZPH01_SENSOR:
		snprintf(buffer, bufferLength, "ZPH01 sensor ");
		break;

	case SDS011_SENSOR:
		snprintf(buffer, bufferLength, "SDS011 sensor ");
		break;

	case PMS5003_SENSOR:
		snprintf(buffer, bufferLength, "PMS 5003 sensor ");
		break;

	default:
		snprintf(buffer, bufferLength, "Unknown sensor ");
		break;

	}

	snprintf(buffer, bufferLength, "%s Readings:%d Errors:%d last tranmitted reading %d averages %d",
		buffer,
		airqualityActiveReading->readings,
		airqualityActiveReading->errors,
		airqSensor->lastTransmittedReadingNumber,
		airqualityActiveReading->airNoOfAveragesCalculated);

	switch (airqSensor->status)
	{
	case SENSOR_OK:
		snprintf(buffer, bufferLength, "%s OK", buffer);
		break;

	case SENSOR_OFF:
		snprintf(buffer, bufferLength, "%s OFF", buffer);
		break;

	case AIRQ_NO_SENSOR_DETECTED_AT_START:
		snprintf(buffer, bufferLength, "No sensor detected at start");
		break;

	case AIRQ_NO_READING_DECODED:
		snprintf(buffer, bufferLength, "%s data received but not decoded", buffer);
		break;

	case AIRQ_NO_DATA_RECEIVED:
		snprintf(buffer, bufferLength, "%s no data received", buffer);
		break;

	case AIRQ_SENSOR_NOT_RECOGNIZED_AT_START:
		snprintf(buffer, bufferLength, "Sensor not recognised at start");
		break;

	default:
		snprintf(buffer, bufferLength, "%s Airq status invalid", buffer);
		break;
	}
}

void resetAllAirqualitySettings()
{

}