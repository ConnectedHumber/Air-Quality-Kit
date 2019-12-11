#include "debug.h"
#include "control.h"
#include "mqtt.h"
#include "pixels.h"
#include "inputswitch.h"
#include "sensors.h"
#include "settings.h"
#include "timing.h"
#include "lcd.h"

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

void enterWiFiConfig()
{
}


#define uS_TO_S_FACTOR 1000000  /* Conversion factor for micro seconds to seconds */
#define TIME_TO_SLEEP  10       /* Time ESP32 will go to sleep (in seconds) */

RTC_DATA_ATTR int bootCount = 0;

void turn_sensor_power_off(int time_to_sleep)
{
	Serial.println("Turning power off");

	stopProcesses();

	// give things a chance to settle down

	delay(500);

	// turn off GPIO16 so that we have power control
	digitalWrite(16, LOW);

	// lifting GPIO16 means that the output from
	// GPIO21 now affects the power - since this has been set high
	// on power up this will casue the sensor power to drop

	lcdSleep();

	esp_sleep_enable_timer_wakeup(time_to_sleep * uS_TO_S_FACTOR);

	esp_deep_sleep_start();

}

void turn_sensor_power_on()
{
	Serial.println("Turning power on");
	// turn on GPIO16 so that the display now works - 
	// because of the link through to GPIO21 this will also cause the 
	// power to the sensor to be switched on
	digitalWrite(16, HIGH);

	lcdWake();
}


