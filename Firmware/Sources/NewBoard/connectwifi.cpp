#pragma once

#include "connectwifi.h"

#include "debug.h"
#include "settings.h"
#include "messages.h"

boolean validateWifiSSID(void* dest, const char* newValueStr)
{
	return (validateString((char*)dest, newValueStr, WIFI_SSID_LENGTH));
}

boolean validateWifiPWD(void* dest, const char* newValueStr)
{
	return (validateString((char*)dest, newValueStr, WIFI_PASSWORD_LENGTH));
}

struct WifiConnectionSettings wifiConnectionSettings;

struct SettingItem wifiOnOff = {
	"Wifi on", "wifiactive",&wifiConnectionSettings.wiFiOn, YESNO_INPUT_LENGTH, yesNo, setFalse, validateYesNo
};

struct SettingItem wifi1SSIDSetting = {
	"WiFiSSID1", "wifissid1", wifiConnectionSettings.wifi1SSID, WIFI_SSID_LENGTH, text, setEmptyString, validateWifiSSID };
struct SettingItem wifi1PWDSetting = {
	"WiFiPassword1", "wifipwd1", wifiConnectionSettings.wifi1PWD, WIFI_PASSWORD_LENGTH, password, setEmptyString, validateWifiPWD };

struct SettingItem wifi2SSIDSetting = {
	"WiFiSSID2", "wifissid2", wifiConnectionSettings.wifi2SSID, WIFI_SSID_LENGTH, text, setEmptyString, validateWifiSSID };
struct SettingItem wifi2PWDSetting = {
	"WiFiPassword2", "wifipwd2", wifiConnectionSettings.wifi2PWD, WIFI_PASSWORD_LENGTH, password, setEmptyString, validateWifiPWD };

struct SettingItem wifi3SSIDSetting = {
	"WiFiSSID3", "wifissid3", wifiConnectionSettings.wifi3SSID, WIFI_SSID_LENGTH, text, setEmptyString, validateWifiSSID };
struct SettingItem wifi3PWDSetting = {
	"WiFiPassword3", "wifipwd3", wifiConnectionSettings.wifi3PWD, WIFI_PASSWORD_LENGTH, password, setEmptyString, validateWifiPWD };

struct SettingItem wifi4SSIDSetting = {
	"WiFiSSID4", "wifissid4", wifiConnectionSettings.wifi4SSID, WIFI_SSID_LENGTH, text, setEmptyString, validateWifiSSID };
struct SettingItem wifi4PWDSetting = {
	"WiFiPassword4", "wifipwd4", wifiConnectionSettings.wifi4PWD, WIFI_PASSWORD_LENGTH, password, setEmptyString, validateWifiPWD };

struct SettingItem wifi5SSIDSetting = {
	"WiFiSSID5", "wifissid5", wifiConnectionSettings.wifi5SSID, WIFI_SSID_LENGTH, text, setEmptyString, validateWifiSSID };
struct SettingItem wifi5PWDSetting = {
	"WiFiPassword5", "wifipwd5", wifiConnectionSettings.wifi5PWD, WIFI_PASSWORD_LENGTH, password, setEmptyString, validateWifiPWD };


struct SettingItem* wifiConnectionSettingItemPointers[] =
{
	&wifiOnOff,
&wifi1SSIDSetting,
&wifi1PWDSetting,

&wifi2SSIDSetting,
&wifi2PWDSetting,

&wifi3SSIDSetting,
&wifi3PWDSetting,

&wifi4SSIDSetting,
&wifi4PWDSetting,

&wifi5SSIDSetting,
&wifi5PWDSetting
};


struct SettingItemCollection wifiConnectionSettingItems = {
	"WiFi",
	"WiFi configuration options",
	wifiConnectionSettingItemPointers,
	sizeof(wifiConnectionSettingItemPointers) / sizeof(struct SettingItem*)
};


// Note that if we add new WiFi passwords into the settings
// we will have to update this table. 

struct WiFiSetting wifiSettings[] =
{
	{wifiConnectionSettings.wifi1SSID, wifiConnectionSettings.wifi1PWD},
	{wifiConnectionSettings.wifi2SSID, wifiConnectionSettings.wifi2PWD},
	{wifiConnectionSettings.wifi3SSID, wifiConnectionSettings.wifi3PWD},
	{wifiConnectionSettings.wifi4SSID, wifiConnectionSettings.wifi4PWD},
	{wifiConnectionSettings.wifi5SSID, wifiConnectionSettings.wifi5PWD}
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

int* wifiStatusAddress;
int wifiConnectAttempts = 0;

int startWifi(struct process* wifiProcess)
{
	if (!wifiConnectionSettings.wiFiOn)
	{
		TRACELN("WiFi switched off");
		wifiProcess->status = WIFI_TURNED_OFF;
		return WIFI_TURNED_OFF;
	}

	wifiStatusAddress = &wifiProcess->status;

	lastWiFiConnectAtteptMillis = offsetMillis();
	int setting_number;
	TRACELN("Starting WiFi");

	// stop the device from being an access point when you don't want it 

	if (firstRun)
	{
		TRACELN("WiFi first run");
		WiFi.mode(WIFI_OFF);
		delay(100);
		WiFi.mode(WIFI_STA);
		delay(100);
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
			unsigned long connectStartTime = offsetMillis();

			while (WiFi.status() != WL_CONNECTED)
			{
				TRACE(".");
				delay(500);
				if (ulongDiff(offsetMillis(), connectStartTime) > WIFI_CONNECT_TIMEOUT_MILLIS)
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
				displayMessage(WIFI_STATUS_OK_MESSAGE_NUMBER, WIFI_STATUS_OK_MESSAGE_TEXT);
				wifiConnectAttempts = 0;
				return WIFI_OK;
			}

			TRACE("Fail status:");
			TRACE_HEXLN(wifiError);
			wifiProcess->status = WIFI_ERROR_CONNECT_FAILED;
			displayMessage(WIFI_STATUS_CONNECT_FAILED_MESSAGE_NUMBER, WIFI_STATUS_CONNECT_FAILED_MESSAGE_TEXT);
			return WIFI_ERROR_CONNECT_FAILED;
		}
	}
	TRACELN("No networks found that match stored network names");
	displayMessage(WIFI_STATUS_NO_MATCHING_NETWORKS_MESSAGE_NUMBER, WIFI_STATUS_NO_MATCHING_NETWORKS_MESSAGE_TEXT);
	wifiProcess->status = WIFI_ERROR_NO_MATCHING_NETWORKS;
	return WIFI_ERROR_NO_MATCHING_NETWORKS;
}

int stopWiFi(struct process* wifiProcess)
{
	TRACELN("WiFi turned off");
	wifiProcess->status = WIFI_TURNED_OFF;
	WiFi.mode(WIFI_OFF);
	delay(500);
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

int updateWifi(struct process* wifiProcess)
{
	if (!wifiConnectionSettings.wiFiOn)
		return WIFI_OFF;

	if (wifiProcess->status == WIFI_OK)
	{
		int wifiStatusValue = WiFi.status();

		if (wifiStatusValue != WL_CONNECTED)
		{
			displayWiFiStatus(wifiStatusValue);
			wifiProcess->status = WIFI_ERROR_DISCONNECTED;
			lastWiFiConnectAtteptMillis = offsetMillis();
		}
	}
	else
	{
		unsigned long millisSinceWiFiConnectAttempt =
			ulongDiff(offsetMillis(), lastWiFiConnectAtteptMillis);

		if (millisSinceWiFiConnectAttempt > WIFI_CONNECT_RETRY_MILLS)
		{
			wifiConnectAttempts++;

			if (wifiConnectAttempts > WIFI_NO_OF_CONNECT_ATTEMPTS)
			{
				displayMessage(WIFI_STATUS_CONNECT_ABANDONED_MESSAGE_NUMBER, WIFI_STATUS_CONNECT_ABANDONED_MESSAGE_TEXT);
				forceSensorShutdown();
				// if we get here we have not been shut down - do some more retries
				wifiConnectAttempts = 0;
			}
			wifiProcess->status = startWifi(wifiProcess);
		}
	}

	return wifiProcess->status;
}

void wifiStatusMessage(struct process* wifiProcess, char* buffer, int bufferLength)
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
		snprintf(buffer, bufferLength, "No networks found that match stored network names");
		break;
	case WIFI_ERROR_DISCONNECTED:
		snprintf(buffer, bufferLength, "WiFi disconnected");
		break;
	default:
		snprintf(buffer, bufferLength, "WiFi status invalid");
		break;
	}
}

