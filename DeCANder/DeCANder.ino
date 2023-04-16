/*
 * DeCANder: Landrover Defender TD4 / TDCI 2,4l (2007-2011) - Arduino UNO - CAN-bus display/tool
 * --> The later 2,2l machines / ECUs are not supported because they use totally different PIDs.
 * display for additional information from the CAN-bus of a Land Rover Defender with the Ford "TD4" / "TDCI" / "duratorq" engine
 * Created 2019 by posmanet (Martin) and released on github.com.
 * USE THIS CODE ON YOUR OWN RISK!
 */
 
// constants
#define DeCANderVer "2.5"
/*
 * bugfix "Starts"
 * corrected dieselTank for my 110 (see comments)
 * activated hot variable and added tooHot variable
 * added hotMillis & tooHotMillis
 * added hotAlert (and a second warning/alert sound)
 * renamed lastMillis & initMillis (big letter M)
 * rpmMaxi --> EEPROM position 140 !!
 * revised showMode5() (show only max values now)
 * set default rpmMax & rpmMaxi = 0
 * revised EEdelete() - added parameter m (max data) (removed s and w)
 * revised EEwrite()
 * erasing all Data is in showMode2() now
 * added character arrow[8] (sum[8] has no longer use)
 * removed variables avgMax & avgMin & avgMinTank (no use)
 * we do not read any min oder max valies from EEprom any more, due to having trip-values in memory
 * revised tankstop-screen
 */
#define modes 5                                             // number of available display modes; 9 MAX for now - see bottom lines
#define hotAlert 105                                        // We want to alert at this temperature.
#define hotWarn 100                                         // We want to warn at this temperature.
                                                            // Defender emergency program kicks in at 111°C ...?
#define notWarm 70                                          // The engine is still very cold... chillax!
#define dieselTank 69                                       // Change this if you have messed with the original tank capacity.
                                                            // Defender TD4 2,4l 110 has 73 liters tank. (see manual; section "mainenance")
                                                            // (Defender TD4 2,4l 90 has 57 liters tank.)
                                                            // "a minimum of 4 litres will be required to restart the engine" -> 73-4=69 !
                                                            // "(warning Light) Illuminates when the fuel remaining in the tank drops to a minimum of 9 litres."
                                                            // "Once the fuel level has dropped to the point where the range is approximately 80 km the low fuel warning indicator will illuminate."
// misc variables
int taster = 0;                                             // ADC button value from LCD-keypad-shield
char keypressed = ' ';                                      // indicate some key still pressed and which ("" = no key pressed)
int warte = 50;                                             // keypress debounce delay
int mode = 1;                                               // selectable display mode (UP/DOWN keys)
bool sound = HIGH;                                          // to deactivate sounds

// CAN variables
unsigned char canLen = 0;                                   // CAN
unsigned char canBuf[8];                                    // CAN
unsigned long canID = 0;                                    // CAN

// calculations helper variables
unsigned long lastMillis = 0;                               // display refresh timer
unsigned long initMillis = 0;                               // init / delete data timer
unsigned long tooHotMillis = 0;                             // temperature alert timer
unsigned long hotMillis = 0;                                // temperature warning timer
unsigned long times = 0;                                    // time calculations help variable (unsigned long for millis())
unsigned long trip = 0;                                     // trip timer
unsigned long tripStart = 0;                                // calculate trip timer
unsigned long tripStop = 0;                                 // calculate trip timer
float ful = 0;                                              // interpreted fuel consumption since start [ml; wraps after 50ml!]
float lastFul = 0;                                          // ml calculation help variable
float ml = 0;                                               // calculate ful -> ml
float liters = 0;                                           // calculate ml -> liters
float lastLiters = 0;                                       // remember the last value
float mil = 0;                                              // interpreted vehicle mileage since start [m; wraps after 50m!]
float lastMil = 0;                                          // meters calculation help variable
float meters = 0;                                           // calculate mil -> meters
float km = 0;                                               // calculate meters -> km,1
float lastKm = 0;                                           // remember the last value
int EE_INT = 0;                                             // EEwrite check help and calculation variable
float EE_FLOAT = 0;                                         // EEwrite check help and calculation variable

// displayable values
int hours = 0;                                              // uptime calculation variable
int minutes = 0;                                            // uptime calculation variable
int seconds = 0;                                            // uptime calculation variable
int rpm = 0;                                                // interpreted engine rpm [1/min]
int wat = 0;                                                // interpreted cooling water temperature [°C]
bool ign = LOW;                                             // interpreted ignition [on/off]
bool pre = LOW;                                             // interpreted diesel preheat [on/off]
bool eng = LOW;                                             // interpreted engine [on/off]
bool hot = false;                                           // warn bit for too hot cooling water (> hotWarn °C)
bool tooHot = false;                                        // alert bit for too hot cooling water (> hotAlert °C)
float ped = 0;                                              // interpreted gas pedal percentage [%]
float spd = 0;                                              // interpreted vehicle speed [km/h]
float avg = 0;                                              // calculate average fuel consumption /100km (trip)
float momAvg = 0;                                           // calculate current fuel consumption /100km
unsigned long momAvgMillis = 0;                             // calculate l/h since last Display

// EEprom max values (100's) -> INT~2Byte. FLOAT~4Byte
float spdMax = 0;                                           // interpreted vehicle speed [km/h]
int watMax = 0;                                             // interpreted cooling water temperature [°C]
int rpmMax = 0;                                             // interpreted engine rpm [1/min]

// EEprom min values (200's)
int watMin = 50;                                            // interpreted cooling water temperature [°C]
int rpmMin = 1000;                                          // interpreted engine rpm [1/min]

// EEprom misc values (300's)
int startCount = 0;                                         // motor start counter
float kmTank = 0;                                           // calculate meters -> km,1 (km + meters / 1000)  = driven km
float litersTank = 0;                                       // calculate ml -> liters (liters + ml / 1000)    = consumed fuel
float kmAll = 0;                                            // calculate meters -> km,1 (km + meters / 1000)  = alltime driven km
float litersAll = 0;                                        // calculate ml -> liters (liters + ml / 1000)    = alltime consumed fuel

// Libraries
#include <LiquidCrystal.h>                                  // "LCD-keypad-shield from D1 Robot" using a 2x16 Hitachi HD44780 display
LiquidCrystal lcd(8, 9, 4, 5, 6, 7);                        // Disconnect pin 10! It conflicts with the CAN-shield!
#include "DeCANderCharacters.h"                             // We want some special characters on the LCD.
#include <SPI.h>                                            // sending serial data via Arduinos USB-Port
#include "mcp_can.h"                                        // CAN-bus-shield "DIGITAL V1.2 10/10/2013 by ElecFreaks"
                                                            // ...You have to get this one from the manufacturer and save in libraries!
MCP_CAN CAN(10);                                            // Set CS pin to Arduino pin 10 - is hardwired on my shield!
#include <EEPROM.h>                                         // We're using ATMEGA's internal EEPROM.

void setup() {
  pinMode(A1,OUTPUT);
  pinMode(A2,OUTPUT);
  digitalWrite(A2,LOW);
  Serial.begin(115200);
  Serial.print("DeCANder v");
  Serial.println(DeCANderVer);
  Serial.println("=============");
  Serial.println("connecting...");
  lcd.createChar(1,lr1);
  lcd.createChar(2,lr2);
  lcd.createChar(3,lr3);
  lcd.createChar(4,lr4);
  lcd.createChar(5,lr5);
  lcd.createChar(6,lr6);
  lcd.createChar(7,lr7);
  lcd.begin(16, 2);                                         // set up the LCD's number of columns and rows
  blingbling();
  delay(500);
  initCAN();
  delay(1000);
  lcd.clear();
  lcd.createChar(0,deg);
  lcd.createChar(1,coolant);
  lcd.createChar(2,average);
  lcd.createChar(3,play);
  lcd.createChar(4,gas1);
  lcd.createChar(5,gas2);
  lcd.createChar(6,gas3);
  lcd.createChar(7,gas4);
  EEread();
}

void blingbling() {
  lcd.setCursor(4,0);
  lcd.write(byte(1));
  lcd.write(byte(2));
  lcd.write(byte(3));
  lcd.write(byte(3));
  lcd.write(byte(3));
  lcd.write(byte(2));
  lcd.write(byte(4));
  delay(1000);
  lcd.setCursor(4,0);
  lcd.write(byte(6));
  lcd.setCursor(10,0);
  lcd.write(byte(7));
  delay(1000);
  lcd.setCursor(0,1);
  lcd.print(" DeCANder  v");
  lcd.print(DeCANderVer);
  tone(A1,2349,50);
  delay(100);
  tone(A1,1976,50);
  delay(100);
  tone(A1,2349,50);
  delay(100);
  tone(A1,1976,50);
  delay(500);
  lcd.setCursor(5,0);
  lcd.write(byte(5));
  lcd.setCursor(9,0);
  lcd.write(byte(5));
  delay(300);
  lcd.setCursor(5,0);
  lcd.write(byte(2));
  lcd.setCursor(9,0);
  lcd.write(byte(2));
  delay(500);
  lcd.setCursor(5,0);
  lcd.write(byte(5));
  lcd.setCursor(9,0);
  lcd.write(byte(5));
  delay(300);
  lcd.setCursor(5,0);
  lcd.write(byte(2));
  lcd.setCursor(9,0);
  lcd.write(byte(2));
  delay(500);
}

void EEdelete(char what) {                                  // delete selected values (t=tankstop; s=speeds; w=water temperatures; k=ALL-values)
  if (what == 't') {                                        // tankstop values
    EEPROM.put(340,10.0);                                   // default value litersTank (consumed)
    EEPROM.put(320,10.0);                                   // default value kmTank
    EEPROM.put(260,10.0);                                   // default value avgMinTank
  } else if (what == 'm') {                                 // maximum values
    EEPROM.put(120,100);                                    // default value watMax
    EEPROM.put(100,100.0);                                  // default value spdMax
    EEPROM.put(140,1000);                                   // default value rpmMax
    EEPROM.put(160,10.0);                                   // default value avgMax
  } else if (what == 'r') {                                 // remaining values
    EEPROM.put(220,0);                                      // default value watMin
    EEPROM.put(240,1000);                                   // default value rpmMin
    EEPROM.put(200,10.0);                                   // default value avgMin
    EEPROM.put(300,0);                                      // default value startCount
    EEPROM.put(380,4099.8);                                 // default value litersAll (consumed)     2019-08-09
    EEPROM.put(360,40306.0);                                // default value kmAll                    2019-08-09
  }
}

void EEerase() {                                            // erase eeprom and fill in default values
  for(int i = 0; i < EEPROM.length(); i++) { EEPROM.write(i,0); }
  EEdelete('t');
  EEdelete('m');
  EEdelete('r');
  EEread();
}

void EEread(){                                              // tuts EEPROM.get auch mit INT? Woher weiß er, wie viele Bytes er lesen muss?
  EEPROM.get(300,startCount);                               // motor start counter [1]
  EEPROM.get(320,kmTank);                                   // distance since tank stop [km]
  EEPROM.get(340,litersTank);                               // fuel since tank stop [l]
  EEPROM.get(360,kmAll);                                    // distance sum [km]
  EEPROM.get(380,litersAll);                                // fuel sum [l]
}

void EEwrite() {                                            // only write to EEPROM if values have changed
  EEPROM.get(100,EE_FLOAT);
  if (spdMax > EE_FLOAT) { EEPROM.put(100,spdMax); }
  EEPROM.get(120,EE_INT);
  if (watMax > EE_INT) { EEPROM.put(120,watMax); }
  EEPROM.get(140,EE_INT);
  if (rpmMax > EE_INT) { EEPROM.put(140,rpmMax); }
  EEPROM.get(160,EE_FLOAT);
  if ((avg > EE_FLOAT) and (avg < 50)) { EEPROM.put(160,avg); }   // keep avgMax < 50 l/100km.
  EEPROM.get(200,EE_FLOAT);
  if ((avg < EE_FLOAT) and (avg > 0)) { EEPROM.put(200,avg); }    // keep avgMin > 0 l/100km
  EEPROM.get(220,EE_INT);
  if (watMin < EE_INT) { EEPROM.put(220,watMin); }
  EEPROM.get(240,EE_INT);
  if (rpmMin < EE_INT) { EEPROM.put(240,rpmMin); }
  EEPROM.get(260,EE_FLOAT);
  if ((avg < EE_FLOAT) and (avg > 0)) { EEPROM.put(260,avg); }    // keep avgMinTank > 0 l/100km
  EEPROM.get(300,EE_INT);
  if (startCount > EE_INT) { EEPROM.put(300,startCount); }
  EEPROM.get(320,EE_FLOAT);
  if (kmTank > EE_FLOAT) { EEPROM.put(320,kmTank); }
  EEPROM.get(340,EE_FLOAT);
  if (litersTank > EE_FLOAT) { EEPROM.put(340,litersTank); }
  EEPROM.get(360,EE_FLOAT);
  if (kmAll > EE_FLOAT) { EEPROM.put(360,kmAll); }
  EEPROM.get(380,EE_FLOAT);
  if (litersAll > EE_FLOAT) { EEPROM.put(380,litersAll); }
}

void keysCheck() {
  taster = analogRead(A0);
  if (keypressed == ' ') {
    if (taster < 71) {                                        // right
      if (mode == modes) { mode = 1; } else { mode++; }
      lcd.clear();
      keypressed = 'r';
    } else if (taster < 235) {                                // up
      keypressed = 'u';
    } else if (taster < 416) {                                // down
      if (sound == HIGH) { sound = LOW; } else { sound = HIGH; }
      keypressed = 'd';
    } else if (taster < 624) {                                // left
      if (mode == 1) { mode = modes; } else { mode--; }
      lcd.clear();
      keypressed = 'l';
    } else if (taster < 884) {                                // select
      initMillis = millis();
      keypressed = 's';
    }
  } else {
    if (taster > 1000) { keypressed = ' '; }
  }
}

void initCAN() {
  while (CAN_OK != CAN.begin(CAN_500KBPS)) {                // init can bus : baudrate TD4 = 500k
    lcd.setCursor(0,1);
    lcd.print(" CAN init fail. ");
    delay(1000);
  }
  lcd.setCursor(0,1);
  lcd.print(" CAN init done! ");
}

void evalCAN() {
  if (canID == 1218) { if (canBuf[3] < 80) { ign = HIGH; } else if (canBuf[3] > 130) { ign = LOW; } }
  if (canID == 295)  { if (canBuf[7] == 1) { pre = HIGH;} else if (canBuf[7] == 0) { pre = LOW; } }
  if (canID == 297)  {
    if (canBuf[2] == 3) {
      if (eng == LOW) {
        tripStart = millis();
        eng = HIGH;
        startCount = startCount + 1;
        if (startCount > 9999) { startCount = startCount - 10000; }
        liters = 0;
        ml = 0;
        km = 0;
        meters = 0;
        mode = 2;
      }
    } else if (canBuf[2] == 0) {
      if (eng == HIGH) {
        tripStop = millis();
        eng = LOW;
        EEwrite();
        mode = 3;
      }
    }
  }
  if (canID == 907)  { 
    spd = (canBuf[4] * 256 + canBuf[5]) / 100;
    if (spd > spdMax) { spdMax = spd; }
  }
  if (canID == 246)  {
    rpm = (canBuf[4] - 96) * 256 + canBuf[5];
    if ((rpm < 9999) and (rpm > rpmMax)) { rpmMax = rpm; }
    if ((rpm > 0) and (rpm < rpmMin)) { rpmMin = rpm; }
  }
  if (canID == 297)  { 
    ful = float(canBuf[3]) / 5.12;
    if (ml < 50) {
      if (ful < lastFul) { ml = ful + 50; } else { ml = ful; }
    } else {
      if (ful < lastFul) { liters = liters + 0.1; ml = ful; } else { ml = ful + 50; }
    }
    if (ml > 99) { ml = 99; }
    lastFul = ful;
    // calculate litersAll, litersTank
    EEPROM.get(380,EE_FLOAT);
    litersAll = EE_FLOAT + (liters + ml / 1000);
    EEPROM.get(340,EE_FLOAT);
    litersTank = EE_FLOAT  + (liters + ml / 1000);
    // calculate fuel consumption
    avg = 100 * (liters + ml / 1000) / (km + meters / 1000);
    if (avg < 0) { avg = 0; }
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
    // calculate kmAll, kmTank
    EEPROM.get(360,EE_FLOAT);
    kmAll = EE_FLOAT + (km + meters / 1000);
    EEPROM.get(320,EE_FLOAT);
    kmTank = EE_FLOAT  + (km + meters / 1000);
    // calculate fuel consumption
    avg = 100 * (liters + ml / 1000) / (km + meters / 1000);
  }
  if (canID == 297)  { ped = float((canBuf[6] - 12) * 256 + canBuf[7]) / 10; }
  if (canID == 295)  { 
    wat = canBuf[6] - 60;
    if ((wat > hotAlert) and (tooHot == false)) { tooHot = true; tooHotMillis = millis(); } else     // set alert BIT here
    if ((wat < hotAlert) and (tooHot == true)) { tooHot = false; tooHotMillis = 0; }                 // de-alert =)
    if ((wat > hotWarn) and (hot == false)) { hot = true; hotMillis = millis(); } else               // set warn BIT here
    if ((wat < hotWarn) and (hot == true)) { hot = false; hotMillis = 0; }                           // de-warn =)
    if ((tooHot == true) and (millis() - tooHotMillis < 500)) {
      tone(A1,2349);
    } else if ((hot == true) and (millis() - hotMillis < 50)) {
      tone(A1,2349);
    } else {
      noTone(A1);
    }
//old    if ((wat > hotWarn) and (((millis()/1000)%2)==0) and (sound == HIGH)) { tone(A1,2349,50); } // acoustic alert
    if ((wat < 200) and (wat > watMax)) { watMax = wat; }
    if ((wat > -30) and (wat < watMin)) { watMin = wat; }
  }
}

void soundOnOff() {
    lcd.setCursor(0,0);
    if (sound == HIGH) { lcd.print("sound turned on "); } else { lcd.print("sound turned off"); }
    lcd.setCursor(0,1);
    lcd.print("                ");
}

void showMode1() {                                          // startup-screen (no icon)
  if (keypressed == 's') {
    lcd.setCursor(0,0);
    lcd.print("Hold SELECT to  ");
    lcd.setCursor(0,1);
    lcd.print("erase values!   ");
  } else if (keypressed == 'd') {
    soundOnOff();
  } else if (keypressed == 'u') {
    lcd.setCursor(0,0);
    lcd.print("...since powerup");
    lcd.setCursor(0,1);
    lcd.print("   [ign pre eng]");
  } else {
    lcd.setCursor(0,0);
    if ((tripStart == 0) or ((millis() - tripStart) < 60000)) {
      lcd.print(" uptime   :  :  ");
      times = millis() / 1000;
    } else {
      lcd.print("   trip=  :  :  ");
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
    lcd.setCursor(8,0);
    if (hours < 10) { lcd.print("0"); }
    lcd.print(hours);
    lcd.setCursor(11,0);
    if (minutes < 10) { lcd.print("0"); }
    lcd.print(minutes);
    lcd.setCursor(14,0);
    if (seconds < 10) { lcd.print("0"); }
    lcd.print(seconds);
    lcd.setCursor(0,1);
    if (startCount == 1) { lcd.print("     start [   ]"); } else { lcd.print("     starts[   ]"); }
    lcd.setCursor(0,1);
    if (startCount < 10) { lcd.print("   "); } else if (startCount < 100) { lcd.print("  "); } else if (startCount < 1000) { lcd.print(" "); }
    lcd.print(startCount);
    lcd.setCursor(12,1);
    lcd.print(ign);
    lcd.print(pre);
    lcd.print(eng);
  }
}

void showMode2() {                                          // driving-screen with current values (play-icon)
  if (keypressed == 's') {
    lcd.setCursor(0,0);
    lcd.print("Erasing ALL data");
    lcd.setCursor(0,1);
    lcd.print("in x seconds !!!");
    if (((millis() - initMillis) < 5000) and (initMillis > 0)) {
      lcd.setCursor(3,1);
      lcd.print(5-((millis() - initMillis)/1000));
    } else if (initMillis != 0) {
      initMillis = 0;
      lcd.setCursor(0,1);
      lcd.print(" in progress... ");
      EEerase();
    } else {
      lcd.setCursor(0,0);
      lcd.print("ALL data erased!");
      lcd.setCursor(0,1);
      lcd.print("(release SELECT)");
    }
  } else if (keypressed == 'd') {
    soundOnOff();
  } else if (keypressed == 'u') {
    lcd.setCursor(0,0);
    lcd.print(" v_current 1/min");
    lcd.setCursor(0,0);
    lcd.write(byte(3));
    lcd.setCursor(0,1);
    if (spd < 5) { lcd.print("       %   l/h  "); } else { lcd.print("       % l/100km"); }
    lcd.setCursor(4,1);
    lcd.write(byte(1));
  } else {
    lcd.setCursor(0,0);
    lcd.print("     km/h      r");
    lcd.setCursor(0,0);
    lcd.write(byte(3));
    lcd.setCursor(0,1);
    lcd.print("      C        l");
    lcd.setCursor(5,1);
    lcd.write(byte(0));
    lcd.setCursor(0,1);
    if ((wat < notWarm) and (((millis()/1000)%2)==0)) { lcd.print("*"); }
    if ((wat > hotWarn) and (((millis()/1000)%2)==0)) { lcd.print("!"); }
    lcd.setCursor(2,0);
    if (spd < 10) { lcd.print("  "); } else
    if (spd < 100) { lcd.print(" "); }
    lcd.print(spd,0);
    lcd.setCursor(11,0);
    if (rpm <= 0) { lcd.print("  off"); } else
    if (rpm > 9999) { lcd.print("  off"); } else {
      if (rpm < 10) { lcd.print("   "); } else
      if (rpm < 100) { lcd.print("  "); } else
      if (rpm < 1000) { lcd.print(" "); }
      lcd.print(rpm);
    }
    lcd.setCursor(2,1);
    if ((wat < 0) and (wat > -10)) { lcd.print(" "); } else
    if (wat < 10) { lcd.print("  "); } else
    if (wat < 100) { lcd.print(" "); }
    lcd.print(wat);
    lcd.setCursor(7,1);
    if (ped==0) { lcd.print("    "); } else
    if (ped<25) { lcd.write(byte(4)); lcd.print("   "); } else
    if (ped<50) { lcd.write(byte(4)); lcd.write(byte(5)); lcd.print("  "); } else
    if (ped<75) { lcd.write(byte(4)); lcd.write(byte(5)); lcd.write(byte(6)); lcd.print(" "); } else 
                { lcd.write(byte(4)); lcd.write(byte(5)); lcd.write(byte(6)); lcd.write(byte(7)); }
//    if (ped==0) { lcd.print("    "); } else
//    if (ped<25) { lcd.print("_   "); } else
//    if (ped<50) { lcd.print("__  "); } else
//    if (ped<75) { lcd.print("___ "); } else { lcd.print("____"); }
    lcd.setCursor(11,1);
    momAvg = 100 * (liters + ml / 1000 - lastLiters) / (km + meters / 1000 - lastKm);
    if (momAvg < 0) { momAvg = 0; }
    if (momAvg < 100) {
      if (momAvg < 10) { lcd.print(" "); }
      lcd.print(momAvg,1);
    } else {
      times = (millis() - momAvgMillis) / 1000;
      momAvg = (liters + ml / 1000 - lastLiters) / (float(times) / 60 / 60);
      if (momAvg < 0) { momAvg = 0; }
      if (momAvg < 100) {
        if (momAvg < 10) { lcd.print(" "); }
        lcd.print(momAvg,1);
      } else {
        lcd.print("> 99");
      }
      momAvgMillis = millis();
    }
    lastLiters = liters + ml / 1000;
    lastKm = km + meters / 1000;
  }
}

void showMode3() {                                          // trip-screen with current or last trip values (hourglass-icon)
  if (keypressed == 's') {
    lcd.setCursor(0,0);
    lcd.print("Erasing trip    ");
    lcd.setCursor(0,1);
    lcd.print("data in x sec...");
    if (((millis() - initMillis) < 5000) and (initMillis > 0)) {
      lcd.setCursor(8,1);
      lcd.print(5-((millis() - initMillis)/1000));
    } else if (initMillis != 0) {
      initMillis = 0;
      lcd.setCursor(0,1);
      lcd.print("data in progress");
      km = 0;
      rpmMax = 0;
      watMax = 0;
      // whatabout consumption? (killing litres is no good for the statistics?)
    } else {
      lcd.setCursor(0,0);
      lcd.print("...data erased! ");
      lcd.setCursor(0,1);
      lcd.print("(release SELECT)");
    }
  } else if (keypressed == 'd') {
    soundOnOff();
  } else if (keypressed == 'u') {
    lcd.setCursor(0,0);
    lcd.print("   trip    1/min");
    lcd.setCursor(0,0);
    lcd.write(byte(3));
    lcd.setCursor(0,1);
    lcd.print("         l/100km");
    lcd.setCursor(4,1);
    lcd.write(byte(1));
  } else {
    lcd.setCursor(0,0);
    lcd.print("       km      r");
    lcd.setCursor(0,0);
    lcd.write(byte(3));
    lcd.setCursor(0,1);
    lcd.print("      C        l");
    lcd.setCursor(5,1);
    lcd.write(byte(0));
    lcd.setCursor(2,0);
    if (km < 10) { lcd.print("  "); } else if (km < 100) { lcd.print(" "); }
    lcd.print((km + meters / 1000),1);
    lcd.setCursor(11,0);
    if (rpmMax > 9999) { lcd.print("Erpm"); } else {
      if (rpmMax < 10) { lcd.print("   "); } else
      if (rpmMax < 100) { lcd.print("  "); } else
      if (rpmMax < 1000) { lcd.print(" "); }
      lcd.print(rpmMax);
    }
    lcd.setCursor(2,1);
    if ((watMax < 10) and (watMax > -1)) { lcd.print("  "); } else
    if (((watMax < 100) and (watMax > 9)) or ((watMax < 0) and (watMax > -10))) { lcd.print(" "); }
    lcd.print(watMax);
    lcd.setCursor(10,1);
    if (avg < 100) {
      if (avg < 10) { lcd.print(" "); }
      lcd.write(byte(2));
      lcd.print(avg,1);
    } else {
      times = (millis() - tripStart) / 1000;
      avg = liters / (float(times) / 60 / 60);
      if (avg < 0) { avg = 0; }
      if (avg < 100) {
        if (avg < 10) { lcd.print(" "); }
        lcd.print(avg,1);
      } else {
        lcd.print("> 99");
      }
    }
  }
}

void showMode4() {                                          // tankstop-screen with values since last tankstop (reset when filling up!) (gaspump-icon)
  if (keypressed == 's') {
    lcd.setCursor(0,0);
    lcd.print("Erasing tankstop");
    lcd.setCursor(0,1);
    lcd.print("data in x sec...");
    if (((millis() - initMillis) < 5000) and (initMillis > 0)) {
      lcd.setCursor(8,1);
      lcd.print(5-((millis() - initMillis)/1000));
    } else if (initMillis != 0) {
      initMillis = 0;
      lcd.setCursor(0,1);
      lcd.print("data in progress");
      EEdelete('t');
    } else {
      lcd.setCursor(0,0);
      lcd.print("...data erased! ");
      lcd.setCursor(0,1);
      lcd.print("(release SELECT)");
    }
  } else if (keypressed == 'd') {
    soundOnOff();
  } else if (keypressed == 'u') {
    lcd.setCursor(0,0);
    lcd.print("range (distance)");
    lcd.setCursor(0,0);
    lcd.write(byte(3));
    lcd.setCursor(0,1);
    lcd.print("  diesel l/100km");
  } else {
    lcd.setCursor(0,0);
    lcd.print("      km (   km)");
    lcd.setCursor(0,0);
    lcd.write(byte(3));
    lcd.setCursor(0,1);
    lcd.print("     /  l      l");
    
    lcd.setCursor(3,0);
    if (EE_FLOAT != 0) { EE_FLOAT = 100 * (dieselTank - litersTank) / EE_FLOAT; } 
    else if (litersAll != 0) { EE_FLOAT = 100 * (dieselTank - litersTank) / (100 * litersAll / kmAll); }
    else { EE_FLOAT = 0; }
    if (EE_FLOAT < 9.95) { lcd.print("  "); } else if (EE_FLOAT < 99.95) { lcd.print(" "); }
    lcd.print(EE_FLOAT,0);
    
    lcd.setCursor(10,0);
    if (kmTank < 10) { lcd.print("  "); } else
    if (kmTank < 100) { lcd.print(" "); }
    lcd.print(kmTank,0);

    lcd.setCursor(3,1);
    EE_FLOAT = dieselTank - litersTank;
    if (EE_FLOAT < 9.95) { lcd.print(" "); }
    lcd.print(EE_FLOAT,0);
    lcd.setCursor(6,1);
    lcd.print(dieselTank);
    
    lcd.setCursor(10,1);
    if (kmTank != 0) { EE_FLOAT = 100 * litersTank / kmTank; } else { EE_FLOAT = 0; }
    if (EE_FLOAT < 9.95) { lcd.print(" "); }
    lcd.write(byte(2));
    lcd.print(EE_FLOAT,1);
  }
}

void showMode5() {                                          // max-screen with all-time maximum values (arrow-icon)
  if (keypressed == 's') {
    lcd.setCursor(0,0);
    lcd.print("Erasing max data");
    lcd.setCursor(0,1);
    lcd.print("in x seconds !!!");
    if (((millis() - initMillis) < 5000) and (initMillis > 0)) {
      lcd.setCursor(3,1);
      lcd.print(5-((millis() - initMillis)/1000));
    } else if (initMillis != 0) {
      initMillis = 0;
      lcd.setCursor(0,1);
      lcd.print(" in progress... ");
      EEdelete('m');
    } else {
      lcd.setCursor(0,0);
      lcd.print("Max data erased!");
      lcd.setCursor(0,1);
      lcd.print("(release SELECT)");
    }
  } else if (keypressed == 'd') {
    soundOnOff();
  } else if (keypressed == 'u') {
    lcd.setCursor(0,0);
    lcd.print("  v_max    r_max");
    lcd.setCursor(0,0);
    lcd.write(byte(3));
    lcd.setCursor(0,1);
    lcd.print("   _max     _max");
    lcd.setCursor(2,1);
    lcd.write(byte(1));
    lcd.setCursor(11,1);
    lcd.write(byte(2));
  } else {
    lcd.setCursor(0,0);
    lcd.print("     km/h      r");
    lcd.setCursor(0,0);
    lcd.write(byte(3));
    lcd.setCursor(0,1);
    lcd.print("      C        l");
    lcd.setCursor(5,1);
    lcd.write(byte(0));
    lcd.setCursor(10,1);
    lcd.write(byte(2));
    lcd.setCursor(2,0);
    EEPROM.get(100,EE_FLOAT); // read v_max
    if (EE_FLOAT < 9.95) { lcd.print("  "); } else if (EE_FLOAT < 99.95) { lcd.print(" "); }
    lcd.print(EE_FLOAT,0);
    lcd.setCursor(11,0);
    EEPROM.get(140,EE_INT); // read r_max
    if (EE_INT > 9999) { lcd.print("Erpm"); } else {
      if (EE_INT < 10) { lcd.print("   "); } else
      if (EE_INT < 100) { lcd.print("  "); } else
      if (EE_INT < 1000) { lcd.print(" "); }
      lcd.print(EE_INT);
    }
    lcd.setCursor(2,1);
    EEPROM.get(120,EE_INT); // read temp_max
    if (EE_INT > 999) { lcd.print("Ewm"); } else {
      if (EE_INT < 10) { lcd.print("  "); } else
      if (EE_INT < 100) { lcd.print(" "); }
      lcd.print(EE_INT);
    }
    lcd.setCursor(11,1);
    EEPROM.get(160,EE_FLOAT); // read avg_max
    if (EE_FLOAT < 9.95) { lcd.print(" "); }
    lcd.print(EE_FLOAT,1);
  }
}

void loop() {
  keysCheck();
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
  if (millis() > (lastMillis + 1000)) {
    if (mode == 2) { lcd.createChar(3,play); showMode2(); } else
    if (mode == 3) { lcd.createChar(3,hourglass); showMode3(); } else
    if (mode == 4) { lcd.createChar(3,tank); showMode4(); } else
    if (mode == 5) { lcd.createChar(3,arrow); showMode5(); } else 
                   { mode = 1; showMode1(); }
    lastMillis = millis();
  }
}
