# 1 "c:\\Users\\Rob\\Documents\\GitHub\\Air-Quality-Kit\\Firmware\\Sources\\SensorReaderV3\\SensorReaderV3.ino"
# 2 "c:\\Users\\Rob\\Documents\\GitHub\\Air-Quality-Kit\\Firmware\\Sources\\SensorReaderV3\\SensorReaderV3.ino" 2

# 4 "c:\\Users\\Rob\\Documents\\GitHub\\Air-Quality-Kit\\Firmware\\Sources\\SensorReaderV3\\SensorReaderV3.ino" 2
# 5 "c:\\Users\\Rob\\Documents\\GitHub\\Air-Quality-Kit\\Firmware\\Sources\\SensorReaderV3\\SensorReaderV3.ino" 2
# 6 "c:\\Users\\Rob\\Documents\\GitHub\\Air-Quality-Kit\\Firmware\\Sources\\SensorReaderV3\\SensorReaderV3.ino" 2
# 7 "c:\\Users\\Rob\\Documents\\GitHub\\Air-Quality-Kit\\Firmware\\Sources\\SensorReaderV3\\SensorReaderV3.ino" 2

# 9 "c:\\Users\\Rob\\Documents\\GitHub\\Air-Quality-Kit\\Firmware\\Sources\\SensorReaderV3\\SensorReaderV3.ino" 2
# 10 "c:\\Users\\Rob\\Documents\\GitHub\\Air-Quality-Kit\\Firmware\\Sources\\SensorReaderV3\\SensorReaderV3.ino" 2
# 11 "c:\\Users\\Rob\\Documents\\GitHub\\Air-Quality-Kit\\Firmware\\Sources\\SensorReaderV3\\SensorReaderV3.ino" 2
# 12 "c:\\Users\\Rob\\Documents\\GitHub\\Air-Quality-Kit\\Firmware\\Sources\\SensorReaderV3\\SensorReaderV3.ino" 2
# 13 "c:\\Users\\Rob\\Documents\\GitHub\\Air-Quality-Kit\\Firmware\\Sources\\SensorReaderV3\\SensorReaderV3.ino" 2
# 14 "c:\\Users\\Rob\\Documents\\GitHub\\Air-Quality-Kit\\Firmware\\Sources\\SensorReaderV3\\SensorReaderV3.ino" 2
# 15 "c:\\Users\\Rob\\Documents\\GitHub\\Air-Quality-Kit\\Firmware\\Sources\\SensorReaderV3\\SensorReaderV3.ino" 2
# 16 "c:\\Users\\Rob\\Documents\\GitHub\\Air-Quality-Kit\\Firmware\\Sources\\SensorReaderV3\\SensorReaderV3.ino" 2
# 17 "c:\\Users\\Rob\\Documents\\GitHub\\Air-Quality-Kit\\Firmware\\Sources\\SensorReaderV3\\SensorReaderV3.ino" 2
# 18 "c:\\Users\\Rob\\Documents\\GitHub\\Air-Quality-Kit\\Firmware\\Sources\\SensorReaderV3\\SensorReaderV3.ino" 2

# 20 "c:\\Users\\Rob\\Documents\\GitHub\\Air-Quality-Kit\\Firmware\\Sources\\SensorReaderV3\\SensorReaderV3.ino" 2
# 21 "c:\\Users\\Rob\\Documents\\GitHub\\Air-Quality-Kit\\Firmware\\Sources\\SensorReaderV3\\SensorReaderV3.ino" 2
# 22 "c:\\Users\\Rob\\Documents\\GitHub\\Air-Quality-Kit\\Firmware\\Sources\\SensorReaderV3\\SensorReaderV3.ino" 2
# 23 "c:\\Users\\Rob\\Documents\\GitHub\\Air-Quality-Kit\\Firmware\\Sources\\SensorReaderV3\\SensorReaderV3.ino" 2
# 24 "c:\\Users\\Rob\\Documents\\GitHub\\Air-Quality-Kit\\Firmware\\Sources\\SensorReaderV3\\SensorReaderV3.ino" 2
# 25 "c:\\Users\\Rob\\Documents\\GitHub\\Air-Quality-Kit\\Firmware\\Sources\\SensorReaderV3\\SensorReaderV3.ino" 2
# 26 "c:\\Users\\Rob\\Documents\\GitHub\\Air-Quality-Kit\\Firmware\\Sources\\SensorReaderV3\\SensorReaderV3.ino" 2
# 27 "c:\\Users\\Rob\\Documents\\GitHub\\Air-Quality-Kit\\Firmware\\Sources\\SensorReaderV3\\SensorReaderV3.ino" 2
# 28 "c:\\Users\\Rob\\Documents\\GitHub\\Air-Quality-Kit\\Firmware\\Sources\\SensorReaderV3\\SensorReaderV3.ino" 2
# 29 "c:\\Users\\Rob\\Documents\\GitHub\\Air-Quality-Kit\\Firmware\\Sources\\SensorReaderV3\\SensorReaderV3.ino" 2
# 30 "c:\\Users\\Rob\\Documents\\GitHub\\Air-Quality-Kit\\Firmware\\Sources\\SensorReaderV3\\SensorReaderV3.ino" 2
# 31 "c:\\Users\\Rob\\Documents\\GitHub\\Air-Quality-Kit\\Firmware\\Sources\\SensorReaderV3\\SensorReaderV3.ino" 2
# 32 "c:\\Users\\Rob\\Documents\\GitHub\\Air-Quality-Kit\\Firmware\\Sources\\SensorReaderV3\\SensorReaderV3.ino" 2
# 33 "c:\\Users\\Rob\\Documents\\GitHub\\Air-Quality-Kit\\Firmware\\Sources\\SensorReaderV3\\SensorReaderV3.ino" 2
# 34 "c:\\Users\\Rob\\Documents\\GitHub\\Air-Quality-Kit\\Firmware\\Sources\\SensorReaderV3\\SensorReaderV3.ino" 2
# 35 "c:\\Users\\Rob\\Documents\\GitHub\\Air-Quality-Kit\\Firmware\\Sources\\SensorReaderV3\\SensorReaderV3.ino" 2
# 36 "c:\\Users\\Rob\\Documents\\GitHub\\Air-Quality-Kit\\Firmware\\Sources\\SensorReaderV3\\SensorReaderV3.ino" 2

void setup() {
 Serial.begin(115200);
 delay(500);
 setupSettings();
 Serial.printf("\nConnected Humber Sensor %s\nVersion %d.%d\n\n", settings.deviceName,
  3, 0);

 startDevice();

 Serial.printf("\n\nType help and press enter for help\n\n");
}

void loop()
{
 updateProcesses();
 updateSensors();
 sendSensorReadings();
 delay(1);
}
