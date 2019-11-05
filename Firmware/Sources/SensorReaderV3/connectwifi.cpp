#pragma once

#include "connectwifi.h"

#include "debug.h"
#include "settings.h"

// Note that if we add new WiFi passwords into the settings
// we will have to update this table. 

struct WiFiSetting wifiSettings[] =
{
	{settings.wifi1SSID, settings.wifi1PWD},
	{settings.wifi2SSID, settings.wifi2PWD},
	{settings.wifi3SSID, settings.wifi3PWD},
	{settings.wifi4SSID, settings.wifi4PWD},
	{settings.wifi5SSID, settings.wifi5PWD}
};

#define NO_OF_WIFI_SETTINGS sizeof(wifiSettings) / sizeof(struct WiFiSetting)

#define WIFI_SETTING_NOT_FOUND -1

int findWifiSetting(String ssidName)
{
	char ssidBuffer[WIFI_SSID_LENGTH];

	ssidName.toCharArray(ssidBuffer, WIFI_SSID_LENGTH);

	for (int i = 0; i < NO_OF_WIFI_SETTINGS; i++)
	{
		if (strcasecmp(wifiSettings[i].wifiSsid, ssidBuffer) == 0)
		{
			return i;
		}
	}
	return WIFI_SETTING_NOT_FOUND;
}

char wifiActiveAPName[WIFI_SSID_LENGTH];
int wifiError;

boolean firstRun = true;
unsigned long lastWiFiConnectAtteptMillis;

int * wifiStatusAddress;

int startWifi(struct process * wifiProcess)
{
	if (!settings.wiFiOn)
	{
		TRACELN("WiFi switched off");
		wifiProcess->status = WIFI_TURNED_OFF;
		return WIFI_TURNED_OFF;
	}

	wifiStatusAddress = &wifiProcess->status;

	lastWiFiConnectAtteptMillis = millis();
	int setting_number;
	TRACELN("Starting WiFi");

	// stop the device from being an access point when you don't want it 

	if (firstRun)
	{
		TRACELN("First run");
		WiFi.mode(WIFI_OFF);
		delay(500);
		WiFi.mode(WIFI_STA);
		delay(500);
		firstRun = false;
	}

	int noOfNetworks = WiFi.scanNetworks();

	if (noOfNetworks == 0)
	{
		TRACELN("No networks");
		wifiProcess->status = WIFI_ERROR_NO_NETWORKS_FOUND;
		WiFi.scanDelete();
		return WIFI_ERROR_NO_NETWORKS_FOUND;
	}

	TRACELN("Networks found");

	for (int i = 0; i < noOfNetworks; ++i) {
		setting_number = findWifiSetting(WiFi.SSID(i));
		if (setting_number != WIFI_SETTING_NOT_FOUND)
		{
			snprintf(wifiActiveAPName, WIFI_SSID_LENGTH, "%s", wifiSettings[setting_number].wifiSsid);
			TRACE("Connecting to ");
			TRACELN(wifiActiveAPName);
			WiFi.begin(wifiSettings[setting_number].wifiSsid,
				wifiSettings[setting_number].wifiPassword);
			unsigned long connectStartTime = millis();

			while (WiFi.status() != WL_CONNECTED)
			{
				TRACE(".");
				delay(500);
				if (ulongDiff(millis(), connectStartTime) > WIFI_CONNECT_TIMEOUT_MILLIS)
				{
//					WiFi.disconnect();
					wifiProcess->status = WIFI_ERROR_CONNECT_TIMEOUT;
					TRACELN("Timeout");
					return WIFI_ERROR_CONNECT_TIMEOUT;
				}
			}

			TRACELN("Deleting scan");
			WiFi.scanDelete();

			wifiError = WiFi.status();

			if (wifiError == WL_CONNECTED)
			{
				TRACELN("Wifi OK");
				wifiProcess->status = WIFI_OK;
				return WIFI_OK;
			}

			TRACE("Fail status:");
			TRACE_HEXLN(wifiError);
			wifiProcess->status = WIFI_ERROR_CONNECT_FAILED;
			return WIFI_ERROR_CONNECT_FAILED;
		}
	}
	TRACELN("No matching networks");
	wifiProcess->status = WIFI_ERROR_NO_MATCHING_NETWORKS;
	return WIFI_ERROR_NO_MATCHING_NETWORKS;
}

int stopWiFi(struct process * wifiProcess)
{
	wifiProcess->status = WIFI_TURNED_OFF;
	return WIFI_TURNED_OFF;
}

void displayWiFiStatus(int status)
{
	switch (status)
	{
	case WL_IDLE_STATUS:
		TRACE("Idle");
		break;
	case WL_NO_SSID_AVAIL:
		TRACE("No SSID");
		break;
	case WL_SCAN_COMPLETED:
		TRACE("Scan completed");
		break;
	case WL_CONNECTED:
		TRACE("Connected");
		break;
	case WL_CONNECT_FAILED:
		TRACE("Connect failed");
		break;
	case WL_CONNECTION_LOST:
		TRACE("Connection lost");
		break;
	case WL_DISCONNECTED:
		TRACE("Disconnected");
		break;
	default:
		TRACE("WiFi status value: ");
		TRACE(status);
		break;
	}
}

int updateWifi(struct process * wifiProcess)
{
	if (!settings.wiFiOn)
		return WIFI_OFF;

	if (wifiProcess->status == WIFI_OK)
	{
		int wifiStatusValue = WiFi.status();

		if (wifiStatusValue != WL_CONNECTED)
		{
			displayWiFiStatus(wifiStatusValue);
			wifiProcess->status = WIFI_ERROR_DISCONNECTED;
			lastWiFiConnectAtteptMillis = millis();
		}
	}
	else
	{
		unsigned long millisSinceWiFiConnectAttempt =
			ulongDiff(millis(), lastWiFiConnectAtteptMillis);

		if (millisSinceWiFiConnectAttempt > WIFI_CONNECT_RETRY_MILLS)
		{
			wifiProcess->status = startWifi(wifiProcess);
		}
	}

	return wifiProcess->status;
}

void wifiStatusMessage(struct process * wifiProcess, char * buffer, int bufferLength)
{
	switch (wifiProcess->status)
	{
	case WIFI_OK:
		snprintf(buffer, bufferLength, "%s: %s", wifiActiveAPName, WiFi.localIP().toString().c_str());
		break;
	case WIFI_TURNED_OFF:
		snprintf(buffer, bufferLength, "Wifi OFF");
		break;
	case WIFI_ERROR_NO_NETWORKS_FOUND:
		snprintf(buffer, bufferLength, "No Wifi networks found");
		break;
	case WIFI_ERROR_CONNECT_TIMEOUT:
		snprintf(buffer, bufferLength, "%s connect timeout", wifiActiveAPName);
		break;
	case WIFI_ERROR_CONNECT_FAILED:
		snprintf(buffer, bufferLength, "%s connect failed with error %d", wifiActiveAPName, wifiError);
		break;
	case WIFI_ERROR_NO_MATCHING_NETWORKS:
		snprintf(buffer, bufferLength, "No stored Wifi networks found");
		break;
	case WIFI_ERROR_DISCONNECTED:
		snprintf(buffer, bufferLength, "WiFi disconnected");
		break;
	default:
		snprintf(buffer, bufferLength, "WiFi status invalid");
		break;
	}
}

