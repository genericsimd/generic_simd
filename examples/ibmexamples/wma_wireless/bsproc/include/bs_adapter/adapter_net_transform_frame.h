/* ----------------------------------------------------------------------------
 * IBM Confidential

 IBM Wireless M2M Platform

 (C)Copyright IBM Corp. 2009, 2010, 2011

 All Rights Reserved.

 File Name: adapter_net_transform_frame.h

 Change Activity:

 Date             Description of Change                            By
 -----------      ---------------------                            --------
 25-May 2011       Created                                         jian jun chang 

 ----------------------------------------------------------------------------
 PROLOG END TAG zYx                                                         */
#ifndef  _ADAPTER_NET_TRANSFORM_FRAME_H_
#define  _ADAPTER_NET_TRANSFORM_FRAME_H_
#include "adapter_net_frame.h"
#include "mac_frame.h"
int net_transform_frame(struct net_frame *p_netframe, physical_subframe **p_phy_frame);
#endif
