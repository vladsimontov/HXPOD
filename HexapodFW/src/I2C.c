/*! \file  12c.c
*
* \brief
* I2C control functions to be used with the TIVA TM4C123G Development Kit
*
* \details
* All functions:
*    - use the return value to communicate driver status.
*    - return information through pointer arguments.
*
*
* \author vsimontov
*
* \info
* Based on TIVA User Reference manual, starting on pg.997
*
* This code was tested using an osciliscope to verify proper packet transfer
*
* For details on programming, refer to TM4C123G datasheet :
* http://www.ti.com/lit/ds/spms376e/spms376e.pdf
*
******************************************************************************/

#include <stdint.h>
#include <stdio.h>
#include "I2C.h"
#include <tm4c123gh6pm.h>
#include "GPIO.h"

void I2C_InitPort1(void){
    //16.4.1 Configure the I2C Module to Transmit a Single Byte as a Master
    //The following example shows how to configure the I2C module to transmit a single byte as a master.

    //This assumes the system clock is 20 MHz.

    //1. Enable the I2C clock using the RCGCI2C register in the System Control module (see page 348).
    SYSCTL_RCGCI2C_R |= (1<<1);//selected i2c module 1 (will connect to portA pin 6 and 7 with I2C1_SCL and I2C1_SDA respectively

    //2. Enable the clock to the appropriate GPIO module via the RCGCGPIO register in the System
    //Control module (see page 340). To find out which GPIO port to enable, refer to Table
    //23-5 on page 1351.
    SYSCTL_RCGCGPIO_R |= (1<<0);//Clock to port A (pin 6 and 7 will have I2C1_SCL and I2C1_SDA respectively)


    //3. In the GPIO module, enable the appropriate pins for their alternate function using the
    //GPIOAFSEL register (see page 671). To determine which GPIOs to configure, see Table
    ///23-4 on page 1344.
    GPIO_PORTA_AFSEL_R |= 0xC0;//pin 6 and 7 are assigned alternative function. Nanmely connecting to I2C1_SCL and I2C1_SDA respectively
    GPIO_PORTA_DEN_R |= (0xC0);//set pin 6 and 7 to be digitcal pins
    GPIO_PORTA_PUR_R |= 0xC0;//SDA and SCL pulled up


    //4. Enable the I2CSDA pin for open-drain operation. See page 676.
    GPIO_PORTA_ODR_R |= 0x80;//set SDA pin to open drain (bit 7 (pin 8) is set to high)

    //5. Configure the PMCn fields in the GPIOPCTL register to assign the I2C signals to the appropriate
    //pins. See page 688 and Table 23-5 on page 1351.
    GPIO_PORTA_PCTL_R |= (0x33 <<24);

    //6. Initialize the I2C Master by writing the I2CMCR register with a value of 0x0000.0010.
    I2C1_MCR_R=0x10;

    //7. Set the desired SCL clock speed of 100 Kbps by writing the I2CMTPR register with the correct
    //value. The value written to the I2CMTPR register represents the number of system clock periods
    //in one SCL clock period. The TPR value is determined by the following equation:
    //TPR = (System Clock/(2*(SCL_LP + SCL_HP)*SCL_CLK))-1;
    //TPR = (16MHz/(2*(6+4)*100000))-1;
    //TPR = 7
    //Write the I2CMTPR register with the value of 0x0000.0007.
    I2C1_MTPR_R=0x7;

}

i2c_status_t I2C_WriteByte(uint8_t address, uint8_t data)
/*!\brief   Write one byte to the client
\details: This function will send one byte to the client, then check for any 
          error condition after transfer. Reference p.1008 in datasheet
\return i2c_status_t : status of i2c bus
                i2c_OK : I2C transfer completed successfully
                i2c_CLK_TO : Clock timeout error has occurred.
                i2c_NO_ACK : I2C did not acknowledge
                i2c_ERROR : The error can be from the slave address not being 
                            acknowledged or the transmit data not being acknowledged.
*/
{
  I2C1_MSA_R = ((address << 1) | WRITE); //Write Address
  I2C1_MDR_R= data;     //write one byte to the bus
  I2C1_MCS_R= (GEN_START | GEN_RUN | GEN_STOP); //Single  TX
  
  while(I2C1_MCS_R & BUSBSY == BUSBSY); // wait for bus to become idle after tx
  
  while(I2C1_MCS_R & BUSY == BUSY); //wait for controller to become idle after tx
  
  //Check below for errors that occured
  if ((I2C1_MCS_R & ERROR) == ERROR){    
    return i2c_ERROR;
  }
  else if ((I2C1_MCS_R & DATACK) == DATACK){    
    return i2c_NO_ACK;
  }
  else if ((I2C1_MCS_R & CLKTO) == CLKTO){    
    return i2c_CLK_TO;
  }
  //All is well, return an Ok
  else{    
    return i2c_OK;
  }
}

i2c_status_t I2C_WriteBytes(uint8_t address,uint8_t controlRegister, uint8_t data)
/*!\brief   Writes one byte to control address and client address in function.
            This is useful for I2C devices that have control address that 
            need to be specifically written before data is written. 
            For reference p.1008 in datasheet
\return i2c_status_t : status of i2c bus
                i2c_OK : I2C transfer completed successfully
                i2c_CLK_TO : Clock timeout error has occurred.
                i2c_NO_ACK : I2C did not acknowledge
                i2c_ERROR : The error can be from the slave address not being 
                            acknowledged or the transmit data not being acknowledged.
*/
{  
  I2C1_MSA_R = ((address << 1) | WRITE); //Specifiy client address
  I2C1_MDR_R= controlRegister;               //Specifiy clients control address

  I2C1_MCS_R |= (GEN_START | GEN_RUN);       //Generate start condition and tx
  while(I2C1_MCS_R & BUSBSY == BUSBSY);      //Wait for bus to become idle 
  while(I2C1_MCS_R & BUSY == BUSY);          //Wait for controller to become idle


  I2C1_MDR_R= data;                          //Write data to client
  I2C1_MCS_R |= (GEN_RUN | GEN_STOP );       //TX data and generate stop condition

  while(I2C1_MCS_R & BUSBSY == BUSBSY);      //Wait for bus to become idle 
  while(I2C1_MCS_R & BUSY == BUSY);          //Wait for controller to become idle

    //Check below for errors that occured
  if ((I2C1_MCS_R & ERROR) == ERROR){    
    return i2c_ERROR;
  }
  else if ((I2C1_MCS_R & DATACK) == DATACK){    
    return i2c_NO_ACK;
  }
  else if ((I2C1_MCS_R & CLKTO) == CLKTO){    
    return i2c_CLK_TO;
  }
  //All is well, return an Ok
  else{    
    return i2c_OK;
  }

}
i2c_status_t I2C_Read(uint8_t address,uint8_t controlRegister, uint8_t * data)
/*!\brief   Requests to read one byte from the client. Sends the Read Request to 
            specific control register, then performs read
            For reference p.1008 in datasheet
\return i2c_status_t : status of i2c bus
                i2c_OK : I2C transfer completed successfully
                i2c_CLK_TO : Clock timeout error has occurred.
                i2c_NO_ACK : I2C did not acknowledge
                i2c_ERROR : The error can be from the slave address not being 
                            acknowledged or the transmit data not being acknowledged.
*/
{
    
  if (I2C_WriteByte(address,controlRegister) == i2c_OK){
    I2C1_MSA_R = ((address << 1) | READ);
    I2C1_MCS_R = (GEN_START | GEN_RUN | GEN_STOP);   //Single byte TX
    while(I2C1_MCS_R & BUSY == BUSY);
    while(I2C1_MCS_R & BUSBSY == BUSBSY);
    *data=I2C1_MDR_R;
    if (I2C1_MCS_R & ERROR == ERROR){
      //*data = 0xff;
      return i2c_ERROR;
    }
    else{
      //*data=I2C1_MDR_R;
      return i2c_OK;
    }
  }
  else{
    return i2c_WRITE_ERROR;
  }
}
