#pragma once

#include <Arduino.h>

#include "timing.h"
#include "debug.h"
#include "utils.h"
#include "settings.h"
#include "sensors.h"
#include "mqtt.h"
#include "lora.h"
#include "airquality.h"
#include "bme280.h"
#include "processes.h"
#include "control.h"
#include "statusled.h"
#include "powercontrol.h"

String loggingStateNames [] = { "off", "particles", "temp", "pressure", "humidity", "all"};

struct TimingSettings timingSettings;

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
RTC_DATA_ATTR unsigned long milliseconds_at_last_mqtt_update = 0;

unsigned long lora_reading_retry_interval_in_millis;
unsigned long lora_reading_interval_in_millis;
RTC_DATA_ATTR unsigned long milliseconds_at_last_lora_update = 0;
RTC_DATA_ATTR int bootCount = 0;

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


RTC_DATA_ATTR unsigned long millisOffset=0;

unsigned long offsetMillis()
{
	return millis() + millisOffset;
}

// return true if it is time for an update
bool mqtt_interval_has_expired()
{
	unsigned long time_in_millis = offsetMillis();
	unsigned long millis_since_last_mqtt_update = ulongDiff(time_in_millis, milliseconds_at_last_mqtt_update);


	if (millis_since_last_mqtt_update > mqtt_reading_interval_in_millis)
	{
		return true; 
	}
	return false;
}

bool lora_interval_has_expired()
{
	unsigned long time_in_millis = offsetMillis();
	unsigned long millis_since_last_lora_update = ulongDiff(time_in_millis, milliseconds_at_last_lora_update);


	if (millis_since_last_lora_update > lora_reading_interval_in_millis)
	{
		return true;
	}
	return false;
}

unsigned long time_to_next_mqtt_update()
{
	if (mqttSettings.mqtt_enabled)
	{
		unsigned long millis_since_last_mqtt_update = ulongDiff(offsetMillis(), milliseconds_at_last_mqtt_update);

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
		unsigned long millis_since_last_lora_update = ulongDiff(offsetMillis(), milliseconds_at_last_lora_update);

		if (millis_since_last_lora_update > lora_reading_interval_in_millis)
			return 0;
		else
			return lora_reading_interval_in_millis - millis_since_last_lora_update;
	}
	else
		return ULONG_MAX;
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

bool updates_active()
{
	if (loRaSettings.loraOn) return true;
	if (mqttSettings.mqtt_enabled) return true;

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

boolean send_to_lora()
{
	airqualityReading* airq = (airqualityReading*)airqSensor.activeReading;
	bme280Reading* env = (bme280Reading*)bme280Sensor.activeReading;

	return publishReadingsToLoRa(airq->pm25Average, airq->pm10Average, env->temperature, env->pressure, env->humidity);
}

void sendReadings()
{
	unsigned long time_in_millis = offsetMillis();

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

	if (loRaSettings.loraOn)
	{
		unsigned long millis_since_last_lora_update = ulongDiff(time_in_millis, milliseconds_at_last_lora_update);

		if (millis_since_last_lora_update > lora_reading_interval_in_millis)
		{
			milliseconds_at_last_lora_update = time_in_millis;
			send_to_lora();
		}
		else
		{
			Serial.printf("Got readings but not time to send yet %lu %lu\n", 
				millis_since_last_lora_update, lora_reading_interval_in_millis);
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


void startParticleSensorWarmingUp()
{
	if (timing_state != sensorWarmingUp)
	{
		if (powerControlSettings.powerControlFitted)
		{
			setPowerOn();
		}
		else {
			Serial.println("Turning sensor on");
			setParticleSensorWorking(true);
		}
		timing_state = sensorWarmingUp;
	}
}

void turnParticleSensorOff()
{
	if (timing_state != particleSensorOff)
	{
		if (powerControlSettings.powerControlFitted)
		{
			// if we have power control just turn the power off
			setPowerOff();
		}
		else {
			// otherwise send a command to the sensor to turn it off
			// Note that not all sensors can do this
			setParticleSensorWorking(false);
		}
		timing_state = particleSensorOff;
	}
}

unsigned long readingFetchStartMillis;

void startGettingSensorReadings()
{
	Serial.println("Starting to fetch readings");
    startSensorsReading();
	readingFetchStartMillis = offsetMillis();
	timing_state = sensorGettingReading;
}

bool can_power_off_sensor()
{
	// can never power off the sensor if the display is on

	if (timingSettings.logging != loggingOff)
		return false;

	// can never power off it we are waiting for a LoRa message to transfer

	if (loRaActive())
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


void checkSensorPowerOff()
{
	unsigned long minOffTimeInMillis = powerControlSettings.minimumPowerOffIntervalSecs * 1000;

	if (time_to_next_update() > minOffTimeInMillis)
	{
		turnParticleSensorOff();

		if (timingSettings.sleepProcessor)
		{
			long milliseconds_for_sensor_warmup = airqualitySettings.airqSecnondsSensorWarmupTime * 1000;
			long milliseconds_for_averaging = (airqualitySettings.airqNoOfAverages / 2) * 1000;
			unsigned sleepMillis = time_to_next_update() - (milliseconds_for_sensor_warmup + milliseconds_for_averaging);

			// when we wake up we need to have the clock set to the current millis
			// plus the length of the sleep. This should then trigger the next send

			millisOffset = offsetMillis() + sleepMillis;

			sleepSensorForTime(sleepMillis);
		}
	}
}

void timingSensorOff()
{
	// performed if the processor is running and we have turned
	// off the power to the sensor only

	if (can_power_off_sensor())
	{
		// check in case we can now turn everything off
		checkSensorPowerOff();
	}
	else
	{
		// need to start the sensor warming up
		startParticleSensorWarmingUp();
	}
}

void timingSensorWarmingUp()
{
	if (can_start_reading())
		startGettingSensorReadings();
}


void timingSensorGettingReading()
{
	unsigned long time_in_millis = offsetMillis();

	unsigned long millis = ulongDiff(time_in_millis, readingFetchStartMillis);

	struct bme280Reading * bme280activeReading =
		(bme280Reading *) bme280Sensor.activeReading;

	struct airqualityReading * airqualityActiveReading =
		(airqualityReading *)airqSensor.activeReading;

	if (airqSensor.lastTransmittedReadingNumber != airqualityActiveReading->airNoOfAveragesCalculated && 
        bme280Sensor.lastTransmittedReadingNumber != bme280activeReading->envNoOfAveragesCalculated) 
	{
        // got readings from both sensors

		sendReadings();

		airqSensor.lastTransmittedReadingNumber = airqualityActiveReading->airNoOfAveragesCalculated;
		bme280Sensor.lastTransmittedReadingNumber = bme280activeReading->envNoOfAveragesCalculated;

		unsigned long minOffTimeInMillis = powerControlSettings.minimumPowerOffIntervalSecs * 1000;

		if (time_to_next_update() > minOffTimeInMillis)
		{
			timing_state = sensorWaitingForPowerDown;
		}
	}
}

void timingSensorWaitingForPowerDown()
{
	if (can_power_off_sensor())
	{
		// only get here once LoRa has finished transmitting
		checkSensorPowerOff();
	}
}

bool readingSendComplete()
{
	return true;
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

#define uS_TO_mS_FACTOR 1000  /* Conversion factor for micro seconds to miliseconds */
#define TIME_TO_SLEEP  10       /* Time ESP32 will go to sleep (in seconds) */


void sleepSensorForTime(unsigned long sleepMillis)
{

	Serial.printf("Going to sleep for %lu milliseconds\n", sleepMillis);

	// stop all the processes
	stopProcesses();

	// turn off the BME280

	pinMode(26, OUTPUT);
	digitalWrite(26, LOW);

	esp_sleep_enable_timer_wakeup(sleepMillis * uS_TO_mS_FACTOR);

	esp_deep_sleep_start();
}



// forces a sensor shutdown
// this is called because we can't continue
// we are going to wake up the sensor to do the next update
// don't want to retry too frequently as this would kill the battery
// I'd rather miss a bunch of readigns while something is broken than 
// disable the sensor by killing the battery

void forceSensorShutdown()
{
	unsigned long millisToNextUdate;

	if (mqtt_reading_interval_in_millis < lora_reading_interval_in_millis)
		millisToNextUdate = mqtt_reading_interval_in_millis;
	else
		millisToNextUdate = lora_reading_interval_in_millis;

	// force a clean boot next time so that we start the timings from then

	bootCount = 0;

	turnParticleSensorOff();

	sleepSensorForTime(millisToNextUdate);
}


// Runs when the device is first powered up but not at successive boots
void powerUpTimingStart()
{
	Serial.printf("Power on restart");

	unsigned long time_in_millis = offsetMillis();

	milliseconds_at_last_mqtt_update = time_in_millis - mqtt_reading_interval_in_millis;
	milliseconds_at_last_lora_update = time_in_millis - lora_reading_interval_in_millis;
}

void flashBootReasonOnBuiltInLed()
{
	esp_reset_reason_t reset_reason = esp_reset_reason();
	flashStatusLed(reset_reason);

	delay(2000);

	esp_sleep_wakeup_cause_t wakeup_reason;
	wakeup_reason = esp_sleep_get_wakeup_cause();

	flashStatusLed(wakeup_reason);
}

void printBootReason()
{
	esp_reset_reason_t reset_reason = esp_reset_reason();

	switch (reset_reason)
	{
	case ESP_RST_UNKNOWN:    Serial.println("Reset reason can not be determined"); break;
	case ESP_RST_POWERON:    Serial.println("Reset due to power-on event"); break;
	case ESP_RST_EXT:        Serial.println("Reset by external pin (not applicable for ESP32)"); break;
	case ESP_RST_SW:         Serial.println("Software reset via esp_restart"); break;
	case ESP_RST_PANIC:      Serial.println("Software reset due to exception/panic"); break;
	case ESP_RST_INT_WDT:    Serial.println("Reset (software or hardware) due to interrupt watchdog"); break;
	case ESP_RST_TASK_WDT:   Serial.println("Reset due to task watchdog"); break;
	case ESP_RST_WDT:        Serial.println("Reset due to other watchdogs"); break;
	case ESP_RST_DEEPSLEEP:  Serial.println("Reset after exiting deep sleep mode"); break;
	case ESP_RST_BROWNOUT:   Serial.println("Brownout reset (software or hardware)"); break;
	case ESP_RST_SDIO:       Serial.println("Reset over SDIO"); break;
	}

	if (reset_reason == ESP_RST_DEEPSLEEP)
	{
		esp_sleep_wakeup_cause_t wakeup_reason = esp_sleep_get_wakeup_cause();

		switch(wakeup_reason)
		{ 
			case ESP_SLEEP_WAKEUP_UNDEFINED:    Serial.println("In case of deep sleep: reset was not caused by exit from deep sleep"); break;
			case ESP_SLEEP_WAKEUP_ALL:          Serial.println("Not a wakeup cause: used to disable all wakeup sources with esp_sleep_disable_wakeup_source"); break;
			case ESP_SLEEP_WAKEUP_EXT0:         Serial.println("Wakeup caused by external signal using RTC_IO"); break;
			case ESP_SLEEP_WAKEUP_EXT1:         Serial.println("Wakeup caused by external signal using RTC_CNTL"); break;
			case ESP_SLEEP_WAKEUP_TIMER:        Serial.println("Wakeup caused by timer"); break;
			case ESP_SLEEP_WAKEUP_TOUCHPAD:     Serial.println("Wakeup caused by touchpad"); break;
			case ESP_SLEEP_WAKEUP_ULP:          Serial.println("Wakeup caused by ULP program"); break;
			case ESP_SLEEP_WAKEUP_GPIO:         Serial.println("Wakeup caused by GPIO (light sleep only)"); break;
			case ESP_SLEEP_WAKEUP_UART:         Serial.println("Wakeup caused by UART (light sleep only)"); break;
		}
	}
}

int startTiming(struct process* timingProcess)
{
	//flashBootReasonOnBuiltInLed();
	
	printBootReason();

	Serial.printf("Millis offset: %lu Boot Count: %d", millisOffset, bootCount);

	mqtt_reading_interval_in_millis = mqttSettings.mqttSecsPerUpdate * 1000;
	lora_reading_interval_in_millis = loRaSettings.seconds_per_lora_update * 1000;

	if (bootCount == 0)
	{
		powerUpTimingStart();
	}

	bootCount++;

	timing_state = particleSensorOff;

	mqttForceSend = false;
	loraForceSend = false;

	return PROCESS_OK;
}

int updateTiming(struct process* timingProcess)
{
	updateSerialDump();

	switch (timing_state)
	{
	case particleSensorOff:
		timingSensorOff();
		break;

	case sensorWarmingUp:
		timingSensorWarmingUp();
		break;

	case sensorGettingReading:
		timingSensorGettingReading();
		break;

	case sensorWaitingForPowerDown:
		timingSensorWaitingForPowerDown();
		break;
	}

	if (loraForceSend)
	{
		unsigned long time_in_millis = offsetMillis();
		milliseconds_at_last_lora_update = time_in_millis -
			((loRaSettings.seconds_per_lora_update * 1000) - (airqualitySettings.airqSecnondsSensorWarmupTime * 1000));
		loraForceSend = false;
	}

	if (mqttForceSend)
	{
		long milliseconds_for_sensor_warmup = airqualitySettings.airqSecnondsSensorWarmupTime * 1000;
		unsigned long time_in_millis = offsetMillis();
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
	unsigned long mqtt = time_to_next_mqtt_update();
	unsigned long lora = time_to_next_lora_update();


	switch (timing_state)
	{
	case particleSensorOff:
		snprintf(buffer, bufferLength, "Sensor off lora: %lu mqtt: %lu", lora, mqtt);
		break;

	case sensorWarmingUp:
		snprintf(buffer, bufferLength, "Sensor warming up lora: %lu mqtt: %lu", lora, mqtt);
		break;

	case sensorGettingReading:
		snprintf(buffer, bufferLength, "Sensor getting readings lora: %lu mqtt: %lu", lora, mqtt);
		break;

	case sensorWaitingForPowerDown:
		snprintf(buffer, bufferLength, "Sensor waiting for powerdown lora: %lu mqtt: %lu", lora, mqtt);
		break;

	}
}
