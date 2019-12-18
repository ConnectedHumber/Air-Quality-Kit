#include "HeltecMD_DS3231.h"
#include <Wire.h>

#define PRINTS(s)   Serial.print(F(s))
#define PRINT(s, v) { Serial.print(F(s)); Serial.print(v); }



MD_DS3231 RTC;

void setup() {
  Serial.begin(115200);
  delay(500);
  RTC.setAlarm1Type(DS3231_ALM_SEC);
//  RTC.setAlarm2Type(DS3231_ALM_MIN);
}

void loop() {
//  if(RTC.checkAlarm2())
//    Serial.println("Alarm 2");

  if(RTC.checkAlarm1())
   Serial.println("Alarm 1");

  delay(5000);
}