#include "debug.h"
#include "control.h"
#include "mqtt.h"
#include "pixels.h"
#include "inputswitch.h"
#include "sensors.h"
#include "settings.h"
#include "timing.h"
#include "messages.h"
#include "statusled.h"

void displayControlMessage(int messageNumber, char* messageText)
{
	Serial.printf("Message: %d %s\n", messageNumber, messageText);
}

void startDevice()
{
	Serial.begin(115200);

	delay(100);

	Serial.println("\n\n\n\n");

	setupSettings();

	// Start the core processes used by all the others

	startBaseProcesses();

	bindMessageHandler(displayControlMessage);

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
		startActiveSensorProcesses();
		startSensors();
		delay(2000); // show the status for a while
		setupWalkingColour(BLUE_PIXEL_COLOUR);
		Serial.printf("Start complete\n\nType help and press enter for help\n\n");

	}
}


