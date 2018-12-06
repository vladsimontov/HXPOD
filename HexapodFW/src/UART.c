/*! \file  UART.c
*
* \brief
* UART control functions to be used with the TIVA TM4C123G Development Kit
*
* \details
*  This modules sets up the uart serial driver for receiving communication from
*  any serial device
*
* \author livey
*
* \info
* Based on TIVA User Reference manual, starting on pg.893
*
* This code was tested using an osciliscope to verify proper packet transfers
*
* For details on programming, refer to TM4C123G datasheet :
* http://www.ti.com/lit/ds/spms376e/spms376e.pdf
*
******************************************************************************/

#include "UART.h"
uint8_t storedDataByte = 0;

void UART_InitPort1( void )
/*!\brief   Initialize UART for Port 1. 
\details set the port to 38400 8N1
\return none
*/
{
    //1. Enable the I2C clock using the RCGCI2C register in the System Control module (see page 348).
    RCGC_UART |= (0x01 << 1); //enable UART 1

    //2. Enable the clock to the appropriate GPIO module via the RCGCGPIO register in the System
    //Control module (see page 340). To find out which GPIO port to enable, refer to Table
    //23-5 on page 1351.
    RCGCGPIO |= (0x04);//Clock to port C (pin 4 and 5 for UART Rx and Tx respectively)

    //3. In the GPIO module, enable the appropriate pins for their alternate function using the
    //GPIOAFSEL register (see page 671). To determine which GPIOs to configure, see Table
    ///23-4 on page 1344.
    GPIO_C_AFSEL |= (0x30);//pin 4 and 5 are assigned alternative function
    
    //4. Set drive current and slew rate 
    //(use default 2-mA)
    GPIO_C_DEN |= (0x30);//pin 4 and 5 are assigned alternative function
    GPIO_C_DIR |= (0x20); //pin 5 is an output
    GPIO_C_DR8R |= (0x30);//pin 4 and 5 are assigned alternative function
    GPIO_C_SLR |= (0x30);//pin 4 and 5 are assigned alternative function
    
    //5. Configure the PMCn fields in the GPIOPCTL register to assign the UART signals to the appropriate
    //pins. See page 688 and Table 23-5 on page 1351.
    GPIO_C_PCTL |= (0x22 << (16));

    //6. Disable UART in UART_CTL register
    UART_CTL(1) &= ~UART_ENABLE;
    
    //7. Calculate baudrate divisor for 16MHz system
    //BRD = 16,000,000 / (16 * 38,400) = 26.04167
    //FBRD = .04167 * 64 + .5 = 3.1667
    UART_IBRD(1) = 26;
    UART_FBRD(1) = 3;
    
    //8. Set configuration 
    UART_LCRH(1) |= UART_8BIT_CFG;  //8 bits, (no parity, one stop bit by default)
    UART_LCRH(1) |= UART_FIFO_EN;  //16-entry FIFOs enabled
    
    //9. Interrupts to come
    
    //10. UART_CC is set to the system clock by default
    
    //11. Re-enable UART module
    UART_CTL(1) |= 0x0001;
    
}

UART_status_t UART_ReadByte(uint8_t * dataByte)
/*!\brief   Read one byte from the client
\details: This function will read one data byte and pass back via pointer
\return UART_status_t : status of i2c bus 
              UART_STATUS_OK : UART packet succesfully read
              UART_STATUS_UNKNOWN: default state. Assume the worst and hope for the best
              UART_STATUS_RxEMPTY : The RX buffer is empy
*/
{

     *dataByte = UART1_DATA;
     storedDataByte = *dataByte;
     UART_status_t FIFOStatus = UART_STATUS_UNKNOWN;
      if ((UART_FR(1) & UART_RxFIFO_EMPTY_FLAG) != 0){
         FIFOStatus = UART_STATUS_RxEMPTY;
      }
      else {
        //could add in more error checking here but we don't need it presently
        *dataByte = UART1_DATA;
        FIFOStatus = UART_STATUS_OK;
      }
      return FIFOStatus;
}

uint8_t UART_lastRxByte( void )
/*!\brief   Read back last byte received
\return One byte of data representing last byte received
*/
{
  return storedDataByte;
}

uint8_t UART_Rx_available( void )
/*!\brief   Check if RX data is availble
\details Poll the UART registers to see if data has arrived. 
\return 0 if false
        1 if true
*/
{
  if ((UART_FR(1) & UART_RxFIFO_EMPTY_FLAG) != 0) return 0;
  else return 1;
}