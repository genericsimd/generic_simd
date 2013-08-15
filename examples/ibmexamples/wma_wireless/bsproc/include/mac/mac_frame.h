/* ----------------------------------------------------------------------------
   IBM Confidential

   IBM Wireless M2M Platform   

   (C)Copyright IBM Corp. 2008, 2009, 2010, 2011

   All Rights Reserved.

   File Name: mac_frame.h

   Change Activity:

   Date             Description of Change                   By
   -----------      ---------------------					--------
   1-Oct.2008       Created                                 Malolan Chetlur

   ----------------------------------------------------------------------------
   PROLOG END TAG zYx                                                         */

#ifndef __MAC_FRAME_H__
#define __MAC_FRAME_H__

#include <stdlib.h>
#include "string.h"
#include "mac_config.h"
#include "logical_packet.h"
#include "mac_message.h"
#include "ranging_mm.h"
#include "mac.h"

typedef struct {
    int num_bytes;
    int cid;
    int id;
    int start_bsn;
}transport_sdu_map;

typedef struct  {
  logical_packet* mac_mgmt_msg_sdu;
}mgmtmsg_sdu_map;

typedef struct{
  logical_packet* arq_retransmit_block;
} arq_retrans_sdu_map ;

// contains all the sdu that should be contained in this map
typedef struct logicalpdumap logical_pdu_map;
struct logicalpdumap{
  int cid;
  int sdu_num;
  transport_sdu_map* transport_sdu_map;
  mgmtmsg_sdu_map* mac_msg_map;
  arq_retrans_sdu_map* arq_sdu_map;  
  logical_pdu_map* next;
}; 

typedef struct logicalburstmap logical_burst_map;

struct logicalburstmap{
    int map_burst_index;
    int pdu_num;
    int burst_bytes_num;
    logical_pdu_map * pdu_map_header;
    logical_burst_map * next;
};

typedef struct{
    char is_active[21];
    int num_dl_interference;
    int num_ul_interference;
}dts_info;


typedef struct {
    u_int8_t used_subchannel_bitmap;
    u_int8_t rsv1;
    u_int8_t repetition_coding_indication;
    u_int8_t coding_indication;
    u_int8_t dl_map_length; // define the length in slots of the burst which contains only DL-MAP message or compressed DL-MAP messge and compressed UL-MAP.
    u_int8_t rsv2;
    dts_info *p_dts_info; 
}dl_subframe_prefix;

typedef struct {
    int num_bursts;
    dl_subframe_prefix * fch;
    dl_map_msg * dl_map;
    ul_map_msg * ul_map;
    dcd_msg * dcd;
    ucd_msg * ucd;
    logical_burst_map * burst_header;
}logical_dl_subframe_map;

typedef struct phyburst phy_burst;

struct phyburst{
    int length;
    u_char * burst_payload;
    int map_burst_index;
    /*---------------add by changjj-----------------*/
    u_int32_t   cid;
#ifdef AMC_ENABLE
    int		cinr;
#endif
    u_int32_t   burst_len;
    /*---------------------------------*/
    phy_burst * next;
};

typedef struct physicalsubframe physical_subframe;
struct physicalsubframe{
    unsigned int frame_num;
    int bursts_num;
    int sense_flag; //Flag to tell Phy to sense spectrum.
    int sensing_info_present; //Flag Phy can set to tell MAC that interference_info is present
    dts_info *interference_info; 
	ranging_info *p_ranging_info;
    dl_map_msg * dl_map;  /* will not be used */
    ul_map_msg * ul_map;  /* will not be used */
    void * fch_dl_map;
    unsigned int fch_dl_map_len;
    /** add by zzb */
    void * raw_ul_map;
    unsigned int raw_ul_map_len;
    /** end by zzb */
    phy_burst * burst_header;
    physical_subframe * next;
};

int initialize_logical_subframe_map(logical_dl_subframe_map** subframe_map);

int release_logical_subframe_map(logical_dl_subframe_map* subframe_map);

#endif
