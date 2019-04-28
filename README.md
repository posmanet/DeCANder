# DeCANder
Landrover Defender TD4 / TDCI - Arduino UNO - CAN-bus display/tool

USE THIS CODE ON YOUR OWN RISK!

This is a small project for creating a display for additional information from the CAN-bus of a Land Rover Defender with the Ford "TD4" / "TDCI" / "duratorq" engine. This 4x4 has a CAN-bus connection between the dashboard and the engine control unit (ECU).

The idea is not to send messages to the CAN bus (i.e. standard OBD requests) but only to read and interpret the messages from the bus that are being sent from the car's own instances. These messages are not documented whatsoever. Thus this information has to be reverse-engineered for the CAN-bus traffic to make sense.

Most of the data already could be recognized - some other data is still a mystery. =)

Hardware:
I am using an Arduino "UNO" + a CAN-bus-shield "DIGITAL V1.2 10/10/2013 by ElecFreaks" + a "LCD-keypad-shield from D1 Robot" using a 2x16 Hitachi HD44780 display. Supply voltage +12V can be taken from the car's OBD connector. "CAN H", "CAN L", "+12V and "GND" have to be connected to the d-sub 9 connector on the CAN-bus-shield. PIN 10 from the display-shield conflicts with the CAN-bus-shield so i just desoldered this pin for now. On the LCD-keypad-shield it is used to dim the LCD backlight. Disconnected, the backlight is set to 100% - so no serious problem here.

Libraries used:
SPI.h for sending serial data via USB.
LiquidCrystal.h for the LCD on the LCD-keypad-shield
mcp_can.h for the CAN-bus-shield (download from manufacturer and save in libraries!)

Software:
This Arduino Code interprets and displays selected CAN-data on LCD and sends all CAN-data in a csv-like shape via USB so you can collect it with a connected computer and terminal. (i.e. putty for MS-Windows or Linux) Keys from the LCD-keypad-shield select display modes (UP/DOWN).
