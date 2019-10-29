#pragma once

#include "utils.h"
#include "commands.h"

unsigned long mqtt_reading_retry_interval_in_millis;
unsigned long mqtt_reading_interval_in_millis;
unsigned long milliseconds_at_last_mqtt_update;

unsigned long lora_reading_retry_interval_in_millis;
unsigned long lora_reading_interval_in_millis;
unsigned long milliseconds_at_last_lora_update;

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
	if (settings.mqttOn)
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
	if (settings.loraOn)
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
	if(settings.loraOn) return true;
	if(settings.mqttOn) return true;

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

void readings_ready()
{
	unsigned long time_in_millis = millis();

	if (settings.mqttOn)
	{
		unsigned long millis_since_last_mqtt_update = ulongDiff(time_in_millis, milliseconds_at_last_mqtt_update);


		if (millis_since_last_mqtt_update > mqtt_reading_interval_in_millis)
		{
			milliseconds_at_last_mqtt_update = time_in_millis;

			if (send_to_mqtt())
			{
				mqtt_reading_interval_in_millis = settings.seconds_per_mqtt_update * 1000;
			}
			else
			{
				mqtt_reading_interval_in_millis = settings.seconds_per_mqtt_retry * 1000;
			}
		}
	}

	if (pub_mqtt_force_send)
	{
		if (send_to_mqtt())
		{
			pub_mqtt_force_send = false;
		}
	}

	if (settings.loraOn)
	{
		unsigned long millis_since_last_lora_update = ulongDiff(time_in_millis, milliseconds_at_last_lora_update);

		if (millis_since_last_lora_update > lora_reading_interval_in_millis)
		{
			milliseconds_at_last_lora_update = time_in_millis ;
			send_to_lora();
		}
	}

	if (pub_lora_force_send || pub_lora_force_test)
	{
		if (send_to_lora())
		{
			pub_lora_force_send = false;
			pub_lora_force_test = false;
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

#define READING_FETCH_TIMOUT_MILLIS 30000

void start_getting_readings()
{
	Serial.println("Starting to fetch readings");
	// clear the averages
	pub_pm25Average.restartAverage();
	pub_pm10Average.restartAverage();
	readingFetchStartMillis = millis();
	timing_state = sensorGettingReading;
}

bool can_power_off_sensor()
{
	// can never power off the sensor if the display is on

	if (settings.loggingActive)
		return false;

	long milliseconds_for_sensor_warmup = settings.seconds_sensor_warmup * 1000;
	long milliseconds_for_averaging = (NO_OF_AVERAGES / 2) * 1000;

	if (time_to_next_update() > (milliseconds_for_sensor_warmup + milliseconds_for_averaging)) {
		return true;
	}
	else
		return false;
}

bool can_start_reading()
{

	// take two readings a second
	long milliseconds_for_averaging = (NO_OF_AVERAGES / 2) * 1000;

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

	if (millis > READING_FETCH_TIMOUT_MILLIS)
	{
		// clear the missing data and send what we have
		if (!pub_air_values_ready)
		{
			Serial.println("Missing air quality");
			pub_avg_ppm_10 = 0;
			pub_avg_ppm_25 = 0;
		}
		if (!pub_bme_values_ready)
		{
			Serial.println("Missing BME");
			pub_temp = 0;
			pub_pressure = 0;
			pub_humidity = 0;
		}
		pub_air_values_ready = true;
		pub_bme_values_ready = true;
	}

#ifdef TEST_LORA
	if (1)
#else
	if (pub_air_values_ready && pub_bme_values_ready) 
#endif
	{
		Serial.println("Sending values");
		readings_ready();
		pub_air_values_ready = false;
		pub_bme_values_ready = false;
		check_sensor_power();
	}
}

void start_sensor()
{
	for (int i = 0; i < 5; i++)
	{
		set_sensor_working(true);
		delay(500);
	}
}

unsigned long dump_air_values_reading_count = 0;
unsigned long dump_bme_values_reading_count = 0;

void enable_serial_dump ()
{
	settings.loggingActive = true;
	save_settings();
}

void disable_serial_dump()
{
	settings.loggingActive = false;
	save_settings();
}

void do_serial_dump()
{
	if(!settings.loggingActive) return;

	char number_buffer[100];

	if((dump_air_values_reading_count != pub_air_values_reading_count) &&
		(dump_bme_values_reading_count != pub_bme_values_reading_count)) 
	{
		sprintf(number_buffer, "%.1f,%.1f,%.1f,%.1f,%.1f", pub_disp_ppm_10, pub_disp_ppm_25, pub_temp, pub_pressure, pub_humidity);
		Serial.println(number_buffer);
		dump_air_values_reading_count = pub_air_values_reading_count;
		dump_bme_values_reading_count = pub_bme_values_reading_count;
	}
}

void setup_timing()
{
	unsigned long time_in_millis = millis();
	mqtt_reading_interval_in_millis = settings.seconds_per_mqtt_update * 1000;
	milliseconds_at_last_mqtt_update = time_in_millis - mqtt_reading_interval_in_millis;
	lora_reading_interval_in_millis = settings.seconds_per_lora_update * 1000;
	milliseconds_at_last_lora_update = time_in_millis - lora_reading_interval_in_millis;
}

void loop_timing()
{

	do_serial_dump();

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

	if (pub_lora_force_send)
	{
		unsigned long time_in_millis = millis();
		milliseconds_at_last_lora_update = time_in_millis - 
			((settings.seconds_per_lora_update * 1000) - (settings.seconds_sensor_warmup * 1000));
		pub_lora_force_send = false;
	}

	if (pub_mqtt_force_send)
	{
		long milliseconds_for_sensor_warmup = settings.seconds_sensor_warmup * 1000;
		unsigned long time_in_millis = millis();
		milliseconds_at_last_mqtt_update = time_in_millis -
			((settings.seconds_per_mqtt_update * 1000) - (settings.seconds_sensor_warmup * 1000));
		pub_mqtt_force_send = false;
	}

	if (pub_lora_force_test)
	{
		if (send_to_lora())
		{
			pub_lora_force_test = false;
		}
	}
}
