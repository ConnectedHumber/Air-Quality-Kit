#include "HeltecMD_DS3231.h"
#include <Wire.h>

#define PRINTS(s)   Serial.print(F(s))
#define PRINT(s, v) { Serial.print(F(s)); Serial.print(v); }



MD_DS3231 RTC;

void setup() {
  Serial.begin(115200);
  delay(500);
}

void loop() {
  // put your main code here, to run repeatedly:
    RTC.readTime();
    Serial.println(RTC.s);
    delay(1000);
}