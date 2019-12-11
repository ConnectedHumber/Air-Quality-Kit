# 1 "c:\\Users\\Rob\\Documents\\GitHub\\Air-Quality-Kit\\Firmware\\Sources\\Heltec Cube\\LoraWanTest\\LoraWanTest.ino"
# 2 "c:\\Users\\Rob\\Documents\\GitHub\\Air-Quality-Kit\\Firmware\\Sources\\Heltec Cube\\LoraWanTest\\LoraWanTest.ino" 2
# 3 "c:\\Users\\Rob\\Documents\\GitHub\\Air-Quality-Kit\\Firmware\\Sources\\Heltec Cube\\LoraWanTest\\LoraWanTest.ino" 2
# 4 "c:\\Users\\Rob\\Documents\\GitHub\\Air-Quality-Kit\\Firmware\\Sources\\Heltec Cube\\LoraWanTest\\LoraWanTest.ino" 2

/*

 * set LoraWan_RGB to Active,the RGB active in loraWan

 * RGB red means sending;

 * RGB purple means joined done;

 * RGB blue means RxWindow1;

 * RGB yellow means RxWindow2;

 * RGB green means received done;

 */
# 17 "c:\\Users\\Rob\\Documents\\GitHub\\Air-Quality-Kit\\Firmware\\Sources\\Heltec Cube\\LoraWanTest\\LoraWanTest.ino"
/*LoraWan Class*/
DeviceClass_t CLASS=CLASS_A;
/*OTAA or ABP*/

# 20 "c:\\Users\\Rob\\Documents\\GitHub\\Air-Quality-Kit\\Firmware\\Sources\\Heltec Cube\\LoraWanTest\\LoraWanTest.ino" 3 4
bool 
# 20 "c:\\Users\\Rob\\Documents\\GitHub\\Air-Quality-Kit\\Firmware\\Sources\\Heltec Cube\\LoraWanTest\\LoraWanTest.ino"
    OVER_THE_AIR_ACTIVATION = 
# 20 "c:\\Users\\Rob\\Documents\\GitHub\\Air-Quality-Kit\\Firmware\\Sources\\Heltec Cube\\LoraWanTest\\LoraWanTest.ino" 3 4
                              false
# 20 "c:\\Users\\Rob\\Documents\\GitHub\\Air-Quality-Kit\\Firmware\\Sources\\Heltec Cube\\LoraWanTest\\LoraWanTest.ino"
                                             ;
/*ADR enable*/

# 22 "c:\\Users\\Rob\\Documents\\GitHub\\Air-Quality-Kit\\Firmware\\Sources\\Heltec Cube\\LoraWanTest\\LoraWanTest.ino" 3 4
bool 
# 22 "c:\\Users\\Rob\\Documents\\GitHub\\Air-Quality-Kit\\Firmware\\Sources\\Heltec Cube\\LoraWanTest\\LoraWanTest.ino"
    LORAWAN_ADR_ON = 
# 22 "c:\\Users\\Rob\\Documents\\GitHub\\Air-Quality-Kit\\Firmware\\Sources\\Heltec Cube\\LoraWanTest\\LoraWanTest.ino" 3 4
                     true
# 22 "c:\\Users\\Rob\\Documents\\GitHub\\Air-Quality-Kit\\Firmware\\Sources\\Heltec Cube\\LoraWanTest\\LoraWanTest.ino"
                                ;
/* set LORAWAN_Net_Reserve ON, the node could save the network info to flash, when node reset not need to join again */

# 24 "c:\\Users\\Rob\\Documents\\GitHub\\Air-Quality-Kit\\Firmware\\Sources\\Heltec Cube\\LoraWanTest\\LoraWanTest.ino" 3 4
bool 
# 24 "c:\\Users\\Rob\\Documents\\GitHub\\Air-Quality-Kit\\Firmware\\Sources\\Heltec Cube\\LoraWanTest\\LoraWanTest.ino"
    KeepNet = 
# 24 "c:\\Users\\Rob\\Documents\\GitHub\\Air-Quality-Kit\\Firmware\\Sources\\Heltec Cube\\LoraWanTest\\LoraWanTest.ino" 3 4
              true
# 24 "c:\\Users\\Rob\\Documents\\GitHub\\Air-Quality-Kit\\Firmware\\Sources\\Heltec Cube\\LoraWanTest\\LoraWanTest.ino"
                                 ;
/*LoraWan REGION*/
LoRaMacRegion_t REGION = LORAMAC_REGION_EU868;

/* Indicates if the node is sending confirmed or unconfirmed messages */

# 29 "c:\\Users\\Rob\\Documents\\GitHub\\Air-Quality-Kit\\Firmware\\Sources\\Heltec Cube\\LoraWanTest\\LoraWanTest.ino" 3 4
bool 
# 29 "c:\\Users\\Rob\\Documents\\GitHub\\Air-Quality-Kit\\Firmware\\Sources\\Heltec Cube\\LoraWanTest\\LoraWanTest.ino"
    IsTxConfirmed = 
# 29 "c:\\Users\\Rob\\Documents\\GitHub\\Air-Quality-Kit\\Firmware\\Sources\\Heltec Cube\\LoraWanTest\\LoraWanTest.ino" 3 4
                    true
# 29 "c:\\Users\\Rob\\Documents\\GitHub\\Air-Quality-Kit\\Firmware\\Sources\\Heltec Cube\\LoraWanTest\\LoraWanTest.ino"
                        ;
/*!

* Number of trials to transmit the frame, if the LoRaMAC layer did not

* receive an acknowledgment. The MAC performs a datarate adaptation,

* according to the LoRaWAN Specification V1.0.2, chapter 18.4, according

* to the following table:

*

* Transmission nb | Data Rate

* ----------------|-----------

* 1 (first)       | DR

* 2               | DR

* 3               | max(DR-1,0)

* 4               | max(DR-1,0)

* 5               | max(DR-2,0)

* 6               | max(DR-2,0)

* 7               | max(DR-3,0)

* 8               | max(DR-3,0)

*

* Note, that if NbTrials is set to 1 or 2, the MAC will not decrease

* the datarate, in case the LoRaMAC layer did not receive an acknowledgment

*/
# 50 "c:\\Users\\Rob\\Documents\\GitHub\\Air-Quality-Kit\\Firmware\\Sources\\Heltec Cube\\LoraWanTest\\LoraWanTest.ino"
uint8_t ConfirmedNbTrials = 8;

/* Application port */
uint8_t AppPort = 2;

/*the application data transmission duty cycle.  value in [ms].*/
uint32_t APP_TX_DUTYCYCLE = 15000;

BME280 bme280;

void bmeDump()
{
 while(
# 62 "c:\\Users\\Rob\\Documents\\GitHub\\Air-Quality-Kit\\Firmware\\Sources\\Heltec Cube\\LoraWanTest\\LoraWanTest.ino" 3 4
      true
# 62 "c:\\Users\\Rob\\Documents\\GitHub\\Air-Quality-Kit\\Firmware\\Sources\\Heltec Cube\\LoraWanTest\\LoraWanTest.ino"
          )
 {
  float temp = bme280.getTemperature();
  float humidity = bme280.getHumidity();
  float pressure = bme280.getPressure();
  Serial.print("Temp: "); Serial.print(temp);
  Serial.print("  Humidity: "); Serial.print(humidity);
  Serial.print("  Pressure: "); Serial.print(pressure);
  Serial.println();
  delay(500);
 }
}






int pms5003Len = 0;
int pms5003Pm10Serial = 0;
int pms5003Pm25Serial = 0;
unsigned int pms5003CalcChecksum;
unsigned int pms5003RecChecksum;

struct AirQualitySettings {
 int airqNoOfAverages;
};

AirQualitySettings airqualitySettings = {5};

struct AirqualityReading {
 float pm25;
 float pm10;
 unsigned long lastAirqAverageMillis;
 int airAverageReadingCount;
 int airNoOfAveragesCalculated;
 float pm25Average;
 float pm10Average;
 // these are temporary values that are not for public use
 float pm25AvgTotal;
 float pm10AvgTotal;
 int averageCount;
 int readings;
 int errors;
};

AirqualityReading airqualityReading;

void resetAirqAverages(struct AirqualityReading* reading)
{
 reading->pm10AvgTotal = 0;
 reading->pm25AvgTotal = 0;
 reading->averageCount = 0;
}


# 117 "c:\\Users\\Rob\\Documents\\GitHub\\Air-Quality-Kit\\Firmware\\Sources\\Heltec Cube\\LoraWanTest\\LoraWanTest.ino" 3 4
bool 
# 117 "c:\\Users\\Rob\\Documents\\GitHub\\Air-Quality-Kit\\Firmware\\Sources\\Heltec Cube\\LoraWanTest\\LoraWanTest.ino"
    updateAirqAverages(struct AirqualityReading* reading)
{

 reading->pm10AvgTotal += reading->pm10;
 reading->pm25AvgTotal += reading->pm25;

 reading->averageCount++;

 if (reading->averageCount == airqualitySettings.airqNoOfAverages)
 {
  reading->pm10Average = reading->pm10AvgTotal / airqualitySettings.airqNoOfAverages;
  reading->pm25Average = reading->pm25AvgTotal / airqualitySettings.airqNoOfAverages;
  reading->lastAirqAverageMillis = millis();
  reading->airNoOfAveragesCalculated++;
  resetAirqAverages(reading);
  return 
# 132 "c:\\Users\\Rob\\Documents\\GitHub\\Air-Quality-Kit\\Firmware\\Sources\\Heltec Cube\\LoraWanTest\\LoraWanTest.ino" 3 4
        true
# 132 "c:\\Users\\Rob\\Documents\\GitHub\\Air-Quality-Kit\\Firmware\\Sources\\Heltec Cube\\LoraWanTest\\LoraWanTest.ino"
            ;
 }
 return 
# 134 "c:\\Users\\Rob\\Documents\\GitHub\\Air-Quality-Kit\\Firmware\\Sources\\Heltec Cube\\LoraWanTest\\LoraWanTest.ino" 3 4
       false
# 134 "c:\\Users\\Rob\\Documents\\GitHub\\Air-Quality-Kit\\Firmware\\Sources\\Heltec Cube\\LoraWanTest\\LoraWanTest.ino"
            ;
}


boolean pumppms5003Byte(struct AirqualityReading* result, byte pms5003Value)
{
 switch (pms5003Len) {
 case (0): if (pms5003Value != 0x42) { pms5003Len = -1; }; pms5003CalcChecksum = 0x42; break;
 case (1): if (pms5003Value != 0x4d) { pms5003Len = -1; }; pms5003CalcChecksum += pms5003Value; break;
 case (2): /* Frame Length High:*/ if (pms5003Value != 0) { pms5003Len = -1; }; pms5003CalcChecksum += pms5003Value; break;
 case (3): /* Frame Length Low:*/ if (pms5003Value != 0x1c) { pms5003Len = -1; }; pms5003CalcChecksum += pms5003Value; break;
 case (4): /* PM1 standard particle High:*/ pms5003CalcChecksum += pms5003Value; break;
 case (5): /* PM1 standard particle Low:*/ pms5003CalcChecksum += pms5003Value; break;
 case (6): /* PM25 standard particle High:*/ pms5003CalcChecksum += pms5003Value; break;
 case (7): /* PM25 standard particle Low:*/ pms5003CalcChecksum += pms5003Value; break;
 case (8): /* PM10 standard particle High:*/ pms5003CalcChecksum += pms5003Value; break;
 case (9): /* PM10 standard particle Low:*/ pms5003CalcChecksum += pms5003Value; break;
 case (10): /* PM10 mgram/m3 High:*/ pms5003Pm10Serial = pms5003Value; pms5003CalcChecksum += pms5003Value; break;
 case (11): /* PM10 mgram/m3 Low:*/ pms5003Pm10Serial = (pms5003Pm10Serial << 8) + pms5003Value; pms5003CalcChecksum += pms5003Value; break;
 case (12): /* PM25 mgram/m3 High:*/ pms5003Pm25Serial = pms5003Value; pms5003CalcChecksum += pms5003Value; break;
 case (13): /* PM25 mgram/m3 Low:*/ pms5003Pm25Serial = (pms5003Pm25Serial << 8) + pms5003Value; pms5003CalcChecksum += pms5003Value; break;
 case (14): /* Cocentration Unit High:*/ pms5003CalcChecksum += pms5003Value; break;
 case (15): /* Concentration Unit Low:*/ pms5003CalcChecksum += pms5003Value; break;
 case (16): /* Particles>0.3um in 1 li High:*/ pms5003CalcChecksum += pms5003Value; break;
 case (17): /* Particles>0.3um in 1 li Low:*/ pms5003CalcChecksum += pms5003Value; break;
 case (18): /* Particles>0.5um in 1 li High:*/ pms5003CalcChecksum += pms5003Value; break;
 case (19): /* Particles>0.5um in 1 li Low:*/ pms5003CalcChecksum += pms5003Value; break;
 case (20): /* Particles>1.0um in 1 li High:*/ pms5003CalcChecksum += pms5003Value; break;
 case (21): /* Particles>1.0um in 1 li Low:*/ pms5003CalcChecksum += pms5003Value; break;
 case (22): /* Particles>2.5um in 1 li High:*/ pms5003CalcChecksum += pms5003Value; break;
 case (23): /* Particles>2.5um in 1 li Low:*/ pms5003CalcChecksum += pms5003Value; break;
 case (24): /* Particles>5.0um in 1 li High:*/ pms5003CalcChecksum += pms5003Value; break;
 case (25): /* Particles>5.0um in 1 li Low:*/ pms5003CalcChecksum += pms5003Value; break;
 case (26): /* Particles>10.0um in 1 li High:*/ pms5003CalcChecksum += pms5003Value; break;
 case (27): /* Particles>10.0um in 1 li Low:*/ pms5003CalcChecksum += pms5003Value; break;
 case (28): /* Reserved High:*/ pms5003CalcChecksum += pms5003Value; break;
 case (29): /* Reserved Low:*/ pms5003CalcChecksum += pms5003Value; break;
 case (30): /* Check High:*/ pms5003RecChecksum = pms5003Value; break;
 case (31): /* Check Low:*/
  pms5003RecChecksum = (pms5003RecChecksum << 8) + pms5003Value; break;
 }

 pms5003Len++;

 if (pms5003Len == 32)
 {
  pms5003Len = 0;

  if (pms5003RecChecksum == pms5003CalcChecksum & 0xFFFF)
  {
   result->pm10 = pms5003Pm10Serial;
   result->pm25 = pms5003Pm25Serial;
   result->readings++;
   Serial.println("Got a reading");
   return updateAirqAverages(result);
  }
  else
  {
   result->errors++;
  }
 }
 return 
# 195 "c:\\Users\\Rob\\Documents\\GitHub\\Air-Quality-Kit\\Firmware\\Sources\\Heltec Cube\\LoraWanTest\\LoraWanTest.ino" 3 4
       false
# 195 "c:\\Users\\Rob\\Documents\\GitHub\\Air-Quality-Kit\\Firmware\\Sources\\Heltec Cube\\LoraWanTest\\LoraWanTest.ino"
            ;
}

// flashes the LED green when an average is calculated
void airQDump()
{
 resetAirqAverages(&airqualityReading);

 while(
# 203 "c:\\Users\\Rob\\Documents\\GitHub\\Air-Quality-Kit\\Firmware\\Sources\\Heltec Cube\\LoraWanTest\\LoraWanTest.ino" 3 4
      true
# 203 "c:\\Users\\Rob\\Documents\\GitHub\\Air-Quality-Kit\\Firmware\\Sources\\Heltec Cube\\LoraWanTest\\LoraWanTest.ino"
          )
 {
  if(Serial.available())
  {
   int ch;
   ch = Serial.read();
   boolean gotAverage = pumppms5003Byte(&airqualityReading, (byte) ch);
   if(gotAverage)
   {
    // Serial.print("PM10: ");
    // Serial.print(airqualityReading.pm10Average);
    // Serial.print("PM25: ");
    // Serial.print(airqualityReading.pm25Average);
    // Serial.println();

    RGB_ON(0x005000 /*color green, light 0x10*/,5);
    RGB_OFF();

   }
  }

  delay(1);
 }
}

unsigned long ulongDiff(unsigned long end, unsigned long start)
{
 if (end >= start)
 {
  return end - start;
 }
 else
 {
  return 
# 236 "c:\\Users\\Rob\\Documents\\GitHub\\Air-Quality-Kit\\Firmware\\Sources\\Heltec Cube\\LoraWanTest\\LoraWanTest.ino" 3 4
        (0x7fffffffL * 2UL + 1UL) 
# 236 "c:\\Users\\Rob\\Documents\\GitHub\\Air-Quality-Kit\\Firmware\\Sources\\Heltec Cube\\LoraWanTest\\LoraWanTest.ino"
                  - start + end + 1;
 }
}


# 240 "c:\\Users\\Rob\\Documents\\GitHub\\Air-Quality-Kit\\Firmware\\Sources\\Heltec Cube\\LoraWanTest\\LoraWanTest.ino" 3 4
bool 
# 240 "c:\\Users\\Rob\\Documents\\GitHub\\Air-Quality-Kit\\Firmware\\Sources\\Heltec Cube\\LoraWanTest\\LoraWanTest.ino"
    getAirQReading(struct AirqualityReading * reading, unsigned long timeout)
{
 unsigned long startMillis = millis();

 while(
# 244 "c:\\Users\\Rob\\Documents\\GitHub\\Air-Quality-Kit\\Firmware\\Sources\\Heltec Cube\\LoraWanTest\\LoraWanTest.ino" 3 4
      true
# 244 "c:\\Users\\Rob\\Documents\\GitHub\\Air-Quality-Kit\\Firmware\\Sources\\Heltec Cube\\LoraWanTest\\LoraWanTest.ino"
          )
 {
  if(Serial.available())
  {
   int ch;
   ch = Serial.read();
   boolean gotAverage = pumppms5003Byte(reading, (byte) ch);
   if(gotAverage)
   {
    return 
# 253 "c:\\Users\\Rob\\Documents\\GitHub\\Air-Quality-Kit\\Firmware\\Sources\\Heltec Cube\\LoraWanTest\\LoraWanTest.ino" 3 4
          true
# 253 "c:\\Users\\Rob\\Documents\\GitHub\\Air-Quality-Kit\\Firmware\\Sources\\Heltec Cube\\LoraWanTest\\LoraWanTest.ino"
              ;
   }
  }

  delay(1);

  if(ulongDiff(millis(), startMillis)> timeout) return 
# 259 "c:\\Users\\Rob\\Documents\\GitHub\\Air-Quality-Kit\\Firmware\\Sources\\Heltec Cube\\LoraWanTest\\LoraWanTest.ino" 3 4
                                                      false
# 259 "c:\\Users\\Rob\\Documents\\GitHub\\Air-Quality-Kit\\Firmware\\Sources\\Heltec Cube\\LoraWanTest\\LoraWanTest.ino"
                                                           ;
 }
}

void flushSerialBufferInput()
{
 while(Serial.available()) Serial.read();
}

void buildBuffer(byte * buffer, float pub_avg_ppm_25, float pub_avg_ppm_10, float temperature, float pascal, float humidity)
{
 uint16_t t_value, p_value, s_value;
 uint8_t h_value;
 temperature = ((temperature)<(-24)?(-24):((temperature)>(40)?(40):(temperature))); //temp in range -24 to 40 (64 steps)
 pascal = ((pascal)<(970)?(970):((pascal)>(1034)?(1034):(pascal))); //pressure in range 970 to 1034 (64 steps)*/
 t_value = uint16_t((temperature * (100 / 6.25) + 2400 / 6.25)); //0.0625 degree steps with offset
 p_value = uint16_t((pascal - 970) / 1); //1 mbar steps, offset 970.
 s_value = (p_value << 10) + t_value; // putting the bits in the right place
 h_value = uint8_t(humidity);
 buffer[0] = s_value & 0xFF; //lower byte
 buffer[1] = s_value >> 8; //higher byte
 buffer[2] = h_value;
 buffer[3] = (int)(pub_avg_ppm_25 + 0.5f); // just send the sensor values as int readings
 buffer[4] = (int)(pub_avg_ppm_10 + 0.5f);
}




/* Prepares the payload of the frame */
static void PrepareTxFrame( uint8_t port )
{
    AppDataSize = 5;//AppDataSize max value is 64


 RGB_ON(0x000050 /*color blue, light 0x10*/,0);


 pinMode(P3_2 /*gpio6*/,OUTPUT);
 digitalWrite(P3_2 /*gpio6*/,LOW); //turn sensor on

 delay(5000); // wait 30 seconds


 Serial.println("power on");
 RGB_ON(0x505000 /*color yellow, light 0x10*/,0);


 flushSerialBufferInput();

 // start the reading
 resetAirqAverages(&airqualityReading);

 Serial.println("starting reading");

 if(getAirQReading(&airqualityReading, 5000))
 {
  float temp = bme280.getTemperature();
  float humidity = bme280.getHumidity();
  float pressure = bme280.getPressure() / 100 ;
  buildBuffer(AppData, airqualityReading.pm25Average, airqualityReading.pm10Average, temp, pressure, humidity);

  RGB_ON(0x005000 /*color green, light 0x10*/,5);
  RGB_OFF();

 }
 else{
  Serial.println("Reading timeout");
  AppData[0]=1;
  AppData[1]=2;
  AppData[2]=3;
  AppData[3]=4;
  AppData[4]=5;

  RGB_ON(0x500000 /*color red, light */,5);
  RGB_OFF();

 }

 pinMode(P3_2 /*gpio6*/,OUTPUT);
 digitalWrite(P3_2 /*gpio6*/,LOW); //turn sensor off

}



void setup() {
    BoardInitMcu();
    Serial.begin(9600);

 delay(500);

 Serial.println("Board Starting");

 if(bme280.init())
 {
  Serial.print("Got BME280");
 }



//    Enable_AT();

    DeviceState = DEVICE_STATE_INIT;
    LoRaWAN.Ifskipjoin();
}

void loop()
{
 switch( DeviceState )
 {
  case DEVICE_STATE_INIT:
  {

      getDevParam();

   printDevParam();
   Serial.printf("LoRaWan Class%X  start! \r\n",CLASS+10);
   LoRaWAN.Init(CLASS,REGION);
   DeviceState = DEVICE_STATE_JOIN;
   break;
  }
  case DEVICE_STATE_JOIN:
  {
   LoRaWAN.Join();
   break;
  }
  case DEVICE_STATE_SEND:
  {
   PrepareTxFrame( AppPort );
   LoRaWAN.Send();
   DeviceState = DEVICE_STATE_CYCLE;
   break;
  }
  case DEVICE_STATE_CYCLE:
  {
   // Schedule next packet transmission
   TxDutyCycleTime = APP_TX_DUTYCYCLE + randr( 0, 1000 );
   LoRaWAN.Cycle(TxDutyCycleTime);
   DeviceState = DEVICE_STATE_SLEEP;
   break;
  }
  case DEVICE_STATE_SLEEP:
  {
   LoRaWAN.Sleep();
   break;
  }
  default:
  {
   DeviceState = DEVICE_STATE_INIT;
   break;
  }
 }
}
