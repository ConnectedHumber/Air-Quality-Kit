#ifndef SENSORS_H

#define SENSORS_H

#include <Arduino.h>
#include "settings.h"

#define SENSOR_OK 0
#define SENSOR_OFF 1

struct sensor
{
	char * sensorName;
	unsigned long millisAtLastReading;
	int readingNumber;
	int lastTransmittedReadingNumber;
	int(*startSensor)(struct sensor *);
	int(*updateSensor)(struct sensor *);
	void(*startReading)(struct sensor *);
	int(*addReading)(struct sensor *, char * jsonBuffer, int jsonBufferSize);
	void(*getStatusMessage)(struct sensor *, char * buffer, int bufferLength);
	int status;      // zero means OK - any other value is an error state
	boolean beingUpdated;  // active means that the sensor will be updated 
	void * activeReading;
	unsigned int activeTime;
	unsigned char* settingsStoreBase;
	int settingsStoreLength;
	struct SettingItemCollection* settingItems;
};

struct sensor * findSensorByName(const char * name);

extern struct sensor gpsSensor;
extern struct sensor bme280Sensor;
extern struct sensor airqSensor;
extern struct sensor clockSensor;

void startSensors();

void dumpSensorStatus();

void startSensorsReading();

void updateSensors();

void createSensorJson(char * buffer, int bufferLength);

void displaySensorStatus();

SettingItem* FindSensorSettingByFormName(const char* settingName);
struct sensor * findSensorSettingCollectionByName(const char * name);

void resetSensorsToDefaultSettings();

void iterateThroughSensors(void (*func) (sensor* s));
void iterateThroughSensorSettings(void (*func) (SettingItem* s));
void iterateThroughSensorSettingCollections(void (*func) (SettingItemCollection* s));
void iterateThroughSensorSecttings(void (*func) (unsigned char* settings, int size));

#endif