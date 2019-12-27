#include <LocoNet.h>
#include "loconet_sv.h"

#define  TX_PIN   6

static LnBuf LnTxBuffer ;
static lnMsg *LnPacket;
static char buffer[30];

int sv_addr = 100;
int sv_size = 16;


void receive_string_callback(int sv_addr, char *str) {
    Serial.write("Receive string: ");
    Serial.write(str);
    Serial.write(" at address ");
    Serial.write(sv_addr);
    Serial.write("\n");
}



void setup() {
  // Configure the serial port for 57600 baud
  Serial.begin(57600);
  
  // First initialize the LocoNet interface, specifying the TX Pin
  LocoNet.init(TX_PIN);
  
  // Initialize a LocoNet packet buffer to buffer bytes from the PC 
  initLnBuf(&LnTxBuffer) ;

  register_sv_string(receive_string_callback, sv_addr, sv_size);
}

void loop() {
  // Check for any received LocoNet packets
  LnPacket = LocoNet.receive() ;
  if( LnPacket )
  {
    // Get the length of the received packet
    uint8_t Length = getLnMsgSize( LnPacket ) ;

    // Send the received packet out byte by byte to the PC
    for( uint8_t Index = 0; Index < Length; Index++ ) {
      ltoa(LnPacket->data[Index],buffer,10);
      Serial.write(buffer);
      Serial.write(", ");
    }
    Serial.write("\n");
  }
}
