#ifndef PCA9685
#define PCA9685

#include <stdbool.h>

#define ONE_MSEC       (0.001)
#define MAX_ROTATION   (180)
#define PERIOD         (1.0/60.0)

#define PCA9685_ADDR   (0x40)   /*Address for PCA9685*/
#define PCA9685_READ   (0x01)   /*Read Operation*/
#define PCA9685_WRITE  (0x00)   /*Write Operation*/


#define PCA_9685_ADDR  (0x40)
#define MODE1          (0x00)
#define MODE2          (0x01)
#define EN_RST         (0x80)   /*Enable Restart operation, per Mode2*/
#define EN_ALLCALL     (0x01)   /*PCA9685 responds to LED All Call I2C-bus address. per Mode2 */
#define PRESCALE       (0xfe)
#define OCH_ACK        (0x04)   /*PCA9685 responds to LED All Call I2C-bus address. per Mode2 */
#define SLEEP          (0x10)   /*on MODE1 register*/
#define RESTART        (0x80)
#define MIN_HZ         (24u)
#define MAX_HZ         (1526u)
#define CLKRATE        (25000000) /*internal clock rate of the PCA9685*/



#define LED4_ON_L      (0x16)
#define LED4_ON_H      (0x17)
#define LED4_OFF_L     (0x18)
#define LED4_OFF_H     (0x19)

#define ALL_LED_OFF_H  (0xFD)
#define ALL_LED_OFF_L  (0xFC)

typedef enum pca9685_status
/*! -- */
{
/*@{*/
  PCA_9685_OK,           //!<PCA is ok
  PCA_9685_NOT_SET,      //!<PCA has not been configured
  PCA_9685_UNRESPONSIVE, //!<Comms attempts are unsuccesful
  PCA_9685_TIMEOUT,      //!<Timeout error condition
  PCA_9685_OTHER,        //!<Unknown error state
  PCA_9685_UNKNOWN       //!<default setting
/*@}*/
} pca9685_status_t;

pca9685_status_t PCA9685_Wake(void);
pca9685_status_t PCA9685_Sleep(void);
pca9685_status_t PCA9685_UpdatePWMFrequency(uint16_t newFrequency);
pca9685_status_t PCA9685_Restart(void);
pca9685_status_t PCA9685_SetPWM_Ch4(uint8_t dutyCycle);
pca9685_status_t PCA9685_Init(void);

void PCA9685_SetDutyCycle(float dutyCycle);
void PCA9685_SetDegreeCycle(float degree);
void PCA9685_convertDutyCycleToCounts(float dutyCycle, uint16_t * highCount, uint16_t * lowCount);
void PCA9685_setServo(uint8_t leg, float degree);
void PCA9685_SetLeg(float dutyCycle, uint8_t legNum);
#endif