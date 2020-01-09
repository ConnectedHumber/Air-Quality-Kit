#ifndef POWERCONTROL_H

#define POWERCONTROL_H

#include "processes.h"

#define POWER_CONTROL_STOPPED 1

boolean powerOn();
boolean powerOnOutputHigh();

boolean setPowerOn();
boolean setPowerOff();

int startPowerControl(struct process* inputSwitchProcess);
int updatePowerControl(struct process* inputSwitchProcess);
int stopPowerControl(struct process* inputSwitchProcess);
void powerControlStatusMessage(struct process* inputSwitchProcess, char* buffer, int bufferLength);

struct PowerControlSettings {
	boolean powerControlFitted;
	int powerControlOutputPin;
	boolean powerControlOutputPinActiveHigh;
};

extern struct PowerControlSettings powerControlSettings;

extern struct SettingItemCollection powerControlOutputSettingItems;

#endif