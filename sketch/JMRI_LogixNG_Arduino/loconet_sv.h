#ifndef __LOCONET_SV
#define __LOCONET_SV

/**
 * Callback function for SV registers containing a string.
 * When a string has been received, this method is called.
 *
 * Parameters:
 *   sv_addr: the first SV address
 *   str: the received string
 */
typedef void (*receive_string)(int sv_addr, char *str);

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
int register_sv_string(receive_string callback, int sv_addr, int size);

#endif
