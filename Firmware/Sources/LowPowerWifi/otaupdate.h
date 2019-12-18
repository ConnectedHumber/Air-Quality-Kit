#ifndef OTAUPDATE_H

#define OTAUPDATE_H

#include "processes.h"
#include "connectwifi.h"

#define OTAUPDATE_OK 0
#define OTAUPDATE_OFF 1
#define OTAUPDATE_ERROR_NO_WIFI -1

int startOtaUpdate(struct process * otaUpdateProcess);
int updateOtaUpdate(struct process * otaUpdateProcess);
int stopOtaUpdate(struct process * otaUpdateProcess);
void otaUpdateStatusMessage(struct process * otaUpdateProcess, char * buffer, int bufferLength);

struct OtaUpdateSettings {
	char autoUpdateImageServer[SERVER_NAME_LENGTH];
	char autoUpdateStatusServer[SERVER_NAME_LENGTH];
	boolean autoUpdateEnabled;
};

extern struct OtaUpdateSettings otaUpdateSettings;

extern struct SettingItemCollection otaUpdateSettingItems;

#endif