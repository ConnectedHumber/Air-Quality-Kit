#pragma once

#include "processes.h"

#include "pixels.h"
#include "configwifi.h"
#include "connectwifi.h"
#include "console.h"
#include "inputswitch.h"
#include "mqtt.h"
#include "otaupdate.h"
#include "settingsWebServer.h"
#include "timing.h"
#include "lcd.h"
#include "inputkeys.h"
#include "menu.h"
#include "lora.h"
#include "utils.h"

#define STATUS_DESCRIPTION_LENGTH 200



//								name	 start		update		  stop        status             activeAtStart beingUpdated status activeTime processDetails
struct process PixelProcess = { "Pixel", startPixel, updatePixel, stopPixel, pixelStatusMessage, true,		   false,		0,		0,		NULL };
struct process WiFiProcessDescriptor = { "WiFi", startWifi, updateWifi, stopWiFi, wifiStatusMessage, true, false, 0, 0, NULL  };
struct process ConsoleProcessDescriptor = { "Console", startConsole, updateConsole, stopConsole, consoleStatusMessage, true, false, 0, 0, NULL  };
struct process WebServerProcessDescriptor = { "Settingswebserver", startWebServer, updateWebServer, stopWebserver, webserverStatusMessage, false, false, 0, 0, NULL  }; // don't start the web server by default
struct process MQTTProcessDescriptor = { "MQTT", startMQTT, updateMQTT, stopMQTT, mqttStatusMessage, true,  false, 0, 0, NULL  };
struct process OTAUpdateProcess = { "OTA", startOtaUpdate, updateOtaUpdate, stopOtaUpdate, otaUpdateStatusMessage, false, false, 0, 0, NULL  }; // don't start the ota update by default
struct process InputSwitchProcess = { "Input switch", startInputSwitch, updateInputSwitch, stopInputSwitch, inputSwitchStatusMessage, true, false, 0, 0, NULL  };
struct process WiFiConfigProcess = { "Wifi Config", startWifiConfig, updateWifiConfig, stopWifiConfig, wifiConfigStatusMessage, true, false, 0, 0, NULL  };
struct process LCDProcess = { "LCD", startLCD, updateLCD, stopLCD, LCDStatusMessage, true, false, 0, 0, NULL  };
struct process TimingProcess = { "Timing", startTiming, updateTiming, stopTiming, timingStatusMessage , true, false, 0, 0, NULL  };
struct process InputKeysProcess = { "Input keys", startInputKeys, updateInputKeys, stopInputKeys, inputKeysStatusMessage , true, false, 0, 0, NULL  };
struct process MenuProcess = { "Menu", startMenu, updateMenu, stopMenu, menuStatusMessage , true, false, 0, 0, NULL  };
struct process LoRaProcess= { "LoRa", startLoRa, updateLoRa, stopLoRa, loraStatusMessage , true, false, 0, 0, NULL  };

struct process * runningProcessList[] =
{
//	&PixelProcess,
	&WiFiProcessDescriptor,
	&ConsoleProcessDescriptor,
	&WebServerProcessDescriptor,
	&MQTTProcessDescriptor,
//	&OTAUpdateProcess,
//	&InputSwitchProcess,
	&LCDProcess,
	&TimingProcess,
	&InputKeysProcess,
	&MenuProcess,
	&LoRaProcess
};

struct process * wifiConfigProcessList[] =
{
	&WiFiConfigProcess,
	&WebServerProcessDescriptor,
	&ConsoleProcessDescriptor,
	&LCDProcess,
	&MenuProcess
};

struct process * findProcessByName(char * name)
{
	for (int i = 0; i < sizeof(runningProcessList) / sizeof(struct process *); i++)
	{
		if (strcasecmp(runningProcessList[i]->processName, name) == 0)
		{
			return runningProcessList[i];
		}
	}
	return NULL;
}

struct process * startProcessByName(char * name)
{
	struct process * targetProcess = findProcessByName(name);

	if (targetProcess != NULL)
	{
		targetProcess->startProcess(targetProcess);
		targetProcess->beingUpdated = true;
	}

	return targetProcess;
}

#define PROCESS_STATUS_BUFFER_SIZE 300

char processStatusBuffer[PROCESS_STATUS_BUFFER_SIZE];

void startProcess(process * proc)
{
	Serial.printf("Starting process %s: ", proc->processName);
	proc->startProcess(proc);
	proc->getStatusMessage(proc, processStatusBuffer, PROCESS_STATUS_BUFFER_SIZE);
	Serial.printf("%s\n", processStatusBuffer);
	proc->beingUpdated = true;  // process only gets updated after it has started
	addStatusItem(proc->status == PROCESS_OK);
	renderStatusDisplay();
}

void startDeviceProcesses()
{
	// start all the management processes
	for (int i = 0; i < sizeof(runningProcessList) / sizeof(struct process *); i++)
	{
		process * proc = runningProcessList[i];
		if (!proc->activeAtStart) continue;
		startProcess(proc);
	}
}

void startWifiConfigProcesses()
{
	for (int i = 0; i < sizeof(wifiConfigProcessList) / sizeof(struct process *); i++)
	{
		process * proc = wifiConfigProcessList[i];
		startProcess(proc);
	}
}

void updateWifiConfigProcesses()
{
	for (int i = 0; i < sizeof(wifiConfigProcessList) / sizeof(struct process *); i++)
	{
		process * proc = wifiConfigProcessList[i];
		proc->udpateProcess(proc);
	}
}

void dumpProcessStatus()
{
	Serial.println("Processes");

	for (int i = 0; i < sizeof(runningProcessList) / sizeof(struct process *); i++)
	{
		if (!runningProcessList[i]->beingUpdated)
			continue;
		Serial.printf("    %s:", runningProcessList[i]->processName);
		runningProcessList[i]->getStatusMessage(runningProcessList[i], processStatusBuffer, PROCESS_STATUS_BUFFER_SIZE);
		Serial.printf("%s Active time(microsecs):", processStatusBuffer);
		Serial.println(runningProcessList[i]->activeTime);
	}
}

void updateProcess(struct process * process)
{
	unsigned long startMicros = micros();
	process->udpateProcess(process);
	process->activeTime = ulongDiff(micros(), startMicros);
}

void updateProcesses()
{
	for (int i = 0; i < sizeof(runningProcessList) / sizeof(struct process *); i++)
	{
		if (runningProcessList[i]->beingUpdated)
		{
			//Serial.print(runningProcessList[i]->processName);
			//Serial.print(' ');
			updateProcess(runningProcessList[i]);
		}
	}
}

void displayProcessStatus()
{
	for (int i = 0; i < sizeof(runningProcessList) / sizeof(struct process *); i++)
	{
		process * displayProcess = runningProcessList[i];
		addStatusItem(displayProcess->status == PROCESS_OK);
	}
}

