#pragma once

/*******************************************************************************
* Copyright (c) 2015 Thomas Telkamp and Matthijs Kooijman
*
* Permission is hereby granted, free of charge, to anyone
* obtaining a copy of this document and accompanying files,
* to do whatever they want with them without any restriction,
* including, but not limited to, copying, modification and redistribution.
* NO WARRANTY OF ANY KIND IS PROVIDED.
*
* This uses ABP (Activation-by-personalisation), where a DevAddr and
* Session keys are preconfigured (unlike OTAA, where a DevEUI and
* application key is configured, while the DevAddr and session keys are
* assigned/generated in the over-the-air-activation procedure).
*
*
* Note: LoRaWAN per sub-band duty-cycle limitation is enforced (1% in
* g1, 0.1% in g2), but not the TTN fair usage policy (which is probably
* violated by this sketch when left running for longer)!

* To use this sketch, first register your application and device with
* the things network, to set or generate an AppEUI, DevEUI and AppKey.
* Multiple devices can use the same AppEUI, but each device has its own
* DevEUI and AppKey.
*
* Do not forget to define the radio type correctly in config.h.
*
*******************************************************************************/

#include <lmic.h>
#include <hal/hal.h>
#include <SPI.h>

#include <Arduino.h>

#include "lora.h"
#include "debug.h"
#include "utils.h"
#include "settings.h"
#include "menu.h"
#include "processes.h"
#include "errors.h"

bool lora_otaa_joined = false;
bool sleeping = false;

#define LORA_STATUS_BUFFER_SIZE 200
#define LORA_STATUS_BUFFER_LIMIT LORA_STATUS_BUFFER_SIZE - 1

char LoraStatusBuffer[LORA_STATUS_BUFFER_SIZE];

int loraSentCount = 0;

#define DEBUG_LORA //change to DEBUG to see Serial messages

#ifdef DEBUG_LORA
#define DEBUG_PRINT(x) Serial.print(x)
#define DEBUG_PRINTLN(x) Serial.println(x)
#define DEBUG_HEX_PRINT(x, y) Serial.print(x, y)
#else
#define DEBUG_PRINT(x)
#define DEBUG_PRINTLN(x)
#define DEBUG_HEX_PRINT(x, y)
#endif

boolean validateLoRAKey(void *dest, const char *newValueStr)
{
	return decodeHexValueIntoBytes((uint8_t *)dest, newValueStr, LORA_KEY_LENGTH) == WORKED_OK;
}

boolean validateLoRaID(void *dest, const char *newValueStr)
{
	return decodeHexValueIntoUnsignedLong((u4_t *)dest, newValueStr) == WORKED_OK;
}

void setDefaultLoRasecsPerUpdate(void *dest)
{
	int *destInt = (int *)dest;
	*destInt = 360;
}

void setDefaultLoRaAbpAppKey(void *dest)
{
	validateLoRAKey(dest, "B04C8C1286439B81F1AAA2D04FEA1B31");
}

void setDefaultLoRaAbpNwkKey(void *dest)
{
	validateLoRAKey(dest, "27315ED03ED864DA00B0744DB3715D28");
}

void setDefaultLoRaAbpDevAddrKey(void *dest)
{
	validateLoRaID(dest, "26011DAB");
}

struct SettingItem loraSettingItems[] =
	{
		"LoRa Active (yes or no)", "loraactive", &loRaSettings.loraOn, ONOFF_INPUT_LENGTH, yesNo, setFalse, validateYesNo,
		"LoRa Seconds per update", "lorasecsperupdate", &loRaSettings.seconds_per_lora_update, NUMBER_INPUT_LENGTH, integerValue, setDefaultLoRasecsPerUpdate, validateInt,
		"LoRa Using ABP (yes or no)", "lorausingabp", &loRaSettings.loraAbp, YESNO_INPUT_LENGTH, yesNo, setTrue, validateYesNo,
		"Lora ABP App Key", "loraabpappkey", &loRaSettings.lora_abp_APPSKEY, LORA_KEY_LENGTH, loraKey, setDefaultLoRaAbpAppKey, validateLoRAKey,
		"Lora ABP Nwk Key", "loraabpnwkkey", &loRaSettings.lora_abp_NWKSKEY, LORA_KEY_LENGTH, loraKey, setDefaultLoRaAbpNwkKey, validateLoRAKey,
		"Lora ABP Dev. addr", "loraabpdevaddr", &loRaSettings.lora_abp_DEVADDR, LORA_EUI_LENGTH, loraID, setDefaultLoRaAbpDevAddrKey, validateLoRaID};

int noOfLoraSettingItems = sizeof(loraSettingItems) / sizeof(struct SettingItem);

void os_getArtEui(u1_t *buf)
{
	memcpy(buf, loRaSettings.lora_otaa_APPEUI, 8);
}

// provide DEVEUI (8 bytes, LSBF)
void os_getDevEui(u1_t *buf)
{
	memcpy(buf, loRaSettings.lora_otaa_DEVEUI, 8);
}

// provide APPKEY key (16 bytes)
void os_getDevKey(u1_t *buf)
{
	memcpy(buf, loRaSettings.lora_otaa_APPKEY, 16);
}

static osjob_t sendjob;
static osjob_t initjob;

// Pin mapping is hardware specific.
// Pin mapping is hardware specific.
// GPIO5  -- SX1278's SCK
// GPIO19 -- SX1278's MISO
// GPIO27 -- SX1278's MOSI
// GPIO18 -- SX1278's CS
// GPIO14 -- SX1278's RESET
// GPIO26 -- SX1278's IRQ(Interrupt Request)
//

// From https://github.com/jpraychev/Heltec_ESP32/blob/master/test_heltec.ino

const lmic_pinmap lmic_pins = {
	.nss = 18,
	.rxtx = LMIC_UNUSED_PIN,
	.rst = 14,
	.dio = {26, 35, 34},
};

bool OK_to_send = true;

// initial job
static void initfunc(osjob_t *j)
{
	if (loRaSettings.loraAbp)
	{
		// do nothing for abp
	}
	else
	{
		// reset MAC state
		LMIC_reset();
		// start joining
		LMIC_startJoining();
		// init done - onEvent() callback will be invoked...
	}
}

void lora_deliver_command_result(char *result)
{
	if (LMIC.opmode & OP_TXRXPEND)
	{
		DEBUG_PRINTLN("OP_TXRXPEND, not sending");
	}
	else
	{
		// Prepare upstream data transmission at the next possible time.
		LMIC_setTxData2(1, (uint8_t *)result, strlen(result), 0);
		DEBUG_PRINTLN("Sending: ");
	}
}

void default_act_onBinary_command(u1_t *buffer, int length, char *displayBuffer, int displayBufferLength)
{
}

void (*act_onBinary_command)(u1_t *buffer, int length, char *displayBuffer, int displayBufferLength);

void onEvent(ev_t ev)
{
	int i, j;
	switch (ev)
	{
	case EV_SCAN_TIMEOUT:
		DEBUG_PRINTLN("EV_SCAN_TIMEOUT");
		break;
	case EV_BEACON_FOUND:
		DEBUG_PRINTLN("EV_BEACON_FOUND");
		break;
	case EV_BEACON_MISSED:
		DEBUG_PRINTLN("EV_BEACON_MISSED");
		break;
	case EV_BEACON_TRACKED:
		DEBUG_PRINTLN("EV_BEACON_TRACKED");
		break;
	case EV_JOINING:
		DEBUG_PRINTLN("EV_JOINING");
		break;
	case EV_JOINED:
		DEBUG_PRINTLN("EV_JOINED");
		// Disable link check validation (automatically enabled
		// during join, but not supported by TTN at this time).
		LMIC_setLinkCheckMode(0);
		// after Joining a job with the values will be sent.
		lora_otaa_joined = true;
		break;
	case EV_RFU1:
		DEBUG_PRINTLN("EV_RFU1");
		break;
	case EV_JOIN_FAILED:
		DEBUG_PRINTLN("EV_JOIN_FAILED");
		break;
	case EV_REJOIN_FAILED:
		DEBUG_PRINTLN("EV_REJOIN_FAILED");
		// Re-init
		os_setCallback(&initjob, initfunc);
		break;
	case EV_TXCOMPLETE:
		if (LMIC.dataLen)
		{
			act_onBinary_command(&LMIC.frame[LMIC.dataBeg], LMIC.dataLen, LoraStatusBuffer, LORA_STATUS_BUFFER_LIMIT);
		}

		DEBUG_PRINTLN("EV_TXCOMPLETE (includes waiting for RX windows)");
		loraSentCount++;
		OK_to_send = true;
		startPopUpMessage("LoRa", "Message sent", POPUP_DURATION_MILLIS);
		break;
	case EV_LOST_TSYNC:
		DEBUG_PRINTLN("EV_LOST_TSYNC");
		break;
	case EV_RESET:
		DEBUG_PRINTLN("EV_RESET");
		break;
	case EV_RXCOMPLETE:
		// data received in ping slot
		DEBUG_PRINTLN("EV_RXCOMPLETE");
		break;
	case EV_LINK_DEAD:
		DEBUG_PRINTLN("EV_LINK_DEAD");
		break;
	case EV_LINK_ALIVE:
		DEBUG_PRINTLN("EV_LINK_ALIVE");
		break;
	default:
		DEBUG_PRINTLN("Unknown event");
		break;
	}
}

void do_send(osjob_t *j, float pub_avg_ppm_25, float pub_avg_ppm_10, float temperature, float pascal, float humidity)
{
	Serial.println("sending");
	byte buffer[5];
	uint16_t t_value, p_value, s_value;
	uint8_t h_value;
	DEBUG_PRINT("Pressure: ");
	DEBUG_PRINT(pascal);
	DEBUG_PRINT("Pa\t");
	DEBUG_PRINT("Temperature: ");
	DEBUG_PRINT(temperature);
	DEBUG_PRINT("C\t");
	DEBUG_PRINT("\tHumidity: ");
	DEBUG_PRINT(humidity);
	DEBUG_PRINTLN("%");
	temperature = constrain(temperature, -24, 40);					//temp in range -24 to 40 (64 steps)
	pascal = constrain(pascal, 970, 1034);							//pressure in range 970 to 1034 (64 steps)*/
	t_value = uint16_t((temperature * (100 / 6.25) + 2400 / 6.25)); //0.0625 degree steps with offset
																	// no negative values
	DEBUG_PRINT("Coded TEMP: ");
	DEBUG_HEX_PRINT(t_value, HEX);
	p_value = uint16_t((pascal - 970) / 1); //1 mbar steps, offset 970.
	DEBUG_PRINT("\tCoded Pascal: ");
	DEBUG_HEX_PRINT(p_value, HEX);
	s_value = (p_value << 10) + t_value; // putting the bits in the right place
	h_value = uint8_t(humidity);
	DEBUG_PRINT("\tCoded Humidity: ");
	DEBUG_HEX_PRINT(h_value, HEX);
	DEBUG_PRINTLN("");
	DEBUG_PRINT("\tCoded sent: ");
	DEBUG_HEX_PRINT(h_value, HEX);
	DEBUG_HEX_PRINT(s_value, HEX);
	DEBUG_PRINTLN("");
	buffer[0] = s_value & 0xFF; //lower byte
	buffer[1] = s_value >> 8;   //higher byte
	buffer[2] = h_value;
	buffer[3] = (int)(pub_avg_ppm_25 + 0.5f); // just send the sensor values as int readings
	buffer[4] = (int)(pub_avg_ppm_10 + 0.5f);
	// Check if there is not a current TX/RX job running
	if (LMIC.opmode & OP_TXRXPEND)
	{
		DEBUG_PRINTLN("OP_TXRXPEND, not sending");
	}
	else
	{
		// Prepare upstream data transmission at the next possible time.
		LMIC_setTxData2(1, (uint8_t *)buffer, 5, 0);
		DEBUG_PRINTLN("Sending: ");
	}
}

void abp_setup_lora()
{
	// LMIC init
	os_init();
	// Reset the MAC state. Session and pending data transfers will be discarded.
	LMIC_reset();

	// Must be called after the loRaSettings values have been loaded by command.h
	LMIC_setSession(0x1, loRaSettings.lora_abp_DEVADDR, loRaSettings.lora_abp_NWKSKEY, loRaSettings.lora_abp_APPSKEY);

	LMIC_setClockError(MAX_CLOCK_ERROR * 10 / 100);

#if defined(CFG_eu868)
	// Set up the channels used by the Things Network, which corresponds
	// to the defaults of most gateways. Without this, only three base
	// channels from the LoRaWAN specification are used, which certainly
	// works, so it is good for debugging, but can overload those
	// frequencies, so be sure to configure the full frequency range of
	// your network here (unless your network autoconfigures them).
	// Setting up channels should happen after LMIC_setSession, as that
	// configures the minimal channel set.
	// NA-US channels 0-71 are configured automatically
	LMIC_setupChannel(0, 868100000, DR_RANGE_MAP(DR_SF12, DR_SF7), BAND_CENTI);  // g-band
	LMIC_setupChannel(1, 868300000, DR_RANGE_MAP(DR_SF12, DR_SF7B), BAND_CENTI); // g-band
	LMIC_setupChannel(2, 868500000, DR_RANGE_MAP(DR_SF12, DR_SF7), BAND_CENTI);  // g-band
	LMIC_setupChannel(3, 867100000, DR_RANGE_MAP(DR_SF12, DR_SF7), BAND_CENTI);  // g-band
	LMIC_setupChannel(4, 867300000, DR_RANGE_MAP(DR_SF12, DR_SF7), BAND_CENTI);  // g-band
	LMIC_setupChannel(5, 867500000, DR_RANGE_MAP(DR_SF12, DR_SF7), BAND_CENTI);  // g-band
	LMIC_setupChannel(6, 867700000, DR_RANGE_MAP(DR_SF12, DR_SF7), BAND_CENTI);  // g-band
	LMIC_setupChannel(7, 867900000, DR_RANGE_MAP(DR_SF12, DR_SF7), BAND_CENTI);  // g-band
	LMIC_setupChannel(8, 868800000, DR_RANGE_MAP(DR_FSK, DR_FSK), BAND_MILLI);   // g2-band
																				 // TTN defines an additional channel at 869.525Mhz using SF9 for class B
																				 // devices' ping slots. LMIC does not have an easy way to define set this
																				 // frequency and support for class B is spotty and untested, so this
																				 // frequency is not configured here.
#elif defined(CFG_us915)
	// NA-US channels 0-71 are configured automatically
	// but only one group of 8 should (a subband) should be active
	// TTN recommends the second sub band, 1 in a zero based count.
	// https://github.com/TheThingsNetwork/gateway-conf/blob/master/US-global_conf.json
	LMIC_selectSubBand(1);
#endif

	// Disable link check validation
	LMIC_setLinkCheckMode(0);

	// TTN uses SF9 for its RX2 window.
	LMIC.dn2Dr = DR_SF9;

	// Set data rate and transmit power for uplink (note: txpow seems to be ignored by the library)
	LMIC_setDrTxpow(DR_SF7, 14);
}

void otaa_setup_lora()
{
	os_init();
	// Reset the MAC state. Session and pending data transfers will be discarded.
	os_setCallback(&initjob, initfunc);
	LMIC_reset();
	LMIC_setClockError(MAX_CLOCK_ERROR * 10 / 100);
}

boolean publishReadingsToLoRa(float pm25Average, float pm10Average, float temperature, float pressure, float humidity)
{
	if (!loRaSettings.loraAbp)
	{
		// Using otaa
		if (!lora_otaa_joined)
		{
			TRACELN("Waiting to join using otaa");
			return false;
		}
	}

	if (!OK_to_send)
	{
		TRACELN("waiting to send");
		return false;
	}

	do_send(&sendjob, pm25Average, pm10Average, temperature, pressure, humidity); // Sent sensor values

	OK_to_send = false;
	return true;
}

int getLoraSentCount()
{
	return loraSentCount;
}

int startLoRa(struct process *loraProcess)
{
	act_onBinary_command = default_act_onBinary_command;
	LoraStatusBuffer[0] = 0;

	loraSentCount = 0;

	if (loRaSettings.loraAbp)
	{
		abp_setup_lora();
	}
	else
	{
		otaa_setup_lora();
	}

	return PROCESS_OK;
}

int updateLoRa(struct process *loraProcess)
{
	os_runloop_once();
	return PROCESS_OK;
}

int stopLoRa(struct process *loraProcess)
{
	return PROCESS_OK;
}

void loraStatusMessage(struct process *loraProcess, char *buffer, int bufferLength)
{
	snprintf(buffer, bufferLength, "Lora sent: %d", loraSentCount);
}
