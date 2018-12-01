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

void setUp( void ) {
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
  //if((GPTMRIS & 0x00000001) == 0x1){	//Check for interrupt
     timerCounter++;                     //increment counter by 1
   //  if(timerCounter == 1000){           //1 second (1000 milliseconds)
   //  timerCounter = 0;                   //reset counter
   //  }
     GPTMICR |= 0x01; 	                //Clear the interrupt
   //}
}

uint32_t millis( void ){
  return timerCounter;
}

/*
void TIMER_InitTimerA(uint32_t systemSpeed){ 
 /*Initialize Timer A to be a 32 Bit periodic counter, 
  which counts down from 16,000,000 to 0.

  //Initialization instructions on p.722  
  RCGCTIMER |= TIMER0;           //Enable TImer 0 
  GPTMCTL   &= (~GPTMCTL_ENABLE); //Disable Counter for now, drive pin low
   GPTMCTL |= TAOTE; 
  GPTMCFG   = 0x00000000;
  GPTMTAMR  |= PERIODIC_TIMER;    //Set to be Periodic 
  if (systemSpeed == SYS_CLK_4MHZ)
    GPTMTAILR = 4000000;        //Set Timer to 16Mticks
  else if (systemSpeed == SYS_CLK_80MHZ)
    GPTMTAILR = 80000000;  
  
  GPTMCTL |= TIMERENABLE; 

}

void  enableTimerInterrupt(void){
  GPTMICR   |= CLEARINT;     //Clear Previous Interupt Flags, if any
  ENABLEINT |= TIMERA_INT; //Enable interupts for Timer 0A
  GPTMIMR   |= TATOIM;       //Set Interupt Mask to Capture Mode Events
}

void TIMER_Wait(uint32_t milliseconds){
  
  TIMEDOUT = 0;            //Clear finished flag
  enableTimerInterrupt();
  while (TIMEDOUT==0 && STATECHANGED == 0);     //Wait for ISR to set this flag
}

void TIMER_InitSystemClock(uint32_t clkSpeed){

// 0) Use RCC2  
  SYSCTL_RCC2_R |=  USERCC2;         // USERCC2// 1) bypass PLL while initializing  
  SYSCTL_RCC2_R |=  BYPASSPLL;         // BYPASS2, PLL bypass// 2) select the crystal value and oscillator source  
  SYSCTL_RCC_R = (SYSCTL_RCC_R &~XTAL_VAL_CLR)   // clear XTAL field, bits 10-6                 
    + XTAL_16MHZ;                       // 10101, configure for 16 MHz crystal  
  SYSCTL_RCC2_R &= ~XTAL_EXT;         // configure for main oscillator source// 3) activate PLL by clearing PWRDN  
  SYSCTL_RCC2_R &= ~START_PLL;         // 4) set the desired system divider  
  SYSCTL_RCC2_R |= DIV_400;          // use 400 MHz PLL  
  SYSCTL_RCC2_R = (SYSCTL_RCC2_R&~ SYS_CLK_CLR)  // clear system clock divider                  
    + (clkSpeed);                          // configure for 80 MHz clock// 5) wait for the PLL to lock by polling PLLLRIS
  while((SYSCTL_RIS_R & PLLRIS)==0){};  // wait for PLLRIS bit// 6) enable use of PLL by clearing BYPASS  
  SYSCTL_RCC2_R &= ~BYPASSPLL;
}

*/
void TIMER_ChangeSpeed(uint32_t clkSpeed){

SYSCTL_RCC2_R = (SYSCTL_RCC2_R&~ SYS_CLK_CLR)  // clear system clock divider                  
    + (clkSpeed); 

if (clkSpeed == SYS_CLK_4MHZ)
    GPTMTAILR = 4000000;        //Set Timer to 16Mticks
  else if (clkSpeed == SYS_CLK_80MHZ)
    GPTMTAILR = 80000000;  
  else if (clkSpeed == SYS_CLK_20MHZ)
    GPTMTAILR = 20000000; 


}
/*
void TIMER_Delay(uint32_t milliseconds){
  
  //Clock speed is set to 20MHz, 1 tick is 1/25Mhz
    
  uint32_t setTickDelay = (milliseconds * (16000000) ) / 1000;
  uint32_t delayCounter = 0;  
  setTickDelay = 100000u;
  while(delayCounter < setTickDelay){
    delayCounter++;
  }
}

 */ 
  