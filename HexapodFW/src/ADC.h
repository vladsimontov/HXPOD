#if !defined(ADC_H) 
#define ADC_H

#define ADC0_BASE       (0x40038000)


#define RCGCADC      (*((volatile uint32_t *)  0x400FE638)) 
  #define ENABLE_ADC0     (0x01)

#define RCGC0ADC      (*((volatile uint32_t *)  0x400FE100)) 
  #define EN_ADC0     (0x01 << 16)

#define ADCIM        (*((volatile uint32_t *) (ADC0_BASE + 0x008)))
  #define MASK3           (0x01 << 3)        //pg827 The raw interrupt signal from Sample Sequencer 3 (ADCRIS register INR0 bit



#define ADCEMUX       (*((volatile uint32_t *) (ADC0_BASE + 0x014)))
  #define TIMER_EVNT      (0x5 << 12)   //EM3 bit of ADCEMUX pg.837


#define ADCACTSS      (*((volatile uint32_t *) (ADC0_BASE + 0x000))) //pg 821 ADC Active Sample Sequencer (ADCACTSS), offset 0x000 This register controls the activation of the sample sequencers. Each sample sequencer can be enabled or disabled independently.
  #define BUSY            (0x01 << 16)  //for ADCACTSS, pg 821
  #define ASEN3           (0x01 << 3)   //Enable Sequencer 3 for ADCACTSS, pg 821
  #define DISABLE         (0x00)


#define ADCSSCTL3     (*((volatile uint32_t *) (ADC0_BASE + 0x0A4)))
  #define TS0             (0x01 << 3)   //1st Sample Temp Sensor Select pg 876
  #define IE0             (0x01 << 2)   //Sample Interrupt enable for ADCSSCTL3 pg. 876
  #define END0            (0x01 <<1)    //End of Sequence
  #define D0              (0x01)        //Sample Differential Input Select

#define ADCRIS         (*((volatile uint32_t *) (ADC0_BASE + 0x004))) //pg 823
  #define INR3            (0x01 << 3)

#define ADCDCCTL0      (*((volatile uint32_t *) (ADC0_BASE + 0xE00))) //pg 823
  #define CIE     (0x01<<4)

#define ADCSSPRI        (*((volatile uint32_t *) (ADC0_BASE + 0x020)))
#define PRI_SS3 (0x03 << 12) //Default priority is
#define ADC0SSMUX (*((volatile uint32_t *) (ADC0_BASE + 0x040)))

#define ADC0PSSI (*((volatile uint32_t *) (ADC0_BASE + 0x028)))
#define ADCRIS  (*((volatile uint32_t *) (ADC0_BASE + 0x004)))
#define ADCISC  (*((volatile uint32_t *) (ADC0_BASE + 0x00C)))
  #define IN3 (0x01 << 3)
#define SSFIFO3  (*((volatile uint32_t *) (ADC0_BASE + 0x0A8)))

void ADC_Init(void);
void ADC0_SequencerInit(void);
void ADC0_ModuleInit(void);

#define ADC0_INT      (0x01 << 17) 
#endif
