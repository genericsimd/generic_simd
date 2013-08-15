/* ----------------------------------------------------------------------------
   IBM Confidential

   IBM Wireless M2M Platform   

   (C)Copyright IBM Corp. 2008, 2009, 2010, 2011

   All Rights Reserved.

   File Name: ul_scheduler.h

   Change Activity:

   Date             Description of Change                   By
   -----------      ---------------------					--------
   1-Oct.2008       Created                                 Parul Gupta

   ----------------------------------------------------------------------------
   PROLOG END TAG zYx                                                         */

#ifndef _UL_SCHEDULER_H
#define _UL_SCHEDULER_H

#include<math.h>

#include "mac_sdu_queue.h"
#include "mac_frame.h"
#include "phy_params.h" 
#include "mac_amc.h"
#include "br_queue.h"
#include "memmgmt.h"
#include "mac_header.h"
#include "mac_serviceflow.h"
#include "mac_connection.h"
#include "mac_common_data.h"
#include "init_maps.h"

int ul_scheduler(br_queue** br_q_list, amc_info* amc_info_header, int num_ul_data_slots, int num_ul_symbols, int num_ss, ul_map_msg* ul_map);

int serve_fifo(br_queue *br_q, int* ul_ss_allocation, int* num_UL_data_slots);

int ul_map_builder(int* ul_ss_allocation, int num_ul_symbols, int num_ss, ul_map_msg* ul_map, ranging_adjust_ie *p_list_head);

int print_ulmap(ul_map_msg *ul_map);

int dump_ulmap(FILE * fp_ulmap, ul_map_msg *ul_map);

// This function will be used by the SS side
int get_ss_allocation(long int frame_num, int basic_cid, short *uiuc, short *config_count, int *num_allotted_slots);

int initialize_ulmap_list_for_ss();

#endif
