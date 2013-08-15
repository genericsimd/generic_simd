/* ----------------------------------------------------------------------------
   IBM Confidential

   IBM Wireless M2M Platform

   (C)Copyright IBM Corp. 2011

   All Rights Reserved.

   File Name: mac_dsc_utm.c

   Change Activity:

   Date             Description of Change                   By
   -----------      ---------------------               --------
   14-April.2012       Created                          Xianwei.Yi

   ----------------------------------------------------------------------------
   PROLOG END TAG zYx                                                         */

#include <unistd.h>
#include <pthread.h>
#include <assert.h>
#include <errno.h>

#include "mac.h"
#include "mac_sf_api.h"
#include "mac_connection.h"
#include "flog.h"

extern pthread_mutex_t bs_ss_list_mutex;
void sf_change_callback(sf_result *result)
{
	bs_ss_info              *ss_info;

	assert(result != NULL);
	if (result->cfm_code != CC_SUCCESS)
	{
		FLOG_INFO("peer %lld reject service flow change\n", result->peer_mac);
	}
	else
	{
		FLOG_INFO("peer %lld accept service flow change\n", result->peer_mac);
	}
}

void* mac_dsc_utm_thread(void *arg)
{
	bs_ss_info		*ss_info;
	struct service_flow	*sflow;
	int			ii;
	int			ret;
	int			ss_index;
	
	FLOG_INFO("-------- %s start to run --------\n", __FUNCTION__);

	while (1)
	{
#ifdef SS_RX
		for (ii = UGS_CID_MIN_VALUE; ii <= max_valid_ugs_cid; ii++)
#else
		for (ii = UL_UGS_CID_MIN_VALUE; ii <= max_valid_ul_ugs_cid; ii++)
#endif
		{
			pthread_mutex_lock(&bs_ss_list_mutex);
			get_ss_index(ii, &ss_index);
			get_service_flow(ii, &sflow);
			pthread_mutex_unlock(&bs_ss_list_mutex);

			ss_info = get_ssinfo_from_ssindex(ss_index);
			if ((sflow != NULL) && (ss_info != NULL))
			{
				sleep(3);
				
				FLOG_INFO("-------- start to do dsc test --------\n");	
				ret = sf_change(ss_info->mac_addr, \
								sflow->sfid,
								sflow->service_class_name,
								sflow->schedule_type,
								sflow->br_trans_plc,
								sflow->traffic_priority,
								sflow->max_sustained_traffic_rate,
								sflow->max_traffic_burst,
								sflow->min_reserved_traffic_rate >> 1U,
								sflow->tolerated_jitter,
								sflow->max_latency,
								sflow->unsolicited_grant_interval,
								sflow->unsolicited_polling_interval,
								sflow->cs_specific_parameter,
								sflow->sdu_inter_arrival_interval,
								sf_change_callback);
				if (ret != 0)
				{
					FLOG_ERROR("error in sf_change\n");
				}

				pthread_exit(NULL);
			}
		}

		FLOG_INFO("-------- no ss for dsc test found --------\n");
		sleep(1);
	}

	pthread_exit(NULL);
}

int mac_dsc_utm(void)
{
	pthread_t	dsc_test_thread;
	int			ret;

	ret = pthread_create(&dsc_test_thread, NULL, mac_dsc_utm_thread, NULL);
	if (ret != 0)
	{
		FLOG_INFO("error in pthread_create: %s", strerror(errno));
	}
	else
	{
		FLOG_INFO("succeeded in pthread_create: mac_dsc_utm_thread");
	}
}

