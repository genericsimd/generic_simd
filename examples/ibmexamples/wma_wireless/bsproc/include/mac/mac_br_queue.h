/* ----------------------------------------------------------------------------
   IBM Confidential

   IBM Wireless M2M Platform   

   (C)Copyright IBM Corp. 2008, 2009, 2010, 2011

   All Rights Reserved.

   File Name: mac_br_queue.h

   Change Activity:

   Date             Description of Change                   By
   -----------      ---------------------					--------
   1-Oct.2008       Created                                 Chen Lin

   ----------------------------------------------------------------------------
   PROLOG END TAG zYx                                                         */

#ifndef __MAC_BR_QUEUE_H__
#define __MAC_BR_QUEUE_H__

#include <stdlib.h>
#include "string.h"
#include <pthread.h>
#include "ul_mgt_msg_queue.h"

typedef struct brcidqueue br_cid_queue;

struct brcidqueue{
   int cid;
   int ul_bw;
   int dl_bw;
   mgt_msg* br_msg_head;
   mgt_msg* br_msg_tail;
   br_cid_queue* next;
   pthread_mutex_t brcidq_mutex;
};

typedef struct {
    int num_cids;
    br_cid_queue* brcidq_head;
    br_cid_queue* brcidq_tail;
    //pthread_mutex_t brq_mutex;
}ul_br_queue;

ul_br_queue* brqueue;

int initialize_br_queue(ul_br_queue** brq);

int get_br_queue(ul_br_queue** brq);

int initialize_br_cid_queue(br_cid_queue** br_cidq, int cid);

int enqueue_br_queue(ul_br_queue* brq, unsigned int frame_num, int cid, int msg_type, int length, void* data);

int dequeue_br_cid_queue(ul_br_queue* brq, int cid, mgt_msg** br_msgs);

int release_br_queue(ul_br_queue* brq);

int release_br_cid_queue(ul_br_queue* brq, int cid);

#endif

