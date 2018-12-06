/*
Module to Control Timer A.
*/

#include <stdint.h>
#include <stdio.h>
#include "Timer.h"
#include <tm4c123gh6pm.h>
#include "GPIO.h"

uint32_t timerCounter = 0;

void TimerA_Handler( void );

static volatile int TIMEDOUT;

void Timer_setUp( void ) {
  ENABLEINT = (0x01 << 19);             //Enable TimerA interrupt
  SYSCTL_RCGC2_R |= 0x20;               //Enable Run Mode Clock Gating Control Register 2
  RCGCTIMER |= 0x01;                    //select Timer 0 in gating control reg
  GPTMCTL &= ~0x01;                     //disable timer in Timer 0 control reg 
  GPTMCFG = 0x00;                       //set timer to 32bit mode, and not a RTC
  GPTMTAMR = 0x02;                      //configure Timer_0_A to periodic mode
  GPTMTAMR &= ~0x10;                    //Direction is set to down counter
  GPTMTAILR = 0x3E80;                   //16MH / 1000 to return time in milliseconds
  GPTMIMR = 0x00000001;                 //enable time-out interrupt
  GPTMCTL |= 0x01u;                     //re-enable timer in Timer 0 control reg 
}

void TimerA_Handler( void ){
      timerCounter++;                     //increment counter by 1
     GPTMICR |= 0x01; 	                //Clear the interrupt
}

uint32_t Timer_millis( void ){
  return timerCounter;
}