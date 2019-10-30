// timeout in millis
#define WIFI_START_TIMEOUT 20000
#define WIFI_DISPLAY_TIMEOUT 3000
#define WIFI_RECONNECT_TIMEOUT 10000

unsigned long wifi_timer_start;

void start_wifi()
{
	wifi_timer_start = millis();
	wifiState = WiFiStarting;
}

void setup_wifi()
{
	if (settings.wiFiOn)
	{
		start_wifi();
	}
	else
	{
		wifiState = WiFiOff;
	}
}

#define WIFI_SETTING_NOT_FOUND -1

int find_wifi_setting(String ssidName)
{
	char ssidBuffer[WIFI_SSID_LENGTH];

	ssidName.toCharArray(ssidBuffer, WIFI_SSID_LENGTH);

	TRACE("Checking: ");
	TRACELN(ssidName);

	for (int i = 0; i < NO_OF_WIFI_SETTINGS; i++)
	{
		if (strcasecmp(settings.wifiSettings[i].wifiSsid, ssidBuffer) == 0)
		{
			return i;
		}
	}
	return WIFI_SETTING_NOT_FOUND;
}

void start_wifi_scan()
{
	startPopUpMessage("WiFi", "Resetting");
	WiFi.mode(WIFI_MODE_STA);
	delay(500);

	WiFi.disconnect();

	delay(500);

	WiFi.scanNetworks(true); // perform an asynchronous scan

	delay(500);

	startPopUpMessage("WiFi", "Scanning");
	wifiState = WiFiScanning;
	wifi_connect_count++;
}

void loop_wifi()
{
	unsigned long elapsed_time;
	int no_of_networks;
	int setting_number;

	elapsed_time = millis() - wifi_timer_start;

	switch (wifiState)
	{
	case WiFiStarting:
		TRACELN("WiFi Connection starting");
		start_wifi_scan();
		break;

	case WiFiScanning:
		
		no_of_networks = WiFi.scanComplete();

		if (no_of_networks == WIFI_SCAN_RUNNING)
			break;

		if(no_of_networks == WIFI_SCAN_FAILED)
		{
			Serial.println("Reset due to WiFi lockup");
			delay(200);
			ESP.restart();
		}

		TRACE("Networks found: ");
		TRACELN(no_of_networks);

		// if we get here we have some networks
		for (int i = 0; i < no_of_networks; ++i) {
			setting_number = find_wifi_setting(WiFi.SSID(i));
			if (setting_number != WIFI_SETTING_NOT_FOUND)
			{
				WiFi.begin(settings.wifiSettings[setting_number].wifiSsid, 
					settings.wifiSettings[setting_number].wifiPassword);
				startPopUpMessage(settings.wifiSettings[setting_number].wifiSsid, 
					"Connecting");
				wifiState = WiFiConnecting;
				break;
			}
		}

		if (wifiState != WiFiConnecting)
		{
			// didn't find a matching network
			startPopUpMessage("WiFi", "No networks");
			wifi_timer_start = millis();
			wifiState = WiFiConnectFailed;
		}

		break;

	case WiFiConnecting:

		if (WiFi.status() == WL_CONNECTED)
		{
			startPopUpMessage("WiFi", "Connected OK");
			wifiState = ShowingWifiConnected;
			wifi_timer_start = millis();
		}

		if (elapsed_time > WIFI_START_TIMEOUT)
		{
			wifi_timer_start = millis();
			startPopUpMessage("WiFi", "Connect failed");
			wifiState = WiFiConnectFailed;
		}

		break;

	case ShowingWifiConnected:
		if (elapsed_time > WIFI_DISPLAY_TIMEOUT)
		{
			wifiState = WiFiConnected;
		}
		break;

	case WiFiConnectFailed:
		if (elapsed_time > WIFI_DISPLAY_TIMEOUT)
		{
			wifi_timer_start = millis();
			wifiState = WiFiNotConnected;
		}
		break;

	case WiFiConnected:

		if (WiFi.status() != WL_CONNECTED)
		{
			wifi_timer_start = millis();
			startPopUpMessage("WiFi", "Failed");
			wifiState = ShowingWiFiFailed;
		}
		break;

	case ShowingWiFiFailed:

		if (elapsed_time > WIFI_DISPLAY_TIMEOUT)
		{
			start_wifi_scan();
		}
		break;

	case WiFiNotConnected:

		if (elapsed_time > WIFI_RECONNECT_TIMEOUT)
		{
			start_wifi_scan();
		}
		break;
	}
}

void getWiFiStatusString(char * buffer, int bufferLength)
{
	switch (wifiState)
	{
	case WiFiStarting:
		snprintf(buffer, bufferLength, "WiFi Starting");
		break;

	case WiFiScanning:
		snprintf(buffer, bufferLength, "WiFi scanning");
		break;

	case WiFiConnecting:
		snprintf(buffer, bufferLength, "WiFi connecting");
		break;

	case ShowingWifiConnected:
		snprintf(buffer, bufferLength, "WiFi connected");
		break;

	case WiFiConnectFailed:
		snprintf(buffer, bufferLength, "WiFi connect failed");
		break;

	case WiFiConnected:
		snprintf(buffer, bufferLength, "WiFi connected");
		break;

	case ShowingWiFiFailed:
		snprintf(buffer, bufferLength, "WiFi failed");
		break;

	case WiFiNotConnected:
		snprintf(buffer, bufferLength, "WiFi not connected");
		break;
	}
}
