/*
Module for controlling the GPIO functions of the TIVA. 

*/
#include <tm4c123gh6pm.h>
#include <stdint.h>
#include <stdio.h>
#include "GPIO.h"
#include "Timer.h"


int SW1_PRESSED;
int SW2_PRESSED;
int STATECHANGED;
void PortF_Handler(void);

void PortF_Handler(void){
  if((~PORTF_DATA & SW1) == SW1){
    TIMER_ChangeSpeed(SYS_CLK_4MHZ);
  }
  else if ((~PORTF_DATA & SW2) == SW2){
    TIMER_ChangeSpeed(SYS_CLK_80MHZ);
  }
  GPIO_PORTF_ICR_R |= 0x11;
}
void GPIO_configInt(void){
GPIO_PORTF_IS_R &= ~0x11; /* make bit 4, 0 edge sensitive */
GPIO_PORTF_IBE_R &= ~0x11; /* trigger is controlled by IEV */
GPIO_PORTF_IEV_R= ~0x11; /* falling edge trigger */
GPIO_PORTF_ICR_R |= 0x11; /* clear any prior interrupt */
GPIO_PORTF_IM_R |= 0x11; /* unmask interrupt */
NVIC_PRI17_R = (3 << 21); 

ENABLEINT |= GPIOF_INT; //Same as: NVIC_EN0_R |= 0x40000000; /* enable IRQ30 */
}
void GPIO_configPortE(void){

RCGC2 |= PORTE;
GPIO_E_AFSEL |= 0x8;
GPIO_E_DEN &= ~0x8; /* disable digital function */
GPIO_E_AMSEL |= 0x8; /* enable analog function */


}
void GPIO_configPortF(void){
/*Function for configuring the GPIO pins on Port F.
Pins 1,2,3 will be set up for driving LEDs while Pins 0 and 4 will
be set as inputs to accept User Input via Switch.
*/

  //RCGC2 = RCGCGPIO;     // Set Port F to turn on its clocks 
  RCGC2 |= PORTF;
  //Set all LED pins to be outputs
  PORTF_DIR = ALL_LEDS; //Output = 1
 
  //Set all SW pins to be inputs (default is input)
  PORTF_DIR &= (~ALL_SW); //Input = 0
  
  PORTF_LOCK = UNLOCK_CODE; //Unlock Port F to modify registers
  PORTF_COMMIT = ALL_SW;    //Allow only pins with switches to be modified
  PORTF_PULLUP = ALL_SW;    //Add weak pull up resistor to switch pins
  
  PORTF_EN = (ALL_LEDS | ALL_SW); //Enable all LED and Switch pins
  
  //Initialize All LEDs to be off
  PORTF_DATA = ~ALL_LEDS;
  
  GPIO_configInt();
  
  return;
}

void GPIO_LED_ON(uint8_t color){
/*Function to drive an LED on
Pass in Pin location assigned to LED color to turn that light on 
*/

  PORTF_DATA = color;
}

void GPIO_LED_OFF(void){
 /*Turn off all LEDS on Port F*/
  PORTF_DATA = 0x00000000;
}