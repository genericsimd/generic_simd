/* ----------------------------------------------------------------------------
 IBM Confidential

 IBM Wireless M2M Platform

 (C)Copyright IBM Corp. 2009, 2010, 2011

 All Rights Reserved.

 File Name: mac_bs_ranging.c

 Change Activity:

 Date             Description of Change                   By
 -----------      ---------------------					--------
 1-Jul.2009       Created                                 Parul Gupta

 ----------------------------------------------------------------------------
 PROLOG END TAG zYx                                                         */

#include <stdio.h>

#include "bs_ss_info.h"

#include  "mac_bs_ranging.h"
#include "debug.h"

#include "hash_table.h"
#include "cs_proc.h"
#include "addr_con_inter.h"

#include "bs_cfg.h"

#include "metric_proc.h"

#include "mac_serviceflow.h"
#include "dl_exp_params.h"

sll_fifo_q *bs_ranging_q;
extern sll_fifo_q *ranging_scheduler_q;
extern sdu_queue* dl_sdu_queue;

extern int param_MAX_VALID_UGS_PER_SS;
extern int param_MAX_VALID_BE_PER_SS;
extern int param_TX_UGS_CID_OFFSET;
extern int param_TX_BE_CID_OFFSET;

static void add_cons_for_ss
(
    u_int64_t mac_addr,
    char *mac_string,
    rng_req_msg *rng_req,
    rng_rsp_msg *rng_rsp,
    u_int8_t re_enter
)
{
    int ssnum;
    int tcid;
#ifdef USE_CPE_SF_CFG
    serviceflow *sflow = NULL;
    int cbr_size = 0;
    int cbr_delay = 0;
    int dl_mcs = 0;
    int ul_mcs = 0;
#endif
    int err_no;

    assert(mac_string != NULL);
    assert(rng_req != NULL);

    // Flow/Connection info element initializations. Under lock
    pthread_rwlock_wrlock (&conn_info_rw_lock);

    if (re_enter == 0)
    {
        rng_rsp->basic_cid.value = param_MAX_VALID_BASIC_CID + 1;
        rng_rsp->primary_cid.value = max_valid_primary_cid + 1;
    }

    ssnum = rng_rsp->basic_cid.value - BASIC_CID_MIN_VALUE;

    if (re_enter == 0)
    {
        add_basic_con (rng_rsp->basic_cid.value, ssnum, mac_addr);
        add_primary_con (rng_rsp->primary_cid.value, ssnum, mac_addr);

        FLOG_INFO("One CPE Enter the network, MAC addr %s, bcid %d, pcid %d\n",
                  mac_string, rng_rsp->basic_cid.value, rng_rsp->primary_cid.value);

        FLOG_INFO("Total ss in system: %d\n", ssnum + 1);
    }

#ifdef USE_CPE_SF_CFG
    dl_mcs = rng_req->ho_id.value & 0x0f;
    ul_mcs = ( rng_req->ho_id.value & 0xf0 ) >> 4;
    set_ss_dl_mcs(ssnum, dl_mcs);
    set_ss_ul_mcs(ssnum, ul_mcs);
#endif

    //tcid = (UGS_CID_MIN_VALUE) + (2 * (ssnum - 1) );
    tcid = ( UGS_CID_MIN_VALUE ) + ( 1 * ( ssnum ) );

#ifdef USE_CPE_SF_CFG
    sflow = dl_serviceflow_init_simple(tcid, 0, tcid,  SERVICE_UGS, DL, NULL);

    cbr_size = (unsigned short)((rng_req->serving_bs_id.value[3] << 8) + rng_req->serving_bs_id.value[4]);
    cbr_delay = (unsigned char)(rng_req->serving_bs_id.value[5]);

    sflow->max_sustained_traffic_rate = (cbr_size * 1000)/(cbr_delay);
    sflow->min_reserved_traffic_rate = (cbr_size * 1000)/(cbr_delay);
    sflow->sdu_inter_arrival_interval = cbr_delay;
#ifdef ARQ_ENABLED
    add_transport_con (tcid, mac_addr, 1, sflow, SERVICE_UGS, DL);
#else
    add_transport_con (tcid, mac_addr, 0, sflow, SERVICE_UGS, DL);
#endif
    FLOG_INFO("One DL connection cid %d, cbr_size %d, MCS %d\n",
                   tcid, cbr_size, dl_mcs);
#else
#ifdef DSX_ENABLE
    #ifdef DSA_SPONSOR
	#ifdef ARQ_ENABLED
        	add_transport_con (tcid, mac_addr, 1, NULL, SERVICE_UGS, DL);
	#else
        	add_transport_con (tcid, mac_addr, 0, NULL, SERVICE_UGS, DL);
	#endif
    #else
    #endif
#else
	#ifdef ARQ_ENABLED
		add_transport_con (tcid, mac_addr, 1, NULL, SERVICE_UGS, DL);
	#else
		add_transport_con (tcid, mac_addr, 0, NULL, SERVICE_UGS, DL);
	#endif
#endif
#endif


	//add_transport_con(tcid + 1, mac_addr, 0, NULL, SERVICE_UGS, DL);

#ifndef MULTIPLE_CPE
    sprintf (mac_string, "%d", param_MAX_VALID_BASIC_CID);
#endif
    err_no = insert_addr_con (gp_hash_table, mac_string, tcid);

    if (err_no == 3)
    {
        release_addr_con (gp_hash_table, mac_string);
	insert_addr_con (gp_hash_table, mac_string, tcid);
    }

    //tcid = (UL_UGS_CID_MIN_VALUE) + (2 * (ssnum - 1));
    tcid = ( UL_UGS_CID_MIN_VALUE ) + ( 1 * ( ssnum ) );

#ifdef USE_CPE_SF_CFG
    sflow = dl_serviceflow_init_simple(tcid, 0, tcid,  SERVICE_UGS, UL, NULL);

    cbr_size = (unsigned short)((rng_req->serving_bs_id.value[0] << 8) + rng_req->serving_bs_id.value[1]);
    cbr_delay = (unsigned char)(rng_req->serving_bs_id.value[2]);

    sflow->max_sustained_traffic_rate = (cbr_size * 1000)/(cbr_delay);
    sflow->min_reserved_traffic_rate = (cbr_size * 1000)/(cbr_delay);
    sflow->sdu_inter_arrival_interval = cbr_delay;
#ifdef ARQ_ENABLED
    add_transport_con (tcid, mac_addr, 1, sflow, SERVICE_UGS, UL);
#else
    add_transport_con (tcid, mac_addr, 0, sflow, SERVICE_UGS, UL);
#endif
    FLOG_INFO("One UL commection cid %d, cbr_size %d, MCS %d\n",
                   tcid, cbr_size, ul_mcs);
#else
#ifdef DSX_ENABLE
    #ifdef DSA_SPONSOR
		#ifdef ARQ_ENABLED
        		add_transport_con (tcid, mac_addr, 1, NULL, SERVICE_UGS, UL);    
		#else
        		add_transport_con (tcid, mac_addr, 0, NULL, SERVICE_UGS, UL);    
		#endif
    #else
    #endif
#else
	#ifdef ARQ_ENABLED
		add_transport_con (tcid, mac_addr, 1, NULL, SERVICE_UGS, UL);
	#else
		add_transport_con (tcid, mac_addr, 0, NULL, SERVICE_UGS, UL);
	#endif
#endif
#endif

    if (re_enter == 0)
    {
        param_MAX_VALID_BASIC_CID += 1;
        max_valid_primary_cid += 1;
        NUM_SS += 1;

        max_valid_ugs_cid += 1;
        max_valid_ul_ugs_cid += 1;

        ul_total_connection ++;
        dl_total_connection ++;
        total_wmb_in_system ++;
    }

    pthread_rwlock_unlock (&conn_info_rw_lock);
}

#ifndef SS_TX
#ifndef SS_RX
extern void release_ss_sfs(bs_ss_info *ss_info);
void* bs_init_ranging (void *args)
{
    if (pthread_setcanceltype (PTHREAD_CANCEL_DEFERRED, NULL) != 0)
    {
        return NULL;
    }

    int ii = 0, kk = 0;
    q_container *q_cntr = NULL;
    ranging_info *p_rng_info = NULL;
    ranging_adjust_ie *p_rng_adjust = NULL;
    //ranging_adjust_ie *p_rng_adjust = NULL;//, *p_ie_tmp = NULL;
    rng_req_msg *rng_req = NULL;
    rng_rsp_msg* rng_rsp = NULL;
    u_int64_t mac_addr = 0;
    char tmp_string[128];
    bs_ss_info * tmp_ss_info;

    set_global_param ("RangingSuccessCount",
            (void *) & ( ranging_success_count ));

    static int re_entry_count[256];
    memset (re_entry_count, 0, 256 * sizeof(int));

    while (1)
    {
        u_char *payload = NULL;
        int mm_len = 0;

        // The first step is contention ranging. PHY will send CDMA codes and
        // SS MAC will listen for RNG-RSP from BS in the UL
        sll_fifo_q_dequeue_with_wait (bs_ranging_q, &q_cntr);
        switch (q_cntr->data_type)
        {
            case RANGING_ADJUST:
                // Here the PHY, on detecting a CDMA code (without collisions)
                // and computing its correction values, passes it to MAC to build
                // and send the RNG-RSP.
                p_rng_info = q_cntr->data;
                rng_rsp = (rng_rsp_msg*) malloc (sizeof(rng_rsp_msg));
                for (ii = 0; ii < p_rng_info->num_ranging_adjust_ie; ii++)
                {
                    // Delink one ranging_adjust node from the top of the list
                    p_rng_adjust = p_rng_info->p_ranging_ie_list;
                    assert (p_rng_adjust != NULL);
                    p_rng_info->p_ranging_ie_list = p_rng_adjust->p_next;
                    p_rng_adjust->p_next = NULL;

                    // populate fields depending on the msg enqueued by PHY. Esp status
                    // according to the correction values sent
                    init_rng_rsp_msg (rng_rsp, p_rng_adjust);

                    if ( ( p_rng_adjust->ranging_code >= RANGING_CODE_S )
                            && ( p_rng_adjust->ranging_code < ( RANGING_CODE_S
                                    + RANGING_CODE_N ) ))
                    {
                        // If the received ranging code is from IR subset
                        if (rng_rsp->ranging_status.value == RNG_SUCCESS)
                        {
                            // UL scheduler will add a CDMA allocatioe IE for this
                            sll_fifo_q_enqueue (ranging_scheduler_q,
                                    p_rng_adjust, sizeof(ranging_adjust_ie),
                                    RANGING_ADJUST);
                        }
                        else
                        {
                            // Sending the RNG_RSP with SUCCESS status is optional. Send
                            // CDMA_alloc_IE only for Success, RNG_RSP for others
                            // The build_rng_rsp function parses the TLV values to be included in
                            // this RNG_RSP message and makes a byte payload for enqueuing in
                            // INIT_RNG_CID sdu queue
                            build_rng_rsp (&payload, &mm_len, rng_rsp);
                            u_char* p_to_enq = mac_sdu_malloc (mm_len,
                                    MMM_CLASS_TYPE);
                            memcpy (p_to_enq, payload, mm_len);
                            enqueue_transport_sdu_queue (dl_sdu_queue,
                                    INIT_RNG_CID, mm_len, p_to_enq);
                            free (payload);
                            payload = NULL;

                            free (p_rng_adjust);
                        }
                    }
                    else
                        if ( ( p_rng_adjust->ranging_code >= RANGING_CODE_S
                                + RANGING_CODE_N )
                                && ( p_rng_adjust->ranging_code
                                        < RANGING_CODE_S + RANGING_CODE_N
                                                + RANGING_CODE_M ))
                        {
                            // If the received ranging code is from PR subset
                            build_rng_rsp (&payload, &mm_len, rng_rsp);
                            u_char* p_to_enq = mac_sdu_malloc (mm_len,
                                    MMM_CLASS_TYPE);
                            memcpy (p_to_enq, payload, mm_len);
                            enqueue_transport_sdu_queue (dl_sdu_queue,
                                    INIT_RNG_CID, mm_len, p_to_enq);
                            free (payload);
                            payload = NULL;
                            free (p_rng_adjust);
                        }
                        else
                        {
                            FLOG_ERROR ("Unsupported Ranging Code in BS\n");
                        }
                }
                free (rng_rsp);
                if (p_rng_info->p_ranging_ie_list != NULL)
                {
                    FLOG_WARNING (
                            "Warning in mac_bs_ranging.c: Ptr at the end of ranging_adjust list is not NULL\n");
                }
                free_ranging_info (p_rng_info);
                break;
            case RNG_REQ_IR_CID:
                FLOG_DEBUG (
                        "Received RNG_REQ on INIT_RNG CID in the BS ranging thread\n");
                parse_rng_req (q_cntr->data, q_cntr->len, &rng_req);
                if (rng_req->ss_mac.length != 6)
                {
                    // This is not a valid SS MAC ID (6 bytes)
                    FLOG_WARNING (
                            "ERROR: Invalid SS MAC ID in RNG_REQ. Ignoring\n");
                    continue;
                }

                rng_rsp = (rng_rsp_msg*) malloc (sizeof(rng_rsp_msg));
                init_rng_rsp_msg (rng_rsp, NULL);

                rng_rsp->ss_mac.length = 6;

                mac_addr = 0;

                for (kk = 0; kk < 6; kk++)
                {
                    rng_rsp->ss_mac.value[kk] = rng_req->ss_mac.value[kk];
                    mac_addr += ( rng_req->ss_mac.value[kk] << ( 5 - kk ) * 8 );
                }
                rng_rsp->basic_cid.length = 2;
                rng_rsp->primary_cid.length = 2;

                memset (tmp_string, 0, 128);

                sprintf (tmp_string, "%02x:%02x:%02x:%02x:%02x:%02x",
                        rng_req->ss_mac.value[0], rng_req->ss_mac.value[1],
                        rng_req->ss_mac.value[2], rng_req->ss_mac.value[3],
                        rng_req->ss_mac.value[4], rng_req->ss_mac.value[5]);

                send_network_entry_msg (tmp_string);

                tmp_ss_info = find_bs_ss_info (mac_addr);

		u_int8_t re_enter;
                if (tmp_ss_info != NULL)
                {
                    FLOG_INFO (
                            "CPE re-enter the network, MAC addr %s, basic CID %d, primary CID %d, count %d\n",
                            tmp_string, tmp_ss_info->basic_cid,
                            tmp_ss_info->primary_cid,
                            ++re_entry_count[tmp_ss_info->basic_cid]);
		    FLOG_INFO("will release all the service flows of the ss\n");
                    release_ss_sfs(tmp_ss_info);

                    rng_rsp->basic_cid.value = tmp_ss_info->basic_cid;
                    rng_rsp->primary_cid.value = tmp_ss_info->primary_cid;

		    re_enter = 1;
                    add_cons_for_ss(mac_addr, tmp_string, rng_req, rng_rsp, re_enter);
                }
                else
                {
		    re_enter = 0;
                    add_cons_for_ss(mac_addr, tmp_string, rng_req, rng_rsp, re_enter);
                }
#ifdef AMC_ENABLE
		u_int16_t	assigned_basic_cid = rng_rsp->basic_cid.value;
		u_int16_t	assigned_primary_cid = rng_rsp->primary_cid.value;
		FLOG_DEBUG("++++++++ [%d]th ms: basic cid = %d, primary cid = %d ++++++++\n", assigned_basic_cid - BASIC_CID_MIN_VALUE, \
										assigned_basic_cid, assigned_primary_cid);
#endif
                ranging_success_count++;

                // Currently, there isn't a good way to measure and communicate
                // time/freq/pwr adjustments for RNG_REQs on INIT_RNG_CID
                // Since no corrections are sent with this, default status is continue
                rng_rsp->ranging_status.length = 1;
                rng_rsp->ranging_status.value = RNG_CONTINUE;

                build_rng_rsp (&payload, &mm_len, rng_rsp);
                u_char* p_to_enq = mac_sdu_malloc (mm_len, MMM_CLASS_TYPE);
                memcpy (p_to_enq, payload, mm_len);
                enqueue_transport_sdu_queue (dl_sdu_queue, INIT_RNG_CID,
                        mm_len, p_to_enq);
                free (payload);
                payload = NULL;
                FLOG_DEBUG (
                        "Enqueued RNG_RSP with Basic anad primary CID on the INIT_RNG CID\n");
                free (rng_rsp);
                free (rng_req);
                free (q_cntr->data);

#ifdef AMC_ENABLE
#ifndef SS_TX
//Is this the right place to add REP REQ
		if (re_enter == 0)
		{
			long long int current_time;long long int firing_time; struct timeval temp_time;
			gettimeofday(&temp_time, NULL);
			current_time = temp_time.tv_sec*1000000L + temp_time.tv_usec;
			firing_time = current_time + REP_REQ_PERIOD;
			app_timer_add(firing_time,&transmit_rep_req, NULL, NULL, 0,0, \
						(void**)(&rep_req_timer[assigned_basic_cid - BASIC_CID_MIN_VALUE]), \
						(void*)(assigned_primary_cid));
		}
#endif
#endif
                break;
            default:
                FLOG_ERROR (
                        "ERROR: Unrecognized data type in BS Ranging Queue\n");
        }
        free (q_cntr);
        q_cntr = NULL;
    }
    return NULL;
}
#endif
#endif
