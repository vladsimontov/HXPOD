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
  //Initial Test comment for commit to github
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

  }
return 0;
}
