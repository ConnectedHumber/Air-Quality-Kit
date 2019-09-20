EESchema Schematic File Version 4
LIBS:Heltec-cache
EELAYER 29 0
EELAYER END
$Descr A4 11693 8268
encoding utf-8
Sheet 1 1
Title "Heltec Lora 32 Air Quality"
Date "2019-08-31"
Rev "2.0"
Comp "Connected Humber CIC"
Comment1 ""
Comment2 ""
Comment3 ""
Comment4 ""
$EndDescr
Text GLabel 2250 1050 2    50   Input ~ 0
VEXT_2
Text GLabel 4450 4400 0    50   BiDi ~ 0
SCL
Text GLabel 4450 4600 0    50   BiDi ~ 0
SDA
Text GLabel 2250 1250 2    50   BiDi ~ 0
SCL
Text GLabel 2250 1350 2    50   BiDi ~ 0
SDA
$Comp
L Connector:Conn_01x06_Male J1
U 1 1 5CFFA93C
P 2050 1250
F 0 "J1" H 2156 1628 50  0000 C CNN
F 1 "BME280" H 2156 1537 50  0000 C CNN
F 2 "Connector_JST:JST_XH_B06B-XH-A_1x06_P2.50mm_Vertical" H 2050 1250 50  0001 C CNN
F 3 "~" H 2050 1250 50  0001 C CNN
	1    2050 1250
	1    0    0    -1  
$EndComp
Text GLabel 10050 5150 0    50   Input ~ 0
SDS_5V
Text GLabel 10050 4850 0    50   Output ~ 0
SDS_TX
Text GLabel 10050 4950 0    50   Input ~ 0
SDS_RX
Text GLabel 4450 4700 0    50   Input ~ 0
SDS_TX
$Comp
L Connector:Conn_01x04_Male J5
U 1 1 5CFFF2ED
P 6200 1150
F 0 "J5" H 6173 1030 50  0000 R CNN
F 1 "GPS" H 6173 1121 50  0000 R CNN
F 2 "Connector_JST:JST_XH_B04B-XH-A_1x04_P2.50mm_Vertical" H 6200 1150 50  0001 C CNN
F 3 "~" H 6200 1150 50  0001 C CNN
	1    6200 1150
	-1   0    0    1   
$EndComp
Text GLabel 6000 1250 0    50   Input ~ 0
VEXT_2
Text GLabel 6000 1050 0    50   Output ~ 0
GPS_TX
NoConn ~ 6000 950 
$Comp
L Mechanical:MountingHole H1
U 1 1 5D0012E2
P 10400 700
F 0 "H1" H 10500 746 50  0000 L CNN
F 1 "MountingHole" H 10500 655 50  0000 L CNN
F 2 "MountingHole:MountingHole_3.2mm_M3" H 10400 700 50  0001 C CNN
F 3 "~" H 10400 700 50  0001 C CNN
	1    10400 700 
	1    0    0    -1  
$EndComp
$Comp
L Mechanical:MountingHole H2
U 1 1 5D00135B
P 10400 1000
F 0 "H2" H 10500 1046 50  0000 L CNN
F 1 "MountingHole" H 10500 955 50  0000 L CNN
F 2 "MountingHole:MountingHole_3.2mm_M3" H 10400 1000 50  0001 C CNN
F 3 "~" H 10400 1000 50  0001 C CNN
	1    10400 1000
	1    0    0    -1  
$EndComp
$Comp
L Mechanical:MountingHole H3
U 1 1 5D0013B6
P 10400 1300
F 0 "H3" H 10500 1346 50  0000 L CNN
F 1 "MountingHole" H 10500 1255 50  0000 L CNN
F 2 "MountingHole:MountingHole_3.2mm_M3" H 10400 1300 50  0001 C CNN
F 3 "~" H 10400 1300 50  0001 C CNN
	1    10400 1300
	1    0    0    -1  
$EndComp
$Comp
L Mechanical:MountingHole H4
U 1 1 5D0014CA
P 10400 1650
F 0 "H4" H 10500 1696 50  0000 L CNN
F 1 "MountingHole" H 10500 1605 50  0000 L CNN
F 2 "MountingHole:MountingHole_3.2mm_M3" H 10400 1650 50  0001 C CNN
F 3 "~" H 10400 1650 50  0001 C CNN
	1    10400 1650
	1    0    0    -1  
$EndComp
$Comp
L Switch:SW_Push SW3
U 1 1 5D00EAD6
P 1950 7100
F 0 "SW3" H 1950 7385 50  0000 C CNN
F 1 "Up" H 1950 7294 50  0000 C CNN
F 2 "Button_Switch_SMD:SW_SPST_B3S-1000" H 1950 7300 50  0001 C CNN
F 3 "" H 1950 7300 50  0001 C CNN
	1    1950 7100
	1    0    0    -1  
$EndComp
$Comp
L power:GND #PWR013
U 1 1 5D00F184
P 1750 7100
F 0 "#PWR013" H 1750 6850 50  0001 C CNN
F 1 "GND" H 1755 6927 50  0000 C CNN
F 2 "" H 1750 7100 50  0001 C CNN
F 3 "" H 1750 7100 50  0001 C CNN
	1    1750 7100
	1    0    0    -1  
$EndComp
$Comp
L Switch:SW_Push SW1
U 1 1 5D00F762
P 1900 5650
F 0 "SW1" H 1900 5935 50  0000 C CNN
F 1 "Down" H 1900 5844 50  0000 C CNN
F 2 "Button_Switch_SMD:SW_SPST_B3S-1000" H 1900 5850 50  0001 C CNN
F 3 "" H 1900 5850 50  0001 C CNN
	1    1900 5650
	-1   0    0    -1  
$EndComp
$Comp
L power:GND #PWR05
U 1 1 5D00F769
P 1700 5650
F 0 "#PWR05" H 1700 5400 50  0001 C CNN
F 1 "GND" H 1705 5477 50  0000 C CNN
F 2 "" H 1700 5650 50  0001 C CNN
F 3 "" H 1700 5650 50  0001 C CNN
	1    1700 5650
	-1   0    0    -1  
$EndComp
$Comp
L power:GND #PWR07
U 1 1 5D0125C9
P 4450 3100
F 0 "#PWR07" H 4450 2850 50  0001 C CNN
F 1 "GND" V 4455 2972 50  0000 R CNN
F 2 "" H 4450 3100 50  0001 C CNN
F 3 "" H 4450 3100 50  0001 C CNN
	1    4450 3100
	0    1    1    0   
$EndComp
Text GLabel 4450 3200 0    50   BiDi ~ 0
5V
$Comp
L power:GND #PWR01
U 1 1 5D013446
P 2250 1150
F 0 "#PWR01" H 2250 900 50  0001 C CNN
F 1 "GND" V 2255 1022 50  0000 R CNN
F 2 "" H 2250 1150 50  0001 C CNN
F 3 "" H 2250 1150 50  0001 C CNN
	1    2250 1150
	0    -1   -1   0   
$EndComp
$Comp
L power:GND #PWR015
U 1 1 5D014514
P 6000 1150
F 0 "#PWR015" H 6000 900 50  0001 C CNN
F 1 "GND" V 6005 1022 50  0000 R CNN
F 2 "" H 6000 1150 50  0001 C CNN
F 3 "" H 6000 1150 50  0001 C CNN
	1    6000 1150
	0    1    1    0   
$EndComp
$Comp
L power:GND #PWR012
U 1 1 5CFA5FEC
P 5850 3100
F 0 "#PWR012" H 5850 2850 50  0001 C CNN
F 1 "GND" V 5855 2972 50  0000 R CNN
F 2 "" H 5850 3100 50  0001 C CNN
F 3 "" H 5850 3100 50  0001 C CNN
	1    5850 3100
	0    -1   -1   0   
$EndComp
Text GLabel 5850 3200 2    50   Output ~ 0
3V3_1
Text GLabel 2150 7100 2    50   Output ~ 0
UP_SW
Text GLabel 5850 4700 2    50   Output ~ 0
SDS_RX
Text GLabel 5850 4600 2    50   Output ~ 0
UP_SW
Text GLabel 2100 5650 2    50   Input ~ 0
DN_SW
Text GLabel 4450 3800 0    50   Output ~ 0
DN_SW
Text GLabel 4450 3700 0    50   Input ~ 0
RST
Text GLabel 4450 3300 0    50   Output ~ 0
Vext_1
$Comp
L Connector:Conn_01x02_Male J3
U 1 1 5CFE8353
P 4250 1200
F 0 "J3" H 4356 1378 50  0000 C CNN
F 1 "5V" H 4356 1287 50  0000 C CNN
F 2 "Connector_JST:JST_XH_B02B-XH-A_1x02_P2.50mm_Vertical" H 4250 1200 50  0001 C CNN
F 3 "~" H 4250 1200 50  0001 C CNN
	1    4250 1200
	1    0    0    -1  
$EndComp
$Comp
L power:GND #PWR010
U 1 1 5CFE8D1A
P 4450 1200
F 0 "#PWR010" H 4450 950 50  0001 C CNN
F 1 "GND" V 4450 1100 50  0000 R CNN
F 2 "" H 4450 1200 50  0001 C CNN
F 3 "" H 4450 1200 50  0001 C CNN
	1    4450 1200
	0    -1   -1   0   
$EndComp
Text GLabel 4450 1300 2    50   Output ~ 0
5V
$Comp
L power:GND #PWR02
U 1 1 5D590118
P 2900 3700
F 0 "#PWR02" H 2900 3450 50  0001 C CNN
F 1 "GND" H 2905 3527 50  0000 C CNN
F 2 "" H 2900 3700 50  0001 C CNN
F 3 "" H 2900 3700 50  0001 C CNN
	1    2900 3700
	1    0    0    -1  
$EndComp
Text GLabel 2900 3400 1    50   BiDi ~ 0
5V
Text Notes 2350 4050 0    50   ~ 0
Place close to Heltec 5V & GND
$Comp
L Device:R R3
U 1 1 5D593057
P 7150 5800
F 0 "R3" H 7220 5846 50  0000 L CNN
F 1 "10k" H 7220 5755 50  0000 L CNN
F 2 "Resistor_SMD:R_0805_2012Metric_Pad1.15x1.40mm_HandSolder" V 7080 5800 50  0001 C CNN
F 3 "~" H 7150 5800 50  0001 C CNN
	1    7150 5800
	1    0    0    -1  
$EndComp
$Comp
L Device:CP C1
U 1 1 5D59E3A1
P 2900 3550
F 0 "C1" H 3018 3596 50  0000 L CNN
F 1 "470uF" H 3018 3505 50  0000 L CNN
F 2 "Capacitor_THT:CP_Radial_D6.3mm_P2.50mm" H 2938 3400 50  0001 C CNN
F 3 "~" H 2900 3550 50  0001 C CNN
	1    2900 3550
	1    0    0    -1  
$EndComp
Text Notes 1550 2200 0    50   ~ 0
Suits 4pin BME280, 6 pin BME280 and 6 pin BME680\n4 pin BME280 defaults to addr 0x76\n6 pin variants & BME680 default to 0x77 but\ncan be changed to 0x76 by adding one link.
Wire Wire Line
	2250 1450 2700 1450
$Comp
L Device:Jumper_NO_Small JP1
U 1 1 5D5A3B4B
P 2800 1450
F 0 "JP1" H 2800 1635 50  0000 C CNN
F 1 "BME680_0x76" H 2800 1544 50  0000 C CNN
F 2 "Jumper:SolderJumper-2_P1.3mm_Open_Pad1.0x1.5mm" H 2800 1450 50  0001 C CNN
F 3 "~" H 2800 1450 50  0001 C CNN
	1    2800 1450
	1    0    0    -1  
$EndComp
Wire Wire Line
	2250 1550 2700 1550
$Comp
L Device:Jumper_NO_Small JP2
U 1 1 5D5A5047
P 2800 1550
F 0 "JP2" H 2800 1450 50  0000 C CNN
F 1 "BME280_0x76" H 2800 1350 50  0000 C CNN
F 2 "Jumper:SolderJumper-2_P1.3mm_Open_Pad1.0x1.5mm" H 2800 1550 50  0001 C CNN
F 3 "~" H 2800 1550 50  0001 C CNN
	1    2800 1550
	1    0    0    -1  
$EndComp
Wire Wire Line
	2900 1450 2900 1500
$Comp
L power:GND #PWR04
U 1 1 5D5A5BC5
P 3100 1500
F 0 "#PWR04" H 3100 1250 50  0001 C CNN
F 1 "GND" H 3300 1400 50  0000 R CNN
F 2 "" H 3100 1500 50  0001 C CNN
F 3 "" H 3100 1500 50  0001 C CNN
	1    3100 1500
	1    0    0    -1  
$EndComp
Wire Wire Line
	2900 1500 3100 1500
Connection ~ 2900 1500
Wire Wire Line
	2900 1500 2900 1550
$Comp
L power:GND #PWR014
U 1 1 5D5ABF8B
P 9550 5050
F 0 "#PWR014" H 9550 4800 50  0001 C CNN
F 1 "GND" H 9555 4922 50  0000 R CNN
F 2 "" H 9550 5050 50  0001 C CNN
F 3 "" H 9550 5050 50  0001 C CNN
	1    9550 5050
	0    1    1    0   
$EndComp
Text Notes 5650 1400 0    50   ~ 0
Pin 4 is GPS RX - Not used
NoConn ~ 4450 4800
NoConn ~ 4450 3500
NoConn ~ 4450 3600
NoConn ~ 4450 4000
NoConn ~ 4450 4200
NoConn ~ 4450 4300
NoConn ~ 4450 4500
NoConn ~ 5850 3700
NoConn ~ 5850 3800
NoConn ~ 5850 3900
NoConn ~ 5850 4000
NoConn ~ 5850 4100
NoConn ~ 5850 4200
NoConn ~ 5850 4300
NoConn ~ 5850 4400
NoConn ~ 5850 4500
NoConn ~ 5850 4800
NoConn ~ 5850 3400
NoConn ~ 5850 3500
NoConn ~ 5850 3600
NoConn ~ 5850 3300
Text Notes 9050 4700 0    50   ~ 0
SDS011 can have a low-side switch to\nturn it on/off using Vext1 OR JP3 can be made\nto leave the 5V permanently on
$Comp
L Connector:Conn_01x05_Male J2
U 1 1 5D7A8474
P 10250 5050
F 0 "J2" H 10222 4982 50  0000 R CNN
F 1 "SDS011" H 10222 5073 50  0000 R CNN
F 2 "Connector_JST:JST_XH_B05B-XH-AM_1x05_P2.50mm_Vertical" H 10250 5050 50  0001 C CNN
F 3 "~" H 10250 5050 50  0001 C CNN
	1    10250 5050
	-1   0    0    1   
$EndComp
$Comp
L Heltec:Heltect_Lora_32 U1
U 1 1 5D7EDA6C
P 5300 3900
F 0 "U1" H 5150 4965 50  0000 C CNN
F 1 "Heltect_Lora_32" H 5150 4874 50  0000 C CNN
F 2 "Heltec_Models:Heltec_LoRa_32" H 5300 3900 50  0001 C CNN
F 3 "" H 5300 3900 50  0001 C CNN
	1    5300 3900
	1    0    0    -1  
$EndComp
Text GLabel 4450 3400 0    50   Output ~ 0
Vext_2
$Comp
L HighSideSwitch:STMPS2171MTR U2
U 1 1 5D7F52C7
P 7850 5850
F 0 "U2" H 7750 6775 50  0000 C CNN
F 1 "STMPS2171MTR" H 7750 6684 50  0000 C CNN
F 2 "Package_TO_SOT_SMD:SOT-23-5_HandSoldering" H 7850 5850 50  0001 C CNN
F 3 "" H 7850 5850 50  0001 C CNN
	1    7850 5850
	1    0    0    -1  
$EndComp
$Comp
L power:GND #PWR08
U 1 1 5D7F6E90
P 7850 6000
F 0 "#PWR08" H 7850 5750 50  0001 C CNN
F 1 "GND" H 7855 5827 50  0000 C CNN
F 2 "" H 7850 6000 50  0001 C CNN
F 3 "" H 7850 6000 50  0001 C CNN
	1    7850 6000
	1    0    0    -1  
$EndComp
Text GLabel 7250 5150 0    50   Input ~ 0
5V
Text GLabel 8250 5150 2    50   Output ~ 0
SDS_5V
Text GLabel 6900 5500 0    50   Input ~ 0
Vext_1
Wire Wire Line
	6900 5500 7150 5500
Wire Wire Line
	7150 5500 7150 5650
Connection ~ 7150 5500
Wire Wire Line
	7150 5500 7250 5500
$Comp
L power:GND #PWR03
U 1 1 5D7F938A
P 7150 5950
F 0 "#PWR03" H 7150 5700 50  0001 C CNN
F 1 "GND" H 7155 5777 50  0000 C CNN
F 2 "" H 7150 5950 50  0001 C CNN
F 3 "" H 7150 5950 50  0001 C CNN
	1    7150 5950
	1    0    0    -1  
$EndComp
NoConn ~ 8250 5500
Wire Wire Line
	9550 5050 10050 5050
Text Notes 7000 4850 0    50   ~ 0
Highside switch used to turn off power\nto the SDS011
Text GLabel 2100 6350 2    50   Input ~ 0
SEL_SW
Text GLabel 4450 4100 0    50   Output ~ 0
SEL_SW
$Comp
L power:GND #PWR06
U 1 1 5D00F81F
P 1700 6350
F 0 "#PWR06" H 1700 6100 50  0001 C CNN
F 1 "GND" H 1705 6177 50  0000 C CNN
F 2 "" H 1700 6350 50  0001 C CNN
F 3 "" H 1700 6350 50  0001 C CNN
	1    1700 6350
	-1   0    0    -1  
$EndComp
$Comp
L Switch:SW_Push SW2
U 1 1 5D00F818
P 1900 6350
F 0 "SW2" H 1900 6635 50  0000 C CNN
F 1 "Select" H 1900 6544 50  0000 C CNN
F 2 "Button_Switch_SMD:SW_SPST_B3S-1000" H 1900 6550 50  0001 C CNN
F 3 "" H 1900 6550 50  0001 C CNN
	1    1900 6350
	1    0    0    -1  
$EndComp
Text GLabel 4450 3900 0    50   Input ~ 0
GPS_TX
Text Notes 1700 5150 0    79   ~ 0
Menu Buttons
Text Notes 4000 1550 0    50   ~ 0
5V Power In. \n500mA capable recommended
Text Notes 5850 750  0    79   ~ 0
GPS
Text Notes 4300 750  0    79   ~ 0
POWER
Text Notes 2050 750  0    79   ~ 0
BMEx80
Text Notes 8250 4200 0    79   ~ 0
SDS011 Dust Sensor
Text Notes 5050 2550 0    79   ~ 0
MCU
Text Notes 6900 6400 0    79   ~ 0
Vext is switched by GPIO21
$EndSCHEMATC
