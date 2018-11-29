#ifndef BLUETOOTH_H
#define BLUETOOTH_H

#include "UART.h"
#include "Gaits.h"

typedef enum packetStateType {
  P_WAITING_FOR_HEADER_55,
  P_WAITING_FOR_HEADER_AA,
  P_WAITING_FOR_ADDRESS,
  P_WAITING_FOR_LENGTH,
  P_READING_DATA,
  P_WAITING_FOR_CHECKSUM,
  P_NEW_DATA_AVAILABLE,
  P_PACKET_ERROR 
} packetState_t;

void BlueTooth_Init( void );
packetState_t BlueTooth_PacketHandler( void );
uint8_t decodePacket( void );
void checkBlueTooth(gaitCommand_t * cmd);


#endif