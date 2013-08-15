/* ----------------------------------------------------------------------------
   IBM Confidential

   IBM Wireless M2M Platform   

   (C)Copyright IBM Corp. 2008, 2009, 2010, 2011

   All Rights Reserved.

   File Name: mac_subframe_queue.h

   Change Activity:

   Date             Description of Change                       By
   -----------      ---------------------                       --------
   3-Aug.2008       Created                                     Chen Lin

   ----------------------------------------------------------------------------
   PROLOG END TAG zYx                                                         */

#ifndef __MAC_SUBFRAME_QUEUE_H__
#define __MAC_SUBFRAME_QUEUE_H__

#include <pthread.h>
#include <stdlib.h>
#include "mac_frame.h"
#include "debug.h"
typedef struct {
    int overall_frame_num;
    physical_subframe* subframe_head;
    physical_subframe* subframe_tail;
    pthread_mutex_t qmutex;
    pthread_cond_t notempty;
}subframe_queue;

subframe_queue* dl_subframe_queue;
subframe_queue* ul_subframe_queue;

int initialize_subframe_queue(subframe_queue** subframeq, u_int8_t is_dl);

int initialize_subframe(physical_subframe** phy_subframe);

int dequeue_subframe(subframe_queue* subframeq, physical_subframe** phy_subframe);

int enqueue_subframe(subframe_queue* subframeq, physical_subframe* phy_subframe);

int get_subframe_queue(u_int8_t is_dl, subframe_queue** subframeq);

int release_subframe(physical_subframe* phy_subframe);

int release_subframe_queue(subframe_queue* subframeq, u_int8_t is_dl);
#endif
