/* ----------------------------------------------------------------------------
   IBM Confidential

   IBM Wireless M2M Platform   

   (C)Copyright IBM Corp. 2008, 2009, 2010, 2011

   All Rights Reserved.

   File Name: scheduler.h

   Change Activity:

   Date             Description of Change                   By
   -----------      ---------------------					--------
   1-Oct.2008       Created                                 Parul Gupta

   ----------------------------------------------------------------------------
   PROLOG END TAG zYx                                                         */

#ifndef _SCHEDULER_H
#define _SCHEDULER_H

#include "mac_sdu_queue.h"
#include "mac_frame.h"
#include "phy_params.h" 
#include "mac_amc.h"
#include "br_queue.h"
#include "memmgmt.h"
#include "mac_header.h"
#include "mac_serviceflow.h"
#include "perf_log_defns.h"
#include "perf_log.h"
#include "dl_exp_params.h"
#include "ul_scheduler.h"
#include "dl_scheduler.h"

int schedule(sdu_queue* dl_sdu_queue, br_queue **br_q_list, amc_info* amc_info_header, logical_dl_subframe_map* frame_map,int num_dl_subch, int num_ul_subch);

int calculate_phy_capacity(int num_ss, logical_dl_subframe_map *frame_map, float* dl_capacity, float* ul_capacity, int* num_dl_slots, int* num_ul_slots,int num_dl_subch,int num_ul_subch);

int fch_builder(dl_subframe_prefix* fch, int dlmap_len_in_slots);
#endif
