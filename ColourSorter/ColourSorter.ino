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

#include <Servo.h>

#include "Adafruit_TCS34725.h"
#include <Servo.h>

// Initialise with default values (int time = 2.4ms, gain = 1x)
// Adafruit_TCS34725 tcs = Adafruit_TCS34725();

// Initialise with specific int time and gain values
Adafruit_TCS34725 tcs = Adafruit_TCS34725(TCS34725_INTEGRATIONTIME_700MS, TCS34725_GAIN_1X);

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

void setup() {
  pinMode(stp, OUTPUT);
  pinMode(dir, OUTPUT);
  pinMode(EN, OUTPUT);
  digitalWrite(EN, HIGH); // Disable motor
  Serial.begin(9600);
  if (tcs.begin()) {
    Serial.println("Found sensor");
  } else {
    Serial.println("No TCS34725 found ... check your connections");
    while (1);
  }

  myservo.attach(servo);  // attaches the servo on pin 12 to the servo object
  myservo.write(0); // initialise servo position
  
  delay(500);
  Serial.println("Begin test");
}

void loop() {
  digitalWrite(EN, HIGH); // Disable motor 
  uint16_t colour_temp = 0;
  int colour_index;   
  //int position = random(0, 5); // Generate random position
  showIndex();

  moveToSensor();
  delay(dwell_time);

  colour_temp = getColourTemperature();
  while ((colour_index = getIndex(colour_temp)) < 0){
    delay(5);
    colour_temp = getColourTemperature();
    colour_index = getIndex(colour_temp);
    Serial.print("Index :");
    Serial.print(colour_index);
    Serial.print(" ");
    Serial.print("Colour :");
    Serial.print(colour_temp);
    Serial.println();
  }
  setPosition(colour_index);
  digitalWrite(EN, HIGH); // Disable motor  
  showIndex();
  delay(dwell_time);  

  moveToEjector();
  delay(dwell_time);

  moveToLoader();  
  delay(dwell_time);
  
}

int getIndex(uint16_t colour){
  int index = 0;
  if(colour >= 0 && colour <= 3870)
    return index = 0; //Orange;
  if(colour >= 3892 && colour <= 4150)
    return index = 1; //Yellow;
  if(colour >= 4832 && colour <= 5027)
    return index = 2; //Red; 
  if(colour >= 5086 && colour <= 5339)
    return index = 3; //Green;
  if(colour >= 6019)
    return index = 4; //Purple;
  return -1;
}

uint16_t getColourTemperature() {
  uint16_t r, g, b, c, colour_temp, lux;
    tcs.getRawData(&r, &g, &b, &c);
    colour_temp = tcs.calculateColorTemperature(r, g, b);

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

void moveToSensor(){
  for (pos = 0; pos <= 30; pos += 1) { // goes from 0 degrees to 30 degrees
    // in steps of 1 degree
    myservo.write(pos);              // tell servo to go to position in variable 'pos'
    delay(15);                       // waits 15ms for the servo to reach the position
  }
}

void moveToEjector(){
  for (pos = 30; pos <= 60; pos += 1) { // goes from 30 degrees to 60 degrees
    // in steps of 1 degree
    myservo.write(pos);              // tell servo to go to position in variable 'pos'
    delay(15);                       // waits 15ms for the servo to reach the position
  }      
}

void moveToLoader(){
  delay(dwell_time);  
  for (pos = 60; pos >= 0; pos -= 1) { // goes from 60 degrees to 0 degrees
    myservo.write(pos);              // tell servo to go to position in variable 'pos'
    delay(15);                       // waits 15ms for the servo to reach the position
  }  
}









