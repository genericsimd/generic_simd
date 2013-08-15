/* ----------------------------------------------------------------------------
   IBM Confidential

   IBM Wireless M2M Platform   

   (C)Copyright IBM Corp. 2008, 2009, 2010, 2011

   All Rights Reserved.

   File Name: mac_serviceflow.c

   Change Activity:

   Date             Description of Change                   By
   -----------      ---------------------					--------
   1-Oct.2008       Created                                 Malolan Chetlur

   ----------------------------------------------------------------------------
   PROLOG END TAG zYx                                                         */

#include "mac_serviceflow.h"
#include "phy_params.h"
#include "mac_connection.h"

int get_min_rsvrd_transfer_rate_per_frame(int cid) {
  //ToDo: Uncomment the following code during integration
  serviceflow* svcflow=NULL;
  get_service_flow(cid, &svcflow);
  if(svcflow!=NULL) {
    int trnsfr_rate=svcflow->min_reserved_traffic_rate;
    int trnsfr_rate_per_frame=trnsfr_rate*frame_duration[FRAME_DURATION_CODE]/1000;
    return trnsfr_rate_per_frame;
  }
  else {
    //some default value
    return 30;
  }
}

int get_min_rsvd_transfer_rate(int cid) {
  serviceflow* svcflow=NULL;
  get_service_flow(cid, &svcflow);
  if(svcflow!=NULL) {
    int trnsfr_rate=svcflow->min_reserved_traffic_rate;
    return trnsfr_rate;
  }
  else {
    //some default value
    FLOG_WARNING("No service flow structure found for this CID\n");
    return -1;
  }
}

int print_sf(serviceflow *sf)
{
	FLOG_INFO("%d, %d, %d, %s, %d, %d, %d, %d, %d, %d, %d, %d, %d, %d, %d, %d, %d, %d, %p\n", sf->sfid, sf->trans_id, sf->cid, sf->service_class_name, sf->qos_param_set_type, sf->schedule_type, sf->br_trans_plc, sf->traffic_priority, sf->max_sustained_traffic_rate, sf->max_traffic_burst, sf->min_reserved_traffic_rate, sf->tolerated_jitter, sf->max_latency, sf->unsolicited_grant_interval, sf->unsolicited_polling_interval, sf->cs_specific_parameter, sf->sdu_inter_arrival_interval, sf->sf_direction, sf->next);
/*	printf("SFID: %d, trans_id: %d, CID: %d\n", sf->sfid, sf->trans_id, sf->cid);
	printf("Service Class name: %s\n", sf->service_class_name);
	printf("QoS type: %d, Scheduling Type: %d, BR trans PLC: %d\n", sf->qos_param_set_type, sf->schedule_type, sf->br_trans_plc);
	printf("Traffic priority: %d, Max sust rate; %d, max traffic burst: %d\n", sf->traffic_priority, sf->max_sustained_traffic_rate, sf->max_traffic_burst);
	printf("Min rsvd rate: %d, Tolerated jitter: %d, max latency: %d\n", sf->min_reserved_traffic_rate, sf->tolerated_jitter, sf->max_latency);
	printf("UG interval: %d, UP interval: %d, CS param: %d, SDU interval: %d, Direction: %d, Next: %x\n", sf->unsolicited_grant_interval, sf->unsolicited_polling_interval, sf->cs_specific_parameter, sf->sdu_inter_arrival_interval, sf->sf_direction, sf->next);*/
	return 0;
}
