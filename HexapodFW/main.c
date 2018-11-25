 #include <stdint.h>
 #include <stdbool.h>
 #include <stdio.h>
 #include "tm4c123gh6pm.h"
 #include "src/I2C.h"
 #include "src/Timer.h"
 #include "src/PCA9685.h"


 //==============================================================================
#define NUM_LEGS 6

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

// individual leg bitmasks
#define LEG0 0b1
#define LEG1 0b10
#define LEG2 0b100
#define LEG3 0b1000
#define LEG4 0b10000
#define LEG5 0b100000

#define LEG0BIT  0b1
#define LEG1BIT  0b10
#define LEG2BIT  0b100
#define LEG3BIT  0b1000
#define LEG4BIT  0b10000
#define LEG5BIT  0b100000

#define ISFRONTLEG(LEG) (LEG==0||LEG==5)
#define ISMIDLEG(LEG)   (LEG==1||LEG==4)
#define ISBACKLEG(LEG)  (LEG==2||LEG==3)
#define ISLEFTLEG(LEG)  (LEG==0||LEG==1||LEG==2)
#define ISRIGHTLEG(LEG) (LEG==3||LEG==4||LEG==5)

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

 //Prototype functions
 void setLeg1(int legmask, int hip_pos, int knee_pos, int adj);
 void setLeg2(int legmask, int hip_pos, int knee_pos, int adj, int raw);
 void setLeg(int legmask, int hip_pos, int knee_pos, int adj, int raw, int leanangle);
 void walkForward();
 void walkBackward();
 void stand();
 void laydown();
 void gait_tripod(int reverse, int hipforward, int hipbackward, int kneeup, int kneedown, long timeperiod, int leanangle);
 void transactServos();
 void commitServos();
 void setHip(int leg, int pos, int adj);
 void setHipRaw(int leg, int pos);
 void setKnee(int leg, int pos);
 void turn(int ccw, int hipforward, int hipbackward, int kneeup, int kneedown, long timeperiod, int leanangle);
 void millis();
 
 /*
  Needs to be created.
*/
 void millis(){
   
 }

  int main(void) {
     
   I2C_InitPort1();
   
   PCA9685_Init();
   PCA9685_UpdatePWMFrequency(60u);
   PCA9685_Restart();
   
   
  PCA9685_setServo(90, 5);
  PCA9685_setServo(90, 0);
  PCA9685_setServo(90, 11);
       
       

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
     turn(1, HIP_FORWARD, HIP_BACKWARD, KNEE_NEUTRAL, KNEE_DOWN, TRIPOD_CYCLE_TIME, 0); // turn CCW
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
 
 void setLeg1(int legmask, int hip_pos, int knee_pos, int adj) {
  setLeg(legmask, hip_pos, knee_pos, adj, 0, 0);  // use the non-raw version with leanangle=0
}

// version with leanangle = 0
void setLeg2(int legmask, int hip_pos, int knee_pos, int adj, int raw) {
  setLeg(legmask, hip_pos, knee_pos, adj, raw, 0);
}
 
/*

*/
void setLeg(int legmask, int hip_pos, int knee_pos, int adj, int raw, int leanangle) {
  for(int i = 0; i < NUM_LEGS; i++) {
    if(legmask & 0b1) {  // if the lowest bit is ON
      if(hip_pos != NOMOVE) {
        if(!raw) {
          setHip(i, hip_pos, adj);
        } else {
          setHipRaw(i, hip_pos);
        }
      }
      if(knee_pos != NOMOVE) {
        int pos = knee_pos;
        if(leanangle != 0) {
          switch(i) {
            case 0: case 6: case 5: case 11:
              if(leanangle < 0) pos -= leanangle;
              break;
            case 1: case 7: case 4: case 10:
              pos += abs(leanangle/2);
              break;
            case 2: case 8: case 3: case 9:
              if(leanangle > 0) pos += leanangle;
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
 
 /*

*/
void walkForward(){
    gait_tripod(0, HIP_FORWARD, HIP_BACKWARD, KNEE_NEUTRAL, KNEE_DOWN, TRIPOD_CYCLE_TIME, 0);
}

/*

*/
void walkBackward(){
    gait_tripod(1, HIP_FORWARD, HIP_BACKWARD, KNEE_NEUTRAL, KNEE_DOWN, TRIPOD_CYCLE_TIME, 0);
}

/*

*/
void stand() {
  setLeg1(ALL_LEGS, HIP_NEUTRAL, KNEE_STAND, 0);
}

/*

*/
void laydown() {
  setLeg1(ALL_LEGS, HIP_NEUTRAL, KNEE_UP, 0);
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
      setLeg1(TRIPOD1_LEGS, hipforward, NOMOVE, FBSHIFT);
      setLeg1(TRIPOD2_LEGS, hipbackward, NOMOVE, FBSHIFT);
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
      setLeg1(TRIPOD1_LEGS, hipbackward, NOMOVE, FBSHIFT);
      setLeg1(TRIPOD2_LEGS, hipforward, NOMOVE, FBSHIFT);
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
    PCA9685_setServo(servo, ServoPos[servo]);
  }
}

/*

*/
// this version of setHip adjusts not only for left and right,
// but also shifts the front legs a little back and the back legs
// forward to make a better balance for certain gaits like tripod or quadruped

void setHip(int leg, int pos, int adj) {
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

*/
// this version of setHip does no processing at all (for example
// to distinguish left from right sides)
void setHipRaw(int leg, int pos) {
  PCA9685_setServo(leg, pos);
}

/*

*/
void setKnee(int leg, int pos) {
  // find the knee associated with leg if this is not already a knee
  if (leg < KNEE_OFFSET) {
    leg += KNEE_OFFSET;
  }
  PCA9685_setServo(leg, pos);
}

/*

*/
void turn(int ccw, int hipforward, int hipbackward, int kneeup, int kneedown, long timeperiod, int leanangle) {
  // use tripod groups to turn in place
  if (ccw) {
    int tmp = hipforward;
    hipforward = hipbackward;
    hipbackward = tmp;
  }

#define NUM_TURN_PHASES 6
#define FBSHIFT_TURN    40   // shift front legs back, back legs forward, this much
  
  long t = millis()%timeperiod;
  long phase = (NUM_TURN_PHASES*t)/timeperiod;

  //Serial.print("PHASE: ");
  //Serial.println(phase);

  switch (phase) {
    case 0:
      // in this phase, center-left and noncenter-right legs raise up at
      // the knee
      setLeg1(TRIPOD1_LEGS, NOMOVE, kneeup, 0);
      break;

    case 1:
      // in this phase, the center-left and noncenter-right legs move clockwise
      // at the hips, while the rest of the legs move CCW at the hip
      setLeg2(TRIPOD1_LEGS, hipforward, NOMOVE, FBSHIFT_TURN, 1);
      setLeg2(TRIPOD2_LEGS, hipbackward, NOMOVE, FBSHIFT_TURN, 1);
      break;

    case 2: 
      // now put the first set of legs back down on the ground
      setLeg1(TRIPOD1_LEGS, NOMOVE, kneedown, 0);
      break;

    case 3:
      // lift up the other set of legs at the knee
      setLeg1(TRIPOD2_LEGS, NOMOVE, kneeup, 0);
      break;
      
    case 4:
      // similar to phase 1, move raised legs CW and lowered legs CCW
      setLeg2(TRIPOD1_LEGS, hipbackward, NOMOVE, FBSHIFT_TURN, 1);
      setLeg2(TRIPOD2_LEGS, hipforward, NOMOVE, FBSHIFT_TURN, 1);
      break;

    case 5:
      // put the second set of legs down, and the cycle repeats
      setLeg1(TRIPOD2_LEGS, NOMOVE, kneedown, 0);
      break;  
  }
  
}
