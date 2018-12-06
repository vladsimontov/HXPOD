/*! \file  Timer.c
*
* \brief
* Timer control functions to be used with the TIVA TM4C123G Development Kit
*
* \details
*  Sets up the TimerA for use with the Hexapod. Limited functionality as we
*  only need to count milliseconds
*
* \author vsimontov
          clopez
*
* \info
* Based on TIVA User Reference manual, starting on pg.705
*
* This code was tested using an osciliscope to verify acturate clock setting
*
* For details on programming, refer to TM4C123G datasheet :
* http://www.ti.com/lit/ds/spms376e/spms376e.pdf
*
******************************************************************************/

#include <stdint.h>
#include <stdio.h>
#include "Timer.h"
#include <tm4c123gh6pm.h>
#include "GPIO.h"

void TimerA_Handler( void ); /* Protoype def*/

uint32_t timerCounter = 0;
static volatile int TIMEDOUT;


void TimerA_Handler( void )
/*!\brief   ISR for Timer A interupts. 
\details    1ms has elapsed since we got here, increment the global
            variable timerCounter by one
\return none
*/
{
  timerCounter++;     //increment counter by 1
  GPTMICR |= 0x01; 	  //Clear the interrupt
}

void Timer_setUp( void ) 
/*!\brief   Configure timer A for use as a count down timer
\details Load the timer with 1ms worth of ticks based on system clock and
         start counting down.
\return none
*/
{
  
  ENABLEINT = TIMERA_INT;             //Enable TimerA interrupt
  SYSCTL_RCGC2_R |= EN_RUN_GCR;               //Enable Run Mode Clock Gating Control Register 2
  RCGCTIMER |= SEL_TIMER0_GCR;                    //select Timer 0 in gating control reg
  GPTMCTL &= ~ENABLE;                     //disable timer in Timer 0 control reg 
  GPTMCFG = SET_32K_MODE;                       //set timer to 32bit mode, and not a RTC
  GPTMTAMR = PERIODIC_MODE;                      //configure Timer_0_A to periodic mode
  GPTMTAMR &= ~COUNT_DIR;                    //Direction is set to down counter
  GPTMTAILR = ONE_MS;                   //16MH / 1000 to return time in milliseconds
  GPTMIMR = TIMEOUT_INT;                 //enable time-out interrupt
  GPTMCTL |= ENABLE;                     //re-enable timer in Timer 0 control reg 
}


uint32_t Timer_millis( void )
/*!\brief   Wrapper function to access global variable timerCounter
\details Function returns current number of milliseconds since restart of system
\return timerCounter in milliseconds
*/
{
  return timerCounter;
}