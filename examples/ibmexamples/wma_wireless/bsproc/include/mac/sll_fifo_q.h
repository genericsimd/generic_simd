/* ----------------------------------------------------------------------------
   IBM Confidential

   IBM Wireless M2M Platform   

   (C)Copyright IBM Corp. 2009, 2010, 2011

   All Rights Reserved.

   File Name: sll_fifo_q.h

   Change Activity:

   Date             Description of Change                       By
   -----------      ---------------------                       --------
   1-Apr.2009       Created                                     Malolan Chetlur

   ----------------------------------------------------------------------------
   PROLOG END TAG zYx                                                         */


#ifndef _SLL_FIFO_Q_H_
#define _SLL_FIFO_Q_H_

#include <pthread.h>
#include <stdlib.h>
#include "q_container.h"

typedef struct {
  int num_elems;
  int32_t num_bytes;
} q_aggr_info;

typedef struct sllfifoq sll_fifo_q;
struct sllfifoq {
  q_container* head;
  q_container* tail;
  q_aggr_info* q_info;
  pthread_mutex_t qmutex;
  pthread_cond_t qempty_cv;
};

extern int flush_sll_fifo_q(sll_fifo_q* que);
extern int sll_fifo_q_enqueue(sll_fifo_q* que, void* data, size_t len, int type);
extern int sll_fifo_q_dequeue(sll_fifo_q* que, q_container** data, size_t* len);
extern int sll_fifo_q_dequeue_with_wait(sll_fifo_q* que, q_container** data);
extern int sll_fifo_q_init(sll_fifo_q** que);
extern int release_sll_fifo_q(sll_fifo_q* que);
extern int sll_fifo_q_peek(sll_fifo_q* que, void** data, size_t* len);
extern int get_sll_fifo_q_statistics(sll_fifo_q* que, int16_t* num_elements, int32_t* num_bytes);
#endif //ifndef _SLL_FIFO_Q_H
