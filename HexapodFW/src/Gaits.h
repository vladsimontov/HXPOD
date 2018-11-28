#ifndef GAITS_H 
#define GAITS_H

#include <stdint.h>
#include <stdio.h>
#include "PCA9685.h"

//==============================================================================
#define NUM_LEGS 6

 // Bit patterns for the different combinations of legs
 // bottom six bits. LSB is leg number 0
 #define ALL_LEGS      0x3F //0b 11 1111
 #define LEFT_LEGS     0x38 //0b 11 1000
 #define RIGHT_LEGS    0x07 //0b 00 0111
 #define TRIPOD1_LEGS  0x15 //0b 01 0101
 #define TRIPOD2_LEGS  0x2A //0b 10 1010
 #define FRONT_LEGS    0x21 //0b 10 0001
 #define MIDDLE_LEGS   0x12 //0b 01 0010
 #define BACK_LEGS     0x0C //0b 00 1100
 #define NO_LEGS       0x00 //0b 00 0000

// individual leg bitmasks
#define LEG0 0x01 //0b1
#define LEG1 0x02 //0b10
#define LEG2 0x04 //0b100
#define LEG3 0x08 //0b1000
#define LEG4 0x10 //0b10000
#define LEG5 0x20 //0b100000

#define LEG0BIT  0x01 //0b1
#define LEG1BIT  0x02 //0b10
#define LEG2BIT  0x04 //0b100
#define LEG3BIT  0x08 //0b1000
#define LEG4BIT  0x10 //0b10000
#define LEG5BIT  0x20 //0b100000

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

typedef enum phaseType
{
  SITTING,
  STANDING,
  INIT_WALK,
  WALKING,
  DONE_WALKING,
  TRIPOD1_LIFT,
  TRIPOD1_SWIVEL,
  TRIPOD1_SET,
  TRIPOD2_LIFT,
  TRIPOD2_SWIVEL,
  TRIPOD2_SET,
} phase_t;
  

typedef enum gaitCommand
{
  BOT_STAND,
  BOT_SIT,
  BOT_WALK_FWD,
  BOT_WALK_BACK,
  BOT_WALK_NW,
  BOT_WALK_NE,
  BOT_WALK_SW,
  BOT_WALK_SE,
  BOT_ROTATE_LEFT,
  BOT_ROTATE_RIGHT,
  BOT_STOP
} gaitCommand_t;

 //Prototype functions
 void setRotateLegs(uint8_t legmask, int16_t hip_pos, int16_t knee_pos, uint8_t adj);
 void setWalkLegs(uint8_t legmask, int16_t hip_pos, int16_t knee_pos, uint8_t adj);
 void setLegs(uint8_t legmask, int16_t hip_pos, int16_t knee_pos, uint8_t adj, uint8_t raw, int16_t leanangle);
 void setKneesOnly(uint8_t legMask, int16_t knee_pos);
 void stand();
 void laydown();
 void gait_tripod(uint8_t reverse, uint8_t hipforward, uint8_t hipbackward, uint8_t kneeup, uint8_t kneedown, long timeperiod, uint8_t leanangle);
 void transactServos();
 void commitServos();
 void setHip(uint8_t leg, int16_t pos, uint8_t adj);
 void setHipRaw(uint8_t leg, int16_t pos);
 void setKnee(uint8_t leg, int16_t pos);
 void turn(uint8_t ccw, uint8_t hipforward, uint8_t hipbackward, int16_t kneeup, int16_t kneedown, long timeperiod, uint8_t leanangle);
 phase_t walk_FSM( gaitCommand_t newCmd);
 phase_t turn_FSM( gaitCommand_t newCmd);

#endif