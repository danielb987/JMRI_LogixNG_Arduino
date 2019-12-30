#include <LocoNet.h>
#include "loconet_sv.h"

#define  TX_PIN   6

LocoNetVirtualSystemVariableClass virtual_sv;
LocoNetSystemVariableClass sv;
SV_STATUS svStatus = SV_OK;
boolean deferredProcessingNeeded = false;

static LnBuf LnTxBuffer ;
static lnMsg *LnPacket;
// static char buffer[30];

int card_addr = 200;
int sv_addr = 100;
int sv_size = 16;


void receiveStringCallback(int sv_addr, char *str) {
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

  // ManufacturerId: 13 = NMRA DIY DCC ManufacturerId
  // DeveloperId: 17 = Daniel Bergqvist
  // Product ID: 1
  // SwVersion: 1
  // Note that sv.init() and virtual_sv.init() needs the same ManufacturerId and DeveloperID !
  sv.init(13, 17, 1, 1);
  virtual_sv.init(13, 17);

  virtual_sv.registerSV_WriteString(&receiveStringCallback, 20, 50);
  
  sv.writeSVStorage(SV_ADDR_NODE_ID_H, 1 );
  sv.writeSVStorage(SV_ADDR_NODE_ID_L, 0);
  
  sv.writeSVStorage(SV_ADDR_SERIAL_NUMBER_H, 0x56);
  sv.writeSVStorage(SV_ADDR_SERIAL_NUMBER_L, 0x78);

  // Initialize a LocoNet packet buffer to buffer bytes from the PC 
  initLnBuf(&LnTxBuffer) ;

//  register_sv_string(receive_string_callback, sv_addr, sv_size);
}

void loop() {
  // Check for any received LocoNet packets
  LnPacket = LocoNet.receive() ;
  if( LnPacket )
  {
/*
    // Get the length of the received packet
    uint8_t Length = getLnMsgSize( LnPacket ) ;
    // Send the received packet out byte by byte to the PC
    for( uint8_t Index = 0; Index < Length; Index++ ) {
      ltoa(LnPacket->data[Index],buffer,10);
      Serial.write(buffer);
      Serial.write(", ");
    }
    Serial.write("\n");
*/

    
    int virtualSvStatus = virtual_sv.processMessage(LnPacket);
//    int virtualSvStatus = VIRTUAL_SV_NOT_HANDLED;
//    virtualSvStatus = VIRTUAL_SV_NOT_HANDLED;
//    Serial.print("Virtual SV processMessage - Status: ");
//    Serial.println(virtualSvStatus);

    if (virtualSvStatus == VIRTUAL_SV_NOT_HANDLED) {
      svStatus = sv.processMessage(LnPacket);
//      Serial.print("LNSV processMessage - Status: ");
//      Serial.println(svStatus);
    
      deferredProcessingNeeded = (svStatus == SV_DEFERRED_PROCESSING_NEEDED);
    }
  }
  
  if(deferredProcessingNeeded) {
    deferredProcessingNeeded = (sv.doDeferredProcessing() != SV_OK);
  }
}


void notifySVChanged(uint16_t Offset){
  Serial.print("LNSV SV: ");
  Serial.print(Offset);
  Serial.print(" Changed to: ");
  Serial.println(sv.readSVStorage(Offset));
}
