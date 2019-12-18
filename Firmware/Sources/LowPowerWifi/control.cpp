#include "debug.h"
#include "control.h"
#include "mqtt.h"
#include "pixels.h"
#include "inputswitch.h"
#include "sensors.h"
#include "settings.h"
#include "timing.h"
#include "statusled.h"

void showDeviceStatus()
{
	beginStatusDisplay();
	displayProcessStatus();
	displaySensorStatus();
	renderStatusDisplay();
}

void startDevice()
{
	if (readInputSwitch())
	{
		startstatusLedFlash(500);

		Serial.println("Starting WiFi configuration");
		Serial.println("Reset the device to exit configuration mode\n\n");
		beginWifiStatusDisplay();
		startWifiConfigProcesses();
		while (1)
		{
			updateWifiConfigProcesses();
		}
	}
	else
	{
		Serial.println("Starting node operation");
		beginStatusDisplay();
		startDeviceProcesses();
		startSensors();
		delay(1000); // show the status for a second
		setupWalkingColour(GREEN_PIXEL_COLOUR);
	}
}


