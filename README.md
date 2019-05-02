# DeCANder
Landrover Defender TD4 / TDCI - Arduino UNO - CAN-bus display/tool

USE THIS CODE ON YOUR OWN RISK!

This is a small project for creating a display for additional information from the CAN-bus of a Land Rover Defender with the Ford "TD4" / "TDCI" / "duratorq" engine. This classical 4x4 has a CAN-bus connection between the dashboard and the engine control unit (ECU).

The primary idea is not to send messages to the CAN bus (i.e. standard OBD requests) but only to read and interpret the messages from the bus that are being sent from the car's own instances. These messages are not documented whatsoever. Thus, this information has to be reverse-engineered for the CAN-bus traffic to make sense. However, most of the (interesting) data already could be recognized - some other data still is a mystery. =) So if you also like to reverse-engineer some of the not-yet-known values then you should try DeCANder version 1 from the master branch which was uploaded 2019-04-27 on github.

Hardware:
I am using an Arduino "UNO" + a CAN-bus-shield "DIGITAL V1.2 10/10/2013 by ElecFreaks" + a "LCD-keypad-shield" from "D1 Robot" using a 2x16 Hitachi HD44780 display. Supply voltage +12V can be taken from the car's OBD connector. "CAN H", "CAN L", "+12V and "GND" have to be connected to the d-sub 9 connector on the CAN-bus-shield. PIN 10 from the display-shield conflicts with the CAN-bus-shield so i just desoldered this pin from the LCD-keypad-shield for now. There it is used to dim the LCD backlight. Disconnected, the backlight is set to 100% - so no serious problem here.

Libraries used:
"SPI.h" for sending serial data via USB.
"LiquidCrystal.h" for the LCD on the LCD-keypad-shield
"mcp_can.h" for the CAN-bus-shield (download from manufacturer's site and save in libraries!)

Software:
This Arduino code interprets and displays selected CAN-data on LCD (using SI units) and sends all CAN-data in a csv-like shape via USB so you can collect it with a connected computer and terminal. (i.e. Putty for MS-Windows or Linux) The (UP/DOWN) keys from the LCD-keypad-shield select multiple display modes. The (SELECT) key will switch to an alternate set of display modes, showing some of the most interesting-looking (raw int) CAN-values that are not known yet.
