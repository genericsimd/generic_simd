/* ----------------------------------------------------------------------------
 * IBM Confidential

 IBM Wireless M2M Platform

 (C)Copyright IBM Corp. 2009, 2010, 2011

 All Rights Reserved.

 File Name: adaptr_dump.h

 Change Activity:

 Date             Description of Change                            By
 -----------      ---------------------                            --------
 20-Jan 2011       Created                                         jian jun chang 

 ----------------------------------------------------------------------------
 PROLOG END TAG zYx                                                         */
#ifndef _ADAPTER_DUMP_H_
#define _ADAPTER_DUMP_H_

#include <stdio.h>
#include  "dlmap.h"
#include  "mac_frame.h"
#include  "phy_dl_tx.h"
#include  "adapter_bs_dl_interface_data.h"

void   dump_dlmap_ie_link(dl_map_ie *p_dl_map_ie_head,int itimes,int nums);
void   dump_burst_link(phy_burst *p_burst_head,int itimes,int nums);
void   dump_map_ie_node(dl_map_ie *p_node,int itimes);
void   dump_burst_node(phy_burst *p_burst_head,int itimes);

void adapter_dump_slot_symbol_data(FILE *fp, struct phy_dl_slotsymbol *phybuffer,int ntimes);
void adapter_dump_phy_dl_slot_bench(FILE *fp, struct phy_dl_slotsymbol *phybuffer);
#endif
