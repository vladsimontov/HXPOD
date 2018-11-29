#include "Bluetooth.h"
#include "UART.h"

uint8_t GOBLE_ADDRESS = 0x11;
uint8_t headerCHKSUM = 0x10;

#define MAXPACKETDATA 48
uint8_t packetData[MAXPACKETDATA];
uint8_t packetLength = 0;
uint8_t packetLengthReceived = 0;
uint32_t flushcount = 0;
uint32_t packetErrorCount = 0; 

void BlueTooth_Init( void ) {
   UART_InitPort1();
}

void checkBlueTooth(gaitCommand_t * cmd){
  static gaitCommand_t lastCmd = BOT_STOP;
  if (BlueTooth_PacketHandler() == P_NEW_DATA_AVAILABLE){
    //do some logic to set the new command;
  }
  *cmd = BOT_STAND;
}

packetState_t BlueTooth_PacketHandler( void ) {

  static packetState_t packetState = P_WAITING_FOR_HEADER_55;
  static uint8_t checksum = 0;
  uint8_t c;
  
  while (UART_Rx_available() > 0) {
    UART_ReadByte(&c);
    
    switch (packetState) {
      
      case P_WAITING_FOR_HEADER_55:
        flushcount = 0;
        while ((UART_Rx_available() > 0) && (c != 0x55)) {
          //Discard everything up to the start of a new header, right here
          UART_ReadByte(&c);
          flushcount++;
        }
        if (c == 0x55){
        //Got first byte of header, go to next state
        packetState = P_WAITING_FOR_HEADER_AA;
        }
        break;
        
      case P_WAITING_FOR_HEADER_AA:
        if (c == 0xAA){
          //Got second byte of header, go to address
          packetState = P_WAITING_FOR_ADDRESS;
        }
        else if (c == 0x55) break; //Unlikely event that we have a doubleshot, stay in this state
        else{
          packetState = P_WAITING_FOR_HEADER_55; //something when wrong, start over
        }
        break;
        
      case P_WAITING_FOR_ADDRESS:
        if (c == GOBLE_ADDRESS) {
          packetState = P_WAITING_FOR_LENGTH;
        } else if (c == 0x55) {
          packetState = P_WAITING_FOR_HEADER_AA; //check for the first byte again to avoid skipping a whole packet
        } else {
          packetErrorCount++;
          packetState = P_WAITING_FOR_HEADER_55; // go back to looking for a 0x55 again
          return P_PACKET_ERROR;
        }
        break;
        
      case P_WAITING_FOR_LENGTH:
        if (c < 7) {  
          checksum = headerCHKSUM + c;       //start generating checksum from known header and variable length
          packetLength = c + 6;              //data length is the number of pressed buttons plus 4 analog, 2 digital (not incl. CHKSUM)
          packetLengthReceived = 0;
          packetData[packetLengthReceived++] = packetLength;
          packetState = P_READING_DATA;
        } else {
          packetErrorCount++;
          packetState = P_WAITING_FOR_HEADER_55; // go back to looking for a 0x55 again
          return P_PACKET_ERROR;
        }
        break;
        
      case P_READING_DATA:
        if (packetLengthReceived >= MAXPACKETDATA) {
          // if we're getting garbage in, abandon the packet and go back to waiting for headers
          packetState = P_WAITING_FOR_HEADER_55;
          packetLengthReceived = 0;
          return P_PACKET_ERROR;
        }
        packetData[packetLengthReceived++] = c;
        checksum += c;
        if (packetLengthReceived == packetLength) {
          //last byte is the checksum, not included in the packet length calculation
          packetState = P_WAITING_FOR_CHECKSUM;
        }
        break;

      case P_WAITING_FOR_CHECKSUM:        
        if (checksum != c) {
          //if the packet checksum fails, give up on it
          packetErrorCount++;
          packetState = P_WAITING_FOR_HEADER_55;
          return P_PACKET_ERROR;
        } else {          
//          LastValidReceiveTime = millis();  // set the time we received a valid packet.  We can bring this back! --LI
          decodePacket();
          packetState = P_WAITING_FOR_HEADER_55;
          return P_NEW_DATA_AVAILABLE; // new data arrived!
        }
        break;
    }
  }

  return packetState; // no new data arrived
}

uint8_t decodePacket( void ) {
  uint8_t lastCmd = 0;
  //uint8_t i = 0;
  uint8_t buttonsPressed = packetData[0];
  static uint8_t lastChangeFlag = 0x00;
  uint8_t thisChangeFlag = 0;
  
  while (buttonsPressed) { //consume the buttons from greatest to least
    switch(packetData[1+buttonsPressed--]) {
      case 0x06:
        break;
      case 0x05:
        break;
      case 0x04:
        lastCmd = 'r';
      //depending on movement state, rotate or walk sideways
        break;
      case 0x03:
        lastCmd = 'b';//same as 4
        break;
      case 0x02:
        lastCmd = '1';//same as 3/4 but walk backwards
        break;
      case 0x01:
        lastCmd = 'f';//same as prev but walk forwards
        break;
      default:
        lastCmd = 's';
        break;
    }
  }
  lastChangeFlag = thisChangeFlag;
  
  if (packetData[1]) {
    //i = packetData[0]+2;
    //add in use for joystick here
  }  
  return lastCmd;
}

