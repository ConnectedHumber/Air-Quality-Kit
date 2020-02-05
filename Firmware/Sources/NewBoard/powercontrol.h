#ifndef POWERCONTROL_H

#define POWERCONTROL_H

#include "processes.h"

#define POWER_CONTROL_STOPPED 1

boolean particlePowerOn();
boolean particlePowerOutputHigh();

boolean BME280PowerOn();
boolean BMEPowerOnOutputHigh();

boolean setParticleSensorPowerOn();
boolean setParticleSensorPowerOff();
boolean setBME280SensorPowerOn();
boolean setBME280SensorPowerOff();

int startPowerControl(struct process* inputSwitchProcess);
int updatePowerControl(struct process* inputSwitchProcess);
int stopPowerControl(struct process* inputSwitchProcess);
void powerControlStatusMessage(struct process* inputSwitchProcess, char* buffer, int bufferLength);

struct PowerControlSettings {
	boolean particleSensorPowerControlFitted;
	int particleSensorPowerControlOutputPin;
	boolean particleSensorPowerControlOutputPinActiveHigh;

	boolean bme280PowerControlActive;
	int bme280PowerControlPin;
	boolean bme280PowerControlOutputPinActiveHigh;
	int minimumPowerOffIntervalSecs;
};

extern struct PowerControlSettings powerControlSettings;

extern struct SettingItemCollection powerControlOutputSettingItems;

#endif