/* ----------------------------------------------------------------------------
   IBM Confidential

   IBM Wireless M2M Platform   

   (C)Copyright IBM Corp. 2008, 2009, 2010, 2011

   All Rights Reserved.

   File Name: mac_headermsg_builder.h

   Change Activity:

   Date             Description of Change                       By
   -----------      ---------------------                       --------
   03-Aug.2008		Created                                     Chen Lin

   ----------------------------------------------------------------------------
   PROLOG END TAG zYx                                                         */


#ifndef __MAC_HEADERMSG_BUILDER_H__
#define __MAC_HEADERMSG_BUILDER_H__

#include "mac_header.h"
#include "mac_config.h"
#include "mac_frame.h"
#include "ulmap.h"
#include "dlmap.h"

#define MAP_ALLOC_LENGTH 500

/** function interface */

/**---------------------------------------------------------------------------+
 | Function: build_ul_map_msg()
 |
 | Description: build UL MAP message
 |
 | Parameters:
 |           p_msg_header: [IN] UL MAP information structure head pointer
 |           p_payload: [OUT] message memory map
 |           p_payload_len: [OUT] message memory map length (in Byte)
 |
 | Return Values:
 |           0        successful
 |           other    faild
 |
 +---------------------------------------------------------------------------*/

int build_ul_map_msg (ul_map_msg const * p_msg_header, u_int8_t * p_payload,
        u_int32_t * p_payload_len);


/**---------------------------------------------------------------------------+
 | Function: build_frame_msg()
 |
 | Description: build DCD, UCD, DL-MAP, UL-MAP messages in Frame
 |
 | Parameters:
 |           frame_map: [IN/OUT] logical dl subframe map, the DCD, UCD, UL-MAP
 |                               will be filled in the first burst
 |           phy_subframe: [IN/OUT] phy subframe, FCH and DL-MAP will be filled
 |                                  in
 |
 | Return Values:
 |           0        successful
 |           other    faild
 |
 +---------------------------------------------------------------------------*/

int build_frame_msg( logical_dl_subframe_map * frame_map,
                     physical_subframe* phy_subframe );


int build_psh(const pack_sub_hdr* const psh, u_char* payload, u_int8_t is_extend, int* length);

int build_fsh(const frag_sub_hdr* const fsh, u_char* payload, u_int8_t is_extend, int* length);

extern int build_gmh(const generic_mac_hdr* const gmh, u_char* payload, int* length);

int build_dlframeprefix(const dl_subframe_prefix* const dlp, u_char* payload, int is_128fft, int* length);

int build_dlframeprefix_foak(const dl_subframe_prefix* const dlp, u_char* payload, int is_128fft, int* length);

int build_dlmap(const dl_map_msg* const dlmap, u_char* payload, int* length);

int build_mdbi(region_attri *ra, u_char** cur_ptr, int *length, int nibble_left);
/*
int build_dlmap(const dl_map_msg* const dlmap, u_char* payload, int* length);

int build_ulmap(const ul_map_msg* const ulmap, u_char* payload, int* length);

int build_dcd(const dcd_msg* const dlmap, u_char* payload, int* length);

int build_ucd(const ucd_msg* const ulmap, u_char* payload, int* length);
*/
#endif

