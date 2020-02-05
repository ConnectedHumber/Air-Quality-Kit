#include <Arduino.h>

#include <MicroNMEA.h>

#include "utils.h"
#include "sensors.h"
#include "settings.h"
#include "processes.h"
#include "gps.h"

struct GpsSetting gpsSetting;

void setDefaultGpsPinNo(void* dest)
{
	int* destInt = (int*)dest;
	*destInt = 35;
}

struct SettingItem gpsFittedSetting = {
"GPS fitted (yes or no)",
"gpsfitted",
& gpsSetting.gpsFitted,
ONOFF_INPUT_LENGTH,
yesNo,
setFalse,
validateYesNo };

struct SettingItem gpsRXPinNoSetting = {
"GPS RX Pin",
"gpsrxpin",
& gpsSetting.gpsRXPinNo,
NUMBER_INPUT_LENGTH,
integerValue,
setDefaultGpsPinNo,
validateInt };

void setDefaultPositionValue(void* dest)
{
	double* destDouble = (double*)dest;
	*destDouble = -1000;
}

struct SettingItem fixedLocationSetting = {
		"Fixed location", "fixedlocation", & gpsSetting.fixedLocation, ONOFF_INPUT_LENGTH, yesNo, setTrue, validateYesNo };

struct SettingItem lattitudeSetting = {
		"Device lattitude", "lattitude", & gpsSetting.lattitude, NUMBER_INPUT_LENGTH, doubleValue, setDefaultPositionValue, validateDouble };

struct SettingItem longitudeSetting = {
		"Device longitude", "longitude", & gpsSetting.longitude, NUMBER_INPUT_LENGTH, doubleValue, setDefaultPositionValue, validateDouble };

struct SettingItem indoorDeviceSetting = {
		"Device indoors", "indoorDevice", & gpsSetting.indoorDevice, ONOFF_INPUT_LENGTH, yesNo, setFalse, validateYesNo };

struct SettingItem* GpsSettingItemPointers[] =
{
	&gpsFittedSetting,
	&gpsRXPinNoSetting,
	&fixedLocationSetting,
	&lattitudeSetting,
	&longitudeSetting,
	&indoorDeviceSetting
};

struct SettingItemCollection gpsSettingItems = {
	"gps",
	"Set the GPS RX pin and operating mode",
	GpsSettingItemPointers,
	sizeof(GpsSettingItemPointers) / sizeof(struct SettingItem*)
};


char nmeaBuffer[100];
MicroNMEA nmea(nmeaBuffer, sizeof(nmeaBuffer));

HardwareSerial * gpsSerial ;

unsigned long gpsReadingStartTime;

unsigned long lastGPSserialDataReceived;
int gpsCharCount = 0;

void printUnknownSentence(const MicroNMEA& nmea)
{
	//	TRACELN();
	//	TRACE("Unknown sentence: ");
	//	TRACELN(nmea.getSentence());
}

int startGps(struct sensor * gpsSensor)
{
	gpsCharCount = 0;
	struct gpsReading * activeGPSReading ;

	if (gpsSensor->activeReading == NULL)
	{
		activeGPSReading = new gpsReading();
		gpsSensor->activeReading = activeGPSReading;
	}
	else
	{
		activeGPSReading =
			(struct gpsReading *) gpsSensor->activeReading;
	}

	// Open the serial port
	if (gpsSerial == NULL)
	{
		gpsSerial = new HardwareSerial(2);
		gpsSerial->begin(9600, SERIAL_8N1, gpsSetting.gpsRXPinNo, -1);
	}

	lastGPSserialDataReceived = offsetMillis();

	nmea.setUnknownSentenceHandler(printUnknownSentence);

	gpsSensor->status = GPS_ERROR_NO_DATA_RECEIVED;
	return gpsSensor->status;
}

int stopGps(struct sensor* airqSensor)
{
	return SENSOR_OK;
}


void startGPSReading(struct sensor * gpsSensor)
{
}

int updateGpsReading(struct sensor * gpsSensor)
{
	struct gpsReading * activeGPSReading;
	activeGPSReading =
		(struct gpsReading *) gpsSensor->activeReading;

	unsigned long updateMillis = offsetMillis();

	if (gpsSerial->available() == 0)
	{
		if (gpsSensor->status != GPS_ERROR_NO_DATA_RECEIVED)
		{
			unsigned long timeSinceLastSerialData = ulongDiff(updateMillis, lastGPSserialDataReceived);

			// if the data timeout has expired, change the state
			if (timeSinceLastSerialData > GPS_SERIAL_DATA_TIMEOUT_MILLIS)
				gpsSensor->status = GPS_ERROR_NO_DATA_RECEIVED;
		}

		return gpsSensor->status;
	}

	// got some serial data - yay

	lastGPSserialDataReceived = updateMillis;

	if (gpsSensor->status == GPS_ERROR_NO_DATA_RECEIVED)
	{
		// Now have data - but no fix
		gpsSensor->status = GPS_ERROR_NO_FIX;
	}

	while (gpsSerial->available())
	{
		gpsCharCount++;

		char ch = gpsSerial->read();
		
		nmea.process(ch);

		if (nmea.isValid())
		{
			activeGPSReading->lattitude = nmea.getLatitude() / 1000000.0;
			activeGPSReading->longitude = nmea.getLongitude() / 1000000.0;
			gpsSensor->millisAtLastReading = updateMillis;
			gpsSensor->readingNumber++;
			gpsSensor->status = SENSOR_OK;
		}
	}

	unsigned long millisSinceGPSupate = ulongDiff(updateMillis, gpsSensor->millisAtLastReading);
		
	if (millisSinceGPSupate > GPS_READING_LIFETIME_MSECS)
	{
		gpsSensor->status = GPS_ERROR_NO_FIX;
	}

	return gpsSensor->status;
}

int addGpsReading(struct sensor * gpsSensor, char * jsonBuffer, int jsonBufferSize)
{
	struct gpsReading * activeGPSReading;
	activeGPSReading =
		(struct gpsReading *) gpsSensor->activeReading;

	if (ulongDiff(offsetMillis(), gpsSensor->millisAtLastReading) < GPS_READING_LIFETIME_MSECS)
	{
		snprintf(jsonBuffer, jsonBufferSize, "%s,\"Lat\" : %.6f, \"Long\" : %.6f,",
			jsonBuffer,
			activeGPSReading->lattitude, activeGPSReading->longitude);
	}

	return gpsSensor->status;
}

void gpsStatusMessage(struct sensor * gpsSensor, char * buffer, int bufferLength)
{
	struct gpsReading * activeGPSReading;
	activeGPSReading =
		(struct gpsReading *) gpsSensor->activeReading;

	switch (gpsSensor->status)
	{
	case SENSOR_OK:
		if (ulongDiff(offsetMillis(), gpsSensor->millisAtLastReading) < GPS_READING_LIFETIME_MSECS)
			snprintf(buffer, bufferLength, "GPS sensor OK Lat: %.6f Long: %.6f Fix valid",
				activeGPSReading->lattitude, activeGPSReading->longitude);
		else
			snprintf(buffer, bufferLength, "GPS sensor OK Lat: %.6f Long: %.6f Fix timed out",
				activeGPSReading->lattitude, activeGPSReading->longitude);
		break;
	case GPS_ERROR_NO_DATA_RECEIVED:
		snprintf(buffer, bufferLength, "GPS sensor no data received");
		break;
	case GPS_ERROR_NO_FIX:
		snprintf(buffer, bufferLength, "GPS sensor no fix chars: %d", gpsCharCount);
		break;
	}
}
