
int getLoraSentCount();

boolean publishReadingsToLoRa (float pm10Average, float pm25Average, float temperature, float pressure, float humidity);

int startLoRa(struct process * loraProcess);

int updateLoRa(struct process * loraProcess);

int stopLoRa(struct process * loraProcess);

void loraStatusMessage(struct process * loraProcess, char * buffer, int bufferLength);


