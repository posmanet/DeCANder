/*
 * DeCANder: Landrover Defender TD4 / TDCI - Arduino UNO - CAN-bus display/tool
 * display for additional information from the CAN-bus of a Land Rover Defender with the Ford "TD4" / "TDCI" / "duratorq" engine
 * Created 2019 by Posmanet (Martin) and released on github.com.
 * USE THIS CODE ON YOUR OWN RISK!
 */

// Constants
#define DeCANderVer "2.2"
#define modes 5                                             // number of available display modes; 9 MAX - see bottom lines
#define hotWarn 100                                         // We want to warn at this temperature.
#define notWarm 70                                          // engine still very cold... chillax!

// Variables
int taster = 0;                                             // ADC button value from LCD-keypad-shield
unsigned char canLen = 0;                                   // CAN
unsigned char canBuf[8];                                    // CAN
unsigned long canID = 0;                                    // CAN
int warte = 50;                                             // keypress delay
int mode = 1;                                               // selectable display mode (UP/DOWN keys)
bool sel = HIGH;                                            // select alternative display mode (SELECT key)
                                                            // ...used to display some yet unknown values
unsigned long lastmillis = 0;                               // display refresh timer
int hours = 0;                                              // uptime calculation variable
int minutes = 0;                                            // uptime calculation variable
int seconds = 0;                                            // uptime calculation variable
unsigned long times = 0;                                    // time calculations help variable (unsigned long for millis())
unsigned long trip = 0;                                     // trip timer
unsigned long tripStart = 0;                                // calculate trip timer
unsigned long tripStop = 0;                                 // calculate trip timer
bool ign = LOW;                                             // interpreted ignition [on/off]
bool pre = LOW;                                             // interpreted diesel preheat [on/off]
bool eng = LOW;                                             // interpreted engine [on/off]
float spd = 0;                                              // interpreted vehicle speed [km/h]
int rpm = 0;                                                // interpreted engine rpm [1/min]
float ful = 0;                                              // interpreted fuel consumption since start [ml; wraps after 50ml!]
float lastFul = 0;                                          // ml calculation help variable
float liters = 0;                                           // calculate ml -> liters
float lastLiters = 0;                                       // remember the last value
float mil = 0;                                              // interpreted vehicle mileage since start [m; wraps after 50m!]
float lastMil = 0;                                          // meters calculation help variable
float ml = 0;                                               // calculate ful -> ml
float km = 0;                                               // calculate meters -> km,1
float lastKm = 0;                                           // remember the last value
float meters = 0;                                           // calculate mil -> meters
float avg = 0;                                              // calculate average fuel consumption /100km since start
float momAvg = 0;                                           // calculate average fuel consumption /100km since last display
unsigned long momAvgMillis = 0;                             // calculate l/h since last Display
float ped = 0;                                              // interpreted gas pedal percentage [%]
int wat = 0;                                                // interpreted cooling water temperature [°C]
bool hot = false;                                           // alert bit for too hot cooling water (> hotWarn °C)
int un1 = 0;                                                // not yet interpreted but interesting looking value
int un2 = 0;                                                // not yet interpreted but interesting looking value
int un3 = 0;                                                // not yet interpreted but interesting looking value
int un4 = 0;                                                // not yet interpreted but interesting looking value
int un5 = 0;                                                // not yet interpreted but interesting looking value
int un6 = 0;                                                // not yet interpreted but interesting looking value
int un7 = 0;                                                // not yet interpreted but interesting looking value
int un8 = 0;                                                // not yet interpreted but interesting looking value
int un9 = 0;                                                // not yet interpreted but interesting looking value
int un10 = 0;                                               // not yet interpreted but interesting looking value

// Libraries
#include <LiquidCrystal.h>                                  // "LCD-keypad-shield from D1 Robot" using a 2x16 Hitachi HD44780 display
LiquidCrystal lcd(8, 9, 4, 5, 6, 7);                        // Disconnect pin 10! It conflicts with the CAN-shield!
#include "DeCANderCharacters.h"                             // We want some special characters on the LCD.
#include <SPI.h>                                            // sending serial data via Arduinos USB-Port
#include "mcp_can.h"                                        // CAN-bus-shield "DIGITAL V1.2 10/10/2013 by ElecFreaks"
                                                            // ...You have to get this one from the manufacturer and save in libraries!
MCP_CAN CAN(10);                                            // Set CS pin to Arduino pin 10 - is hardwired on my shield!

void setup() {
  Serial.begin(115200);
  Serial.print("DeCANder v");
  Serial.println(DeCANderVer);
  Serial.println("=============");
  Serial.println("connecting...");
  lcd.createChar(0,degC);
  lcd.createChar(1,play);
  lcd.createChar(2,average);
  lcd.createChar(3,snowflake);
  lcd.createChar(4,lr1);
  lcd.createChar(5,lr2);
  lcd.createChar(6,lr3);
  lcd.createChar(7,lr4);
  lcd.begin(16, 2);                                         // set up the LCD's number of columns and rows
  lcd.setCursor(4,0);
  lcd.write(byte(4));
  lcd.write(byte(5));
  lcd.write(byte(6));
  lcd.write(byte(6));
  lcd.write(byte(6));
  lcd.write(byte(5));
  lcd.write(byte(7));
  delay(2000);
  lcd.setCursor(0,1);
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
  if (canID == 297)  { 
    ful = float(canBuf[3]) / 5.12;
    if (ml < 50) {
      if (ful < lastFul) { ml = ful + 50; } else { ml = ful; }
    } else {
      if (ful < lastFul) { liters = liters + 0.1; ml = ful; } else { ml = ful + 50; }
    }
    if (ml > 99) { ml = 99; }
    lastFul = ful;
  }
  if (canID == 246)  {
    mil = float(canBuf[1]) / 5.12;
    if (meters < 50) {
      if (mil < lastMil) { meters = mil + 50; } else { meters = mil; }
    } else {
      if (mil < lastMil) { km = km + 0.1; meters = mil; } else { meters = mil + 50; }
    }
    if (meters > 99) { meters = 99; }
    lastMil = mil;
  }
  if (canID == 297)  { ped = float((canBuf[6] - 12) * 256 + canBuf[7]) / 10; }
  if (canID == 295)  { 
    wat = canBuf[6] - 60;
    if ((wat > hotWarn) and (hot == false)) { hot = true; mode = 2; } else  // alert here
    if ((wat < hotWarn) and (hot == true)) { hot = false; mode = 2; }       // de-alert =)
  }
  // here comes some unknown data:
  if (canID == 1218) { un1 = canBuf[4]; } else
  if (canID == 1024) { un2 = canBuf[0]; } else
  if (canID == 296)  { un3 = (canBuf[0] - 6) * 256 + canBuf[1]; } else
  if (canID == 296)  { un4 = (canBuf[2] - 8) * 256 + canBuf[3]; } else
  if (canID == 296)  { un5 = (canBuf[4] - 56) * 256 + canBuf[5]; } else
  if (canID == 295)  { un6 = canBuf[5]; } else // + 295.2??
  if (canID == 246)  { un7 = canBuf[2]; } else
  if (canID == 246)  { un8 = canBuf[3]; } else
  if (canID == 295)  { un9 = canBuf[3]; } else
  if (canID == 295)  { un10 = canBuf[4]; }
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
  lcd.print("rpm:  ");
  if (rpm <= 0) { lcd.print("engine off"); } else
  if (rpm > 9999) { lcd.print(" off again"); } else {
    if (rpm < 10) { lcd.print("   "); } else
    if (rpm < 100) { lcd.print("  "); } else
    if (rpm < 1000) { lcd.print(" "); }
    lcd.print(rpm);
    lcd.print(" 1/min");
  }
}

void showMode3() {
  lcd.setCursor(0,0);
  lcd.write(byte(1));
  lcd.print(" ");
  momAvg = 100 * (liters + ml / 1000 - lastLiters) / (km + meters / 1000 - lastKm);
  if (momAvg < 0) { momAvg = 0; }
  if (momAvg < 1000) {
    if (momAvg < 10) { lcd.print("  "); } else if (momAvg < 100) { lcd.print(" "); }
    lcd.print(momAvg,1); lcd.print(" l/100km ");
  } else {
    times = (millis() - momAvgMillis) / 1000;
//    fliess = times; // better use float() instead ?
    momAvg = (liters + ml / 1000 - lastLiters) / (float(times) / 60 / 60);
    if (momAvg < 0) { momAvg = 0; }
    if (momAvg < 1000) {
      if (momAvg < 10) { lcd.print("  "); } else if (momAvg < 100) { lcd.print(" "); }
      lcd.print(momAvg,1); lcd.print(" l/h    ");
    } else {
      lcd.print("momAvg > 1000");
    }
    momAvgMillis = millis();
  }
  lastLiters = liters + ml / 1000;
  lastKm = km + meters / 1000;
  lcd.setCursor(0,1);
  lcd.write(byte(2));
  lcd.print(" ");
  avg = 100 * (liters + ml / 1000) / (km + meters / 1000);
  if (avg < 0) { avg = 0; }
  if (avg < 1000) {
    if (avg < 10) { lcd.print("  "); } else if (avg < 100) { lcd.print(" "); }
    lcd.print(avg,1); lcd.print(" l/100km ");
  } else {
    times = (millis() - tripStart) / 1000;
//    fliess = times;
    avg = (liters + ml / 1000) / (float(times) / 60 / 60);
    if (avg < 0) { avg = 0; }
    if (avg < 1000) {
      if (avg < 10) { lcd.print("  "); } else if (avg < 100) { lcd.print(" "); }
      lcd.print(avg,1); lcd.print(" l/h    ");
    } else {
      lcd.print("   avg > 1000");
    }
  }
}

void showMode4() {
  lcd.setCursor(0,0);
  lcd.print("fuel: ");
  if (liters < 10) { lcd.print("  "); } else if (liters < 100) { lcd.print(" "); }
  lcd.print((liters + ml / 1000),3);
  lcd.print(" l ");
  lcd.setCursor(0,1);
  lcd.print("dist: ");
  if (km < 10) { lcd.print("  "); } else if (km < 100) { lcd.print(" "); }
  lcd.print((km + meters / 1000),3);
  lcd.print(" km");
}

void showMode5() {
  lcd.setCursor(0,0);
  lcd.print("pedal: ");
  if (ped < 10) { lcd.print("  "); } else if (ped < 100) { lcd.print(" "); }
  lcd.print(ped,1);
  lcd.print(" %  ");
  lcd.setCursor(0,1);
  lcd.print("water: ");
  if ((wat < 0) and (wat > -10)) { lcd.print(" "); } else if (wat < 10) { lcd.print("  "); } else if (wat < 100) { lcd.print(" "); }
  lcd.print(wat);
  lcd.print(".0 ");
  lcd.write(byte(0));
  lcd.print(" ");
  if (wat < notWarm) { lcd.write(byte(3)); } 
  else if (wat > hotWarn) { lcd.print("!"); } 
  else { lcd.print(" "); }
}

void showAltMode1() {                                          // unknown data
  lcd.setCursor(0,0);
  lcd.print("4C2.4 : ");
  if (un1 < 10) { lcd.print("    "); } else if (un1 < 100) { lcd.print("   "); } else if (un1 < 1000) { lcd.print("  "); } else if (un1 < 10000) { lcd.print(" "); } 
  lcd.print(un1);
  lcd.setCursor(0,1);
  lcd.print("400.0 : ");
  if (un2 < 10) { lcd.print("    "); } else if (un2 < 100) { lcd.print("   "); } else if (un2 < 1000) { lcd.print("  "); } else if (un2 < 10000) { lcd.print(" "); } 
  lcd.print(un2);
}

void showAltMode2() {                                          // unknown data
  lcd.setCursor(0,0);
  lcd.print("128.0f: ");
  if (un3 < 10) { lcd.print("    "); } else if (un3 < 100) { lcd.print("   "); } else if (un3 < 1000) { lcd.print("  "); } else if (un3 < 10000) { lcd.print(" "); } 
  lcd.print(un3);
  lcd.setCursor(0,1);
  lcd.print("128.2f: ");
  if (un4 < 10) { lcd.print("    "); } else if (un4 < 100) { lcd.print("   "); } else if (un4 < 1000) { lcd.print("  "); } else if (un4 < 10000) { lcd.print(" "); } 
  lcd.print(un4);
}

void showAltMode3() {                                          // unknown data
  lcd.setCursor(0,0);
  lcd.print("128.4f: ");
  if (un5 < 10) { lcd.print("    "); } else if (un5 < 100) { lcd.print("   "); } else if (un5 < 1000) { lcd.print("  "); } else if (un5 < 10000) { lcd.print(" "); } 
  lcd.print(un5);
  lcd.setCursor(0,1);
  lcd.print("127.5 : ");
  if (un6 < 10) { lcd.print("    "); } else if (un6 < 100) { lcd.print("   "); } else if (un6 < 1000) { lcd.print("  "); } else if (un6 < 10000) { lcd.print(" "); } 
  lcd.print(un6);
}

void showAltMode4() {                                          // unknown data
  lcd.setCursor(0,0);
  lcd.print(" F6.2 : ");
  if (un7 < 10) { lcd.print("    "); } else if (un7 < 100) { lcd.print("   "); } else if (un7 < 1000) { lcd.print("  "); } else if (un7 < 10000) { lcd.print(" "); } 
  lcd.print(un7);
  lcd.setCursor(0,1);
  lcd.print(" F6.3 : ");
  if (un8 < 10) { lcd.print("    "); } else if (un8 < 100) { lcd.print("   "); } else if (un8 < 1000) { lcd.print("  "); } else if (un8 < 10000) { lcd.print(" "); } 
  lcd.print(un8);
}

void showAltMode5() {                                          // unknown data
  lcd.setCursor(0,0);
  lcd.print("127.3 : ");
  if (un9 < 10) { lcd.print("    "); } else if (un9 < 100) { lcd.print("   "); } else if (un9 < 1000) { lcd.print("  "); } else if (un9 < 10000) { lcd.print(" "); } 
  lcd.print(un9);
  lcd.setCursor(0,1);
  lcd.print("127.4 : ");
  if (un10 < 10) { lcd.print("    "); } else if (un10 < 100) { lcd.print("   "); } else if (un10 < 1000) { lcd.print("  "); } else if (un10 < 10000) { lcd.print(" "); } 
  lcd.print(un10);
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
    if (sel == HIGH) { sel = LOW; } else { sel = HIGH; }
    lcd.clear();
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
         if (mode == 2) { if (sel == HIGH) { showMode2(); } else { showAltMode2(); } }
    else if (mode == 3) { if (sel == HIGH) { showMode3(); } else { showAltMode3(); } }
    else if (mode == 4) { if (sel == HIGH) { showMode4(); } else { showAltMode4(); } }
    else if (mode == 5) { if (sel == HIGH) { showMode5(); } else { showAltMode5(); } }
    else { mode = 1; if (sel == HIGH) { showMode1(); } else { showAltMode1(); } }
    lastmillis = millis();
  }
}
