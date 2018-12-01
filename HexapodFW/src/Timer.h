#if !defined(TIMER_H) 
#define TIMER_H

#define BASE_TIMER_A    (0x40030000)
#define RCGCTIMER       (*((volatile uint32_t *)0x400FE604)) /*Enable Timer pg.338*/
#define SYSCTL_RCGC2_R  (*((volatile uint32_t *)0x400FE108))

#define GPTMCFG         (*((volatile uint32_t *) (BASE_TIMER_A + 0x000))) /*Configures type of Timer, 16 or 32 bit pg.727, no offset*/
#define GPTMTAMR        (*((volatile uint32_t *) (BASE_TIMER_A + 0x004)))
#define GPTMCTL         (*((volatile uint32_t *) (BASE_TIMER_A + 0x00C))) /*Fine Tune Timer, pg. 737*/
#define GPTMIMR         (*((volatile uint32_t *) (BASE_TIMER_A + 0x018))) /*GPTM Interrupt Mask, pg. 745*/
#define GPTMRIS         (*((volatile uint32_t *) (BASE_TIMER_A + 0x01C))) /*Raw interupt status. p.748*/
#define GPTMICR         (*((volatile uint32_t *) (BASE_TIMER_A + 0x024))) /*Interupt Clear Register p.754*/
#define GPTMTAILR       (*((volatile uint32_t *) (BASE_TIMER_A + 0x028))) /*Loads Timer Interval, default 0xFFFF.FFFF pg.756*/
#define GPTMTBILR       (*((volatile uint32_t *) (BASE_TIMER_A + 0x02C)))
 
#define TIMER0          (0x00000001)  /* Enables Timer 0(R0) of RCGCTIMER*/
#define GPTMCTL_ENABLE  (0x00000001)  /*Enables GPTMCTL, pg.740*/
#define SET32BIT        (0x00000001)  /*Sets Timer to be 32 Bit p.728*/
#define PERIODIC_TIMER  (0x00000002)  /*Enables Periodic Timer Mode, p.732*/
#define TIMERVALUE      (0x00F42400)  /*16,000,000 Timer Ticks*/
#define TIMERENABLE     (0x00000001)  /*Enable Timer to start counting p.740*/
#define TATORIS         (0x00000001)  /*Timer has run out*/
#define CLEARINT        (0x00000001)  /*Clear Interupt Flag*/
#define TATOIM          (0x00000001)  /*GPTM Timer A Time-Out Interrupt Mask p.747*/
#define TAOTE           (0x01 << 5)   /*GPTM Timer A Output Trigger Enable pg. 739*/

#define USERCC2 0x80000000
#define BYPASSPLL 0x00000800
#define XTAL_16MHZ 0x00000540
#define XTAL_EXT 0x00000070
#define DIV_400 0x40000000
#define START_PLL 0x00002000
#define XTAL_VAL_CLR 0x000007C0
#define SYS_CLK_CLR 0x1FC00000
#define SYS_CLK_80MHZ (4<<22)
#define SYS_CLK_4MHZ  (99<<22)
#define SYS_CLK_20MHZ  (20<<22)
#define PLLRIS 0x00000040

#define ENABLEINT       (*((volatile uint32_t *)0xE000E100)) /*Set Enable EN0, pg. 142*/
#define DISABLEINT      (*((volatile uint32_t *)0xE000E180)) /*Set Disable EN0, pg. 144*/

//#define TIMER_INT       (0x00080000) 
//#define GPIOF_INT       (0x40000000) /*30th Interupt Location*/
#define TIMERA_INT      (0x01 << 19)   /*19th Interupt Location*/


void setUp(void);
uint32_t millis( void );
void TIMER_InitTimerA(uint32_t systemSpeed);
void TIMER_Wait(uint32_t milliseconds);
void TIMER_ChangeSpeed(uint32_t clockSpeed);
void TIMER_InitSystemClock(uint32_t clkSpeed);
void enableTimerInterrupt(void);
void TIMER_Delay(uint32_t milliseconds);
#endif