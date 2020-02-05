#include <Arduino.h>

#include "utils.h"
#include "powercontrol.h"

#include "settings.h"

#include "soc/soc.h"
#include "soc/rtc_cntl_reg.h"

void setPowerControlPinActive(PowerControlPinDescription* descr)
{
	if (!descr->outputEnabled)
		return;

	if (descr->active)
		return;

	descr->active = true;

	if (descr->useBrownoutProtection)
	{
		WRITE_PERI_REG(RTC_CNTL_BROWN_OUT_REG, 0); //disable brownout detector   
	}

	if (descr->activeHigh)
	{
		digitalWrite(descr->pinNo, HIGH);
	}
	else
	{
		digitalWrite(descr->pinNo, LOW);
	}

	if (descr->useBrownoutProtection)
	{
		delay(500); // give devices time to stablilise
		WRITE_PERI_REG(RTC_CNTL_BROWN_OUT_REG, 1); //enable brownout detector   
	}
}

void setPowerControlPinInactive(PowerControlPinDescription* descr)
{
	if (!descr->outputEnabled)
		return;

	if (!descr->active)
		return;

	descr->active = false;

	if (descr->activeHigh)
	{
		digitalWrite(descr->pinNo, LOW);
	}
	else
	{
		digitalWrite(descr->pinNo, HIGH);
	}
}

void setPowerControlPinActivity(PowerControlPinDescription* descr, bool value)
{
	if (value)
	{
		setPowerControlPinActive(descr);
	}
	else
	{
		setPowerControlPinInactive(descr);
	}
}

void disablePowerControlPin(PowerControlPinDescription* descr)
{
	if (!descr->outputEnabled)
		return;

	// set it to an input to tri-state it
	pinMode(descr->pinNo, INPUT);
}

void setupPowerControlPin(PowerControlPinDescription* descr, boolean initialSetting, int pinNo, boolean activeHigh, boolean outputEnabled, boolean useBrownoutProtection)
{
	descr->pinNo = pinNo;
	descr->activeHigh = activeHigh;
	descr->outputEnabled = outputEnabled;
	descr->useBrownoutProtection = useBrownoutProtection;

	if (outputEnabled)
	{
		// set the initial state
		pinMode(pinNo, OUTPUT);
		setPowerControlPinActivity(descr, initialSetting);
	}
}

