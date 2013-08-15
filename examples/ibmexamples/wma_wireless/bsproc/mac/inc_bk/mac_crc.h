/* ----------------------------------------------------------------------------
   IBM Confidential

   IBM Wireless M2M Platform   

   (C)Copyright IBM Corp. 2008, 2009, 2010, 2011

   All Rights Reserved.

   File Name: mac_crc.h

   Change Activity:

   Date             Description of Change                   By
   -----------      ---------------------					--------
   03-Aug.2008      Created                                 Zhenbo Zhu

   ----------------------------------------------------------------------------
   PROLOG END TAG zYx                                                         */

#ifndef __MAC_CRC_H__
#define __MAC_CRC_H__
#include "debug.h"
#include "mac.h"

#define LEN    32
#define MSB    0x80000000

#define BYTE_A    0xFF000000
#define BYTE_B    0x00FF0000
#define BYTE_C    0x0000FF00
#define BYTE_D    0x000000FF

/*
 * Macro define following the IEEE 802.16e standard 6.3.3.5.2.1
 */

#define POLY    0x04C11DB7
#define INIT_REM    0xFFFFFFFF
#define INIT_XOR    0xFFFFFFFF


/**
 * Function: crc_init()
 *
 * Description: initialize a 256 entries Teable for CRC-32 algorithms
 *              it can be implemented offline.
 *
 * Parameters:
 *           poly: [IN] polynomial for CRC-32 calculation
 *
 * Global variable:
 *           crc_table: [IN/OUT] pointer which points to a 256 entries * 4Bytes Table
 *
 * Return Values:
 *           0        successful
 *           other    faild
 */

int crc_init (u_int32_t const poly);


/**
 * Function: crc_calculation()
 *
 * Description: calculate the 32 bits CRC
 *
 * Parameters:
 *           input: [IN] input data for CRC-32 calculation
 *           length: [IN] Bytes number of input data
 *           output: [OUT] CRC-32 result
 *
 * Global variable:
 *           crc_table: [IN] pointer which points to a 256 entries * 4Bytes Table
 *
 * Return Values:
 *           0        successful
 *           other    faild
 */

int crc_calculation (u_char const * input,
                     int const length,
                     u_char* output);

/*
 * Function: crc_verification()
 *
 * Description: ckeck the 32 bits CRC results
 *
 * Parameters:
 *           input: [IN] input data for CRC-32 calculation
 *           length: [IN] Bytes number of input data
 *           output: [OUT] CRC-32 result, 0 for pass, other value for faild
 *
 * Global variable:
 *           crc_table: [IN] pointer which points to a 256 entries * 4Bytes Table
 *
 * Return Values:
 *           0        successful
 *           other    faild
 */

int crc_verification (u_char const * input,
                      int const length,
                      u_char * output);


#endif /* __MAC_CRC_H__ */

