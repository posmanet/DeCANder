/*
 * DeCANder: Landrover Defender TD4 / TDCI - Arduino UNO - CAN-bus display/tool
 * display for additional information from the CAN-bus of a Land Rover Defender with the Ford "TD4" / "TDCI" / "duratorq" engine
 * Created 2019 by Posmanet (Martin) and released on github.com.
 * USE THIS CODE ON YOUR OWN RISK!
 */

// Constants
#define DeCANderVer "2.1"
#define modes 2                                             // number of available display modes; 9 MAX - see bottom lines

// Variables
int taster = 0;                                             // ADC button value from LCD-keypad-shield
unsigned char canLen = 0;                                   // CAN
unsigned char canBuf[8];                                    // CAN
unsigned long canID = 0;                                    // CAN
int warte = 50;                                             // keypress delay
int mode = 1;                                               // selectable display mode (up/down)
unsigned long lastmillis = 0;                               // display refresh timer
int hours = 0;                                              // uptime calculation variable
int minutes = 0;                                            // uptime calculation variable
int seconds = 0;                                            // uptime calculation variable
unsigned long times = 0;                                    // time calculations help variable (unsigned long for millis())
unsigned long trip = 0;                                     // trip timer
unsigned long tripStart = 0;                                // calculate trip timer
unsigned long tripStop = 0;                                 // calculate trip timer
bool ign = 0;                                               // interpreted ignition on/off
bool pre = 0;                                               // interpreted diesel preheat on/off
bool eng = 0;                                               // interpreted engine on/off
float spd = 0;                                              // interpreted vehicle speed
int rpm = 0;                                                // interpreted engine rpm

// Libraries
#include <LiquidCrystal.h>                                  // "LCD-keypad-shield from D1 Robot" using a 2x16 Hitachi HD44780 display
LiquidCrystal lcd(8, 9, 4, 5, 6, 7);                        // Disconnect pin 10! It conflicts with the CAN-shield!
#include <SPI.h>                                            // sending serial data via Arduinos USB-Port
#include "mcp_can.h"                                        // CAN-bus-shield "DIGITAL V1.2 10/10/2013 by ElecFreaks"
MCP_CAN CAN(10);                                            // Set CS pin to Arduino pin 10 - is hardwired on my shield!

void setup() {
  Serial.begin(115200);
  Serial.print("DeCANder v");
  Serial.println(DeCANderVer);
  Serial.println("=============");
  Serial.println("connecting...");
  lcd.begin(16, 2);                                         // set up the LCD's number of columns and rows: 
  lcd.print(" DeCANder  v");
  lcd.print(DeCANderVer);
  delay(2000);
  while (CAN_OK != CAN.begin(CAN_500KBPS)) {                // init can bus : baudrate TD4 = 500k
    lcd.setCursor(0,1);
    lcd.print(" CAN init fail. ");
    delay(1000);
  }
  lcd.setCursor(0,1);
  lcd.print(" CAN init done! ");
  delay(1000);
  lcd.clear();
}

void evalCAN() {
  if (canID == 1218) { if (canBuf[3] < 80) { ign = HIGH; } else if (canBuf[3] > 130) { ign = LOW; } }
  if (canID == 295)  { if (canBuf[7] == 1) { pre = HIGH;} else if (canBuf[7] == 0) { pre = LOW; } }
  if (canID == 297)  {
      if (canBuf[2] == 3) {
        if (eng == LOW) { tripStart = millis(); } 
        eng = HIGH;
      } else if (canBuf[2] == 0) {
        if (eng == HIGH) { tripStop = millis(); } 
        eng = LOW;
      }
    }
  if (canID == 907)  { spd = (canBuf[4] * 256 + canBuf[5]) / 100; }
  if (canID == 246)  { rpm = (canBuf[4] - 96) * 256 + canBuf[5]; }
}

void showMode1() {
  lcd.setCursor(0,0);
  if ((tripStart == 0) or ((millis() - tripStart) < 60000)) {
    lcd.print("uptime: ");
    times = millis() / 1000;
  } else {
    lcd.print("trip  : ");
    if (tripStop > tripStart) {
      times = (tripStop - tripStart) / 1000;
    } else {
      times = (millis() - tripStart) / 1000;
    }
  }
  hours = times / 60 / 60;
  minutes = times / 60 - hours * 60;
  seconds = times - minutes * 60 - hours * 60 * 60;
  if (hours > 99) { hours = 99; minutes = 99; seconds = 99; }
  if (hours < 10) { lcd.print("0"); }
  lcd.print(hours);
  lcd.print(":");
  if (minutes < 10) { lcd.print("0"); }
  lcd.print(minutes);
  lcd.print(":");
  if (seconds < 10) { lcd.print("0"); }
  lcd.print(seconds);
  lcd.setCursor(0,1);
  lcd.print("ig=");
  lcd.print(ign);
  lcd.setCursor(6,1);
  lcd.print("pr=");
  lcd.print(pre);
  lcd.setCursor(12,1);
  lcd.print("en=");
  lcd.print(eng);
}

void showMode2() {
  lcd.setCursor(0,0);
  lcd.print(" v : ");
  if (spd < 10) { lcd.print("  "); } else if (spd < 100) { lcd.print(" "); }
  lcd.print(spd,1);
  lcd.print(" km/h ");
  lcd.setCursor(0,1);
  lcd.print("rpm: ");
  if (rpm <= 0) { lcd.print("engine off"); } else
  if (rpm > 9999) { lcd.print(" off again"); } else {
    if (rpm < 10) { lcd.print("   "); } else
    if (rpm < 100) { lcd.print("  "); } else
    if (rpm < 1000) { lcd.print(" "); }
    lcd.print(rpm);
    lcd.print(" 1/min ");
  }
}

void showMode3() {
}

void showMode4() {
}

void showMode5() {
}

void showMode6() {
}

void showMode7() {
}

void showMode8() {
}

void showMode9() {
}

void loop() {
  taster = analogRead(A0);
  if (taster < 71) {                                        // r
    while (taster < 71) { taster = analogRead(A0); delay(10); }
  } else if (taster < 235) {                                // u
    if (mode == modes) { mode = 1; } else { mode++; }
    lcd.clear();
    while ((taster < 235) and (taster >= 71)) { taster = analogRead(A0); delay(10); }
  } else if (taster < 416) {                                // d
    if (mode == 1) { mode = modes; } else { mode--; }
    lcd.clear();
    while ((taster < 416) and (taster >= 253)) { taster = analogRead(A0); delay(10); }
  } else if (taster < 624) {                                // l
    while ((taster < 624) and (taster >= 416)) { taster = analogRead(A0); delay(10); }
  } else if (taster < 884) {                                // x
    while ((taster < 884) and (taster >= 624)) { taster = analogRead(A0); delay(10); }
  }
  if (CAN_MSGAVAIL == CAN.checkReceive()) {                 // check if data coming
    CAN.readMsgBuf(&canLen,canBuf);                         // read data; len: data length, buf: data buf
    canID = CAN.getCanId();
    Serial.print(canID,HEX);
    Serial.print(",");
    for (int i = 0; i < canLen; i++) {                      // print data (serial)
      Serial.print(canBuf[i],HEX);
      if (i < (canLen - 1)) { Serial.print(","); }
    }
    Serial.println();
    evalCAN();
  }
  if (millis() > (lastmillis + 1000)) {
         if (mode == 2) { showMode2(); }
    else if (mode == 3) { showMode3(); }
    else if (mode == 4) { showMode4(); }
    else if (mode == 5) { showMode5(); }
    else if (mode == 6) { showMode6(); }
    else if (mode == 7) { showMode7(); }
    else if (mode == 8) { showMode8(); }
    else if (mode == 9) { showMode9(); }
    else { showMode1(); }
    lastmillis = millis();
  }
}
