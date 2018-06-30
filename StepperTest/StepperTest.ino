/*!
    File:     StepperTest.ino
    Author:   Peter Milne
    License:  GNU GENERAL PUBLIC LICENSE (see license.txt)

    Driver test for Colour Sorter stepper motor.

*/

//Define EasyDriver pins
#define stp 8    // stp pin connected to D2
#define dir 9    // dir pin connected to GND (forwards)
// #define MS1   MS1 & MS2 connected to 5V (1/8th step resoluion)
// #define MS2
#define EN  10    // EN connected to D6

// Define directions
#define REVERSE HIGH // Move in reverse4
#define FORWARD  LOW // Move forwards

// 5 equal positions for platter
const int POSITIONS = 5;
// Number of micro-steps to move one position at 1/8th step resolution (Default)
const int ONE_POSITION = 200 / POSITIONS * 8;

// Stores position from Origin of each index (Origin is always zero)
int indexArray[] = {0, 4, 3, 2, 1};

void setup() {
  pinMode(stp, OUTPUT);
  pinMode(dir, OUTPUT);
  pinMode(EN, OUTPUT);
  digitalWrite(EN, HIGH); // Disable motor
  delay(5000);
  Serial.begin(9600);
  Serial.println("Begin test");
}

void loop() {
  digitalWrite(EN, HIGH); // Disable motor    
  int position = random(0, 5); // Generate random position
  Serial.println(position);
  showIndex();
  setPosition(position);
  digitalWrite(EN, HIGH); // Disable motor  
  showIndex();
  delay(2000);
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











