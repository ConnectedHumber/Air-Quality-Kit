#include <Arduino.h>

#include "utils.h"
#include "inputswitch.h"
#include "settings.h"

struct InputSwitchSettings inputSwitchSettings;

void setDefaultControlInputPin(void* dest)
{
	int* destInt = (int*)dest;
	*destInt = 36;
}

struct SettingItem controlInputPinSetting = {
		"Control Input Pin",
		"controlinputpin",
		& inputSwitchSettings.controlInputPin,
		NUMBER_INPUT_LENGTH,
		integerValue,
		setDefaultControlInputPin,
		validateInt };

struct SettingItem controlInputPinActiveLowSetting = {
		"Control Input Active Low",
		"controlinputlow",
		& inputSwitchSettings.controlInputPinActiveLow,
		ONOFF_INPUT_LENGTH,
		yesNo,
		setFalse,
		validateYesNo
};


struct SettingItem* inputSwitchSettingItemPointers[] =
{
&controlInputPinSetting,
&controlInputPinActiveLowSetting
};

struct SettingItemCollection inputSwitchSettingItems = {
	"inputswitch",
	"Pin assignment, level (high or low) and enable/disable for hardware configuration switch",
	inputSwitchSettingItemPointers,
	sizeof(inputSwitchSettingItemPointers) / sizeof(struct SettingItem*)
};




int lastInputValue;
long inputDebounceStartTime;

boolean switchValue;

boolean getInputSwitchValue()
{
	return switchValue;
}

unsigned long millisAtLastInputChange;

// number of milliseconds that the signal must be stable
// before we read it

#define INPUT_DEBOUNCE_TIME 10


int startInputSwitch(struct process * inputSwitchProcess)
{
	pinMode(inputSwitchSettings.controlInputPin, INPUT_PULLUP);
	return PROCESS_OK;
}

int updateInputSwitch(struct process * inputSwitchProcess)
{
	int newInputValue = digitalRead(inputSwitchSettings.controlInputPin);

	if (newInputValue == lastInputValue)
	{
		inputDebounceStartTime = millis();
	}
	else
	{
		long currentMillis = millis();
		long millisSinceChange = ulongDiff(currentMillis, inputDebounceStartTime);

		if (++millisSinceChange > INPUT_DEBOUNCE_TIME)
		{
			if (newInputValue)
				switchValue = !inputSwitchSettings.controlInputPinActiveLow;
			else
				switchValue = inputSwitchSettings.controlInputPinActiveLow;
			lastInputValue = newInputValue;
		}
	}
	return PROCESS_OK;
}

int stopInputSwitch(struct process * inputSwitchProcess)
{
	return INPUT_SWITCH_STOPPED;
}

void inputSwitchStatusMessage(struct process * inputSwitchProcess, char * buffer, int bufferLength)
{
	if (switchValue)
		snprintf(buffer, bufferLength, "Input switch pressed");
	else
		snprintf(buffer, bufferLength, "Input switch released");
}


boolean readInputSwitch()
{
	int newInputValue;
	int lastInputValue = digitalRead(inputSwitchSettings.controlInputPin);
	long inputDebounceStartTime = millis();

	while (true)
	{
		int newInputValue = digitalRead(inputSwitchSettings.controlInputPin);

		if (newInputValue != lastInputValue)
		{
			inputDebounceStartTime = millis();
			lastInputValue = newInputValue;
		}
		else
		{
			long currentMillis = millis();
			long millisSinceChange = ulongDiff(currentMillis, inputDebounceStartTime);

			if (++millisSinceChange > INPUT_DEBOUNCE_TIME)
			{
				if (newInputValue)
					return !inputSwitchSettings.controlInputPinActiveLow;
				else
					return inputSwitchSettings.controlInputPinActiveLow;
			}
		}
	}
}
