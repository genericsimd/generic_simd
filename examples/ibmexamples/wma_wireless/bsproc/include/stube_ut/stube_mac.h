/* ----------------------------------------------------------------------------
 * IBM Confidential

 IBM Wireless M2M Platform

 (C)Copyright IBM Corp. 2009, 2010, 2011

 All Rights Reserved.

 File Name: stube_mac.h

 Change Activity:

 Date             Description of Change                            By
 -----------      ---------------------                            --------
 23-JUL 2011       Created                                         jian jun chang 

 ----------------------------------------------------------------------------
 PROLOG END TAG zYx                                                         */
#ifndef  _STUBE_MAC_H_
#define  _STUBE_MAC_H_
#include "mac_frame.h"
#include "dlmap.h"
//changed by changjj
int  generate_physubframe(physical_subframe**mac_frame);

int  generate_dlmap(dl_map_msg**pp_dlmap);

#endif
