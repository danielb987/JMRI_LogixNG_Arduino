/****************************************************************************
 * 	Copyright (C) 2009 to 2013 Alex Shepherd
 * 	Copyright (C) 2013 Damian Philipp
 *  Copyright (C) 2019 Daniel Bergqvist
 * 
 * 	Portions Copyright (C) Digitrax Inc.
 * 	Portions Copyright (C) Uhlenbrock Elektronik GmbH
 * 
 * 	This library is free software; you can redistribute it and/or
 * 	modify it under the terms of the GNU Lesser General Public
 * 	License as published by the Free Software Foundation; either
 * 	version 2.1 of the License, or (at your option) any later version.
 * 
 * 	This library is distributed in the hope that it will be useful,
 * 	but WITHOUT ANY WARRANTY; without even the implied warranty of
 * 	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * 	Lesser General Public License for more details.
 * 
 * 	You should have received a copy of the GNU Lesser General Public
 * 	License along with this library; if not, write to the Free Software
 * 	Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 * 
 *****************************************************************************
 * 
 * 	IMPORTANT:
 * 
 * 	Some of the message formats used in this code are Copyright Digitrax, Inc.
 * 	and are used with permission as part of the MRRwA (previously EmbeddedLocoNet) project.
 *  That permission does not extend to uses in other software products. If you wish
 * 	to use this code, algorithm or these message formats outside of
 * 	MRRwA, please contact Digitrax Inc, for specific permission.
 * 
 * 	Note: The sale any LocoNet device hardware (including bare PCB's) that
 * 	uses this or any other LocoNet software, requires testing and certification
 * 	by Digitrax Inc. and will be subject to a licensing agreement.
 * 
 * 	Please contact Digitrax Inc. for details.
 * 
 *****************************************************************************
 * 
 * 	IMPORTANT:
 * 
 * 	Some of the message formats used in this code are Copyright Uhlenbrock Elektronik GmbH
 * 	and are used with permission as part of the MRRwA (previously EmbeddedLocoNet) project.
 *  That permission does not extend to uses in other software products. If you wish
 * 	to use this code, algorithm or these message formats outside of
 * 	MRRwA, please contact Copyright Uhlenbrock Elektronik GmbH, for specific permission.
 * 
 *****************************************************************************
 * 	DESCRIPTION
 *  This module provides virtual SV registers that uses callback functions
 *  instead of reading or writing eeprom memory. It's used to send character
 *  strings and floating point numbers to and from the device.
 *
 *  This package relies on the mrrwa LocoNet library and a lot of code is
 *  fetched from that library.
 * 
 *****************************************************************************/


// Uncomment to enable SV Processing Debug Print statements
//#define DEBUG_SV


#include <stdlib.h>
#include <LocoNet.h>
// #include <ln_sw_uart.h>
// #include <ln_config.h>
// #include <utils.h>

#include <avr/eeprom.h>
#include <avr/wdt.h>

#include "loconet_sv.h"


// The maximum number of SV string items we can handle.


/*
typedef struct
{
	receive_string callback;
	int sv_addr;
	int size;
	char *buffer;
	int index;
} sv_string_struct;


sv_string_struct *sv_string_list[MAX_SV_ADDR_LIST];

int sv_string_list_size = 0;



int register_sv_string(receive_string callback, int sv_addr, int size) {

	if (sv_string_list_size+1 >= MAX_SV_ADDR_LIST)
	{
		// Too many sv_strings are already registered
		return 1;
	}

	sv_string_struct *sv_struct = (sv_string_struct*) malloc(sizeof(sv_string_struct));
	sv_struct->callback = callback;
	sv_struct->sv_addr = sv_addr;
	sv_struct->size = size;
	sv_struct->buffer = (char*) malloc(size+1);		// The asciiz 0 is not included in size
	sv_struct->index = 0;
	sv_string_list[++sv_string_list_size] = sv_struct;

	// Success
	return 0;
}


void handleLocoNetPackage(lnMsg *LnPacket) {

    // Get the length of the received packet
    uint8_t Length = getLnMsgSize( LnPacket ) ;

}
*/






typedef union
{
word                   w;
struct { byte lo,hi; } b;
} U16_t;

typedef union
{
struct
{
  U16_t unDestinationId;
  U16_t unMfgIdDevIdOrSvAddress;
  U16_t unproductId;
  U16_t unSerialNumber;
}    stDecoded;
byte abPlain[8];
} SV_Addr_t;



void LocoNetVirtualSystemVariableClass::init(uint8_t newMfgId, uint8_t newDevId)
{
  DeferredProcessingRequired = 0;
  DeferredSrcAddr = 0;
  
  mfgId = newMfgId;
  devId = newDevId;

  numVirtualSV = 0;
}


uint8_t LocoNetVirtualSystemVariableClass::readSVStorage(uint16_t Offset )
{
	uint8_t retValue;
	
  if( Offset == SV_ADDR_EEPROM_SIZE)
#if (E2END==0x0FF)	/* E2END is defined in processor include */
								return SV_EE_SZ_256;
#elif (E2END==0x1FF)
								return SV_EE_SZ_512;
#elif (E2END==0x3FF)
								return SV_EE_SZ_1024;
#elif (E2END==0x7FF)
								return SV_EE_SZ_2048;
#elif (E2END==0xFFF)
								return SV_EE_SZ_4096;
#else
								return 0xFF;
#endif
  if( Offset == SV_ADDR_SW_VERSION )
    retValue = swVersion ;
    
  else
  {
    Offset -= 2;    // Map SV Address to EEPROM Offset - Skip SV_ADDR_EEPROM_SIZE & SV_ADDR_SW_VERSION
    retValue = eeprom_read_byte((uint8_t*)Offset);
  }
	return retValue;
}


uint16_t LocoNetVirtualSystemVariableClass::readSVNodeId(void)
{
    return (readSVStorage(SV_ADDR_NODE_ID_H) << 8 ) | readSVStorage(SV_ADDR_NODE_ID_L);
}



int LocoNetVirtualSystemVariableClass::processMessage(lnMsg *LnPacket )
{
  SV_Addr_t unData ;
   
  if( ( LnPacket->sv.mesg_size != (byte) 0x10 ) ||
      ( LnPacket->sv.command != (byte) OPC_PEER_XFER ) ||
      ( LnPacket->sv.sv_type != (byte) 0x02 ) ||
      ( LnPacket->sv.sv_cmd & (byte) 0x40 ) ||
      ( ( LnPacket->sv.svx1 & (byte) 0xF0 ) != (byte) 0x10 ) ||
      ( ( LnPacket->sv.svx2 & (byte) 0xF0 ) != (byte) 0x10 ) )
    return SV_OK ;
 
  decodePeerData( &LnPacket->px, unData.abPlain ) ;

#ifdef DEBUG_SV
    Serial.print("LNSV Src: ");
    Serial.print(LnPacket->sv.src);
    Serial.print("  Dest: ");
    Serial.print(unData.stDecoded.unDestinationId.w);
    Serial.print("  CMD: ");
    Serial.println(LnPacket->sv.sv_cmd, HEX);
#endif
  if ((LnPacket->sv.sv_cmd != SV_DISCOVER) && 
      (LnPacket->sv.sv_cmd != SV_CHANGE_ADDRESS) && 
      (unData.stDecoded.unDestinationId.w != readSVNodeId()))
  {
#ifdef DEBUG_SV
    Serial.print("LNSV Dest Not Equal: ");
    Serial.println(readSVNodeId());
#endif
    return SV_OK;
  }

  switch( LnPacket->sv.sv_cmd )
  {
    case SV_WRITE_QUAD:
        // fall through intended!
    case SV_READ_QUAD:
#ifdef DEBUG_SV
        Serial.print("SV address: ");
        Serial.println(unData.stDecoded.unMfgIdDevIdOrSvAddress.w);
#endif
		if (handleSVReadWrite(unData.stDecoded.unMfgIdDevIdOrSvAddress.w, unData.abPlain[4], unData.abPlain[5], unData.abPlain[6], unData.abPlain[7]))
			return VIRTUAL_SV_NOT_HANDLED;

        break;

    default:
        return VIRTUAL_SV_NOT_HANDLED;
  }
    
  encodePeerData( &LnPacket->px, unData.abPlain ); // recycling the received packet
    
  LnPacket->sv.sv_cmd |= 0x40;    // flag the message as reply
  
  LN_STATUS lnStatus = LocoNet.send(LnPacket, LN_BACKOFF_INITIAL);
	
#ifdef DEBUG_SV
  Serial.print("LNSV Send Response - Status: ");
  Serial.println(lnStatus);   // report status value from send attempt
#endif

  if (lnStatus != LN_DONE) {
    // failed to send the SV reply message.  Send will NOT be re-tried.
    LocoNet.sendLongAck(44);  // indicate failure to send the reply
  }
    
  return SV_OK;
}




int LocoNetVirtualSystemVariableClass::handleSVReadWrite(uint16_t address, uint8_t &d1, uint8_t &d2, uint8_t &d3, uint8_t &d4)
{
  for (int i=0; i < numVirtualSV; i++)
  {
    if (virtual_SV_Struct[i].svAddr == address)
    {
      char *buffer = virtual_SV_Struct[i].buffer;
      int index = d4 & 0x7F;

      // String too long
      if (index+4 >= virtual_SV_Struct[i].bufSize) {
        Serial.print("String too long: ");
        Serial.print(index);
        Serial.print(". Buffer size: ");
        Serial.println(virtual_SV_Struct[i].bufSize);
        return 0;
      }

      switch (virtual_SV_Struct[i].type)
      {
        case READ_FLOAT:
          return 0;

        case WRITE_FLOAT:
          return 0;

        case READ_STRING:
          return 0;

        case WRITE_STRING:
          buffer[index] = d1;
          buffer[index + 1] = d2;
          buffer[index + 2] = d3;
          buffer[index + 3] = 0;		// End of string

          // If highest bit in index is set, the complete string is received.
		  // If the buffer isn't big enough for these three bytes and an additional
		  // four bytes, in total seven bytes, we cannot receive any more characters.
		  // In either case, call the callback function.
          if ((d4 >= 0x80) || (index+7 >= virtual_SV_Struct[i].bufSize)) {
            virtual_SV_Struct[i].callbackString(address, buffer);
          }

          return 0;

        default:
          Serial.print("Unknown type: ");
          Serial.println(virtual_SV_Struct[i].type);
      }
    }
  }

  return 1;
}




int LocoNetVirtualSystemVariableClass::registerSV_WriteString(receiveString callback, int svAddr, int size)
{
  if (numVirtualSV >= MAX_VIRTUAL_SV) {
    Serial.print("Too many virtual SVs: ");
    Serial.println(numVirtualSV);
    return TOO_MANY_VIRTUAL_SV_REG;
  }

  virtual_SV_Struct[numVirtualSV].callbackString = callback;
  virtual_SV_Struct[numVirtualSV].svAddr = svAddr;
  virtual_SV_Struct[numVirtualSV].bufSize = size+4;    // Ensure buffer size is a multiple of 3 and add an extra byte for the asciiz 0.
  virtual_SV_Struct[numVirtualSV].buffer = (char*) malloc(virtual_SV_Struct[numVirtualSV].bufSize);
  virtual_SV_Struct[numVirtualSV].type = WRITE_STRING;

  numVirtualSV++;

  return SUCCESS;
}

