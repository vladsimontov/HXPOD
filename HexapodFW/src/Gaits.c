/*
Source file for Vorpal Hexapod gaits and leg movements
*/
#include "Gaits.h"
#include "PCA9685.h"
#include "Timer.h"
#include "GPIO.h"

uint8_t deferServoSet = 0;
uint32_t timeToMove = 0;
uint32_t timeToMoveDemo = 0;
volatile uint32_t ms_sinceStart = 0;

#define POSITION_FEEDBACK_ENABLED 0



void updateMillis( void ){
  ms_sinceStart++;
  return;
}
#if USE_GOBLE_AS_MOVEMENT_CLOCK
uint32_t millis( void ){
   return ms_sinceStart;
}
#endif

int16_t ServoPos[2*NUM_LEGS]; //store last servo position instruction
uint8_t servoOffset[2*NUM_LEGS] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};  /*knee offsets (6-11)*/

/*
Wrapper functions to be used for clarity in state machines and outside the source file
* SetWalkLegs
* SetRotateLegs

*/
//version for just the knees
void setKneesOnly( uint8_t legmask, int16_t knee_pos ) {
  setLegs(legmask, NOMOVE, knee_pos, 0, 0, 0);
}

//return to default stand position
void stand( void ) {
  setLegs(ALL_LEGS, HIP_NEUTRAL, NOMOVE, 0, 0, 0);
  delay(200);
  setLegs(ALL_LEGS, HIP_NEUTRAL, KNEE_STAND, 0, 0, 0);
}

//return to default sit position
void laydown( void ) {
  setLegs(ALL_LEGS, HIP_NEUTRAL, KNEE_UP_MAX, 0, 0, 0);
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


void runGaitFSM( gaitCommand_t lastCmd ){
  static phase_t position = SITTING;
  
  switch(position){
  case SITTING: //sit
    if (lastCmd == BOT_STAND) {
      stand(); 
      position = STANDING;
    }
    break;
  case STANDING: //stand
    if (lastCmd == BOT_SIT) {
      laydown();
      position = SITTING;
    } 
    else if (lastCmd != BOT_STAND){
      setKneesOnly(TRIPOD1_LEGS, KNEE_UP);
      position = WALKING;
    }
    break;
  case WALKING: //handles walking, turning, and veering
    if (timeToMove < millis() || POSITION_FEEDBACK_ENABLED){
      if (GaitHandler(lastCmd) == DONE_WALKING){
        stand();
        position = STANDING;
      }
      else if (lastCmd == BOT_STOP){
        //bot is frozen
        position = FROZEN;
      }
    }
    break;
  case FROZEN:
    if (lastCmd == BOT_STAND) {
      stand();
      position = STANDING;
    } 
    break;
  default:
    break;
  }
}    

#define WALK_MODE 0x00
#define TURN_MODE 0x01
#define FBSHIFT    15   // shift front legs back, back legs forward, this much
#define FBSHIFT_TURN    40

#if USE_GOBLE_AS_MOVEMENT_CLOCK
 //these "times" are multipliers for the incoming packet rate (~5 per second)
 #define TRIPOD_LIFT_TIME 1
 #define TRIPOD_SWIVEL_TIME 1
 #define TRIPOD_SET_TIME 1
#else
 //else use a (yet to be created) millis function which returns milliseconds
 #define TRIPOD_LIFT_TIME 50
 #define TRIPOD_SWIVEL_TIME 50
 #define TRIPOD_SET_TIME 50
#endif

/*
Each gait consists of 6 phases (walking, veering, turning)
The gait parameters may be changed at any time (based on the incoming command)
which allows for smooth transitions between different motions

*/

uint8_t hipdir1 = HIP_FORWARD;
uint8_t hipdir2 = HIP_BACKWARD;
uint8_t servoShift = FBSHIFT;
uint8_t moveType = WALK_MODE;
uint8_t leanangle = 0;
  
phase_t GaitHandler( gaitCommand_t lastCmd ){

  static phase_t gaitPhase = TRIPOD1_LIFT;
  
  if (lastCmd == BOT_STOP){
    //freeze and wait for a new command
    return FROZEN;
  }
  
  switch (gaitPhase) {
    case TRIPOD1_LIFT:     
      // in this phase, center-left and noncenter-right legs raise up at
      // the knee
      setLegs(TRIPOD1_LEGS, NOMOVE, KNEE_NEUTRAL, 0, 0, leanangle);
      timeToMove = millis() + TRIPOD_LIFT_TIME;
      gaitPhase = TRIPOD1_SWIVEL;
      break;
      
    case TRIPOD1_SWIVEL:
      // in this phase, the center-left and noncenter-right legs move forward
      // at the hips, while the rest of the legs move backward at the hip      
      setGaitVariables(lastCmd, gaitPhase); 
      setLegs(TRIPOD1_LEGS, hipdir1, NOMOVE, servoShift, moveType, leanangle);  
      setLegs(TRIPOD2_LEGS, hipdir2, NOMOVE, servoShift, moveType, leanangle);
      timeToMove = millis() + TRIPOD_SWIVEL_TIME;
      if (lastCmd == BOT_STAND || lastCmd == BOT_SIT){
        gaitPhase = WALK_STOPPING;
      }else {
        gaitPhase = TRIPOD1_SET;
      }
      break;
      
    case TRIPOD1_SET: 
      // now put the first set of legs back down on the ground
      setLegs(TRIPOD1_LEGS, NOMOVE, KNEE_DOWN, 0, 0, leanangle);
      timeToMove = millis() + TRIPOD_SET_TIME;
      gaitPhase = TRIPOD2_LIFT;
      break;
      
    case TRIPOD2_LIFT:
      // lift up the other set of legs at the knee
      setLegs(TRIPOD2_LEGS, NOMOVE, KNEE_NEUTRAL, 0, 0, leanangle);
      timeToMove = millis() + TRIPOD_LIFT_TIME;
      gaitPhase = TRIPOD2_SWIVEL;
      break;
      
    case TRIPOD2_SWIVEL:
      // similar to phase 1, move raised legs forward and lowered legs backward     
      setGaitVariables(lastCmd, gaitPhase); 
      setLegs(TRIPOD1_LEGS, hipdir2, NOMOVE, servoShift, moveType, leanangle);
      setLegs(TRIPOD2_LEGS, hipdir1, NOMOVE, servoShift, moveType, leanangle);
      timeToMove = millis() + TRIPOD_SWIVEL_TIME;
      if (lastCmd == BOT_STAND || lastCmd == BOT_SIT){
        gaitPhase = WALK_STOPPING;
      }else {
        gaitPhase = TRIPOD2_SET;
      }
      break;

    case TRIPOD2_SET:
      // put the second set of legs down, and the cycle repeats
      setLegs(TRIPOD2_LEGS, NOMOVE, KNEE_DOWN, 0, 0, leanangle);
      timeToMove = millis() + TRIPOD_SET_TIME;
      gaitPhase = TRIPOD1_LIFT;
      break;
      
    case WALK_STOPPING:
      setLegs(TRIPOD1_LEGS, NOMOVE, KNEE_DOWN, 0, 0, leanangle);
      setLegs(TRIPOD2_LEGS, NOMOVE, KNEE_DOWN, 0, 0, leanangle);
      gaitPhase = TRIPOD1_LIFT;
      return DONE_WALKING;
      break;
      
  }
  return gaitPhase;
}


void setGaitVariables(gaitCommand_t lastCmd, phase_t gaitPhase){
  if (gaitPhase == TRIPOD1_SWIVEL){
    switch(lastCmd){    
       case BOT_WALK_NW:
       case BOT_WALK_NE:
         lastCmd = BOT_WALK_FWD;
         break;
       case BOT_WALK_SW:
       case BOT_WALK_SE:        
         lastCmd = BOT_WALK_BACK;
         break;
    }
  }
  else if (gaitPhase == TRIPOD1_SWIVEL){
    switch(lastCmd){    
       case BOT_WALK_NW:
       case BOT_WALK_SE:
         lastCmd = BOT_ROTATE_LEFT;
         break;
       case BOT_WALK_SW:
       case BOT_WALK_NE:        
         lastCmd = BOT_ROTATE_RIGHT;
         break;
    }
  }
     switch(lastCmd){
        case BOT_WALK_FWD:
           hipdir1 = HIP_FORWARD;
           hipdir2 = HIP_BACKWARD;
           servoShift = FBSHIFT;
           moveType = WALK_MODE;
           break;
        case BOT_WALK_BACK:
           hipdir1 = HIP_BACKWARD;
           hipdir2 = HIP_FORWARD;
           servoShift = FBSHIFT;
           moveType = WALK_MODE;
           break;
        case BOT_ROTATE_LEFT:
           hipdir1 = HIP_FORWARD;
           hipdir2 = HIP_BACKWARD;
           servoShift = FBSHIFT_TURN;
           moveType = TURN_MODE;
           break;
        case BOT_ROTATE_RIGHT:
           hipdir1 = HIP_BACKWARD;
           hipdir2 = HIP_FORWARD;
           servoShift = FBSHIFT_TURN;
           moveType = TURN_MODE;
           break;
        case BOT_STAND:
        case BOT_SIT:
           hipdir1 = HIP_NEUTRAL;
           hipdir2 = HIP_NEUTRAL;
           servoShift = 0;
           moveType = WALK_MODE;
           break;
       
     }        
  return;
}

void demo() {
  
  //Start by standing:
  stand();
  delay(500);//small delay before the next move 1/2 sec
    
  /*Call function to rotate each leg*/
  rotateLegs();
  /*
    uint8_t currentLeg = 0;
  //loop to cycle through each leg, 6 legs
  for(int i = 0; i < NUM_LEGS; i++) {
    
    switch(i) {
      case 0:
        currentLeg = LEG0;
        break;
      case 1:
        currentLeg = LEG1;
        break;
      case 2:
        currentLeg = LEG2;
        break;
      case 3:
        currentLeg = LEG3;
        break;
      case 4:
        currentLeg = LEG4;
        break;
      case 5:
        currentLeg = LEG5;
        break;
    default:
      currentLeg = -1;
        break;
    }
    
    setLegs(currentLeg, HIP_NEUTRAL, KNEE_UP_MAX, 0, 0, 0);
    delay(50);//small delay before the next move
    
    setLegs(currentLeg, HIP_FORWARD_MAX - 15, KNEE_UP - 10, 0, 0, 0);
    delay(50);//small delay before the next move
    
    setLegs(currentLeg, HIP_FORWARD_MAX, KNEE_CROUCH, 0, 0, 0);
    delay(50);//small delay before the next move
    
    setLegs(currentLeg, HIP_FORWARD_MAX - 15, KNEE_HALF_CROUCH, 0, 0, 0);
    delay(50);//small delay before the next move
    
    setLegs(currentLeg, HIP_NEUTRAL, KNEE_CROUCH - 10, 0, 0, 0);
    delay(50);//small delay before the next move
    
    setLegs(currentLeg, HIP_BACKWARD_MAX + 15, KNEE_HALF_CROUCH, 0, 0, 0);
    delay(50);///small delay before the next move
    
    setLegs(currentLeg, HIP_BACKWARD_MAX, KNEE_CROUCH, 0, 0, 0);
    delay(50);///small delay before the next move
    
    setLegs(currentLeg, HIP_BACKWARD_MAX + 15, KNEE_UP - 10, 0, 0, 0);
    delay(50);///small delay before the next move

    setLegs(currentLeg, HIP_NEUTRAL, KNEE_UP_MAX, 0, 0, 0);
    delay(50);///small delay before the next move
    
    setLegs(currentLeg, HIP_NEUTRAL, KNEE_STAND, 0, 0, 0);
    delay(50);///small delay before the next move
  }
*/
  
    /*Swim*/
    delay(500);//small delay before the next move 1 sec
    //Set legs to not interfere with movement of middle legs and middle legs to a neutral position
    setLegs(BACK_LEGS, HIP_BACKWARD_MAX, KNEE_RELAX, 0, 0, leanangle);
    setLegs(FRONT_LEGS, HIP_FORWARD_MAX, KNEE_RELAX, 0, 0, leanangle);
    setLegs(MIDDLE_LEGS, HIP_NEUTRAL, KNEE_UP, 0, 0, leanangle);
    
    delay(500);//small delay before the next move 1 sec
      
    //Swim forward
    for(int i = 0; i < 6; i++) {
      
      //move middle legs forward and knees up
      delay(100);//small delay before the next move 1/10th sec
      setLegs(MIDDLE_LEGS, HIP_FORWARD_MAX, KNEE_UP_MAX, 0, 0, leanangle);
      
      //dont move legs and have knees push off surface
      delay(100);//small delay before the next move 1/10th sec
      setLegs(MIDDLE_LEGS, NOMOVE, 50, 0, 0, leanangle);
      
      //move legs back to neutral position with knee pushing off surface
      delay(200);//small delay before the next move 1/5th sec
      setLegs(MIDDLE_LEGS, HIP_NEUTRAL, NOMOVE, 0, 0, leanangle);
      
      //move legs back to back position with knee pushing off surface
      delay(200);//small delay before the next move 1/5th sec
      setLegs(MIDDLE_LEGS, HIP_BACKWARD_MAX, NOMOVE, 0, 0, leanangle);
      
      delay(100);//small delay before the next move 1/10th sec
      setLegs(MIDDLE_LEGS, NOMOVE, KNEE_UP_MAX, 0, 0, leanangle);
    }
    
    //Swim backward
    for(int i = 0; i < 6; i++) {
      
      //move middle legs forward and knees up
      delay(100);//small delay before the next move 
      setLegs(MIDDLE_LEGS, HIP_BACKWARD_MAX, KNEE_UP_MAX, 0, 0, leanangle);
      
      //dont move legs and have knees push off surface
      delay(100);//small delay before the next move 
      setLegs(MIDDLE_LEGS, NOMOVE, 50, 0, 0, leanangle);
      
      //move legs back to neutral position with knee pushing off surface
      delay(200);//small delay before the next move 
      setLegs(MIDDLE_LEGS, HIP_NEUTRAL, NOMOVE, 0, 0, leanangle);
      
      //move legs back to back position with knee pushing off surface
      delay(200);//small delay before the next move 
      setLegs(MIDDLE_LEGS, HIP_FORWARD_MAX, NOMOVE, 0, 0, leanangle);
      
      delay(100);//small delay before the next move 
      setLegs(MIDDLE_LEGS, NOMOVE, KNEE_UP_MAX, 0, 0, leanangle);
    }
    
    /*Bow*/
    delay(500);//small delay before the next move .5 sec
    stand();
    delay(500);//small delay before the next move .5 sec
      setLegs(BACK_LEGS, HIP_NEUTRAL, KNEE_STAND, 0, 0, leanangle);
      setLegs(MIDDLE_LEGS, HIP_NEUTRAL, KNEE_HALF_CROUCH, 0, 0, leanangle);
      setLegs(FRONT_LEGS, HIP_NEUTRAL, KNEE_CROUCH, 0, 0, leanangle);
      
      delay(1500);//small delay before the next move 1.5 sec
      stand();
      delay(1000);//small delay before the next move 1 sec
}

void delay(int milliSec){
  timeToMoveDemo = millis() + milliSec;
  while(millis() < timeToMoveDemo){}
}

void rotateLegs(){
  uint8_t currentLeg = 0;
    for(int i = 0; i < NUM_LEGS; i++) {
    
    switch(i) {
      case 0:
        currentLeg = LEG0;
        break;
      case 1:
        currentLeg = LEG1;
        break;
      case 2:
        currentLeg = LEG2;
        break;
      case 3:
        currentLeg = LEG3;
        break;
      case 4:
        currentLeg = LEG4;
        break;
      case 5:
        currentLeg = LEG5;
        break;
    default:
      currentLeg = -1;
        break;
    }
    int KNEE_LOCATION = 175; 
    int HIP_LOCATION = 90;  
    while(HIP_LOCATION < 130){
      setLegs(currentLeg, HIP_LOCATION, KNEE_LOCATION, 0, 0, 0);
      delay(5);
      HIP_LOCATION++;
      KNEE_LOCATION--;
    }
    
    KNEE_LOCATION = 110;
    HIP_LOCATION = 130;
    while(HIP_LOCATION > 90){
      setLegs(currentLeg, HIP_LOCATION, KNEE_LOCATION, 0, 0, 0);
      delay(5);
      HIP_LOCATION--;
      KNEE_LOCATION--;
    }
    
    KNEE_LOCATION = 45;
    HIP_LOCATION = 90;
    while(HIP_LOCATION > 50){
      setLegs(currentLeg, HIP_LOCATION, KNEE_LOCATION, 0, 0, 0);
      delay(5);
      HIP_LOCATION--;
      KNEE_LOCATION++;
    }
    
    KNEE_LOCATION = 110;
    HIP_LOCATION = 50;
    while(HIP_LOCATION < 90){
      setLegs(currentLeg, HIP_LOCATION, KNEE_LOCATION, 0, 0, 0);
      delay(5);
      HIP_LOCATION++;
      KNEE_LOCATION++;
    }
    setLegs(currentLeg, HIP_LOCATION, KNEE_STAND, 0, 0, 0);
  }
}