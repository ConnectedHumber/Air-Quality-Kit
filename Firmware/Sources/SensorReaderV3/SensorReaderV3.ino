#include <HTTP_Method.h>
#include <Arduino.h>

#include <MicroNMEA.h>
#include <DNSServer.h>
#include <ezTime.h>
#include <PubSubClient.h>
#include <Adafruit_BME280.h>
#include <Adafruit_Sensor.h>
#include <WiFiUdp.h>
#include <WiFiServer.h>
#include <WiFiClient.h>
#include <EEPROM.h>

#include "utils.h"
#include "settings.h"
#include "processes.h"
#include "sensors.h"
#include "bme280.h"
#include "airquality.h"
#include "connectwifi.h"
#include "configwifi.h"
#include "webserver.h"
#include "otaupdate.h"
#include "pixels.h"
#include "inputswitch.h"
#include "mqtt.h"
#include "clock.h"
#include "gps.h"
#include "console.h"
#include "control.h"

void setup() {
	Serial.begin(115200);
	delay(500);

	pinMode(16,OUTPUT);
	digitalWrite(16, LOW);
	delay(50);
	digitalWrite(16, HIGH);
	pinMode(16, INPUT_PULLUP);

	pinMode(21, OUTPUT);
	digitalWrite(21, HIGH);

	setupSettings();

	PrintSystemDetails();

	startDevice();

	Serial.printf("\n\nType help and press enter for help\n\n");
}

void loop() 
{
	updateProcesses();
	updateSensors();
	delay(1);
}
