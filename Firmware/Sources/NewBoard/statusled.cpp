#include <Arduino.h>

#include "utils.h"
#include "settings.h"
#include "statusled.h"
#include "processes.h"
#include "timing.h"
#include "messages.h"

unsigned long millisAtLastFlash;
int flashDurationInMillis = 0;
bool ledLit = false;

struct StatusLedSettings statusLedSettings;

void setDefaultstatusLedOutputPin(void *dest)
{
    int *destInt = (int *)dest;
    *destInt = LED_BUILTIN;
}

struct SettingItem statusLedOutputPinSetting = {
    "Status LED output Pin",
    "statusledoutputpin",
    &statusLedSettings.statusLedOutputPin,
    NUMBER_INPUT_LENGTH,
    integerValue,
    setDefaultstatusLedOutputPin,
    validateInt};

struct SettingItem statusLedOutputPinActiveLowSetting = {
    "Status LED output Active Low",
    "statusledoutputactivelow",
    &statusLedSettings.statusLedOutputPinActiveLow,
    ONOFF_INPUT_LENGTH,
    yesNo,
    setFalse,
    validateYesNo};

struct SettingItem statusLedEnabled = {
    "Status LED enabled",
    "statusledactive",
    &statusLedSettings.statusLedEnabled,
    ONOFF_INPUT_LENGTH,
    yesNo,
    setTrue,
    validateYesNo};

struct SettingItem *statusLedSettingItemPointers[] =
    {
        &statusLedOutputPinSetting,
        &statusLedOutputPinActiveLowSetting,
        &statusLedEnabled};

struct SettingItemCollection statusLedSettingItems = {
    "statusled",
    "Pin assignment, level (high or low) and enable/disable for status led output",
    statusLedSettingItemPointers,
    sizeof(statusLedSettingItemPointers) / sizeof(struct SettingItem *)};

void statusLedOff()
{
    if (!ledLit)
        return;

    if (statusLedSettings.statusLedOutputPinActiveLow)
        digitalWrite(statusLedSettings.statusLedOutputPin, true);
    else
        digitalWrite(statusLedSettings.statusLedOutputPin, false);

    ledLit = false;
}

void statusLedOn()
{
    if (ledLit)
        return;

    if (statusLedSettings.statusLedOutputPinActiveLow)
        digitalWrite(statusLedSettings.statusLedOutputPin, false);
    else
        digitalWrite(statusLedSettings.statusLedOutputPin, true);

    ledLit = true;
}

void ledToggle()
{
    millisAtLastFlash = offsetMillis();

    if (ledLit)
        statusLedOff();
    else
        statusLedOn();
}

void startstatusLedFlash(int flashLength)
{
    statusLedOn();

    millisAtLastFlash = offsetMillis();

    flashDurationInMillis = flashLength;
}

void updateStatusLedFlash()
{
    if(flashDurationInMillis == 0 )
    {
        statusLedOff();
        return;
    }

    unsigned long millisSinceLastFlash = offsetMillis() - millisAtLastFlash;

    if (millisSinceLastFlash > flashDurationInMillis)
        ledToggle();
}

#define DIGIT_FLASH_INTERVAL 400
#define DIGIT_GAP 500

void flashStatusLedDigits(int flashes)
{
    for (int i = 0; i < flashes; i++)
    {
        statusLedOn();
        delay(DIGIT_FLASH_INTERVAL);
        statusLedOff();
        delay(DIGIT_FLASH_INTERVAL);
    }
}

void flashStatusLed(int flashes)
{
    int tenPowers = 1;

    while (flashes > tenPowers)
    {
        tenPowers = tenPowers * 10;
    }

    tenPowers = tenPowers / 10;

    while (tenPowers > 0)
    {
        int digit = (flashes / tenPowers) % 10;
        flashStatusLedDigits(digit);
        tenPowers = tenPowers / 10;
        delay(DIGIT_GAP);
    }
}


void displayMessageOnStatusLed(int messageNumber, char* messageText)
{
    flashStatusLed(messageNumber);
}

int startStatusLed(struct process *statusLedProcess)
{
    pinMode(statusLedSettings.statusLedOutputPin, OUTPUT);
    statusLedOff();
    millisAtLastFlash = offsetMillis();
    bindMessageHandler(displayMessageOnStatusLed);
    return PROCESS_OK;
}

int updateStatusLed(struct process *statusLedProcess)
{
    if (!statusLedSettings.statusLedEnabled)
    {
        statusLedOff();
        return PROCESS_OK;
    }

    updateStatusLedFlash();

    return PROCESS_OK;
}

int stopstatusLed(struct process * inputSwitchProcess)
{
	return OUTPUT_LED_STOPPED;
}

void statusLedStatusMessage(struct process *inputSwitchProcess, char *buffer, int bufferLength)
{
    if (ledLit)
        snprintf(buffer, bufferLength, "Status led lit");
    else
        snprintf(buffer, bufferLength, "Status led off");
}
