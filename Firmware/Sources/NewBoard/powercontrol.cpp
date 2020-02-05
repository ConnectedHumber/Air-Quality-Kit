#include <Arduino.h>

#include "utils.h"
#include "powercontrol.h"

#include "settings.h"

#include "soc/soc.h"
#include "soc/rtc_cntl_reg.h"

struct PowerControlSettings powerControlSettings;

void setDefaultPowerContrlPin(void* dest)
{
	int* destInt = (int*)dest;
	*destInt = 27;
}

void setDefaultBME280PowerPin(void* dest)
{
	int* destInt = (int*)dest;
	*destInt = 26;
}

void setDefaultpowerMinPowerOffIntervalSetting(void* dest)
{
	int* destInt = (int*)dest;
	*destInt = 30;
}

struct SettingItem powerControlPinSetting = {
		"Power Control Pin",
		"powercontrolpin",
		&powerControlSettings.particleSensorPowerControlOutputPin,
		NUMBER_INPUT_LENGTH,
		integerValue,
		setDefaultPowerContrlPin,
		validateInt };

struct SettingItem powerMinPowerOffIntervalSetting = {
		"Minimum power off time in seconds",
		"powercontrolmininterval",
		&powerControlSettings.minimumPowerOffIntervalSecs,
		NUMBER_INPUT_LENGTH,
		integerValue,
		setDefaultpowerMinPowerOffIntervalSetting,
		validateInt };


struct SettingItem powercontrolOutputFitted = {
		"Particle sensor power control fitted",
		"particlePowerControlFitted",
		&powerControlSettings.particleSensorPowerControlFitted,
		ONOFF_INPUT_LENGTH,
		yesNo,
		setTrue,
		validateYesNo
};

struct SettingItem powercontrolOutputPinActiveHighSetting = {
		"Power Control Output Active High",
		"powerOutputActivehigh",
		&powerControlSettings.particleSensorPowerControlOutputPinActiveHigh,
		ONOFF_INPUT_LENGTH,
		yesNo,
		setTrue,
		validateYesNo
};

struct SettingItem bme280PowerControlSetting = {
		"BME 280 power control active (yes or no)",
		"bme280powercontrolactive",
		&powerControlSettings.bme280PowerControlActive,
		ONOFF_INPUT_LENGTH,
		yesNo,
		setFalse,
		validateYesNo };

struct SettingItem bme280PowerPinSetting = {
		"BME280 power control pin",
		"bme280powerpin",
		&powerControlSettings.bme280PowerControlPin,
		NUMBER_INPUT_LENGTH,
		integerValue,
		setDefaultBME280PowerPin,
		validateInt };

struct SettingItem bme280ControlOutputPinActiveHighSetting = {
		"BME280 Power Control Output Active High",
		"bme280powerOutputActivehigh",
		&powerControlSettings.bme280PowerControlOutputPinActiveHigh,
		ONOFF_INPUT_LENGTH,
		yesNo,
		setTrue,
		validateYesNo
};


struct SettingItem* powerControlOutputSettingItemPointers[] =
{
	&powercontrolOutputFitted,
	&powerControlPinSetting,
	&powercontrolOutputPinActiveHighSetting,
	&bme280PowerControlSetting,
	&bme280PowerPinSetting,
	&bme280ControlOutputPinActiveHighSetting,
	&powerMinPowerOffIntervalSetting
};

struct SettingItemCollection powerControlOutputSettingItems = {
	"powercontroloutput",
	"Pin assignment and active level (high or low) for hardware power switches",
	powerControlOutputSettingItemPointers,
	sizeof(powerControlOutputSettingItemPointers) / sizeof(struct SettingItem*)
};


boolean particlePowerOnValue;
boolean bmePowerOnValue;

boolean particlePowerOutputHigh()
{
	if (powerControlSettings.particleSensorPowerControlOutputPinActiveHigh)
		return particlePowerOnValue;
	else
		return !particlePowerOnValue;
}

boolean BMEPowerOnOutputHigh()
{
	if (powerControlSettings.bme280PowerControlOutputPinActiveHigh)
		return bmePowerOnValue;
	else
		return !bmePowerOnValue;
}


boolean particlePowerOn()
{
	return particlePowerOnValue;
}


boolean BME280PowerOn()
{
	return bmePowerOnValue;
}

boolean setParticleSensorPowerOn()
{
	if (particlePowerOnValue) return true;

	particlePowerOnValue = true;

	WRITE_PERI_REG(RTC_CNTL_BROWN_OUT_REG, 0); //disable brownout detector   

	if (powerControlSettings.particleSensorPowerControlOutputPinActiveHigh)
	{
		digitalWrite(powerControlSettings.particleSensorPowerControlOutputPin, HIGH);
	}
	else
	{
		digitalWrite(powerControlSettings.particleSensorPowerControlOutputPin, LOW);
	}

	// let things power up

	delay(500);

	WRITE_PERI_REG(RTC_CNTL_BROWN_OUT_REG, 1); //enable brownout detector   

	return true;
}

boolean setParticleSensorPowerOff()
{
	if (!particlePowerOnValue) return true;

	particlePowerOnValue = false;

	if (powerControlSettings.particleSensorPowerControlOutputPinActiveHigh)
	{
		digitalWrite(powerControlSettings.particleSensorPowerControlOutputPin, LOW);
	}
	else
	{
		digitalWrite(powerControlSettings.particleSensorPowerControlOutputPin, HIGH);
	}
	return true;
}

boolean setBME280SensorPowerOn()
{
	if (bmePowerOnValue) return true;

	bmePowerOnValue = true;

	if (powerControlSettings.bme280PowerControlOutputPinActiveHigh)
	{
		digitalWrite(powerControlSettings.bme280PowerControlPin, HIGH);
	}
	else
	{
		digitalWrite(powerControlSettings.bme280PowerControlPin, LOW);
	}

	return true;
}

boolean setBME280SensorPowerOff()
{
	if (!bmePowerOnValue) return true;

	bmePowerOnValue = false;

	if (powerControlSettings.bme280PowerControlOutputPinActiveHigh)
	{
		digitalWrite(powerControlSettings.bme280PowerControlPin, LOW);
	}
	else
	{
		digitalWrite(powerControlSettings.bme280PowerControlPin, HIGH);
	}

	return true;
}


int startPowerControl(struct process* powerControlProcess)
{
	if (powerControlSettings.particleSensorPowerControlFitted)
	{
		pinMode(powerControlSettings.particleSensorPowerControlOutputPin, OUTPUT);
		particlePowerOnValue = false;
		setParticleSensorPowerOn();
	}

	if (powerControlSettings.bme280PowerControlActive)
	{
		pinMode(powerControlSettings.bme280PowerControlPin, OUTPUT);
		bmePowerOnValue = false;
		setBME280SensorPowerOn();
	}

	delay(100);
	return PROCESS_OK;
}

int updatePowerControl(struct process* powerControlProcess)
{
	return PROCESS_OK;
}

int stopPowerControl(struct process* powerControlProcess)
{
	// turn everything off

	setParticleSensorPowerOff();
	setBME280SensorPowerOff();

	// set the control pin to be an input to see if this reduces power consumption
	// will be set back to an input when the device restarts

	pinMode(powerControlSettings.particleSensorPowerControlOutputPin, INPUT);
	pinMode(powerControlSettings.bme280PowerControlPin, INPUT);

	return POWER_CONTROL_STOPPED;
}

void powerControlStatusMessage(struct process* inputSwitchProcess, char* buffer, int bufferLength)
{
	snprintf(buffer, bufferLength, "Particle pin: %d BME pin: %d", 
		particlePowerOutputHigh(),BMEPowerOnOutputHigh());
}

