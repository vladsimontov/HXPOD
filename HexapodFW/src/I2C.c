/*
Module for all I2C related Drivers
*/

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
    SYSCTL_RCGCI2C_R=(1<<1);//selected i2c module 1 (will connect to portA pin 6 and 7 with I2C1_SCL and I2C1_SDA respectively

    //2. Enable the clock to the appropriate GPIO module via the RCGCGPIO register in the System
    //Control module (see page 340). To find out which GPIO port to enable, refer to Table
    //23-5 on page 1351.
    SYSCTL_RCGCGPIO_R=(1<<0);//Clock to port A (pin 6 and 7 will have I2C1_SCL and I2C1_SDA respectively)


    //3. In the GPIO module, enable the appropriate pins for their alternate function using the
    //GPIOAFSEL register (see page 671). To determine which GPIOs to configure, see Table
    ///23-4 on page 1344.
    GPIO_PORTA_AFSEL_R=0xC0;//pin 6 and 7 are assigned alternative function. Nanmely connecting to I2C1_SCL and I2C1_SDA respectively
    GPIO_PORTA_DEN_R=(0xC0);//set pin 6 and 7 to be digitcal pins
    GPIO_PORTA_PUR_R=0xC0;//SDA and SCL pulled up


    //4. Enable the I2CSDA pin for open-drain operation. See page 676.
    GPIO_PORTA_ODR_R=0x80;//set SDA pin to open drain (bit 7 (pin 8) is set to high)

    //5. Configure the PMCn fields in the GPIOPCTL register to assign the I2C signals to the appropriate
    //pins. See page 688 and Table 23-5 on page 1351.
    GPIO_PORTA_PCTL_R=(0x33 <<24);

    //6. Initialize the I2C Master by writing the I2CMCR register with a value of 0x0000.0010.
    I2C1_MCR_R=0x10;

    //7. Set the desired SCL clock speed of 100 Kbps by writing the I2CMTPR register with the correct
    //value. The value written to the I2CMTPR register represents the number of system clock periods
    //in one SCL clock period. The TPR value is determined by the following equation:
    //TPR = (System Clock/(2*(SCL_LP + SCL_HP)*SCL_CLK))-1;
    //TPR = (16MHz/(2*(6+4)*100000))-1;
    //TPR = 7
    //Write the I2CMTPR register with the value of 0x0000.0007.
    I2C1_MTPR_R=0x9;

}

void I2C_initialize(void){
/*1. Enable the I2C clock using the RCGCI2C register in the System Control module (see page 348).*/
  RCGCI2C |= R0; //Enable clock on I2C
  
  /*2. Enable the clock to the appropriate GPIO module via the RCGCGPIO register 
       in the System Control module (see page 340). To find out which GPIO port 
       to enable, refer to Table 23-5 on page 1351.*/
  RCGCGPIO_B |= PORTB; //Enable Port B for I2C_0 block
  
  /*3. In the GPIO module, enable the appropriate pins for their alternate 
       function using the GPIOAFSEL register (see page 671). To determine 
       which GPIOs to configure, see Table 23-4 on page 1344.*/
  
  GPIO_B_PCTL |= 0x3;
  GPIO_B_CR |= (1 << PIN2); 
  GPIO_B_CR |= (1 << PIN3); 
  
  
  GPIO_B_PCTL |= (0x3 << PMC2);
  GPIO_B_PCTL |= (0x3 << PMC3);
    
  I2CMCR |= MFE; //Enable Master Mode
  I2CMCR |= CLK_100KBPS;
  
  I2CMDR = (0xFF);
  I2CMCS = (0x00000007);
  while(I2CMCS & BUSY == BUSY);
    
}

i2c_status_t I2C_WriteByte(uint8_t address, uint8_t data){
//function to write, as detailed on p.1008
  I2C1_MSA_R = ((address << 1) | WRITE);
  I2C1_MDR_R= data;//write the register to read from
  I2C1_MCS_R= (GEN_START | GEN_RUN | GEN_STOP); //Single byte TX
  while(I2C1_MCS_R & BUSBSY == BUSBSY);
  while(I2C1_MCS_R & BUSY == BUSY); //wait for finish
  
  if ((I2C1_MCS_R & ERROR) == ERROR){    
    return i2c_ERROR;
  }
  else if ((I2C1_MCS_R & DATACK) == DATACK){    
    return i2c_NO_ACK;
  }
  else if ((I2C1_MCS_R & CLKTO) == CLKTO){    
    return i2c_CLK_TO;
  }
  else{    
    return i2c_OK;
  }
}
#if 0 
i2c_status_t I2C_WriteBytes(uint8_t address,uint8_t controlRegister, uint8_t data, uint8_t operation){
//function to write, as detailed on p.1008
  
  uint8_t ad_ack = 0;
  uint8_t dat_ack = 0;

  uint8_t op =  ((address << 1) | operation);
  I2C1_MSA_R = op;
  I2C1_MCS_R |= (GEN_START | GEN_RUN);
  while(I2C1_MCS_R & BUSY == BUSY); //wait for finish
  while(I2C1_MCS_R & BUSBSY == BUSBSY);
  if (I2C1_MCS_R & ADRACK == ADRACK){ 
    ad_ack++;
  }

    
    
  I2C1_MDR_R= controlRegister;//write the register to read from
  I2C1_MCS_R |= (GEN_RUN); //Single byte TX
  while(I2C1_MCS_R & BUSY == BUSY); //wait for finish
  while(I2C1_MCS_R & BUSBSY == BUSBSY);
  
  if (I2C1_MCS_R & DATACK == DATACK){ 
    dat_ack++;
  }
  I2C1_MDR_R= data;//write the register to read from
  I2C1_MCS_R |= (GEN_RUN | GEN_STOP ); //Single byte TX

  while(I2C1_MCS_R & BUSY == BUSY); //wait for finish
  while(I2C1_MCS_R & BUSBSY == BUSBSY);
  if (I2C1_MCS_R & DATACK == DATACK){ 
      dat_ack++;
    }
  if (I2C1_MCS_R & ERROR == ERROR){    
    return i2c_ERROR;
  }
  else{    
    return i2c_OK;
  }

}

#else
i2c_status_t I2C_WriteBytes(uint8_t address,uint8_t controlRegister, uint8_t data, uint8_t operation){
//function to write, as detailed on p.1008
  
  uint8_t ad_ack = 0;
  uint8_t dat_ack = 0;

  I2C1_MSA_R = ((address << 1) | operation);
  I2C1_MDR_R= controlRegister;//write the register to read from

  I2C1_MCS_R |= (GEN_START | GEN_RUN);
  while(I2C1_MCS_R & BUSY == BUSY); //wait for finish
  while(I2C1_MCS_R & BUSBSY == BUSBSY);
  if (I2C1_MCS_R & ADRACK == ADRACK){ 
    ad_ack++;
  }

  I2C1_MDR_R= data;//write the register to read from
  I2C1_MCS_R |= (GEN_RUN | GEN_STOP ); //Single byte TX

  while(I2C1_MCS_R & BUSY == BUSY); //wait for finish
  while(I2C1_MCS_R & BUSBSY == BUSBSY);
  if (I2C1_MCS_R & DATACK == DATACK){ 
      dat_ack++;
    }
  if (I2C1_MCS_R & ERROR == ERROR){    
    return i2c_ERROR;
  }
  else{    
    return i2c_OK;
  }

}
#endif
i2c_status_t I2C_Read(uint8_t address,uint8_t controlRegister, uint8_t * data){
  i2c_status_t writeStatus = I2C_WriteByte(address,controlRegister);
  
  
  if (writeStatus == i2c_OK){
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

i2c_status_t I2C_WriteManyBytes(uint8_t address,uint8_t controlRegister, uint8_t data[4], uint8_t operation){
//function to write, as detailed on p.1008
  
  uint8_t ad_ack = 0;
  uint8_t dat_ack = 0;

  I2C1_MSA_R = ((address << 1) | operation);
    I2C1_MDR_R= controlRegister;//write the register to read from

  I2C1_MCS_R |= (GEN_START | GEN_RUN);
  while(I2C1_MCS_R & BUSY == BUSY); //wait for finish
  while(I2C1_MCS_R & BUSBSY == BUSBSY);
  if (I2C1_MCS_R & ADRACK == ADRACK){ 
    ad_ack++;
  }

    
#if 0 
  I2C1_MDR_R= controlRegister;//write the register to read from
  I2C1_MCS_R |= (GEN_RUN); //Single byte TX
  while(I2C1_MCS_R & BUSY == BUSY); //wait for finish
  while(I2C1_MCS_R & BUSBSY == BUSBSY);
  
  if (I2C1_MCS_R & DATACK == DATACK){ 
    dat_ack++;
  }
#else
  for (uint8_t i = 0; i < 4; i++){
         I2C1_MDR_R= data[i];//write the register to read fro
     I2C1_MCS_R |= (GEN_RUN); //Single byte TX
  

     while(I2C1_MCS_R & BUSY == BUSY); //wait for finish
     while(I2C1_MCS_R & BUSBSY == BUSBSY);
     if (I2C1_MCS_R & DATACK == DATACK){ 
         dat_ack++;
       }
  }
#endif
  I2C1_MCS_R |= (GEN_STOP ); //Single byte TX
  if (I2C1_MCS_R & ERROR == ERROR){    
    return i2c_ERROR;
  }
  else{    
    return i2c_OK;
  }

}
