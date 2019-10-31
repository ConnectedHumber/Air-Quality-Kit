#include <Arduino.h>
#line 1 "c:\\Users\\Rob\\Documents\\GitHub\\Air-Quality-Kit\\Firmware\\Sources\\SensorReader\\SensorReader.ino"
#include <MicroNMEA.h>

#include <EEPROM.h>
#include <base64.h>

#include <PubSubClient.h>
#include <WiFi.h>
#include <WiFiClientSecure.h>
#include <lmic.h>
#include "ArduinoJson-v5.13.2.h"

#include "HeltecMD_DS3231.h"
//#define DEBUG
//#define TEST_LORA

#ifdef DEBUG

#define TRACE(s) Serial.print(s)
#define TRACE_HEX(s) Serial.print(s, HEX)
#define TRACELN(s) Serial.println(s)
#define TRACE_HEXLN(s) Serial.println(s, HEX)

#else

#define TRACE(s) 
#define TRACE_HEX(s) 
#define TRACELN(s) 
#define TRACE_HEXLN(s) 

#endif

#include "utils.h"
#include "shared.h"


boolean send_to_lora();
boolean send_to_mqtt();

#include "commands.h"
#include "bme280.h"
#include "lcd.h"
#include "SDS011.h"
#include "rtc.h"
#include "menu.h"
#include "WiFiConnection.h"
#include "mqtt.h"
#include "lora.h"
#include "input.h"
#include "timing.h"
#include "GPS.h"

#line 52 "c:\\Users\\Rob\\Documents\\GitHub\\Air-Quality-Kit\\Firmware\\Sources\\SensorReader\\SensorReader.ino"
void setup();
#line 85 "c:\\Users\\Rob\\Documents\\GitHub\\Air-Quality-Kit\\Firmware\\Sources\\SensorReader\\SensorReader.ino"
void loop();
#line 52 "c:\\Users\\Rob\\Documents\\GitHub\\Air-Quality-Kit\\Firmware\\Sources\\SensorReader\\SensorReader.ino"
void setup() {
	// put your setup code here, to run once:
	Serial.begin(115200);
	Serial.println("Starting...");


	// turn on the 3.3 volt rail
	// drop it to reset any I2C devices

	pinMode(21, OUTPUT);
	digitalWrite(21, LOW);
	delay(500);
	digitalWrite(21, HIGH);
	delay(500);

	uint64_t chipid = ESP.getEfuseMac();//The chip ID is essentially its MAC address(length: 6 bytes).
	Serial.printf("ESP32 Chip ID = %04X", (uint16_t)(chipid >> 32));//print High 2 bytes
	Serial.printf("%08X\n", (uint32_t)chipid);//print Low 4bytes.
	setup_lcd();
	setup_bme280();
	setup_sensor();
	setup_input();
	if(settings.rtcOn) setup_rtc();
	setup_menu();
	setup_commands();
	setup_timing();
	if(settings.loraOn) setup_lora();
	if(settings.wiFiOn) setup_wifi();
	if(settings.mqttOn) setup_mqtt();
	if(settings.gpsOn) setup_gps();

}

void loop()
{
	loop_sensor();
	loop_input();
	loop_bme280();
	if (settings.rtcOn) loop_rtc();
	loop_menu();
	loop_timing();
	if (settings.wiFiOn) loop_wifi();
	if (settings.mqttOn) loop_mqtt();
	if (settings.loraOn) loop_lora();
	loop_commands();
	if (settings.gpsOn) loop_gps();
	// update the display last of all
	loop_lcd();
	delay(1);  /// always delay at least 1msec
}

