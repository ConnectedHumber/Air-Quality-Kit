
enum MenuState
{
	// screen is turned off - nothing is displayed 
	screenOff,
	// have a special state for the situation when an action is started and the screen is off
	// this is to allow us to properly handle the situation when the screen is turned back on
	// again when an action is being displayed
	runningDisplayActive,
	// screen is showing a mnenu of some kind
	menuDisplay,
	// screen is showing a message as a result of a menu action
	// usually the message confirms that an action has been performed 
	menuMessage,
	// screen is reading a numeric value from the user
	readingNumber,
	// screen is selecting a string from a number of input strings
	stringSelection,
	// an action is a pop up message that can be displayed at any time and will
	// appear on top of the existing display
	// The action will time out after a set time and the original display reappear
	popUpDisplaying
};

MenuState menuState;

unsigned long millisecsAtLastUserInput;
unsigned long displayTimeoutInMillis = 100000;

struct Menu
{
	uint8_t activeMenuItem;
	String menuText;
	void (*methods[5])();
};

void setRunningDisplay()
{
	TRACELN("Setting running display active");
	menuState = runningDisplayActive;
	setWorkingDisplay();
}

#define MENU_STACK_SIZE 15

Menu *menuStack[MENU_STACK_SIZE];

// Stack top is the location of the top of the stack
// If it is -1 this means that the stack is empty
// Increment the stack top BEFORE adding anything to the stack

int8_t menuStackTop = -1;

void pushMenu(Menu *menu)
{
	TRACELN("Pushing menu");
	menuStackTop++;
	menuStack[menuStackTop] = menu;
}

Menu *popMenu()
{
	TRACELN("Popping menu");

	if (menuStackTop == -1)
	{
		return NULL;
	}

	Menu *result = menuStack[menuStackTop];

	menuStackTop--;
	return result;
}

Menu *peekMenuStackTop()
{
	if (menuStackTop == -1)
		return NULL;

	return menuStack[menuStackTop];
}

bool menuStackIsEmpty()
{
	return menuStackTop == -1;
}

void enterAMenu(Menu *menu)
{
	// record the position in the current menu for return
	if (!menuStackIsEmpty())
		peekMenuStackTop()->activeMenuItem = getSelectedItem();

	pushMenu(menu);

	setMenuDisplay(menu->menuText, menu->activeMenuItem);

	menuState = menuDisplay;
}

void exitFromMenu()
{
	// record the position in the current menu for return
	peekMenuStackTop()->activeMenuItem = getSelectedItem();

	popMenu();

	Menu *nextMenu = peekMenuStackTop();

	if (nextMenu == NULL)
	{
		setRunningDisplay();
		return;
	}

	setMenuDisplay(nextMenu->menuText, nextMenu->activeMenuItem);
}

unsigned long messageDisplayStartTime;
unsigned long messageDisplayTimeInMillis;

unsigned long popUpDisplayStartTime;
unsigned long popUpDisplayTimeInMillis = 1000;

void (*messageDisplayCompleteCallback)();

void beginDisplayingAMessage(String title, String text, int messageDelayInMillis, void (*callback)())
{
	messageDisplayCompleteCallback = callback;
	set_message_display(title, text);
	menuState = menuMessage;
	messageDisplayTimeInMillis = messageDelayInMillis;
	messageDisplayStartTime = millis();
}

void endTheMessageDisplay()
{
	messageDisplayCompleteCallback();
}

void checkMessageTimeout()
{
	ulong time = millis();

	ulong diff = ulongDiff(time, messageDisplayStartTime);

	if (diff > messageDisplayTimeInMillis)
	{
		// timer has expired
		endTheMessageDisplay();
	}
}

int numberBeingInput;
int numberBeingInputUpperLimit;
int numberBeingInputLowerLimit;

void (*numberInputCompleteCallback)(int result);

void completeNumberInput()
{
	numberInputCompleteCallback(numberBeingInput);
}

void incrementNumberValue()
{
	TRACELN("Increment");
	numberBeingInput++;
	if (numberBeingInput > numberBeingInputUpperLimit)
	{
		numberBeingInput = numberBeingInputLowerLimit;
	}
	TRACELN(numberBeingInput);
	set_display_number_being_input(numberBeingInput);
}

void decrementNumberValue()
{
	TRACELN("Decrement");
	numberBeingInput--;
	if (numberBeingInput < numberBeingInputLowerLimit)
	{
		numberBeingInput = numberBeingInputUpperLimit;
	}
	TRACELN(numberBeingInput);
	set_display_number_being_input(numberBeingInput);
}

void beginNumberInput(String numberInputPrompt, int initialNumberValue,
						  int lowerLimit, int upperLimit, void (*callback)(int result))
{
	numberInputCompleteCallback = callback;

	set_number_input_display(numberInputPrompt, initialNumberValue);

	numberBeingInput = initialNumberValue;
	numberBeingInputUpperLimit = upperLimit;
	numberBeingInputLowerLimit = lowerLimit;
	menuState = readingNumber;
}

void getNumber(String prompt, int start, int low, int high, void (*value_read)(int))
{
	peekMenuStackTop()->activeMenuItem = getSelectedItem();
	beginNumberInput(prompt, start, low, high, value_read);
}

#define INDIVIDUAL_SELECTION_STRING_MAX_LENGTH 10

String selectFromStringPrompt;
String stringSelections;

int currentlySelectedStringNumber;
int numberOfStringOptions;
char currentlySelectedString[INDIVIDUAL_SELECTION_STRING_MAX_LENGTH+1];

void setNoOfOptions()
{
	int pos = 0;
	numberOfStringOptions = 0;
	while (stringSelections[pos] != 0)
	{
		if (stringSelections[pos] == '\n')
		{
			numberOfStringOptions++;
		}
		pos++;
	}
	numberOfStringOptions++;
}

void setSelectedOptionFromNumber()
{
	int pos = 0;
	int selectionNo = currentlySelectedStringNumber;

	while (selectionNo > 0)
	{
		// wrap round if we hit the end of the string
		if (stringSelections[pos] == 0)
		{
			// counts as a line end
			selectionNo--;
			// wrap round to the start
			pos = 0;
		}
		else
		{
			if (stringSelections[pos] == '\n')
			{
				selectionNo--;
			}
			pos++;
		}

		if (selectionNo == 0)
		{
			break;
		}
	}

	// when we get here we are pointing at the first
	// character in the selected item

	int outputPos = 0;

	while ((outputPos < INDIVIDUAL_SELECTION_STRING_MAX_LENGTH) &&
		(stringSelections[pos] != 0) &&
		(stringSelections[pos] != '\n'))
	{
		currentlySelectedString[outputPos++] = stringSelections[pos++];
	}


	currentlySelectedString[outputPos] = 0;
}

void advanceToNextStringSelection()
{
	TRACELN("Advance to next string selection");
	currentlySelectedStringNumber++;
	if (currentlySelectedStringNumber == numberOfStringOptions)
	{
		currentlySelectedStringNumber = 0;
	}
	setSelectedOptionFromNumber();
	setStringSelection(currentlySelectedString);
}

void moveToPreviousStringSelection()
{
	TRACELN("Move to previous string selection");
	currentlySelectedStringNumber--;
	if (currentlySelectedStringNumber < 0 )
	{
		currentlySelectedStringNumber = numberOfStringOptions-1;
	}
	setSelectedOptionFromNumber();
	setStringSelection(currentlySelectedString);
}

void (*stringSelectionCompleteCallback)(int result);

void completeStringSelection()
{
	stringSelectionCompleteCallback(currentlySelectedStringNumber);
}

void beginSelectFromString(String inSelectFromStringPrompt, int initialSelectionValue, 
	String inStringSelections, void (*callback)(int result))
{
	selectFromStringPrompt = inSelectFromStringPrompt;
	currentlySelectedStringNumber = initialSelectionValue;
	stringSelections = inStringSelections;
	stringSelectionCompleteCallback = callback;

	setSelectedOptionFromNumber();

	setNoOfOptions();

	setStringSelectionDisplay(selectFromStringPrompt, currentlySelectedString);

	menuState = stringSelection;
}

void getSelectionFromString(String prompt, int start, String options, void (*value_read)(int))
{
	peekMenuStackTop()->activeMenuItem = getSelectedItem();
	beginSelectFromString(prompt, start, options, value_read);
}

void doBackFromMenu()
{
	TRACELN("Back called");
	exitFromMenu();
}

void displayMessage(String title, String message, int length, void (*completed)())
{
	// store the currectly selected menu item so that it is restored after the n
	// message has been displayed
	peekMenuStackTop()->activeMenuItem = getSelectedItem();
	beginDisplayingAMessage(title, message, length, completed);
}

void messageDisplayComplete()
{
	// restore the menu and the position in it
	Menu *next_menu = peekMenuStackTop();
	setMenuDisplay(next_menu->menuText, next_menu->activeMenuItem);
	menuState = menuDisplay;
}

void dumpTime()
{
	char time_buffer[100];
	read_time();
	sprintf(time_buffer, "%02d:%02d:%02d", pub_hour, pub_minute, pub_second);
	displayMessage("Time", time_buffer, 2000, messageDisplayComplete);
}

void dumpDate()
{

	char date_buffer[100];
	read_time();
	sprintf(date_buffer, "%d/%d/%d", pub_day, pub_month, pub_year);
	displayMessage("Date", date_buffer, 2000, messageDisplayComplete);
}

void doneSetHours(int readHours)
{
	read_time();
	set_time(readHours, pub_minute, pub_second, pub_day, pub_month, pub_year);
	dumpTime();
}

void setHours()
{
	TRACELN("Set hours called");
	getNumber("Set hours", pub_hour, 0, 23, doneSetHours);
}

void doneSetMinutes(int readMins)
{
	read_time();
	set_time(pub_hour, readMins, pub_second, pub_day, pub_month, pub_year);
	dumpTime();
}

void setMinutes()
{
	TRACELN("Set minutes called");
	getNumber("Set minutes", pub_minute, 0, 59, doneSetMinutes);
}

void zeroSeconds()
{
	read_time();
	set_time(pub_hour, pub_minute, 0, pub_day, pub_month, pub_year);
	dumpTime();
}

Menu timeMenu = {0, "Set hours\nSet minutes\nZero seconds\nBack", {setHours, setMinutes, zeroSeconds, doBackFromMenu}};

//                          J	F	M	A	M	J	J	A	S	O	N	D
int monthLengths[] = {0, 31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};

int getMonthLength(uint8_t month, uint16_t year)
{
	if (year % 4)
		return monthLengths[month];
	else
		return 29;
}

void setTime()
{
	TRACELN("Set date time called");
	enterAMenu(&timeMenu);
}

void doneSetDays(int readDays)
{
	read_time();
	set_time(pub_hour, pub_minute, pub_second, readDays, pub_month, pub_year);
	dumpDate();
}

void setDays()
{
	TRACELN("Set days called");
	int monthLength = getMonthLength(pub_month, pub_year);
	getNumber("Set days", pub_day, 1, monthLength, doneSetDays);
}

void doneSetMonths(int readMonth)
{
	if (pub_day > getMonthLength(readMonth, pub_year))
	{
		displayMessage("Invalid Month", "Day not valid for this month", 2000, messageDisplayComplete);
	}
	else
	{
		set_time(pub_hour, pub_minute, pub_second, pub_day, readMonth, pub_year);
		dumpDate();
	}
}

void setMonths()
{
	TRACELN("Set months called");
	getNumber("Set month", pub_month, 1, 12, doneSetMonths);
}

void doneSetYears(int readYears)
{
	if (pub_day > getMonthLength(pub_month, readYears))
	{
		displayMessage("Invalid year", "Day not valid for this month", 2000, messageDisplayComplete);
	}
	else
	{
		set_time(pub_hour, pub_minute, pub_second, pub_day, pub_month, readYears);
		dumpDate();
	}
}

void setYears()
{
	TRACELN("Set years called");
	getNumber("Set year", pub_year, 2000, 3000, doneSetYears);
}

Menu dateMenu = {0, "Set days\nSet months\nSet Years\nBack", {setDays, setMonths, setYears, doBackFromMenu}};

void setDate()
{
	enterAMenu(&dateMenu);
}


void menuLoraOff()
{
	TRACELN("LoRa off called");
	displayMessage("LoRa", "Off", 2000, messageDisplayComplete);
	settings.loraOn = false;
	save_settings();
}

void menuLoraOn()
{
	TRACELN("LoRa on called");
	displayMessage("LoRa", "On", 2000, messageDisplayComplete);
	settings.loraOn = true;
	save_settings();
}

void menuLoraSend()
{
	TRACELN("LoRa send called");
	displayMessage("LoRa Send", "Sending", 2000, messageDisplayComplete);
	pub_lora_force_send = true;
}

void menuLoraGapDone (int readGap)
{
	displayMessage("Lora", "Gap set", 2000, messageDisplayComplete);
	settings.seconds_per_lora_update = readGap;
	save_settings();
}

void menuLoraGap()
{
	TRACELN("Set Lora gap called");
	getNumber("Set gap", settings.seconds_per_lora_update, 60, 3600, menuLoraGapDone);
}

Menu loraMenu = {0, "Turn Lora on\nTurn Lora off\nForce Lora send\nSet send Interval\nBack", {menuLoraOn, menuLoraOff, menuLoraSend, menuLoraGap, doBackFromMenu}};

void loraSetup()
{
	enterAMenu(&loraMenu);
}

void menuMQTTOff()
{
	TRACELN("MQTT off called");
	displayMessage("MQTT", "Off", 2000, messageDisplayComplete);
	settings.mqttOn = false;
	save_settings();
}

void menuMQTTOn()
{
	TRACELN("MQTT on called");
	displayMessage("MQTT", "On", 2000, messageDisplayComplete);
	settings.mqttOn = true;
	save_settings();
}

void mqtt_send()
{
	TRACELN("MQTT send");
	displayMessage("MQTT Send", "Sending", 2000, messageDisplayComplete);
	pub_mqtt_force_send = true;
}

void menuMQTTGapDone (int readGap)
{
	settings.seconds_per_mqtt_update = readGap;
	save_settings();
	displayMessage("MQTT", "Gap set", 2000, messageDisplayComplete);
}

void menuMQTTGap()
{
	TRACELN("Set Lora gap called");
	getNumber("Set gap", settings.seconds_per_mqtt_update, 60, 3600, menuMQTTGapDone);
}


Menu mqttMenu = {0, "Turn MQTT on\nTurn MQTT off\nForce MQTT send\nSet send Interval\nBack", {menuMQTTOn, menuMQTTOff, mqtt_send, menuMQTTGap, doBackFromMenu}};

void mqtt_setup()
{
	enterAMenu(&mqttMenu);
}

// from timing.handle
void disable_serial_dump ();
void enable_serial_dump ();

void menuLoggingStateSelected(int stateValue)
{
	char buffer[100];
	sprintf(buffer, "State %s", loggingStateNames[stateValue]);
	displayMessage("Logging", buffer, 2000, messageDisplayComplete);
	settings.logging = (Logging_State) stateValue;
	save_settings();
}

void selectLoggingState()
{
	TRACELN("Set logging state called");
	getSelectionFromString("Set Logging", settings.logging, "off\nparticles\ntemp\npressure\nhumidity\nall",menuLoggingStateSelected);
}

// from timing.h
void deepSleepProcessor(int timeInSeconds);
void startPopUpMessage(String title, String text);

void sleepProcessor()
{
	startPopUpMessage("Sleep", "Going to sleep");
	delay(1000);
	deepSleepProcessor(30);
}

Menu logingMenu = {0, "State\nSleep\nBack", {selectLoggingState, sleepProcessor, doBackFromMenu}};

void loggingSetup()
{
	enterAMenu(&logingMenu);
}


Menu mainMenu = {0, "Logging\nLoRa\nMQTT\nBack", {loggingSetup, loraSetup, mqtt_setup, doBackFromMenu}};

void updatePopupMessage(String title, String text)
{
	setPopupMessage(title, text);
}

MenuState stateBeforeAction;

// actions are "pop up" items that are displayed on top of the existing screen
// they are displayed for a set time and then clear, restoring the display underneath
// they are asynchronous (can happen at any time)
// when the screen is turned off they must not be displayed

void startPopUpMessage(String title, String text)
{
	TRACELN("Start action");

	switch (menuState)
	{
	case screenOff:
		// do nothing
		break;

	case runningDisplayActive:
	case menuMessage:
	case menuDisplay:
	case readingNumber:
	case stringSelection:
		// these states can be interrupted by an action display
		// record the previous state, display the action and then 
		// change to the performing action state
		stateBeforeAction = menuState;
		setPopupMessage(title, text);
		menuState = popUpDisplaying;
		popUpDisplayStartTime = millis();
		break;

	case popUpDisplaying:
		// If we try to start an action in the middle of one
		// just update the action on the screen
		updatePopupMessage(title, text);
		popUpDisplayStartTime = millis();
		break;
	}
}

// called to tear down the existing popup and return to the previous screen display state

void endPopupDisplay()
{
	TRACE("End pop up: ");
	TRACELN(stateBeforeAction);

	switch (stateBeforeAction)
	{
	case screenOff:
		// this should not happen because when we start an action we are 
		// moved into screenOffActionActive
		break;

	case runningDisplayActive:
		setRunningDisplay();
		break;

	case menuMessage:
		activate_message_display();
		break;

	case menuDisplay:
		if (!menuStackIsEmpty())
		{
			Menu *menu = peekMenuStackTop();
			setMenuDisplay(menu->menuText, menu->activeMenuItem);
		}
		break;

	case readingNumber:
		activate_number_input_display();
		break;

	case popUpDisplaying:
		// this should never happen
		break;
	}

	menuState = stateBeforeAction;
}

void errorStop(String title, String text)
{
	startPopUpMessage(title, text);
	loop_lcd();
	while (true)
		delay(1);
}

MenuState menuStateAtTurnedOff;
int menuItemSelectedAtTurnedOff;

void turnOffDisplay()
{
	TRACELN("Display turning off");

	setClearDisplayPage();

	menuStateAtTurnedOff = menuState;

	switch (menuState)
	{
		// we don't need to do anything special for these states
	case screenOff:
	case runningDisplayActive:
	case menuMessage:
	case readingNumber:
	case stringSelection:
	case popUpDisplaying:
		break;

	case menuDisplay:
		// make a note of the menu items so that you can restore that
		// position when the screen is restored
		menuItemSelectedAtTurnedOff = getSelectedItem();
		break;
	}

	// set the screen state to off
	menuState = screenOff;
}

void turnOnDisplay()
{
	TRACELN("Display turning on");

	// first we need to sort out the current state
	// this will be either screenOff or screenOffActionActive

	menuState = menuStateAtTurnedOff ;

	millisecsAtLastUserInput = millis();

	switch (menuState)
	{
	case screenOff:
		break;

	case runningDisplayActive:
		setRunningDisplay();
		break;

	case menuMessage:
		// restart the message display delay so that we can
		// read the message when the display is reactivated
		messageDisplayStartTime = millis();
		activate_message_display();
		break;

	case menuDisplay:
		if (!menuStackIsEmpty())
		{
			Menu *menu = peekMenuStackTop();
			setMenuDisplay(menu->menuText, menu->activeMenuItem);
			set_selected_item(menuItemSelectedAtTurnedOff);
		}
		break;

	case readingNumber:
		activate_number_input_display();
		break;

	case stringSelection:
		activeStringSelectionDisplay();
		break;

	case popUpDisplaying:
		activate_action_display();
		break;
	}
}

void keepDisplayAlive()
{
	millisecsAtLastUserInput = millis();
}

void menuUpButtonPresssed()
{
	switch (menuState)
	{
	case screenOff:
		turnOnDisplay();
		break;
	case runningDisplayActive:
		break;
	case menuMessage:
		break;
	case menuDisplay:
		move_selector_up();
		break;
	case readingNumber:
		incrementNumberValue();
		break;
	case stringSelection:
		moveToPreviousStringSelection();
		break;
	case popUpDisplaying:
		// Up is presently disabled when performing an action
		break;
	}
	keepDisplayAlive();
}

void menuDownButtonPressed()
{
	switch (menuState)
	{
	case screenOff:
		turnOnDisplay();
		break;
	case runningDisplayActive:
		break;
	case menuMessage:
		break;
	case menuDisplay:
		move_selector_down();
		break;
	case readingNumber:
		decrementNumberValue();
		break;
	case stringSelection:
		advanceToNextStringSelection();
		break;
	case popUpDisplaying:
		// Down is presently disabled when performing an action
		break;
	}
	keepDisplayAlive();
}

void menuSelectButtonPressed()
{
	switch (menuState)
	{
	case screenOff:
		turnOnDisplay();
		break;
	case runningDisplayActive:
		// Going into menu mode at the top level -
		enterAMenu(&mainMenu);
		break;
	case menuMessage:
		break;
	case readingNumber:
		completeNumberInput();
		break;
	case stringSelection:
		completeStringSelection();
		break;
	case popUpDisplaying:
		// Enter is presently disabled when performing an action
		break;
	case menuDisplay:
		uint8_t selected_item = getSelectedItem();
		TRACE("selected:");
		TRACELN(selected_item);
		Menu *menu = peekMenuStackTop();
		TRACELN(menu->menuText);
		menu->methods[selected_item]();
		break;
	}
	keepDisplayAlive();
}

// called if any of the display options change
// - only presently used by the remote commands that
// can download splash screen text that is displayed
// in workingStateActive

void refreshDisplay()
{
	switch (menuState)
	{
	case screenOff:
		break;
	case runningDisplayActive:
		setWorkingDisplay();
		break;
	case menuMessage:
		break;
	case menuDisplay:
		break;
	case readingNumber:
		break;
	case stringSelection:
		break;
	case popUpDisplaying:
		break;
	}
}

void checkPopUpTimeout()
{
	ulong time = millis();

	ulong diff = ulongDiff(time, popUpDisplayStartTime);

	if (diff > popUpDisplayTimeInMillis)
	{
		// popup has expired
		endPopupDisplay();
	}
}

void setup_menu()
{
	millisecsAtLastUserInput = millis();

	setRunningDisplay();
}

void loop_menu()
{
	if (menuState!=screenOff)
	{
		if ((millis() - millisecsAtLastUserInput) > displayTimeoutInMillis)
		{
			turnOffDisplay();
		}
	}

	switch (menuState)
	{
	case screenOff:
		break;
	case runningDisplayActive:
		break;
	case menuMessage:
		checkMessageTimeout();
		break;
	case menuDisplay:
		break;
	case readingNumber:
		break;
	case stringSelection:
		break;
	case popUpDisplaying:
		checkPopUpTimeout();
		break;
	}
}
