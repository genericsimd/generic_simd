/* ----------------------------------------------------------------------------
   IBM Confidential

   IBM Wireless M2M Platform   

   (C)Copyright IBM Corp. 2011

   All Rights Reserved.

   File Name: mac_dsd_processing.c

   Change Activity:

   Date             Description of Change                   		By
   -----------      ---------------------		--------
   1-Mar.2011       	Created                                 	Parul Gupta
   03-Feb.2012	modified for the support of dsd		Xianwei. Yi

   ----------------------------------------------------------------------------
   PROLOG END TAG zYx                                                         */

#include <stdio.h>
#include "mac_dsd_processing.h"
#include "ul_mgt_msg_queue.h"
#include "phy_params.h"

extern pthread_mutex_t bs_ss_list_mutex;
void delete_sf_by_sfid(int sf_id, bs_ss_info *ss_info)
{
	serviceflow *sf;
	serviceflow *prev_sf;

	if (ss_info != NULL)
	{
		pthread_mutex_lock(&bs_ss_list_mutex);
		sf = ss_info->sf_list_head;
		prev_sf = NULL;
		while (sf != NULL)
		{
			if (sf->sfid == sf_id)
			{
				if (prev_sf == NULL)
				{
					ss_info->sf_list_head = sf->next;
				}
				else
				{
					prev_sf->next = sf->next;
				}
				break;
			}
			prev_sf = sf;
			sf = sf->next;
		}
		pthread_mutex_unlock(&bs_ss_list_mutex);

		pthread_rwlock_wrlock(&conn_info_rw_lock);
		FLOG_INFO("-------- will delete cid %d --------\n", sf->cid);
		delete_connection(sf->cid);
		pthread_rwlock_unlock(&conn_info_rw_lock);

		if (sf->service_class_name != NULL)
		{
			free(sf->service_class_name);
		}
		free(sf);
	}
}

/*
  * dsd_t10_timeout - dsc T10 timeout function
  * @arg: pointer to transaction node
  *
  * The API is dsd T10 timeout function,  it delete the connect and serivice flow for the peer device
  *
  */
static int dsd_t10_timeout(void *arg)
{
	struct transaction_node		*trans_node;
	sf_result					sf_delete_result;

	trans_node = (struct transaction_node *)arg;
	
	pthread_mutex_lock(&trans_node->mutex);
	trans_node->t10_timer = NULL;
	if(trans_node->sf != NULL) 
	{
		if (trans_node->notify != NULL)
		{
			if ((trans_node->trans_status == DSD_LOCAL_HOLDING_DOWN) || \
				(trans_node->trans_status == DSD_REMOTE_HOLDING_DOWN))
			{
				sf_delete_result.cfm_code = CC_SUCCESS;
			}
			else
			{
				sf_delete_result.cfm_code = CC_REJECT_OTHER;
			}
			sf_delete_result.sf_id = trans_node->sf->sfid;
			sf_delete_result.peer_mac = trans_node->peer_mac;
			(*trans_node->notify)(&sf_delete_result);
			trans_node->notify = NULL;
		}
		delete_sf_by_sfid(trans_node->sf->sfid, get_ssinfo_from_pcid(trans_node->peer_primary_cid));
		trans_node->sf = NULL;
	}
	pthread_mutex_unlock(&trans_node->mutex);
	
	delete_trans_node(&dsd_trans_list, trans_node);

	return 0;
}

/*
  * dsd_t7_timeout - dsd T7 timeout function
  * @arg: pointer to transaction node
  *
  * The API is dsd T7 timeout function,  it checks if retries are available, if yes, do retry,
  * otherwise terminate the dsc transaction
  *
  * Return
  *		0 if successful
  *		otherwise if error happened
  */
static int dsd_t7_timeout(void *arg)
{
	unsigned long long 		current_time;
	unsigned long long 		firing_time; 
	char 					*mac_payload;
	struct transaction_node *trans_node;
	int						ret;

	assert(arg != NULL);
	
	trans_node = (struct transaction_node*)arg;
	ret = -1;
	
	pthread_mutex_lock(&trans_node->mutex);
	trans_node->t7_timer = NULL;
	
	if (trans_node->re_msg.retrytimes < NUM_DSX_RSP_RETRIES)
	{
		mac_payload = (char *)mac_sdu_malloc(trans_node->re_msg.length, MMM_CLASS_TYPE);
		if (mac_payload != NULL)
		{
			memcpy(mac_payload, trans_node->re_msg.msg, trans_node->re_msg.length);
			
			/* enqueue the DSD request on the Primary MM CID of the sender */
			enqueue_transport_sdu_queue(dl_sdu_queue, trans_node->peer_primary_cid, \
											trans_node->re_msg.length, mac_payload);

			trans_node->re_msg.retrytimes++;

			/* activate T7 timer */
			unsigned long long current_time = readtsc(); 
			unsigned long long firing_time = current_time + T7_DURATION;
			app_timer_add(firing_time, &dsd_t7_timeout, NULL, NULL, 0, 0, \
							(void **)&trans_node->t7_timer, (void*)trans_node);
			ret = 0;

			trans_node->trans_status = DSD_LOCAL_RSP_PENDING;
		}
	}
	else
	{
		/* call DSD-Erred, Ended */
		FLOG_INFO("DSD has timed out for TID: %d. Ending\n", trans_node->trans_id);
		trans_node->trans_status = DSD_ERRED;

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
			app_timer_add(firing_time, (timeout_action)&dsd_t10_timeout, \
									NULL, NULL, 0, 0, (void **)&trans_node->t10_timer, (void*)trans_node);
		}
	}
	pthread_mutex_unlock(&trans_node->mutex);
	
	return ret;
}

/*
  * dsd_t14_timeout - dsd t14 timeout function
  * @arg: point to transaction node
  *
  * The API is used to send dsd request,  it checks if retries are available, if yes, do retry,
  * otherwise terminate the dsd transaction
  *
  */
static int dsd_t14_timeout(void *arg)
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
			trans_node->trans_status = DSD_LOCAL_RSP_PENDING;

			/* if itself is SS, make T14 active */
			current_time = readtsc();
			firing_time = current_time + T14_DURATION;
			app_timer_add(firing_time, (timeout_action)&dsd_t14_timeout, \
							NULL, NULL, 0, 0, (void **)&trans_node->t14_timer, (void*)trans_node);
			ret = 0;
		}
	}
	else
	{
		FLOG_INFO("DSD has timed out for TID: %d. Ending\n", trans_node->trans_id);
		trans_node->trans_status = DSD_ERRED;

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
			app_timer_add(firing_time, (timeout_action)&dsd_t10_timeout, \
						NULL, NULL, 0, 0, (void **)&trans_node->t10_timer, (void*)trans_node);
		}
	}
	pthread_mutex_unlock(&trans_node->mutex);
	
	return ret;
}

/*
  * send_dsd_req - send dsd request
  * @arg: point to transaction node
  *
  * The API is used to send dsd request,  it checks if retries are available, if yes, do retry,
  * otherwise terminate the dsc transaction
  *
  */
static int send_dsd_req(void *arg)
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
		trans_node->trans_status = DSD_LOCAL_RSP_PENDING;

		/* make T7 active */
		current_time = readtsc(); 
		firing_time = current_time + T7_DURATION;
		app_timer_add(firing_time, (timeout_action)&dsd_t7_timeout, \
							NULL, NULL, 0, 0, (void **)&trans_node->t7_timer, (void*)trans_node);


#ifdef SS_TX
		/* if itself is SS, make T14 active */
		firing_time = current_time + T14_DURATION;
		app_timer_add(firing_time, (timeout_action)&dsd_t14_timeout, \
							NULL, NULL, 0, 0, (void **)&trans_node->t14_timer, (void*)trans_node);
#endif
		ret = 0;
	}
	
	pthread_mutex_unlock(&trans_node->mutex);
	
	return ret;
}

/*
  * dsd_primitive_handler - to process the delete primitive of service flow
  * @trans_node: transaction node
  * @primit: primitive
  *
  * The API is used to process the delete primitive of service flow, which is passed down by
  * APP layer
  *
  * Return:
  *		0 if successful
  *		otherwise if error happened
  */
int dsd_primitive_handler(struct transaction_node *trans_node, primitivetype primit)
{ 
   	int 					length;
   	dsd_req_msg				dsd_req;
	char					*dsd_req_payload;
   	long long int 			current_time;
   	long long int 			fire_time;
	int						ret;
   
   	switch(primit)
   	{
    	case SF_DELETE:
			dsd_req_payload = (char *)malloc(MAX_DSD_REQ_LEN);
			if (dsd_req_payload != NULL)
			{
				init_dsd_req(trans_node->sf->sfid, trans_node->trans_id, &dsd_req);
				build_dsd_req(&dsd_req, dsd_req_payload, &length);

				trans_node->re_msg.type = DSD_REQ;
				trans_node->re_msg.retrytimes = 0;
				trans_node->re_msg.msg = dsd_req_payload;
				trans_node->re_msg.length = length;

				ret = send_dsd_req(trans_node);
			}			
			break;
		default:
			ret = -1;
			break;
   	}
	return ret;
}

static int dsd_sm_dsd_req_handler(dsd_req_msg *dsd_req, struct transaction_node* trans_node)
{
	int					length;
	char				*dsd_rsp_payload;
	char				*mac_payload;
	int					ret;
	unsigned long long 	current_time;
	unsigned long long 	firing_time;
	dsd_rsp_msg			dsd_rsp;
		
	assert(dsd_req != NULL);
	assert(trans_node != NULL);

	ret = -1;

	/* build dsc response */
	dsd_rsp_payload = (char *)mac_sdu_malloc(MAX_DSD_RSP_LEN, MMM_CLASS_TYPE);
	if (dsd_rsp_payload != NULL)
	{
		pthread_mutex_lock(&trans_node->mutex);
		init_dsd_rsp(dsd_req, &dsd_rsp);
		dsd_rsp.cfm_code = CC_SUCCESS;
		build_dsd_rsp(&dsd_rsp, dsd_rsp_payload, &length);

		/* delete T10 if T10 active */
		if (trans_node->t10_timer != NULL)
		{
			app_timer_delete(trans_node->t10_timer);
			trans_node->t10_timer = NULL;
		}

		/* send dsd response */
		mac_payload = (char *)mac_sdu_malloc(length, MMM_CLASS_TYPE);
		if (mac_payload != NULL)
		{
			memcpy(mac_payload, dsd_rsp_payload, length);
			
			/* enqueue the DSD RSP on the Primary MM CID of the sender */
			enqueue_transport_sdu_queue(dl_sdu_queue, trans_node->peer_primary_cid, \
													length, mac_payload);

			/* activate T10 timer */
			current_time = readtsc(); 
			firing_time = current_time + T10_DURATION;
			app_timer_add(firing_time, (timeout_action)&dsd_t10_timeout, \
									NULL, NULL, 0, 0, \
									(void **)&trans_node->t10_timer, (void*)trans_node);

			/* switch to dsd succeeded state */
			trans_node->trans_status = DSD_REMOTE_HOLDING_DOWN;
			ret = 0;
		}
		pthread_mutex_unlock(&trans_node->mutex);

		free(dsd_rsp_payload);
	}

	return ret;
}

static int dsd_sm_dsd_rsp_handler(dsd_rsp_msg *dsd_rsp, struct transaction_node* trans_node)
{
	long long int current_time;
	long long int fire_time;

	pthread_mutex_lock(&trans_node->mutex);
	if (trans_node->trans_status == DSD_LOCAL_RSP_PENDING)
	{
		if (trans_node->t7_timer != NULL)
		{
			app_timer_delete((void*)trans_node->t7_timer);
			trans_node->t7_timer = NULL;
		}
#ifdef SS_TX
		if (trans_node->t14_timer != NULL)
		{
			app_timer_delete((void*)trans_node->t14_timer);
			trans_node->t14_timer = NULL;
		}
#endif

		if (trans_node->t10_timer != NULL)
		{
			app_timer_delete((void*)trans_node->t10_timer);
			trans_node->t10_timer = NULL;
		}
		
		current_time = readtsc();
		fire_time = current_time + T10_DURATION;
		app_timer_add(fire_time, &dsd_t10_timeout, NULL, NULL, 0, 1,\
								(void**)&(trans_node->t10_timer), (void*)trans_node);
	}
	trans_node->trans_status = DSD_LOCAL_HOLDING_DOWN;
	pthread_mutex_unlock(&trans_node->mutex);

	return 0;
}

int dsd_msg_handler(void *dsd_msg, int msg_type, struct transaction_node *trans_node)
{
	dsd_req_msg		*dsd_req;
	dsd_rsp_msg		*dsd_rsp;
	int				ret;

	switch(msg_type)
	{
		case DSD_REQ:
			dsd_req = (dsd_req_msg *)dsd_msg;
			ret = dsd_sm_dsd_req_handler(dsd_req, trans_node);
			break;
		case DSD_RSP:
			dsd_rsp = (dsd_rsp_msg *)dsd_msg;
			ret = dsd_sm_dsd_rsp_handler(dsd_rsp, trans_node);
			break;
		default:
			ret = -1;
			break;
	}
	return ret;
}
	
