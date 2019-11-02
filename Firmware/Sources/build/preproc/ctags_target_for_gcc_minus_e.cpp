# 1 "c:\\Users\\Rob\\Documents\\GitHub\\Air-Quality-Kit\\Firmware\\Sources\\SensorReader\\SensorReader.ino"
# 2 "c:\\Users\\Rob\\Documents\\GitHub\\Air-Quality-Kit\\Firmware\\Sources\\SensorReader\\SensorReader.ino" 2

# 4 "c:\\Users\\Rob\\Documents\\GitHub\\Air-Quality-Kit\\Firmware\\Sources\\SensorReader\\SensorReader.ino" 2
# 5 "c:\\Users\\Rob\\Documents\\GitHub\\Air-Quality-Kit\\Firmware\\Sources\\SensorReader\\SensorReader.ino" 2

# 7 "c:\\Users\\Rob\\Documents\\GitHub\\Air-Quality-Kit\\Firmware\\Sources\\SensorReader\\SensorReader.ino" 2
# 8 "c:\\Users\\Rob\\Documents\\GitHub\\Air-Quality-Kit\\Firmware\\Sources\\SensorReader\\SensorReader.ino" 2
# 9 "c:\\Users\\Rob\\Documents\\GitHub\\Air-Quality-Kit\\Firmware\\Sources\\SensorReader\\SensorReader.ino" 2
# 10 "c:\\Users\\Rob\\Documents\\GitHub\\Air-Quality-Kit\\Firmware\\Sources\\SensorReader\\SensorReader.ino" 2
# 11 "c:\\Users\\Rob\\Documents\\GitHub\\Air-Quality-Kit\\Firmware\\Sources\\SensorReader\\SensorReader.ino" 2

# 13 "c:\\Users\\Rob\\Documents\\GitHub\\Air-Quality-Kit\\Firmware\\Sources\\SensorReader\\SensorReader.ino" 2
//#define DEBUG
//#define TEST_LORA
# 25 "c:\\Users\\Rob\\Documents\\GitHub\\Air-Quality-Kit\\Firmware\\Sources\\SensorReader\\SensorReader.ino"
#define TRACE(s) 
#define TRACE_HEX(s) 
#define TRACELN(s) 
#define TRACE_HEXLN(s) 



# 33 "c:\\Users\\Rob\\Documents\\GitHub\\Air-Quality-Kit\\Firmware\\Sources\\SensorReader\\SensorReader.ino" 2
# 34 "c:\\Users\\Rob\\Documents\\GitHub\\Air-Quality-Kit\\Firmware\\Sources\\SensorReader\\SensorReader.ino" 2


boolean send_to_lora();
boolean send_to_mqtt();

# 40 "c:\\Users\\Rob\\Documents\\GitHub\\Air-Quality-Kit\\Firmware\\Sources\\SensorReader\\SensorReader.ino" 2
# 41 "c:\\Users\\Rob\\Documents\\GitHub\\Air-Quality-Kit\\Firmware\\Sources\\SensorReader\\SensorReader.ino" 2
# 42 "c:\\Users\\Rob\\Documents\\GitHub\\Air-Quality-Kit\\Firmware\\Sources\\SensorReader\\SensorReader.ino" 2
# 43 "c:\\Users\\Rob\\Documents\\GitHub\\Air-Quality-Kit\\Firmware\\Sources\\SensorReader\\SensorReader.ino" 2
# 44 "c:\\Users\\Rob\\Documents\\GitHub\\Air-Quality-Kit\\Firmware\\Sources\\SensorReader\\SensorReader.ino" 2
# 45 "c:\\Users\\Rob\\Documents\\GitHub\\Air-Quality-Kit\\Firmware\\Sources\\SensorReader\\SensorReader.ino" 2
# 46 "c:\\Users\\Rob\\Documents\\GitHub\\Air-Quality-Kit\\Firmware\\Sources\\SensorReader\\SensorReader.ino" 2
# 47 "c:\\Users\\Rob\\Documents\\GitHub\\Air-Quality-Kit\\Firmware\\Sources\\SensorReader\\SensorReader.ino" 2
# 48 "c:\\Users\\Rob\\Documents\\GitHub\\Air-Quality-Kit\\Firmware\\Sources\\SensorReader\\SensorReader.ino" 2
# 49 "c:\\Users\\Rob\\Documents\\GitHub\\Air-Quality-Kit\\Firmware\\Sources\\SensorReader\\SensorReader.ino" 2
# 50 "c:\\Users\\Rob\\Documents\\GitHub\\Air-Quality-Kit\\Firmware\\Sources\\SensorReader\\SensorReader.ino" 2
# 51 "c:\\Users\\Rob\\Documents\\GitHub\\Air-Quality-Kit\\Firmware\\Sources\\SensorReader\\SensorReader.ino" 2

//Function that prints the reason by which ESP32 has been awaken from sleep
void print_wakeup_reason(){
 esp_sleep_wakeup_cause_t wakeup_reason;
 wakeup_reason = esp_sleep_get_wakeup_cause();
 switch(wakeup_reason)
 {
  case 1 : Serial.println("Wakeup caused by external signal using RTC_IO"); break;
  case 2 : Serial.println("Wakeup caused by external signal using RTC_CNTL"); break;
  case 3 : Serial.println("Wakeup caused by timer"); break;
  case 4 : Serial.println("Wakeup caused by touchpad"); break;
  case 5 : Serial.println("Wakeup caused by ULP program"); break;
  default : Serial.println("Wakeup was not caused by deep sleep"); break;
 }
}


void setup() {
 // put your setup code here, to run once:
 Serial.begin(115200);
 Serial.println("Starting...");
 print_wakeup_reason();

 // turn on the 3.3 volt rail
 // drop it to reset any I2C devices

 pinMode(21, 0x02);
 digitalWrite(21, 0x0);
 delay(500);
 digitalWrite(21, 0x1);
 delay(500);

 uint64_t chipid = ESP.getEfuseMac();//The chip ID is essentially its MAC address(length: 6 bytes).
 Serial.printf("ESP32 Chip ID = %04X", (uint16_t)(chipid >> 32));//print High 2 bytes
 Serial.printf("%08X\n", (uint32_t)chipid);//print Low 4bytes.
 setup_lcd();
 setup_bme280();
 setup_sensor();
 setup_input();
 if(settings.rtcOn) setup_rtc();
 setup_menu();
 setup_commands();
 setup_timing();
 if(settings.loraOn) setup_lora();
 if(settings.wiFiOn) setup_wifi();
 if(settings.mqttOn) setup_mqtt();
 if(settings.gpsOn) setup_gps();

}

void loop()
{
 loop_sensor();
 loop_input();
 loop_bme280();
 if (settings.rtcOn) loop_rtc();
 loop_menu();
 loop_timing();
 if (settings.wiFiOn) loop_wifi();
 if (settings.mqttOn) loop_mqtt();
 if (settings.loraOn) loop_lora();
 loop_commands();
 if (settings.gpsOn) loop_gps();
 // update the display last of all
 loop_lcd();
 delay(1); /// always delay at least 1msec
}
