/* Sweep
 by BARRAGAN <http://barraganstudio.com>
 This example code is in the public domain.

 modified 8 Nov 2013
 by Scott Fitzgerald
 modified by Pete Milne
 http://www.arduino.cc/en/Tutorial/Sweep
*/

#include <Servo.h>

const int servo_pin = 12;
const int dwell_time = 1000;

Servo myservo;  // create servo object to control a servo
// twelve servo objects can be created on most boards

int pos = 0;    // variable to store the servo position

void setup() {
  myservo.attach(servo_pin);  // attaches the servo on pin 12 to the servo object
  myservo.write(60); // initialise servo position
  delay(5000);

}

void loop() {
  moveToSensor();
  delay(dwell_time);

  moveToEjector();
  delay(dwell_time);

  moveToLoader();  
  delay(dwell_time);
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

