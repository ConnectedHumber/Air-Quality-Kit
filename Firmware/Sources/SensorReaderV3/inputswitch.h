#ifndef INPUTSWITCH_H

#define INPUTSWITCH_H

#include "processes.h"

#define INPUT_SWITCH_STOPPED 1

boolean getInputSwitchValue();
boolean readInputSwitch();

int startInputSwitch(struct process * inputSwitchProcess);
int updateInputSwitch(struct process * inputSwitchProcess);
int stopInputSwitch(struct process * inputSwitchProcess);
void inputSwitchStatusMessage(struct process * inputSwitchProcess, char * buffer, int bufferLength);

struct InputSwitchSettings {
	int controlInputPin;
	boolean controlInputPinActiveLow;
};

extern struct InputSwitchSettings inputSwitchSettings;

extern struct SettingItemCollection inputSwitchSettingItems;

#endif