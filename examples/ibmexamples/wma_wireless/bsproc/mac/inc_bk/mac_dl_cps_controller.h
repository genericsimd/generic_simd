/* ----------------------------------------------------------------------------
   IBM Confidential

   IBM Wireless M2M Platform   

   (C)Copyright IBM Corp. 2008, 2009, 2010, 2011

   All Rights Reserved.

   File Name: mac_dl_cps_controller.h

   Change Activity:

   Date             Description of Change                   By
   -----------      ---------------------					--------
   3-Aug.2008       Created                                 Chen Lin

   ----------------------------------------------------------------------------
   PROLOG END TAG zYx                                                         */


#ifndef __MAC_DL_CPS_CONTROLLER_H__
#define __MAC_DL_CPS_CONTROLLER_H__

#include <pthread.h>
#include <sys/time.h>
#include <unistd.h>
#include "mac_config.h"
#include "mac_scheduling.h"
#include "mac_common_data.h"
#include "mac_sdu_queue.h"
#include "mac_subframe_queue.h"
#include "mac_dl_frag_pack.h"
#include "mac_dl_concatenation.h"
#include "init_maps.h"
//#include "mac_timer_schedulertrigger.h"

extern long int frame_number;


typedef struct 
{
    sdu_queue* dl_sduq;
    subframe_queue* dl_subframeq;
    br_queue** br_q_list;
    // pthread_cond_t* scheduler_call;
    // pthread_mutex_t* scheduler_call_lock;
}mac_dl_cps_args;

mac_dl_cps_args dl_arg;

pthread_t dl_processing_thread;

#define MAX_NUM_BURSTS 35

typedef enum {
  PACK_AND_FRAG=0,
  CONCATENATE=1,
  NOTHING=2
} CpsFunction;

typedef struct {
  //pthread_t burst_thrd;
  //pthread_mutex_t burst_mutex;
  //pthread_cond_t ready_to_process;
  sdu_queue* dl_sduq;
  logical_burst_map* cur_burst;
  logical_packet* pdu_list;
  logical_element* le_tobe_discard;
  phy_burst* phyburst;
  //CpsFunction func;
  int status;
}burst_args;

typedef struct {
  pthread_t burst_thrd;
  pthread_mutex_t burst_mutex;
  pthread_cond_t ready_to_process;
  //sdu_queue* dl_sduq;
  //logical_burst_map* cur_burst;
  //logical_packet* pdu_list;
  //logical_element* le_tobe_discard;
  //phy_burst* phyburst;
  CpsFunction func;
  int resume_status;
}burst_thrd_info;

int dl_cps_controller(br_queue** br_q_list);

int release_dl_cps_controller();

void init_pi_mutex(pthread_mutex_t *m);
int release_burst_threads();
#endif

