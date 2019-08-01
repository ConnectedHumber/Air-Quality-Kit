# Sources

This is the Arduino IDE source code for the CH HumberLora Kit as written by CrazyRobMiles.

Install the SensorReader folder into your sketches folder then, using the Arduino IDE, open the SensorReader.ino file.

Select the Heltec WiFi Lora 32(V2) board.

You will also have to install some additional libraries:-
- microNMEA
- NeoPixelBus_by_Makuna
- PubSubClient

NOTE: the sources include a modified version of the LMIC library.

You can then build and install to the Heltec.

Once the board is programmed you should see a number of messages appearing on the OLED display.

To setup the board use the Arduino Serial Monitor set to 115200 baud. Type 'hello' into the input window and send it. The heltec will respond with an error message.

The commands to configure the firmware are in the document 'configuration commands.pdf'
