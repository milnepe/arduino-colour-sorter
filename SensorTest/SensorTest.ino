//#include <Wire.h>
#include "Adafruit_TCS34725.h"
#include <Servo.h>

/* Calibration code for the Adafruit TCS34725 breakout library */

/* Connect SCL    to analog 5
   Connect SDA    to analog 4
   Connect VDD    to 3.3V DC
   Connect GROUND to common ground

   Modified by Pete Milne
*/

/* Initialise with default values (int time = 2.4ms, gain = 1x) */
// Adafruit_TCS34725 tcs = Adafruit_TCS34725();

/* Initialise with specific int time and gain values */
Adafruit_TCS34725 tcs = Adafruit_TCS34725(TCS34725_INTEGRATIONTIME_700MS, TCS34725_GAIN_1X);

uint16_t min_colour = 0;
uint16_t max_colour = 0;
int counter = 0;

const int servo_pin = 12;
const int dwell_time = 1000;

Servo myservo;  // create servo object to control a servo
// twelve servo objects can be created on most boards

int pos = 0;    // variable to store the servo position

void setup(void) {
  Serial.begin(9600);

  if (tcs.begin()) {
    Serial.println("Found sensor");
  } else {
    Serial.println("No TCS34725 found ... check your connections");
    while (1);
  }
  myservo.attach(servo_pin);  // attaches the servo on pin 12 to the servo object
  myservo.write(60); // initialise servo position
  delay(5000);
  // Now we're ready to get readings!
}

void loop(void) {
  uint16_t colour_temp;
  moveToSensor();
  delay(dwell_time);

  colour_temp = calibrate();

  moveToEjector();
  delay(dwell_time);

  moveToLoader();
  delay(dwell_time);

  Serial.print("Color Temp: "); Serial.print(colour_temp, DEC); Serial.print(" K - ");
  Serial.println();
  Serial.print("Min: "); Serial.print(min_colour); Serial.print(" "); Serial.print("Max :"); Serial.print(max_colour);
  Serial.println();
}

uint16_t calibrate() {
  uint16_t r, g, b, c, colour_temp, lux;
  for(int i = 0; i < 2; i++)  {
    tcs.getRawData(&r, &g, &b, &c);
    colour_temp += tcs.calculateColorTemperature(r, g, b);
  //lux = tcs.calculateLux(r, g, b);
  delay(2);
  }
  colour_temp = colour_temp / 2; // Average colour temperature
  
  counter ++;
  // disgard first results
  if (counter < 2) return colour_temp;
  
  if (colour_temp > max_colour) {
    if(min_colour == 0) min_colour = colour_temp;
      
    max_colour = colour_temp;

  } else {
    if (colour_temp < min_colour)
      min_colour = colour_temp;
  }
  return colour_temp;
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

