#include <Arduino.h>
#include <DNSServer.h>
#include <WiFiServer.h>
#include <WiFiClientSecure.h>
#include <WiFiClient.h>

#include "configwifi.h"
#include "settings.h"
#include "lcd.h"

char configSSID[WIFI_SSID_LENGTH];

std::unique_ptr<DNSServer> dnsServer;
const byte DNS_PORT = 53;

int startWifiConfig(struct process * wifiConfigProcess)
{
	setPopupMessage("Config", "Network starting");
	WiFi.mode(WIFI_AP);

	delay(100);

	WiFi.softAP(settings.deviceName);

	dnsServer.reset(new DNSServer());

	dnsServer->setErrorReplyCode(DNSReplyCode::NoError);

	delay(500);

	setPopupMessage("Host", WiFi.localIP().toString().c_str());

	return PROCESS_OK;
}

int updateWifiConfig(struct process * wifiConfigProcess)
{
	return PROCESS_OK;
}

int stopWifiConfig(struct process * wifiConfigProcess)
{
	return PROCESS_OK;
}

void wifiConfigStatusMessage(struct process * wifiConfigProcess, char * buffer, int bufferLength)
{
	snprintf(buffer, bufferLength, "Soft AP running");
}
