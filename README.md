# DeCANder
Landrover Defender TD4 / TDCI - Arduino UNO - CAN-bus display/tool

This is a small project for creating a heads-up-display for additional information from the CAN-bus of a Land Rover Defender with the Ford "TD4" / "TDCI" / "duratorq" engine. This 4x4 has a CAN-bus between the dashboard and the engine control unit (ECU).

The idea is not to send messages to the CAN bus (i.e. standard OBD requests) but only to read and interpret the messages from the bus that are being sent from the car's instances. These messages are not documented whatsoever. Thus this information has to be reverse-engineered for the CAN-bus traffic to make sense.

Most of the data already could be recognized - some other data is still a mystery. =)

Hardware:
I am using an Arduino "UNO" + a CAN-bus-shield "DIGITAL V1.2 10/10/2013 by ElecFreaks" + a "LCD-keypad-shield from D! Robot" using a 2x16 Hitachi HD44780 display. Supply voltage +12V can be taken from the car's OBD connector. "CAN H", "CAN L", "+12V and "GND" have to be connected to the d-sub 9 connector on the CAN-bus-shield. PIN 10 from the display-shield conflicts with the CAN-bus-shield so i just desoldered this pin. On the LCD-keypad-shield it is used to dim the LCD backlight. Disconnected, the backlight is set to 100% - so no serious problem here.

Software:
This version is quite useful already - especially in the matter of reverse engineering capabilities. The CAN-bus is being connected an all messages read. All the messages are being brought in a csv-like shape and send serial via USB so you can collect data with a connected computer and terminal. (i.e. putty for MS-Windows or Linux) CAN-data-bytes can also be displayed on the LCD-keypad-shield. The CAN-ID and one of the 8 data bytes will be shown. You can select the CAN-ID by pressing UP/DOWN buttons and select the data byte by pressing LEFT/RIGHT buttons. By pressing the SELECT button two data bytes are being interpreted as a "hi" an "low" byte, being calculated as a two-byte-value and displayed.

