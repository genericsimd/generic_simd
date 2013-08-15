/* ----------------------------------------------------------------------------
 * IBM Confidential

 IBM Wireless M2M Platform

 (C)Copyright IBM Corp. 2009, 2010, 2011

 All Rights Reserved.

 File Name: adapter_frame_transform_net.h

 Change Activity:

 Date             Description of Change                            By
 -----------      ---------------------                            --------
 20-Mar 2011       Created                                         jian jun chang 

 ----------------------------------------------------------------------------
 PROLOG END TAG zYx                                                         */
#include "mac_frame.h"
#include "adapter_net_frame.h"
#ifndef   ADAPTER_FRAME_TRANSFORM_NET_H_
#define   ADAPTER_FRAME_TRANSFROM_NET_H_
int transform_phyframe_netframe(physical_subframe *p_phy_frame, struct net_frame **p_netframe);
#endif
