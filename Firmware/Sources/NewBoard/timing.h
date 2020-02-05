
#ifndef TIMING_H

#define TIMING_H

#define TIMING_STATUS_READING_TIMOUT_MESSAGE_NUMBER 31
#define TIMING_STATUS_READING_TIMOUT_MESSAGE_TEXT "Reading timeout"


enum timingStates { sensorWarmingUp, sensorGettingReading, sensorWaitingForPowerDown, particleSensorOff } ;

enum Logging_State {
	loggingOff=0,
	loggingParticles=1,
	loggingTemp=2,
	loggingPressure=3,
	loggingHumidity=4,
	loggingAll=5
};

extern String loggingStateNames[];


struct TimingSettings {
	boolean sleepProcessor;
	Logging_State logging;
	int readingTimoutSecs;
	int minimumPowerOffIntervalSecs;
};

extern struct TimingSettings timingSettings;

extern struct SettingItemCollection timingSettingItems;

unsigned long offsetMillis();

bool mqtt_interval_has_expired();
bool updates_active();
unsigned long time_to_next_update();
void startParticleSensorWarmingUp();
void turnParticleSensorOff();
void sleepSensorForTime(unsigned long time_to_sleep);
void startGettingSensorReadings();
bool can_power_off_sensor();
bool can_start_reading();
void forceSensorShutdown();
int startTiming(struct process * timingProcess);
int updateTiming(struct process * wifiConfigProcess);
int stopTiming(struct process * wifiConfigProcess);
void timingStatusMessage(struct process * wifiConfigProcess, char * buffer, int bufferLength);
void force_mqtt_send ();
timingStates getTimingState();

#endif