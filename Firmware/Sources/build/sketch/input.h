
#define UP_BUTTON 12
#define DOWN_BUTTON 0
#define SELECT_BUTTON 23

void up_pressed()
{
	TRACELN("Up Pressed");
	menuUpButtonPresssed();
}

void down_pressed()
{
	TRACELN("Down Pressed");
	menuDownButtonPressed();
}

void select_pressed()
{
	TRACELN("Select Pressed");
	menuSelectButtonPressed();
}

void up_released()
{
	TRACELN("Up released");
}

void down_released()
{
	TRACELN("Down released");
}

void select_released()
{
	TRACELN("Select released");
}

#define AUTO_REPEAT_GAP 300
#define AUTO_REPEAT_SPEEDUP 10
#define AUTO_REPEAT_SPEED_MIN 10

struct Button {
	void(*pressed_handler)();
	void(*released_handler)();
	int pinNo;
	bool lastState;
	unsigned long lastChangeMillis;
	unsigned long lastChangeUpdate;
};

struct Button buttons[] = { {up_pressed, up_released, UP_BUTTON, false, 0, AUTO_REPEAT_GAP }, 
	{ down_pressed, down_released, DOWN_BUTTON, false, 0, AUTO_REPEAT_GAP }, 
	{select_pressed, select_released, SELECT_BUTTON, false, 0, AUTO_REPEAT_GAP } };

int noOfButtons = sizeof(buttons) / sizeof(struct Button);

void setup_input() 
{
	unsigned long millisNow = millis();
	for (int i = 0; i < noOfButtons; i++)
	{
		pinMode(buttons[i].pinNo, INPUT_PULLUP);
		buttons[i].lastState = digitalRead(buttons[i].pinNo);
		buttons[i].lastChangeMillis = millisNow;
	}
}

void loop_input()
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
}

