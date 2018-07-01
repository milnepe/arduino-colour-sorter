/*
    File:     ColourSorter.ino
    Author:   Peter Milne
    License:  GNU GENERAL PUBLIC LICENSE (see license.txt)

    Arduino Colour Sorter demonstrating stepper motor and servo motor control

*/

/* Adafruit TCS34725 breakout library */

/* Connect SCL    to analog 5
   Connect SDA    to analog 4
   Connect VDD    to 3.3V DC
   Connect GROUND to common ground
*/

#include <Wire.h>
#include "Adafruit_TCS34725.h"
#include <Servo.h>

const int dwell_time = 500;

//Define EasyDriver pins
#define stp 8    // stp pin connected to D8
#define dir 9    // dir pin connected to D9
// #define MS1   MS1 & MS2 not connected (default 1/8th step resoluion)
// #define MS2
#define EN  10    // EN connected to D10
#define servo 12  // Servo connection D12

// Define directions
#define REVERSE HIGH // Move in reverse4
#define FORWARD  LOW // Move forwards

// 5 equal positions for platter
const int POSITIONS = 5;
// Number of micro-steps to move one position at 1/8th step resolution (Default)
const int ONE_POSITION = 200 / POSITIONS * 8;

// Stores position from Origin of each index (Origin is always zero)
int indexArray[] = {0, 4, 3, 2, 1};

Servo myservo;  // create servo object to control a servo
int pos = 0;    // variable to store the servo position

// Colour sensor code
//
// An experimental wrapper class that implements the improved lux and color temperature from
// TAOS and a basic autorange mechanism.
//
// Written by ductsoup, public domain
//

// RGB Color Sensor with IR filter and White LED - TCS34725
// I2C 7-bit address 0x29, 8-bit address 0x52
//
// http://www.adafruit.com/product/1334
// http://learn.adafruit.com/adafruit-color-sensors/overview
// http://www.adafruit.com/datasheets/TCS34725.pdf
// http://www.ams.com/eng/Products/Light-Sensors/Color-Sensor/TCS34725
// http://www.ams.com/eng/content/view/download/265215 <- DN40, calculations
// http://www.ams.com/eng/content/view/download/181895 <- DN39, some thoughts on autogain
// http://www.ams.com/eng/content/view/download/145158 <- DN25 (original Adafruit calculations)
//
// connect LED to digital 4 or GROUND for ambient light sensing
// connect SCL to analog 5
// connect SDA to analog 4
// connect Vin to 3.3-5V DC
// connect GROUND to common ground

// some magic numbers for this device from the DN40 application note
#define TCS34725_R_Coef 0.136
#define TCS34725_G_Coef 1.000
#define TCS34725_B_Coef -0.444
#define TCS34725_GA 1.0
#define TCS34725_DF 310.0
#define TCS34725_CT_Coef 3810.0
#define TCS34725_CT_Offset 1391.0

// Autorange class for TCS34725
class tcs34725 {
  public:
    tcs34725(void);

    boolean begin(void);
    void getData(void);

    boolean isAvailable, isSaturated;
    uint16_t againx, atime, atime_ms;
    uint16_t r, g, b, c;
    uint16_t ir;
    uint16_t r_comp, g_comp, b_comp, c_comp;
    uint16_t saturation, saturation75;
    float cratio, cpl, ct, lux, maxlux;

  private:
    struct tcs_agc {
      tcs34725Gain_t ag;
      tcs34725IntegrationTime_t at;
      uint16_t mincnt;
      uint16_t maxcnt;
    };
    static const tcs_agc agc_lst[];
    uint16_t agc_cur;

    void setGainTime(void);
    Adafruit_TCS34725 tcs;
};
//
// Gain/time combinations to use and the min/max limits for hysteresis
// that avoid saturation. They should be in order from dim to bright.
//
// Also set the first min count and the last max count to 0 to indicate
// the start and end of the list.
//
const tcs34725::tcs_agc tcs34725::agc_lst[] = {
  { TCS34725_GAIN_60X, TCS34725_INTEGRATIONTIME_700MS,     0, 47566 },
  { TCS34725_GAIN_16X, TCS34725_INTEGRATIONTIME_154MS,  3171, 63422 },
  { TCS34725_GAIN_4X,  TCS34725_INTEGRATIONTIME_154MS, 15855, 63422 },
  { TCS34725_GAIN_1X,  TCS34725_INTEGRATIONTIME_2_4MS,   248,     0 }
};
tcs34725::tcs34725() : agc_cur(0), isAvailable(0), isSaturated(0) {
}

// initialize the sensor
boolean tcs34725::begin(void) {
  tcs = Adafruit_TCS34725(agc_lst[agc_cur].at, agc_lst[agc_cur].ag);
  if ((isAvailable = tcs.begin()))
    setGainTime();
  return (isAvailable);
}

// Set the gain and integration time
void tcs34725::setGainTime(void) {
  tcs.setGain(agc_lst[agc_cur].ag);
  tcs.setIntegrationTime(agc_lst[agc_cur].at);
  atime = int(agc_lst[agc_cur].at);
  atime_ms = ((256 - atime) * 2.4);
  switch (agc_lst[agc_cur].ag) {
    case TCS34725_GAIN_1X:
      againx = 1;
      break;
    case TCS34725_GAIN_4X:
      againx = 4;
      break;
    case TCS34725_GAIN_16X:
      againx = 16;
      break;
    case TCS34725_GAIN_60X:
      againx = 60;
      break;
  }
}

// Retrieve data from the sensor and do the calculations
void tcs34725::getData(void) {
  // read the sensor and autorange if necessary
  tcs.getRawData(&r, &g, &b, &c);
  while (1) {
    if (agc_lst[agc_cur].maxcnt && c > agc_lst[agc_cur].maxcnt)
      agc_cur++;
    else if (agc_lst[agc_cur].mincnt && c < agc_lst[agc_cur].mincnt)
      agc_cur--;
    else break;

    setGainTime();
    delay((256 - atime) * 2.4 * 2); // shock absorber
    tcs.getRawData(&r, &g, &b, &c);
    break;
  }

  // DN40 calculations
  ir = (r + g + b > c) ? (r + g + b - c) / 2 : 0;
  r_comp = r - ir;
  g_comp = g - ir;
  b_comp = b - ir;
  c_comp = c - ir;
  cratio = float(ir) / float(c);

  saturation = ((256 - atime) > 63) ? 65535 : 1024 * (256 - atime);
  saturation75 = (atime_ms < 150) ? (saturation - saturation / 4) : saturation;
  isSaturated = (atime_ms < 150 && c > saturation75) ? 1 : 0;
  cpl = (atime_ms * againx) / (TCS34725_GA * TCS34725_DF);
  maxlux = 65535 / (cpl * 3);

  lux = (TCS34725_R_Coef * float(r_comp) + TCS34725_G_Coef * float(g_comp) + TCS34725_B_Coef * float(b_comp)) / cpl;
  ct = TCS34725_CT_Coef * float(b_comp) / float(r_comp) + TCS34725_CT_Offset;
}

tcs34725 rgb_sensor;

void setup() {
  pinMode(stp, OUTPUT);
  pinMode(dir, OUTPUT);
  pinMode(EN, OUTPUT);
  digitalWrite(EN, HIGH); // Disable motor
  Serial.begin(115200);
  if (rgb_sensor.begin()) {
    Serial.println("Found sensor");
  } else {
    Serial.println("No TCS34725 found ... check your connections");
    while (1);
  }

  pinMode(4, OUTPUT);
  digitalWrite(4, LOW); // @gremlins Bright light, bright light!

  myservo.attach(servo);  // attaches the servo on pin 12 to the servo object
  myservo.write(0); // initialise servo position

  delay(500);
  Serial.println("Begin test");
}

void loop() {

  digitalWrite(EN, HIGH); // Disable motor
  int colour_index;

  moveToSensor();
  delay(dwell_time);

  setPosition(getIndex());

  digitalWrite(EN, HIGH); // Disable motor
  //showIndex();
  delay(dwell_time);

  moveToEjector();
  delay(dwell_time);

  moveToLoader();
  delay(dwell_time);

}

int getIndex() {
  int index = 0;
  int colour = getColourTemp();
  //Orange;
  if (colour <= 3878) {
    Serial.println("Orange");
    return index = 0;
  }
  if (colour > 3878 && colour <= 4183) {
    Serial.println("Yellow");
    return index = 1;
  }
  if (colour > 4183 && colour <= 4665) {
    Serial.println("Red");
    return index = 2;
  }
  if (colour > 4665 && colour <= 5003) {
    Serial.println("Green");
    return index = 3;
  }
  if (colour > 5003 ) {
    Serial.println("Purple");
    return index = 4 ;
  }
  // Fail
  Serial.println("Oops");
  return -1;
}



int getColourTemp() {
  rgb_sensor.getData();

  int colour_temp = rgb_sensor.ct;

  return colour_temp;
}

// Move any valid index to Origin
void setPosition(int index) {
  int direction;
  // Sanity check index
  if ((index >= 0) && (index < POSITIONS)) {

    int p = indexArray[index]; // Get number of positions from Origin

    if ( !p ) return;    // Check if already at Origin

    // Work out best direction
    if ( p > ( POSITIONS % 3 )) {
      p = POSITIONS - p;
      direction = REVERSE;
    }
    else direction = FORWARD;

    move(p, direction); // Move in the direction the number of positions
  }
  else return; // Do nothing if index is out of range
}

// Steps motor the given number of positions and direction
// Each position is made up of multiple micro-steps according to the sepecific motor
void move(int positions, int direction) {
  digitalWrite(dir, direction); // Direction pin low for forwards / high for reverse
  digitalWrite(EN, LOW); // Enable pin low to allow motor control
  delay(1);

  int steps = ONE_POSITION * positions;   // Calculate number of microsteps

  for (int i = 0; i < steps; i++)   // Move the number of micro-steps
  {
    digitalWrite(stp, HIGH); //Trigger one step forward
    delay(1);
    digitalWrite(stp, LOW); // Pull step pin low so it can be triggered again
    delay(1);
  }

  for (int j = 0; j < positions; j++)   // Update index after each position
    updateIndex(direction);
}

// Recalculate position from Origin of each Index after moving one position
void updateIndex(int direction) {
  for (int i = 0; i < POSITIONS; i++) {
    int p = indexArray[i]; // Get current position from Origin for index
    if (direction == FORWARD) {
      if (p == 0) p = POSITIONS; // Reset position from Origin, if already at Origin
      p--; // Decrement by one as new position is one position closer to Origin
    }
    else if (direction == REVERSE) {
      p++; // Increament by one as new position is one position further from Origin
      if (p == POSITIONS) p = 0; // Reset to Origin, if now at Origin
    }
    indexArray[i] = p; // Update index with new position from Origin
  }
}

// Helper to print position from Origin for each index
void showIndex() {
  for (int i = 0; i < POSITIONS; i++) {
    Serial.print(indexArray[i]);
    Serial.print(",");
  }
  Serial.println();
}

void moveToSensor() {
  for (pos = 0; pos <= 30; pos += 1) { // goes from 0 degrees to 30 degrees
    // in steps of 1 degree
    myservo.write(pos);              // tell servo to go to position in variable 'pos'
    delay(15);                       // waits 15ms for the servo to reach the position
  }
}

void moveToEjector() {
  for (pos = 30; pos <= 60; pos += 1) { // goes from 30 degrees to 60 degrees
    // in steps of 1 degree
    myservo.write(pos);              // tell servo to go to position in variable 'pos'
    delay(15);                       // waits 15ms for the servo to reach the position
  }
}

void moveToLoader() {
  delay(dwell_time);
  for (pos = 60; pos >= 0; pos -= 1) { // goes from 60 degrees to 0 degrees
    myservo.write(pos);              // tell servo to go to position in variable 'pos'
    delay(15);                       // waits 15ms for the servo to reach the position
  }
}









