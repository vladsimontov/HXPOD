#ifndef UART_H 
#define UART_H

#include <stdint.h>
#include <stdio.h>

#define SYSCTL_BASE  0x400FE000
#define PORTC_BASE   0x40006000
#define UART0_BASE   0x4000C000

#define RCGC_UART       (*((volatile uint32_t *) (SYSCTL_BASE | 0x0618)))
#define RCGCGPIO        (*((volatile uint32_t *) (SYSCTL_BASE | 0x0608)))
#define GPIO_C_AFSEL    (*((volatile uint32_t *) (PORTC_BASE |  0x0420)))  
#define GPIO_C_PCTL     (*((volatile uint32_t *) (PORTC_BASE |  0x052C))) 
#define GPIO_C_DR8R     (*((volatile uint32_t *) (PORTC_BASE |  0x0508)))
#define GPIO_C_SLR      (*((volatile uint32_t *) (PORTC_BASE |  0x0518)))
#define GPIO_C_DEN      (*((volatile uint32_t *) (PORTC_BASE |  0x051C)))
#define GPIO_C_DIR      (*((volatile uint32_t *) (PORTC_BASE |  0x0400)))

#define UART1_DATA     (*((volatile uint32_t *) (0x4000D000))) 
#define UART_CTL(N)    (*((volatile uint32_t *) ((UART0_BASE + (0x1000 * N)) | 0x0030)))
#define UART_FR(N)     (*((volatile uint32_t *) ((UART0_BASE + (0x1000 * N)) | 0x0018)))
#define UART_IBRD(N)   (*((volatile uint32_t *) ((UART0_BASE + (0x1000 * N)) | 0x0024)))
#define UART_FBRD(N)   (*((volatile uint32_t *) ((UART0_BASE + (0x1000 * N)) | 0x0028)))
#define UART_LCRH(N)   (*((volatile uint32_t *) ((UART0_BASE + (0x1000 * N)) | 0x002C)))
#define UART_CC(N)     (*((volatile uint32_t *) ((UART0_BASE + (0x1000 * N)) | 0x0FC8)))
#define UART_ICR(N)    (*((volatile uint32_t *) ((UART0_BASE + (0x1000 * N)) | 0x0044)))

#define UART_ENABLE (0x01)
#define UART_8BIT_CFG (0x60)
#define UART_FIFO_EN (0x10)
#define UART_RxFIFO_EMPTY_FLAG (0x10)

typedef enum UART_status
/*! -- */
{
/*@{*/
  UART_STATUS_OK,    //!<Default "success" code
  UART_STATUS_ERROR,      //!<Default ërror: code (more to come?)
  UART_STATUS_TxFULL,
  UART_STATUS_RxEMPTY,
  UART_STATUS_UNKNOWN   //!<Default Status
/*@}*/
} UART_status_t;

void UART_InitPort1( void );
UART_status_t UART_ReadByte(uint8_t * data);
uint8_t UART_Rx_available( void );
#endif