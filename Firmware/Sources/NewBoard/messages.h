#ifndef MESSAGES_H

#define MESSAGES_H


struct MessagesSettings {
	bool messagesEnabled;
};

void displayMessage(int messageNumber, char * messageText);
bool bindMessageHandler(void(*newHandler)(int messageNumber, char* messageText));

void messagesOff();

void messagesOn();

int startMessages(struct process * messagesProcess);

int updateMessages(struct process * messagesProcess);

int stopmessages(struct process * messagesProcess);

void messagesStatusMessage(struct process * inputSwitchProcess, char * buffer, int bufferLength);

extern struct MessagesSettings messagesSettings;

extern struct SettingItemCollection messagesSettingItems;

#define MESSAGES_STOPPED 0

#endif