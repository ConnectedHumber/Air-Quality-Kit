#ifndef PROCESSES_H

#define	PROCESSES_H

#include <Arduino.h>
#include "settings.h"

#define PROCESS_OK 0

struct process
{
	char * processName;
	int(*startProcess)(struct process *);
	int(*udpateProcess)(struct process *);
	int(*stopProcess)(struct process *);
	void(*getStatusMessage)(struct process *, char * buffer, int bufferLength);
	boolean activeAtStart;
	boolean beingUpdated;  // true if the process is running
	int status;      // zero means OK - any other value is an error state
	unsigned long activeTime;
	void * processDetails;
	unsigned char* settingsStoreBase;
	int settingsStoreLength;
	SettingItemCollection * settingItems;
};

extern struct process PixelProcess;
extern struct process WiFiProcessDescriptor;
extern struct process ConsoleProcessDescriptor;
extern struct process WebServerProcessDescriptor;
extern struct process MQTTProcessDescriptor;
extern struct process OTAUpdateProcess;
extern struct process InputSwitchProcess;
extern struct process WiFiConfigProcess;
extern struct process LCDProcess;
extern struct process TimingProcess; 
extern struct process InputKeysProcess; 
extern struct process StatusLedProcess;
extern struct process LoRaProcess;
extern struct process MessageProcess;

SettingItem* FindProcesSettingByFormName(const char* settingName);
void resetProcessesToDefaultSettings();

struct process * findProcessByName(const char * name);
struct process * startProcessByName(char * name);
void startProcess(process * proc);
void startBaseProcesses();
void startActiveSensorProcesses();
void stopProcesses();
void startWifiConfigProcesses();
void updateWifiConfigProcesses();
void dumpProcessStatus();
void updateProcess(struct process * process);
void updateRunningProcesses();
void displayProcessStatus();

void iterateThroughProcessSettings(void (*func) (SettingItem* s));
void iterateThroughProcesses(void (*func) (process* p));
void iterateThroughProcessSettingCollections(void (*func) (SettingItemCollection* s));
void iterateThroughProcessSecttings(void (*func) (unsigned char* settings, int size));

struct process * findProcessSettingCollectionByName(const char * name);

#endif