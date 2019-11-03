#pragma once

enum timingStates { sensorWarmingUp, sensorGettingReading, sensorOff } ;

enum Logging_State {
	loggingOff=0,
	loggingParticles=1,
	loggingTemp=2,
	loggingPressure=3,
	loggingHumidity=4,
	loggingAll=5
};

extern String loggingStateNames [];

bool mqtt_interval_has_expired();
bool lora_interval_has_expired();
unsigned long time_to_next_mqtt_update();
unsigned long time_to_next_lora_update();
bool updates_active();
unsigned long time_to_next_update();
void readings_ready();
void turn_sensor_on();
void turn_sensor_off();
void start_getting_readings();
bool can_power_off_sensor();
bool can_start_reading();
void check_sensor_power();
void start_sensor();
void setup_timing();
void loop_timing();
void force_mqtt_send ();


timingStates getTimingState();