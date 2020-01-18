#include <Arduino.h>

#include "utils.h"
#include "settings.h"
#include "messages.h"
#include "processes.h"
#include "timing.h"

struct MessagesSettings messagesSettings;

struct SettingItem messagesEnabled = {
    "Messages enabled",
    "messagesactive",
    &messagesSettings.messagesEnabled,
    ONOFF_INPUT_LENGTH,
    yesNo,
    setTrue,
    validateYesNo};

struct SettingItem *messagesSettingItemPointers[] =
    {
        &messagesEnabled};

struct SettingItemCollection messagesSettingItems = {
    "messages",
    "Enable/disable for message output",
    messagesSettingItemPointers,
    sizeof(messagesSettingItemPointers) / sizeof(struct SettingItem *)};

void messagesOff()
{
    messagesSettings.messagesEnabled = false;
    saveSettings();
}

void messagesOn()
{
    messagesSettings.messagesEnabled = true;
    saveSettings();
}

int startMessages(struct process *messagesProcess)
{
    return PROCESS_OK;
}

int updateMessages(struct process *messagesProcess)
{
    return PROCESS_OK;
}

int stopmessages(struct process * inputSwitchProcess)
{
	return MESSAGES_STOPPED;
}

void messagesStatusMessage(struct process *inputSwitchProcess, char *buffer, int bufferLength)
{
    if (messagesSettings.messagesEnabled)
        snprintf(buffer, bufferLength, "Messages enabled");
    else
        snprintf(buffer, bufferLength, "Messages disabled");
}
