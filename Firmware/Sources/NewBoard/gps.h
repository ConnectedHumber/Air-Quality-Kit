#ifndef GPS_H

#define GPS_H

#include <Arduino.h>

#include "sensors.h"

#define GPS_ERROR_NO_DATA_RECEIVED -1
#define GPS_ERROR_NO_FIX -2

#define GPS_READING_LIFETIME_MSECS 5000
#define GPS_SERIAL_DATA_TIMEOUT_MILLIS 500

struct gpsReading {
	double lattitude;
	double longitude;
};

int startGps(struct sensor * gpsSensor);
int updateGpsReading(struct sensor * gpsSensor);
int addGpsReading(struct sensor * gpsSensor, char * jsonBuffer, int jsonBufferSize);
void startGPSReading(struct sensor * gpsSensor);
void gpsStatusMessage(struct sensor * gpsSensor, char * buffer, int bufferLength);

struct GpsSetting {
	boolean indoorDevice;

	boolean fixedLocation;
	double lattitude;
	double longitude;

	boolean gpsFitted;
	int gpsRXPinNo;
};

extern struct GpsSetting gpsSetting;

extern struct SettingItemCollection gpsSettingItems;

#endif