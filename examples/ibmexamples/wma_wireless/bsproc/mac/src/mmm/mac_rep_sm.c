/* ----------------------------------------------------------------------------
   IBM Confidential

   IBM Wireless M2M Platform   

   (C)Copyright IBM Corp. 2011

   All Rights Reserved.

   File Name: mac_sf_sm.c

   Change Activity:

   Date             Description of Change                   By
   -----------      ---------------------					--------
   1-Mar.2011       Created                                 Parul Gupta

   ----------------------------------------------------------------------------
   PROLOG END TAG zYx                                                         */

#include "mac_amc.h"
#include "debug.h"
#include "thread_sync.h"
#include "mac_acm_coding.h"
#include "memmgmt.h"

int rep_req_retry(void* arg)
{
	int my_primary_cid = (int)((size_t)(arg));
	int my_basic_cid;
	long long int current_time; long long int firing_time; struct timeval temp_time;
	acm_rep_req_t acm_req;
	int success;
	u_char payload[MAX_REP_REQ_SIZE];int len;u_char* tx_payload;
	get_basic_cid(my_primary_cid, &my_basic_cid);

	/* timer has been timeout, clear the flag */
	rep_retry_timer[my_basic_cid - BASIC_CID_MIN_VALUE] = NULL;

	int index = my_basic_cid - BASIC_CID_MIN_VALUE;
	num_rep_retries_left[index]--;

        update_rep_cinr_adjust(my_basic_cid);

	FLOG_INFO("%s: num_rep_retries_left[%d] = %d\n", __FUNCTION__, index, num_rep_retries_left[index]);
	if (num_rep_retries_left[index] > 0)
	{
		//Resend REP REQ
		acm_req.report_type = INCLUDE_RSSI_REPORT|INCLUDE_CINR_REPORT|INCLUDE_BASIC_REPORT;
		success = build_rep_req(&acm_req, payload, &len);	
		if (success != 0)
		{
			FLOG_ERROR("Failed to build REP REQ in rep_req_retry\n");
			return -1;
		}
		tx_payload = (u_char*)mac_sdu_malloc(len, 5);
		memcpy(tx_payload, payload, len);
		FLOG_DEBUG("Re-enqueueing REP REQ\n");
		enqueue_transport_sdu_queue(dl_sdu_queue, my_primary_cid, len, tx_payload);
		//Add retry timer again
		gettimeofday(&temp_time, NULL);
		current_time = temp_time.tv_sec*1000000L + temp_time.tv_usec;
		firing_time = current_time + REP_REQ_RETRY_TIMEOUT;
		app_timer_add(firing_time,&rep_req_retry,NULL, NULL, 0,0, (void**)&(rep_retry_timer[my_basic_cid - BASIC_CID_MIN_VALUE]),(void *)((size_t)(my_primary_cid)));
	}
	else
	{
		FLOG_WARNING("No REP RSP received for CID %d. Retries exhausted\n",my_basic_cid);
		//Set another REP REQ after a REP_REQ_PERIOD
		rep_retry_timer[my_basic_cid - BASIC_CID_MIN_VALUE] = NULL;
		gettimeofday(&temp_time, NULL);
		current_time = temp_time.tv_sec*1000000L + temp_time.tv_usec;
		firing_time = current_time + REP_REQ_PERIOD;
		app_timer_add(firing_time, &transmit_rep_req, NULL, NULL, 0,0,(void**)&(rep_req_timer[my_basic_cid - BASIC_CID_MIN_VALUE]),(void *)((size_t)(my_primary_cid))); 
	}
	return 0;

}
int transmit_rep_req(void* arg)
{
	int my_primary_cid = (int)((size_t)(arg));
	int my_basic_cid;
	long long int current_time; long long int firing_time; struct timeval temp_time;
	get_basic_cid(my_primary_cid, &my_basic_cid);

	FLOG_DEBUG("%s: for basic cid %d\n", __FUNCTION__, my_basic_cid);
	u_char payload[MAX_REP_REQ_SIZE];u_char* tx_payload;int len;
	int success;
	acm_rep_req_t acm_req;
	acm_req.report_type = INCLUDE_RSSI_REPORT|INCLUDE_CINR_REPORT|INCLUDE_BASIC_REPORT;
	success = build_rep_req(&acm_req, payload, &len);
	if (success != 0)
	{
		FLOG_ERROR("Could not build REP-REQ in transmit_rep_req function\n");
		return -1;
	}
	tx_payload = (u_char*)mac_sdu_malloc(len, 5);
	memcpy(tx_payload, payload, len);
	//Enqueueing packet
	FLOG_DEBUG("Enqueueing REP REQ. Cid %d\n",my_primary_cid );
	enqueue_transport_sdu_queue(dl_sdu_queue, my_primary_cid, len, tx_payload);

	num_rep_retries_left[my_basic_cid - BASIC_CID_MIN_VALUE] = NUM_REP_REQ_RETRIES;
	//Set retry timer
	gettimeofday(&temp_time, NULL);
	current_time = temp_time.tv_sec*1000000L + temp_time.tv_usec;
	firing_time = current_time + REP_REQ_RETRY_TIMEOUT;
	app_timer_add(firing_time,&rep_req_retry,NULL, NULL, 0,0, (void**)&(rep_retry_timer[my_basic_cid - BASIC_CID_MIN_VALUE]), (void *)((size_t)(my_primary_cid)));
	return 0;
}


void* rep_state_machine()
{
    if (pthread_setcanceltype (PTHREAD_CANCEL_DEFERRED, NULL) != 0)
    {
        return NULL;
    }
	acm_rep_req_t *acm_req = NULL;
	acm_rep_rsp_t *acm_rsp = NULL;
	int len;	


	FLOG_DEBUG("++++++++ %s: starts to run ++++++++\n", __FUNCTION__);
	while(can_sync_continue())
	{
		// Dequeues with wait, if the queue is empty
		mgt_msg *mm = dequeue_ul_mgt_msg_queue(&ul_msg_queue[CHANNEL_REPORT_MMM_INDEX]);
		if (mm == NULL) {break;}
		FLOG_DEBUG("Received in ACM MSG SM, msg type: %d\n", mm->msg_type);
		int success;int my_basic_cid;
		long long int current_time; long long int firing_time;
		struct timeval temp_time;
		u_char payload[MAX_REP_RSP_SIZE];u_char* tx_payload;
		switch(mm->msg_type)
		{
			// All the MMM data types
			case REP_REQ:
				FLOG_INFO("Received REP REQ for CID %d\n",mm->cid);
				#ifndef SS_TX
					#ifdef INTEGRATION_TEST 
					FLOG_WARNING("Unexpected REP_REQ in BS mode\n");
					exit(-1);	
					#endif
				#endif
				
				acm_req = (acm_rep_req_t*)malloc(sizeof(acm_rep_req_t));				
				memset(acm_req, 0, sizeof(acm_rep_req_t));
				success = parse_rep_req(mm->data, mm->length, acm_req);
				if (success != 0)
				{
					FLOG_ERROR("Parse ACM REQ failed\n");
					free(mm->data);
					break;
				}	
				//Write in code to get CINR and create a ACM Resp
				get_basic_cid(mm->cid, &my_basic_cid);
				acm_rsp = (acm_rep_rsp_t*)malloc(sizeof(acm_rep_rsp_t));
				acm_rsp->start_frame = get_current_frame_number(); 
				acm_rsp->duration = 0; 
				acm_rsp->basic_report = 0;

				//Read params from shared tables
				pthread_mutex_lock(&cinr_table_lock);
#ifdef SS_TX
				  acm_rsp->cinr_report =round(average_cinr_table[0][0]); 
				  acm_rsp->rssi_report = round(average_rssi_table[0][0]);
#else
			#ifndef INTEGRATION_TEST
					//For MAC loopback testing : 
				  acm_rsp->cinr_report =round(average_cinr_table[my_basic_cid - BASIC_CID_MIN_VALUE][1]);//feeding back UL estimate itself to test 
				  acm_rsp->rssi_report = 0;
			#endif
#endif
				pthread_mutex_unlock(&cinr_table_lock);

				pthread_mutex_lock(&crc_table_lock);
#ifdef SS_TX
				  acm_rsp->crc_error_num = crc_error_count_table[0][0];
				  acm_rsp->total_packet_num = packet_count[0];
#else
			#ifndef INTEGRATION_TEST
				  acm_rsp->crc_error_num = crc_error_count_table[my_basic_cid - BASIC_CID_MIN_VALUE][1];
				  acm_rsp->total_packet_num = packet_count[my_basic_cid - BASIC_CID_MIN_VALUE];

			#endif
#endif
				pthread_mutex_unlock(&crc_table_lock);
			
				success = build_rep_rsp(acm_req, acm_rsp, payload, &len);
				if (success != 0)
				{
					FLOG_ERROR("Build ACM RSP failed\n");
					free(mm->data);
					break;
				}	
				tx_payload = (u_char*)mac_sdu_malloc(len,5);
				memcpy(tx_payload, payload, len);
				enqueue_transport_sdu_queue(dl_sdu_queue, mm->cid, len, tx_payload);
				free(mm->data);
				free(acm_req);
				free(acm_rsp);
				break;
			case REP_RSP:
				#ifdef SS_TX
					#ifdef INTEGRATION_TEST
					FLOG_WARNING("Unexpected REP_RSP in SS mode\n");
					exit(-1);	
					#endif
				#endif
				acm_rsp = (acm_rep_rsp_t*)malloc(sizeof(acm_rep_rsp_t));	
				memset(acm_rsp, 0, sizeof(acm_rep_rsp_t));
				success = parse_rep_rsp(mm->data, mm->length, acm_rsp);
				if (success != 0)
				{
					FLOG_ERROR("Parse ACM RSP failed\n");
					free(mm->data);
					break;
				}
				FLOG_DEBUG("AMC RSP: cinr = %d, rssi = %d, crc_error_num = %d, total_packet_num = %d\n", \
					acm_rsp->cinr_report, acm_rsp->rssi_report, acm_rsp->crc_error_num, acm_rsp->total_packet_num);
				//Code here to store received CINR, CRC erorr etc
				get_basic_cid(mm->cid, &my_basic_cid);	
				num_rep_retries_left[my_basic_cid - BASIC_CID_MIN_VALUE] = NUM_REP_REQ_RETRIES;	
				FLOG_DEBUG("In AMC RSP CINR is %d\n",acm_rsp->cinr_report);
				update_dl_link_quality_from_rep_rsp(my_basic_cid, acm_rsp->cinr_report, acm_rsp->rssi_report);
				//Update CRC error rate
				FLOG_DEBUG("In AMC RSP CRC errno is %d packet num %d\n",acm_rsp->crc_error_num,acm_rsp->total_packet_num);
				update_dl_crc_rate_from_rep_rsp(my_basic_cid, acm_rsp->crc_error_num, acm_rsp->total_packet_num);
				//Set timer for next rep_req
				gettimeofday(&temp_time, NULL);
				current_time = temp_time.tv_sec*1000000L + temp_time.tv_usec;
				firing_time = current_time + REP_REQ_PERIOD;
				app_timer_add(firing_time, &transmit_rep_req, NULL, NULL, 0, 0, (void**)&(rep_req_timer[my_basic_cid - BASIC_CID_MIN_VALUE]), (void*)((size_t)(mm->cid)));
				//Remove retry timer timeout
				if (rep_retry_timer[my_basic_cid - BASIC_CID_MIN_VALUE] != NULL) 
				{
					app_timer_delete(rep_retry_timer[my_basic_cid - BASIC_CID_MIN_VALUE]);
					rep_retry_timer[my_basic_cid - BASIC_CID_MIN_VALUE] = NULL;
				}
				free(mm->data);
				free(acm_rsp);
				break;

			default:
				FLOG_ERROR("Error in rep_state_machine: Incorrect input\n");
				break;
		}
		free(mm);
		mm = NULL;
	}
	return NULL;
}
