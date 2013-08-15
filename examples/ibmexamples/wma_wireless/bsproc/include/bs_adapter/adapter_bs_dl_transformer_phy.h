/* ----------------------------------------------------------------------------
 * IBM Confidential

 IBM Wireless M2M Platform

 (C)Copyright IBM Corp. 2009, 2010, 2011

 All Rights Reserved.

 File Name: adapter_bs_dl_transformer_phy.h

 Change Activity:

 Date             Description of Change                            By
 -----------      ---------------------                            --------
 2-Mar 2011       Created                                         jian jun chang 

 ----------------------------------------------------------------------------
 PROLOG END TAG zYx                                                         */
#ifndef __MAC_PHY_TRANSFORMER_H__
#define __MAC_PHY_TRANSFORMER_H__

#ifndef KERNEL_VERSION
#define KERNEL_VERSION(a,b,c)    ((a)*65536+(b)*256+( c ))
#endif

#include  "selectserver.h"
#include "mac_phy_data.h"
#include "mac_frame.h"

#include "adapter_bs_dl_interface_data.h"

#include "mac_frame.h"
#include "mac_common_data.h"

struct adapter_dts_info
{
    char active_band[21]; //interference information from spectrum scanning, 1--interference
    int dl_unused_subch;  //unused subchannel number of DL,
                               //  including the interference and active forbidden channel
    int ul_unused_subch; //unused subchannel number of DL,
                               //  including the interference and active forbidden channel
};

int phy_subframe_send_transform_dl(physical_subframe* phy_subframe,struct  ofdma_map* p_ofdma_frame,dl_map_msg * p_dl_map);


int adapter_transform_symbolslot_1(struct ofdma_map  *p_ofdma_frame,int islotnum,int frame_index,struct  phy_dl_slotsymbol **p_phy);
int adapter_transform_symbolslot_2(struct ofdma_map  *p_ofdma_frame,int islotnum,int frame_index,struct  phy_dl_slotsymbol **p_phy1,struct phy_dl_slotsymbol **p_phy2);
//
//int ofdma_send_netframe(ofdma_map *ofdma_frame,int fdsocket,struct sockaddr_in *destaddr);









#endif
