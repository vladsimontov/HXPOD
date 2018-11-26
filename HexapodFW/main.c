 #include <stdint.h>
 #include <stdbool.h>
 #include <stdio.h>
 #include "tm4c123gh6pm.h"
 #include "src/I2C.h"
 #include "src/Timer.h"
 #include "src/PCA9685.h"
#include "src/Gaits.h"
#include "src/UART.h"

 // Other initializations
 //==============================================================================

int main(void) {
  //Initialize UART module 1, pin PC4 is Rx
  UART_InitPort1();
  
  //Initialize I2C to 100kHz clock and Servo Driver to 60Hz PWM
   I2C_InitPort1();
   PCA9685_Init();
   PCA9685_UpdatePWMFrequency(60u);
   PCA9685_Restart();

   //Chill until we send a wakeup command
   laydown();
/*   
   PCA9685_setServo(90, 5);
   PCA9685_setServo(90, 0);
   PCA9685_setServo(90, 11);
 */      

 gaitCommand_t newCmd = BOT_STOP;       
  //Main loop: polls for Bluetooth commands
   while(1){
    //parseBluetoothInfo(&newCmd);
    //checkUltrasonicSensor(&newCmd);
    //Wishful thinking: getTiltData(&xTilt, &yTilt, &zTilt); monitorServoCurrent();
    //monitorbatterylife(); ??
    
    /*
     if (executeNextCmdAt < millis()){ //set the next leg movement when the timer
      GaitHandler(newCmd);
     }
    */
   
   }
 return 0;
 }