/* ----------------------------------------------------------------------------
   IBM Confidential

   IBM Wireless M2M Platform   

   (C)Copyright IBM Corp. 2008, 2009, 2010, 2011

   All Rights Reserved.

   File Name: br_queue.c

   Change Activity:

   Date             Description of Change                   By
   -----------      ---------------------					--------
   1-Oct.2008       Created                                 Parul Gupta

   ----------------------------------------------------------------------------
   PROLOG END TAG zYx                                                         */

#include <pthread.h>
#include <assert.h>
#include "memmgmt.h"
#include "br_queue.h"
#include "mutex_macros.h"
#include "log.h"

br_queue** bwr_q_list=NULL;

//enqueue BR mesg into the br_q
int enqueue_bw_req_queue(br_queue** br_q_list, int cid, int num_bytes, BWR_Type type) {
  br_queue* br_q;
  SchedulingType br_q_index=0;

  // This is the method that maps the CID to the Service Class Type
  get_scheduling_type(cid, &br_q_index);
  // Find the right BR queue to enqueue in
  br_q=br_q_list[br_q_index];

  FLOG_DEBUG("Calling enqueue_bw_req_queue() ...");

  // Lock the queue before searching for br_el
  pthread_mutex_lock(&(br_q->qmutex));

  bw_req_el* br_el=find_br_el(br_q,cid);

  if(br_el!=NULL) {
    update_bw_req(br_q, br_el, num_bytes, type);
  }
  else {
    add_new_bw_req(br_q, cid, num_bytes);
  }

  //Unlock the queue after search for br_el 
  pthread_mutex_unlock(&(br_q->qmutex));  

  FLOG_DEBUG("Finished enqueue_bw_req_queue() ...");
  return 0;
}

// dequeue BR message from br_queue 
int dequeue_br_queue(br_queue* br_q, int num_el) 
{
  int ii=0;

  FLOG_DEBUG("Calling dequeue_br_queue() ...");

  
  if (num_el == 0) {
	FLOG_DEBUG("No elements to dequeue from br_queue. Returning..");
	return 0;
  }

  // Lock the queue before dequeuing
  pthread_mutex_lock(&(br_q->qmutex));

  assert(br_q->br_num>=num_el);
  
  bw_req_el* br_el;

  for(ii=0;ii<num_el;ii++)
    {
	br_el=br_q->head;
	br_q->head=br_q->head->next;
	br_q->head->prev=NULL;
	// Update aggregate info in queue
	br_q->br_num--;
	br_q->total_br-=br_el->bandwidth_requested;
	mac_free(sizeof(br_el), br_el);
    }
  
  pthread_mutex_unlock(&(br_q->qmutex));
  return 0;
}

int cleanup_br_queue(br_queue* br_q)
{
bw_req_el *br_el_temp=NULL;
bw_req_el *br_el=br_q->head;
int ii=0;

for (ii=0;ii<br_q->br_num;ii++)
  {
  if (br_el->bandwidth_requested==0)
	{
	br_el_temp=br_el;

	// update head if the first element is being dequeued
	if (br_el==br_q->head) {br_q->head=br_el->next;} 
	else {br_el->prev->next=br_el->next;}
	
	// update tail if the last element is being dequeued
	if (br_el==br_q->tail) {br_q->tail=br_el->prev; }
	else {br_el->next->prev=br_el->prev;}

	br_el=br_el->next;
	mac_free(sizeof(br_el_temp), br_el_temp);
	br_q->br_num--;
	}
  else {br_el=br_el->next;}
  }
return 0;
}

br_queue** br_queue_init() {
  FLOG_DEBUG("Calling br_queue_init() ...");
  //allocate necessary memory for array of br_queue*
  //initialize the individual br_queues
  // BWR queues are needed for 4 QOS classes except UGS
  bwr_q_list= (br_queue**) mac_malloc(NUM_BR_Q * sizeof(br_queue*));
  int ii=0;
  for(ii=0; ii<NUM_BR_Q; ii++) {
    br_queue* br_q= (br_queue*)mac_malloc(sizeof(br_queue));
    bwr_q_list[ii]=br_q;
    br_q->br_num=0;
    br_q->total_br=0;
    br_q->head=NULL;
    br_q->tail=NULL;
    //intialize the mutex locks and conditional variables
    if(pthread_mutex_init(&(br_q->qmutex), NULL)) {
      FLOG_FATAL("br_queue_init(): Error while initializing mutex_lock...");
    }
    if(pthread_cond_init(&(br_q->notempty), NULL)) {
      FLOG_FATAL("br_queue_init(): Error while initializing mutex_conditional variable...");
    }
    //needed for instrumentation
    add_mutex_to_trace_record(&(br_q->qmutex), "BWR_Q_LOCK");

  }

  return bwr_q_list;
}

int release_bwreq_queue()
{
  int ii=0;
  bw_req_el *prev_br_el, *br_el;
  for(ii=0; ii<NUM_BR_Q; ii++) 
    {
      if(bwr_q_list[ii])
	{
	  br_el = bwr_q_list[ii]->head;
	  while(br_el)
	    {
              prev_br_el = br_el;
	      br_el = br_el->next;
              free(prev_br_el);
	    }
	  pthread_mutex_destroy(&(bwr_q_list[ii]->qmutex));
	  free(bwr_q_list[ii]);
	  bwr_q_list[ii] = NULL;
	}
    }
  free(bwr_q_list);
  return 0;
}

bw_req_el* br_el_init(int cid, int num_bytes){

  FLOG_DEBUG("Calling br_el_init() ...");
  bw_req_el* br_el= (bw_req_el*)mac_malloc(sizeof(bw_req_el));
  br_el->bandwidth_requested=num_bytes;
  br_el->cid=cid;
  br_el->next=NULL;
  br_el->prev=NULL;

  return br_el;
}

bw_req_el* find_br_el(br_queue* br_q, int cid) {

  //TODO : currently linear search
  //Can be optimized with sophisticated br_queue DS

  bw_req_el* br_q_head=br_q->head;
  bw_req_el* br_el=br_q_head;
  while(br_el!=NULL) {
    if(br_el->cid==cid) {
      //we have found br_el for the cid
      return br_el;
    }
    else {
      br_el=br_el->next;
    }
  }

  //If we are here - we haven't found the br_el

  return NULL;
}

int update_bw_req(br_queue* br_q, bw_req_el* br_el, int num_bytes, BWR_Type type) {
    if(type == BWR_INCREMENTAL) {
      br_el->bandwidth_requested += num_bytes;
      //update the information used for overall statistics
      br_q->total_br += num_bytes;
    }
    else {
      // BWR is agreggate
      //update the information used for overall statistics
      br_q->total_br -= br_el->bandwidth_requested;
      br_q->total_br += num_bytes;
      br_el->bandwidth_requested = num_bytes;
    }

    return 0;
}

int add_new_bw_req(br_queue* br_q, int cid, int num_bytes) {

  // allocate a BW request element and initialize it with cid & num_bytes
  bw_req_el* br_el = br_el_init(cid, num_bytes);
  
  //If queue is not empty update tail pointer to enqueued bw_req_el
  if(br_q->tail!=NULL) {
    br_q->tail->next=br_el;
    br_el->prev=br_q->tail;
  }
  br_q->tail=br_el;
  
  //increment the number of BR messages
  br_q->br_num++;
  
  //If queue were empty, modify head to point to enqueued bw_req_el
  if(br_q->head==NULL) {
    br_q->head=br_el;
  }
  
  //update the information used for overall statistics
  br_q->total_br += num_bytes;
  
  return 0;
}
