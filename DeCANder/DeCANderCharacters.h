#include <Arduino.h>
// We define special charakters here for the LCD.

byte degC[8] = {
  B11000,
  B11000,
  B00110,
  B01001,
  B01000,
  B01001,
  B00110,
};
byte average[8] = {
  B00001,
  B01110,
  B10011,
  B10101,
  B11001,
  B01110,
  B10000,
};
byte play[8] = {
  B00000,
  B01000,
  B01100,
  B01110,
  B01100,
  B01000,
  B00000,
};
byte trips[8] = {
  B00000,
  B01110,
  B10101,
  B10111,
  B10001,
  B01110,
  B00000,
};
byte snowflake[8] = {
  B00100,
  B10101,
  B01110,
  B00100,
  B01110,
  B10101,
  B00100,
};
byte degr[8] = {
  B00100,
  B01010,
  B00100,
  B00000,
  B00000,
  B00000,
  B00000,
};

// ******************** These are to display the defender front:
byte lr1[8] = {
  B10010,
  B10101,
  B10010,
  B10000,
  B10010,
  B10101,
  B10010,
};
byte lr2[8] = {
  B00000,
  B01110,
  B10001,
  B10001,
  B10001,
  B01110,
  B00000,
};
byte lr3[8] = {
  B11111,
  B00000,
  B11111,
  B00000,
  B11111,
  B00000,
  B11111,
};
byte lr4[8] = {
  B01001,
  B10101,
  B01001,
  B00001,
  B01001,
  B10101,
  B01001,
};
// ********************
