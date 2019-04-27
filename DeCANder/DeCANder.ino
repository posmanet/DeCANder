/*
 * using LiquidCrystal Library (16x2 LCD display; Hitachi HD44780) <- LCD-keypad-shield
 * -> Arduino-Pins D4-D9 (and D10!!!) plus A0 for the keys
 * D10 = Backlight control!! DISCONNECT PIN 10 FROM LCD-SHIELD!
 * LCD RS pin to digital pin 8
 * LCD Enable pin to digital pin 9
 * LCD D4 pin to digital pin 4
 * LCD D5 pin to digital pin 5
 * LCD D6 pin to digital pin 6
 * LCD D7 pin to digital pin 7
 * LCD R/W pin to ground
 * 10K resistor: ends to +5V and ground; wiper to LCD VO pin (pin 3)
*/

/*
 * CAN-BUS Shield; DIGITAL V1.2 10/10/2013 by ElecFreaks (.com)
 * CAN V2.0 up to 1 Mb/s; feat. MCP2515 CAN Bus controller (SPI 10 MHz) & MCP2551 CAN transceiver
 * ODB-II converter cable (import ODB-II library 1.2 2014-07-08)
 * sub-D 2 -> GND -> CAN 4+5
 * sub-D 3 -> CAN H -> CAN 6
 * sub-D 5 -> CAN L -> CAN 14
 * sub-D 9 -> 12V -> CAN 16
 * Ardunino pin 13, 12, 11, 10(cs) and 2 are occupied ! Jackpot!!
 */

#include <LiquidCrystal.h>
LiquidCrystal lcd(8, 9, 4, 5, 6, 7);
int taster = 0;                                        // ADC button value from LCD-keypad-shield
unsigned char canLen = 0;
unsigned char canBuf[8];
int IDs[] = {907, 297, 906, 1218, 246, 298, 296, 295, 1029, 1090, 1024}; // sorted by occurrence
int ID = 0;
int IDbyte = 0;
int sel = 1;
int warte = 250;
int offset = 0;
int twoByte = 0;

#include <SPI.h>
#include "mcp_can.h"

MCP_CAN CAN(10);                                    // Set CS pin to Arduino pin 10

void setup() {
  Serial.begin(115200);
  Serial.println("DeCANder v1.0");
  Serial.println("=============");
  Serial.println("Trying to connect to car's CAN system. Watch the LCD!");
  
  lcd.begin(16, 2);                                    // set up the LCD's number of columns and rows: 
  lcd.print("DeCANder v1.0");
  delay(1000);

  while (CAN_OK != CAN.begin(CAN_500KBPS))              // init can bus : baudrate TD4 = 500k
    {
      lcd.setCursor(0,1);
      lcd.print("CAN init fail...");
      delay(1000);
    }
  lcd.setCursor(0,1);
  lcd.print("CAN init ok     ");
  delay(1000);
  lcd.setCursor(0,0);
  lcd.print("                ");
  lcd.setCursor(0,1);
  lcd.print("                ");
}

void showID() {
  lcd.setCursor(0,0);
  lcd.print("ID:");
  if (IDs[ID] < 256) { lcd.print(" "); }
  lcd.print(IDs[ID],HEX);
  lcd.print(".");
  lcd.print(IDbyte);
  if (sel == 2) {
    lcd.print("&");
    if (IDbyte == 7) { lcd.print("0"); } else { lcd.print(IDbyte+1); }
  } else {
    lcd.print("  ");
  }
}

void showData() {
  lcd.setCursor(0,1);
  if (canBuf[IDbyte] < 10) { lcd.print("  "); } else if (canBuf[IDbyte] < 100) { lcd.print(" "); }
  lcd.print(canBuf[IDbyte]);
  if (sel == 2 ) {
    lcd.print(" ");
    if (IDbyte == 7) {
      if (canBuf[0] < 10) { lcd.print("  "); } else if (canBuf[0] < 100) { lcd.print(" "); }
      lcd.print(canBuf[0]);
    } else {
      if (canBuf[IDbyte+1] < 10) { lcd.print("  "); } else if (canBuf[IDbyte+1] < 100) { lcd.print(" "); }
      lcd.print(canBuf[IDbyte+1]);
    }
    lcd.print(" ");
    if ((ID == 0) and (IDbyte == 4)) { offset = 0; } else
    if ((ID == 0) and (IDbyte == 6)) { offset = 0; } else
    if ((ID == 1) and (IDbyte == 6)) { offset = 12; } else
    if ((ID == 2) and (IDbyte == 4)) { offset = 0; } else
    if ((ID == 2) and (IDbyte == 6)) { offset = 0; } else
    if ((ID == 4) and (IDbyte == 4)) { offset = 96; } else
    if ((ID == 4) and (IDbyte == 6)) { offset = 0; } else
    if ((ID == 6) and (IDbyte == 0)) { offset = 6; } else
    if ((ID == 6) and (IDbyte == 2)) { offset = 7; } else
    if ((ID == 6) and (IDbyte == 4)) { offset = 55; } else
    if ((ID == 6) and (IDbyte == 6)) { offset = 7; } else { offset = 0; }
    twoByte = (canBuf[IDbyte] - offset) * 256 + canBuf[IDbyte+1];
    if (twoByte < 10) { lcd.print("    "); } else
    if (twoByte < 100) { lcd.print("   "); } else
    if (twoByte < 1000) { lcd.print("  "); } else
    if (twoByte < 10000) { lcd.print(" "); }
    lcd.print(twoByte);
  } else {
    lcd.print("             ");
  }
}

void loop() {
  lcd.setCursor(14, 0);
  if ((millis()/1000%60) < 10) { lcd.print(" "); }
  lcd.print(millis()/1000%60);

  taster = analogRead(A0);
  if (taster < 71) { // r
    if (sel == 2 ) {
      if (IDbyte > 5) { IDbyte = 0; } else { IDbyte++; IDbyte++; }
    } else {
      if (IDbyte > 6) { IDbyte = 0; } else { IDbyte++; }
     }
    delay(warte);
  } else if (taster < 235) { // u
    if (ID < 1) { ID = 10; } else { ID--; }
    delay(warte);
  } else if (taster < 416) { // d
    if (ID == 10) { ID = 0; } else { ID++; }
    delay(warte);
  } else if (taster < 624) { // l
    if (sel == 2 ) {
      if (IDbyte < 2) { IDbyte = 7; } else { IDbyte--; IDbyte--; }
    } else {
      if (IDbyte < 1) { IDbyte = 7; } else { IDbyte--; }
     }
    delay(warte);
  } else if (taster < 884) { // x
    if (sel == 1) {
      sel = 2;
      if (IDbyte%2 > 0) { IDbyte--; };
    } else {
      sel = 1;
    }
    delay(warte);
  }
  showID();

  
  if (CAN_MSGAVAIL == CAN.checkReceive())            // check if data coming
  {
    CAN.readMsgBuf(&canLen, canBuf);    // read data,  len: data length, buf: data buf
    unsigned long canID = CAN.getCanId();
    Serial.println(canID, HEX);
    Serial.println(",");
    for (int i = 0; i < canLen; i++)    // print the data
      {
        Serial.print(canBuf[i], HEX);
        if (i<(canLen-1)) { Serial.print(","); }
      }
    if (canID == IDs[ID]) { showData(); }
    Serial.println();
  }

}
