#ifndef POWERCONTROL_H

#define POWERCONTROL_H

struct PowerControlPinDescription {
	bool active;
	int pinNo;
	bool activeHigh;
	bool outputEnabled;
	bool useBrownoutProtection;
};

void setupPowerControlPin(PowerControlPinDescription* descr, boolean initialSetting, 
	int pinNo, boolean activeHigh, boolean outputEnabled, boolean useBrownoutProtection);

void setPowerControlPinActive(PowerControlPinDescription* descr);
void setPowerControlPinInactive(PowerControlPinDescription* descr);
void setPowerControlPinActivity(PowerControlPinDescription* descr, bool value);
void disablePowerControlPin(PowerControlPinDescription* descr);

#endif