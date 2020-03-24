# Firebeetle based AQ monitoring PCB

As the name suggests this uses the Firebeetle device which is ESP32 based BUT when the USB is disconnected the serial circuitry is switched off by a low-side switch which allows it to achieve very low deep sleep power consumption with currents in the microamp range. 

![pcb](https://github.com/ConnectedHumber/Air-Quality-Kit/blob/master/PCB%20Designs/Firebeetle/images/FIREBEETLE_V3.0.jpg)

We added 2 high-side switches to turn the 3V3 and 5V peripherals off at the same time. The high-side switches also operate in the microamp region when they are off. They are controlled by GPIO21. 

When the Firebeetle runs from lithium battery the VCC is only the same as the battery, circa 3V9, so we added a buck dc-dc convertor to push this up to 5V for the SDS011 PM sensor. The buck will ensure that the 5V is present/steady even when the lithium battery voltage drops to 3V7. However, we expect the board to be powered from an external 5V DC supply. That would allow us to bolt on a Solar charger which could be bought off the shelf.

The optional RFM95 and OLED devices run from 3V3 so they can both be switched off when the Firebeetle goes into deep sleep.

The board supports both BME280 and BME680 devices and has a serial input for GPS data.

The board also supports NeoPixels powered from the switched 5V. Although you could have a lots of neopixels you need to remember the battery drain that will result so this is more likely to be used as a status indicator.
