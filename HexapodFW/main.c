#include <stdint.h>
#include <stdbool.h>
#include <stdio.h>
#include "tm4c123gh6pm.h"
#include "src/I2C.h"
#include "src/Timer.h"
#include "src/PCA9685.h"


//==============================================================================
// Bit patterns for the different combinations of legs
// bottom six bits. LSB is leg number 0
#define ALL_LEGS      0b111111
#define LEFT_LEGS     0b111000
#define RIGHT_LEGS    0b000111
#define TRIPOD1_LEGS  0b010101
#define TRIPOD2_LEGS  0b101010
#define FRONT_LEGS    0b100001
#define MIDDLE_LEGS   0b010010
#define BACK_LEGS     0b001100
#define NO_LEGS       0b0

// Default positions for knee and hip. Note that hip position is automatically 
// reversed for the left side by the setHip function. These are in degrees.
#define KNEE_UP_MAX 180
#define KNEE_UP     150
#define KNEE_RELAX  120  
#define KNEE_NEUTRAL 90 
#define KNEE_CROUCH 110
#define KNEE_HALF_CROUCH 80
#define KNEE_STAND 30
#define KNEE_DOWN  30   
#define KNEE_TIPTOES 5
#define KNEE_FOLD 170

#define KNEE_SCAMPER (KNEE_NEUTRAL-20)

#define KNEE_TRIPOD_UP (KNEE_NEUTRAL-40)
#define KNEE_TRIPOD_ADJ 30

#define HIPSWING 25      // how far to swing hips on gaits like tripod or quadruped
#define HIPSMALLSWING 10  // when in fine adjust mode how far to move hips
#define HIPSWING_RIPPLE 20
#define HIP_FORWARD_MAX 175
#define HIP_FORWARD (HIP_NEUTRAL+HIPSWING)
#define HIP_FORWARD_SMALL (HIP_NEUTRAL+HIPSMALLSWING)
#define HIP_NEUTRAL 90
#define HIP_BACKWARD (HIP_NEUTRAL-HIPSWING)
#define HIP_BACKWARD_SMALL (HIP_NEUTRAL-HIPSMALLSWING)
#define HIP_BACKWARD_MAX 0
#define HIP_FORWARD_RIPPLE (HIP_NEUTRAL+HIPSWING_RIPPLE)
#define HIP_BACKWARD_RIPPLE (HIP_NEUTRAL-HIPSWING_RIPPLE)
#define HIP_FOLD 150

#define NOMOVE (-1)   // fake value meaning this aspect of the leg (knee or hip) shouldn't move

#define LEFT_START 3  // first leg that is on the left side
#define RIGHT_START 0 // first leg that is on the right side
#define KNEE_OFFSET 6 // add this to a leg number to get the knee servo number

// these modes are used to interpret incoming bluetooth commands
#define TRIPOD_CYCLE_TIME 750
#define RIPPLE_CYCLE_TIME 1800
#define FIGHT_CYCLE_TIME 660
//==============================================================================

// Other initializations
int lastCmd = 115;
uint8_t deferServoSet = 0;
//==============================================================================

int main(void) {
    
  I2C_InitPort1();
  
  PCA9685_Init();
  PCA9685_UpdatePWMFrequency(60u);
  PCA9685_Restart();
#if 0
      PCA9685_SetDutyCycle(1.0); 
      PCA9685_SetDutyCycle(7.1); 
      PCA9685_SetDutyCycle(7.2); 
      PCA9685_SetDutyCycle(7.3); 
      PCA9685_SetDutyCycle(7.4); 
      PCA9685_SetDutyCycle(7.5); 
      PCA9685_SetDutyCycle(7.6); 
      PCA9685_SetDutyCycle(7.7); 
      PCA9685_SetDutyCycle(7.8); 
      PCA9685_SetDutyCycle(2.0); 
      PCA9685_SetDutyCycle(3.0); 
      PCA9685_SetDutyCycle(4.0); 
      PCA9685_SetDutyCycle(5.0); 
      PCA9685_SetDutyCycle(6.0); 
      PCA9685_SetDutyCycle(7.0); 
      PCA9685_SetDutyCycle(8.0); 
      PCA9685_SetDutyCycle(9.0); 
      PCA9685_SetDutyCycle(10.0); 
      PCA9685_SetDutyCycle(20.0); 
      PCA9685_SetDutyCycle(30.0); 
      PCA9685_SetDutyCycle(40.0); 
      PCA9685_SetDutyCycle(50.0); 
      PCA9685_SetDutyCycle(60.0); 
      PCA9685_SetDutyCycle(70.0); 
      PCA9685_SetDutyCycle(80.0); 
      PCA9685_SetDutyCycle(90.0); 
      PCA9685_SetDutyCycle(99.0); 
#endif
      PCA9685_SetDegreeCycle(0.0);  //Actual 956us, Expected 1ms
      PCA9685_SetDegreeCycle(15.0); //Actual 1.037ms
      PCA9685_SetDegreeCycle(30.0); //Actual 1.115ms
      PCA9685_SetDegreeCycle(45.0); //Actual 1.197ms
      PCA9685_SetDegreeCycle(60.0); //Actual 1.274ms
      PCA9685_SetDegreeCycle(75.0); //Actual 1.356ms
      PCA9685_SetDegreeCycle(90.0); //Actual 1.433ms, Expected 1.5ms
      PCA9685_SetDegreeCycle(105.0);//Actual 1.515ms
      PCA9685_SetDegreeCycle(120.0);//Actual 1.593ms
      PCA9685_SetDegreeCycle(135.0);//Actual 1.670ms
      PCA9685_SetDegreeCycle(150.0);//Actual 1.752ms
      PCA9685_SetDegreeCycle(165.0);//Actual 1.830ms
      PCA9685_SetDegreeCycle(180.0);//Actual 1.911ms, Expected 2ms
      
      

  while(1){

    if(lastCmd == 102){
    while(lastCmd == 102){
    walkForward();
    updateBluetoothInfo();
    }
  } else if(lastCmd == 98){
    while(lastCmd == 98){
    walkBackward();
    updateBluetoothInfo();
    }
  } else if(lastCmd == 114){
    while(lastCmd == 114){
    //laydown();
    turn(1, HIP_FORWARD, HIP_BACKWARD, KNEE_NEUTRAL, KNEE_DOWN, TRIPOD_CYCLE_TIME); // turn CCW
    //turn(0, HIP_FORWARD, HIP_BACKWARD, KNEE_NEUTRAL, KNEE_DOWN, TRIPOD_CYCLE_TIME); //turn CW
    updateBluetoothInfo();
    }
  } else {
    stand();
    updateBluetoothInfo();
  }
  
  }
return 0;
}
//55656
/*

*/
void walkForward(){
    gait_tripod(0, HIP_FORWARD, HIP_BACKWARD, KNEE_NEUTRAL, KNEE_DOWN, TRIPOD_CYCLE_TIME);
}

/*

*/
void gait_tripod(int reverse, int hipforward, int hipbackward, 
          int kneeup, int kneedown, long timeperiod, int leanangle) {

  // the gait consists of 6 phases. This code determines what phase
  // we are currently in by using the millis clock modulo the 
  // desired time period that all six  phases should consume.
  // Right now each phase is an equal amount of time but this may not be optimal

  if (reverse) {
    int tmp = hipforward;
    hipforward = hipbackward;
    hipbackward = tmp;
  }

#define NUM_TRIPOD_PHASES 6
#define FBSHIFT    15   // shift front legs back, back legs forward, this much
  
  long t = millis()%timeperiod;
  long phase = (NUM_TRIPOD_PHASES*t)/timeperiod;

  //Serial.print("PHASE: ");
  //Serial.println(phase);

  transactServos(); // defer leg motions until after checking for crashes
  switch (phase) {
    case 0:
      // in this phase, center-left and noncenter-right legs raise up at
      // the knee
      setLeg(TRIPOD1_LEGS, NOMOVE, kneeup, 0, 0, leanangle);
      break;

    case 1:
      // in this phase, the center-left and noncenter-right legs move forward
      // at the hips, while the rest of the legs move backward at the hip
      setLeg(TRIPOD1_LEGS, hipforward, NOMOVE, FBSHIFT);
      setLeg(TRIPOD2_LEGS, hipbackward, NOMOVE, FBSHIFT);
      break;

    case 2: 
      // now put the first set of legs back down on the ground
      setLeg(TRIPOD1_LEGS, NOMOVE, kneedown, 0, 0, leanangle);
      break;

    case 3:
      // lift up the other set of legs at the knee
      setLeg(TRIPOD2_LEGS, NOMOVE, kneeup, 0, 0, leanangle);
      break;
      
    case 4:
      // similar to phase 1, move raised legs forward and lowered legs backward
      setLeg(TRIPOD1_LEGS, hipbackward, NOMOVE, FBSHIFT);
      setLeg(TRIPOD2_LEGS, hipforward, NOMOVE, FBSHIFT);
      break;

    case 5:
      // put the second set of legs down, and the cycle repeats
      setLeg(TRIPOD2_LEGS, NOMOVE, kneedown, 0, 0, leanangle);
      break;  
  }
  commitServos(); // implement all leg motions
}

/*

*/
void transactServos() {
  deferServoSet = 1;
}

/*

*/
void commitServos() {
  //checkForCrashingHips();
  deferServoSet = 0;
  for (int servo = 0; servo < 12; servo++) {
    setServo(servo, ServoPos[servo]);
  }
}

/*

*/
void setLeg(int legmask, int hip_pos, int knee_pos, int adj, int raw, int leanangle) {
  for (int i = 0; i < NUM_LEGS; i++) {
    if (legmask & 0b1) {  // if the lowest bit is ON
      if (hip_pos != NOMOVE) {
        if (!raw) {
          setHip(i, hip_pos, adj);
        } else {
          setHipRaw(i, hip_pos);
        }
      }
      if (knee_pos != NOMOVE) {
        int pos = knee_pos;
        if (leanangle != 0) {
          switch (i) {
            case 0: case 6: case 5: case 11:
              if (leanangle < 0) pos -= leanangle;
              break;
            case 1: case 7: case 4: case 10:
              pos += abs(leanangle/2);
              break;
            case 2: case 8: case 3: case 9:
              if (leanangle > 0) pos += leanangle;
              break;
          }
          //Serial.print("Lean:"); Serial.print(leanangle); Serial.print("pos="); Serial.println(pos);
        }
        
        setKnee(i, pos);
      }
    }
    legmask = (legmask>>1);  // shift down one bit position
  }
}