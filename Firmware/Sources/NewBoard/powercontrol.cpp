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

void setDefaultpowerMinPowerOffIntervalSetting(void* dest)
{
	int* destInt = (int*)dest;
	*destInt = 30;
}

struct SettingItem powerControlPinSetting = {
		"Power Control Pin",
		"powercontrolpin",
		& powerControlSettings.powerControlOutputPin,
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
		"Power Control Output Fitted",
		"powerControlFitted",
		&powerControlSettings.powerControlFitted,
		ONOFF_INPUT_LENGTH,
		yesNo,
		setTrue,
		validateYesNo
};

struct SettingItem powercontrolOutputPinActiveHighSetting = {
		"Power Control Output Active High",
		"powerOutputActivehigh",
		& powerControlSettings.powerControlOutputPinActiveHigh,
		ONOFF_INPUT_LENGTH,
		yesNo,
		setTrue,
		validateYesNo
};

struct SettingItem* powerControlOutputSettingItemPointers[] =
{
	&powercontrolOutputFitted,
	&powerMinPowerOffIntervalSetting,
	&powerControlPinSetting,
	&powercontrolOutputPinActiveHighSetting
};

struct SettingItemCollection powerControlOutputSettingItems = {
	"powercontroloutput",
	"Pin assignment and active level (high or low) and for hardware configuration switch",
	powerControlOutputSettingItemPointers,
	sizeof(powerControlOutputSettingItemPointers) / sizeof(struct SettingItem*)
};


boolean powerOnValue;

boolean powerOnOutputHigh()
{
	if (powerControlSettings.powerControlOutputPinActiveHigh)
		return powerOnValue;
	else
		return !powerOnValue;
}

boolean powerOn()
{
		return powerOnValue;
}


boolean setPowerOn()
{
	if (powerOnValue) return true;

	powerOnValue = true;

	WRITE_PERI_REG(RTC_CNTL_BROWN_OUT_REG, 0); //disable brownout detector   

	if (powerControlSettings.powerControlOutputPinActiveHigh)
	{
		digitalWrite(powerControlSettings.powerControlOutputPin, HIGH);
	}
	else
	{
		digitalWrite(powerControlSettings.powerControlOutputPin, LOW);
	}

	// let things power up

	delay(500);

	WRITE_PERI_REG(RTC_CNTL_BROWN_OUT_REG, 1); //enable brownout detector   

	return true;
}

boolean setPowerOff()
{
	if (!powerOnValue) return true;

	powerOnValue = false;

	if (powerControlSettings.powerControlOutputPinActiveHigh)
	{
		digitalWrite(powerControlSettings.powerControlOutputPin, LOW);
	}
	else
	{
		digitalWrite(powerControlSettings.powerControlOutputPin, HIGH);
	}
	return true;
}

int startPowerControl(struct process* powerControlProcess)
{
	pinMode(powerControlSettings.powerControlOutputPin, OUTPUT);

	powerOnValue = false;
	setPowerOn();
	delay(100);
	return PROCESS_OK;
}

int updatePowerControl(struct process* powerControlProcess)
{
	return PROCESS_OK;
}

int stopPowerControl(struct process* powerControlProcess)
{
	return POWER_CONTROL_STOPPED;
}

void powerControlStatusMessage(struct process* inputSwitchProcess, char* buffer, int bufferLength)
{
	if (powerOnOutputHigh())
		snprintf(buffer, bufferLength, "Power control pin high");
	else
		snprintf(buffer, bufferLength, "Power contrl pin low");
}

