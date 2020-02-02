#ifndef AIR_QUALITY_H

#define AIR_QUALITY_H

#include "sensors.h"

#define AIRQ_READING_TIMEOUT_MSECS 5000
#define AIRQ_NO_DATA_RECEIVED -1
#define AIRQ_NO_READING_DECODED -2
#define AIRQ_NO_SENSOR_DETECTED_AT_START -3
#define AIRQ_SENSOR_NOT_RECOGNIZED_AT_START -4

#define AIRQ_READING_LIFETIME_MSECS 5000
#define AIRQ_SERIAL_DATA_TIMEOUT_MILLIS 500
#define AIRQ_AVERAGE_LIFETIME_MSECS 10000

struct AirqualitySettings {
	int airqSensorType;
	int airqSecnondsSensorWarmupTime;
	int airqRXPinNo;
	int airqTXPinNo;
	int airqNoOfAverages;
};

extern struct AirqualitySettings airqualitySettings;

extern struct SettingItemCollection airQualitySettingItems;

extern int noOfAirQualitySettingItems;

struct airqualityReading {
	float pm25;
	float pm10;
	unsigned long lastAirqAverageMillis;
	int airAverageReadingCount;
	int airNoOfAveragesCalculated;
	float pm25Average;
	float pm10Average;
	// these are temporary values that are not for public use
	float pm25AvgTotal;
	float pm10AvgTotal;
	int averageCount;
	int readings;
	int errors;
};

int startAirq(struct sensor * airqSensor);
int updateAirqReading(struct sensor * airqSensor);
void startAirqReading(struct sensor * gpsSensor);
int addAirqReading(struct sensor * airqSensor, char * jsonBuffer, int jsonBufferSize);
void airqStatusMessage(struct sensor * airqSensor, char * buffer, int bufferLength);
void setParticleSensorWorking(bool working);
void resetAirqAverages(airqualityReading * reading);

#endif