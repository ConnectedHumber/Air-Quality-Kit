/***************************************************************************
  This is a library for the BME280 humidity, temperature & pressure sensor

  Designed specifically to work with the Adafruit BME280 Breakout
  ----> http://www.adafruit.com/products/2650

  These sensors use I2C or SPI to communicate, 2 or 4 pins are required
  to interface. The device's I2C address is either 0x76 or 0x77.

  Adafruit invests time and resources providing this open source code,
  please support Adafruit andopen-source hardware by purchasing products
  from Adafruit!

  Written by Limor Fried & Kevin Townsend for Adafruit Industries.
  BSD license, all text above must be included in any redistribution
 ***************************************************************************/

#include <Wire.h>
#include <SPI.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_BME280.h>

#define BME_SCK 13
#define BME_MISO 12
#define BME_MOSI 11
#define BME_CS 10

#define SEALEVELPRESSURE_HPA (1013.25)

Adafruit_BME280 bme; // I2C

int bmeAddresses[] = {0x76, 0x77};

enum Bme280_sensor_state
{
  bme280_starting,
  bme280_active,
  bme280_connecting
};

Bme280_sensor_state bme280_sensor_state;

unsigned long delayTime;

void setup_bme280()
{
  bme280_sensor_state = bme280_starting;

  pub_bme_values_ready = false;

  TRACELN("Done setting up BME 280");
}

bool beginBME()
{
  for (int i = 0; i < sizeof(bmeAddresses) / sizeof(int); i++)
  {
    if (bme.begin(bmeAddresses[i]))
    {
      Serial.print("BME x80 found at addr 0x");
      Serial.println(bmeAddresses[i], HEX);
      Serial.print("SensorID was: 0x"); 
      Serial.println(bme.sensorID(),HEX);      
      return true;
    }
  }
  return false;
}

void printBMEValues()
{
  TRACE("Temperature = ");
  TRACE(bme.readTemperature());
  TRACELN(" *C");

  TRACE("Pressure = ");

  TRACE(bme.readPressure() / 100.0F);
  TRACELN(" hPa");

  TRACE("Approx. Altitude = ");
  TRACE(bme.readAltitude(SEALEVELPRESSURE_HPA));
  TRACELN(" m");

  TRACE("Humidity = ");
  TRACE(bme.readHumidity());
  TRACELN(" %");

  TRACELN();
}

void loop_bme280()
{
  switch (bme280_sensor_state)
  {
  case bme280_starting:
    if (beginBME())
    {
      bme280_sensor_state = bme280_active;
    }
    else
    {
      Serial.println("Unable to start BMEx80 sensor");
      bme280_sensor_state = bme280_connecting;
    }
    break;

  case bme280_connecting:
    if (beginBME())
    {
      bme280_sensor_state = bme280_active;
    }
    break;

  case bme280_active:
    pub_temp = bme.readTemperature();
    if (isnan(pub_temp))
    {
      Serial.println("Invalid value from BMEx80 sensor");
      bme280_sensor_state = bme280_connecting;
    }
    else
    {
      pub_pressure = bme.readPressure() / 100.0F;
      pub_humidity = bme.readHumidity();
      pub_bme_values_ready = true;
      pub_bme_values_reading_count++;
    }
    break;
  }
}
