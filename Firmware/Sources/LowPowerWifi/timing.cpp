#pragma once

#include <Arduino.h>

#include "timing.h"
#include "debug.h"
#include "utils.h"
#include "settings.h"
#include "sensors.h"
#include "mqtt.h"
#include "airquality.h"
#include "bme280.h"
#include "processes.h"
#include "control.h"
#include "statusled.h"

String loggingStateNames [] = { "off", "particles", "temp", "pressure", "humidity", "all"};

struct TimingSettings timingSettings;

void setDefaultPowerControlPinNo(void* dest)
{
	int* destInt = (int*)dest;
	*destInt = 27;
}


struct SettingItem powerControlPinSetting = {
		"Power Control Pin",
		"powercontrolpin",
		& timingSettings.powerControlPin,
		NUMBER_INPUT_LENGTH,
		integerValue,
		setDefaultPowerControlPinNo,
		validateInt };

struct SettingItem powerControlFittedSetting = {
		"Power Control fitted (yes or no)",
		"powercontrolfitted",
		& timingSettings.powerControlFitted,
		ONOFF_INPUT_LENGTH,
		yesNo,
		setFalse,
		validateYesNo };

struct SettingItem sleepProcessorSetting = {
		"Sleep processor between readings (yes or no)",
		"sleepProcessorSetting",
		& timingSettings.sleepProcessor,
		ONOFF_INPUT_LENGTH,
		yesNo,
		setFalse,
		validateYesNo };

boolean validateLoggingState(void* dest, const char* newValueStr)
{
	int value;

	if (sscanf(newValueStr, "%d", &value) != 1)
	{
		return false;
	}

	if (value < 0)
	{
		return false;
	}

	if (value > 5)
	{
		return false;
	}

	*(Logging_State*)dest = (Logging_State)value;
	return true;
}

void setDefaultLoggingState(void* dest)
{
	*(Logging_State*)dest = loggingOff;
}

struct SettingItem loggingSetting = {
		"Logging (0=off,1=air,2=temp,3=pres,4=hum,5=all)", "logging", &timingSettings.logging, NUMBER_INPUT_LENGTH, integerValue, setDefaultLoggingState, validateLoggingState };

struct SettingItem* timingSettingItemPointers[] =
{
	&powerControlPinSetting,
	&powerControlFittedSetting,
	&sleepProcessorSetting,
	&loggingSetting
};

struct SettingItemCollection timingSettingItems = {
	"timing",
	"Set logging options and power control setting for external devices",
	timingSettingItemPointers,
	sizeof(timingSettingItemPointers) / sizeof(struct SettingItem*)
};

unsigned long mqtt_reading_retry_interval_in_millis;
unsigned long mqtt_reading_interval_in_millis;
unsigned long milliseconds_at_last_mqtt_update;

unsigned long milliseconds_at_last_lora_update;

timingStates timing_state; 

timingStates getTimingState()
{
    return timing_state;
}

boolean mqttForceSend;

void force_mqtt_send ()
{
    mqttForceSend = true;
}

// return true if it is time for an update
bool mqtt_interval_has_expired()
{
	unsigned long time_in_millis = millis();
	unsigned long millis_since_last_mqtt_update = ulongDiff(time_in_millis, milliseconds_at_last_mqtt_update);


	if (millis_since_last_mqtt_update > mqtt_reading_interval_in_millis)
	{
		return true; 
	}
	return false;
}

unsigned long time_to_next_update()
{
	if (mqttSettings.mqtt_enabled)
	{
		unsigned long millis_since_last_mqtt_update = ulongDiff(millis(), milliseconds_at_last_mqtt_update);

		if (millis_since_last_mqtt_update > mqtt_reading_interval_in_millis)
			return 0;
		else
			return mqtt_reading_interval_in_millis - millis_since_last_mqtt_update;
	}
	else
		return ULONG_MAX;
}


bool updates_active()
{
	if(mqttSettings.mqtt_enabled) return true;

	return false;
}

#define JSON_BUFFER_SIZE 2000

boolean send_to_mqtt()
{
    char jsonBuffer[JSON_BUFFER_SIZE];
	createSensorJson(jsonBuffer, JSON_BUFFER_SIZE);
	boolean result = publishBufferToMQTT(jsonBuffer);
    return result;
}

void sendReadings()
{
	unsigned long time_in_millis = millis();

	if (mqttSettings.mqtt_enabled)
	{
		unsigned long millis_since_last_mqtt_update = ulongDiff(time_in_millis, milliseconds_at_last_mqtt_update);


		if (millis_since_last_mqtt_update > mqtt_reading_interval_in_millis)
		{
			milliseconds_at_last_mqtt_update = time_in_millis;

			if (send_to_mqtt())
			{
				mqtt_reading_interval_in_millis = mqttSettings.mqttSecsPerUpdate * 1000;
			}
			else
			{
				mqtt_reading_interval_in_millis = mqttSettings.seconds_per_mqtt_retry * 1000;
			}
		}
	}

	if (mqttForceSend)
	{
		if (send_to_mqtt())
		{
			mqttForceSend = false;
		}
	}
}


void turn_sensor_on()
{
	if (timing_state != sensorWarmingUp)
	{
		if (timingSettings.powerControlFitted)
		{
			turn_sensor_power_on();
		}
		else {
			Serial.println("Turning sensor on");
			set_sensor_working(true);
		}
		timing_state = sensorWarmingUp;
	}
}

void turn_sensor_off()
{
	if (timing_state != sensorOff)
	{
		if (timingSettings.powerControlFitted)
		{
			// if we have power contrl just turn the power off
			turn_sensor_power_off();
		}
		else {
			// otherwise send a command to the sensor to turn it off
			// Note that not all sensors can do this
			set_sensor_working(false);
		}
		timing_state = sensorOff;
	}
}

unsigned long readingFetchStartMillis;

void start_getting_readings()
{
	Serial.println("Starting to fetch readings");
    startSensorsReading();
	readingFetchStartMillis = millis();
	timing_state = sensorGettingReading;
}

bool can_power_off_sensor()
{
	// can never power off the sensor if the display is on

	if (timingSettings.logging != loggingOff)
		return false;

	long milliseconds_for_sensor_warmup = airqualitySettings.airqSecnondsSensorWarmupTime * 1000;
	long milliseconds_for_averaging = (airqualitySettings.airqNoOfAverages / 2) * 1000;

	if (time_to_next_update() > (milliseconds_for_sensor_warmup + milliseconds_for_averaging)) {
		return true;
	}
	else
		return false;
}

bool can_start_reading()
{
	// take two readings a second
	long milliseconds_for_averaging = (airqualitySettings.airqNoOfAverages / 2) * 1000;

	if (time_to_next_update() > milliseconds_for_averaging) {
		return false;
	}
	else
		return true;
}

void check_sensor_power()
{
	if (can_power_off_sensor())
	{
		turn_sensor_off();
		if(timingSettings.sleepProcessor)
		{
			long milliseconds_for_sensor_warmup = airqualitySettings.airqSecnondsSensorWarmupTime * 1000;
			long milliseconds_for_averaging = (airqualitySettings.airqNoOfAverages / 2) * 1000;
			unsigned sleep_time = time_to_next_update() - milliseconds_for_sensor_warmup - milliseconds_for_averaging;
			sleep_sensor(sleep_time);
		}
	}
	else
	{
		turn_sensor_on();
	}
}

void timingSensorOff()
{
	check_sensor_power();
}

void timingSensorWarmingUp()
{
	if (can_start_reading())
		start_getting_readings();
}


void timingSensorGettingReading()
{
	unsigned long time_in_millis = millis();

	unsigned long millis = ulongDiff(time_in_millis, readingFetchStartMillis);

	struct bme280Reading * bme280activeReading =
		(bme280Reading *) bme280Sensor.activeReading;

	struct airqualityReading * airqualityActiveReading =
		(airqualityReading *)airqSensor.activeReading;

	if (airqSensor.lastTransmittedReadingNumber != airqualityActiveReading->airNoOfAveragesCalculated && 
        bme280Sensor.lastTransmittedReadingNumber != bme280activeReading->envNoOfAveragesCalculated) 
	{
        // got readings from both sensors
		Serial.println("Sending values");
		sendReadings();
        airqSensor.lastTransmittedReadingNumber = airqualityActiveReading->airNoOfAveragesCalculated;
        bme280Sensor.lastTransmittedReadingNumber = bme280activeReading->envNoOfAveragesCalculated;
		check_sensor_power();
	}
}

void start_sensor()
{
	mqttForceSend=false;
	turn_sensor_power_on();
}

unsigned long dump_air_values_reading_count = 0;
unsigned long dump_bme_values_reading_count = 0;

void print_dump_values()
{
	char number_buffer[100];

    airqualityReading *airq = (airqualityReading *)airqSensor.activeReading;
    bme280Reading *env = (bme280Reading *)bme280Sensor.activeReading;

	switch(timingSettings.logging)
	{
		case loggingOff:
		return;

		case loggingParticles:
		sprintf(number_buffer, "%.1f,%.1f", airq->pm10, airq->pm25);
		break;

		case loggingTemp:
		sprintf(number_buffer, "%.1f",  env->temperature);
		break;

		case loggingPressure:
		sprintf(number_buffer, "%.1f",env->pressure);
		break;

		case loggingHumidity:
		sprintf(number_buffer, "%.1f",env->humidity);
		break;

		case loggingAll:
		sprintf(number_buffer, "%.1f,%.1f,%.1f,%.1f,%.1f", 
            airq->pm10, airq->pm25, env->temperature, env->pressure, env->humidity);
		break;
	}

	Serial.println(number_buffer);
}

void updateSerialDump()
{
	if(timingSettings.logging == loggingOff) return;

	if((dump_air_values_reading_count != airqSensor.readingNumber) &&
		(dump_bme_values_reading_count != bme280Sensor.readingNumber)) 
	{
		print_dump_values();		
		dump_air_values_reading_count = airqSensor.readingNumber;
		dump_bme_values_reading_count = bme280Sensor.readingNumber;
	}
}

#define uS_TO_S_FACTOR 1000000  /* Conversion factor for micro seconds to seconds */
#define TIME_TO_SLEEP  10       /* Time ESP32 will go to sleep (in seconds) */

void sleep_sensor(unsigned long time_to_sleep)
{

	Serial.println("Going to sleep...");

	stopProcesses();

	// give things a chance to settle down

	statusLedOn();
	delay(500);
	statusLedOff();
			
	esp_sleep_enable_timer_wakeup(time_to_sleep * uS_TO_S_FACTOR);

	// need to fix this so that other output pins work correctly

 	gpio_hold_en(GPIO_NUM_27);
  	gpio_deep_sleep_hold_en();

	esp_deep_sleep_start();
}


void turn_sensor_power_off()
{
	Serial.println("Turning power off");
	if(timingSettings.powerControlFitted)
	{
		pinMode(timingSettings.powerControlPin,OUTPUT);
		digitalWrite(timingSettings.powerControlPin,LOW);
	}
}

void turn_sensor_power_on()
{
	Serial.println("Turning power on");
	if(timingSettings.powerControlFitted)
	{
		pinMode(timingSettings.powerControlPin,OUTPUT);
		digitalWrite(timingSettings.powerControlPin,HIGH);
	}
}

int startTiming(struct process * timingProcess)
{
	unsigned long time_in_millis = millis();

	mqtt_reading_interval_in_millis = mqttSettings.mqttSecsPerUpdate * 1000;
	milliseconds_at_last_mqtt_update = time_in_millis - mqtt_reading_interval_in_millis;
	timing_state = sensorOff;
	start_sensor();
	return PROCESS_OK;
}

int updateTiming(struct process * timingProcess)
{
	updateSerialDump();

	switch (timing_state)
	{
	case sensorOff:
		timingSensorOff();
		break;

	case sensorWarmingUp:
		timingSensorWarmingUp();
		break;

	case sensorGettingReading:
		timingSensorGettingReading();
		break;
	}

	if (mqttForceSend)
	{
		long milliseconds_for_sensor_warmup = airqualitySettings.airqSecnondsSensorWarmupTime * 1000;
		unsigned long time_in_millis = millis();
		milliseconds_at_last_mqtt_update = time_in_millis -
			((mqttSettings.mqttSecsPerUpdate * 1000) - (airqualitySettings.airqSecnondsSensorWarmupTime * 1000));
		mqttForceSend = false;
	}
	return PROCESS_OK;
}

int stopTiming(struct process * timingProcess)
{
	return PROCESS_OK;
}

void timingStatusMessage(struct process * timingProcess, char * buffer, int bufferLength)
{
	switch (timing_state)
	{
	case sensorOff:
		snprintf(buffer, bufferLength, "Sensor off");
		break;

	case sensorWarmingUp:
		snprintf(buffer, bufferLength, "Sensor warming up");
		break;

	case sensorGettingReading:
		snprintf(buffer, bufferLength, "Sensor getting readings");
		break;
	}
}
