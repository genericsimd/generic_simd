/* ----------------------------------------------------------------------------
 * IBM Confidential

 IBM Wireless M2M Platform

 (C)Copyright IBM Corp. 2009, 2010, 2011

 All Rights Reserved.

 File Name: adapter_bs_interface_phy.h

 Change Activity:

 Date             Description of Change                            By
 -----------      ---------------------                            --------
 3-Mar 2011       Created                                         jian jun chang 

 ----------------------------------------------------------------------------
 PROLOG END TAG zYx                                                         */
#ifndef ADAPTER_INTERFACE_PHY_H_
#define ADAPTER_INTERFACE_PHY_H_
#include <stdlib.h>
#include "dlmap.h"
struct FCH_subframe{
    u_int8_t used_subchannel_bitmap;
    u_int8_t rsv1;
    u_int8_t repetition_coding_indication;
    u_int8_t coding_indication;
    u_int8_t dl_map_length; // define the length in slots of the burst which contains only DL-MAP message or compressed DL-MAP messge and compressed UL-MAP.
    u_int8_t rsv2;
};


int set_fch_subframe(const struct FCH_subframe * const p_fch_subframe);

int get_fch_subframe(struct FCH_subframe *const p_fch_subframe);

int adapter_dl_fch_parsing(u_int8_t *p_payload,u_int8_t is_128fft,u_int32_t * length,struct FCH_subframe  *const p_fch_subframe);

int adapter_init_dlmap(u_int8_t *payload,dl_map_msg* dlmap, u_int32_t *length);

int adapter_deinit_dlmap(dl_map_msg  * dl_map);
int adapter_dl_procblk_valid(u_int32_t n_cid, u_int16_t *cid);
#endif
