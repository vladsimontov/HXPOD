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
  setUp();
  
  //Initialize I2C to 100kHz clock and Servo Driver to 60Hz PWM
   I2C_InitPort1();
   PCA9685_Init();
   PCA9685_UpdatePWMFrequency(60u);
   PCA9685_Restart();
   
   //Confirm I2C/PCA: chill until we send a wakeup command
   laydown();

   //Initialize UART module 1, pin PC4 is Rx
   BlueTooth_Init(); 
   
   //Confirm UART: stand up and get ready to move
   while(UART1_DATA != 0x05);
   stand();  
   gaitCommand_t lastCmd = BOT_STAND;
  
  //Main loop: polls for Bluetooth commands and sends them to a single high-level state machine
   while(1){
     
     //stand();
     checkBlueTooth(&lastCmd);
     runGaitFSM(lastCmd);
   }
 return 0;
 }