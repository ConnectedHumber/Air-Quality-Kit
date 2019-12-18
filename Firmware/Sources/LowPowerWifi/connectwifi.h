#ifndef CONNECT_WIFI_H

#define CONNECT_WIFI_H

#include <Arduino.h>
#include <WiFi.h>
#include <DNSServer.h>
#include <WiFiUdp.h>
#include <WiFiServer.h>
#include <WiFiClient.h>

#include "utils.h"
#include "processes.h"
#include "settings.h"

#define WIFI_CONNECT_TIMEOUT_MILLIS 20000

#define WIFI_OK 0
#define WIFI_TURNED_OFF 1
#define WIFI_ERROR_NO_NETWORKS_FOUND -1
#define WIFI_ERROR_CONNECT_TIMEOUT -2
#define WIFI_ERROR_CONNECT_FAILED -3
#define WIFI_ERROR_NO_MATCHING_NETWORKS -4
#define WIFI_ERROR_DISCONNECTED -5

#define WIFI_CONNECT_RETRY_MILLS 500000

struct WifiConnectionSettings
{
	boolean wiFiOn;
	char wifi1SSID[WIFI_SSID_LENGTH];
	char wifi1PWD[WIFI_PASSWORD_LENGTH];

	char wifi2SSID[WIFI_SSID_LENGTH];
	char wifi2PWD[WIFI_PASSWORD_LENGTH];

	char wifi3SSID[WIFI_SSID_LENGTH];
	char wifi3PWD[WIFI_PASSWORD_LENGTH];

	char wifi4SSID[WIFI_SSID_LENGTH];
	char wifi4PWD[WIFI_PASSWORD_LENGTH];

	char wifi5SSID[WIFI_SSID_LENGTH];
	char wifi5PWD[WIFI_PASSWORD_LENGTH];
};

struct WiFiSetting
{
	char * wifiSsid;
	char * wifiPassword;
};

extern struct WifiConnectionSettings wifiConnectionSettings;

int startWifi(struct process * wifiProcess);
int stopWiFi(struct process * wifiProcess);
int updateWifi(struct process * wifiProcess);
void wifiStatusMessage(struct process * wifiProcess, char * buffer, int bufferLength);

extern struct SettingItemCollection wifiConnectionSettingItems;

#endif
