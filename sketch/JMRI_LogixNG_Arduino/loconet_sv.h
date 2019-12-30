#ifndef __LOCONET_SV
#define __LOCONET_SV

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


#include <LocoNet.h>

#define MAX_VIRTUAL_SV 10

#define SUCCESS 0
#define TOO_MANY_VIRTUAL_SV_REG 1

/**
 * Callback function for SV registers containing a string.
 * When a string has been received, this method is called.
 *
 * Parameters:
 *   sv_addr: the first SV address
 *   str: the received string
 */
typedef void (*receiveString)(int sv_addr, char *str);

/**
 * Callback function for SV registers containing a string.
 * When a string has been received, this method is called.
 *
 * Parameters:
 *   sv_addr: the first SV address
 *   str: the received string
 */
typedef void (*receiveFloat)(int sv_addr, char *str);



typedef struct
{
	int address;
	int size;	// Must be a multiple of 4
//	receive_string callback;	// The callback method for this SV.
} VIRTUAL_SV_STRING_STRUCT;


typedef struct
{
	int address;
//	receive_float callback;		// The callback method for this SV.
} VIRTUAL_SV_FLOAT_STRUCT;

/*
typedef struct
{
	receive_string callback;
	int sv_addr;
	int size;
	char *buffer;
	int index;
} SV_STRUCT;
*/

/*
typedef enum
{
  VIRTUAL_SV_OK = 0,
  VIRTUAL_SV_ERROR = 1,
  VIRTUAL_SV_DEFERRED_PROCESSING_NEEDED = 2,
  VIRTUAL_SV_NOT_HANDLED = 99
} VIRTUAL_SV_STATUS ;
*/

#define VIRTUAL_SV_NOT_HANDLED 99

typedef enum
{
	READ_FLOAT = 1,
	WRITE_FLOAT,
	READ_STRING,
	WRITE_STRING
} VIRTUAL_SV_TYPE;


typedef struct
{
	union {
		receiveFloat	callbackFloat;
		receiveString	callbackString;
	};
	uint16_t	svAddr;
	char		*buffer;
	uint8_t		bufSize;
//	uint8_t		lastIndex;
	VIRTUAL_SV_TYPE		type;
} VIRTUAL_SV_STRUCT;







class LocoNetVirtualSystemVariableClass
{
  private:
//	LocoNetSystemVariableClass  sv;
    
	uint8_t		mfgId ;
	uint8_t 	devId ;
	uint16_t	productId ;
	uint8_t		swVersion ;
    
	uint8_t		DeferredProcessingRequired ;
	uint8_t		DeferredSrcAddr ;
    

	VIRTUAL_SV_STRUCT	virtual_SV_Struct[MAX_VIRTUAL_SV];
	uint8_t numVirtualSV;



	/** Checks whether the given Offset is a valid value.
	 *
	 * Returns:
	 *		True - if the given Offset is valid. False Otherwise.
	 */
    uint8_t isSVStorageValid(uint16_t Offset);
	
	/** Read the NodeId (Address) for SV programming of this module.
	 *
	 * This method accesses multiple special EEPROM locations.
	 */
    uint16_t readSVNodeId(void);
	
	/** Write the NodeId (Address) for SV programming of this module.
	 *
	 * This method accesses multiple special EEPROM locations.
	 */
    uint16_t writeSVNodeId(uint16_t newNodeId);
	
	/**
	 * Checks whether all addresses of an address range are valid (defers to
	 * isSVStorageValid()). Sends a notification for the first invalid address
	 * (long Ack with a value of 42).
	 *
	 *	TODO: There is a Type error in this method. Return type is bool, but
	 *		actual returned values are Integer.
	 *
	 * Returns:
	 *		0 if at least one address of the range is not valid.
	 *		1 if all addresses out of the range are valid.
	 */
    bool CheckAddressRange(uint16_t startAddress, uint8_t Count);

  public:
	void init(uint8_t newMfgId, uint8_t newDevId);
	
	/**
	 * Check whether a message is an SV programming message. If so, the message
	 * is processed.
	 * Call this message in your main loop to implement SV programming.
	 *
	 * TODO: This method should be updated to reflect whether the message has
	 *	been consumed.
	 *
	 * Note that this method will not send out replies.
	 *
	 * Returns:
	 *		SV_OK - the message was or was not an SV programming message.
				It may or may not have been consumed.
	 *		SV_DEFERRED_PROCESSING_NEEDED - the message was an SV programming
				message and has been consumed. doDeferredProcessing() must be
				called to actually process the message.
	 *		SV_ERROR - the message was an SV programming message and carried
				an unsupported OPCODE.
	 *
	 */
	int processMessage(lnMsg *LnPacket );
	
    /** Read a value from the given EEPROM offset.
     *
     * There are two special values for the Offset parameter:
     *	SV_ADDR_EEPROM_SIZE - Return the size of the EEPROM
     *  SV_ADDR_SW_VERSION - Return the value of swVersion
     *  3 and on - Return the byte stored in the EEPROM at location (Offset - 2)
     *
     * Parameters:
     *		Offset: The offset into the EEPROM. Despite the value being passed as 2 Bytes, only the lower byte is respected.
     *
     * Returns:
     *		A Byte containing the EEPROM size, the software version or contents of the EEPROM.
     *
     */
    uint8_t readSVStorage(uint16_t Offset );
    
    /** Write the given value to the given Offset in EEPROM.
     *
     * TODO: Writes to Offset 0 and 1 will cause data corruption.
     *
     * Fires notifySVChanged(Offset), if the value actually chaned.
     *
     * Returns:
     *		A Byte containing the new EEPROM value (even if unchanged).
     */
    uint8_t writeSVStorage(uint16_t Offset, uint8_t Value);
    
	/**
	 * Attempts to send a reply to an SV programming message.
	 * This method will repeatedly try to send the message, until it succeeds.
	 *
	 * Returns:
	 *		SV_OK - Reply was successfully sent.
	 *		SV_DEFERRED_PROCESSING_NEEDED - Reply was not sent, a later retry is needed.
	 */
    SV_STATUS doDeferredProcessing( void );


    int handleSVReadWrite(uint16_t address, uint8_t &d1, uint8_t &d2, uint8_t &d3, uint8_t &d4);

    /**
     * Register SV registers for a string.
     *
     * Parameters:
     *   callback: the function that will be called when a string is received.
     *   sv_addr: the first SV address
     *   size: the maximum number of bytes the string may have, excluding the
     *         0x00 character ending the string.
     * Returns
     *   0 if success. 1 if too many sv_strings are already registered
     */
  public:
    int registerSV_WriteString(receiveString callback, int svAddr, int size);

};





#endif
