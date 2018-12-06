#ifndef I2C_H
#define I2C_H

#define RCGCI2C         (*((volatile uint32_t *)0x400FE620)) /*Register 65: Inter-Integrated Circuit Run Mode Clock Gating Control (RCGCI2C), offset 0x620 p.348*/
#define R0            (0x01)  /*I2C Module 0 Run Mode Clock Gating Control*/ 
  //Gate 0 uses PB2/PB3 for SCL/SDA

#define RCGCGPIO_B         (*((volatile uint32_t *)0x400FE608)) /*Register 65: Inter-Integrated Circuit Run Mode Clock Gating Control (RCGCI2C), offset 0x620 p.348*/
#define PORTB          (0x02)

#define PORTB_BASE      (0x40005000)
#define GPIO_B_AFSEL  (*((volatile uint32_t *) (PORTB_BASE + 0x420))) 
#define GPIO_B_DEN    (*((volatile uint32_t *) (PORTB_BASE + 0x51C))) 
#define GPIO_B_PDR    (*((volatile uint32_t *) (PORTB_BASE + 0x514))) 
#define GPIO_B_PUR    (*((volatile uint32_t *) (PORTB_BASE + 0x510))) 
#define GPIO_B_PCTL   (*((volatile uint32_t *) (PORTB_BASE + 0x52C))) 
#define GPIO_B_CR     (*((volatile uint32_t *) (PORTB_BASE + 0x524))) 

#define PIN2    (0x04)
#define PIN3    (0x08)
#define PMC2    (8)
#define PMC3    (12)

#define I2C_B0_BASE (0x40020000)
#define I2CMCR  (*((volatile uint32_t *) (I2C_B0_BASE + 0x020))) 
#define I2CMTPR (*((volatile uint32_t *) (I2C_B0_BASE + 0x00C))) 
  #define MFE (0x01 << 4) //Master mode enable
  #define CLK_100KBPS (0x00000009)

#define I2CMDR  (*((volatile uint32_t *) (I2C_B0_BASE + 0x008))) 
#define I2CMCS  (*((volatile uint32_t *) (I2C_B0_BASE + 0x004))) 
  #define BUSY   (0x01)
  #define ERROR  (0x02)
  #define ADRACK (0x04)
  #define DATACK (0x08)
  #define IDLE (0x10)
  #define BUSBSY (0x20)
  #define CLKTO (0x80)

#define GEN_START (0x02)
#define GEN_RUN   (0x01)
#define GEN_STOP (0x04)
#define GEN_ACK (0x08)
#define PIN7 (0x80)
#define PIN6 (0x40)

#define ERROR (0x02)
#define READ (0x01)
#define WRITE (0x00)
typedef enum i2c_status
/*! -- */
{
  /*@{*/
  i2c_OK,               //!<I2C Port Ready for Use
  i2c_ERROR,            //!<I2C acknowledged command
  i2c_NO_ACK,           //!<I2C Did not acknowledge command
  i2c_NO_ADDR_ACK,      //!<I2C Did not acknowledge address command
  i2c_TIMEOUT,          //!<I2C bus timed out
  i2c_CLK_TO,           //!<CLK timed out
  i2c_WRITE_ERROR,      //!<I2C bus timed out
  i2c_UNKNOWN           //!<Default Status
  /*@}*/
} i2c_status_t;

void I2C_InitPort1(void);
i2c_status_t I2C_WriteByte(uint8_t address, uint8_t data);
i2c_status_t I2C_Read(uint8_t address,uint8_t controlRegister, uint8_t * data);
i2c_status_t I2C_WriteBytes(uint8_t address,uint8_t controlRegister, uint8_t data);
#endif