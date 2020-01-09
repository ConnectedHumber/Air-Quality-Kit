#ifndef STATUSLED_H

#define STATUSLED_H

#define WIFI_FLASH_MSECS 200
#define MQTT_FLASH_MSECS 500

#define OUTPUT_LED_STOPPED 1


struct StatusLedSettings {
	int statusLedOutputPin;
	bool statusLedOutputPinActiveLow;
	bool statusLedEnabled;
};


void statusLedOff();

void statusLedOn();

void statusLedToggle();

void startstatusLedFlash (int flashLength);

void stopStatusLedFlash();

int startStatusLed(struct process * statusLedProcess);

int updateStatusLed(struct process * statusLedProcess);

int stopstatusLed(struct process * statusLedProcess);

void statusLedStatusMessage(struct process * inputSwitchProcess, char * buffer, int bufferLength);

extern struct StatusLedSettings statusLedSettings;

extern struct SettingItemCollection statusLedSettingItems;

#endif