/* ----------------------------------------------------------------------------
   IBM Confidential

   IBM Wireless M2M Platform   

   (C)Copyright IBM Corp. 2008, 2009, 2010, 2011

   All Rights Reserved.

   File Name: mac_serviceflow.h

   Change Activity:

   Date             Description of Change                   By
   -----------      ---------------------					--------
   1-Oct.2008       Created                                 Partha Dutta

   ----------------------------------------------------------------------------
   PROLOG END TAG zYx                                                         */

#ifndef __MAC_SERVICEFLOW_H__
#define __MAC_SERVICEFLOW_H__

#include "mac_config.h"
#include "mac.h"
#define TEMPSFID 0

typedef struct service_flow serviceflow;

struct service_flow {
    int sfid; //Service Flow ID
    int trans_id; 
    int cid;
    char* service_class_name;
    SfStatus qos_param_set_type;//Provisioned, Admitted or Active

    SchedulingType schedule_type; //Service_Flow_Scheduling_Type
    int br_trans_plc;

     //QoS_Parameter_Set_Type
    int traffic_priority; // value 0 to 7, default 0, bigger => higher priority
    int max_sustained_traffic_rate; // in bits per sec, length: 4 bytes
    int max_traffic_burst; // size in bytes, value number can occupy upto 4 bytes
    int min_reserved_traffic_rate; // in Bytes per sec, length: 4 bytes
    int tolerated_jitter; // maximum delay variation for the connection. in ms

	// maximum interval between the entry of a packet at CS and the reception
	// of the packet at the peer CS. In millisec
    int max_latency;
    int unsolicited_grant_interval; // In millisec
    int unsolicited_polling_interval;

    int cs_specific_parameter; // packet type, eg. ipv4, ipv6, thernet, atm
    int sdu_inter_arrival_interval;

    SfDirection sf_direction;
    serviceflow *next;
};


// the following function not defined anywhere
//hence cmmenting it out
//int get_serviceflow_info(int sfid, serviceflow* sf);

// returns minimum reserved transfer rate per frame for a cid
extern int get_min_rsvrd_tansfer_rate_per_frame(int cid);
int get_min_rsvd_transfer_rate(int cid) ;
int print_sf(serviceflow *sf);

#endif

