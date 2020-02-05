#include "bme280.h"
#include "debug.h"
#include "sensors.h"
#include "settings.h"

struct Bme280Settings bmeSettings;

void setDefaultEnvnoOfAverages(void* dest)
{
	int* destInt = (int*)dest;
	*destInt = 25;
}

struct SettingItem bme280FittedSetting = {
"BME 280 fitted (yes or no)",
"bme280fitted",
& bmeSettings.bme280Fitted,
ONOFF_INPUT_LENGTH,
yesNo,
setTrue,
validateYesNo };

struct SettingItem envNoOfAveragesSetting = {
"Environment Number of averages",
"envnoOfAverages",
&bmeSettings.envNoOfAverages,
NUMBER_INPUT_LENGTH,
integerValue,
setDefaultEnvnoOfAverages,
validateInt };

struct SettingItem* bme280SettingItemPointers[] =
{
	&bme280FittedSetting,
	&envNoOfAveragesSetting
};

struct SettingItemCollection bme280SettingItems = {
	"bme280",
	"BME280 Setting Management",
	bme280SettingItemPointers,
	sizeof(bme280SettingItemPointers) / sizeof(struct SettingItem*)
};

Adafruit_BME280 bme;

int bmeAddresses[] = { 0x76, 0x77 };

int startBme280(struct sensor * bme280Sensor)
{
	if (!bmeSettings.bme280Fitted)
	{
		bme280Sensor->status = BME280_NOT_FITTED;
		return BME280_NOT_FITTED;
	}

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
			float testTemp = bme.readTemperature();

			if (isnan(testTemp))
			{
				// ignpore non-numbers
				continue;
			}

			bme280activeReading->activeBMEAddress = bmeAddresses[i];
			bme280Sensor->status = SENSOR_OK;
			return SENSOR_OK;
		}
	}

	bme280Sensor->status = BME280_NOT_CONNECTED;

	return BME280_NOT_CONNECTED;
}

int stopBme280(struct sensor* bme280Sensor)
{
	return SENSOR_OK;
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

	if (reading->averageCount == bmeSettings.envNoOfAverages)
	{
		reading->temperatureAverage = reading->temperatureTotal / bmeSettings.envNoOfAverages;
		reading->pressureAverage = reading->pressureTotal / bmeSettings.envNoOfAverages;
		reading->humidityAverage = reading->humidityTotal / bmeSettings.envNoOfAverages;
		reading->lastEnvqAverageMillis = offsetMillis();
		reading->envNoOfAveragesCalculated++;
		resetEnvqAverages(reading);
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
	switch (bme280Sensor->status)
	{
	case SENSOR_OK:
	{
		struct bme280Reading* bme280activeReading =
			(struct bme280Reading*) bme280Sensor->activeReading;

		float temp = bme.readTemperature();

		if (isnan(temp))
		{
			bme280Sensor->status = BME280_NOT_CONNECTED;
		}
		else
		{
			bme280activeReading->temperature = temp;
			bme280activeReading->humidity = bme.readHumidity();
			bme280activeReading->pressure = bme.readPressure() / 100.0F;
			bme280Sensor->millisAtLastReading = offsetMillis();
			bme280Sensor->readingNumber++;
			updateEnvAverages(bme280activeReading);
		}
	}
		break;

	case BME280_NOT_CONNECTED:
		break;

	case BME280_NOT_FITTED:
		break;
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
		snprintf(buffer, bufferLength, "BME 280 sensor connected at %x last transmitted reading %d averages %d", 
			bme280activeReading->activeBMEAddress, 
			bme280sensor->lastTransmittedReadingNumber, 
			bme280activeReading->envNoOfAveragesCalculated);
		break;

	case SENSOR_OFF:
		snprintf(buffer, bufferLength, "BME 280 sensor off");
		break;

	case BME280_NOT_CONNECTED:
		snprintf(buffer, bufferLength, "BME 280 sensor not connected");
		break;

	case BME280_NOT_FITTED:
		snprintf(buffer, bufferLength, "BME 280 sensor not fitted");
		break;

	default:
		snprintf(buffer, bufferLength, "BME 280 status invalid");
		break;
	}
}

