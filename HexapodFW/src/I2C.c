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

void I2C_InitPort1(void)
/*!\brief   Initialize I2C to work on Port 1
\details once initialized, I2C will work at 100Kbps. Assumes system clock is 16MHz
        P1[6] is SCLK
        P1[7] is SDA
\return none
*/
{
    //Enable the I2C clock using the RCGCI2C register in the System Control module (see page 348).
    SYSCTL_RCGCI2C_R |= (1<<1);//selected i2c module 1 (will connect to portA pin 6 and 7 with I2C1_SCL and I2C1_SDA respectively

    //Enable the clock to the appropriate GPIO module via the RCGCGPIO register in the System
    SYSCTL_RCGCGPIO_R |= (1<<0);//Clock to port A (pin 6 and 7 will have I2C1_SCL and I2C1_SDA respectively)


    GPIO_PORTA_AFSEL_R |= (PIN6 | PIN7);   //pin 6 and 7 are assigned alternative function. Namely connecting to I2C1_SCL and I2C1_SDA respectively
    GPIO_PORTA_DEN_R |= (PIN6 | PIN7);     //set pin 6 and 7 to be digitcal pins
    GPIO_PORTA_PUR_R |= (PIN6 | PIN7);     //SDA and SCL pulled up


    GPIO_PORTA_ODR_R |= (PIN7); //set SDA pin to open drain (bit 7 (pin 8) is set to high)
    
    GPIO_PORTA_PCTL_R |= (0x33 <<24);

    //Initialize the I2C Master by writing the I2CMCR register with a value of 0x0000.0010.
    I2C1_MCR_R=0x10;

    //Set the desired SCL clock speed of 100 Kbps based on 16Mhz system clock
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
  \detail This is useful for I2C devices that have control address that 
            need to be specifically written before data is written. 
            For reference p.1008 in datasheet
  \param address: address of client
         controlRegister: clients internal address for data
         data: byte to write to client
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
/*!\brief   Requests to read one byte from the client. 
  \detail write to control register prep data for tx. Then request read from client
          and capture data. For reference p.1008 in datasheet
  \param address: address of client (hex)
        controlRegister: clients internal address
        data: data received back from client, passed back
  \return i2c_status_t : status of i2c bus
                i2c_OK : I2C transfer completed successfully
                i2c_CLK_TO : Clock timeout error has occurred.
                i2c_NO_ACK : I2C did not acknowledge
                i2c_ERROR : The error can be from the slave address not being 
                            acknowledged or the transmit data not being acknowledged.
*/
{
  
  //perform initial write to control register
  if (I2C_WriteByte(address,controlRegister) == i2c_OK){
    I2C1_MSA_R = ((address << 1) | READ);            //send read request to client
    I2C1_MCS_R = (GEN_START | GEN_RUN | GEN_STOP);   //Single byte TX
    
    while(I2C1_MCS_R & BUSBSY == BUSBSY);      //Wait for bus to become idle 
    while(I2C1_MCS_R & BUSY == BUSY);          //Wait for controller to become idle
    
    *data=I2C1_MDR_R;                          //Read data from client
    
    //check for errors
    if (I2C1_MCS_R & ERROR == ERROR){
      return i2c_ERROR;
    }
    else{
      return i2c_OK;
    }
  }
  else{
    return i2c_WRITE_ERROR;
  }
}
