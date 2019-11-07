#include "bme280.h"
#include "debug.h"
#include "sensors.h"
#include "settings.h"

Adafruit_BME280 bme;

int bmeAddresses[] = { 0x76, 0x77 };

int startBme280(struct sensor * bme280Sensor)
{
	struct bme280Reading * bme280activeReading;

	if (bme280Sensor->activeReading == NULL)
	{
		bme280activeReading = new bme280Reading();
		bme280Sensor->activeReading = bme280activeReading;
	}
	else
	{
		bme280activeReading =
			(struct bme280Reading *) bme280Sensor->activeReading;
	}

	for (int i = 0; i < sizeof(bmeAddresses) / sizeof(int); i++)
	{
		if (bme.begin(bmeAddresses[i]))
		{
			bme280activeReading->activeBMEAddress = bmeAddresses[i];
			bme280Sensor->status = SENSOR_OK;
			return SENSOR_OK;
		}
	}

	bme280Sensor->status = BME280_NOT_CONNECTED;

	return BME280_NOT_CONNECTED;
}

void resetEnvqAverages(bme280Reading * reading)
{
	reading->temperatureTotal = 0;
	reading->pressureTotal = 0;
	reading->humidityTotal = 0;
	reading->averageCount = 0;
}

void updateEnvAverages(bme280Reading * reading)
{
	reading->temperatureTotal += reading->temperature;
	reading->pressureTotal += reading->pressure;
	reading->humidityTotal += reading->humidity;

	reading->averageCount++;

	if (reading->averageCount == settings.envNoOfAverages)
	{
		reading->temperatureAverage = reading->temperatureTotal / settings.airqNoOfAverages;
		reading->pressureAverage = reading->pressureTotal / settings.airqNoOfAverages;
		reading->humidityAverage = reading->humidityTotal / settings.airqNoOfAverages;
		reading->lastEnvqAverageMillis = millis();
		reading->envNoOfAveragesCalculated++;
	}
}


void startBME280Reading(struct sensor * bme280Sensor)
{
	struct bme280Reading * bme280activeReading =
		(struct bme280Reading *) bme280Sensor->activeReading;
	resetEnvqAverages(bme280activeReading);
}

int updateBME280Reading(struct sensor * bme280Sensor)
{
	if (bme280Sensor->status == SENSOR_OK)
	{
		struct bme280Reading * bme280activeReading =
			(struct bme280Reading *) bme280Sensor->activeReading;

		bme280activeReading->temperature = bme.readTemperature();
		bme280activeReading->humidity = bme.readHumidity();
		bme280activeReading->pressure = bme.readPressure() / 100.0F;
		bme280Sensor->millisAtLastReading = millis();
		bme280Sensor->readingNumber++;
		updateEnvAverages(bme280activeReading);
	}

	return bme280Sensor->status;
}

int addBME280Reading(struct sensor * bme280Sensor, char * jsonBuffer, int jsonBufferSize)
{
	if (bme280Sensor->status == SENSOR_OK)
	{
		struct bme280Reading * bme280activeReading =
			(struct bme280Reading *) bme280Sensor->activeReading;

		snprintf(jsonBuffer, jsonBufferSize, "%s,\"temp\":%.2f,\"humidity\":%.2f,\"pressure\":%.2f",
			jsonBuffer,
			bme280activeReading->temperature,
			bme280activeReading->humidity,
			bme280activeReading->pressure);
	}

	return bme280Sensor->status;
}

void bme280StatusMessage(struct sensor * bme280sensor, char * buffer, int bufferLength)
{
	struct bme280Reading * bme280activeReading =
		(struct bme280Reading *) bme280sensor->activeReading;

	switch (bme280sensor->status)
	{
	case SENSOR_OK:
		snprintf(buffer, bufferLength, "BME 280 sensor connected at %x", bme280activeReading->activeBMEAddress);
		break;

	case SENSOR_OFF:
		snprintf(buffer, bufferLength, "BME 280 sensor off");
		break;

	case BME280_NOT_CONNECTED:
		snprintf(buffer, bufferLength, "BME 280 sensor not connected");
		break;
	default:
		snprintf(buffer, bufferLength, "BME 280 status invalid");
		break;
	}
}

