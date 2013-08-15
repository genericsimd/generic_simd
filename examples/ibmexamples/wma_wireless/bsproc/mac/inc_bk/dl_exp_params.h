/* ----------------------------------------------------------------------------
   IBM Confidential

   IBM Wireless M2M Platform   

   (C)Copyright IBM Corp. 2008, 2009, 2010, 2011

   All Rights Reserved.

   File Name: dl_exp_params.h

   Change Activity:

   Date             Description of Change                   By
   -----------      ---------------------					--------
   1-Oct.2008       Created                                 Partha Dutta

   ----------------------------------------------------------------------------
   PROLOG END TAG zYx                                                         */

#ifndef _DL_EXP_PARAMS_H_
#define _DL_EXP_PARAMS_H_

#include "mac.h"
#include "mac_serviceflow.h"
#include "bs_ss_info.h"

#define NUM_PARAM 8

//begin dl_mme_gen params

// IMPORTANT: Note that in the current generator, all types have same delay and num (as given in the next two lines)
extern int param_DL_MM_DELAY;    // in ms - delay between two set of mgt_msg
extern int param_DL_EXP_DURATION;    //  total time for DL experiment in ms: number of messages sent = duration/delay (very rough estimate)
extern int param_DL_MM_MAX_SIZE; // maximum size of mgt msg
extern int param_DL_MM_NUM_TYPE_PER_CID; // max number of msg msg type sent per CID
extern int param_DL_MM_MIN_TYPE;  // what is the minimum type number of any mgt msg
extern int param_DL_MM_NUM_BWR; // Number of Bandwidth Requests generated in every message generator cycle

extern int param_DL_ARQ_WINDOW_SIZE; // ARQ Window sixe
extern int param_DL_ARQ_BLK_SIZE; // ARQ BLK size
extern int param_DL_ARQ_LOSS_PERCENT; //ARQ loss percent
extern int param_DL_ARQ_CID_PERCENT; // percentage of transport cids (both ugs and be) that are arq enabled - set to 0 to disable arq for all and 100 to enable arq for all, respectively - see the calculations in dl_exp_param_init()
extern int param_DL_IS_FRAG_ENABLED; // is FRAGMENTATION enabled (holds for all UGS and BE cids)
extern int param_DL_IS_PACK_ENABLED; // is PACKING enabled (holds for all UGS and BE cids)

// input configuration file name 
extern char* file_name;

//upper limit of cids that are initialized
extern int param_MAX_VALID_BASIC_CID;

// valid UGS and BE cids per ss
extern int param_MAX_VALID_UGS_PER_SS;
extern int param_MAX_VALID_BE_PER_SS;

extern int param_NUM_ATTACHED_PROCS;
extern int param_DL_MCS;
extern int param_UL_MCS;
extern int param_DL_CBR_PACKET_SIZE;
extern int param_DL_CBR_PACKET_DELAY;

extern int param_FHDC_FLAG;
extern int param_DL_STC_MATRIX_TYPE;
extern int param_UL_STC_MATRIX_TYPE;


extern int free_params();
extern void dl_exp_params_init_from_file();
extern void dl_exp_init();
extern void dl_exp_print();
extern int dl_conn_array_init();

int add_basic_con(int bcid, int ssnum, u_int64_t mac_addr);
int add_primary_con(int cid, int ssnum, u_int64_t mac_addr);
int add_transport_con(int cid, u_int64_t mac_addr, BOOL arq_enabled, serviceflow *sflow, SchedulingType type, SfDirection dir);

serviceflow* dl_serviceflow_init(int sfid, int trans_id, int cid, char* svc_name, SfStatus qos_type, SchedulingType schedule_type, int br_trans_plc, int traffic_priority, int max_sustained_traffic_rate, int max_traffic_burst, int min_reserved_traffic_rate, int tolerated_jitter, int max_latency, int ug_interval, int up_interval, int cs_param, int sdu_interval, SfDirection dir, serviceflow *next);

extern int msleep(unsigned long milisec);
extern int micro_sleep(unsigned long micro_sec);


#endif
