/* ----------------------------------------------------------------------------
   IBM Confidential

   IBM Wireless M2M Platform   

   (C)Copyright IBM Corp. 2011

   All Rights Reserved.

   File Name: mac_sf_api.c

   Change Activity:

   Date                      Description of Change                   By
   -----------      --------------------- 		--------
   02-Feb.2012		Created                          	Xianwei. Yi

   ----------------------------------------------------------------------------
   PROLOG END TAG zYx                                                         */

#include <stdint.h>
#include <stdlib.h>

#include "mac_sf_api.h"
#include "ul_mgt_msg_queue.h"
#include "mac_dsa_list.h"
#include "mac_qos_mm.h"

extern struct service_flow	* find_sf_in_peer(int sf_id, u_int64_t	peer_mac);
/*
  * sf_add - add a service flow
  * @mac_addr: mac address of the peer
  * @sf_id: service flow id
  * @svc_name: service class name
  * @schedule_type: schedule type(UGS, etrPS ... etc)
  * @br_trans_plc: transmission policy
  * @traffic_priority: traffic priority
  * @max_sustained_traffic_rate: maximum Sustained Traffic Rate
  * @max_traffic_burst: max traffic burst
  * @min_reserved_traffic_rate: min reserved traffic rate
  * @tolerated_jitter: tolerated jitter
  * @max_latency: max latency
  * @ugs_grant_interval: unsolicited grant interval
  * @ugs_polling_interval: unsolicited polling interval
  * @cs_param: convergence layer parameter
  * @sdu_interval: sdu arrival interval
  * @dir: direction, UL or DL
  * @notify: callback function used by caller to get operation result
  *
  * The API is used to add a service flow to a mac peer, caller can use the notify callback
  * to get the result of the operation. Now for the QOS parameters, only schedule type, 
  * min_reserved_traffic_rate, unsolicited_grant_interval and dir take effect
  *
  * Return:
  *		0 if successful
  *		otherwise if error happened
  */
int sf_add
(
	u_int64_t		mac_addr,
	u_int16_t		sf_id,
	char			*svc_name,
	SchedulingType 	schedule_type,
	int 			br_trans_plc, 
	int 			traffic_priority, 
	int 			max_sustained_traffic_rate, 
	int 			max_traffic_burst, 
	int 			min_reserved_traffic_rate, 
	int 			tolerated_jitter, 
	int 			max_latency, 
	int 			ugs_grant_interval, 
	int 			ugs_polling_interval, 
	int 			cs_param, 
	int 			sdu_interval, 
	SfDirection 	dir, 
	sf_notify		notify
)
{
	struct service_flow		*flow;
	sf_dsx_param			*dsx_param;
	int						ret;
	bs_ss_info				*ss_info;

	assert(svc_name != NULL);

	ret = -1;
	dsx_param = (sf_dsx_param *)malloc(sizeof(sf_dsx_param));
	flow = (struct service_flow *)malloc(sizeof(struct service_flow));
	/* use mac address to find the ss */
	ss_info = find_bs_ss_info(mac_addr);
	
	if ((flow != NULL) && (dsx_param != NULL) && (ss_info != NULL))
	{
		memset(flow, 0, sizeof(struct service_flow));
		
		/* fill the dsx parameters */
		dsx_param->peer = ss_info;
		dsx_param->notify = notify;
			 
		flow->service_class_name = (char*)malloc(strlen(svc_name) + 1);
		if (flow->service_class_name != NULL) 
		{
			/* fill the new service flow's data structure */
			strcpy(flow->service_class_name, svc_name);
			flow->sfid = sf_id;
			flow->cid = sf_id;
			flow->qos_param_set_type = (1 << Admitted) | (1 << Active);
			flow->schedule_type = schedule_type;
			flow->br_trans_plc = br_trans_plc;
			flow->traffic_priority = traffic_priority;
			flow->max_sustained_traffic_rate = max_sustained_traffic_rate;
			flow->max_traffic_burst = max_traffic_burst;
			flow->min_reserved_traffic_rate = min_reserved_traffic_rate;
			flow->tolerated_jitter = tolerated_jitter;
			flow->max_latency = max_latency;
			flow->unsolicited_grant_interval = ugs_grant_interval;
			flow->unsolicited_polling_interval = ugs_polling_interval;
			flow->cs_specific_parameter = cs_param;
			flow->sdu_inter_arrival_interval = sdu_interval;
			flow->sf_direction = dir;
			flow->next = NULL;

			dsx_param->sf = flow;
			/* post SF_ADD request to DS management thread */
			enqueue_specific_queue(&ul_msg_queue[DS_MMM_INDEX], \
										get_current_frame_number(), ss_info->primary_cid, SF_ADD, \
										sizeof(sf_dsx_param), (char *)dsx_param);

			ret = 0;
		}
		else
		{
			free(flow);
			free(dsx_param);
		}
	}
	else
	{
		free(flow);
		free(dsx_param);
	}

	return ret;
}

/*
  * sf_change - change a service flow
  * @sfid: service flow id
  * @svc_name: service class name
  * @schedule_type: schedule type(UGS, etrPS ... etc)
  * @br_trans_plc: transmission policy
  * @traffic_priority: traffic priority
  * @max_sustained_traffic_rate: maximum Sustained Traffic Rate
  * @max_traffic_burst: max traffic burst
  * @min_reserved_traffic_rate: min reserved traffic rate
  * @tolerated_jitter: tolerated jitter
  * @max_latency: max latency
  * @ugs_grant_interval: unsolicited grant interval
  * @ugs_polling_interval: unsolicited polling interval
  * @cs_param: convergence layer parameter
  * @sdu_interval: sdu arrival interval
  * @dir: direction, UL or DL
  * @notify: callback function used by caller to get operation result
  *
  * The API is used to change a service flow to a mac peer, caller can use the notify callback
  * to get the result of the operation. Now for the QOS parameters, only min_reserved_traffic_rate 
  * and unsolicited_grant_interval take effect.
  *
  * Return:
  *		0 if successful
  *		otherwise if error happened
  */
int sf_change
(
	u_int64_t		mac_addr,
	int 			sf_id,
	char			*svc_name,
	SchedulingType 	schedule_type,
	int 			br_trans_plc, 
	int 			traffic_priority, 
	int 			max_sustained_traffic_rate, 
	int 			max_traffic_burst, 
	int 			min_reserved_traffic_rate, 
	int 			tolerated_jitter, 
	int 			max_latency, 
	int 			ugs_grant_interval, 
	int 			ugs_polling_interval, 
	int 			cs_param, 
	int 			sdu_interval, 
	sf_notify		notify
)
{
	struct service_flow		*flow;
	int						ret;
	bs_ss_info				*ss_info;
	sf_dsx_param			*dsx_param;

	assert(svc_name != NULL);

	ret = -1;
	
	/* use mac address to find the ss */
	ss_info = find_bs_ss_info(mac_addr);
	if (ss_info != NULL)
	{
		flow = find_sf_in_peer(sf_id, ss_info->mac_addr);
		if (flow != NULL)
		{
			flow = (struct service_flow	 *)malloc(sizeof(struct service_flow));
			dsx_param = (sf_dsx_param *)malloc(sizeof(sf_dsx_param));
			if ((flow != NULL) && (dsx_param != NULL))
			{
				memset(flow, 0, sizeof(struct service_flow));
	
				/* fill the dsx parameters */
				dsx_param->peer = ss_info;
				dsx_param->notify = notify;

				flow->sfid = sf_id;
				flow->cid = sf_id;
				flow->service_class_name = (char*)malloc(strlen(svc_name) + 1);
				if (flow->service_class_name != NULL)
				{
					/* fill the new service flow's data structure */
					strcpy(flow->service_class_name, svc_name);
					flow->qos_param_set_type = (1 << Admitted) | (1 << Active);
					flow->schedule_type = schedule_type;
					flow->br_trans_plc = br_trans_plc;
					flow->traffic_priority = traffic_priority;
					flow->max_sustained_traffic_rate = max_sustained_traffic_rate;
					flow->max_traffic_burst = max_traffic_burst;
					flow->min_reserved_traffic_rate = min_reserved_traffic_rate;
					flow->tolerated_jitter = tolerated_jitter;
					flow->max_latency = max_latency;
					flow->unsolicited_grant_interval = ugs_grant_interval;
					flow->unsolicited_polling_interval = ugs_polling_interval;
					flow->cs_specific_parameter = cs_param;
					flow->sdu_inter_arrival_interval = sdu_interval;
					flow->next = NULL;

					dsx_param->sf = flow;
					/* post SF_CHANGE request to DS management thread */
					enqueue_specific_queue(&ul_msg_queue[DS_MMM_INDEX], \
												get_current_frame_number(), ss_info->primary_cid, SF_CHANGE, \
												sizeof(sf_dsx_param), (char *)dsx_param);

					ret = 0;
				}
				else
				{
					free(flow);
					free(dsx_param);
				}
			}
			else
			{
				free(flow);
				free(dsx_param);
			}
		}
	}
	return ret;
}

/*
  * sf_delete - delete a service flow
  * @mac_addr: peer mac address
  * @sf_id: service flow id
  * @notify: callback function used by caller to get operation result
  *
  * The API is used to delete a service flow
  *
  * Return:
  *		0 if successful
  *		otherwise if error happened
  */
int sf_delete
(
	u_int64_t		mac_addr,
	int 			sf_id, 
	sf_notify		notify
)
{	
	int						ret;
	sf_dsx_param			*dsx_param;
	bs_ss_info				*ss_info;
	sf_result				delete_result;
	struct service_flow		*sf_flow;

	ret = -1;
	/* use mac address to find the ss */
	ss_info = find_bs_ss_info(mac_addr);
	if (ss_info != NULL)
	{
		sf_flow = find_sf_in_peer(sf_id, ss_info->mac_addr);
		if (sf_flow != NULL)
		{
			dsx_param = (sf_dsx_param *)malloc(sizeof(sf_dsx_param));
			if (dsx_param != NULL)
			{
				dsx_param->peer = ss_info;
				dsx_param->sf = sf_flow;
				dsx_param->notify = notify;
				
				/* post SF_DELETE request to DS management thread */
				enqueue_specific_queue(&ul_msg_queue[DS_MMM_INDEX], \
											get_current_frame_number(), ss_info->primary_cid, SF_DELETE, \
											sizeof(sf_dsx_param), (char *)dsx_param);
				ret = 0;
			}
		}
	}

	return ret;
}
