# 1 "c:\\Users\\Rob\\Documents\\GitHub\\Air-Quality-Kit\\Firmware\\Sources\\SensorReaderV3\\SensorReaderV3.ino"
# 2 "c:\\Users\\Rob\\Documents\\GitHub\\Air-Quality-Kit\\Firmware\\Sources\\SensorReaderV3\\SensorReaderV3.ino" 2
# 3 "c:\\Users\\Rob\\Documents\\GitHub\\Air-Quality-Kit\\Firmware\\Sources\\SensorReaderV3\\SensorReaderV3.ino" 2

# 5 "c:\\Users\\Rob\\Documents\\GitHub\\Air-Quality-Kit\\Firmware\\Sources\\SensorReaderV3\\SensorReaderV3.ino" 2
# 6 "c:\\Users\\Rob\\Documents\\GitHub\\Air-Quality-Kit\\Firmware\\Sources\\SensorReaderV3\\SensorReaderV3.ino" 2
# 7 "c:\\Users\\Rob\\Documents\\GitHub\\Air-Quality-Kit\\Firmware\\Sources\\SensorReaderV3\\SensorReaderV3.ino" 2
# 8 "c:\\Users\\Rob\\Documents\\GitHub\\Air-Quality-Kit\\Firmware\\Sources\\SensorReaderV3\\SensorReaderV3.ino" 2
# 9 "c:\\Users\\Rob\\Documents\\GitHub\\Air-Quality-Kit\\Firmware\\Sources\\SensorReaderV3\\SensorReaderV3.ino" 2
# 10 "c:\\Users\\Rob\\Documents\\GitHub\\Air-Quality-Kit\\Firmware\\Sources\\SensorReaderV3\\SensorReaderV3.ino" 2
# 11 "c:\\Users\\Rob\\Documents\\GitHub\\Air-Quality-Kit\\Firmware\\Sources\\SensorReaderV3\\SensorReaderV3.ino" 2
# 12 "c:\\Users\\Rob\\Documents\\GitHub\\Air-Quality-Kit\\Firmware\\Sources\\SensorReaderV3\\SensorReaderV3.ino" 2
# 13 "c:\\Users\\Rob\\Documents\\GitHub\\Air-Quality-Kit\\Firmware\\Sources\\SensorReaderV3\\SensorReaderV3.ino" 2
# 14 "c:\\Users\\Rob\\Documents\\GitHub\\Air-Quality-Kit\\Firmware\\Sources\\SensorReaderV3\\SensorReaderV3.ino" 2

# 16 "c:\\Users\\Rob\\Documents\\GitHub\\Air-Quality-Kit\\Firmware\\Sources\\SensorReaderV3\\SensorReaderV3.ino" 2
# 17 "c:\\Users\\Rob\\Documents\\GitHub\\Air-Quality-Kit\\Firmware\\Sources\\SensorReaderV3\\SensorReaderV3.ino" 2
# 18 "c:\\Users\\Rob\\Documents\\GitHub\\Air-Quality-Kit\\Firmware\\Sources\\SensorReaderV3\\SensorReaderV3.ino" 2
# 19 "c:\\Users\\Rob\\Documents\\GitHub\\Air-Quality-Kit\\Firmware\\Sources\\SensorReaderV3\\SensorReaderV3.ino" 2
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

void setup() {
 Serial.begin(115200);
 delay(500);

 Serial.printf("Boot count: %d\n", bootCount);

 bootCount++;

 setupSettings();

 PrintSystemDetails();

 startDevice();

 Serial.printf("\n\nType help and press enter for help\n\n");
}

void loop()
{
 updateProcesses();
 updateSensors();
 delay(1);
}
