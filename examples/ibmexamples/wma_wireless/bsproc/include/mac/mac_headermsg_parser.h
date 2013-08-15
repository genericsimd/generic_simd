/* ----------------------------------------------------------------------------
   IBM Confidential

   IBM Wireless M2M Platform   

   (C)Copyright IBM Corp. 2008, 2009, 2010, 2011

   All Rights Reserved.

   File Name: mac_headermsg_parser.h

   Change Activity:

   Date             Description of Change                       By
   -----------      ---------------------                       --------
   03-Aug.2008		Created                                     Chen Lin

   ----------------------------------------------------------------------------
   PROLOG END TAG zYx                                                         */


#ifndef __MAC_HEADERMSG_PARSER_H__
#define __MAC_HEADERMSG_PARSER_H__

#include "mac_header.h"
#include "mac_config.h"
#include "mac_frame.h"
#include "ulmap.h"

/**---------------------------------------------------------------------------+
 | Function: parse_ul_map_msg()
 |
 | Description: parse UL MAP message
 |
 | Parameters:
 |           p_payload: [IN] message memory map
 |           payload_len: [IN] message memory map length (in Byte)
 |           p_msg_header: [OUT] UL MAP information structure head pointer
 |
 | Return Values:
 |           0        successful
 |           other    faild
 |
 +---------------------------------------------------------------------------*/

int parse_ul_map_msg (u_int8_t * const p_payload, u_int32_t const payload_len,
                      ul_map_msg * p_msg_header);

int parse_psh(u_char* payload, pack_sub_hdr* psh, u_int8_t is_extend, int* length);

int parse_fsh(u_char* payload, frag_sub_hdr* fsh, u_int8_t is_extend, int* length);

extern int parse_gmh(u_char* payload, generic_mac_hdr* gmh, int* length);

int parse_dlframeprefix(u_char* payload, dl_subframe_prefix* dlp, u_int8_t is_128fft, int* length);

int parse_dlmap(u_char* payload, dl_map_msg* dlmap, int* length);
int parse_mdbi(region_attri *ra, u_char* payload, int *p_offset, int nibble_left);

/*
int parse_dlmap(u_char* payload, dl_map_msg* dlmap, int* length);

int parse_ulmap(u_char* payload, ul_map_msg* ulmap, int* length);

int parse_dcd(u_char* payload, dcd_msg* dlmap, int* length);

int parse_ucd(u_char* payload, ucd_msg* ulmap, int* length);
*/
#endif
