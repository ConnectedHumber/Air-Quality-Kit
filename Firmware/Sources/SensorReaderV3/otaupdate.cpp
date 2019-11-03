#include <Arduino.h>
#include <DNSServer.h>
#include <WiFiUdp.h>
#include <WiFiServer.h>
#include <WiFiClientSecure.h>
#include <WiFiClient.h>

#include "utils.h"
#include "processes.h"
#include "otaupdate.h"

struct process * otaWiFiProcess = NULL;


int startOtaUpdate(struct process * otaUpdateProcess)
{
	if (otaWiFiProcess == NULL)
	{
		otaWiFiProcess = findProcessByName("WiFi");
	}

	if (otaWiFiProcess->status != WIFI_OK)
	{
		otaUpdateProcess->status = OTAUPDATE_ERROR_NO_WIFI;
		return OTAUPDATE_ERROR_NO_WIFI;
	}
}
int updateOtaUpdate(struct process * otaUpdateProcess)
{
	return otaUpdateProcess->status;
}

int stopOtaUpdate(struct process * otaUpdateProcess)
{
	otaUpdateProcess->status = OTAUPDATE_OFF;
	return OTAUPDATE_OFF;
}

void otaUpdateStatusMessage(struct process * otaUpdateProcess, char * buffer, int bufferLength)
{
	switch (otaUpdateProcess->status)
	{
	case OTAUPDATE_OK:
		snprintf(buffer, bufferLength, "OTA update OK");
		break;
	case OTAUPDATE_OFF:
		snprintf(buffer, bufferLength, "OTA update OFF");
		break;
	case OTAUPDATE_ERROR_NO_WIFI:
		snprintf(buffer, bufferLength, "No Wifi for OTA update");
		break;
	default:
		snprintf(buffer, bufferLength, "OTA status invalid");
		break;
	}
}


