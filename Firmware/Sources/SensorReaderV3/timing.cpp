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
#include "menu.h"
#include "lora.h"

String loggingStateNames [] = { "off", "particles", "temp", "pressure", "humidity", "all"};

unsigned long mqtt_reading_retry_interval_in_millis;
unsigned long mqtt_reading_interval_in_millis;
unsigned long milliseconds_at_last_mqtt_update;

unsigned long lora_reading_retry_interval_in_millis;
unsigned long lora_reading_interval_in_millis;
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

boolean loraForceSend;

void force_lora_send()
{
    loraForceSend = true;
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

bool lora_interval_has_expired()
{
	unsigned long time_in_millis = millis();
	unsigned long millis_since_last_lora_update = ulongDiff(time_in_millis, milliseconds_at_last_lora_update);


	if (millis_since_last_lora_update > lora_reading_interval_in_millis)
	{
		return true;  
	}
	return false;
}

unsigned long time_to_next_mqtt_update()
{
	if (settings.mqtt_enabled)
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

unsigned long time_to_next_lora_update()
{
	if (loRaSettings.loraOn)
	{
		unsigned long millis_since_last_lora_update = ulongDiff(millis(), milliseconds_at_last_lora_update);

		if (millis_since_last_lora_update > lora_reading_interval_in_millis)
			return 0;
		else
			return lora_reading_interval_in_millis - millis_since_last_lora_update;
	}
	else
		return ULONG_MAX;
}

bool updates_active()
{
	if(loRaSettings.loraOn) return true;
	if(settings.mqtt_enabled) return true;

	return false;
}

unsigned long time_to_next_update()
{
	unsigned long mqtt = time_to_next_mqtt_update();
	unsigned long lora = time_to_next_lora_update();

	if (mqtt < lora)
		return mqtt;
	else
		return lora;
}

#define JSON_BUFFER_SIZE 2000

boolean send_to_mqtt()
{
    char jsonBuffer[JSON_BUFFER_SIZE];
	createSensorJson(jsonBuffer, JSON_BUFFER_SIZE);
	boolean result = publishBufferToMQTT(jsonBuffer);
	if(result)
	{
		startPopUpMessage("MQTT", "Message sent", POPUP_DURATION_MILLIS);
	}
    return result;
}

boolean send_to_lora()
{
    airqualityReading *airq = (airqualityReading *)airqSensor.activeReading;
    bme280Reading *env = (bme280Reading *)bme280Sensor.activeReading;

	return publishReadingsToLoRa ( airq->pm25Average, airq->pm10Average, env->temperature, env->pressure, env->humidity);
}

void sendReadings()
{
	unsigned long time_in_millis = millis();

	if (settings.mqtt_enabled)
	{
		unsigned long millis_since_last_mqtt_update = ulongDiff(time_in_millis, milliseconds_at_last_mqtt_update);


		if (millis_since_last_mqtt_update > mqtt_reading_interval_in_millis)
		{
			milliseconds_at_last_mqtt_update = time_in_millis;

			if (send_to_mqtt())
			{
				mqtt_reading_interval_in_millis = settings.mqttSecsPerUpdate * 1000;
			}
			else
			{
				mqtt_reading_interval_in_millis = settings.seconds_per_mqtt_retry * 1000;
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

	if (loRaSettings.loraOn)
	{
		unsigned long millis_since_last_lora_update = ulongDiff(time_in_millis, milliseconds_at_last_lora_update);

		if (millis_since_last_lora_update > lora_reading_interval_in_millis)
		{
			milliseconds_at_last_lora_update = time_in_millis ;
			send_to_lora();
		}
	}

	if (loraForceSend)
	{
		if (send_to_lora())
		{
			loraForceSend = false;
		}
	}
}

void turn_sensor_on()
{
	if (timing_state != sensorWarmingUp)
	{
		Serial.println("Turning sensor on");
		set_sensor_working(true);
		timing_state = sensorWarmingUp;
	}
}

void turn_sensor_off()
{
	if (timing_state != sensorOff)
	{
		Serial.println("Turning sensor off");
		set_sensor_working(false);
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

	if (settings.logging != loggingOff)
		return false;

	long milliseconds_for_sensor_warmup = settings.airqSecnondsSensorWarmupTime * 1000;
	long milliseconds_for_averaging = (settings.airqNoOfAverages / 2) * 1000;

	if (time_to_next_update() > (milliseconds_for_sensor_warmup + milliseconds_for_averaging)) {
		return true;
	}
	else
		return false;
}

bool can_start_reading()
{
	// take two readings a second
	long milliseconds_for_averaging = (settings.airqNoOfAverages / 2) * 1000;

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
	loraForceSend=false;
	set_sensor_working(true);
}

unsigned long dump_air_values_reading_count = 0;
unsigned long dump_bme_values_reading_count = 0;

void print_dump_values()
{
	char number_buffer[100];

    airqualityReading *airq = (airqualityReading *)airqSensor.activeReading;
    bme280Reading *env = (bme280Reading *)bme280Sensor.activeReading;

	switch(settings.logging)
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
	if(settings.logging == loggingOff) return;

	if((dump_air_values_reading_count != airqSensor.readingNumber) &&
		(dump_bme_values_reading_count != bme280Sensor.readingNumber)) 
	{
		print_dump_values();		
		dump_air_values_reading_count = airqSensor.readingNumber;
		dump_bme_values_reading_count = bme280Sensor.readingNumber;
	}
}


int startTiming(struct process * timingProcess)
{
	unsigned long time_in_millis = millis();

	mqtt_reading_interval_in_millis = settings.mqttSecsPerUpdate * 1000;
	milliseconds_at_last_mqtt_update = time_in_millis - mqtt_reading_interval_in_millis;
	lora_reading_interval_in_millis = loRaSettings.seconds_per_lora_update * 1000;
	milliseconds_at_last_lora_update = time_in_millis - lora_reading_interval_in_millis;
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

	if (loraForceSend)
	{
		unsigned long time_in_millis = millis();
		milliseconds_at_last_lora_update = time_in_millis - 
			((loRaSettings.seconds_per_lora_update * 1000) - (settings.airqSecnondsSensorWarmupTime * 1000));
		loraForceSend = false;
	}

	if (mqttForceSend)
	{
		long milliseconds_for_sensor_warmup = settings.airqSecnondsSensorWarmupTime * 1000;
		unsigned long time_in_millis = millis();
		milliseconds_at_last_mqtt_update = time_in_millis -
			((settings.mqttSecsPerUpdate * 1000) - (settings.airqSecnondsSensorWarmupTime * 1000));
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

