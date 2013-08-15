/* ----------------------------------------------------------------------------
   IBM Confidential

   IBM Wireless M2M Platform   

   (C)Copyright IBM Corp. 2011

   All Rights Reserved.

   File Name: mac_sf_api.h

   Change Activity:

   Date                      Description of Change                   By
   -----------      --------------------- 		--------
   02-Feb.2012		Created                          	Xianwei. Yi

   ----------------------------------------------------------------------------
   PROLOG END TAG zYx                                                         */

#ifndef _MAC_SF_API_H_
#define _MAC_SF_API_H_

#include <stdint.h>
#include "mac.h"
#include "bs_ss_info.h"
#include "mac_serviceflow.h"
#include "mac_qos_mm.h"

typedef struct
{
	u_int64_t		peer_mac;	/* mac address of peer */
	u_int16_t 		sf_id;		/* service flow id */
	u_int8_t 		cfm_code;	/* confirmation code, please see wimax spec's table 599 for its meaning */
} sf_result;

/* callback for service flow transactions, It is better to use it to post the result back to APP using a message  */
typedef void (*sf_notify)(sf_result *result);

typedef struct
{
	bs_ss_info				*peer;	/* peer ss information structure */
	struct service_flow		*sf;	/* service flow */
	sf_notify				notify;	/* service flow transaction callback function */
} sf_dsx_param;

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
);

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
);

int sf_delete
(
	u_int64_t		mac_addr,
	int 			sf_id, 
	sf_notify		notify
);

#endif
