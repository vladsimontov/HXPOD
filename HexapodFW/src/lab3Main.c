//#include <tm4c123gh6pm.h>
#include <stdint.h>

//TIMER OFFSETS
#define GPTM_CFG        0x00000000
#define GPTM_CTL        0x0000000C
#define GPTM_TAMR       0x00000004
#define GPTM_TAILR      0x00000028
#define GPTM_TBILR      0x0000002C
#define GPTM_RIS        0x0000001C
#define GPTM_ICR        0x00000024
#define GPTM_IMR        0x00000018

#define TIMER0_BASE     0x40030000
#define TIMER0(OFFSET)  (*((volatile uint32_t *)(TIMER0_BASE | OFFSET)))


//PORT REGISTER OFFSETS
#define GPIO_DIR        0x0400
#define GPIO_DEN        0x051C
#define GPIO_LOCK       0x0520
#define GPIO_PULLUP     0x0510
#define GPIO_CR         0x0524
#define GPIO_DATA       0x01FC
#define GPIO_IM         0x0410
#define GPIO_IBE        0x0408
#define GPIO_IS         0x0404
#define GPIO_ICR        0x041C

#define GPIO_PORTF_BASE    0x40025000 //0x4005D000 AHP
#define PORTF(OFFSET)   (*((volatile uint32_t *)(GPIO_PORTF_BASE | OFFSET)))

//Other registers
#define SYSCTL_RCGC2_R     (*((volatile uint32_t *)0x400FE108))
#define RCGCTIMER_R        (*((volatile uint32_t *)0x400FE604))   
#define EN0_SET_INTS       (*((volatile uint32_t *)0xE000E100))

//GPIO PINS
#define RED          0x02
#define BLUE         0x04
#define GREEN        0x08
#define SW1          0x10
#define SW2          0x01
#define ALL_PINS     ( RED | GREEN | BLUE | SW1 | SW2 )

#define UNLOCK_PORTF       (PORTF(GPIO_LOCK) = 0x4C4F434B)  
#define COMMIT_PORTF       (PORTF(GPIO_CR) = ALL_PINS)
#define DISABLE_PORTF      (PORTF(GPIO_DEN) = 0x00)
#define ENABLE_PORTF       (PORTF(GPIO_DEN) = ALL_PINS)
#define SET_OUTPUTS(C)     (PORTF(GPIO_DIR) = C)
#define ENABLE_PULLUPS(C)  (PORTF(GPIO_PULLUP) = C)
#define PORTF_WRITE(C)     (PORTF(GPIO_DATA) = C)
#define PORTF_READ(C)      (PORTF(GPIO_DATA) & C)

char BLINK_STATE = 0xFF; //LED is ON
char SW_STATE_1 = 0x00; //SW1 is ON
char SW_STATE_2 = 0x00; //SW2 is ON
char SWITCH_CHANGE = 0x00; //
char LED_COLOR = RED;

#define SYS_RCGC2       0x0108
#define SYS_RCGCTIMER   0x0604
#define PLL_RCC2        0x0070
#define PLL_RCC_R       0x0060
#define PLL_RIS         0x0050
#define RCGC_ADC        0x0638 //set to 1
#define RCGC_GPIO       0x0608
#define SYS_MISC        0x0058

#define SYSCTL_BASE    0x400FE000 
#define SYSCTL(OFFSET)   (*((volatile uint32_t *)(SYSCTL_BASE | OFFSET)))

unsigned int i =0;

void PLL_Init( unsigned int MHz ){
  SYSCTL(PLL_RCC2) =  0x80000000;  // USERCC2
  // 1) bypass PLL while initializing
  SYSCTL(PLL_RCC2) |=  0x00000800;  // BYPASS2, PLL bypass
  // 2) select the crystal value and oscillator source
  SYSCTL(PLL_RCC_R) &= ~0x000007C0;   // clear XTAL field, bits 10-6
  SYSCTL(PLL_RCC_R) += 0x00000540;   // 10101, configure for 16 MHz crystal
  SYSCTL(PLL_RCC2) &= ~0x00000070;  // configure for main oscillator source
  // 3) activate PLL by clearing PWRDN
  SYSCTL(PLL_RCC2) &= ~0x00002000;
  // 4) set the desired system divider
  SYSCTL(PLL_RCC2) |= 0x40000000;   // use 400 MHz PLL
  SYSCTL(PLL_RCC2) &= ~0x1FC00000;  // clear system clock divider
  SYSCTL(PLL_RCC2) += (MHz<<22);      // configure for 80 MHz clock
  // 5) wait for the PLL to lock by polling PLLLRIS
  
  for (i = 0; i > 1000000; i++);  
  while((SYSCTL(PLL_RIS)&0x00000040)==0){};  // wait for PLLRIS bit
  // 6) enable use of PLL by clearing BYPASS
  SYSCTL(PLL_RCC2) &= ~0x00000800;
  
}

void setup()
{
  
  //Configure Interrupt
  EN0_SET_INTS = ((0x01 << 19) | (0x01 << 30));
  
  //Configure Clock
  SYSCTL_RCGC2_R |= 0x20;

  SYSCTL(SYS_MISC) |= ~(0x01 << 6);
  SYSCTL(PLL_RCC2) =  0x80000000;  // USERCC2
  // 1) bypass PLL while initializing
  SYSCTL(PLL_RCC2) |=  0x00000800;  // BYPASS2, PLL bypass
  // 2) select the crystal value and oscillator source
  SYSCTL(PLL_RCC_R) &= ~0x000007C0;   // clear XTAL field, bits 10-6
  SYSCTL(PLL_RCC_R) += 0x00000540;   // 10101, configure for 16 MHz crystal
  SYSCTL(PLL_RCC2) &= ~0x00000070;  // configure for main oscillator source
  // 3) activate PLL by clearing PWRDN
  SYSCTL(PLL_RCC2) &= ~0x00002000;
  // 4) set the desired system divider
  SYSCTL(PLL_RCC2) = 0x40000000;   // use 400 MHz PLL
  SYSCTL(PLL_RCC2) &= ~0x1FC00000;  // clear system clock divider
  SYSCTL(PLL_RCC2) += (99<<22);      // configure for 80 MHz clock
  // 5) wait for the PLL to lock by polling PLLLRIS
  
  while((SYSCTL(PLL_RIS)&0x00000040)==0){};  // wait for PLLRIS bit
  
  // 6) enable use of PLL by clearing BYPASS
  SYSCTL(PLL_RCC2) &= ~0x00000800;
  
  //Configure Timer 0
  RCGCTIMER_R |= 0x01;              //select Timer 0 in gating control reg
  TIMER0(GPTM_CTL) &= ~0x01;        //disable timer in Timer 0 control reg 
  TIMER0(GPTM_CFG) = 0x00;          //set timer to 32bit mode, and not a RTC
  TIMER0(GPTM_TAMR) = 0x02;         //configure Timer_0_A to periodic mode
  TIMER0(GPTM_TAMR) &= ~0x10;       //Direction is set to down counter
  TIMER0(GPTM_TAILR) = 0x00F42400;  //16 LSBs of number to load to timer
  TIMER0(GPTM_IMR) = 0x00000001;    //enable time-out interrupt
  TIMER0(GPTM_CTL) |= 0x01u;        //re-enable timer in Timer 0 control reg 
  
  //Configure GPIO
  UNLOCK_PORTF;                     //Write-enable GPIO config registers
  COMMIT_PORTF;                     //
  DISABLE_PORTF; 
  SET_OUTPUTS( RED | GREEN | BLUE );
  ENABLE_PULLUPS(SW1 | SW2);
  PORTF(GPIO_IM) = (SW1 | SW2);
  PORTF(GPIO_IBE) = (SW1 | SW2);
  ENABLE_PORTF; 
}

void Timer_Handler( void ){
      TIMER0(GPTM_ICR) |= 0x01;
      BLINK_STATE = !BLINK_STATE;
}

void GPIOF_Handler( void ){
      PORTF(GPIO_ICR) |= 0x11;
      SW_STATE_1 = PORTF_READ(SW1);
      SW_STATE_2 = PORTF_READ(SW2);
      SWITCH_CHANGE = 0xFF;
}

int main()
{
  setup();
  
  while(1){   
    if (SWITCH_CHANGE){
      SWITCH_CHANGE= 0x00;
      if (SW_STATE_2 == SW2){ //if PF0 is pressed
        TIMER0(GPTM_IMR) |= 0x01; //disable timer interrupt
      } else {
        TIMER0(GPTM_IMR) &= ~0x01;
        BLINK_STATE = 0x01;
      }
      
      if (SW_STATE_1 == SW1){ //if PF4 is pressed
        LED_COLOR = RED;
      } else LED_COLOR = BLUE;
    }
    PORTF_WRITE(BLINK_STATE * LED_COLOR);
  }
  return 0;
}
