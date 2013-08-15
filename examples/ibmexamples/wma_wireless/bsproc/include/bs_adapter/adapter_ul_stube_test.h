/* ----------------------------------------------------------------------------
 * IBM Confidential

 IBM Wireless M2M Platform

 (C)Copyright IBM Corp. 2009, 2010, 2011

 All Rights Reserved.

 File Name: adapter_test_stube.h

 Change Activity:

 Date             Description of Change                            By
 -----------      ---------------------                            --------
 22-Apr 2011       Created                                         jian jun chang 

 ----------------------------------------------------------------------------
 PROLOG END TAG zYx                                                         */
#ifndef _ADAPTER_UL_STUBE_TEST_H_
#define _ADAPTER_UL_STUBE_TEST_H_

#include "adapter_config.h"
#include "ulmap.h"
#include "mac_frame.h"

struct fake_ulmap_ie
{
    int duration;
    int uiuc;
    int bcid;
    struct fake_ulmap_ie * next;
};

struct fake_ul_map
{
    int ss_num;
    struct fake_ulmap_ie ie;
};


//int build_ul_map(ul_map_msg *p_ul_map, struct fake_ul_map * p_ulmap);
int adapter_build_phy_subframe(physical_subframe* phy_subframe);
int adapter_build_ul_map(ul_map_msg *ul_map, struct fake_ul_map * p_fake_ulmap);

#endif
