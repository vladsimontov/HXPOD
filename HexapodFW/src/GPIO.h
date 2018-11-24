#if !defined(GPIO_H)
#define GPIO_H

#define TRUE            (1)                                             /*Define bool logic for True*/
#define FALSE           (0)                                             /*Define bool logic for False*/

#define PORTF_BASE      (0x40025000)                                    /*Port F Base Address*/
#define PORTE_BASE      (0x40024000)                                    /*Port E Base Address*/

#define RCGCGPIO        (0x00000020)                                    /*Port F Clock Gating Control*/
#define UNLOCK_CODE     (0x4C4F434B)                                    /*Unlock code found on p.684  */

#define PORTF           (0x01<<5)
#define PORTE           (0x01<<4)
#define RCGC2           (*((volatile uint32_t *)0x400FE108))            /*Port F Register Location*/
#define PORTF_EN        (*((volatile uint32_t *)0x4002551C))            /*Port F Register Location to Enable Pins*/
#define PORTF_DIR       (*((volatile uint32_t *)0x40025400))            /*Port F Register Location for define Input or Output of Pin*/
#define PORTF_DATA      (*((volatile uint32_t *)0x400253FC))            /*Port F Register to Drive Pins high/low*/
#define PORTF_LOCK      (*((volatile uint32_t *)0x40025520))            /*Port F Register Lock Area p.684  */
#define CLOCK_REG       (*((volatile uint32_t *)0x400FE060))
#define CLOCK_SRC       (0x540)
#define PORTF_PULLUP    (*((volatile uint32_t *) (PORTF_BASE + 0x510))) /*Port F Pull Up Resistor p.659*/
#define PORTF_COMMIT    (*((volatile uint32_t *) (PORTF_BASE + 0x524))) /*Port F Commit (OCR) Register p.685*/


#define GPIO_E_AFSEL (*((volatile uint32_t *) (PORTE_BASE + 0x420))) 

#define GPIO_E_DEN   (*((volatile uint32_t *) (PORTE_BASE + 0x51C)))
#define GPIO_E_AMSEL (*((volatile uint32_t *) (PORTE_BASE + 0x528))) 


#define SW2             (0x01)                                          /*Port F, Pin 0 [PF0]*/
#define RED_LED         (0x02)                                          /*Port F, Pin 1 [PF1]*/
#define BLUE_LED        (0x04)                                          /*Port F, Pin 2 [PF2]*/
#define GREEN_LED       (0x08)                                          /*Port F, Pin 3 [PF3]*/
#define VIOLET_LED      (RED_LED|BLUE_LED)
#define YELLOW_LED      (RED_LED|GREEN_LED)
#define LBLUE_LED       (BLUE_LED|GREEN_LED)
#define WHITE_LED       (RED_LED | BLUE_LED | GREEN_LED)
#define SW1             (0x10)                                          /*Port F, Pin 4 [PF4]*/
#define LEDONTIME       (1000000)                                       /*Keep LEDs on for 1,000,000 iterations of a for loop*/

#define ALL_LEDS        (RED_LED | BLUE_LED | GREEN_LED)                /*Control All LEDS at once*/
#define ALL_SW          (SW1 | SW2) 

#define GPIOF_INT       (0x01 << 30)                                    /*30th Interupt Location*/

extern int STATECHANGED;
extern int SW1_PRESSED;
extern int SW2_PRESSED;

void GPIO_configInt(void);
void GPIO_configPortF(void);
void GPIO_configPortE(void);
void GPIO_LED_ON(uint8_t color);
void GPIO_LED_OFF(void);
#endif