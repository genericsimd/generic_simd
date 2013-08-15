/* ----------------------------------------------------------------------------
   IBM Confidential

   IBM Wireless M2M Platform   

   (C)Copyright IBM Corp. 2011

   All Rights Reserved.

   File Name: mac_dsa_sm.c

   Change Activity:

   Date             Description of Change                   		By
   -----------      ---------------------		--------
   1-Mar.2011       Created                                 		Parul Gupta
   03-Feb.2012	modified for the support of dsc	Xianwei. Yi

   ----------------------------------------------------------------------------
   PROLOG END TAG zYx                                                         */

#include "mac_dsa_sm.h"

/* interference information, used to calculate phy capacity */
extern dts_info int_info;

#define BITS_PER_BYTE	(8U)
/*
  * can_serve_dsa - judge whether dsa can be served
  * @sf: service flow which dsa created
  *
  * The API is used to judge whether the service flow which dsa created can be served
  *
  * Return:
  *		one of the CC values (Ref: Table 599 of Spec Rev2D5)
  */
static int can_serve_dsa(serviceflow* sf)
{
#ifndef SS_RX
	int 	num_ul_slots = 0;
	int 	num_dl_slots = 0;
    float 	dl_capacity = 0;
	float 	ul_capacity = 0; /* unit: bits per frame */
	int 	total_rsvd_rate;
    
	// Currently this has very simple logic: if the sum of minimum reserved
	// rate for all the serviceflow at the BS exceeds the DL subframe capacity
	// then the DSA-REQ will be refused, else accepted. In future more criteria
	// according to jitter, latency etc might be used to decide
	if (sf->max_traffic_burst > MAX_BURST_SIZE) 
	{
		return CC_REJECT_UNSUPPORTED_PARAM_VALUE;
	}

	if (sf->tolerated_jitter < JITTER_THRESH) 
	{
		return CC_REJECT_UNSUPPORTED_PARAM_VALUE;
	}

	if (sf->max_latency < LATENCY_THRESH) 
	{
		return CC_REJECT_UNSUPPORTED_PARAM_VALUE;
	}

	calculate_phy_capacity(1, NULL, &dl_capacity, &ul_capacity, \
		&num_dl_slots, &num_ul_slots, NUM_DL_SUBCHANNELS - int_info.num_dl_interference, \
		NUM_UL_SUBCHANNELS - int_info.num_ul_interference);
	
	total_rsvd_rate = get_total_rsvd_rate(); /* unit: bits per frame */
	
	if (sf->sf_direction == UL)
	{
		/* check if the new total capacity exceeds the UL subframe cap */
		if ((sf->min_reserved_traffic_rate + total_rsvd_rate) * BITS_PER_BYTE > \
					(ul_capacity * 1000/ frame_duration[FRAME_DURATION_CODE]))
		{
			return CC_REJECT_RESOURCE;
		}
	}
	else
	{
		/* check if the new total capacity exceeds the DL subframe cap */
		if ((sf->min_reserved_traffic_rate + total_rsvd_rate) * BITS_PER_BYTE > \
					(dl_capacity * 1000/frame_duration[FRAME_DURATION_CODE]))
		{
			return CC_REJECT_RESOURCE;
		}
	}

	return CC_SUCCESS;
#else
	return CC_SUCCESS;
#endif
}

/*
  * mac_dsa_end - to finish a dsa transaction 
  * @arg: pointer to dsa transaction node
  *
  * The API is used to finish a dsa transaction, if the transaction is not successful, 
  * delete the created service flow
  *
  * Return:
  *		0 if successful
  *		otherwise if error happened
  */
static void mac_dsa_end(void *arg)
{
	struct transaction_node 	*trans_node;	
	sf_result			sf_add_result;
	
	assert(arg != NULL);

	trans_node = (struct transaction_node*)arg;

	pthread_mutex_lock(&trans_node->mutex);
	if (trans_node->notify != NULL)
	{
		if (trans_node->trans_status == DSA_SUCCESSFUL)
		{
			sf_add_result.cfm_code = CC_SUCCESS;
		}
		else
		{
			sf_add_result.cfm_code = CC_REJECT_OTHER;
		}
		sf_add_result.sf_id = trans_node->sf->sfid;
		sf_add_result.peer_mac = trans_node->peer_mac;
		(*trans_node->notify)(&sf_add_result);
		trans_node->notify = NULL;
	}
	pthread_mutex_unlock(&trans_node->mutex);

	/* delete the transaction node */
	delete_trans_node(&dsa_trans_list, trans_node);
}

/*
  * dsa_t10_timeout - dsa t10 timeout function 
  * @arg: pointer to dsa transaction node
  *
  * The API is dsa t10 timeout function. It is used to finish a dsa transaction
  *
  * Return:
  *		0 if successful
  *		otherwise if error happened
  */
static void dsa_t10_timeout(void *arg)
{
	struct transaction_node 	*trans_node;	
	sf_result			sf_add_result;
	
	assert(arg != NULL);

	trans_node = (struct transaction_node*)arg;
	
	pthread_mutex_lock(&trans_node->mutex);
	trans_node->t10_timer = NULL;
	pthread_mutex_unlock(&trans_node->mutex);

	mac_dsa_end(arg);
}

/*
  * dsa_t7_timeout - dsa t7 timeout function
  * @arg: point to transaction node
  *
  * The API is used to send dsa request,  it checks if retries are available, if yes, do retry,
  * otherwise terminate the dsa transaction
  *
  */
static int dsa_t7_timeout(void *arg)
{
	unsigned long long 		current_time;
	unsigned long long 		firing_time; 
	char 					*mac_payload;
	struct transaction_node *trans_node;
	int 					ret;

	trans_node = (struct transaction_node*)arg;
	ret = -1;

	pthread_mutex_lock(&trans_node->mutex);
	trans_node->t7_timer = NULL;
	
	if (trans_node->re_msg.retrytimes < NUM_DSX_REQ_RETRIES)
	{
		mac_payload = (char *)mac_sdu_malloc(trans_node->re_msg.length, MMM_CLASS_TYPE);
		if (mac_payload != NULL)
		{
			memcpy(mac_payload, trans_node->re_msg.msg, trans_node->re_msg.length);

			/* insert the dsc request to transport queue */
			enqueue_transport_sdu_queue(dl_sdu_queue, trans_node->peer_primary_cid, \
					trans_node->re_msg.length, mac_payload);

			trans_node->re_msg.retrytimes++;

			/* switch to dsc response pending state */
			trans_node->trans_status = DSA_RSP_PENDING;
			

			/* make T7 active */
			current_time = readtsc(); 
			firing_time = current_time + T7_DURATION;
			app_timer_add(firing_time, (timeout_action)&dsa_t7_timeout, \
							NULL, NULL, 0, 0, (void **)&trans_node->t7_timer, (void*)trans_node);
			ret = 0;
		}
	}
	else
	{
		FLOG_INFO("DSA has timed out for TID: %d. Ending\n", trans_node->trans_id);
		trans_node->trans_status = DSA_ERRED;

#ifdef SS_TX
		if (trans_node->t14_timer != NULL)
		{
			app_timer_delete(trans_node->t14_timer);
			trans_node->t14_timer = NULL;
		}
#endif
		
		/* make T10 active */
		if (trans_node->t10_timer == NULL)
		{
			current_time = readtsc(); 
			firing_time = current_time + T10_DURATION;
			app_timer_add(firing_time, (timeout_action)&dsa_t10_timeout, \
							NULL, NULL, 0, 0, (void **)&trans_node->t10_timer, (void*)trans_node);
		}
	}
	pthread_mutex_unlock(&trans_node->mutex);
	
	return ret;
}

/*
  * dsa_t8_expired - dsa T8 timeout function
  * @arg: pointer to transaction node
  *
  * The API is dsa T8 timeout function,  it checks if retries are available, if yes, do retry,
  * otherwise terminate the dsa transaction
  *
  */
static int dsa_t8_expired(void *arg)
{
	char 					*mac_payload;
	struct transaction_node *trans_node;
	int						ret;

	assert(arg != NULL);
	
	trans_node = (struct transaction_node*)arg;

	pthread_mutex_lock(&trans_node->mutex);
	trans_node->t8_timer = NULL;
	if (trans_node->re_msg.retrytimes < NUM_DSX_RSP_RETRIES)
	{
		mac_payload = (char *)mac_sdu_malloc(trans_node->re_msg.length, MMM_CLASS_TYPE);
		if (mac_payload != NULL)
		{
			memcpy(mac_payload, trans_node->re_msg.msg, trans_node->re_msg.length);

			/* enqueue the DSA RSP on the Primary MM CID of the sender */
			enqueue_transport_sdu_queue(dl_sdu_queue, trans_node->peer_primary_cid, \
											trans_node->re_msg.length, mac_payload);
			trans_node->re_msg.retrytimes++;

			/* activate T8 timer */
			unsigned long long current_time = readtsc(); 
			unsigned long long firing_time = current_time + T8_DURATION;
			app_timer_add(firing_time, &dsa_t8_expired, NULL, NULL, 0, 0, \
							(void **)&trans_node->t8_timer, (void*)trans_node);
		}
		pthread_mutex_unlock(&trans_node->mutex);
	}
	else
	{
		/* call DSA-Erred, Ended */
		FLOG_INFO("DSA has timed out for TID: %d. Ending\n", trans_node->trans_id);
		trans_node->trans_status = DSA_ERRED;
		pthread_mutex_unlock(&trans_node->mutex);
		
		mac_dsa_end(trans_node);
	}
	

	return 0;
}

/*
  * dsa_t14_timeout - dsa t14 timeout function
  * @arg: point to transaction node
  *
  * The API is used to send dsa request,  it checks if retries are available, if yes, do retry,
  * otherwise terminate the dsa transaction
  *
  */
static int dsa_t14_timeout(void *arg)
{
	unsigned long long 		current_time;
	unsigned long long 		firing_time; 
	char 					*mac_payload;
	struct transaction_node *trans_node;
	int 					ret;

	trans_node = (struct transaction_node*)arg;
	ret = -1;

	pthread_mutex_lock(&trans_node->mutex);
	trans_node->t14_timer = NULL;
	
	if (trans_node->re_msg.retrytimes < NUM_DSX_REQ_RETRIES)
	{
		mac_payload = (char *)mac_sdu_malloc(trans_node->re_msg.length, MMM_CLASS_TYPE);
		if (mac_payload != NULL)
		{
			memcpy(mac_payload, trans_node->re_msg.msg, trans_node->re_msg.length);

			/* insert the dsc request to transport queue */
			enqueue_transport_sdu_queue(dl_sdu_queue, trans_node->peer_primary_cid, \
					trans_node->re_msg.length, mac_payload);

			trans_node->re_msg.retrytimes++;

			/* switch to dsc response pending state */
			trans_node->trans_status = DSA_RSP_PENDING;

			/* if itself is SS, make T14 active */
			current_time = readtsc(); 
			firing_time = current_time + T14_DURATION;
			app_timer_add(firing_time, (timeout_action)&dsa_t14_timeout, \
							NULL, NULL, 0, 0, (void **)&trans_node->t14_timer, (void*)trans_node);
			ret = 0;
		}
	}
	else
	{
		FLOG_INFO("DSA has timed out for TID: %d. Ending\n", trans_node->trans_id);
		trans_node->trans_status = DSA_ERRED;

		if (trans_node->t7_timer != NULL)
		{
			app_timer_delete(trans_node->t7_timer);
			trans_node->t7_timer = NULL;
		}
		
		/* make T10 active */
		if (trans_node->t10_timer == NULL)
		{
			current_time = readtsc(); 
			firing_time = current_time + T10_DURATION;
			app_timer_add(firing_time, (timeout_action)&dsa_t10_timeout, \
						NULL, NULL, 0, 0, (void **)&trans_node->t10_timer, (void*)trans_node);
		}
	}
	pthread_mutex_unlock(&trans_node->mutex);
	
	return ret;
}

static int send_dsa_req(void *arg)
{
	unsigned long long 		current_time;
	unsigned long long 		firing_time; 
	char 					*mac_payload;
	struct transaction_node *trans_node;
	int 					ret;

	trans_node = (struct transaction_node*)arg;
	ret = -1;

	pthread_mutex_lock(&trans_node->mutex);
	mac_payload = (char *)mac_sdu_malloc(trans_node->re_msg.length, MMM_CLASS_TYPE);
	if (mac_payload != NULL)
	{
		memcpy(mac_payload, trans_node->re_msg.msg, trans_node->re_msg.length);

		/* insert the dsc request to transport queue */
		enqueue_transport_sdu_queue(dl_sdu_queue, trans_node->peer_primary_cid, \
					trans_node->re_msg.length, mac_payload);

		trans_node->re_msg.retrytimes++;

		/* switch to dsc response pending state */
		trans_node->trans_status = DSA_RSP_PENDING;

		/* make T7 active */
		current_time = readtsc(); 
		firing_time = current_time + T7_DURATION;
		app_timer_add(firing_time, (timeout_action)&dsa_t7_timeout, \
							NULL, NULL, 0, 0, (void **)&trans_node->t7_timer, (void*)trans_node);


#ifdef SS_TX
		/* if itself is SS, make T14 active */
		firing_time = current_time + T14_DURATION;
		app_timer_add(firing_time, (timeout_action)&dsa_t14_timeout, \
							NULL, NULL, 0, 0, (void **)&trans_node->t14_timer, (void*)trans_node);
#endif
		ret = 0;
	}
	pthread_mutex_unlock(&trans_node->mutex);
	
	return ret;
}

/*
  * dsa_primitive_handler - to process the add primitive of service flow
  * @primit: primitive type
  * @trans_node: transaction node for the primitive
  *
  * The API is used to process the add primitives of service flow, which is usually passed down by
  * APP layer
  *
  * Return:
  *		0 if successful
  *		otherwise if error happened
  */
int dsa_primitive_handler(primitivetype primit, struct transaction_node* trans_node)
{
	dsa_req_msg	dsa_req;
	char		*dsa_req_payload;
	int 		length;
	int			ret;
	int			dsa_allowed;

	assert(trans_node != NULL);

	ret = -1;
	switch(primit)
	{
		case SF_ADD:
			dsa_allowed = can_serve_dsa(trans_node->sf);
			if (dsa_allowed == CC_SUCCESS)
			{
				dsa_req_payload = (char *)malloc(MAX_DSA_REQ_LEN);
				if (dsa_req_payload != NULL)
				{
					init_dsa_req(trans_node->sf, &dsa_req);
					build_dsa_req(&dsa_req, dsa_req_payload, &length);
					
					/* save the dsa request */
					trans_node->re_msg.type = DSA_REQ;
					trans_node->re_msg.retrytimes = 0;
					trans_node->re_msg.msg = dsa_req_payload;
					trans_node->re_msg.length = length;


					ret = send_dsa_req(trans_node);
				}
			}
			else	
			{
				FLOG_INFO("%s: can't service the dsa request\n", __FUNCTION__);
			}
			break;
		default:
			FLOG_ERROR("%s: unsupported primitive\n", __FUNCTION__);
			break;
	}
	
	return ret;
}

static int dsa_sm_dsa_req_handler(dsa_req_msg *dsa_req, struct transaction_node* trans_node)
{
	char			*dsa_rsp_payload;
	int			length;
	char			*mac_payload;
	int			ret;
	int 			cfm_code;
	unsigned long long 	current_time;
	unsigned long long 	firing_time;
	dsa_rsp_msg		*dsa_rsp;
#ifndef SS_RX
	dsx_rvd_msg		*dsx_rvd;
	char			*dsx_rcv_payload;
#endif
		
	assert(dsa_req != NULL);
	assert(trans_node != NULL);

	ret = -1;
	switch (trans_node->trans_status)
	{
		case DSA_BEGIN:
			dsa_rsp = (dsa_rsp_msg*)malloc(sizeof(dsa_rsp_msg));
			dsa_rsp_payload = (char *)malloc(MAX_DSA_RSP_LEN);
#ifndef SS_RX
			dsx_rvd = (dsx_rvd_msg*)malloc(sizeof(dsx_rvd_msg));
			dsx_rcv_payload = (char *)malloc(DSX_RVD_LEN);
#endif

#ifndef SS_RX
			if ((dsa_rsp != NULL) && (dsa_rsp_payload != NULL) && \
				(dsx_rvd != NULL) && (dsx_rcv_payload != NULL))
#else
			if ((dsa_rsp != NULL) && (dsa_rsp_payload != NULL))
#endif
			{
				pthread_mutex_lock(&trans_node->mutex);
				
				memset(dsa_rsp, 0, sizeof(dsa_rsp_msg));
				cfm_code = can_serve_dsa(trans_node->sf);
				init_dsa_rsp(dsa_req, trans_node->sf, cfm_code, dsa_rsp);
				build_dsa_rsp(dsa_rsp, dsa_rsp_payload, &length);
				
				/* save the dsa response */
				trans_node->re_msg.type = DSA_RSP;
				trans_node->re_msg.retrytimes = 0;
				trans_node->re_msg.msg = dsa_rsp_payload;
				trans_node->re_msg.length = length;

#ifndef SS_RX
				/* build dsa rvd */
				init_dsx_rvd(dsa_req, dsx_rvd);
				dsx_rvd->cfm_code = CC_SUCCESS;
				build_dsx_rvd (dsx_rvd, dsx_rcv_payload, &length);
				
				/* send dsa rvd */
				mac_payload = (char *)mac_sdu_malloc(length, MMM_CLASS_TYPE);
				if (mac_payload != NULL)
				{
					memcpy(mac_payload, dsx_rcv_payload, length);
					enqueue_transport_sdu_queue(dl_sdu_queue, trans_node->peer_primary_cid, \
														length, mac_payload);
				}
#endif

				/* delete T8 if T8 active */
				if (trans_node->t8_timer != NULL)
				{
					app_timer_delete(trans_node->t8_timer);
					trans_node->t8_timer = NULL;
				}

				/* send dsa response */
				mac_payload = (char *)mac_sdu_malloc(trans_node->re_msg.length, MMM_CLASS_TYPE);
				if (mac_payload != NULL)
				{
					memcpy(mac_payload, trans_node->re_msg.msg, trans_node->re_msg.length);

					/* enqueue the DSC RSP on the Primary MM CID of the sender */
					enqueue_transport_sdu_queue(dl_sdu_queue, trans_node->peer_primary_cid, \
													trans_node->re_msg.length, mac_payload);

					/* activate T8 timer */
					current_time = readtsc(); 
					firing_time = current_time + T8_DURATION;
					app_timer_add(firing_time, (timeout_action)&dsa_t8_expired, \
											NULL, NULL, 0, 0, \
											(void **)&trans_node->t8_timer, (void*)trans_node);

					ret = 0;
				}

				/* switch to ack pending state */
				trans_node->trans_status = DSA_ACK_PENDING;
				pthread_mutex_unlock(&trans_node->mutex);

				free_dsa_rsp(dsa_rsp);

#ifndef SS_RX
				free(dsx_rvd);
				free(dsx_rcv_payload);
#endif
			}
			else
			{
				free(dsa_rsp);
				free(dsa_rsp_payload);
#ifndef SS_RX
				free(dsx_rvd);
				free(dsx_rcv_payload);
#endif
			}
			break;
		case DSA_ACK_PENDING:
#ifndef SS_RX
			dsx_rvd = (dsx_rvd_msg*)malloc(sizeof(dsx_rvd_msg));
			dsx_rcv_payload = (char *)malloc(DSX_RVD_LEN);
			if ((dsx_rvd != NULL) && (dsx_rcv_payload != NULL))
			{
				pthread_mutex_lock(&trans_node->mutex);
				
				/* build dsa rvd */
				init_dsx_rvd(dsa_req, dsx_rvd);
				dsx_rvd->cfm_code = 0;
				build_dsx_rvd (dsx_rvd, dsx_rcv_payload, &length);

				/* send dsa rvd */
				mac_payload = (char *)mac_sdu_malloc(length, MMM_CLASS_TYPE);
				if (mac_payload != NULL)
				{
					memcpy(mac_payload, dsx_rcv_payload, length);
					enqueue_transport_sdu_queue(dl_sdu_queue, trans_node->peer_primary_cid, \
														length, mac_payload);
				}

				/* send dsa response again */
				mac_payload = (char *)mac_sdu_malloc(trans_node->re_msg.length, MMM_CLASS_TYPE);
				if (mac_payload != NULL)
				{
					memcpy(mac_payload, trans_node->re_msg.msg, trans_node->re_msg.length);

					/* enqueue the DSA RSP on the Primary MM CID of the sender */
					enqueue_transport_sdu_queue(dl_sdu_queue, \
								trans_node->peer_primary_cid, trans_node->re_msg.length, mac_payload);
					ret = 0;
				}
				pthread_mutex_unlock(&trans_node->mutex);

				free(dsx_rvd);
				free(dsx_rcv_payload);
			}
			else
			{
				free(dsx_rvd);
				free(dsx_rcv_payload);
			}
#else
			pthread_mutex_lock(&trans_node->mutex);
			/* send dsa response again */
			mac_payload = (char *)mac_sdu_malloc(trans_node->re_msg.length, MMM_CLASS_TYPE);
			if (mac_payload != NULL)
			{
				memcpy(mac_payload, trans_node->re_msg.msg, trans_node->re_msg.length);

				/* enqueue the DSA RSP on the Primary MM CID of the sender */
				enqueue_transport_sdu_queue(dl_sdu_queue, \
									trans_node->peer_primary_cid, trans_node->re_msg.length, mac_payload);
				ret = 0;
			}
			pthread_mutex_unlock(&trans_node->mutex);
#endif
			break;
		default:
			FLOG_INFO("%s: unexpected dsa request", __FUNCTION__);
			break;
	}

	return ret;
}

extern void add_conn_for_service_flow(u_int64_t peer_mac, struct service_flow *flow);
extern void add_svc_flow_to_ss(serviceflow* sflow, bs_ss_info* ss_info);
static void dsa_enable_sf(struct transaction_node *trans_node)
{
	struct service_flow		*sf_node;
	sf_result				sf_add_result;
	bs_ss_info				*ss_info;
	
	if (trans_node != NULL)
	{
		ss_info = find_bs_ss_info(trans_node->peer_mac);
		if (ss_info != NULL)
		{
			/* enable service flow, add the service flow to the SS */
			sf_node = trans_node->sf;
			if (sf_node != NULL)
			{			
				/* move the service flow to ss */
				FLOG_INFO("-------- %s: added sf: sf_id = %d, cid = %d list_head = %p --------\n", \
						__FUNCTION__, sf_node->sfid, sf_node->cid, ss_info->sf_list_head);
				
				add_svc_flow_to_ss(sf_node, ss_info);

				trans_node->sf = NULL;

				/* add connection for service flow */
				add_conn_for_service_flow(trans_node->peer_mac, sf_node);

				trans_node->sf = NULL;
				if (trans_node->notify != NULL)
				{
					sf_add_result.peer_mac = trans_node->peer_mac;
					sf_add_result.cfm_code = CC_SUCCESS;
					sf_add_result.sf_id = sf_node->sfid;
					(*trans_node->notify)(&sf_add_result);
					trans_node->notify = NULL;
				}
			}
		}
	}
}

static int dsa_sm_dsa_rsp_handler(dsa_rsp_msg *dsa_rsp, struct transaction_node* trans_node)
{
	int						length;
	char					*mac_payload;
	int						ret;
	unsigned long long 		current_time;
	unsigned long long 		firing_time;
	u_int64_t				mac_addr;
	dsa_ack_msg				*dsa_ack;
	char					*ack_payload;
	
	assert(dsa_rsp != NULL);
	assert(trans_node != NULL);

	ret = -1;
	switch (trans_node->trans_status)
	{
		case DSA_RSP_PENDING:
			/* pass down */
		case LCL_DSA_RETRIES_EXHAUSTED:
			/* pass down */
		case LCL_DSA_HOLDING_DOWN:
			dsa_ack = (dsa_ack_msg*)malloc(sizeof(dsa_ack_msg));
			ack_payload = (u_char*)malloc(MAX_DSA_ACK_LEN);
			if ((dsa_ack != NULL) && (ack_payload != NULL))
			{
				pthread_mutex_lock(&trans_node->mutex);
				/* delete T7 if T7 active */
				if(trans_node->t7_timer != NULL) 
				{
					app_timer_delete(trans_node->t7_timer);
					trans_node->t7_timer = NULL;
				}

				/* delete T10 if T10 active */
				if(trans_node->t10_timer != NULL) 
				{
					app_timer_delete(trans_node->t10_timer);
					trans_node->t10_timer = NULL;
				}

#ifdef SS_TX
				/* delete T14 if T14 active */
				if(trans_node->t14_timer != NULL) 
				{
					app_timer_delete(trans_node->t14_timer);
					trans_node->t14_timer = NULL;
				}
#endif
				
				if (trans_node->trans_status == DSA_RSP_PENDING || trans_node->re_msg.msg == NULL)
				{
					/* release dsa request */
					if (trans_node->trans_status == DSA_RSP_PENDING)
					{
						free(trans_node->re_msg.msg);
						trans_node->re_msg.msg = NULL;
					}
					
					/* build dsa acknowledge */
					memset(dsa_ack, 0, sizeof(dsa_ack_msg));
					init_dsa_ack(dsa_rsp, dsa_ack);
					build_dsa_ack(dsa_ack, ack_payload, &length);
					
					/* save the dsa ack */
					trans_node->re_msg.type = DSA_ACK;
					trans_node->re_msg.retrytimes = 0;
					trans_node->re_msg.msg = ack_payload;
					trans_node->re_msg.length = length;

					free_dsa_ack(dsa_ack);
				}
				else
				{
					free(dsa_ack);
					free(ack_payload);
				}
				
				if ((trans_node->trans_status == DSA_RSP_PENDING) && (dsa_rsp->cfm_code == CC_SUCCESS))
				{
					dsa_rsp_to_sf(dsa_rsp, trans_node->sf);
					dsa_enable_sf(trans_node);
				}

				/* send the dsa ack */
				mac_payload = (char *)mac_sdu_malloc(trans_node->re_msg.length, MMM_CLASS_TYPE);
				if (mac_payload != NULL)
				{
					memcpy(mac_payload, trans_node->re_msg.msg, trans_node->re_msg.length);
					enqueue_transport_sdu_queue(dl_sdu_queue, trans_node->peer_primary_cid,\
														trans_node->re_msg.length, mac_payload);

					/* switch to holding down state*/
					trans_node->trans_status = LCL_DSA_HOLDING_DOWN;
						
					/* make T10 active*/
					current_time = readtsc(); 
					firing_time = current_time + T10_DURATION;
					app_timer_add(firing_time, (timeout_action)&dsa_t10_timeout, NULL, NULL, \
										0, 0, (void **)&trans_node->t10_timer, (void*)trans_node);

					ret = 0;
				}
				pthread_mutex_unlock(&trans_node->mutex);
			}
			else
			{
				free(dsa_ack);
				free(ack_payload);
			}
			break;
		default:
			FLOG_INFO("%s: unexpected dsa response", __FUNCTION__);
			break;
	}

	return ret;
}

static int dsa_sm_dsa_ack_handler(dsa_ack_msg *dsa_ack, struct transaction_node* trans_node)
{
	int 					ret;
	unsigned long long 		current_time;
	unsigned long long 		firing_time;
	u_int64_t				mac_addr;
	
	assert(dsa_ack != NULL);
	assert(trans_node != NULL);

	ret = -1;
	switch (trans_node->trans_status)
	{
		case DSA_ACK_PENDING:
			pthread_mutex_lock(&trans_node->mutex);
			if (trans_node->t8_timer != NULL)
			{
				app_timer_delete(trans_node->t8_timer);
				trans_node->t8_timer = NULL;
			}

			if (dsa_ack->cfm_code == CC_SUCCESS)
			{
				dsa_enable_sf(trans_node);
			}

			/* switch to dsa remote holding down state */
			trans_node->trans_status = RMT_DSA_HOLDING_DOWN;

			/* make T10 active*/
			current_time = readtsc(); 
			firing_time = current_time + T10_DURATION;
			app_timer_add(firing_time, (timeout_action)dsa_t10_timeout, NULL, NULL, \
							0, 0, (void **)&trans_node->t10_timer, (void*)trans_node);

			/* switch to remote holding down state*/
			trans_node->trans_status = RMT_DSA_HOLDING_DOWN;
			pthread_mutex_unlock(&trans_node->mutex);
			
			ret = 0;
			break;
		default:
			FLOG_INFO("unexpected dsa ack, trans_node->trans_status = %d\n", trans_node->trans_status);
			break;
	}

	return ret;
}

static int dsa_sm_dsx_rvd_handler(dsx_rvd_msg *dsx_rvd, struct transaction_node* trans_node)
{
	assert(dsx_rvd != NULL);
	assert(trans_node != NULL);

	/* if T14 active, deactive T14 */
	pthread_mutex_lock(&trans_node->mutex);
	if (trans_node->t14_timer != NULL) 
	{
		app_timer_delete(trans_node->t14_timer);
		trans_node->t14_timer = NULL;
	}
	pthread_mutex_unlock(&trans_node->mutex);
	
	return 0;
}

/*
  * dsa_msg_handler - process the dsa messages
  * @dsa_msg: dsa messae
  * @msg_type: message type
  * @trans_node: transaction node
  *
  * The API is used to process the dsa messages, which is from the peer.
  *
  * Return:
  *		0 if successful
  *		otherwise if error happened
  */
int dsa_msg_handler(void *dsa_msg, int msg_type, struct transaction_node *trans_node)
{
	int 		ret;
	dsa_req_msg	*dsa_req;
	dsa_rsp_msg	*dsa_rsp;
	dsa_ack_msg	*dsa_ack;
	dsx_rvd_msg	*dsx_rvd;
	
	assert(dsa_msg != NULL);
	assert((msg_type == DSA_REQ) || (msg_type == DSA_RSP) \
			|| (msg_type == DSA_ACK) || (msg_type == DSX_RVD));
	assert(trans_node != NULL);

	ret = -1;
	switch (msg_type)
	{
		case DSA_REQ:
			dsa_req = (dsa_req_msg *)dsa_msg;
			ret = dsa_sm_dsa_req_handler(dsa_req, trans_node);
			break;
		case DSA_RSP:
			dsa_rsp = (dsa_rsp_msg *)dsa_msg;
			ret = dsa_sm_dsa_rsp_handler(dsa_rsp, trans_node);
			break;
		case DSA_ACK:
			dsa_ack = (dsa_ack_msg *)dsa_msg;
			ret = dsa_sm_dsa_ack_handler(dsa_ack, trans_node);
			break;
		case DSX_RVD:
			dsx_rvd = (dsx_rvd_msg *)dsa_msg;
			ret = dsa_sm_dsx_rvd_handler(dsx_rvd, trans_node);
			break;
		default:
			break;
	}

	return ret;
}

