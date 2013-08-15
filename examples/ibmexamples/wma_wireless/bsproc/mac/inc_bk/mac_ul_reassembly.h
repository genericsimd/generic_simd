/* ----------------------------------------------------------------------------
   IBM Confidential

   IBM Wireless M2M Platform   

   (C)Copyright IBM Corp. 2008, 2009, 2010, 2011

   All Rights Reserved.

   File Name: mac_ul_reassembly.h

   Change Activity:

   Date             Description of Change                       By
   -----------      ---------------------                       --------
   03-Aug.2008		Created                                     Chen Lin

   ----------------------------------------------------------------------------
   PROLOG END TAG zYx                                                         */


#ifndef __MAC_UL_REASSEMBLY_H__
#define __MAC_UL_REASSEMBLY_H__

#include <stdlib.h>
#include "string.h"
#include "mac_ul_pdu_queue.h"
#include "mac_sdu_queue.h"
#include "mac_connection.h"
#include "ul_mgt_msg_queue.h"
#include "mac_headermsg_parser.h"
#include "mac_ul_frag_queue.h"
#include "mac_arq.h"
#include "br_queue.h"
#include "arq_ifaces.h"
#include "mac_br_queue.h"

#define UL_CON_THREAD_NUM 20

typedef struct {
    int frame_num;
    pdu_cid_queue* pdu_list;
    sdu_queue* sduq;
    frag_queue* fragq;
    ul_br_queue* brq;
    mgt_msg_queue *ul_msgq;
    int status;
}ul_con_thrd_args;

typedef struct {
  pthread_t con_thrd;
  pthread_mutex_t con_mutex;
  pthread_cond_t ready_to_process;
  int resume_status;
}ul_con_thrd_info;

int reassembly(pdu_frame_queue* pduqlist, sdu_queue* sduq, frag_queue* fragq, ul_br_queue* brq, mgt_msg_queue *ul_msgq);

int reassembly_per_con(int frame_num, pdu_cid_queue* pdu_list, sdu_queue* sduq, frag_queue* fragq, ul_br_queue* brq, mgt_msg_queue *ul_msgq);
void init_ul_con_threads(void);
void release_ul_con_threads();
#endif
