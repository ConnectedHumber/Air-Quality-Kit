#ifndef LORA_H

#define LORA_H

#include "utils.h"
#include "processes.h"

int getLoraSentCount();

boolean publishReadingsToLoRa (float pm10Average, float pm25Average, float temperature, float pressure, float humidity);

int startLoRa(struct process * loraProcess);

int updateLoRa(struct process * loraProcess);

int stopLoRa(struct process * loraProcess);

bool loRaActive();

void loraStatusMessage(struct process * loraProcess, char * buffer, int bufferLength);

extern struct SettingItemCollection loraSettingItems;


#define LORA_KEY_LENGTH 16
#define LORA_EUI_LENGTH 8

struct LoRaSettings
{
	boolean loraOn;
	boolean loraAbp;

	int seconds_per_lora_update;

	u4_t lora_abp_DEVADDR;
	u1_t lora_abp_NWKSKEY[LORA_KEY_LENGTH];
	u1_t lora_abp_APPSKEY[LORA_KEY_LENGTH];

	u1_t lora_otaa_APPKEY[LORA_KEY_LENGTH];
	u1_t lora_otaa_DEVEUI[LORA_EUI_LENGTH];
	u1_t lora_otaa_APPEUI[LORA_EUI_LENGTH];
} ;

extern LoRaSettings loRaSettings;

#endif