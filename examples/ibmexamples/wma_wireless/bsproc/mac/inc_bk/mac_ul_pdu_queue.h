/* ----------------------------------------------------------------------------
   IBM Confidential

   IBM Wireless M2M Platform   

   (C)Copyright IBM Corp. 2008, 2009, 2010, 2011

   All Rights Reserved.

   File Name: mac_ul_pdu_queue.h

   Change Activity:

   Date             Description of Change                       By
   -----------      ---------------------                       --------
   03-Aug.2008		Created                                     Chen Lin

   ----------------------------------------------------------------------------
   PROLOG END TAG zYx                                                         */


#ifndef __MAC_UL_PDU_QUEUE_H__
#define __MAC_UL_PDU_QUEUE_H__
#include "stdio.h"
#include "stdlib.h"
#include "mac_config.h"
#include "mac_frame.h"
typedef struct pducidqueue pdu_cid_queue; 
struct pducidqueue{
    int cid;
    int pdu_num;
    u_int8_t is_processed;
    logical_element *head;
    logical_element *tail;
    pdu_cid_queue* next;
};

typedef struct pduframequeue pdu_frame_queue;
struct pduframequeue{
    int frame_no;
    int num_cids;
    pdu_cid_queue* pdu_cid_q_head;
    pdu_frame_queue* next;
};

typedef struct mac_pdu_queue pdu_queue;

struct mac_pdu_queue {
    int frame_num;
    pdu_frame_queue* pdu_frame_q_head;
    pdu_frame_queue* pdu_frame_q_tail;
    pdu_queue* next;
};

// extern pdu_queue* ul_pduq_header;

int initialize_pduq(pdu_queue** ul_pduq);

int initialize_pduframeq(pdu_frame_queue** ul_framepduq, int frame_no);

int initialize_pducidq(int cid, pdu_cid_queue** cidq);

int get_pducidq(pdu_frame_queue* pduframeq, int cid, pdu_cid_queue** ul_pducidq);

int enqueue_pducidq(pdu_cid_queue* ul_pducidq, logical_element* pdu);

int enqueue_pduq(pdu_queue*ul_pduq, pdu_frame_queue* ul_framepduq);

int dequeue_pduq(pdu_queue* ul_pduq_header, pdu_frame_queue** ul_pduframeq);

// remove the parsed logical element from this queue.
int release_pducidq(pdu_cid_queue* ul_pducidq);

int release_pduframeq(pdu_frame_queue * pduframeq);

// only release the pdu cid queue within this queue, not the payload
int release_pduq(pdu_queue* ul_pduq);

#endif

