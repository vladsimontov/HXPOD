/*! \project Vorpal Hexapod on TIVA 
*
* \brief
*   The project is based on the project designed Vorpal Robotics, LLC.
*   Control functionality was ported from the Arduino to TIVA, while new driver
*   functions (I2C, Bluetooth, Servo Driving, UART) were written by the authors 
*   stated below.
*
*  \original codebase:
*   Copyright (C) 2017, 2018 Vorpal Robotics, LLC.
*   https://github.com/vorpalrobotics/VorpalHexapod
*
*
* \author vsimontov
*         livey
*         clopez
* \info
* All licesning agreements defined by Vorpal Robotics, LLC are in effect
*
*
* For details on programming, refer to TM4C123G datasheet :
* http://www.ti.com/lit/ds/spms376e/spms376e.pdf
*
******************************************************************************/ 

#include <stdint.h>
#include <stdbool.h>
#include <stdio.h>
#include "tm4c123gh6pm.h"
#include "src/I2C.h"
#include "src/Timer.h"
#include "src/PCA9685.h"
#include "src/Gaits.h"
#include "src/UART.h"
#include "src/Bluetooth.h"

 // Other initializations

 //==============================================================================

int main(void) {
  
  //Initialize Timer for millis() function
  Timer_setUp();
  
  //Initialize I2C to 100kHz clock and Servo Driver to 60Hz PWM
   I2C_InitPort1();
   PCA9685_Init();
   PCA9685_UpdatePWMFrequency(60u);
   PCA9685_Restart();
   
   //Stand the Hexapod, run a demo
   stand();
   demo();
   
   //Initialize UART module 1, pin PC4 is Rx
   BlueTooth_Init(); 
   
   //Confirm UART: stand up and get ready to move
   gaitCommand_t lastCmd = BOT_STAND;
  
  //Main loop: polls for Bluetooth commands and sends them to a single high-level state machine
   while(1){

     checkBlueTooth(&lastCmd);
     runGaitFSM(lastCmd);
   }
 return 0;
 }