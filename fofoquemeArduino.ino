#include <Servo.h> 
#define UPDATE_DELAY 10
#define NUM_MOTORS 10

// states
#define STATE_WAIT -1
#define STATE_WRITE 10
#define STATE_REPOS 11

// list of pins to use for the motors
//   in the order in which the arms will move
int servoPins[NUM_MOTORS] ={
  2,3,4,5,6,7,8,9,10,11};
// list of pins to use for the light
//   in the order in which the lights will turn on and off
int lightPins[NUM_MOTORS/2] ={
  A4,A3,A2,A1,A0};

// servos
Servo myServos[NUM_MOTORS];

// angle values for the read/write/center positions of each motor
int readPos[NUM_MOTORS] = {
  115,0,115,0,118,0,115,0,115,0};
int centerPos[NUM_MOTORS] = {
  90,90,90,90,90,90,90,90,90,90};
int writePos[NUM_MOTORS] = {
  70,180,75,180,80,180,70,180,70,180};

// to keep track of current position and desired position for each motor
int currPos[NUM_MOTORS] = {  
  0,0,0,0,0,0,0,0,0,0};
int targetPos[NUM_MOTORS] = {  
  0,0,0,0,0,0,0,0,0,0};

// for reading a byte from serial connection
int inByte = 0;
// to keep track of motor update times
unsigned long lastTime;
// current state
int currState;
// current motor being moved
int currWriteMotor;

void setup() { 
  // for bluetooth communication
  Serial.begin(57600);

  // attach servos
  for(int i=0; i<NUM_MOTORS; i++) {
    myServos[i].attach(servoPins[i]);
  }

  // send to start position
  // this also updates/resets a the position arrays (currPos, targetPos)
  for(int i=0; i<NUM_MOTORS; i++) {
    currPos[i] = centerPos[i];
    targetPos[i] = currPos[i];
    myServos[i].write(currPos[i]);
    delay(UPDATE_DELAY);
  }

  // initial condition
  lastTime = millis();
  currState = STATE_WAIT;
  currWriteMotor = 0;

  // setup light pins
  for(int i=0; i<NUM_MOTORS/2; i++){
    pinMode(lightPins[i],OUTPUT);
    digitalWrite(lightPins[i],LOW);
  }

  // for debugging
  pinMode(13,OUTPUT);
  digitalWrite(13,LOW);

} 


void loop() { 
  // idle state.
  if(currState == STATE_WAIT) {
    if (Serial.available() > 0) {
      // get incoming byte:
      inByte = Serial.read();
      // check for GO signal
      if(inByte == 'G') {
        Serial.flush();
        // start dance !!!

        // set targetPos for first 2 motors
        currWriteMotor = 0;
        digitalWrite(lightPins[currWriteMotor/2], HIGH);
        targetPos[currWriteMotor] = writePos[currWriteMotor];
        targetPos[currWriteMotor+1] = writePos[currWriteMotor+1];

        targetPos[currWriteMotor+2] = readPos[currWriteMotor+2];
        targetPos[currWriteMotor+3] = readPos[currWriteMotor+3];

        // new state
        currState = STATE_WRITE;
        digitalWrite(13,HIGH);
      }
    }
  }
  /////
  // new states !!
  else if(currState == STATE_WRITE) {
    // currWriteMotor points to motor that is writing
    // if done moving ...
    if((currPos[currWriteMotor] == targetPos[currWriteMotor])&&(currPos[currWriteMotor+1] == targetPos[currWriteMotor+1])){
      digitalWrite(lightPins[currWriteMotor/2], LOW);
      targetPos[currWriteMotor] = centerPos[currWriteMotor];
      targetPos[currWriteMotor+1] = centerPos[currWriteMotor+1];

      // not all writes have a read
      if((currWriteMotor+3) < NUM_MOTORS) {
        digitalWrite(lightPins[(currWriteMotor+2)/2], HIGH);
        targetPos[currWriteMotor+2] = centerPos[currWriteMotor+2];
        targetPos[currWriteMotor+3] = centerPos[currWriteMotor+3];
      }
      currState = STATE_REPOS;
    }
  }
  else if(currState == STATE_REPOS) {
    // currWriteMotor still points to motor that wrote last
    if((currPos[currWriteMotor] == targetPos[currWriteMotor])&&(currPos[currWriteMotor+1] == targetPos[currWriteMotor+1])){
      // update the current motor
      currWriteMotor += 2;
      // now currWriteMotor points to motor that is gonna be writing next

      // check if it's a valid motor, or if we are done
      if((currWriteMotor+1) < NUM_MOTORS) {
        targetPos[currWriteMotor] = writePos[currWriteMotor];
        targetPos[currWriteMotor+1] = writePos[currWriteMotor+1];
        // not all writes have a read
        if((currWriteMotor+3) < NUM_MOTORS) {
          targetPos[currWriteMotor+2] = readPos[currWriteMotor+2];
          targetPos[currWriteMotor+3] = readPos[currWriteMotor+3];        
        }
        currState = STATE_WRITE;
      }
      else{
        // have reset last arm, send message to phone
        Serial.write('S');
        currState = STATE_WAIT;
        digitalWrite(13,LOW);
      }
    }
  }


  // if in a moving state, update currPos and move motors
  if((currState != STATE_WAIT)){
    // update currPos, move it towards target
    if((millis() - lastTime) > UPDATE_DELAY) {
      // for all motors, check curr against target
      for(int i=0; i<NUM_MOTORS; i++){
        if(currPos[i] > targetPos[i]){
          currPos[i] -= 1;
        }
        else if(currPos[i] < targetPos[i]){
          currPos[i] += 1;
        }
        // else if (other conditions to achieve a fade...)

        // write the position to the motors
        myServos[i].write(currPos[i]);
      }
      lastTime = millis();
    }
  }

}























