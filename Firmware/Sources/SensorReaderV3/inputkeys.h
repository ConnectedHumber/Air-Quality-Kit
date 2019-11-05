int startInputKeys(struct process * timingProcess);

int updateInputKeys(struct process * timingProcess);

int stopInputKeys(struct process * timingProcess);

void inputKeysStatusMessage(struct process * timingProcess, char * buffer, int bufferLength);

boolean bindKeyDownHandler(char * keyName, void(*handler)());

boolean bindKeyUpHandler(char * keyName, void(*handler)());
