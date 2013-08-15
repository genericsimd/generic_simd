/* ----------------------------------------------------------------------------
   IBM Confidential

   IBM Wireless M2M Platform   

   (C)Copyright IBM Corp. 2008, 2009, 2010, 2011

   All Rights Reserved.

   File Name: ss_scheduler.h

   Change Activity:

   Date             Description of Change                   By
   -----------      ---------------------					--------
   1-Oct.2008       Created                                 Parul Gupta

   ----------------------------------------------------------------------------
   PROLOG END TAG zYx                                                         */

#ifndef _SS_SCHEDULER_H
#define _SS_SCHEDULER_H

#include <math.h>
#include "mac.h"
#include "mac_sdu_queue.h"
#include "mac_frame.h"
#include "mac_amc.h"
#include "memmgmt.h"
#include "mac_connection.h"
#include "mac_header.h"
#include "init_maps.h"
#include "arq_ds.h"
#include "perf_log.h"
#include "perf_log_defns.h"
#include "scheduler_utils.h"
#include "dl_exp_params.h"

int ss_scheduler(sdu_queue* dl_sdu_queue, ModulCodingType mcs_type, int num_allotted_slots, logical_dl_subframe_map *frame_map);

int ss_serve_arqReTxQ_in_order(int num_allotted_bytes , int cid_range_start, int cid_range_end, int* dl_ss_bytes, logical_burst_map* lbm_ss, logical_pdu_map** cur_pdu_map_for_ss);

int ss_dl_serve_sduQ_in_order(sdu_queue* dl_sdu_queue, int num_allotted_bytes , int cid_range_start, int cid_range_end, int* dl_ss_bytes, logical_burst_map* lbm_ss, logical_pdu_map** cur_pdu_map_for_ss);

int ss_dl_serve_sduQ_by_QoS(sdu_queue* dl_sdu_queue, int num_allotted_bytes, int cid_range_start, int cid_range_end, int* dl_ss_bytes, logical_burst_map* lbm_ss, logical_pdu_map** cur_pdu_map_for_ss);

#endif

