#pragma once

#include <Arduino.h>

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
extern struct process LoRaProcess;

struct process * findProcessByName(char * name);
struct process * startProcessByName(char * name);
void startProcess(process * proc);
void startDeviceProcesses();
void startWifiConfigProcesses();
void updateWifiConfigProcesses();
void dumpProcessStatus();
void updateProcess(struct process * process);
void updateProcesses();
void displayProcessStatus();



