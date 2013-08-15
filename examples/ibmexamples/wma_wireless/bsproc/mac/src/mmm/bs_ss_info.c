/* ----------------------------------------------------------------------------
   IBM Confidential

   IBM Wireless M2M Platform   

   (C)Copyright IBM Corp. 2008, 2009, 2010, 2011

   All Rights Reserved.

   File Name: bs_ss_info.c

   Change Activity:

   Date             Description of Change                   By
   -----------      ---------------------					--------
   1-Oct.2008       Created                                 Malolan Chetlur

   ----------------------------------------------------------------------------
   PROLOG END TAG zYx                                                         */

#include "mac.h"
#include "bs_ss_info.h"
#include "mac_connection.h"
#include "debug.h"

#include <pthread.h>
#include <assert.h>
pthread_mutex_t bs_ss_list_mutex = PTHREAD_MUTEX_INITIALIZER;

//Global pointer to list of bssInfo structures
bs_ss_info* ssinfo_list_head=NULL;

int add_bs_ss_info(bs_ss_info* ss_info) {
  ss_info->next=ssinfo_list_head;
  if (ssinfo_list_head == NULL)
    {
      // If first element in the list
      ss_info->ss_index = 0;
    }
  else
    {
      ss_info->ss_index = ssinfo_list_head->ss_index + 1;
    }
  ssinfo_list_head=ss_info;
/*  for (ii = 0; ii < sizeof(bs_ss_info); ii++)
  {
	printf("%d ", *((u_char*)ss_info+ii));
  }*/
  return 0;
}

bs_ss_info* find_bs_ss_info(u_int64_t mac_addr) {
  bs_ss_info* ss_info=ssinfo_list_head; 
  while(ss_info!=NULL) {
    if(ss_info->mac_addr == mac_addr) {
      return ss_info;
    }
    ss_info=ss_info->next;
  }
  FLOG_DEBUG("SS for a given mac_addr not found!\n");
  return NULL;
}

u_int16_t get_basic_cid_from_ss_index(int ss_index) {
  bs_ss_info* ss_info=ssinfo_list_head;
  while(ss_info!=NULL) {
    if(ss_info->ss_index == ss_index) {
      return ss_info->basic_cid;
    }
    ss_info=ss_info->next;
  }
  FLOG_WARNING("SS for a given mac_addr not found!\n");
  return -1;
}

u_int64_t get_macaddr_from_pcid(int prim_cid)
{
	bs_ss_info* ss_info=ssinfo_list_head;
	while (ss_info!=NULL)
	{
		if (ss_info->primary_cid == prim_cid)
		{
			return ss_info->mac_addr;
		}
		ss_info=ss_info->next;
	}
	return -1;
}

u_int64_t get_macaddr_from_basic_cid(int basic_cid)
{
	bs_ss_info* ss_info=ssinfo_list_head;
	while (ss_info!=NULL)
	{
		if (ss_info->basic_cid == basic_cid)
		{
			return ss_info->mac_addr;
		}
		ss_info=ss_info->next;
	}
	return -1;
}

bs_ss_info* get_ssinfo_from_pcid(int prim_cid)
{
	bs_ss_info* ss_info=ssinfo_list_head;
	while (ss_info!=NULL)
	{
		if (ss_info->primary_cid == prim_cid)
			{return ss_info;}
		ss_info=ss_info->next;
	}
	return NULL;
}

bs_ss_info* get_ssinfo_from_ssindex(int ss_index)
{
	bs_ss_info* ss_info=ssinfo_list_head;
	while (ss_info!=NULL)
	{
		if (ss_info->ss_index == ss_index)
			{return ss_info;}
		ss_info=ss_info->next;
	}
	return NULL;
}

int delete_sf(serviceflow *sf_to_del, bs_ss_info *ssinfo)
{
	if (ssinfo == NULL) FLOG_WARNING("ss info is null\n");
	serviceflow *sf = ssinfo->sf_list_head;
	serviceflow *prev_sf = NULL;
	while(sf != NULL)
	{
		if(sf == sf_to_del)
		{
			if (prev_sf == NULL)
			{
				ssinfo->sf_list_head = sf->next;
			}
			else
			{
				prev_sf->next = sf->next;
			}
			if(sf->service_class_name != NULL) {free(sf->service_class_name);}
			free(sf);
			break;
		}
		prev_sf = sf;
		sf = sf->next;
	}
	return 0;
}

int add_svc_flow_to_bs(serviceflow* sflow, u_int64_t mac_addr) {
  bs_ss_info* ss_info=find_bs_ss_info(mac_addr);
  if(ss_info!=NULL) {
    sflow->next=ss_info->sf_list_head;
    ss_info->sf_list_head=sflow;
    return 0;
  }
  else {
    return -1; //ss info not found
  }
}

void add_svc_flow_to_ss(serviceflow* sflow, bs_ss_info* ss_info) 
{
	assert(sflow != NULL);
	assert(ss_info != NULL);

	pthread_mutex_lock(&bs_ss_list_mutex);
	sflow->next = ss_info->sf_list_head;
	ss_info->sf_list_head = sflow;
	pthread_mutex_unlock(&bs_ss_list_mutex);
}

void update_svc_flow_to_ss(serviceflow *old_flow, serviceflow *new_flow) 
{
	char 	*service_class_name;
	
	assert(old_flow != NULL);
	assert(new_flow != NULL);

	free(old_flow->service_class_name);

	pthread_mutex_lock(&bs_ss_list_mutex);
	old_flow->trans_id = new_flow->trans_id;
	
	old_flow->qos_param_set_type = new_flow->qos_param_set_type;
	old_flow->schedule_type = new_flow->schedule_type;
	old_flow->br_trans_plc = new_flow->br_trans_plc;

	old_flow->service_class_name = new_flow->service_class_name;
	new_flow->service_class_name = NULL;

	old_flow->traffic_priority = new_flow->traffic_priority;
	old_flow->max_sustained_traffic_rate = new_flow->max_sustained_traffic_rate;
	old_flow->max_traffic_burst = new_flow->max_traffic_burst;
	old_flow->min_reserved_traffic_rate = new_flow->min_reserved_traffic_rate;
	old_flow->tolerated_jitter = new_flow->tolerated_jitter;
	old_flow->max_latency = new_flow->max_latency;
	old_flow->unsolicited_grant_interval = new_flow->unsolicited_grant_interval;
	old_flow->unsolicited_polling_interval = new_flow->unsolicited_polling_interval;
	old_flow->cs_specific_parameter = new_flow->cs_specific_parameter;
	old_flow->sdu_inter_arrival_interval = new_flow->sdu_inter_arrival_interval;
	old_flow->sf_direction = new_flow->sf_direction;

	pthread_mutex_unlock(&bs_ss_list_mutex);
}


int associate_conn_to_bs(connection* conn, u_int64_t mac_addr) {
  bs_ss_info* ss_info=find_bs_ss_info(mac_addr);
  //printf("checkABC %x \n", ss_info);
  if(ss_info!=NULL) {
    conn->owner=ss_info;
    return 0;
  }
  else {
    conn->owner=NULL;
    return -1; //ss info not found
  }

}

