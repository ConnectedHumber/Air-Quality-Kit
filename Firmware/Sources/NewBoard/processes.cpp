#pragma once

#include "processes.h"

#include "pixels.h"
#include "configwifi.h"
#include "connectwifi.h"
#include "console.h"
#include "inputswitch.h"
#include "powercontrol.h"
#include "statusled.h"
#include "mqtt.h"
#include "otaupdate.h"
#include "settingsWebServer.h"
#include "timing.h"
#include "utils.h"

#define STATUS_DESCRIPTION_LENGTH 200



//								name	 start		update		  stop        status             activeAtStart beingUpdated status activeTime processDetails
struct process PixelProcess = { "Pixel", startPixel, updatePixel, stopPixel, pixelStatusMessage, true,		   false,		0,		0,		NULL, 
	(unsigned char *) &pixelSettings, sizeof(PixelSettings), & pixelSettingItems };

struct process WiFiProcessDescriptor = { "WiFi", startWifi, updateWifi, stopWiFi, wifiStatusMessage, true, false, 0, 0, NULL,
	(unsigned char *) &wifiConnectionSettings, sizeof(WifiConnectionSettings), &wifiConnectionSettingItems };

struct process ConsoleProcessDescriptor = { "Console", startConsole, updateConsole, stopConsole, consoleStatusMessage, true, false, 0, 0, NULL,
	NULL, 0, NULL};

struct process WebServerProcessDescriptor = { "Settingswebserver", startWebServer, updateWebServer, stopWebserver, webserverStatusMessage, false, false, 0, 0, NULL,
	NULL, 0, NULL };// don't start the web server by default

struct process MQTTProcessDescriptor = { "MQTT", startMQTT, updateMQTT, stopMQTT, mqttStatusMessage, true,  false, 0, 0, NULL,
	(unsigned char*)&mqttSettings, sizeof(MqttSettings), &mqttSettingItems };

struct process OTAUpdateProcess = { "OTAUpdate", startOtaUpdate, updateOtaUpdate, stopOtaUpdate, otaUpdateStatusMessage, false, false, 0, 0, NULL,
	(unsigned char *) &otaUpdateSettings, sizeof(OtaUpdateSettings), &otaUpdateSettingItems }; // don't start the ota update by default

struct process InputSwitchProcess = { "inputswitch", startInputSwitch, updateInputSwitch, stopInputSwitch, inputSwitchStatusMessage, true, false, 0, 0, NULL,
	(unsigned char*) &inputSwitchSettings, sizeof(InputSwitchSettings), &inputSwitchSettingItems};

struct process PowerControlOutputProcess = { "powercontroloutput", startPowerControl, updatePowerControl, stopPowerControl, powerControlStatusMessage, true, false, 0, 0, NULL,
	(unsigned char*)&powerControlSettings, sizeof(PowerControlSettings), & powerControlOutputSettingItems };


struct process StatusLedProcess = { "statusled", startStatusLed, updateStatusLed, stopstatusLed, statusLedStatusMessage, true, false, 0, 0, NULL,
	(unsigned char*) &statusLedSettings, sizeof(StatusLedSettings), &statusLedSettingItems};

struct process WiFiConfigProcess = { "Wifi Config", startWifiConfig, updateWifiConfig, stopWifiConfig, wifiConfigStatusMessage, true, false, 0, 0, NULL,
	NULL, 0, NULL }; 

struct process TimingProcess = { "Timing", startTiming, updateTiming, stopTiming, timingStatusMessage , true, false, 0, 0, NULL,
	(unsigned char *) &timingSettings, sizeof(TimingSettings), &timingSettingItems};

struct process * allProcessList[] =
{
	&PixelProcess,
	&WiFiProcessDescriptor,
	&ConsoleProcessDescriptor,
	&WebServerProcessDescriptor,
	&MQTTProcessDescriptor,
	&OTAUpdateProcess,
	&InputSwitchProcess,
	&PowerControlOutputProcess,
	&StatusLedProcess,
	&TimingProcess,
	&WiFiConfigProcess
};


struct process* baseProcessList[] =
{
	&PowerControlOutputProcess,
	&PixelProcess,
	&StatusLedProcess,
	&InputSwitchProcess
};

struct process* secondBootProcessList[] =
{
	&WiFiProcessDescriptor,
	&ConsoleProcessDescriptor,
	&WebServerProcessDescriptor,
	&MQTTProcessDescriptor,
	&OTAUpdateProcess,
	&TimingProcess,
};

struct process * runningProcessList[] =
{
	&PowerControlOutputProcess,
	&StatusLedProcess,
	&PixelProcess,
	&WiFiProcessDescriptor,
	&ConsoleProcessDescriptor,
	&WebServerProcessDescriptor,
	&MQTTProcessDescriptor,
	&OTAUpdateProcess,
	&InputSwitchProcess,
	&TimingProcess,
};


// This list is ordered in the sequence of processes that 
// should be stopped, making sure that WiFi is stopped after 
// MQTT

struct process* stoppingProcessList[] =
{
	&PowerControlOutputProcess,
	&MQTTProcessDescriptor,
	&WebServerProcessDescriptor,
	&PixelProcess,
	&WiFiProcessDescriptor,
	&ConsoleProcessDescriptor,
	&OTAUpdateProcess,
	&StatusLedProcess,
	&InputSwitchProcess,
	&TimingProcess,
};

struct process * wifiConfigProcessList[] =
{
	&WiFiConfigProcess,
	&WebServerProcessDescriptor,
	&StatusLedProcess,
	&ConsoleProcessDescriptor,
};

struct process * findProcessByName(const char * name)
{
	for (int i = 0; i < sizeof(allProcessList) / sizeof(struct process *); i++)
	{
		if (strcasecmp(allProcessList[i]->processName, name) == 0)
		{
			return allProcessList[i];
		}
	}
	return NULL;
}

struct process * findProcessSettingCollectionByName(const char * name)
{
	for (int i = 0; i < sizeof(allProcessList) / sizeof(struct process *); i++)
	{
		if (allProcessList[i]->settingItems == NULL)
			continue;

		if (strcasecmp(allProcessList[i]->settingItems->collectionName, name) == 0)
		{
			return allProcessList[i];
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
	Serial.printf("   Starting process %s: ", proc->processName);
	proc->startProcess(proc);
	proc->getStatusMessage(proc, processStatusBuffer, PROCESS_STATUS_BUFFER_SIZE);
	Serial.printf(" %s\n", processStatusBuffer);
	proc->beingUpdated = true;  // process only gets updated after it has started
}


void startProcesses(struct process* *  processList, int noOfProcesses, bool showStatus)
{
	for (int i = 0; i < noOfProcesses; i++)
	{
		process* proc = processList[i];
		if (!proc->activeAtStart) continue;
		startProcess(proc);
		if (showStatus)
		{
			addStatusItem(proc->status == PROCESS_OK);
			renderStatusDisplay();
		}
	}
}

// Start the processes required to load the rest of the processes and generate status output
void startBaseProcesses()
{
	// start all the management processes
	Serial.println("Starting base processes:");

	startProcesses(baseProcessList, sizeof(baseProcessList) / sizeof(struct process*), false);

}

void startActiveSensorProcesses()
{
	Serial.println("Starting main processes:");
	startProcesses(secondBootProcessList, sizeof(secondBootProcessList) / sizeof(struct process*), true);
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

void updateRunningProcesses()
{
	//Serial.println("Updating running processes");
	for (int i = 0; i < sizeof(runningProcessList) / sizeof(struct process *); i++)
	{
		//Serial.print(runningProcessList[i]->processName);
		//Serial.print(' ');
		if (runningProcessList[i]->beingUpdated)
		{
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

void iterateThroughProcesses(void (*func) (process * p))
{
	for (int i = 0; i < sizeof(allProcessList) / sizeof(struct process*); i++)
	{
		func(allProcessList[i]);
	}
}

void stopProcesses()
{
	for (int i = 0; i < sizeof(stoppingProcessList) / sizeof(struct process*); i++)
	{
		runningProcessList[i]->stopProcess(runningProcessList[i]);
	}
}


void iterateThroughProcessSecttings(void (*func) (unsigned char * settings, int size))
{
	for (int i = 0; i < sizeof(allProcessList) / sizeof(struct process*); i++)
	{
		func(allProcessList[i]->settingsStoreBase, 
			allProcessList[i]->settingsStoreLength);
	}
}


void iterateThroughProcessSettingCollections(void (*func) (SettingItemCollection* s))
{
	for (int i = 0; i < sizeof(allProcessList) / sizeof(struct process*); i++)
	{
		if (allProcessList[i]->settingItems != NULL)
		{
			func(allProcessList[i]->settingItems);
		}
	}
}

void iterateThroughProcessSettings(void (*func) (SettingItem* s))
{
	for (int i = 0; i < sizeof(allProcessList) / sizeof(struct process*); i++)
	{
		if (allProcessList[i]->settingItems != NULL)
		{
			for (int j = 0; j < allProcessList[i]->settingItems->noOfSettings; j++)
			{
				func(allProcessList[i]->settingItems->settings[j]);
			}
		}
	}
}

void resetProcessesToDefaultSettings()
{
	for (int i = 0; i < sizeof(allProcessList) / sizeof(struct process*); i++)
	{
		if (allProcessList[i]->settingItems != NULL)
		{
			for (int j = 0; j < allProcessList[i]->settingItems->noOfSettings; j++)
			{
				void* dest = allProcessList[i]->settingItems->settings[j]->value;
				allProcessList[i]->settingItems->settings[j]->setDefault(dest);
			}
		}
	}
}


SettingItem* FindProcesSettingByFormName(const char* settingName)
{
	for (int i = 0; i < sizeof(allProcessList) / sizeof(struct process*); i++)
	{
		process * testProcess = allProcessList[i];

		if (testProcess->settingItems != NULL)
		{
			SettingItemCollection* testItems = testProcess->settingItems;

			for (int j = 0; j < testProcess->settingItems->noOfSettings; j++)
			{
				SettingItem* testSetting = testItems->settings[j];
				if (matchSettingName(testSetting, settingName))
				{
					return testSetting;
				}
			}
		}
	}
	return NULL;
}
