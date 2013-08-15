/* ----------------------------------------------------------------------------
   IBM Confidential

   IBM Wireless M2M Platform   

   (C)Copyright IBM Corp. 2008, 2009, 2010, 2011

   All Rights Reserved.

   File Name: dl_scheduler.h

   Change Activity:

   Date             Description of Change                   By
   -----------      ---------------------					--------
   1-Oct.2008       Created                                 Parul Gupta

   ----------------------------------------------------------------------------
   PROLOG END TAG zYx                                                         */

#ifndef _DL_SCHEDULER_H
#define _DL_SCHEDULER_H

#include <math.h>
#include "mac.h"
#include "mac_sdu_queue.h"
#include "phy_params.h"
#include "mac_frame.h"
#include "mac_amc.h"
#include "memmgmt.h"
#include "mac_connection.h"
#include "mac_header.h"
#include "dlmap_builder.h"
#include "init_maps.h"
#include "arq_ds.h"
#include "perf_log.h"
#include "perf_log_defns.h"
#include "scheduler_utils.h"

int *dl_ss_slots;
int *dl_ss_bytes;
logical_burst_map** lbm_ss;
logical_pdu_map** cur_pdu_map_for_ss;

int alloc_dl_scheduler_var();
int free_dl_scheduler_var();

int dl_scheduler(sdu_queue* dl_sdu_queue, amc_info* amc_info_header, int num_dl_slots, logical_dl_subframe_map* frame_map, int* dlmap_length,int num_dl_subch);

int calc_dlmap_slots(int num_dlmap_ie);

int serve_arqReTxQ_in_order(int num_dl_slots, int cid_range_start, int cid_range_end, int* num_av_dl_slots_ptr, int* num_dlmap_ie_ptr, int* sum_slots_ptr, int* dl_ss_slots, int* dl_ss_bytes, logical_burst_map** lbm_ss, logical_pdu_map** cur_pdu_map_for_ss);

int dl_serve_sduQ_in_order(sdu_queue* dl_sdu_queue, int num_dl_slots, int cid_range_start, int cid_range_end, int* num_av_dl_slots_ptr, int* num_dlmap_ie_ptr, int* sum_slots_ptr, int* dl_ss_slots, int* dl_ss_bytes, logical_burst_map** lbm_ss, logical_pdu_map** cur_pdu_map_for_ss);

int dl_serve_sduQ_by_QoS(sdu_queue* dl_sdu_queue, int num_dl_slots, int cid_range_start, int cid_range_end, int* num_av_dl_slots_ptr, int* num_dlmap_ie_ptr, int* sum_slots_ptr, int* dl_ss_slots, int* dl_ss_bytes, logical_burst_map** lbm_ss, logical_pdu_map** cur_pdu_map_for_ss);

#endif
