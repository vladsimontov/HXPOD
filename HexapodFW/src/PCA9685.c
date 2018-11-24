/*
Module to control PCA9685 to drive servo motors
*/

#include <stdint.h>
#include <stdio.h>
#include "PCA9685.h"
#include <tm4c123gh6pm.h>
#include "GPIO.h"
#include "I2C.h"
#include <math.h>

void PCA9685_Init(void){
#define MODE2 (0x01)
  uint8_t mode2Status = 0xff;
  i2c_status_t writePrescale = I2C_WriteBytes(PCA_9685_ADDR, MODE1,0x81,WRITE);  
  i2c_status_t prescaleStatus = I2C_Read(PCA_9685_ADDR, MODE2, &mode2Status);
    i2c_status_t writePrescaleMOde2 = I2C_WriteBytes(PCA_9685_ADDR, MODE2,0x04,WRITE);  


  
}
pca9685_status_t PCA9685_Restart(void){
  uint8_t modeStatus=0x00;
  uint8_t enableSleep = 0x00;
  uint32_t delayCounter = 0;
  uint8_t verifyRestart = 0x0;
  
  //All leds off (write 1 to 4th bit of ALL_LED_OFF_H register)
i2c_status_t writeOff = I2C_WriteBytes(PCA_9685_ADDR, (0xFD), (0x01 << 4), WRITE);

i2c_status_t writeOff1 = I2C_WriteBytes(PCA_9685_ADDR, (0xFC), (0x01 << 4), WRITE);

  
  
  i2c_status_t prescaleStatus = I2C_Read(PCA_9685_ADDR, MODE1, &modeStatus);
  if((modeStatus & RESTART) == RESTART){
    enableSleep = (modeStatus & (~SLEEP)); //Clear the sleep bit
    while(delayCounter < 100000u){
      delayCounter++;
    }
    i2c_status_t writePrescale = I2C_WriteBytes(PCA_9685_ADDR, MODE1, (modeStatus | RESTART), WRITE);
  }
    i2c_status_t verifyRestartStatus = I2C_Read(PCA_9685_ADDR, MODE1, &verifyRestart);

}

pca9685_status_t PCA9685_Sleep(void)
//Sleep the PCA to make modifications to the PRESCALE
{
  uint8_t sleepResponse=0x00;
  uint8_t modeStatus=0x00;

  //write a 1 to the sleep bit on MODE1 register, this will 
  i2c_status_t prescaleStatus = I2C_Read(PCA_9685_ADDR, MODE1, &modeStatus);
  
  i2c_status_t writePrescale = I2C_WriteBytes(PCA_9685_ADDR, MODE1, (modeStatus | SLEEP), WRITE);  
  
  //read MODE register and make sure unit is sleeping
  i2c_status_t prescaleStatus1 = I2C_Read(PCA_9685_ADDR, MODE1, &sleepResponse);
  
  if((sleepResponse & SLEEP) == SLEEP)
    return PCA_9685_OK;
  else
    return PCA_9685_UNRESPONSIVE;

}

pca9685_status_t PCA9685_Wake(void)
//Sleep the PCA to make modifications to the PRESCALE or other registers
{
  uint8_t currentVal1= 0x00;
  uint8_t updatedVal = 0x00;

  i2c_status_t prescaleStatus3 = I2C_Read(PCA_9685_ADDR, MODE1, &currentVal1);
  uint8_t enableSleep = (currentVal1 & (~SLEEP)); //should be 0xEE
  
  
  i2c_status_t writePrescale = I2C_WriteBytes(PCA_9685_ADDR, MODE1,enableSleep,WRITE); 
  i2c_status_t prescaleStatus4 = I2C_Read(PCA_9685_ADDR, MODE1, &updatedVal);

  if ((~updatedVal & SLEEP) == SLEEP)
    return PCA_9685_OK;
  else
    return PCA_9685_NOT_SET;
    
}

pca9685_status_t PCA9685_UpdatePWMFrequency(uint16_t newFrequency)
/*Sets the new desired frequency of the pwm. Must be between 24hz and 1526hz
*/
{
  uint32_t delayCounter = 0;
  if (newFrequency < MIN_HZ)  newFrequency = MIN_HZ;
  if (newFrequency > MAX_HZ)  newFrequency = MAX_HZ;
  
  //0.5 to round up.
  float prescaleVal = CLKRATE;
  prescaleVal /= 4096;
  prescaleVal /= newFrequency;
  prescaleVal -= 1;
  uint8_t prescale = (uint8_t)floor(prescaleVal);  
  
  uint8_t oldMode=0x00;
  i2c_status_t oldModeStatus = I2C_Read(PCA_9685_ADDR, MODE1, &oldMode);

  uint8_t newMode = ((oldMode & 0x7f) | SLEEP); // check if sleep is 1, otherwise prescale writes are blocked
  i2c_status_t writePrescale = I2C_WriteBytes(PCA_9685_ADDR, MODE1, (uint8_t)(newMode),WRITE);  
 
  i2c_status_t prescaleStatus = I2C_WriteBytes(PCA_9685_ADDR, PRESCALE, (uint8_t) prescale,WRITE);  
  i2c_status_t writePrescale1 = I2C_WriteBytes(PCA_9685_ADDR, MODE1, (uint8_t)(oldMode),WRITE);  
  
  while(delayCounter < 100000u){
        delayCounter++;
      }
  uint8_t verifyOldMode = 0x00;
  i2c_status_t verifyModeStatus = I2C_Read(PCA_9685_ADDR, MODE1, &verifyOldMode);

  if(writePrescale1 == i2c_OK)
    return PCA_9685_OK;
  else
    return PCA_9685_NOT_SET;

  
}

void PCA9685_convertDutyCycleToCounts(float dutyCycle, uint16_t * highCount, uint16_t * lowCount){
  
  //Operating frequency is 60hz. One period is 1/60hz = 0.01666666666
  //100% duty cycle is 0x0FFF, 4095 counts
  //10% duty cycle is 0x199, 409 counts
  
  //duty cycle is a percentage between 0 and 100
  if (dutyCycle < 0) dutyCycle = 0;
  if (dutyCycle > 100) dutyCycle = 100;
  
  float counts = (float)(((float)dutyCycle/100) * 4095);
    
  uint16_t onCounts = (uint16_t)(counts);
  *highCount = ((onCounts >> 8) & 0xff);
  *lowCount = onCounts & 0xff;

}

void PCA9685_SetDegreeCycle(float degree){
 //Sets channel 4 to duty cycle that pertains to the degree of movement. May need
  //to calibrate this to ensure 0degrees is equal to 1ms and 180degrees is 2ms
  
  float onTime = ((ONE_MSEC/ MAX_ROTATION) * degree) + ONE_MSEC; //0 degrees = 1ms on time, 180 degrees = 2ms  
  float dutyCycle = (onTime/PERIOD) * 100.0;
  PCA9685_SetDutyCycle(dutyCycle);

}

void PCA9685_SetDutyCycle(float dutyCycle){
  uint16_t highCount = 0;
  uint16_t lowCount = 0;
  PCA9685_convertDutyCycleToCounts(dutyCycle, &highCount, &lowCount);
  i2c_status_t write1 = I2C_WriteBytes(PCA_9685_ADDR,LED4_OFF_H, (uint8_t)highCount, WRITE);
  i2c_status_t write2 = I2C_WriteBytes(PCA_9685_ADDR,LED4_OFF_L, lowCount, WRITE);
  i2c_status_t write3 = I2C_WriteBytes(PCA_9685_ADDR,LED4_ON_H, 0x0F, WRITE);
  i2c_status_t write4 = I2C_WriteBytes(PCA_9685_ADDR,LED4_ON_L, 0xFF, WRITE);
}
