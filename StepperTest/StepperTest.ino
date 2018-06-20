/*!
    @file     StepperTest.ino
    @author   Peter Milne
    @license  GNU GENERAL PUBLIC LICENSE (see license.txt)

    Driver test for Colour Sorter stepper motor.

*/

//Define EasyDriver pins
#define stp 2    // stp pin connected to D2
#define dir 3    // dir pin connected to GND (forwards)
#define MS1 4    // MS1 & MS2 connected to 5V (1/8th step resoluion)
#define MS2 5    
#define EN  6    // EN connected to D6

// Define directions
#define REVERSE HIGH // Move in reverse4
#define FORWARD  LOW // Move forwards

// 5 equal positions for platter
const int NUM_OF_POSITIONS = 5; 
// Number of micro-steps to move one position at 1/8th step resolution (Default)
const int ONE_POSITION = 200 / NUM_OF_POSITIONS * 8;

// Stores position from Origin of each index (Origin always has value zero)
int index[] = {0, // Start with index 0 at Origin
               4, // index 1 is 4 positions forwards from Origin
               3, // index 2 is 3 positions forwards from Origin
               2, // index 3 is 2 positions forwards from Origin
               1}; // index 4 is 1 position forwards from Origin

String inString = "";    // string to hold input

void setup() {
  pinMode(stp, OUTPUT);
  pinMode(EN, OUTPUT);
  digitalWrite(EN, HIGH); //Pull enable pin high to reduce current
  Serial.begin(9600);
  Serial.println("Begin test"); 
}

void loop() {
  int position = 0;
  // Read serial input:
  while (Serial.available() > 0) {
    int inChar = Serial.read();
    if (isDigit(inChar)) {
      // convert the incoming byte to a char and add it to the string:
      inString += (char)inChar;
    }
    // if you get a newline, print the string, then the string's value:
    if (inChar == '\n') {
      position = inString.toInt();
      Serial.println(position);
      showIndex();
      digitalWrite(EN, LOW); //Pull enable pin low to allow motor control
      setPosition(position);
      digitalWrite(EN, HIGH); //Pull enable pin high to reduce current
      showIndex();
      // clear the string for new input:
      inString = "";
    }
  }
}

// Recalculate position from Origin of each Index after moving one position
void updateIndex(int direction){
  for(int i=0; i<NUM_OF_POSITIONS; i++){
    int p = index[i]; // Get current position from Origin for index 
    if (direction == FORWARD){
      if(p == 0) p = NUM_OF_POSITIONS; // Reset position from Origin, if already at Origin
      p--; // Decrement by one as new position is one position closer to Origin
    }
    else
      if (direction == REVERSE) {
      p++; // Increament by one as new position is one position further from Origin
      if(p == NUM_OF_POSITIONS) p = 0; // Reset to Origin, if now at Origin
    }
    index[i] = p; // Update index with new position from Origin
  }    
}

// Print position from Origin for each index
void showIndex(){
  for(int i=0; i<NUM_OF_POSITIONS; i++){
    Serial.print(index[i]);
    Serial.print(",");
  }
  Serial.println();  
}

// Step motor one position, each position is made up of multiple micro-steps
void stepOnePosition(int direction){
  digitalWrite(dir, direction); //Pull dir pin LOW for forwards / HIGH for reverse
  delay(1);
  for(int i = 0; i < ONE_POSITION; i++)  // Move the number of micro-steps for each Position
  {
    digitalWrite(stp,HIGH); //Trigger one step forward
    delay(1);
    digitalWrite(stp,LOW); //Pull step pin low so it can be triggered again
    delay(1);
  }
  updateIndex(direction); // Update value of index after each change in position
}

// Move any valid index to Origin
void setPosition(int pos){
  int direction;
  if ((pos >= 0) && (pos < NUM_OF_POSITIONS)){
    int p = index[pos];
    if ( p > 2 ) {
      p = NUM_OF_POSITIONS - p;
      direction = REVERSE;
    }
    else
      direction = FORWARD;    
    for (int i=0; i<p; i++){
      stepOnePosition(direction); // Move forwards one position at a time
    }
  }
  else return; // Do nothing if index is out of range
} 













