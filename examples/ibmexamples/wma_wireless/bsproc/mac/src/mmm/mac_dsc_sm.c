/* ----------------------------------------------------------------------------
   IBM Confidential

   IBM Wireless M2M Platform   

   (C)Copyright IBM Corp. 2011

   All Rights Reserved.

   File Name: mac_dsc_sm.c

   Change Activity:

   Date                      Description of Change                   By
   -----------      --------------------- 		--------
   30-Jan.2012		Created                          		Xianwei. Yi

   ----------------------------------------------------------------------------
   PROLOG END TAG zYx                                                         */
   
#include "mac_dsc_sm.h"

extern struct service_flow	* find_sf_in_peer(int sf_id, u_int64_t	peer_mac);

/* interference information, used to calculate phy capacity */
extern dts_info int_info;

/*
  * can_serve_dsc - judge whether the dsc request can be served
  * @dsc_req: dsc request message
  *
  * The API is used to judge whether the dsc request can be served
  *
  * Return:
  *		CC_SUCCESS if OK
  *		otherwise if dsc request can not be served(Ref: Table 599 of Spec Rev2D5)
  */
#define BITS_PER_BYTE (8U)
static int can_serve_dsc(struct service_flow *sf, u_int64_t peer_mac)
{
	int 					num_ul_slots = 0;
	int 					num_dl_slots = 0;
    float 					dl_capacity = 0;
	float 					ul_capacity = 0; /* unit: bits per frame */
	int 					total_rsvd_rate;
	struct	service_flow	*orig_sf;
	int						incr_traffic_rate;

	orig_sf = find_sf_in_peer(sf->sfid, peer_mac);
	if (orig_sf == NULL)
	{
		return CC_REJECT_SF_NOT_FOUND;
	}

	if (sf->min_reserved_traffic_rate <= orig_sf->min_reserved_traffic_rate)
	{
		return CC_SUCCESS;
	}

	incr_traffic_rate = sf->min_reserved_traffic_rate - orig_sf->min_reserved_traffic_rate;

	calculate_phy_capacity(1, NULL, &dl_capacity, &ul_capacity, \
		&num_dl_slots, &num_ul_slots, NUM_DL_SUBCHANNELS - int_info.num_dl_interference, \
		NUM_UL_SUBCHANNELS - int_info.num_ul_interference);
	
	total_rsvd_rate = get_total_rsvd_rate(); /* unit: bits per frame */
	
	if (sf->sf_direction == UL)
	{
		/* check if the new total capacity exceeds the UL subframe cap */
		if ((incr_traffic_rate + total_rsvd_rate) * BITS_PER_BYTE > \
					(ul_capacity * 1000/ frame_duration[FRAME_DURATION_CODE]))
		{
			return CC_REJECT_RESOURCE;
		}
	}
	else
	{
		/* check if the new total capacity exceeds the DL subframe cap */
		if ((incr_traffic_rate + total_rsvd_rate) * BITS_PER_BYTE > \
					(dl_capacity * 1000/ frame_duration[FRAME_DURATION_CODE]))
		{
			return CC_REJECT_RESOURCE;
		}
	}

	return CC_SUCCESS;
}

/*
  * mac_dsc_end - to finish a dsc transaction 
  * @arg: pointer to dsc transaction node
  *
  * The API is used to finish a dsc transaction
  *
  * Return:
  *		0 if successful
  *		otherwise if error happened
  */
static void mac_dsc_end(void *arg)
{
	struct transaction_node *trans_node;
	sf_result				sf_change_result;
	
	assert(arg != NULL);
	
	trans_node = (struct transaction_node*)arg;

	pthread_mutex_lock(&trans_node->mutex);
	if (trans_node->notify != NULL)
	{
		if (trans_node->trans_status == DSC_SUCCESSFUL)
		{
			sf_change_result.cfm_code = CC_SUCCESS;
		}
		else
		{
			sf_change_result.cfm_code = CC_REJECT_OTHER;
		}
		sf_change_result.sf_id = trans_node->sf->sfid;
		sf_change_result.peer_mac = trans_node->peer_mac;
		(*trans_node->notify)(&sf_change_result);
		trans_node->notify = NULL;
	}
	pthread_mutex_unlock(&trans_node->mutex);

	delete_trans_node(&dsc_trans_list, trans_node);
}

/*
  * dsc_t10_timeout - dsc t10 timeout function 
  * @arg: pointer to dsc transaction node
  *
  * The API is dsc t10 timeout function. It is used to finish a dsc transaction
  *
  * Return:
  *		0 if successful
  *		otherwise if error happened
  */
static void dsc_t10_timeout(void *arg)
{
	struct transaction_node 	*trans_node;	
	sf_result			sf_add_result;
	
	assert(arg != NULL);

	trans_node = (struct transaction_node*)arg;
	
	pthread_mutex_lock(&trans_node->mutex);
	trans_node->t10_timer = NULL;
	pthread_mutex_unlock(&trans_node->mutex);

	mac_dsc_end(arg);
}

/*
  * dsc_t7_timeout - dsc t7 timeout function
  * @arg: point to transaction node
  *
  * The API is used to send dsc request,  it checks if retries are available, if yes, do retry,
  * otherwise terminate the dsc transaction
  *
  */
static int dsc_t7_timeout(void *arg)
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
			trans_node->trans_status = DSC_RSP_PENDING;
			

			/* make T7 active */
			current_time = readtsc(); 
			firing_time = current_time + T7_DURATION;
			app_timer_add(firing_time, (timeout_action)&dsc_t7_timeout, \
							NULL, NULL, 0, 0, (void **)&trans_node->t7_timer, (void*)trans_node);
			ret = 0;
		}
	}
	else
	{
		FLOG_INFO("DSC has timed out for TID: %d. Ending\n", trans_node->trans_id);
		trans_node->trans_status = DSC_ERRED;

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
			app_timer_add(firing_time, (timeout_action)&dsc_t10_timeout, \
							NULL, NULL, 0, 0, (void **)&trans_node->t10_timer, (void*)trans_node);
		}
	}
	pthread_mutex_unlock(&trans_node->mutex);
	
	return ret;
}

/*
  * dsc_t14_timeout - dsc t14 timeout function
  * @arg: point to transaction node
  *
  * The API is used to send dsc request,  it checks if retries are available, if yes, do retry,
  * otherwise terminate the dsc transaction
  *
  */
static int dsc_t14_timeout(void *arg)
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
			trans_node->trans_status = DSC_RSP_PENDING;

			/* if itself is SS, make T14 active */
			current_time = readtsc();
			firing_time = current_time + T14_DURATION;
			app_timer_add(firing_time, (timeout_action)&dsc_t14_timeout, \
							NULL, NULL, 0, 0, (void **)&trans_node->t14_timer, (void*)trans_node);
			ret = 0;
		}
	}
	else
	{
		FLOG_INFO("DSC has timed out for TID: %d. Ending\n", trans_node->trans_id);
		trans_node->trans_status = DSC_ERRED;

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
			app_timer_add(firing_time, (timeout_action)&dsc_t10_timeout, \
						NULL, NULL, 0, 0, (void **)&trans_node->t10_timer, (void*)trans_node);
		}
	}
	pthread_mutex_unlock(&trans_node->mutex);
	
	return ret;
}


static int send_dsc_req(void *arg)
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
		trans_node->trans_status = DSC_RSP_PENDING;

		/* make T7 active */
		current_time = readtsc(); 
		firing_time = current_time + T7_DURATION;
		app_timer_add(firing_time, (timeout_action)&dsc_t7_timeout, \
							NULL, NULL, 0, 0, (void **)&trans_node->t7_timer, (void*)trans_node);


#ifdef SS_TX
		/* if itself is SS, make T14 active */
		firing_time = current_time + T14_DURATION;
		app_timer_add(firing_time, (timeout_action)&dsc_t14_timeout, \
							NULL, NULL, 0, 0, (void **)&trans_node->t14_timer, (void*)trans_node);
#endif
		ret = 0;
	}
	pthread_mutex_unlock(&trans_node->mutex);
	
	return ret;
}

/*
  * dsc_primitive_handler - to process the change primitive of service flow
  * @primit: primitive type
  * @trans_node: transaction node for the primitive
  *
  * The API is used to process the change primitive of service flow, which is usually passed down by
  * APP layer
  *
  * Return:
  *		0 if successful
  *		otherwise if error happened
  */
int dsc_primitive_handler(primitivetype primit, struct transaction_node* trans_node)
{
	int				dsc_allowed;
	dsc_req_msg		dsc_req;
	char			*dsc_req_payload;
	int 			length;
	int				ret;

	assert(trans_node != NULL);

	ret = -1;
	switch (primit)
	{
		case SF_CHANGE:
			dsc_allowed = can_serve_dsc(trans_node->sf, trans_node->peer_mac);
			if (dsc_allowed == CC_SUCCESS)
			{
				dsc_req_payload = (char *)malloc(MAX_DSC_REQ_LEN);
				if (dsc_req_payload != NULL)
				{
					init_dsc_req(trans_node->sf, &dsc_req);
					build_dsc_req(&dsc_req, dsc_req_payload, &length);
					
					/* save the dsc request */
					trans_node->re_msg.type = DSC_REQ;
					trans_node->re_msg.retrytimes = 0;
					trans_node->re_msg.msg = dsc_req_payload;
					trans_node->re_msg.length = length;


					ret = send_dsc_req(trans_node);
				}
			}
			break;
		default:
			FLOG_INFO("%s: unexpected primitive type: %d\n", __FUNCTION__, primit);
			break;
	}
	return ret;
}

char *dsc_state_to_string(int trans_state)
{
	char	*result;
	
	switch (trans_state)
	{
		case DSC_RSP_PENDING:
			result = "dsc response pending";
			break;
		case DSC_LOCAL_RETRIES_EXHAUSTED:
			result = "dsc local retries exhausted";
			break;
		case DSC_LOCAL_HOLDING_DOWN:
			result = "dsc local holding downing";
			break;
		case DSC_LOCAL_DELETING_SF:
			result = "dsc local deleting service flow";
			break;
		case DSC_ACK_PENDING:
			result = "dsc ack pending";
			break;
		case DSC_REMOTE_HOLDING_DOWN:
			result = "dsc remote holding down";
			break;
		case DSC_REMOTE_DELETING_SF:
			result = "dsc remote deleting service flow";
			break;
		case DSC_BEGIN:
			result = "dsc begin";
			break;
		case DSC_FAILED:
			result = "dsc failed";
			break;
		case DSC_ERRED:
			result = "dsc errored";
			break;
		case DSC_SUCCESSFUL:
			result = "dsc successful";
			break;
		default:
			result = "dsc unknown state";
			break;
	} 

	return result;
}

/*
  * dsc_t8_expired - dsc T8 timeout function
  * @arg: pointer to transaction node
  *
  * The API is dsc T8 timeout function,  it checks if retries are available, if yes, do retry,
  * otherwise terminate the dsc transaction
  *
  */
int dsc_t8_expired(void *arg)
{
	char 						*mac_payload;
	struct transaction_node 	*trans_node;
	int							ret;

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

			/* enqueue the DSC RSP on the Primary MM CID of the sender */
			enqueue_transport_sdu_queue(dl_sdu_queue, trans_node->peer_primary_cid, \
											trans_node->re_msg.length, mac_payload);
			trans_node->re_msg.retrytimes++;

			/* activate T8 timer */
			unsigned long long current_time = readtsc(); 
			unsigned long long firing_time = current_time + T8_DURATION;
			app_timer_add(firing_time, &dsc_t8_expired, NULL, NULL, 0, 0, \
							(void **)&trans_node->t8_timer, (void*)trans_node);
		}
		pthread_mutex_unlock(&trans_node->mutex);
	}
	else
	{
		/* call DSC-Erred, Ended */
		FLOG_INFO("DSC has timed out for TID: %d. Ending\n", trans_node->trans_id);
		trans_node->trans_status = DSC_ERRED;
		pthread_mutex_unlock(&trans_node->mutex);
		
		mac_dsc_end(trans_node);
	}
	return 0;
}

int dsc_sm_dsc_req_handler(dsc_req_msg *dsc_req, struct transaction_node* trans_node)
{
	int					length;
	char				*mac_payload;
	int					ret;
	int 				cfm_code;
	unsigned long long 	current_time;
	unsigned long long 	firing_time;
	dsc_rsp_msg			*dsc_rsp;
	char				*dsc_rsp_payload;
#ifndef SS_RX
	dsx_rvd_msg			*dsx_rvd;
	char				*dsx_rcv_payload;
#endif
		
	assert(dsc_req != NULL);
	assert(trans_node != NULL);

	ret = -1;
	switch (trans_node->trans_status)
	{
		case DSC_BEGIN:
			dsc_rsp = (dsc_rsp_msg*)malloc(sizeof(dsc_rsp_msg));
			dsc_rsp_payload = (char *)malloc(MAX_DSC_RSP_LEN);
#ifndef SS_RX
			dsx_rvd = (dsx_rvd_msg*)malloc(sizeof(dsx_rvd_msg));
			dsx_rcv_payload = (char *)malloc(DSX_RVD_LEN);
#endif

#ifndef SS_RX
			if ((dsc_rsp != NULL) && (dsc_rsp_payload != NULL) && \
				(dsx_rvd != NULL) && (dsx_rcv_payload != NULL))
#else
			if ((dsc_rsp != NULL) && (dsc_rsp_payload != NULL))
#endif
			{
				pthread_mutex_lock(&trans_node->mutex);
				
				memset(dsc_rsp, 0, sizeof(dsc_rsp_msg));
				cfm_code = can_serve_dsc(trans_node->sf, trans_node->peer_mac);
				init_dsc_rsp(dsc_req, trans_node->sf, cfm_code, dsc_rsp);
				build_dsc_rsp(dsc_rsp, dsc_rsp_payload, &length);
				
				/* save the dsc response */
				trans_node->re_msg.type = DSC_RSP;
				trans_node->re_msg.retrytimes = 0;
				trans_node->re_msg.msg = dsc_rsp_payload;
				trans_node->re_msg.length = length;

#ifndef SS_RX
				/* build dsc rvd */
				init_dsx_rvd((dsa_req_msg *)dsc_req, dsx_rvd);
				dsx_rvd->cfm_code = CC_SUCCESS;
				build_dsx_rvd (dsx_rvd, dsx_rcv_payload, &length);
				
				/* send dsc rvd */
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

				/* send dsc response */
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
					app_timer_add(firing_time, (timeout_action)&dsc_t8_expired, \
											NULL, NULL, 0, 0, \
											(void **)&trans_node->t8_timer, (void*)trans_node);

					/* switch to ack pending state */
					trans_node->trans_status = DSC_ACK_PENDING;
					ret = 0;
				}
				pthread_mutex_unlock(&trans_node->mutex);

				free_dsc_rsp(dsc_rsp);
#ifndef SS_RX
				free(dsx_rvd);
				free(dsx_rcv_payload);
#endif
			}
			else
			{
				free(dsc_rsp);
				free(dsc_rsp_payload);
#ifndef SS_RX
				free(dsx_rvd);
				free(dsx_rcv_payload);
#endif
			}
			break;
		case DSC_ACK_PENDING:
#ifndef SS_RX
			dsx_rvd = (dsx_rvd_msg*)malloc(sizeof(dsx_rvd_msg));
			dsx_rcv_payload = (char *)malloc(DSX_RVD_LEN);
			if ((dsx_rvd != NULL) && (dsx_rcv_payload != NULL))
			{
				pthread_mutex_lock(&trans_node->mutex);
				
				/* build dsc rvd */
				init_dsx_rvd((dsa_req_msg *)dsc_req, dsx_rvd);
				dsx_rvd->cfm_code = CC_SUCCESS;
				build_dsx_rvd (dsx_rvd, dsx_rcv_payload, &length);
				
				/* send dsc rvd */
				mac_payload = (char *)mac_sdu_malloc(length, MMM_CLASS_TYPE);
				if (mac_payload != NULL)
				{
					memcpy(mac_payload, dsx_rcv_payload, length);
					enqueue_transport_sdu_queue(dl_sdu_queue, trans_node->peer_primary_cid, \
														length, mac_payload);
				}

				/* send dsc response again */
				mac_payload = (char *)mac_sdu_malloc(trans_node->re_msg.length, MMM_CLASS_TYPE);
				if (mac_payload != NULL)
				{
					memcpy(mac_payload, trans_node->re_msg.msg, trans_node->re_msg.length);

					/* enqueue the DSC RSP on the Primary MM CID of the sender */
					enqueue_transport_sdu_queue(dl_sdu_queue, \
												trans_node->peer_primary_cid, length, mac_payload);
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
			ret = 0;
#endif
			break;
		default:
			FLOG_INFO("%s: unexpected dsc request, current state: %s", \
				__FUNCTION__, dsc_state_to_string(trans_node->trans_status));
			break;
	}

	return ret;
}

extern void update_svc_flow_to_ss(serviceflow *old_flow, serviceflow *new_flow);
static void dsc_update_sf(struct transaction_node *trans_node)
{
	struct service_flow		*new_sf_node;
	struct service_flow		*old_sf_node;
	sf_result				sf_change_result;
	
	if (trans_node != NULL)
	{
		FLOG_INFO("----++++ %s enter: sf direction = %s sfid = %d ++++----\n", \
			__FUNCTION__, trans_node->sf->sf_direction == UL ? "UL" : "DL", trans_node->sf->sfid);
		old_sf_node = find_sf_in_peer(trans_node->sf->sfid, trans_node->peer_mac);
		new_sf_node = trans_node->sf;

		if ((old_sf_node != NULL) && (new_sf_node != NULL))
		{
			update_svc_flow_to_ss(old_sf_node, new_sf_node);
				
			if (trans_node->notify != NULL)
			{
				sf_change_result.peer_mac = trans_node->peer_mac;
				sf_change_result.cfm_code = CC_SUCCESS;
				sf_change_result.sf_id = new_sf_node->sfid;
				(*trans_node->notify)(&sf_change_result);
				trans_node->notify = NULL;
			}
		}
	}
}

static int dsc_sm_dsc_rsp_handler(dsc_rsp_msg *dsc_rsp, struct transaction_node* trans_node)
{
	int						length;
	char					*mac_payload;
	int						ret;
	unsigned long long 		current_time;
	unsigned long long 		firing_time;
	u_int64_t				mac_addr;
	dsc_ack_msg				*dsc_ack;
	char					*dsc_ack_payload;
	
	assert(dsc_rsp != NULL);
	assert(trans_node != NULL);

	ret = -1;
	switch (trans_node->trans_status)
	{
		case DSC_RSP_PENDING:
			/* pass down */
		case DSC_LOCAL_RETRIES_EXHAUSTED:
			/* pass down */
		case DSC_LOCAL_HOLDING_DOWN:
			dsc_ack = (dsc_ack_msg*)malloc(sizeof(dsc_ack_msg));
			dsc_ack_payload = (u_char*)malloc(MAX_DSC_ACK_LEN);
			if ((dsc_ack != NULL) && (dsc_ack_payload != NULL))
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

				/* delete T14 if T14 active */
				if(trans_node->t14_timer != NULL) 
				{
					app_timer_delete(trans_node->t14_timer);
					trans_node->t14_timer = NULL;
				}
				
				if (trans_node->trans_status == DSC_RSP_PENDING || trans_node->re_msg.msg == NULL)
				{
					/* release dsc request */
					if (trans_node->trans_status == DSC_RSP_PENDING)
					{
						free(trans_node->re_msg.msg);
						trans_node->re_msg.msg = NULL;
					}
					
					/* build dsc acknowledge */
					memset(dsc_ack, 0, sizeof(dsc_ack_msg));
					init_dsc_ack(dsc_rsp, dsc_ack);
					build_dsc_ack(dsc_ack, dsc_ack_payload, &length);
					
					/* save the dsc ack */
					trans_node->re_msg.type = DSC_ACK;
					trans_node->re_msg.retrytimes = 0;
					trans_node->re_msg.msg = dsc_ack_payload;
					trans_node->re_msg.length = length;

					free_dsc_ack(dsc_ack);
				}
				else
				{
					free(dsc_ack);
					free(dsc_ack_payload);
				}
				
				if ((trans_node->trans_status == DSC_RSP_PENDING) && (dsc_rsp->cfm_code == CC_SUCCESS))
				{
					dsc_rsp_to_sf(dsc_rsp, trans_node->sf);
					dsc_update_sf(trans_node);
				}

				/* send the dsc ack */
				mac_payload = (char *)mac_sdu_malloc(trans_node->re_msg.length, MMM_CLASS_TYPE);
				if (mac_payload != NULL)
				{
					memcpy(mac_payload, trans_node->re_msg.msg, trans_node->re_msg.length);
					enqueue_transport_sdu_queue(dl_sdu_queue, trans_node->peer_primary_cid,\
														trans_node->re_msg.length, mac_payload);

					/* switch to holding down state*/
					trans_node->trans_status = DSC_LOCAL_HOLDING_DOWN;
						
					/* make T10 active*/
					current_time = readtsc(); 
					firing_time = current_time + T10_DURATION;
					app_timer_add(firing_time, (timeout_action)&dsc_t10_timeout, NULL, NULL, \
										0, 0, (void **)&trans_node->t10_timer, (void*)trans_node);

					ret = 0;
				}
				pthread_mutex_unlock(&trans_node->mutex);
			}
			else
			{
				free(dsc_ack);
				free(dsc_ack_payload);
			}
			break;
		default:
			FLOG_INFO("unexpected dsc response, current state: %s\n", \
					dsc_state_to_string(trans_node->trans_status));
			break;
	}

	return ret;
}

int dsc_sm_dsx_rvd_handler(dsx_rvd_msg *dsx_rvd, struct transaction_node* trans_node)
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

int dsc_sm_dsc_ack_handler(dsc_ack_msg *dsc_ack, struct transaction_node* trans_node)
{
	int 					ret;
	unsigned long long 		current_time;
	unsigned long long 		firing_time;
	struct service_flow		*sf_node;
	
	assert(dsc_ack != NULL);
	assert(trans_node != NULL);

	ret = -1;
	switch (trans_node->trans_status)
	{
		case DSC_ACK_PENDING:
			pthread_mutex_lock(&trans_node->mutex);
			if (trans_node->t8_timer != NULL)
			{
				app_timer_delete(trans_node->t8_timer);
				trans_node->t8_timer = NULL;
			}

			if (dsc_ack->cfm_code == CC_SUCCESS)
			{
				/* update the service flow parameter */
				dsc_update_sf(trans_node);
			}

			/* switch to dsc remote holding down state */
			trans_node->trans_status = DSC_REMOTE_HOLDING_DOWN;

			/* make T10 active*/
			current_time = readtsc(); 
			firing_time = current_time + T10_DURATION;
			app_timer_add(firing_time, (timeout_action)dsc_t10_timeout, NULL, NULL, \
							0, 0, (void **)&trans_node->t10_timer, (void*)trans_node);
			pthread_mutex_unlock(&trans_node->mutex);

			ret = 0;
			break;
		default:
			FLOG_INFO("unexpected dsc ack, current state: %s\n", \
							dsc_state_to_string(trans_node->trans_status));
			break;
	}

	return ret;
}

/*
  * dsc_msg_handler - process the dsc messages
  * @dsc_msg: dsc messae
  * @msg_type: message type
  * @trans_node: transaction node
  *
  * The API is used to process the dsc messages, which is from the peer.
  *
  * Return:
  *		0 if successful
  *		otherwise if error happened
  */

int dsc_msg_handler(void *dsc_msg, int msg_type, struct transaction_node *trans_node)
{
	int 		ret;
	dsc_req_msg	*dsc_req;
	dsc_rsp_msg	*dsc_rsp;
	dsc_ack_msg	*dsc_ack;
	dsx_rvd_msg	*dsx_rvd;
	
	assert(dsc_msg != NULL);
	assert((msg_type == DSC_REQ) || (msg_type == DSC_RSP) \
			|| (msg_type == DSC_ACK) || (msg_type == DSX_RVD));
	assert(trans_node != NULL);

	ret = 0;
	switch (msg_type)
	{
		case DSC_REQ:
			dsc_req = (dsc_req_msg *)dsc_msg;
			ret = dsc_sm_dsc_req_handler(dsc_req, trans_node);
			break;
		case DSC_RSP:
			dsc_rsp = (dsc_rsp_msg *)dsc_msg;
			ret = dsc_sm_dsc_rsp_handler(dsc_rsp, trans_node);
			break;
		case DSC_ACK:
			dsc_ack = (dsc_ack_msg *)dsc_msg;
			ret = dsc_sm_dsc_ack_handler(dsc_ack, trans_node);
			break;
		case DSX_RVD:
			dsx_rvd = (dsx_rvd_msg *)dsc_msg;
			ret = dsc_sm_dsx_rvd_handler(dsx_rvd, trans_node);
			break;
		default:
			ret = -1;
			break;
	}

	return ret;
}

