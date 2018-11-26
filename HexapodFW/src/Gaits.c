/*
Source file for Vorpal Hexapod gaits and leg movements
*/

#include <stdint.h>
#include <stdio.h>
#include "Gaits.h"
#include "PCA9685.h"

/*
Wrapper functions to be used for clarity in state machines and outside the source file
* SetWalkLegs
* SetRotateLegs

*/

//setWalkLegs
 void setWalkLegs(uint8_t legmask, int16_t hip_pos, int16_t knee_pos, uint8_t adj) {
  setLegs(legmask, hip_pos, knee_pos, adj, 0, 0);  // invert left-side servo angles
}

//setRotateLegs
void setRotateLegs(uint8_t legmask, int16_t hip_pos, int16_t knee_pos, uint8_t adj) {
  setLegs(legmask, hip_pos, knee_pos, adj, 1, 0); //don't invert left side servo angles
}

//version for just the knees
void setKneesOnly( uint8_t legmask, int16_t knee_pos ) {
  setLegs(legmask, NOMOVE, knee_pos, 0, 0, 0);
}

//return to default stand position
void stand( void ) {
  setLegs(ALL_LEGS, HIP_NEUTRAL, KNEE_STAND, 0, 0, 0);
}

//return to default sit position
void laydown( void ) {
  setLegs(ALL_LEGS, HIP_NEUTRAL, KNEE_UP, 0, 0, 0);
}

 
/*
setLegs function for all your hip and knee directing needs
not sure we're going to use the leanangle but it's here
*/
void setLegs(uint8_t legmask, int16_t hip_pos, int16_t knee_pos, uint8_t adj, uint8_t raw, int16_t leanangle) {
  for(uint8_t i = 0; i < NUM_LEGS; i++) {
    if(legmask & 0x01) {  // if the lowest bit is ON
      if(hip_pos != NOMOVE) {
        if(!raw) {
          setHip(i, hip_pos, adj);
        } else {
          setHipRaw(i, hip_pos);
        }
      }
      if(knee_pos != NOMOVE) {
        uint8_t pos = knee_pos;
        if(leanangle != 0) {
          switch(i) {
            case 0: case 6: case 5: case 11:
              if(leanangle < 0) pos -= leanangle;
              break;
            case 1: case 7: case 4: case 10:
              pos += (leanangle/2); //abs()
              break;
            case 2: case 8: case 3: case 9:
              if(leanangle > 0) pos += leanangle;
              break;
          }
        }       
        setKnee(i, pos);
      }
    }
    legmask = (legmask>>1);  // shift down one bit position
  }
}
 
/*
???? -LI
*/
void transactServos() {
  deferServoSet = 1;
}

/*
also ?? --LI
*/
void commitServos() {
  //checkForCrashingHips();
  deferServoSet = 0;
  for (uint8_t servo = 0; servo < 12; servo++) {
    PCA9685_setServo(servo, ServoPos[servo]);
  }
}

/*

*/
// this version of setHip adjusts not only for left and right,
// but also shifts the front legs a little back and the back legs
// forward to make a better balance for certain gaits like tripod or quadruped
//

void setHip(uint8_t leg, int16_t pos, uint8_t adj) {
  if (ISFRONTLEG(leg)) {
    pos -= adj;
  } else if (ISBACKLEG(leg)) {
    pos += adj;
  }
  // reverse the left side for consistent forward motion
  if (leg >= LEFT_START) {
    pos = 180 - pos;
  }

  setHipRaw(leg, pos);
}

/*
// this version of setHip does no processing at all (for example
// to distinguish left from right sides)
*/
void setHipRaw(uint8_t leg, int16_t pos) {
  pos += servoOffset[leg];  //needs tuning! --LI
  ServoPos[leg] = pos;
  PCA9685_setServo(leg, (float)pos);
}

/*

*/
void setKnee(uint8_t leg, int16_t pos) {
  // find the knee associated with leg if this is not already a knee
  if (leg < KNEE_OFFSET) {
    leg += KNEE_OFFSET;
  }
  pos += servoOffset[leg];
  PCA9685_setServo(leg, (float)pos);
}

/*

*/

/*

*/

#define NUM_TRIPOD_PHASES 6
#define FBSHIFT    15   // shift front legs back, back legs forward, this much

void gait_tripod(uint8_t reverse, uint8_t hipforward, uint8_t hipbackward, 
          uint8_t kneeup, uint8_t kneedown, long timeperiod, uint8_t leanangle) {

  // the gait consists of 6 phases. This code determines what phase
  // we are currently in by using the millis clock modulo the 
  // desired time period that all six  phases should consume.
  // Right now each phase is an equal amount of time but this may not be optimal

  if (reverse) {
    uint8_t tmp = hipforward;
    hipforward = hipbackward;
    hipbackward = tmp;
  }
  
  //long t = millis()%timeperiod;
  //long phase = (NUM_TRIPOD_PHASES*t)/timeperiod;
static uint8_t phase = 0;
  //Serial.print("PHASE: ");
  //Serial.println(phase);

  transactServos(); // defer leg motions until after checking for crashes
  switch (phase) {
    case 0:
      setLegs(TRIPOD1_LEGS, NOMOVE, KNEE_UP, 0, 0, leanangle);
    case 1:
      // in this phase, the center-left and noncenter-right legs move forward
      // at the hips, while the rest of the legs move backward at the hip
      setWalkLegs(TRIPOD1_LEGS, hipforward, NOMOVE, FBSHIFT);
      setWalkLegs(TRIPOD2_LEGS, hipbackward, NOMOVE, FBSHIFT);
      break;

    case 2: 
      // now put the first set of legs back down on the ground
      setLegs(TRIPOD1_LEGS, NOMOVE, KNEE_DOWN, 0, 0, leanangle);
      break;

    case 3:
      // lift up the other set of legs at the knee
      setLegs(TRIPOD2_LEGS, NOMOVE, KNEE_UP, 0, 0, leanangle);
      break;
      
    case 4:
      // similar to phase 1, move raised legs forward and lowered legs backward
      setWalkLegs(TRIPOD1_LEGS, hipbackward, NOMOVE, FBSHIFT);
      setWalkLegs(TRIPOD2_LEGS, hipforward, NOMOVE, FBSHIFT);
      break;

    case 5:
      // put the second set of legs down, and the cycle repeats
      setLegs(TRIPOD2_LEGS, NOMOVE, kneedown, 0, 0, leanangle);
      break;  
  }
  commitServos(); // implement all leg motions
}


/*
"Turn" gait state machine
*/
#define NUM_TURN_PHASES 6
#define FBSHIFT_TURN    40

void gait_turn(uint8_t ccw, uint8_t hipforward, uint8_t hipbackward, uint8_t kneeup, uint8_t kneedown, long timeperiod, uint8_t leanangle) { 
  //reverse direction of hip movement if CCW variable is set
  if (ccw) {
    uint8_t tmp = hipforward;
    hipforward = hipbackward;
    hipbackward = tmp;
  }
  
  static uint8_t phase = 0;

  switch (phase) {
    case 0:
      // in this phase, center-left and noncenter-right legs raise up at
      // the knee
      setLegs(TRIPOD1_LEGS, NOMOVE, KNEE_UP, 0, 0, leanangle);
      break;

    case 1:
      // in this phase, the center-left and noncenter-right legs move clockwise
      // at the hips, while the rest of the legs move CCW at the hip
      setRotateLegs(TRIPOD1_LEGS, hipforward, NOMOVE, FBSHIFT_TURN);
      setRotateLegs(TRIPOD2_LEGS, hipbackward, NOMOVE, FBSHIFT_TURN);
      break;

    case 2: 
      // now put the first set of legs back down on the ground
      setLegs(TRIPOD1_LEGS, NOMOVE, KNEE_DOWN, 0, 0, leanangle);
      break;

    case 3:
      // lift up the other set of legs at the knee
      setLegs(TRIPOD2_LEGS, NOMOVE, KNEE_UP, 0, 0, leanangle);
      break;
      
    case 4:
      // similar to phase 1, move raised legs CW and lowered legs CCW
      setRotateLegs(TRIPOD1_LEGS, hipbackward, NOMOVE, FBSHIFT_TURN);
      setRotateLegs(TRIPOD2_LEGS, hipforward, NOMOVE, FBSHIFT_TURN);
      break;

    case 5:
      // put the second set of legs down, and the cycle repeats
      setLegs(TRIPOD2_LEGS, NOMOVE, KNEE_DOWN, 0, 0, leanangle);
      break;  
  }
  
}

void GaitHandler( gaitCommand_t lastCmd ){
  static uint8_t positionPhase = 0;  //replace these uints with enumerated states
  static uint8_t gaitPhase = 0;
  
  switch(positionPhase){
  case 0: //sit
    if (lastCmd == BOT_STAND) {
      stand();
      positionPhase = 1;
    }
    //else do nothing until we get a command to stand up
    break;
  case 1: //stand
    if (lastCmd == BOT_SIT) {
      laydown();
      positionPhase = 0;
    } 
    else if (lastCmd == BOT_STAND){
      ;
      }//do nothing
    else {
      setKneesOnly(TRIPOD1_LEGS, KNEE_UP);
      positionPhase = 2;
    }
    break;
  case 2: //legs up
    //switch (gaitPhase){}
      
    break;
  default:
    break;
  }
}    
    


