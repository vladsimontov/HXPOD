/*
Module to Control ADC.
*/

#include <stdint.h>
#include <stdio.h>
#include "ADC.h"
#include "Timer.h"
#include "GPIO.h"
#include <tm4c123gh6pm.h>
#include <stdint.h>
#include <stdio.h>

void ADC0_Handler( void ){
  volatile int result = SSFIFO3;
  float vref= (((float)result) *3.3)/4095.0;
  float temp = ((2.7-vref)*75.0) - 55.0;
  
  if (temp > 0 && temp < 17)    
    GPIO_LED_ON(RED_LED);
  else if (temp > 17 && temp < 19)  
    GPIO_LED_ON(BLUE_LED);
  else if (temp > 19 && temp < 21)   
    GPIO_LED_ON(VIOLET_LED);
  else if (temp > 21 && temp < 23)   
    GPIO_LED_ON(GREEN_LED);
  else if (temp > 23 && temp < 25)   
    GPIO_LED_ON(YELLOW_LED);
  else if (temp > 25 && temp < 27)   
    GPIO_LED_ON(LBLUE_LED);
  else if (temp > 27)               
    GPIO_LED_ON(WHITE_LED);
  else GPIO_LED_OFF();
  
  ADCISC |= IN3; //Clear the interrupt

}

void ADC0_ModuleInit(void){
  RCGC0ADC|=EN_ADC0;
  GPIO_configPortE();
  ADCSSPRI = (ADCSSPRI & ~PRI_SS3); //Set priority of Sequencer 3 to 0 (highest)
  
  ADCSSPRI |=(0x3); //Set Sequencer 0 priority from 0 to 3
}

void ADC0_SequencerInit(void){
  ADCACTSS = (ADCACTSS & ~0xF); //Disable all sequencers
  ADCEMUX |= TIMER_EVNT;
   ADCSSCTL3 |= (TS0|IE0|END0);
   ADCIM   |= MASK3;
   ADCACTSS |= ASEN3;
   //ADC0PSSI |= (0x01 << 3); 
}
void ADC_Init(void){
  ADC0_ModuleInit();
  ADC0_SequencerInit();
  ENABLEINT |= ADC0_INT;
}