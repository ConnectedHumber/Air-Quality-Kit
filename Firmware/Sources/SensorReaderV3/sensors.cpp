#include "sensors.h"
#include "airquality.h"
#include "bme280.h"
#include "clock.h"
#include "gps.h"
#include "pixels.h"
#include "settings.h"

struct sensor airqSensor = { 
	"Air quality",	// sensor name
	0,				// millis at last reading
	0,				// reading number
	0,				// 
	startAirq,		// start function
	updateAirqReading,	// update function
	startAirqReading,	// begin a reading sequence
	addAirqReading,		// add a reading to the JSON object
	airqStatusMessage,	// get a status messsage
	-1,					// get a status value
	false,				// sensor is active
	NULL,				// active sensor reading
	0,					// active time
	(unsigned char *) &airqualitySettings,// settings storage block
	sizeof(struct AirqualitySettings),	// number of bytes in the setting
	&airQualitySettingItems	// sensor setting items
	};


struct sensor gpsSensor = 
{ "GPS", 0, 0, 0, startGps, updateGpsReading, startGPSReading, addGpsReading, gpsStatusMessage, -1, false, NULL, 0, 
(unsigned char *) &gpsSetting, sizeof(struct GpsSetting), &gpsSettingItems };

struct sensor bme280Sensor = { "BME280", 0, 0, 0, startBme280, updateBME280Reading, startBME280Reading, addBME280Reading, bme280StatusMessage, -1, false, NULL, 0,
	(unsigned char*)&bmeSettings, sizeof(struct Bme280Settings), &bme280SettingItems };

struct sensor clockSensor = { "Clock", 0, 0, 0, startClock, updateClockReading, startClockReading, addClockReading, clockStatusMessage, -1, false, NULL, 0,
	NULL, 0, NULL 
};

struct sensor * sensorList[] =
{
	&bme280Sensor,
	&airqSensor,
	&clockSensor,
	&gpsSensor
};

struct sensor * findSensorByName(const char * name)
{
	for (int i = 0; i < sizeof(sensorList) / sizeof(struct sensor *); i++)
	{
		if (strcasecmp(sensorList[i]->sensorName, name) == 0)
		{
			return sensorList[i];
		}
	}
	return NULL;
}

#define SENSOR_STATUS_BUFFER_SIZE 300

char sensorStatusBuffer[SENSOR_STATUS_BUFFER_SIZE];

#define SENSOR_VALUE_BUFFER_SIZE 300

char sensorValueBuffer[SENSOR_VALUE_BUFFER_SIZE];

void startSensors()
{
	// start all the sensor managers
	for (int i = 0; i < sizeof(sensorList) / sizeof(struct sensor *); i++)
	{
		struct sensor * startSensor = sensorList[i];
		Serial.printf("Starting sensor %s: ", startSensor->sensorName);
		startSensor->startSensor(sensorList[i]);
		startSensor->getStatusMessage(startSensor, sensorStatusBuffer, SENSOR_STATUS_BUFFER_SIZE);
		Serial.printf("%s\n", sensorStatusBuffer);
		startSensor->beingUpdated = true;
		addStatusItem(startSensor->status == SENSOR_OK);
		renderStatusDisplay();
	}
}

void dumpSensorStatus()
{
	Serial.println("Sensors");
	unsigned long currentMillis = millis();
	for (int i = 0; i < sizeof(sensorList) / sizeof(struct sensor *); i++)
	{
		sensorList[i]->getStatusMessage(sensorList[i], sensorStatusBuffer, SENSOR_STATUS_BUFFER_SIZE);
		sensorValueBuffer[0] = 0; // empty the buffer string
		sensorList[i]->addReading(sensorList[i], sensorValueBuffer, SENSOR_VALUE_BUFFER_SIZE);
		Serial.printf("    %s  %s Active time(microsecs): ",
			sensorStatusBuffer, sensorValueBuffer);
		Serial.print(sensorList[i]->activeTime);
		Serial.print("  Millis since last reading: ");
		Serial.println(ulongDiff(currentMillis, sensorList[i]->millisAtLastReading));
	}
}

void startSensorsReading()
{
	for (int i = 0; i < sizeof(sensorList) / sizeof(struct sensor *); i++)
	{
		if (sensorList[i]->beingUpdated)
		{
			sensorList[i]->startReading(sensorList[i]);
		}
	}
}

void updateSensors()
{
	for (int i = 0; i < sizeof(sensorList) / sizeof(struct sensor *); i++)
	{
		if (sensorList[i]->beingUpdated)
		{
			//Serial.print(sensorList[i]->sensorName);
			//Serial.print(' ');
			unsigned long startMicros = micros();
			sensorList[i]->updateSensor(sensorList[i]);
			sensorList[i]->activeTime = ulongDiff(micros(), startMicros);
		}
	}
}

void createSensorJson(char * buffer, int bufferLength)
{
	snprintf(buffer, bufferLength, "{ \"dev\":\"%s\"", settings.deviceName);

	for (int i = 0; i < sizeof(sensorList) / sizeof(struct sensor *); i++)
	{
		if (sensorList[i]->beingUpdated)
		{
			sensorList[i]->addReading(sensorList[i], buffer, bufferLength);
		}
	}
	snprintf(buffer, bufferLength, "%s}", buffer);
}

void displaySensorStatus()
{
	for (int i = 0; i < sizeof(sensorList) / sizeof(struct sensor *); i++)
	{
		sensor * displayProcess = sensorList[i];
		addStatusItem(displayProcess->status == SENSOR_OK);
	}
}

void iterateThroughSensors ( void (*func) (sensor * s) )
{
	for (int i = 0; i < sizeof(sensorList) / sizeof(struct sensor*); i++)
	{
		func(sensorList[i]);
	}
}

void iterateThroughSensorSettingCollections(void (*func) (SettingItemCollection* s))
{
	for (int i = 0; i < sizeof(sensorList) / sizeof(struct sensor*); i++)
	{
		if (sensorList[i]->settingItems != NULL)
		{
			func(sensorList[i]->settingItems);
		}
	}
}


void iterateThroughSensorSettings(void (*func) (SettingItem* s))
{
	for (int i = 0; i < sizeof(sensorList) / sizeof(struct sensor*); i++)
	{
		if (sensorList[i]->settingItems != NULL)
		{
			for (int j = 0; j++; j < sensorList[i]->settingItems->noOfSettings)
			{
				func(sensorList[i]->settingItems->settings[j]);
			}
		}
	}
}

void resetSensorsToDefaultSettings()
{
	for (int i = 0; i < sizeof(sensorList) / sizeof(struct sensor*); i++)
	{
		if (sensorList[i]->settingItems != NULL)
		{
			for (int j = 0; j < sensorList[i]->settingItems->noOfSettings; j++)
			{
				void* dest = sensorList[i]->settingItems->settings[j]->value;
				sensorList[i]->settingItems->settings[j]->setDefault(dest);
			}
		}
	}
}

SettingItem* FindSensorSettingByFormName(const char* settingName)
{
	for (int i = 0; i < sizeof(sensorList) / sizeof(struct sensor*); i++)
	{
		sensor* testSensor = sensorList[i];

		if (testSensor->settingItems != NULL)
		{
			SettingItemCollection* testItems = testSensor->settingItems;

			for (int j = 0; j++; j < testSensor->settingItems->noOfSettings)
			{
				SettingItem * testSetting = testItems->settings[j];
				if (strcasecmp(settingName, testSetting->formName))
				{
					return testSetting;
				}
			}
		}
	}
	return NULL;
}
