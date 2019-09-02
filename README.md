# DeCANder
Landrover Defender TD4 / TDCI - Arduino UNO - CAN-bus display/tool (2,4l Machine only; MY 2007-2011)

USE THIS CODE ON YOUR OWN RISK!

This is a small project for creating a display for additional information from the CAN-bus of a Land Rover Defender with the Ford "TD4" / "TDCI" / "duratorq" 2,4l engine. (MY 2007 - 2011) This classical 4x4 has a CAN-bus connection between the dashboard and the engine control unit (ECU).

The primary idea is NOT to send any messages to the CAN bus (i.e. standard OBD requests) but only to read and interpret the messages from the bus that are being sent by the car's own instances. These messages are not documented whatsoever. Thus, this information has to be reverse-engineered for the CAN-bus traffic to make sense. However, most of the (interesting) data already could be recognized - some other data still is a mystery. =) So if you also like to reverse-engineer some of the not-yet-known values then you should try DeCANder version 1 from the master branch which was uploaded 2019-04-27 on github.

Hardware:
I am using an Arduino "UNO" + a CAN-bus-shield "DIGITAL V1.2 10/10/2013 by ElecFreaks" + a "LCD-keypad-shield" from "D1 Robot" using a 2x16 Hitachi HD44780 display. Supply voltage +12V can be taken from the car's OBD connector. "CAN H", "CAN L", "+12V and "GND" have to be connected to the d-sub 9 connector on the CAN-bus-shield. PIN 10 from the display-shield conflicts with the CAN-bus-shield so i just desoldered this pin from the LCD-keypad-shield for now. There it is used to dim the LCD backlight. Disconnected, the backlight is set to 100% - so no serious problem here. A piezo speaker can be connected to Pins A2 (+) and A3 (-). It will give a warning sound if coolant temperature gets over 100°C. (-> constant "hotWarn" in the code)

Libraries used:
"SPI.h" for sending serial data via USB.
"LiquidCrystal.h" for the LCD on the LCD-keypad-shield
"mcp_can.h" for the CAN-bus-shield (download from manufacturer's site and save in libraries!)

Software:
This Arduino code interprets and displays selected CAN-data on LCD (using SI units) and sends all received CAN-data in a csv-like shape via USB so you can collect it with a connected computer and terminal. (i.e. Putty for MS-Windows or Linux) The (LEFT/RIGHT) keys from the LCD-keypad-shield select five different display modes. (startup-data; driving-data; trip-data; tankstop-data; all-time-data) Pressing the (UP) key, Arduino will show some explanation to the displayed values. Some data will be stored in Arduino's EEPROM. The (SELECT) key will reset these data-sets in selected display-modes. (-> Especially the tankstop-data, if the Defender has been filled up with diesel, should be reset!)
