/* ----------------------------------------------------------------------------
   IBM Confidential

   IBM Wireless M2M Platform   

   (C)Copyright IBM Corp. 2009, 2010, 2011

   All Rights Reserved.

   File Name: mac_ss_ranging.c

   Change Activity:

   Date             Description of Change                   By
   -----------      ---------------------					--------
   1-Jul.2009       Created                                 Parul Gupta

   ----------------------------------------------------------------------------
   PROLOG END TAG zYx                                                         */


#include <assert.h>
#include "dl_exp_params.h"
#include "mac_ss_ranging.h"
#include "mac_ss_ranging_utm.h"
#include "debug.h"

sll_fifo_q *ranging_q;
extern sdu_queue* dl_sdu_queue;
extern int num_ranging_retries;
extern int num_rng_opps_toskip;
extern int rng_backoff_window;
#ifdef SS_TX
#ifdef SS_RX
int ranging_code, ranging_symbol, ranging_subchannel;
static BOOL ranging_adjust_flag;
static app_timer *last_t3_timer;
static app_timer *last_t4_timer;
#endif
#endif
extern int8_t power_adjust;
extern int32_t timing_adjust, frequency_adjust;
extern int ms_ranging_flag;
u_int16_t my_basic_cid;
extern int param_MAX_VALID_BASIC_CID;

int cdma_allocation;
int cdma_uiuc;

#ifdef SS_TX
#ifdef SS_RX
int init_ss_nw_entry()
{
	FLOG_DEBUG("Initializing SS NE variables\n");
	num_ranging_retries = 0;
	ms_ranging_flag = DO_NOTHING;
	rng_backoff_window = RNG_BACKOFF_WMIN;
	num_rng_opps_toskip = 0;

	ranging_code = RANGING_CODE_S;
	ranging_symbol = 0; 
	ranging_subchannel = 0;
	power_adjust = 0;
	timing_adjust = 0; 
	frequency_adjust = 0;
	ranging_adjust_flag = FALSE;
	last_t3_timer = NULL;
	ranging_type = NO_RANGING;
	pthread_mutex_lock(&ranging_mutex);
	cdma_allocation = 0;
	cdma_uiuc = param_UL_MCS + 1;
	pthread_mutex_unlock(&ranging_mutex);

	// Flush all the ranging messages rcvd on previous channel
	flush_sll_fifo_q(ranging_q);

	// Flush the global timer list containing the ranging events
	dll_ordered_list_cleanup(g_timer_list);

	return 0;
}

// This function sets the ranging_info structure in phy_subframe for SS_TX
int set_rng_info(ranging_info *p_ranging_info)
{
	// These is not applicable for SS_TX, only applicable for SS_RX
	p_ranging_info->mac_ranging_start_flag = FALSE;

	// Initialize the other members of p_ranging_info
	p_ranging_info->p_rng_tx_info = NULL;
	p_ranging_info->p_ranging_ie_list = NULL;
	p_ranging_info->num_ranging_adjust_ie = 0; 
	p_ranging_info->ms_ranging_flag = ms_ranging_flag;

	switch(ms_ranging_flag)
	{
		case SEND_CDMA_CODE:
			// The logic below assumes there are fixed num of ranging opportunities 
			// every frame (our current config). In reality, each frane could have
			// a varying # of ranging opps, sometimes none. Extend later to check
			// ULMAP dynamically for Ranging IE and adjust accordingly
			if (ranging_type == INIT_RANGING)
			{
				num_rng_opps_toskip -= NUM_IR_OPPS_PERFRAME;
			}
			else if (ranging_type == PERIODIC_RANGING)
			{
				num_rng_opps_toskip -= NUM_PR_OPPS_PERFRAME;
			}

			if (num_rng_opps_toskip >= 0)
			{
				// Need to skip some more, do nothing now
				p_ranging_info->ms_ranging_flag = DO_NOTHING;
				FLOG_DEBUG("Ranging Backoff: Need to skip %d more ranging opps, do nothing now\n", num_rng_opps_toskip);
				return 1;
			}
			p_ranging_info->p_rng_tx_info = malloc(sizeof(ranging_tx_info));

			unsigned long long current_time, firing_time;
			// TODO: Currently the ranging slot (sym + subchan) assignment assumes 
			// only one ranging opportunity, so assignment is fixed, not random. 
			// Extend later. They will also depend on backoff values
			if (ranging_type == INIT_RANGING)
			{
				ranging_code = (RANGING_CODE_S + (int)(RANGING_CODE_N * (rand() / (RAND_MAX + 1.0)))) % NUM_RANGING_CODES;
				ranging_symbol = 0;
				ranging_subchannel = 0;
			}
			else if (ranging_type == PERIODIC_RANGING)
			{
				ranging_code = (RANGING_CODE_S + RANGING_CODE_N + (int)(RANGING_CODE_M * (rand() / (RAND_MAX + 1.0))))%NUM_RANGING_CODES;
				ranging_symbol = PR_REGION_START_SYMBOL;
				ranging_subchannel = 0;
			}
		
			// Power adjustment algo is currently handled by PHY. This value is unused
			p_ranging_info->p_rng_tx_info->power_adjust = 0;
			p_ranging_info->p_rng_tx_info->ranging_code = ranging_code;
			p_ranging_info->p_rng_tx_info->ranging_symbol = ranging_symbol;
			p_ranging_info->p_rng_tx_info->ranging_subchannel = ranging_subchannel;
			#ifdef RANGING_TEST
			RNG_TEST_TRACE("Sent signal to PHY to send CDMA code\n");
			#endif
			current_time = readtsc(); 
			firing_time = current_time + T3_RNG;
			num_ranging_retries++;
			// Add a timer T3 (from sending of a CDMA code, i.e. function called, till RNG_RSP received)
			app_timer_add(firing_time, &t3_expired, NULL, NULL, 0, 0, (void **)&last_t3_timer, NULL);
			//printf("Current Time: %lld, Added T3 timer expiring at: %lld\n", current_time, firing_time);
			break;
		case BASIC_CID_READY:
			p_ranging_info->basic_cid = my_basic_cid;
			ranging_type = START_PERIODIC_RANGING;
			break;
		case INIT_RNG_REQ:
			current_time = readtsc(); 
			firing_time = current_time + T3_RNG;
			// Add a timer T3 (from sending of a CDMA code, i.e. function called, till RNG_RSP received)
			app_timer_add(firing_time, &t3_expired, NULL, NULL, 0, 0, (void **)&last_t3_timer, NULL);
			//printf("Current Time: %lld, Added T3 timer expiring at: %lld\n", current_time, firing_time);
			break;
		case RESCAN:
			break;
		case DO_NOTHING:
			break;
		default:
			FLOG_ERROR("Unknown status in ms_ranging_flag\n");
	}

	if (ranging_adjust_flag == TRUE)
	{
		// Set the structure which will give adjustment values to PHY for SS_TX
		p_ranging_info->num_ranging_adjust_ie = 1;

		p_ranging_info->p_ranging_ie_list = malloc(sizeof(ranging_adjust_ie));

		p_ranging_info->p_ranging_ie_list->timing_adjust = timing_adjust; 
		p_ranging_info->p_ranging_ie_list->power_adjust = power_adjust;
		p_ranging_info->p_ranging_ie_list->frequency_adjust = frequency_adjust;
		// Other members are not applicable and will not be used
		p_ranging_info->p_ranging_ie_list->p_next = NULL;

		// Reset the values
		power_adjust = 0;
		timing_adjust = 0;
		frequency_adjust = 0;
		ranging_adjust_flag = FALSE;
	}

	// Reset the values
	ms_ranging_flag = DO_NOTHING;
	return 0;
}

// This function will be called by PHY when UL synch is established (in PHY) 
void* ss_init_ranging(void *args)
{
    if (pthread_setcanceltype (PTHREAD_CANCEL_DEFERRED, NULL) != 0)
    {
        return NULL;
    }
	unsigned long long firing_time = 0;
	q_container *q_cntr = NULL;
	rng_rsp_msg *rng_rsp = NULL;
	cdma_ranging_ie *cdma_rng_ie = NULL;
	cdma_alloc_ie *cdma_ie = NULL;
	u_char *payload = NULL;
	int mm_len = 0, ret = 0, ii = 0;
	u_int64_t mac_addr = 0;

	// This increments the num_ranging_retries & sets status to SEND_CDMA_CODE
	FLOG_DEBUG("Calling t4_expired from ss_init_ranging for the first time\n");
	ret = (*t4_expired)();
	// The first step is contention ranging. PHY will send CDMA codes and
	// SS MAC will listen for RNG-RSP from BS in the UL
	while(1)
	{
		FLOG_DEBUG("Waiting to dequeue from ranging_q\n");
		sll_fifo_q_dequeue_with_wait(ranging_q, &q_cntr);
		FLOG_DEBUG("In ranging_q, Dequeued msg of type %d\n", q_cntr->data_type);
		switch(q_cntr->data_type)
		{
			case RNG_RSP:
				parse_rng_rsp(q_cntr->data, q_cntr->len, &rng_rsp);

				// Default value
				ms_ranging_flag = DO_NOTHING;
				// Decode RNG-RSP and see if ranging code, slot matches mine
#ifdef _OFDMA_
				int ofdm_sym = 0, ofdm_subchan = 0, rng_code = 0, frame_num = 0; 
				if (rng_rsp->ranging_code_attributes.length != 0)
				{
					ofdm_sym = (rng_rsp->ranging_code_attributes.value & 0xFFC00000) >> 22; 
					ofdm_subchan = (rng_rsp->ranging_code_attributes.value & 0x3F0000) >> 16;
					rng_code = (rng_rsp->ranging_code_attributes.value & 0xFF00) >> 8;
					frame_num = rng_rsp->ranging_code_attributes.value & 0xFF;
					FLOG_DEBUG("Checking CDMA code attribs Rx vs mine. OFDM sym: %d vs %d, OFDM subchan: %d vs %d, rng_code: %d vs %d\n", ofdm_sym, ranging_symbol, ofdm_subchan, ranging_subchannel, rng_code, ranging_code);
					// Check if this RNG_RSP is meant for me
					if ((rng_code == ranging_code) &&  (ofdm_subchan == ranging_subchannel) && (ofdm_sym == ranging_symbol))
					{
						set_phy_corrections(rng_rsp);
						ranging_adjust_flag = TRUE;
					}
					else
						continue; // Not for me, go to the next msg in queue
				}
				else if (rng_rsp->ss_mac.length != 0) 
				{
					for (ii = 0; ii < 6; ii++)
					{
						if(rng_rsp->ss_mac.value[ii] != param_MY_MAC[ii])
							continue;
					}
#ifdef RANGING_TEST
					RNG_TEST_TRACE("Received RNG_RSP with matching MAC in SS ranging Q.\n");
#endif
					// set basic CID, prim CID if present and call PR
					set_phy_corrections(rng_rsp);
					ranging_adjust_flag = TRUE;

					// Ref: Sec 6.3.10.3.1 and Pg 339/407: If a valid basic_cid is
					// received, Init Ranging is over, Periodic ranging starts
					if (rng_rsp->basic_cid.length != 0)
					{
						// Call the functions to initialize connection params for the new CID
						NUM_SS = 1;
						for (ii = 0; ii < 6; ii++)
						{
							mac_addr += (param_MY_MAC[ii]<<(5-ii)*8);
						}

						pthread_rwlock_wrlock(&conn_info_rw_lock);

						param_MAX_VALID_BASIC_CID = rng_rsp->basic_cid.value;
						add_basic_con(param_MAX_VALID_BASIC_CID, NUM_SS-1, mac_addr);
						ms_ranging_flag = BASIC_CID_READY;
						my_basic_cid = param_MAX_VALID_BASIC_CID;
						
						assert(rng_rsp->primary_cid.length != 0);
						max_valid_primary_cid = rng_rsp->primary_cid.value;
						add_primary_con(max_valid_primary_cid, NUM_SS, mac_addr);
						pthread_rwlock_unlock(&conn_info_rw_lock);

						// Flush all the ranging messages rcvd on previous channel
						flush_sll_fifo_q(ranging_q);

						// Flush the global timer list containing the ranging events
						dll_ordered_list_cleanup(g_timer_list);
						free(rng_rsp);//If we are exiting thread, need to free this.
						free(q_cntr->data);
						free(q_cntr);
						q_cntr = NULL;
						pthread_exit(NULL);
					}
					else if (rng_rsp->ranging_status.value != RNG_ABORT)
					{
						FLOG_WARNING("Error: RNG_RSP with matching MAC doesn't contain basic/primary CIDs. Continuing Initial Ranging\n"); 
						if (last_t3_timer != NULL)
							app_timer_delete(last_t3_timer);
						last_t3_timer = NULL;
						(*t3_expired)();
						continue;
					}
				}
#endif
				if (rng_rsp->ranging_status.length == 0)
				{
					FLOG_WARNING("ERROR: Ranging status TLV not found in RNG_RSP message\n");
					assert(0);
				}
				else 
				{
					FLOG_DEBUG("Rcvd RNG_RSP with status: %d\n", rng_rsp->ranging_status.value);
					switch(rng_rsp->ranging_status.value)
					{
						case RNG_CONTINUE:
#ifdef RANGING_TEST 
							RNG_TEST_TRACE("Received RNG_RSP message with CONTINUE status\n");
#endif
							if (last_t3_timer != NULL)
								app_timer_delete(last_t3_timer);
							last_t3_timer = NULL;

							// Send immediately
							trigger_cdmacode_now();
							break;
						case RNG_ABORT:
#ifdef RANGING_TEST 
							RNG_TEST_TRACE("Received RNG_RSP message with ABORT status\n");
#endif
							// Reset all the Network entry variables and cleanup queues
							init_ss_nw_entry();
							ms_ranging_flag = RESCAN;
							free(rng_rsp);//If we are exiting thread, need to free this.
							free(q_cntr->data);
							free(q_cntr);
							q_cntr = NULL;
							pthread_exit(NULL);
							break;
						case RNG_SUCCESS: 
#ifdef RANGING_TEST
							RNG_TEST_TRACE("Received RNG_RSP message with SUCCESS status\n");
#endif
							// Flush all the ranging messages rcvd on previous channel
							//flush_sll_fifo_q(ranging_q);

							// Flush the global timer list containing the ranging events
							dll_ordered_list_cleanup(g_timer_list);

							// Now add a T3 timer for waiting for CDMA allocation IE
							// If the timer expires without receiving an allocation,
							// then wait for initial ranging opportunity again
							//num_ranging_retries = 0;
							firing_time = readtsc() + T3_RNG;
							app_timer_add(firing_time, &t3_expired, NULL, NULL, 0, 0, (void **)&last_t3_timer, NULL);
							break;
						default:
							FLOG_WARNING("ERROR in parse_rng_rsp: Unrecognized Ranging Status\n");
					}
				}
				free(rng_rsp);
				rng_rsp = NULL;
				break;
			case CDMA_ALLOC_UIUC:
				cdma_rng_ie = q_cntr->data;				 
				cdma_ie = cdma_rng_ie->cdma_ie;

				// Check if this CDMA Allocation IE is meant for me
				if ((cdma_ie->ranging_code == ranging_code) &&  \
					(cdma_ie->ranging_subchannel == ranging_subchannel) && \
					(cdma_ie->ranging_symbol == ranging_symbol))
				{				
					// Remove last T3. 
					if (last_t3_timer != NULL)
						app_timer_delete(last_t3_timer);
					last_t3_timer = NULL;
					firing_time = readtsc() + T3_RNG;
					// Add a new timer T3 for unicast RNG_RSP
					app_timer_add(firing_time, &t3_expired, NULL, NULL, 0, 0, (void **)&last_t3_timer, NULL);

#ifdef RANGING_TEST
					RNG_TEST_TRACE("Found a matching CDMA alloc IE. \n");
#endif
					// Set UIUC and duration to be used by ss_scheduler
					pthread_mutex_lock(&ranging_mutex);
					FLOG_DEBUG("Lock acquired in ranging thd. set values for SCH\n");
					/*
					cdma_allocation = cdma_ie->duration * \
					bits_per_car[get_mcs_from_uiuc(get_ucd_msg(cdma_rng_ie->ucd_count), cdma_ie->uiuc)]*DL_DATA_CAR_PER_SLOT/8;
					*/

					cdma_allocation = cdma_ie->duration * \
					                bits_per_car[cdma_ie->uiuc - 1]*DL_DATA_CAR_PER_SLOT/8;
					cdma_uiuc = cdma_ie->uiuc;
					pthread_mutex_unlock(&ranging_mutex);
					FLOG_DEBUG("Lock released in ranging thd. set values for SCH\n");

					build_rng_req(&payload, &mm_len, cdma_allocation);
					u_char *p_to_enq = mac_sdu_malloc(mm_len, MMM_CLASS_TYPE);
					memcpy(p_to_enq, payload, mm_len);
					free(payload);
					payload = NULL;

					enqueue_transport_sdu_queue(dl_sdu_queue, INIT_RNG_CID, mm_len, p_to_enq);
					ms_ranging_flag = INIT_RNG_REQ;
#ifdef RANGING_TEST
					RNG_TEST_TRACE("SS enq'ed a RNG_REQ on INIT_RNG_CID\n");
#endif
				}
				free(cdma_ie);
				break;
			case ADJUST_FAILED:
				// Reset all the Network entry variables and cleanup queues
				init_ss_nw_entry();
				ms_ranging_flag = RESCAN;
				free(q_cntr);
				pthread_exit(NULL);
				break;
			default:
				FLOG_WARNING("ERROR: Unrecognized Type in Ranging Queue\n");
		}
		free(q_cntr->data);
		free(q_cntr);
		q_cntr = NULL;
	}
	return NULL;
}

int init_ss_periodic_rng()
{
	num_ranging_retries = 0;
	ms_ranging_flag = DO_NOTHING;
	rng_backoff_window = RNG_BACKOFF_WMIN;
	num_rng_opps_toskip = 0;

	ranging_code = RANGING_CODE_S + RANGING_CODE_N;
	ranging_symbol = PR_REGION_START_SYMBOL; 
	ranging_subchannel = 0;

	power_adjust = 0;
	timing_adjust = 0; 
	frequency_adjust = 0;
	ranging_adjust_flag = FALSE;
	last_t3_timer = NULL;
	last_t4_timer = NULL;
	ranging_type = PERIODIC_RANGING;

	// Flush all the ranging messages rcvd on previous channel
	flush_sll_fifo_q(ranging_q);

	// Flush the global timer list containing the ranging events
	dll_ordered_list_cleanup(g_timer_list);
#ifdef RANGING_TEST
	RNG_TEST_TRACE("Initialized or reset Periodic ranging variables\n");
#endif

	return 0;
}

void* ss_periodic_ranging(void *args)
{
    if (pthread_setcanceltype (PTHREAD_CANCEL_DEFERRED, NULL) != 0)
    {
        return NULL;
    }
	unsigned long long firing_time = 0;
	q_container *q_cntr = NULL;
	rng_rsp_msg *rng_rsp = NULL;

	// This function will clear earlier data and initialize new. 
	// Works for reset also
#ifdef RANGING_TEST	
	RNG_TEST_TRACE("Started Periodic ranging thread\n");
#endif
	init_ss_periodic_rng();

	// This increments the num_ranging_retries & sets status to SEND_CDMA_CODE
	(*t4_expired)();
	// The first step is contention ranging. PHY will send CDMA codes and
	// SS MAC will listen for RNG-RSP from BS in the UL
	while(1)
	{
		FLOG_DEBUG("Waiting to dequeue from ranging_q in periodic_ranging\n");
		sll_fifo_q_dequeue_with_wait(ranging_q, &q_cntr);
		switch(q_cntr->data_type)
		{
			case RNG_RSP:
				FLOG_DEBUG("Received RNG_RSP in periodic_ranging_thread\n");
				parse_rng_rsp(q_cntr->data, q_cntr->len, &rng_rsp);
				// Decode RNG-RSP and see if ranging code, slot matches mine
#ifdef _OFDMA_
				int ofdm_sym = 0, ofdm_subchan = 0, rng_code = 0, frame_num = 0; 
				if (rng_rsp->ranging_code_attributes.length != 0)
				{
					ofdm_sym = (rng_rsp->ranging_code_attributes.value & 0xFFC00000) >> 22; 
					ofdm_subchan = (rng_rsp->ranging_code_attributes.value & 0x3F0000) >> 16;
					rng_code = (rng_rsp->ranging_code_attributes.value & 0xFF00) >> 8;
					frame_num = rng_rsp->ranging_code_attributes.value & 0xFF;
					// Check if this RNG_RSP is meant for me
					if ((rng_code == ranging_code) &&  (ofdm_subchan == ranging_subchannel) && (ofdm_sym == ranging_symbol))
					{
						set_phy_corrections(rng_rsp);
						ranging_adjust_flag = TRUE;
					}
					else
					{
						//FLOG_WARNING("Ranging code attributes don't match. code:%d, subchan:%d, symbol:%d\n", rng_code, ofdm_subchan, ofdm_sym);
						continue; // Not for me, go to the next msg in queue
					}
				}
				else 
				{
					FLOG_ERROR("Ranging code attributes TLV not found\n");
					continue; // Not for me, go to the next msg in queue
				}
#endif
				if (rng_rsp->ranging_status.length == 0)
				{
					FLOG_WARNING("ERROR: Ranging status TLV not found in RNG_RSP message\n");
				}
				else 
				{
					switch(rng_rsp->ranging_status.value)
					{
						case RNG_CONTINUE: 
							if (last_t3_timer != NULL)
								app_timer_delete(last_t3_timer);
							last_t3_timer = NULL;
							trigger_cdmacode_now();
							break;
						case RNG_ABORT: 
							// Reset all the Network entry variables and cleanup queues
							init_ss_nw_entry();
							ms_ranging_flag = RESCAN;
							free(rng_rsp);
							free(q_cntr->data);
							free(q_cntr);
							pthread_exit(NULL);
							break;
						case RNG_SUCCESS:
#ifdef RANGING_TEST
							RNG_TEST_TRACE("Received RNG_RSP with SUCCESS status\n");
#endif
							// PR continues. Flush timers, restart T4, reset all values
							init_ss_periodic_rng();
							firing_time = readtsc() + T4_RNG;
							// Add a timer T4
							app_timer_add(firing_time, &t4_expired, NULL, NULL, 0, 0, (void **)&last_t4_timer, NULL);
							break;
						default:
							FLOG_WARNING("ERROR in parse_rng_rsp: Unrecognized Ranging Status\n");
					}
				}
				free(rng_rsp);
				rng_rsp = NULL;
				break;
			case ADJUST_FAILED:
				// Reset all the Network entry variables and cleanup queues
				init_ss_nw_entry();
				ms_ranging_flag = RESCAN;
				free(q_cntr);
				pthread_exit(NULL);
				break;
			default:
				FLOG_WARNING("ERROR: Unrecognized Type in Ranging Queue\n");
		}
		free(q_cntr->data);
		free(q_cntr);
		q_cntr = NULL;
	}
	return NULL;
}

int t4_expired()
{
	num_ranging_retries = 0; // send 1st PR code CDMA 
	ms_ranging_flag = SEND_CDMA_CODE;
	rng_backoff_window = RNG_BACKOFF_WMIN;
	num_rng_opps_toskip = 0;
	return 0;
}
int t3_expired()
{

	if (num_ranging_retries >= CONTENTION_RANGING_RETRIES) 
	{
		ms_ranging_flag = RESCAN;
		release_ss_ranging_sm();
		// Reset all the Network entry variables and cleanup queues
		init_ss_nw_entry();
		return -1;
	}
	else
	{
		ms_ranging_flag = SEND_CDMA_CODE;
		num_rng_opps_toskip = rng_backoff_window * (rand()/(RAND_MAX + 1.0));
		FLOG_DEBUG("T3 expired. rng_backoff_window: %d, opps2skip: %d\n", rng_backoff_window, num_rng_opps_toskip);

		//Double the backoff window for next time 
		if (rng_backoff_window < RNG_BACKOFF_WMAX)
		{
			rng_backoff_window *= 2;
		}
	}
	return 0;
}

// This performs the same function as t3_expired, without the backoff
int trigger_cdmacode_now()
{
	if (num_ranging_retries >= CONTENTION_RANGING_RETRIES)
	{
		ms_ranging_flag = RESCAN;
		release_ss_ranging_sm();
		// Reset all the Network entry variables and cleanup queues
		init_ss_nw_entry();
		return -1;
	}
	else
	{
		ms_ranging_flag = SEND_CDMA_CODE;
		num_rng_opps_toskip = 0;
	}
	
	return 0;
}

int set_phy_corrections(rng_rsp_msg *rng_rsp)
{
	if (rng_rsp->power_adjust.length != 0)
		power_adjust = rng_rsp->power_adjust.value;
	if (rng_rsp->timing_adjust.length != 0)
		timing_adjust = rng_rsp->timing_adjust.value;
	if (rng_rsp->frequency_adjust.length != 0)
		frequency_adjust = rng_rsp->frequency_adjust.value;
	
	return 0;
}
#endif
#endif
