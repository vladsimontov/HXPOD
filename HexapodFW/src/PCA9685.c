/*! \file  PCA9685.c
*
* \brief  Driver module for interfacing with PCA9685. 
*
* \details This device is 16 channel, 12-bit used for driving LEDs or Servos 
*          with PWMs. In this module, we allow the frequency and duty cycle to 
*          be changed.
*       All functions:
*           - use the return value to communicate driver status.
*           - return information through pointer arguments.
*
*
* \author vsimontov
*
*
* This code was tested using an osciliscope to verify frequency and duty cycle
*
* For details on programming, refer to PCA9685 datasheet :
* https://cdn-shop.adafruit.com/datasheets/PCA9685.pdf
*
******************************************************************************/

#include <stdint.h>
#include <stdio.h>
#include "PCA9685.h"
#include <tm4c123gh6pm.h>
#include "GPIO.h"
#include "I2C.h"
#include <math.h>

pca9685_status_t PCA9685_Init(void)
/*! \brief   Initialize the PCA9685 for use. 
    \details: Set MODE1 enable restarts and all calls to all channels
          set MODE2 to output all changes on ACKs
    \return pca9685_status_t : status of i2c bus
                PCA_9685_OK : I2C transfer completed successfully
                PCA_9685_NOT_SET : Clock timeout error has occurred.
*/
{
  i2c_status_t writePrescaleMode1 = I2C_WriteBytes(PCA_9685_ADDR, MODE1,(EN_RST | EN_ALLCALL));  
  i2c_status_t writePrescaleMode2 = I2C_WriteBytes(PCA_9685_ADDR, MODE2, OCH_ACK);
  if((writePrescaleMode1 == i2c_OK) && (writePrescaleMode2 == i2c_OK))
    return PCA_9685_OK;
  else
    return PCA_9685_NOT_SET;
  
}

pca9685_status_t PCA9685_Restart(void)
/*! \brief   Software restart the PCA9685
    \details: Checks MODE1 to see if Restart bit is high, indicates that a restart
          is needed. 
    \return pca9685_status_t : status of i2c bus
                PCA_9685_OK : Unit is restarted and ready to accept new commands
                PCA_9685_NOT_SET : Unit has not accepted the restart sequence
                PCA_9685_UNKNOWN : Unknown error condition
*/
{
  uint8_t modeStatus=0x00;
  uint32_t delayCounter = 0;
  uint8_t verifyRestart = 0x0;
  
  //All leds off (write 1 to 4th bit of ALL_LED_OFF_H register pg.15 of datasheet)
  //fastest way to shutdown
  i2c_status_t writeOff = I2C_WriteBytes(PCA_9685_ADDR, ALL_LED_OFF_H, (0x01 << 4));
  i2c_status_t writeOff1 = I2C_WriteBytes(PCA_9685_ADDR, ALL_LED_OFF_L, (0x01 << 4));


  if(I2C_Read(PCA_9685_ADDR, MODE1, &modeStatus) == i2c_OK){   
    
    //trigger a restart if the bit is high
    if((modeStatus & RESTART) == RESTART){  
      
      I2C_WriteBytes(PCA_9685_ADDR, MODE1, (modeStatus | RESTART));
      //arbitrary delay here. Need to scope out this delay
      while(delayCounter < 100000u){
        delayCounter++;
      }
    }
    
    I2C_Read(PCA_9685_ADDR, MODE1, &verifyRestart);
    if ((verifyRestart & RESTART) == 0)
      return PCA_9685_OK;
    else
      return PCA_9685_NOT_SET;
  }
  return PCA_9685_UNKNOWN;
}


pca9685_status_t PCA9685_UpdatePWMFrequency(uint16_t newFrequency)
/*! \brief   Sets the desired frequency of the PCA9685 output pins
    \notes: frequency should be between 24hz and 1526hz
    \param[in]: newFrequency, desired frequency value
    \return pca9685_status_t : status of i2c bus
                PCA_9685_OK : Frequency has been set and old mode has been restored
                PCA_9685_NOT_SET : The old mode has not been restored, problems happened
*/
{
  if (newFrequency < MIN_HZ)  newFrequency = MIN_HZ;
  if (newFrequency > MAX_HZ)  newFrequency = MAX_HZ;
  
  uint32_t delayCounter = 0;
  uint8_t oldMode=0x00;
  float prescaleVal = CLKRATE;
  
  prescaleVal /= 4096;
  prescaleVal /= newFrequency;
  prescaleVal -= 1;
  uint8_t prescale = (uint8_t)floor(prescaleVal);  
  
  
  I2C_Read(PCA_9685_ADDR, MODE1, &oldMode);

  uint8_t newMode = ((oldMode & ~RESTART) | SLEEP); // check if sleep is 1, otherwise prescale writes are blocked
  I2C_WriteBytes(PCA_9685_ADDR, MODE1, (uint8_t)(newMode));  
 
  //write new value
  I2C_WriteBytes(PCA_9685_ADDR, PRESCALE, (uint8_t) prescale);  
  
  //reset the original mode register
  I2C_WriteBytes(PCA_9685_ADDR, MODE1, (uint8_t)(oldMode));  
  
  //bit of a delay here to allow things to stabilize
  while(delayCounter < 100000u){
        delayCounter++;
  }
  uint8_t verifyOldMode = 0x00;
  i2c_status_t verifyModeStatus = I2C_Read(PCA_9685_ADDR, MODE1, &verifyOldMode);

  //check to make sure the old mode was reset
  if((verifyOldMode == oldMode) && (verifyModeStatus == i2c_OK))
    return PCA_9685_OK;
  else
    return PCA_9685_NOT_SET;

}

void PCA9685_convertDutyCycleToCounts(float dutyCycle, uint16_t * highCount, uint16_t * lowCount)
/*!\brief   Converts duty cycle(%) to counts as understood by the PCA9685
\notes: Operating frequency should be 60hz for Servo motoros
        100% Duty Cycle is 0x0FFF (4095 counts)
          0% Duty Cycle is 0x199 (409 counts)
\param dutyCycle[in]: on-time percentage of square wave
       highCount[out]: upper 8-bit value of the 16-bit register as needed by PCA
       lowCount[out]:  lower 8-bit value of the 16-bit register as needed by PCA
\return none 
*/
{
  //check duty cycle bounds and clamp
  if (dutyCycle < 0) dutyCycle = 0;
  if (dutyCycle > 100) dutyCycle = 100;
  
  //convert duty cycle into counts
  float counts = (float)(((float)dutyCycle/100) * 4095);
    
  
  uint16_t onCounts = (uint16_t)(counts);
  *highCount = ((onCounts >> 8) & 0xff); //isolate upper 8-bits
  *lowCount = onCounts & 0xff;           //isolate lower 8-bits
}
  
void PCA9685_setServo(uint8_t leg, float degree)
/*!\brief   main function to drive each leg
   \details each leg will take a degree position as target input, and this function
            will translate the required degree to duty cycle/pulse width to enable
            the movement
   \notes: degree range should be clamped between 0 and 180
   \param leg[in]: leg number to be moved
          degree[in]: number of degrees to move leg
   \return none 
*/
{
  //check for proper degree range and leg number, clamp if outside range
  if (degree < 0.0) degree = 0.0;
  if (degree > 180.0) degree = 180.0;
  if(leg > 11) leg = 11;
   
  uint16_t highCount = 0;
  uint16_t lowCount = 0;  
  
  //Set address of new leg   
  uint8_t addr_on_l = (6 + (4*leg));
  uint8_t addr_on_h = (7 + (4*leg));
  uint8_t addr_off_l= (8 + (4*leg));
  uint8_t addr_off_h= (9 + (4*leg));
  
  
  //find out on time
  float onTime = ((ONE_MSEC/ MAX_ROTATION) * degree) + ONE_MSEC; //0 degrees = 1ms on time, 180 degrees = 2ms
  
  //Calibrate PCA9685 pulse width to be what we expect (small error)
  //onTime = ((onTime* 1.0489) - 0.0039); //calibrating the cycle time.
  

  float dutyCycle = (onTime/PERIOD) * 100.0;

  //convert duty cycle to counts as understood by PCA
  PCA9685_convertDutyCycleToCounts(dutyCycle, &highCount, &lowCount);
  
  //Write to the address, make the legs move!
  //(we could optimize this to make one long write instead of four seperate calls)
  i2c_status_t write1 = I2C_WriteBytes(PCA_9685_ADDR, addr_off_h, (uint8_t)highCount);
  i2c_status_t write2 = I2C_WriteBytes(PCA_9685_ADDR, addr_off_l, lowCount);
  i2c_status_t write3 = I2C_WriteBytes(PCA_9685_ADDR, addr_on_h,  0x0F);
  i2c_status_t write4 = I2C_WriteBytes(PCA_9685_ADDR, addr_on_l,  0xFF);
  
}
