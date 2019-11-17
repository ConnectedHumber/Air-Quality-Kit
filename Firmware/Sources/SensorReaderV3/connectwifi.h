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

#define WIFI_CONNECT_TIMEOUT_MILLIS 20000

#define WIFI_OK 0
#define WIFI_TURNED_OFF 1
#define WIFI_ERROR_NO_NETWORKS_FOUND -1
#define WIFI_ERROR_CONNECT_TIMEOUT -2
#define WIFI_ERROR_CONNECT_FAILED -3
#define WIFI_ERROR_NO_MATCHING_NETWORKS -4
#define WIFI_ERROR_DISCONNECTED -5

#define WIFI_CONNECT_RETRY_MILLS 500000

struct WiFiSetting
{
	char * wifiSsid;
	char * wifiPassword;
};

int startWifi(struct process * wifiProcess);
int stopWiFi(struct process * wifiProcess);
int updateWifi(struct process * wifiProcess);
void wifiStatusMessage(struct process * wifiProcess, char * buffer, int bufferLength);

#endif
