/* ----------------------------------------------------------------------------
   IBM Confidential

   IBM Wireless M2M Platform   

   (C)Copyright IBM Corp. 2008, 2009, 2010, 2011

   All Rights Reserved.

   File Name: mac_ul_cps_controller.h

   Change Activity:

   Date             Description of Change                       By
   -----------      ---------------------                       --------
   03-Aug.2008		Created                                     Chen Lin

   ----------------------------------------------------------------------------
   PROLOG END TAG zYx                                                         */



#ifndef __MAC_UL_CPS_CONTROLLER_H__
#define __MAC_UL_CPS_CONTROLLER_H__

#include <sys/time.h>
#include <unistd.h>
#include <pthread.h>
#include "mac_config.h"
#include "ul_mgt_msg_queue.h"
#include "mac_sdu_queue.h"
#include "mac_ul_frag_queue.h"
#include "mac_ul_pdu_queue.h"
#include "mac_subframe_queue.h"
#include "mac_frame.h"
#include "mac_ul_pdu_parser.h"
#include "mac_ul_reassembly.h"
#include "mac_ss_ranging.h"
#include "mac_br_queue.h"

typedef struct
{
    sdu_queue* ul_sduq;
    frag_queue* fragq;
    pdu_queue * pduq;
    subframe_queue* ul_subframeq;
    ul_br_queue *br_q_list;
    mgt_msg_queue *ul_msgq;

}mac_ul_cps_args;

mac_ul_cps_args ul_arg;
pthread_t ul_parse_thread;

int ul_cps_controller(ul_br_queue* br_q_list);

int release_ul_cps_controller();
#endif
