#include <stdint.h>
#include <stdbool.h>
#include <stdio.h>
#include "tm4c123gh6pm.h"
#include "src/I2C.h"
#include "src/Timer.h"
#include "src/PCA9685.h"

int main(void) {

  I2C_InitPort1();

  PCA9685_Init();
  PCA9685_UpdatePWMFrequency(60u);
  PCA9685_Restart();

  while(1){

    PCA9685_setServo(90, 5);
    PCA9685_setServo(90, 0);
    PCA9685_setServo(90, 11);

  }
return 0;
}
