/*
 * using LiquidCrystal Library (16x2 LCD display; Hitachi HD44780
 * -> Arduino-Pins D4-D9 plus A0 for the keys
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
 * Ardunino pin 13, 12, 11, 10 and 2 are occupied ! Jackpot!!
 */

#include <LiquidCrystal.h>
LiquidCrystal lcd(8, 9, 4, 5, 6, 7);
int taster = 0;                                        // ADC button value

void setup() {
  lcd.begin(16, 2);                                    // set up the LCD's number of columns and rows: 
  lcd.print("Mahlzaid!");
  delay(1000);
}

void loop() {
  lcd.setCursor(10, 0);
  lcd.print(millis()/1000);

  taster = analogRead(A0);

  lcd.setCursor(0, 1);
  if(taster<71) {
    lcd.print("r");
  } else if(taster<235) {
    lcd.print("o");
  } else if(taster<416) {
    lcd.print("u");
  } else if(taster<624) {
    lcd.print("l");
  } else if(taster<884) {
    lcd.print("x");
  } else {
    lcd.print(" ");
  }

  delay(100);

}
