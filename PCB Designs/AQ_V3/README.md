# AQ_V3 PCB design

These are KiCAD files for the PCB design version 3. Historically V1 was based on a WeMOS device, V2 was based on a Heltec to enable the use of LoRa but had a track error.

V3 also incorporates a high side switch to allow the 5V to the SDS011 sensor to be turned off when the Vext is turned off. The GPS option (3V3) is also powered from the Vext. A large capacitor has been added to the 5V input to the Heltec to prevent resets caused by the SDS011 switching on.
