/* ----------------------------------------------------------------------------
 * IBM Confidential

 IBM Wireless M2M Platform

 (C)Copyright IBM Corp. 2009, 2010, 2011

 All Rights Reserved.

 File Name: adapter_prefix_build.h

 Change Activity:

 Date             Description of Change                            By
 -----------      ---------------------                            --------
 22-Feb 2011       Created                                         jian jun chang 

 ----------------------------------------------------------------------------
 PROLOG END TAG zYx                                                         */
#ifndef ADAPTER_PREFIX_BUILD_H_
#define ADAPTER_PREFIX_BUILD_H_
#include "mac_frame.h"
#include "mac_header.h"
#include "mac_crc.h"
#include "mac_hcs.h"
#include "init_maps.h"
int build_dlmap (const dl_map_msg* const dlmap, u_char* payload, int* length);
#endif
