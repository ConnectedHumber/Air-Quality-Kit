int startMenu(struct process * menuProcess);

int updateMenu(struct process * menuProcess);

int stopMenu(struct process * menuProcess);

void menuStatusMessage(struct process * menuProcess, char * buffer, int bufferLength);

void startPopUpMessage(String title, String text, int popUpTime);

void endPopupDisplay();

#define POPUP_DURATION_MILLIS 800
#define POPUP_NO_TIMEOUT 0