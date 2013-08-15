/* ----------------------------------------------------------------------------
   IBM Confidential

   IBM Wireless M2M Platform   

   (C)Copyright IBM Corp. 2008, 2009, 2010, 2011

   All Rights Reserved.

   File Name: mac_scheduling.c

   Change Activity:

   Date             Description of Change                   By
   -----------      ---------------------					--------
   3-Aug.2008       Created                                 Chen Lin

   ----------------------------------------------------------------------------
   PROLOG END TAG zYx                                                         */

#include "mac_scheduling.h"
#include "mac_amc.h"
#include "mac.h"
#include "mac_sdu_queue.h"
#include "phy_params.h"
#include "br_queue.h"
#include "mac_frame.h"
#include "mac_amc.h"
#include "init_maps.h"
#include "free_maps.h"
#include "scheduler.h"
#include "ul_scheduler.h"
#include "ss_scheduler.h"
#include "ul_mgt_msg_queue.h"

#include "sdu_cid_queue.h"

extern int cdma_allocation;
extern int cdma_uiuc;
extern short ranging_type;

int bs_scheduling(sdu_queue* dl_sduq, br_queue** brqlist, logical_dl_subframe_map * frame_map,int num_dl_subch, int num_ul_subch){
    
    amc_info* amc = NULL;
    get_amc_info(&amc);

#ifdef SS_TX
    short config_count = 0;
	int num_allotted_slots = 0;
	short uiuc = param_UL_MCS + 1;
	mgt_msg *mm = NULL;

	#ifndef RANGING_ENABLED
		#ifdef INTEGRATION_TEST
		mm = dequeue_ul_mgt_msg_queue(&ul_msg_queue[ULMAP_MMM_INDEX]);
		
		// Currently PHY has a one frame delay, so the ULMAP rcvd in Frame n
		// will be  used to form the UL tx subframe of Frame (n+1). If the delay
		// is reduced, change this code
		set_current_frame_number(mm->rcv_frame_num + 1);
		get_ss_allocation(mm->rcv_frame_num, param_MAX_VALID_BASIC_CID, &uiuc, &config_count, &num_allotted_slots);
		free(mm);

		#else
		get_ss_allocation(get_current_frame_number()%NUM_ULMAP_STORED , param_MAX_VALID_BASIC_CID, &uiuc, &config_count, &num_allotted_slots);
		#endif
	#else
		#ifdef SS_RX
		// For SS side, there is only one basic CID - its own - which will be taken from the simulation settings as below for now.
		if(ranging_type == PERIODIC_RANGING  /*|| ranging_type == NO_RANGING*/)
		{
			#ifdef INTEGRATION_TEST
			mm = dequeue_ul_mgt_msg_queue(&ul_msg_queue[ULMAP_MMM_INDEX]);
			
			// Currently PHY has a one frame delay, so the ULMAP rcvd in Frame n
			// will be  used to form the UL tx subframe of Frame (n+1). If the delay
			// is reduced, change this code
			set_current_frame_number(mm->rcv_frame_num + 1);
			get_ss_allocation(mm->rcv_frame_num, param_MAX_VALID_BASIC_CID, &uiuc, &config_count, &num_allotted_slots);
			free(mm);
			#else
			get_ss_allocation(get_current_frame_number()%NUM_ULMAP_STORED , param_MAX_VALID_BASIC_CID, &uiuc, &config_count, &num_allotted_slots);
			#endif
		}
		else
		{
			#ifdef INTEGRATION_TEST
			mm = dequeue_ul_mgt_msg_queue(&ul_msg_queue[ULMAP_MMM_INDEX]);
			
			// Currently PHY has a one frame delay, so the ULMAP rcvd in Frame n
			// will be  used to form the UL tx subframe of Frame (n+1). If the delay
			// is reduced, change this code
			set_current_frame_number( mm->rcv_frame_num + 1);
			free(mm);
			#endif

			pthread_mutex_lock(&ranging_mutex);
			num_allotted_slots = cdma_allocation;
			uiuc = cdma_uiuc;
			cdma_allocation = 0;
			pthread_mutex_unlock(&ranging_mutex);
		}
		#else
			FLOG_FATAL("Error: RANGING_ENABLED defined with wrong SS_TX and SS_RX settings\n");
			return -1;
		#endif
	#endif

	//    ModulCodingType ul_mcs = get_mcs_from_uiuc(get_ucd_msg(config_count), uiuc);
	// Currently for FOAK, UIUC values are hardcoded QPSK 1/2: 1, i
	// 16QAM 1/2: 3, 64QAM 1/2: 5 .. i.e. uiuc = ul_mcs + 1
	ModulCodingType ul_mcs = uiuc - 1;

    //printf("Calling SS Scheduler for frame: %ld. allocation for this SS is: %d, MCS is: %d\n", frame_number, num_allotted_slots, ul_mcs);

    // Right now we don't have AMC functionality hence taking the DL MCS from simulation settings
    // Later, when the AMC functionality is implemented, it would have to be taken from the AMC module 
    ss_scheduler(dl_sduq, ul_mcs, num_allotted_slots, frame_map);
	frame_map->num_bursts = 1;
#else
    // Changed brq to &brq during code integration. This is a quick fix for
    // consistency in function prototype. bs_scheduling will be removed
#ifdef AMC_ENABLE
	update_amc_before_bs_scheduling(amc);
#endif
    schedule(dl_sduq, brqlist, amc, frame_map,num_dl_subch, num_ul_subch);
#endif
    
    return 0;
}


