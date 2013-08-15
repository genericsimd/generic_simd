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
#ifndef _ADAPTER_TEST_STUBE_H_
#define _ADAPTER_TEST_STUBE_H_
#include "dcd.h"
#include "ucd.h"
#include "dlmap.h"
#include "ulmap.h"
#include "mac_modulation_adapter.h"

dl_map_msg* dl_map_stube;
ul_map_msg* ul_map_stube;
dcd_msg* dcd_stube;
ucd_msg* ucd_stube;


void setdcdmsgdata(dcd_msg * dcdmsg);


int set_dcd_msg_stube(dcd_msg* dcdmsg);
int set_ucd_msg_stube(ucd_msg* ucdmsg);
int set_dl_map_msg_stube(dl_map_msg* dlmap);
int set_ul_map_msg_stube(ul_map_msg* ulmap);
int get_dcd_msg_stube(dcd_msg** dcdmsg);
int get_ucd_msg_stube(ucd_msg** ucdmsg);
int get_dl_map_msg_stube(dl_map_msg** dlmap);
int get_ul_map_msg_stube(ul_map_msg** ulmap);


#endif
