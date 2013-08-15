/* ----------------------------------------------------------------------------
   IBM Confidential

   IBM Wireless M2M Platform   

   (C)Copyright IBM Corp. 2008, 2009, 2010, 2011

   All Rights Reserved.

   File Name: mac_ul_cps_controller.c

   Change Activity:

   Date             Description of Change                       By
   -----------      ---------------------                       --------
   03-Aug.2008		Created                                     Chen Lin

   ----------------------------------------------------------------------------
   PROLOG END TAG zYx                                                         */


#include "mac_ul_cps_controller.h"
#include "ul_mgt_msg_queue.h"
#include "thread_sync.h"
#include "mac_ss_ranging_utm.h"

/** add by zzb for integration */
#ifdef INTEGRATION_TEST
#include "queue_util.h"
#include "adapter_bs_ul_transform_mac.h"
#include "adapter_bs_ul_map_interface.h"
#include "phy_ul_rx_ranging.h"
#include "monitor_proc.h"
#endif
/** end by zzb for integration */

dts_info int_info;
int interference_flag=0;

/** add by zzb for integration */
dts_info * gp_int_info = NULL;
/** end by zzb for integration */

extern pthread_mutex_t int_info_lock;
pthread_t ss_ir_thread, ss_pr_thread;
short ranging_type;
extern sll_fifo_q *bs_ranging_q;
extern sll_fifo_q *ranging_q;

#ifdef INTEGRATION_TEST
int convert_ranging_result(physical_subframe *sub_frame, struct phy_ranging_result *p_ranging_result)
{
    ranging_adjust_ie * p_ranging_ie = NULL;
    ranging_adjust_ie *p_ranging_cur = NULL;

    if(p_ranging_result == NULL)
    {
        FLOG_DEBUG("no ranging_result\n");
        return 1;
    }

    if(p_ranging_result->user_num == 0)
    {
        FLOG_DEBUG("no ranging user check\n");
        return 1;
    }

    if (sub_frame->p_ranging_info == NULL)
    {
        sub_frame->p_ranging_info = malloc(sizeof(ranging_info));
        memset(sub_frame->p_ranging_info, 0, sizeof(ranging_info));
    }

    p_ranging_ie = (ranging_adjust_ie * )malloc(sizeof(ranging_adjust_ie));
    memset(p_ranging_ie, 0, sizeof(ranging_adjust_ie));

    p_ranging_cur = sub_frame->p_ranging_info->p_ranging_ie_list;

    if (p_ranging_cur != NULL)
    {
        while(p_ranging_cur->p_next != NULL)
        {
            p_ranging_cur = p_ranging_cur->p_next;
        }

        p_ranging_cur->p_next = p_ranging_ie;
    }else
    {
        sub_frame->p_ranging_info->p_ranging_ie_list = p_ranging_ie;
    }

    p_ranging_cur = p_ranging_ie;

    p_ranging_ie->timing_adjust = p_ranging_result->ranging_result_0.time_offset;

    if (p_ranging_result->ranging_result_0.adj_power_fix > 127)
    {
        p_ranging_ie->power_adjust = 127;
    }else if (p_ranging_result->ranging_result_0.adj_power_fix < -128)
    {
        p_ranging_ie->power_adjust = -128;
    }else
    {
        p_ranging_ie->power_adjust = p_ranging_result->ranging_result_0.adj_power_fix;
    }

    p_ranging_ie->frequency_adjust = p_ranging_result->ranging_result_0.frequency_offset;
    p_ranging_ie->ranging_code = p_ranging_result->ranging_result_0.ranging_code_id;

    if (p_ranging_result->ranging_type == BITMAP_IR)
    {
        p_ranging_ie->ranging_symbol = 0;
        p_ranging_ie->ranging_subchannel = 0;
    }else if (p_ranging_result->ranging_type == BITMAP_PR)
    {
        p_ranging_ie->ranging_symbol = PR_REGION_START_SYMBOL;
        p_ranging_ie->ranging_subchannel = 0;
    }

    sub_frame->p_ranging_info->num_ranging_adjust_ie ++;

    if(p_ranging_result->user_num > 1 )
    {
        p_ranging_ie = (ranging_adjust_ie * )malloc(sizeof(ranging_adjust_ie));
        memset(p_ranging_ie, 0, sizeof(ranging_adjust_ie));

        p_ranging_cur->p_next = p_ranging_ie;

        p_ranging_ie->timing_adjust = p_ranging_result->ranging_result_1.time_offset;

        if (p_ranging_result->ranging_result_0.adj_power_fix > 127)
        {
            p_ranging_ie->power_adjust = 127;
        }else if (p_ranging_result->ranging_result_0.adj_power_fix < -128)
        {
            p_ranging_ie->power_adjust = -128;
        }else
        {
            p_ranging_ie->power_adjust = p_ranging_result->ranging_result_0.adj_power_fix;
        }

        p_ranging_ie->frequency_adjust = p_ranging_result->ranging_result_1.frequency_offset;
        p_ranging_ie->ranging_code = p_ranging_result->ranging_result_1.ranging_code_id;

        if (p_ranging_result->ranging_type == BITMAP_IR)
        {
            p_ranging_ie->ranging_symbol = 0;
            p_ranging_ie->ranging_subchannel = 0;
        }else if (p_ranging_result->ranging_type == BITMAP_PR)
        {
            p_ranging_ie->ranging_symbol = PR_REGION_START_SYMBOL;
            p_ranging_ie->ranging_subchannel = 0;
        }

        sub_frame->p_ranging_info->num_ranging_adjust_ie ++;
    }

    return 0;
}
#endif

void* ul_parsing(void* arg)
{
#ifdef INTEGRATION_TEST
    if (pthread_setcanceltype (PTHREAD_CANCEL_ASYNCHRONOUS, NULL) != 0)
    {
        return NULL;
    }
#else
    if (pthread_setcanceltype (PTHREAD_CANCEL_DEFERRED, NULL) != 0)
    {
        return NULL;
   }
#endif

#ifndef SS_TX
#ifndef SS_RX
	int i;
        int ret = 0;
#endif
#endif

#ifdef SS_TX
#ifdef SS_RX
	int ret = 0,test_flag=0;
#endif
#endif
    int pkm_ret;
    int my_basic_cid;
    mac_ul_cps_args * ul_args = (mac_ul_cps_args *) arg;
    physical_subframe* phy_subframe;
    subframe_queue* ul_subframeq = ul_args->ul_subframeq;
    pdu_queue* ul_pduq_head = ul_args->pduq;
    ul_br_queue* brqlist = ul_args->br_q_list;
    pdu_frame_queue* pdu_frameq;
    frag_queue* fragq = ul_args->fragq;
    sdu_queue* ul_sduq = ul_args->ul_sduq;
    mgt_msg_queue *ul_msgq = ul_args->ul_msgq;
    // now obtain the downlink subframe queue to implement our close-loop testing
    get_subframe_queue(1, &ul_subframeq);

    void * pr_result = NULL;

/** add by zzb for integration */
#ifdef INTEGRATION_TEST
    struct queue_msg *p_msg = (struct queue_msg *)malloc(sizeof(struct queue_msg));
    int ranging_count = 0;
    int ranging_idx;
#endif

/** end by zzb for integration */

    while (can_sync_continue()){
        phy_subframe = NULL;

/** add by zzb for integration */
#ifdef INTEGRATION_TEST
        phy_subframe = NULL;
        pr_result = NULL;

        p_msg->my_type= mac_ul_de_id[0];

        if (wmrt_dequeue (mac_ul_de_id[0], p_msg, sizeof(struct queue_msg)) == -1)
        {
            FLOG_WARNING ("DEQUEUE ERROR in MAC layer\n");
        }

        //adapter_get_phy_subframe(p_msg->p_buf,&phy_subframe);
        adapter_transform_physical_frame(p_msg->p_buf, &phy_subframe, &pr_result);

        if ( (pr_result != NULL) && (phy_subframe != NULL) )
        {
            if (phy_subframe->interference_info != NULL)
            {
                send_peri_senssing_msg(phy_subframe->interference_info->is_active, pr_result);
            }
        }

        free_adapter_frame((void **)&(p_msg->p_buf));

        pthread_mutex_lock(&mutex_tx_phy_en_flag);
        ranging_count = ranging_en_flag;
        ranging_en_flag = 0;
        pthread_mutex_unlock(&mutex_tx_phy_en_flag);

        if (ranging_count != 0)
        {
//            FLOG_INFO("%d ranging result found!\n", ranging_count);

            p_msg->my_type= mac_ul_de_id[2];

            for (ranging_idx = 0; ranging_idx < ranging_count; ranging_idx ++)
            {
                if (wmrt_dequeue (mac_ul_de_id[2], p_msg, sizeof(struct queue_msg)) == -1)
                {
                    FLOG_WARNING ("DEQUEUE ERROR in MAC layer\n");
                }
#ifdef RANGING_ENABLED
                phy_dump_ranging_result(p_msg->p_buf);

                ret = convert_ranging_result(phy_subframe, p_msg->p_buf);
#endif
                if (ret != 0)
                {
//                    FLOG_WARNING ("Convert ranging result error!\n");
                }

                free(p_msg->p_buf);
            }
        }

/** for testing currently */
/*
        if (phy_subframe->interference_info != NULL)
        {
//            FLOG_INFO("got dts_info from PHY %p\n", phy_subframe->interference_info);
            free(phy_subframe->interference_info);

            phy_subframe->interference_info = NULL;
        }
*/
/*
        if (phy_subframe->p_ranging_info != NULL)
        {
            free_ranging_info(phy_subframe->p_ranging_info);
        }

        release_subframe(phy_subframe);


        phy_subframe = NULL;
*/
#else
        dequeue_subframe(ul_subframeq, &phy_subframe);
#endif
/** end by zzb for integration */

        if (phy_subframe!= NULL){

#ifndef SS_TX

/** add by zzb for integration */

//Below is the code that should be enabled when MAC-PHY integration testing.  (The block is enclosed in ********)
//*******************BELOW IS THE CODE FOR READING INTERFERENCE INFO IN REAL MAC-PHY*********************************************	

#ifdef INTEGRATION_TEST
            pthread_mutex_lock(&int_info_lock);

            if (phy_subframe->interference_info != NULL)
            {
                gp_int_info = phy_subframe->interference_info;
                interference_flag = 1;
                FLOG_DEBUG("update interference_info");
            }
            else
            {
                gp_int_info = NULL;
                interference_flag = 0;
            }

            pthread_mutex_unlock(&int_info_lock);
#endif

//*******************************************************************************************************************************
//END OF CODE BLOCK TO BE ENABLED FOR MAC-PHY INTEGRATION.

//Disable the block below when MAC-PHY integration testing.
//*******************FOR MAC ONLY TESTING, IM USING THIS DUMMY GENERATION********************************************************
#ifndef INTEGRATION_TEST
            pthread_mutex_lock (&int_info_lock);

            if (phy_subframe->sense_flag == 1)
            {
                gp_int_info = &int_info;
                interference_flag = 1;
            }
            else
            {
                gp_int_info = NULL;
                interference_flag = 0;
            }

            pthread_mutex_unlock (&int_info_lock);
#endif
//******************************************************************************************************************************
//END OF MAC-only Testing Code Block

/** end by zzb for integration */

#endif
#ifdef SS_RX
            /** add by zzb for integration */
#if 0
            /** deal with the FCH and DL_MAP if nessuary */
            if (phy_subframe->fch_dl_map == NULL)
            {
                //ERROR_TRACE("No FCH and DL_MAP found!");
            }
            else
            {
                /* free it temporary */
                free (phy_subframe->fch_dl_map);
            }
#endif
            /** end by zzb for integration */
			#ifdef SS_TX
			if(phy_subframe->p_ranging_info != NULL)
			{
				#ifdef RANGING_TEST
					if(test_flag == 0)
					{
						// Test 1: Simulate the indication from PHY to start MAC ranging module
                        phy_subframe->p_ranging_info->mac_ranging_start_flag = START_INIT_RANGING;
                        FLOG_INFO("Starting SS Test 1: Set the mac_ranging_start_flag from PHY\n");
                        test_flag = 1;
                    }
                #endif
                // If UL synch established, PHY will set this flag. Start MAC Nw entry
                if ((ranging_type != INIT_RANGING) && \
                    (phy_subframe->p_ranging_info->mac_ranging_start_flag == START_INIT_RANGING))
                {
                    ret = pthread_create(&ss_ir_thread, NULL, ss_init_ranging, NULL);
                    if(ret)
                        FLOG_FATAL("Cannot create SS Ranging Thread\n");
                    else
                    {
                        FLOG_INFO("Started SS Ranging Thread\n");
                        ranging_type = INIT_RANGING;
                    }
                }
                else if (phy_subframe->p_ranging_info->mac_ranging_start_flag == ADJUST_FAILED)
                {
                    sll_fifo_q_enqueue(ranging_q, NULL, 0, ADJUST_FAILED);
                }
				if (phy_subframe->p_ranging_info == NULL)
        {
            FLOG_ERROR("No ranginfo (null)\n");
        }
				my_basic_cid = phy_subframe->p_ranging_info->basic_cid;
				// Discard and Free the messages. 
				// Vestige of SS loopback testing, not relevant at SS-RX
				free_ranging_info(phy_subframe->p_ranging_info);
				phy_subframe->p_ranging_info = NULL;
			}
			if(ranging_type == START_PERIODIC_RANGING)
			{
				ret = pthread_create(&ss_pr_thread, NULL, ss_periodic_ranging, NULL);
				if(ret)
					FLOG_FATAL("Cannot create SS Periodic Ranging Thread\n");
				else
					FLOG_INFO("Started SS Periodic Ranging Thread\n");
				ranging_type = PERIODIC_RANGING;
#ifdef __ENCRYPT__
				
				bs_ss_info* ss_info = ssinfo_list_head;
				my_basic_cid = ss_info->basic_cid;
				int mm_macadd = get_macaddr_from_basic_cid(my_basic_cid);
				ss_info = find_bs_ss_info(mm_macadd);
				if (ss_info == NULL) 
				{
					FLOG_FATAL("Could not find SS Info\n");exit(-1);
				}
				else
				{
					FLOG_DEBUG("Beginning Authorization for bcid %d macAddr %d\n",ss_info->basic_cid, ss_info->mac_addr);
				}
				pkm_ret = ss_init_authorization(ss_info);
				if (pkm_ret != 0)
				{
					FLOG_INFO("SS Initial Authorization Failed\n");
				}
#endif
			}
			#endif
#else // BS_RX
            /** add by zzb for integration */
#if 0
            if (phy_subframe->fch_dl_map != NULL)
            {
                // There should be no FCH/DLMAP in the UL subframe. Discard
                free (phy_subframe->fch_dl_map);
            }
#endif
            /** end by zzb for integration */
			#ifndef SS_TX
			if (phy_subframe->p_ranging_info != NULL)
			{
				if (phy_subframe->p_ranging_info->num_ranging_adjust_ie > 0)
				{
					FLOG_DEBUG("Enqueuing RANGING_ADJUST messages from PHY\n");
					
#ifndef BR_ENABLE
					sll_fifo_q_enqueue(bs_ranging_q, (void*)phy_subframe->p_ranging_info, sizeof(ranging_info), RANGING_ADJUST);
#else
					FLOG_DEBUG("ranging detected: %d\n", phy_subframe->p_ranging_info->p_ranging_ie_list->ranging_code);
					extern sll_fifo_q *mac_br_q;
					if ((phy_subframe->p_ranging_info->p_ranging_ie_list) && \
						(phy_subframe->p_ranging_info->p_ranging_ie_list->ranging_code >= \
								(RANGING_CODE_S + RANGING_CODE_N + RANGING_CODE_M)) && \
						(phy_subframe->p_ranging_info->p_ranging_ie_list->ranging_code < \
								(RANGING_CODE_S + RANGING_CODE_N + RANGING_CODE_M + RANGING_CODE_L)) && \
								(mac_br_q != NULL))
					{
						sll_fifo_q_enqueue(mac_br_q, (void*)phy_subframe->p_ranging_info, sizeof(ranging_info), RANGING_ADJUST);
					}
					else
					{
						sll_fifo_q_enqueue(bs_ranging_q, (void*)phy_subframe->p_ranging_info, sizeof(ranging_info), RANGING_ADJUST);
					}
#endif
				}
			}
			#endif
#endif

            initialize_pduframeq(&pdu_frameq, phy_subframe->frame_num);
            enqueue_pduq(ul_pduq_head, pdu_frameq);

			pthread_rwlock_rdlock(&conn_info_rw_lock);
            parse_frame_pdu(phy_subframe, pdu_frameq, brqlist, ul_msgq);


            // here could be implemented by another thread
            dequeue_pduq(ul_pduq_head, &pdu_frameq);

            reassembly(pdu_frameq, ul_sduq, fragq, brqlist, ul_msgq);
			pthread_rwlock_unlock(&conn_info_rw_lock);

            // now could release the memory of the unused pdu_cid_queue
            release_pduframeq(pdu_frameq);
            release_subframe(phy_subframe);
        }
    }
	FLOG_INFO("Terminated UL cps thread\n");
    decrement_sync_count();
	pthread_exit((void*)0);
    return NULL;
}


int ul_cps_controller(ul_br_queue *br_q_list)
{

    subframe_queue* ul_subframeq = NULL;
    frag_queue* fragq;
    sdu_queue* ul_sduq;
    pdu_queue* ul_pduq_header;
    mgt_msg_queue *ul_msgq;
    int ret;

    // get the uplink sduq
    get_sduq(&ul_sduq, 0);

#ifndef INTEGRATION_TEST
    // initialize the uplink physical subframe queue
    initialize_subframe_queue(&ul_subframeq, 0);
#endif
    // initialize the uplink pdu queue
    initialize_pduq(&ul_pduq_header);

    // initialize the fragment queue
    initialize_fragq(&fragq);

    // initialize the uplink management message queue
    ul_msgq = ul_mgt_msg_queue_init();
	FLOG_INFO("Initialized UL mm q\n");
    init_ul_con_threads();
    // start the upplink parsing thread to parse the mac pdu, sdu and management message
    ul_arg.fragq = fragq;
    ul_arg.ul_sduq = ul_sduq;
    ul_arg.ul_subframeq = ul_subframeq;
    ul_arg.pduq = ul_pduq_header;
    ul_arg.br_q_list = br_q_list;
    ul_arg.ul_msgq = ul_msgq;

    ret = pthread_create(&ul_parse_thread, NULL, ul_parsing, (void *) &ul_arg);
    if(ret)
    {
        FLOG_FATAL("Cannot create thread to start uplink parsing");
    }

    return 0;

}

int release_ul_cps_controller()
{
//    pthread_cancel (ul_parse_thread);
//    pthread_join (ul_parse_thread, NULL);

//    pthread_cancel(ul_parse_thread);
    free_ul_mm_queue(&ul_msg_queue);
#ifndef INTEGRATION_TEST
    release_subframe_queue(ul_arg.ul_subframeq, 0);
#endif
    release_pduq(ul_arg.pduq);
    release_fragq(ul_arg.fragq);
    return 0;
}

