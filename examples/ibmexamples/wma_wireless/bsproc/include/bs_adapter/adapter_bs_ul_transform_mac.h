/* ----------------------------------------------------------------------------
 * IBM Confidential

 IBM Wireless M2M Platform

 (C)Copyright IBM Corp. 2009, 2010, 2011

 All Rights Reserved.

 File Name: adapter_bs_ul_transform_mac.h

 Change Activity:

 Date             Description of Change                            By
 -----------      ---------------------                            --------
 20-May 2011       Created                                         jian jun chang 

 ----------------------------------------------------------------------------
 PROLOG END TAG zYx                                                         */
#ifndef  _ADAPTER_BS_UL_TRANSFORM_MAC_H_
#define  _ADAPTER_BS_UL_TRANSFORM_MAC_H_

#include "mac_frame.h"
#include "adapter_bs_ul_map_interface.h"

int adapter_transform_physical_frame(struct  ul_frame_ie *p_frame, physical_subframe **phy_frame, void ** pr_result);
#endif
