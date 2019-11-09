#line 1 "c:\\Users\\Rob\\Desktop\\GItHub\\Air-Quality-Kit\\Firmware\\Sources\\SensorReaderV3\\SensorReaderV3.ino"
#line 1 "c:\\Users\\Rob\\Desktop\\GItHub\\Air-Quality-Kit\\Firmware\\Sources\\SensorReaderV3\\SensorReaderV3.ino"
#include <Arduino.h>

#include <WiFi.h>
#include <WiFiClient.h>
#include <WebServer.h>
#include <ESPmDNS.h>

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

#line 37 "c:\\Users\\Rob\\Desktop\\GItHub\\Air-Quality-Kit\\Firmware\\Sources\\SensorReaderV3\\SensorReaderV3.ino"
void setup();
#line 49 "c:\\Users\\Rob\\Desktop\\GItHub\\Air-Quality-Kit\\Firmware\\Sources\\SensorReaderV3\\SensorReaderV3.ino"
void loop();
#line 37 "c:\\Users\\Rob\\Desktop\\GItHub\\Air-Quality-Kit\\Firmware\\Sources\\SensorReaderV3\\SensorReaderV3.ino"
void setup() {
	Serial.begin(115200);
	delay(500);
	setupSettings();
	Serial.printf("\nConnected Humber Sensor %s\nVersion %d.%d\n\n", settings.deviceName, 
		MAJOR_VERSION, MINOR_VERSION);

	startDevice();

	Serial.printf("\n\nType help and press enter for help\n\n");
}

void loop() 
{
	updateProcesses();
	updateSensors();
	delay(1);
}

