#ifndef CONTROL_H

#define CONTROL_H

void showDeviceStatus();
void startDevice();
void sendSensorReadings();
void forceMQTTSend();
void enterWiFiConfig();

void turn_sensor_power_off(int sleepTime);

void turn_sensor_power_on();

extern int bootCount;

#endif