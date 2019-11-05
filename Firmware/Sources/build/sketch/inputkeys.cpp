#include <Arduino.h>

#include "inputkeys.h"
#include "debug.h"
#include "processes.h"

#define UP_BUTTON 12
#define DOWN_BUTTON 0
#define SELECT_BUTTON 23

void emptyHandler()
{

}

#define AUTO_REPEAT_GAP 300
#define AUTO_REPEAT_SPEEDUP 10
#define AUTO_REPEAT_SPEED_MIN 10

struct Button {
    char * buttonName;
	void(*pressed_handler)();
	void(*released_handler)();
	int pinNo;
	bool lastState;
	unsigned long lastChangeMillis;
	unsigned long lastChangeUpdate;
};

struct Button buttons[] = { 
    { "up", emptyHandler, emptyHandler, UP_BUTTON, false, 0, AUTO_REPEAT_GAP }, 
	{ "down", emptyHandler, emptyHandler, DOWN_BUTTON, false, 0, AUTO_REPEAT_GAP }, 
	{ "enter", emptyHandler, emptyHandler, SELECT_BUTTON, false, 0, AUTO_REPEAT_GAP } };

int noOfButtons = sizeof(buttons) / sizeof(struct Button);

int startInputKeys(struct process * inputKeysProcess)
{
	unsigned long millisNow = millis();
	for (int i = 0; i < noOfButtons; i++)
	{
		pinMode(buttons[i].pinNo, INPUT_PULLUP);
		buttons[i].lastState = digitalRead(buttons[i].pinNo);
		buttons[i].lastChangeMillis = millisNow;
	}
	return PROCESS_OK;
}

int updateInputKeys(struct process * inputKeysProcess)
{
	unsigned long millisNow = millis();

	for (int i = 0; i < noOfButtons; i++)
	{
		bool state = digitalRead(buttons[i].pinNo);
		if (state != buttons[i].lastState)
		{
			// change of state
			if (state)
			{
				buttons[i].released_handler();
			}
			else
			{
				buttons[i].pressed_handler();
			}
			buttons[i].lastState = state;
			buttons[i].lastChangeMillis = millisNow; 
			buttons[i].lastChangeUpdate = AUTO_REPEAT_GAP;
		}
		if (!buttons[i].lastState)
		{
			// button is pressed - do we auto repeat?
			unsigned long timeSincePressed = millisNow - buttons[i].lastChangeMillis;
			if(timeSincePressed > buttons[i].lastChangeUpdate)
			{
				// do an auto repeat and then update the timeSincePressed
				buttons[i].pressed_handler();
				buttons[i].lastChangeMillis = millisNow; 
				if(buttons[i].lastChangeUpdate > AUTO_REPEAT_SPEED_MIN)
				{
					buttons[i].lastChangeUpdate = buttons[i].lastChangeUpdate - AUTO_REPEAT_SPEEDUP;
				}
			}
		}
	}
    return PROCESS_OK;
}

int stopInputKeys(struct process * inputKeysProcess)
{
	return PROCESS_OK;
}


void inputKeysStatusMessage(struct process * timingProcess, char * buffer, int bufferLength)
{
	snprintf(buffer, bufferLength, "Keys OK");
}

boolean bindKeyDownHandler(char * keyName, void(*handler)())
{
	for (int i = 0; i < noOfButtons; i++)
	{
        if (strcasecmp(keyName, buttons[i].buttonName) == 0)
        {
            // found the button
            buttons[i].pressed_handler = handler;
            return true;
        }
    }
    return false;
}

boolean bindKeyUpHandler(char * keyName, void(*handler)())
{
	for (int i = 0; i < noOfButtons; i++)
	{
        if (strcasecmp(keyName, buttons[i].buttonName) == 0)
        {
            // found the button
            buttons[i].pressed_handler = handler;
            return true;
        }
    }
    return false;
}
