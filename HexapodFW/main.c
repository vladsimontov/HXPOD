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
  
  //Initialize UART module 1, pin PC4 is Rx
   BlueTooth_Init();
  //Initialize I2C to 100kHz clock and Servo Driver to 60Hz PWM
   I2C_InitPort1();
   PCA9685_Init();
   PCA9685_UpdatePWMFrequency(60u);
   PCA9685_Restart();

   //Chill until we send a wakeup command
   laydown();
 
   //Test UART (to remove)
  if(UART1_DATA == 0x04) {
    stand();
    while(1);
  }
  gaitCommand_t lastCmd = BOT_STAND;
  //Main loop: polls for Bluetooth commands and sends them to a single high-level state machine
   while(1){
     checkBlueTooth(&lastCmd);
     runGaitFSM(lastCmd);
   }
 return 0;
 }