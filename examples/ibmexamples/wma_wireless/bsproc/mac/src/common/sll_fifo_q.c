/* ----------------------------------------------------------------------------
   IBM Confidential

   IBM Wireless M2M Platform   

   (C)Copyright IBM Corp. 2009, 2010, 2011

   All Rights Reserved.

   File Name: sll_fifo_q.c

   Change Activity:

   Date             Description of Change                   By
   -----------      ---------------------					--------
   1-Apr.2009       Created                                 Malolan Chetlur

   ----------------------------------------------------------------------------
   PROLOG END TAG zYx                                                         */

#include <assert.h>
#include "debug.h"
#include "q_container.h"
#include "sll_fifo_q.h"
#include "memmgmt.h"

int sll_fifo_q_enqueue(sll_fifo_q* que, void* data, size_t len, int type) {

  q_container* q_cntr=NULL;
  if(app_malloc((void**)&q_cntr, sizeof(q_container))) {
    return -1;
  }

  //Lock the queue before enqueuing
  pthread_mutex_lock(&(que->qmutex));

  FLOG_DEBUG("sll_fifo_q_enqueue: Enqueueing payload of size %d with address: %p\n", len, data);

  assert(q_cntr!=NULL);
  q_cntr->data=data;
  q_cntr->len=len;
  q_cntr->next=NULL;
  q_cntr->data_type = type;
  //Lock the queue before enqueuing
//  pthread_mutex_lock(&(que->qmutex));

  //If queue is not empty update tail pointer to enqueued logical_packet
  if(que->tail!=NULL) {
    que->tail->next=q_cntr;
  }
  que->tail=q_cntr;

  //increment the number of sdus
  que->q_info->num_elems++;
  que->q_info->num_bytes +=(int32_t)len;

  //If queue were empty, modify head to point to enqueued logical_packet
  if(que->q_info->num_elems==1) {
    que->head=q_cntr;
    //broadcast to waiting threads to resume dequeue
    pthread_cond_broadcast(&(que->qempty_cv));
  }

  //unlock the queue before enqueuing
  pthread_mutex_unlock(&(que->qmutex));
  return 0;
}

int sll_fifo_q_dequeue(sll_fifo_q* que, q_container** data, size_t* len) {
  //Lock the queue before enqueuing
  pthread_mutex_lock(&(que->qmutex));
  assert(que!=NULL);
  assert(que->q_info!=NULL);
  if(que->q_info->num_elems==0) {
    //que is empty
    //unock the queue before exiting
    pthread_mutex_unlock(&(que->qmutex));
    return -1;
  }

  // Changed from the original logic which returned data, now the q_container 
  // at the head of the queue is returned
  *data=que->head;
  *len=que->head->len;

  que->q_info->num_elems--;
  que->q_info->num_bytes -= (*len);
  que->head=que->head->next;
  if (0 == que->q_info->num_elems) {
  	que->tail = NULL;
  }

  pthread_mutex_unlock(&(que->qmutex));

  FLOG_DEBUG("sll_fifo_q_dequeue: Dequeueing payload of size %zu with address: %p\n", (*len), *data);

  return 0;
}

int sll_fifo_q_dequeue_with_wait(sll_fifo_q* que, q_container** data) {
  assert(que!=NULL);
  assert(que->q_info!=NULL);
  //Lock the queue before enqueuing
  pthread_cleanup_push((void*)pthread_mutex_unlock, (void *) &(que->qmutex));
  pthread_mutex_lock(&(que->qmutex));
  //Wait if the queue is empty
  while(que->q_info->num_elems==0) {
    //que is empty
    //unock the queue before enqueuing
    pthread_cond_wait(&(que->qempty_cv), &(que->qmutex));
  }

  *data=que->head;

  que->q_info->num_elems--;
  que->q_info->num_bytes -= que->head->len;
  que->head=que->head->next;
  if (0 == que->q_info->num_elems) {
  	que->tail = NULL;
  }

  pthread_mutex_unlock(&(que->qmutex));
  pthread_cleanup_pop(0);
  return 0;
}

int sll_fifo_q_init(sll_fifo_q** que) {
  if(app_malloc((void**)que, sizeof(sll_fifo_q))) {
    return -1;
  }
  assert(*que!=NULL);
  (*que)->head=NULL;
  (*que)->tail=NULL;

  pthread_mutex_init(&((*que)->qmutex), NULL);
  pthread_cond_init(&((*que)->qempty_cv), NULL);

  //allocate and initialize que->q_info
  if(app_malloc((void**)&((*que)->q_info), sizeof(q_aggr_info))) {
    return -1;
  }
  (*que)->q_info->num_elems=0;
  (*que)->q_info->num_bytes=0;

  return 0;
}

int flush_sll_fifo_q(sll_fifo_q* que) {

  pthread_mutex_lock(&(que->qmutex));

  q_container* qc;
  q_container* next_qc;

  qc = que->head;

  // Free elements in the queue one at a time
  while (qc)
  {
      next_qc = qc->next;
      // free memory for data. TODO: check that there are no
	  // invalid or double frees, or if any need recursive free
	  if (qc-> data != NULL)
		free(qc->data);
      free(qc);
      qc = next_qc;
  }
  que->head=NULL;
  que->tail=NULL;
  que->q_info->num_elems=0;
  que->q_info->num_bytes=0;
  pthread_mutex_unlock(&(que->qmutex));

  return 0;
}


int release_sll_fifo_q(sll_fifo_q* que) {

  pthread_mutex_lock(&(que->qmutex));

  // Free que->q_info
  if(app_free((void*)que->q_info, sizeof(q_aggr_info))) {
    return -1;
  }
  q_container* qc;
  q_container* next_qc;

  qc = que->head;

  // Free elements in the queue one at a time
  while (qc)
  {
      next_qc = qc->next;
      // free memory for data. TODO: check that there are no
	  // invalid or double frees, or if any need recursive free
	  if (qc-> data != NULL)
		free(qc->data);
      free(qc);
      qc = next_qc;
  }

  pthread_mutex_unlock(&(que->qmutex));

  pthread_mutex_destroy(&(que->qmutex));
  pthread_cond_destroy(&(que->qempty_cv));

  if(app_free((void*)que, sizeof(sll_fifo_q))) {
    return -1;
  }

  return 0;
}

int sll_fifo_q_peek(sll_fifo_q* que, void** data, size_t* len) {
  //currently not implemented
  //peeks the head and return its contents
  pthread_mutex_lock(&(que->qmutex));
  if(que->head!=NULL) {
    *data=que->head->data;
    *len=que->head->len;
  }
  else {
    *data=NULL;
    *len=0;
  }
  pthread_mutex_unlock(&(que->qmutex));
  return 0;
}

int get_sll_fifo_q_statistics(sll_fifo_q* que, int16_t* num_elements, int32_t* num_bytes) {
  //Lock the queue before enqueuing
  pthread_mutex_lock(&(que->qmutex));
  *num_elements=que->q_info->num_elems;
  *num_bytes=que->q_info->num_bytes;
  pthread_mutex_unlock(&(que->qmutex));
  return 0;
}
