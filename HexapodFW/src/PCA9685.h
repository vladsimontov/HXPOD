#if !defined(PCA9685) 

#include <stdbool.h>

#define PCA9685

#define ONE_MSEC 0.001
#define MAX_ROTATION 180 
#define PERIOD ((1.0/60.0))

#define PCA9685_ADDR (0x40) 
#define PCA9685_READ (0x01)
#define PCA9685_WRITE (0x00)


#define PCA_9685_ADDR (0x40)
#define MODE1 (0x00)
#define MODE2 (0x01)

#define PRESCALE (0xfe)
#define SLEEP (0x10) /*on MODE1 register*/
#define RESTART (0x80)
#define MIN_HZ 24u 
#define MAX_HZ 1526u
#define CLKRATE 25000000 /*internal clock rate of the PCA9685*/



#define LED4_ON_L  (0x16)
#define LED4_ON_H  (0x17)
#define LED4_OFF_L (0x18)
#define LED4_OFF_H (0x19)

#define ALL_LED_OFF_H (0xFD)
#define ALL_LED_OFF_L (0xFC)

typedef enum pca9685_status
/*! -- */
{
/*@{*/
  PCA_9685_OK,    //!<PCA
  PCA_9685_NOT_SET,      //!<I2C acknowledged command
  PCA_9685_UNRESPONSIVE,   //!<I2C Did not acknowledge command
  PCA_9685_TIMEOUT,  //!<I2C bus timed out
  PCA_9685_OTHER,  //!<I2C bus timed out
  PCA_9685_UNKNOWN   //!<Default Status
/*@}*/
} pca9685_status_t;

pca9685_status_t PCA9685_Wake(void);
pca9685_status_t PCA9685_Sleep(void);
pca9685_status_t PCA9685_UpdatePWMFrequency(uint16_t newFrequency);
pca9685_status_t PCA9685_Restart(void);
pca9685_status_t PCA9685_SetPWM_Ch4(uint8_t dutyCycle);
void PCA9685_Init(void);

void PCA9685_SetDutyCycle(float dutyCycle);
void PCA9685_SetDegreeCycle(float degree);

void PCA9685_convertDutyCycleToCounts(float dutyCycle, uint16_t * highCount, uint16_t * lowCount);
void PCA9685_setServo(float degree, uint8_t leg);
void PCA9685_SetLeg(float dutyCycle, uint8_t legNum);
#endif