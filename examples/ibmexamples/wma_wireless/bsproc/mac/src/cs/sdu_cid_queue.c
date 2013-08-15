/*-------------------------------------------------------------------
IBM Confidential

IBM Wireless M2M Platform

(C)Copyright IBM Corp. 2009,2010,2011

All Rights Reserved

File Name: sdu_cid_queue.c

Change Activity:

Date    	Description of Change        	By
----------------------------------------------------------
01-Oct-2008 	     Created		   Malolan Chetlur

----------------------------------------------------------
*/

#include <pthread.h>
#include <assert.h>
#include <limits.h>
#include <stdlib.h>
#include <string.h>
#include "arq_const.h"
#include "sdu_cid_queue.h"
#include "logical_packet.h"
#include "arq_ifaces.h"
#include "CS.h"
#include "debug.h"
#include "memmgmt.h"
#include "log.h"
#include "mutex_macros.h"
#include "perf_log.h"
#include "flog.h"
#include "mac.h"

#define SDU_Q_INSTRUMENTATION_CYCLE 2
#define PEEK_CID_WITH_NO_LOCKS

static int mac_global_frame_number = 0;

extern int get_min_rsvrd_transfer_rate_per_frame(int cid);

//enqueue mem into the cid_q
int enqueue_sdu_cid_queue(sdu_cid_queue* cid_q, int cid, int num_bytes, void* mem) {
  FLOG_DEBUG("Calling enqueue_sdu_cid_queue() ...");
  PLOG(PLOG_SDUQ_ENQUEUE, METHOD_BEGIN);

  if ( (cid_q->sdu_num > MAX_SDU_IN_CID_QUEUE)
      || (cid_q->sdu_cid_aggr_info->overall_bytes > MAX_CID_QUEUE_SIZE) )
  {
    FLOG_WARNING("SDU QUEUE Full\n");
    return 1;
  }

  // initialize the logical_packet with the  
  // cid, element type, bsn, block-offset
  logical_packet* lp=logical_packet_init(cid, num_bytes, mem, MAC_SDU);
  //Lock the queue before enqueuing
  pthread_mutex_lock(&(cid_q->qmutex));

  //If queue is not empty update tail pointer to enqueued logical_packet
  if(cid_q->tail!=NULL) {
    cid_q->tail->next=lp;
    lp->prev=cid_q->tail;
  }
  cid_q->tail=lp;

  //increment the number of sdus
  cid_q->sdu_num++;
  //If queue were empty, modify head to point to enqueued logical_packet
  if(cid_q->sdu_num==1) {
    cid_q->head=lp;
  }

  //update the information used for overall statistics
  cid_q->sdu_cid_aggr_info->overall_bytes += num_bytes;

  //If the BE CID Q contains sdu data after being empty 
  // increment the num_be_cids_with_sdu_data used by scheduler
  if(cid_q->sdu_num==1){
    if(is_be_cid(cid)==TRUE) {
      //Total number of BE cids with non-zero sdu data increases by one
      dl_sdu_queue->num_be_cids_with_sdu_data++;
    }
  }

  pthread_mutex_unlock(&(cid_q->qmutex));
  FLOG_DEBUG("Finished enqueue_sdu_cid_queue() ...");
  PLOG(PLOG_SDUQ_ENQUEUE, METHOD_END);
  return 0;
}

// dequeue sdu from cid queue for the specified number of bytes
// return the list of logical packets as the sdu_list for the cid
int dequeue_sdu_cid_queue(sdu_cid_queue* cid_q, int cid, int num_bytes, logical_packet** sdu_list) {
  FLOG_DEBUG("Calling dequeue_sdu_cid_queue() ...");
  PLOG(PLOG_SDUQ_DEQUEUE, METHOD_BEGIN);
  int cid_blk_sz=get_blk_size(cid);
  int nxt_bsn=0;

  //Number of packets that will be returned as sdu_list
  int num_packets=0;

  if(num_bytes<=0) {
   	FLOG_DEBUG("Requested num_bytes is zero-- not dequeuing");
    *sdu_list=NULL;
    PLOG(PLOG_SDUQ_DEQUEUE, METHOD_END);
    return 0;
  }

  //Do not wait till cid queue has elements to dequeue
  //Return NULL as sdu_list when no elements to dequeue
  // Also Note: pthread lock obtained to check num_sdu>0
  // If enqueue is adding first sdu at the same time it will not
  // be considered and dequeued in the next frame
  if(cid_q->sdu_num==0) {
    FLOG_DEBUG("No of Sdus is zero ---  not dequeuing");
    *sdu_list=NULL;
    PLOG(PLOG_SDUQ_DEQUEUE, METHOD_END);
    return 0;
  }
  blocks_info_t* transmitted_blocks=NULL;
  pthread_mutex_lock(&(cid_q->qmutex));

  assert(cid_q->sdu_num>0);

  logical_packet* lp_list_head=NULL;
  logical_packet* lp_list_tail=NULL;

  int encountered_bytes=partition_sdu_cid_queue(cid_q, cid, num_bytes, &lp_list_head, &lp_list_tail, &nxt_bsn, &num_packets);

  /*  if(encountered_bytes==0) {
      DEBUG("Encountered byes is 0, lp list is NULL");
      *sdu_list=NULL;
      pthread_mutex_unlock(&(cid_q->qmutex));
      PLOG(PLOG_SDUQ_DEQUEUE, METHOD_END);
      return 0;
      }
  */

  if(encountered_bytes==0) {
     FLOG_DEBUG("Encountered byes is 0, lp list is NULL");
    *sdu_list=NULL;
    //check the sdu_num==0 for BE cid's  Scheduler uses this data
    if(is_be_cid(cid)==TRUE) {
      if(cid_q->head == NULL) {
	//Total number of BE cids with non-zero sdu data decreases by one
	dl_sdu_queue->num_be_cids_with_sdu_data--;
      }
    }
    pthread_mutex_unlock(&(cid_q->qmutex));
    PLOG(PLOG_SDUQ_DEQUEUE, METHOD_END);
    return 0;
  }

  int max_allowed_bsn=INT_MAX;
  int more_bytes_or_blocks=0;
  int more_bytes=0;
  int more_blocks=0;
  if(is_conn_arq_enabled(cid)) {
    more_bytes=(encountered_bytes > num_bytes);
    max_allowed_bsn=(ARQ_get_tx_window_start(cid)+ARQ_get_tx_window_size(cid)-1)%ARQ_BSN_MODULUS;

    if(max_allowed_bsn<=ARQ_get_tx_window_start(cid)) {
      more_blocks=(nxt_bsn>max_allowed_bsn+ARQ_BSN_MODULUS);
    }
    else {
      more_blocks=(nxt_bsn>max_allowed_bsn);
    }
    more_bytes_or_blocks= more_bytes || more_blocks;
    FLOG_DEBUG("SDUQ: MAX BSN:%d window_start:%d window_size:%d max_allowed_bsn:%d nxt_bsn:%d more_blocks:%d\n", cid, ARQ_get_tx_window_start(cid), ARQ_get_tx_window_size(cid), max_allowed_bsn,nxt_bsn, more_blocks);

  }
  else {

    more_bytes_or_blocks=(encountered_bytes > num_bytes);
  }

  //We have have more bytes or blocks
  if(more_bytes_or_blocks) {
    FLOG_DEBUG("encountered greater than num_bytes");
    // encountered bytes is more than num_bytes
    // and lp could be null
    if(lp_list_tail!=NULL) {
      //lp_list_tail packet must be split
      if(!is_fragmentation_enabled(cid)) {
	// Fragmentation is not enabled
	// encountered bytes is greater than num_bytes
	// therefore, ignore the last sdu -- since that cannot be fragmented
	// add it back to sdu Q
	encountered_bytes=add_tail_packet_back_to_queue(cid_q, cid, encountered_bytes, &lp_list_head, &lp_list_tail, &nxt_bsn);
	num_packets--;
      }
      else { //fragmentation enabled
	//the logical_packet being pointed to must be split
	int needed_size = 0;
	int remaining_size=0;
	if(is_conn_arq_enabled(cid)) {
	  FLOG_DEBUG("SDUQ: Determinign  needed remaining size CID:%d ec_bytes:%d num_bytes:%d le-head:%d start_bsn:%d max_allowed_bsn:%d \n", cid,encountered_bytes, num_bytes, lp_list_tail->element_head->length,  lp_list_tail->element_head->start_bsn, max_allowed_bsn);

	  //lets calculate the needed size according to num_bytes 
	  //and encountered_bytes
	  //more_bytes_or_blocks=(encountered_bytes > num_bytes) || ((ARQ_BSN_MODULUS+nxt_bsn-ARQ_get_tx_window_start(cid))%ARQ_BSN_MODULUS>max_allowed_bsn);
	  if(encountered_bytes > num_bytes) {
	    needed_size=num_bytes - encountered_bytes + lp_list_tail->element_head->length;
	    //split the sdu at ARQ boundaries
	    // fragment size is integral multiple of cid_blk_sz
	    needed_size=(needed_size/cid_blk_sz)*cid_blk_sz;
	  }
	  else if(encountered_bytes==num_bytes) {
	    needed_size=lp_list_tail->element_head->length;
	  }
	  else {
	    needed_size=INT_MAX;
	  }
	  FLOG_DEBUG("SDUQ: needed size according to size CID:%d needed_size:%d\n", cid, needed_size);
	  int needed_no_of_blks=0;
	  int needed_size_based_on_blks=0;
	  //if((ARQ_BSN_MODULUS+nxt_bsn-ARQ_get_tx_window_start(cid))%ARQ_BSN_MODULUS>max_allowed_bsn) {
	  if(more_blocks) {
	    //Lets calculate the needed_size satisfying the max_allowed_bsn
	    //condition
	    if(max_allowed_bsn>=lp_list_tail->element_head->start_bsn) {
	      needed_no_of_blks=max_allowed_bsn-lp_list_tail->element_head->start_bsn+1;
	       FLOG_DEBUG("SDUQ: max_alowed > start bsn CID:%d needed_no of blks:%d\n", cid, needed_size);
	    }
	    else {
	      needed_no_of_blks=(ARQ_BSN_MODULUS+max_allowed_bsn-lp_list_tail->element_head->start_bsn)%ARQ_BSN_MODULUS+1;
	      FLOG_DEBUG("SDUQ: max_alowed <= start bsn CID:%d needed_no of blks:%d\n", cid, needed_size);
	    }
	    //needed_no_of_blks=(ARQ_BSN_MODULUS+max_allowed_bsn-lp_list_tail->element_head->start_bsn)%ARQ_BSN_MODULUS+1;
	    needed_size_based_on_blks=needed_no_of_blks*cid_blk_sz;
	  }
	  else {
	    needed_size_based_on_blks=INT_MAX;
	  }
	  FLOG_DEBUG("SDUQ: needed size according to blocks CID:%d needed_size:%d\n", cid, needed_size);
	    

	  //Now choos the size that is smaller either according to num_bytes
	  // requirement or arq num of blocks allowed
	  if(needed_size_based_on_blks<needed_size) {
	    needed_size=needed_size_based_on_blks;
	  }
	  assert(needed_size!=INT_MAX);
	  remaining_size=lp_list_tail->element_head->length-needed_size;

	  FLOG_DEBUG("SDUQ: Remaining size:%d Needed size:%d  num_byes:%d encountered_bytes:%d\n", remaining_size, needed_size, num_bytes, encountered_bytes);
	  FLOG_DEBUG("SDUQ: tail element: start_bsn:%d length:%d max_allowed_bsn:%d \n", lp_list_tail->element_head->start_bsn, lp_list_tail->element_head->length, max_allowed_bsn);

	}
	else {//arq not enabled
	  needed_size=num_bytes - encountered_bytes + lp_list_tail->element_head->length;
	  remaining_size=lp_list_tail->element_head->length-needed_size;
	}
	assert(remaining_size>=0);
	assert(needed_size>=0);
	assert(needed_size+remaining_size==lp_list_tail->element_head->length);

	if((needed_size!=0)&&(remaining_size!=0)) {
	  FLOG_DEBUG("split lp");
	  encountered_bytes=fragment_tail_packet(cid_q, cid, encountered_bytes, &lp_list_tail, needed_size, remaining_size, &nxt_bsn);
	}
	else if(needed_size==0) { //needed_size=0;
	  FLOG_DEBUG("needed size is 0");
	  assert(is_conn_arq_enabled(cid)==TRUE);
	  encountered_bytes=add_tail_packet_back_to_queue(cid_q, cid, encountered_bytes, &lp_list_head, &lp_list_tail, &nxt_bsn);
	  num_packets--;
	}
	else {//remaining size ==0
	  //do nothing
	}
      }
    }
  }
  else {
    FLOG_DEBUG("encountered is less or equal than num_bytes");
    //lp is NULL  -- thats why we are here
    // there are no more packets
    //do nothing
  }

  //update over all aggr info
  cid_q->sdu_cid_aggr_info->overall_bytes -= encountered_bytes;
  update_overall_deficit_info(cid_q, cid, encountered_bytes);
  transmitted_blocks=NULL;
  if(is_conn_arq_enabled(cid)) {
    transmitted_blocks=create_transmitted_blocks(lp_list_head, num_packets);

    //check the sdu_num==0 for BE cid's  Scheduler uses this data
    if(is_be_cid(cid)==TRUE) {
      if(cid_q->sdu_num==0) {
	//Total number of BE cids with non-zero sdu data decreases by one
	dl_sdu_queue->num_be_cids_with_sdu_data--;
      }
    }
  }

  pthread_mutex_unlock(&(cid_q->qmutex));
  *sdu_list=lp_list_head;
  if(is_conn_arq_enabled(cid)) {
    enqueue_transmitted_blocks(cid, transmitted_blocks, num_packets);
  }

#ifdef LOG_METRICS
  if(get_current_frame_number() % SDU_Q_INSTRUMENTATION_CYCLE ==0) {
    FLOG_DEBUG("logging sdu q len\n");
    LOG_EVENT_sdu_len(readtsc(),cid, cid_q->sdu_num);
  }

#endif

  //###The following line should be deleted after ARQ integration
  //mac_free(transmitted_blocks);
  PLOG(PLOG_SDUQ_DEQUEUE, METHOD_END);
  return encountered_bytes;
}

sdu_cid_queue* mac_sdu_cid_queue_init(int cid) {
  FLOG_DEBUG("Calling mac_sdu_cid_queue_init() ...");
  sdu_cid_queue* cid_q= (sdu_cid_queue*)mac_malloc(sizeof(sdu_cid_queue));
  cid_q->sdu_num=0;
  cid_q->head=NULL;
  cid_q->tail=NULL;

/*
#ifdef PRINT_DATA
        char file_name[8];
        sprintf(file_name,"%d.dl", cid);
        cid_q->dl_fp = fopen(file_name, "w");
        if (cid_q->dl_fp)
        {
          printf("Error opening file %s\n", file_name);
        }
        else
        {
          printf("Opened file %s\n", file_name);
        }
#endif
*/

  //allocate memory for sdu_cid_aggr_info
  cid_q->sdu_cid_aggr_info=(sdu_cid_overall_info*) mac_malloc(sizeof(sdu_cid_overall_info));

  cid_q->sdu_cid_aggr_info->next_bsn=0;
  cid_q->sdu_cid_aggr_info->overall_bytes=0;
  cid_q->sdu_cid_aggr_info->last_dequeued_frame_number= -1; 

  struct timeval deq_time;
   
  gettimeofday (&deq_time, NULL);
  double deq_time_in_us = (double)deq_time.tv_sec * 1000000 + deq_time.tv_usec;  
  cid_q->sdu_cid_aggr_info->last_dequeued_time = deq_time_in_us;
  // Initialize the deficit to one packet, to avoid excess fragmentation 
  // of UGS CIDs with small per-frame bandwidth requirement
  cid_q->sdu_cid_aggr_info->overall_deficit = 0;

  cid_q->next=NULL;

  //intialize the mutex locks and conditional variables
  if(pthread_mutex_init(&(cid_q->qmutex), NULL)) {
    FLOG_ERROR("mac_sdu_cid_queue_init(): Error while initializing mutex_lock...");
  }

  //needed for instrumentation
  add_mutex_to_trace_record(&(cid_q->qmutex), "SDU_CID_Q_LOCK");

  if(pthread_cond_init(&(cid_q->notempty), NULL)) {
    FLOG_ERROR("mac_sdu_cid_queue_init(): Error while initializing mutex_conditional variable...");
  }
  return cid_q;
}

//This internal function splits the sdu_cid_queue
//into two parts. 1) containig the payload as requested by the scheduler
//2) remains in the sdu queue
//lp_list updated by this function points to the list of logical_packets
//forming the payload in the current/future frame
//After split-- the last logical packet has to be split again to
//to account for fragmentation and ARQ boundaries
int partition_sdu_cid_queue(sdu_cid_queue* cid_q, int cid, int num_bytes, logical_packet** lp_list_head, logical_packet** lp_list_tail, int* nxt_bsn, int* num_packets) {

  int encountered_bytes=0;
  int current_sdu_bytes=0;
  int cid_blk_sz=get_blk_size(cid);
  int num_full_blks=0;
  int num_partial_blks=0;

  *nxt_bsn=cid_q->sdu_cid_aggr_info->next_bsn;
  FLOG_DEBUG("SDUQ: CID:%d nxt_bsn:%d\n", cid, *nxt_bsn);

  FLOG_DEBUG("partition sdu q: cid:%d num_bytes:%d nxt_bsn:%d head:%p tail:%p num_sdu:%d \n", cid, num_bytes, (*nxt_bsn), cid_q->head, cid_q->tail, cid_q->sdu_num);

  *lp_list_head=cid_q->head;
  *lp_list_tail=cid_q->head;
  logical_packet* lp=cid_q->head;

  //partition_sdu_cid_queue goes upto the sdu
  // that has more than requested num_bytes
  // OR the more the max_bsn allowed as per ARQ window (arq_window_start _arq_window_size)
  // OR the list is empty
  // all these conditions are captured in the dq_cond_expr
  int max_allowed_bsn=INT_MAX;
  int dq_cond_expr=0;
  if(is_conn_arq_enabled(cid)) {
    max_allowed_bsn=(ARQ_get_tx_window_start(cid)+ARQ_get_tx_window_size(cid)-1)%ARQ_BSN_MODULUS;
    if(max_allowed_bsn <= ARQ_get_tx_window_start(cid)) {
      dq_cond_expr=((encountered_bytes<num_bytes)&&(lp!=(void*)0)&&((*nxt_bsn)<=max_allowed_bsn+ARQ_BSN_MODULUS));
    }
    else {
      dq_cond_expr=((encountered_bytes<num_bytes)&&(lp!=(void*)0)&&((*nxt_bsn)<=max_allowed_bsn));
    }
    FLOG_DEBUG("SDUQ: Initial MAX BSN:%d window_start:%d window_size:%d max_allowed_bsn:%d nxt_bsn:%d dq_cond_expr:%d\n", cid, ARQ_get_tx_window_start(cid), ARQ_get_tx_window_size(cid), max_allowed_bsn, (*nxt_bsn), dq_cond_expr);

    //Check if we can't dequeue to exceeding the max_allowed_bsn
    if(!dq_cond_expr) {
      *lp_list_head=NULL;
      *lp_list_tail=NULL;
      *num_packets=0;
      FLOG_DEBUG("SDUQ: Unable to send due to MAX BSN:%d window_start:%d window_size:%d max_allowed_bsn:%d nxt_bsn:%d\n", cid, ARQ_get_tx_window_start(cid), ARQ_get_tx_window_size(cid), max_allowed_bsn, (*nxt_bsn));
      return encountered_bytes;
    }
  }
  else {
    dq_cond_expr=((encountered_bytes<num_bytes)&&(lp!=(void*)0));
  }



  //TRACE6(4, "Partition sdu q: cid:%d encountered_bytes:%d num_bytes:%d lp:%x dq_cond_expr:%d nxt_bsn:%d \n", cid, encountered_bytes, num_bytes, lp, dq_cond_expr, (*nxt_bsn));
  while(dq_cond_expr) {
    FLOG_DEBUG("Partition sdu q: cid:%d encountered_bytes:%d num_bytes:%d lp:%p  dq_cond_expr:%d nxt_bsn:%d \n", cid, encountered_bytes, num_bytes, lp, dq_cond_expr, (*nxt_bsn));
    *lp_list_tail=lp;
    //TRACE6(4, "Partition sdu q: cid:%d encountered_bytes:%d num_bytes:%d lp:%x dq_cond_expr:%d nxt_bsn:%d \n", cid, encountered_bytes, num_bytes, lp, dq_cond_expr, (*nxt_bsn));
    current_sdu_bytes=lp->element_head->length;
    cid_q->sdu_num--;
    // calculate the block sequence number for the  next sdu 
    // in the case of arq  connection
    if(is_conn_arq_enabled(cid)) {
      //////
      // update the starting block sequence number in case of ARQ
      // start_bsn = next_bsn;
      // next_bsn = next_bsn + (num_bytes/BLK_SIZE[cid]);
      /////
      lp->element_head->start_bsn=*nxt_bsn;
      FLOG_DEBUG("SDUQ: CID:%d nxt_bsn:%d\n", cid, *nxt_bsn);
      num_full_blks= current_sdu_bytes/cid_blk_sz;
      num_partial_blks=((current_sdu_bytes%cid_blk_sz)==0 ? 0 : 1);
      *nxt_bsn=((*nxt_bsn)+num_full_blks + num_partial_blks) % ARQ_BSN_MODULUS;
      FLOG_DEBUG("SDUQ: CID:%d nxt_bsn:%d\n", cid, *nxt_bsn);
    }
    encountered_bytes += current_sdu_bytes;
    (*num_packets)++;
    lp=lp->next;
    /*     if(is_conn_arq_enabled(cid)) { */
    /*       dq_cond_expr=((encountered_bytes<num_bytes)&&(lp!=(void*)0)&&((ARQ_BSN_MODULUS+(*nxt_bsn)-ARQ_get_tx_window_start(cid))%ARQ_BSN_MODULUS<=max_allowed_bsn)); */
    /*     } */
    /*     else { */
    /*       dq_cond_expr=((encountered_bytes<num_bytes)&&(lp!=(void*)0)); */
    /*     } */
    if(is_conn_arq_enabled(cid)) {
      if(max_allowed_bsn <= ARQ_get_tx_window_start(cid)) {
	dq_cond_expr=((encountered_bytes<num_bytes)&&(lp!=(void*)0)&&((*nxt_bsn)<=max_allowed_bsn+ARQ_BSN_MODULUS));
      }
      else {
	dq_cond_expr=((encountered_bytes<num_bytes)&&(lp!=(void*)0)&&((*nxt_bsn)<=max_allowed_bsn));
      }
      FLOG_DEBUG("SDUQ: End of while MAX BSN:%d window_start:%d window_size:%d max_allowed_bsn:%d nxt_bsn:%d dq_cond_expr:%d\n", cid, ARQ_get_tx_window_start(cid), ARQ_get_tx_window_size(cid), max_allowed_bsn, (*nxt_bsn), dq_cond_expr);
    }
    else {
      dq_cond_expr=((encountered_bytes<num_bytes)&&(lp!=(void*)0));
    }
  }

  cid_q->head=lp;

  if(lp!=NULL) {
    cid_q->head->prev=NULL;
  }
  else {
    FLOG_DEBUG("SDUQ: CID:%d sdu_num:%d lp:%p head:%p tail:%p\n",cid, cid_q->sdu_num,lp, cid_q->head, cid_q->tail);
    cid_q->head=NULL;
    cid_q->tail=NULL;
    assert(cid_q->sdu_num==0);
  }

  //update the information used for overall statistics
  if(is_conn_arq_enabled(cid)) {
    cid_q->sdu_cid_aggr_info->next_bsn=*nxt_bsn;
    FLOG_DEBUG("SDUQ: CID:%d nxt_bsn:%d\n", cid, *nxt_bsn);
  }
  if(*lp_list_tail!=NULL) {
    (*lp_list_tail)->next=NULL;
  }

  return encountered_bytes;
}


int add_tail_packet_back_to_queue(sdu_cid_queue* cid_q, int cid, int encountered_bytes, logical_packet** lp_list_head, logical_packet** lp_list_tail, int* nxt_bsn) {

  int cid_blk_sz=get_blk_size(cid);
  BOOL empty_list=FALSE;
  if(*lp_list_head==*lp_list_tail) {
    //Once we add the tail packet to sdu q -- the payload lp list will be empty
    empty_list=TRUE;
  }

  (*lp_list_tail)->next=cid_q->head;
  if(cid_q->head!=NULL) {
    cid_q->head->prev=*lp_list_tail;
  }
  cid_q->head=*lp_list_tail;
  cid_q->sdu_num++;
  if(cid_q->tail==NULL) {
    cid_q->tail=*lp_list_tail;
  }

  if(empty_list) {
    *lp_list_head=NULL;
    *lp_list_tail=NULL;
  }
  else {
    *lp_list_tail=(*lp_list_tail)->prev;
    if((*lp_list_tail)!=NULL) {
      (*lp_list_tail)->next=NULL;
    }
  }


  int list_byte_count = encountered_bytes - cid_q->head->element_head->length;
  if(is_conn_arq_enabled(cid)) {
    //set the starting block sequence number
    int num_full_blks= cid_q->head->element_head->length/cid_blk_sz;
    int num_partial_blks=((cid_q->head->element_head->length%cid_blk_sz)==0 ? 0 : 1);
    *nxt_bsn=(ARQ_BSN_MODULUS+(*nxt_bsn) - num_full_blks - num_partial_blks)%ARQ_BSN_MODULUS;
    FLOG_DEBUG("SDUQ: CID:%d nxt_bsn:%d\n", cid, *nxt_bsn);
    cid_q->head->element_head->start_bsn=*nxt_bsn;
    cid_q->sdu_cid_aggr_info->next_bsn=*nxt_bsn;
  }

  return list_byte_count;
}

int fragment_tail_packet(sdu_cid_queue* cid_q, int cid, int encountered_bytes, logical_packet** lp_list_tail, int needed_size, int remaining_size, int* nxt_bsn) {
  int cid_blk_sz=get_blk_size(cid);
  //total sdu size that will be dequeued is the
  // size of the fragment to be added and not the whole sdu
  int list_byte_count = encountered_bytes + needed_size-(*lp_list_tail)->element_head->length;
	  
  //create a new logical_packet with a new logical_element 
  // containing remaining sdu fagment
  // add it to main queue
  logical_packet* hd_packet= logical_packet_init(cid, remaining_size,((*lp_list_tail)->element_head->data+needed_size), MAC_SDU_FRAG);
  hd_packet->next=cid_q->head;
  if(cid_q->head!=NULL) {
    cid_q->head->prev=hd_packet;
  }
  
  hd_packet->prev=NULL;
  hd_packet->element_head->blk_type=LAST_FRAGMENT;
  cid_q->sdu_num++;
  
  //update the head of cid_q
  cid_q->head=hd_packet;
  //update the tail if needed
  if(cid_q->tail==NULL) {
    cid_q->tail=hd_packet;
  }

  if(is_conn_arq_enabled(cid)) {

    //set the starting block sequence number
    int num_full_blks= (*lp_list_tail)->element_head->length/cid_blk_sz;
    int num_partial_blks=(((*lp_list_tail)->element_head->length%cid_blk_sz)==0 ? 0 : 1);
    *nxt_bsn=(ARQ_BSN_MODULUS+(*nxt_bsn) - num_full_blks - num_partial_blks)%ARQ_BSN_MODULUS;
    FLOG_DEBUG("SDUQ: CID:%d nxt_bsn:%d\n", cid, *nxt_bsn);
    num_full_blks=needed_size/cid_blk_sz;
    num_partial_blks=((needed_size%cid_blk_sz)==0 ?0 : 1);
    *nxt_bsn=((*nxt_bsn) + num_full_blks + num_partial_blks)%ARQ_BSN_MODULUS;
    FLOG_DEBUG("SDUQ: CID:%d nxt_bsn:%d\n", cid, *nxt_bsn);
    hd_packet->element_head->start_bsn=*nxt_bsn;
    cid_q->sdu_cid_aggr_info->next_bsn=*nxt_bsn;

  }

  
  if((*lp_list_tail)->element_head->type == MAC_SDU) {
    (*lp_list_tail)->element_head->blk_type=FIRST_FRAGMENT;
    (*lp_list_tail)->element_head->type=MAC_SDU_FRAG;
  }
  else if ((*lp_list_tail)->element_head->type == MAC_SDU_FRAG) {
    (*lp_list_tail)->element_head->blk_type=CONTINUING_FRAGMENT;
  }
  // else do nothing
  
  //update the last element of sdu_list
  (*lp_list_tail)->element_head->length=needed_size;
  (*lp_list_tail)->next=NULL;

  return list_byte_count;
}


//This function creates an array of transmitted blocks of type blocks_info_t* 
//needed for ARQ
// The logical_packet list to be returned by dequeue function is used to
//construct the blocks_info_t array
blocks_info_t* create_transmitted_blocks(logical_packet* lp_list_head, int num_packets) {
  logical_packet* lp=lp_list_head;
  blocks_info_t* transmitted_blocks= (blocks_info_t*) mac_malloc(sizeof(blocks_info_t)*num_packets);
  int ii=0;
  while(lp!=NULL) {
    transmitted_blocks[ii].start_bsn=lp->element_head->start_bsn;
    transmitted_blocks[ii].size=lp->element_head->length;
    transmitted_blocks[ii].data=(char*)lp->element_head->data;
    transmitted_blocks[ii].btype=lp->element_head->blk_type;
    FLOG_DEBUG("creating transmitted blocks: cid :  start_bsn: %d frag type: %d \n", transmitted_blocks[ii].start_bsn, transmitted_blocks[ii].btype);
    FLOG_DEBUG("SDUQ: Transmitted Blocks: start_bsn:%d length:%d  blk_type:%d\n",  transmitted_blocks[ii].start_bsn, transmitted_blocks[ii].size, transmitted_blocks[ii].btype);
    ii++;

    assert(lp->element_head->start_bsn>=0);
    lp=lp->next;
  }
  assert(num_packets==ii);
  return transmitted_blocks;
}

void enqueue_transmitted_blocks(int cid, blocks_info_t* transmitted_blocks, int num_packets) {
  int ii=0;
  for(ii=0; ii<num_packets; ii++) {
    FLOG_DEBUG("Enqueue transmitted blocks: cid : %d  start_bsn: %d frag type: %d \n", cid, transmitted_blocks[ii].start_bsn, transmitted_blocks[ii].btype);
    if(ARQ_enqueue_tx_blocks_q(cid, &(transmitted_blocks[ii]))) {
      FLOG_ERROR("Error: Unable to enqueue into transmitted blocks queue");
    }
  }

  // Free the blocks created in create_transmitted_blocks
  mac_free(sizeof(blocks_info_t)*num_packets, transmitted_blocks);

}


//###This dummy function should be removed once ARQ module is added
/*
  op_status_t ARQ_enqueue_tx_blocks_q (int cid, blocks_info_t *element) {
  DEBUG("Enqueue into ARQ Transmitted blocks list");
  printf("cid:%d start_bsn:%d size:%d frag_type:%d\n",
  cid,
  element->start_bsn,
  element->size,
  element->btype);
  return SUCCESS;
  }
*/

// peek cid_q and determine for requested num_bytes of data
// the number of sdus and their max and min size that will be dequeues
int peek_cid_queue(sdu_cid_queue* cid_q, int cid, int num_bytes,
		   int* num_sdu, int* min_sdu_size, int* max_sdu_size) {

#ifdef PEEK_CID_WITH_NO_LOCKS
  peek_cid_queue_without_locks(cid_q, cid, num_bytes, num_sdu, min_sdu_size, max_sdu_size);
#else
  peek_cid_queue_with_locks(cid_q, cid, num_bytes, num_sdu, min_sdu_size, max_sdu_size);

#endif
  return 0;
}

// NOTE: The actual number of sdus can be one less than the returned value
// depending on fragmentation is enabled or not
// Since the scheduler uses this peek to get an estimate on overheads
// this conservativve estimate of num_sdu can still guarantee
// to CPS the size of the burst that is estimated
// Similarly min can be incorrect-- if the last packet 
// is fragmented and having effect to a lower min value. however
// this will not affect the guarantee provided to CPS.
int peek_cid_queue_with_locks(sdu_cid_queue* cid_q, int cid, int num_bytes,
			      int* num_sdu, int* min_sdu_size, int* max_sdu_size) {

  FLOG_DEBUG("Calling peek_cid_queue() ...");
  PLOG(PLOG_SDUQ_PEEK, METHOD_BEGIN);
  int encountered_bytes=0;  
  pthread_mutex_lock(&(cid_q->qmutex));
  
  *num_sdu=0;
  *min_sdu_size=INT_MAX; //a large number
  *max_sdu_size=0;

  logical_packet* lp=cid_q->head;
  
  while((encountered_bytes < num_bytes) && (lp!=NULL)) {
    (*num_sdu)++;
    if(*min_sdu_size>lp->element_head->length) {
      *min_sdu_size=lp->element_head->length;
    }
    if(*max_sdu_size<lp->element_head->length) {
      *max_sdu_size=lp->element_head->length;
    }
    encountered_bytes +=lp->element_head->length;
    lp=lp->next;
  }
  pthread_mutex_unlock(&(cid_q->qmutex));
  FLOG_DEBUG("Finished peek_cid_queue() ...");
  PLOG(PLOG_SDUQ_PEEK, METHOD_END);
  return 0;
}

int peek_cid_queue_without_locks(sdu_cid_queue* cid_q, int cid, int num_bytes,
				 int* num_sdu, int* min_sdu_size, int* max_sdu_size) {

  FLOG_DEBUG("Calling peek_cid_queue() ...");
  PLOG(PLOG_SDUQ_PEEK, METHOD_BEGIN);
  int encountered_bytes=0;  
  int total_sdu_in_q=cid_q->sdu_num;

  *num_sdu=0;
  *min_sdu_size=INT_MAX; //a large number
  *max_sdu_size=0;
  int sdu_seen=0;
  logical_packet* lp=cid_q->head;
  
  while((sdu_seen<total_sdu_in_q) &&
	(encountered_bytes < num_bytes) && (lp!=NULL)) {
    sdu_seen++;
    (*num_sdu)++;
    if(*min_sdu_size>lp->element_head->length) {
      *min_sdu_size=lp->element_head->length;
    }
    if(*max_sdu_size<lp->element_head->length) {
      *max_sdu_size=lp->element_head->length;
    }
    encountered_bytes +=lp->element_head->length;
    lp=lp->next;
  }
  FLOG_DEBUG("Finished peek_cid_queue() ...");
  PLOG(PLOG_SDUQ_PEEK, METHOD_END);
  return 0;
}

//ToDo: This method should be replaced by helper methods in scheduler module
// returns the current frame_number -- maintained in cps
int get_current_frame_number() {
  return mac_global_frame_number;
}

int set_current_frame_number(int frm_num) {
  mac_global_frame_number = frm_num;
  return 0;
}

#if 0
int update_current_frame_number(int add) {
  mac_global_frame_number += add;
  return 0;
}
#else
int update_current_frame_number(int add) {
  if ( (mac_global_frame_number + add) < 0)
  {
    mac_global_frame_number = 0;
  }else
  {
    mac_global_frame_number += add;
  }
  return 0;
}
#endif

// This function is needed to account for any buildup in queues due to delay in 
// the time when the SDUs are actually dequeued from the SDU queues (e.g. due to 
// large processing or context switch delay in some part of MAC). If there are
// no delays, the deficit will be 0. The scheduler
// allocates according to frame number (since it does not know the dequeue time)
// and further allocates extra for this deficit.
int update_overall_deficit_info(sdu_cid_queue* cid_q, int cid, int dequeued_bytes) {
  if(is_ugs_cid(cid)) {
    struct timeval deq_time;
    gettimeofday (&deq_time, NULL);
    double deq_time_in_us = (double)deq_time.tv_sec * 1000000 + deq_time.tv_usec;
    int budget_by_time = get_min_rsvd_transfer_rate(cid) * (deq_time_in_us - cid_q->sdu_cid_aggr_info->last_dequeued_time)/1000000;

    int frame_allocation_gap=(get_current_frame_number() - cid_q->sdu_cid_aggr_info->last_dequeued_frame_number);
    int total_budget= max(budget_by_time, frame_allocation_gap*get_min_rsvrd_transfer_rate_per_frame(cid)) + cid_q->sdu_cid_aggr_info->overall_deficit ;
//    cid_q->sdu_cid_aggr_info->overall_deficit= total_budget - dequeued_bytes;
    cid_q->sdu_cid_aggr_info->overall_deficit = 0;
    //printf("CID: %d, Last deq'ed frame: %ld, LDT: %f, frame alloc gap: %ld, budget_by_time: %ld, total budget: %ld, deq'ed bytes: %d\n", cid,cid_q->sdu_cid_aggr_info->last_dequeued_frame_number,  cid_q->sdu_cid_aggr_info->last_dequeued_time, frame_allocation_gap, budget_by_time, total_budget, dequeued_bytes);
    cid_q->sdu_cid_aggr_info->last_dequeued_frame_number=get_current_frame_number();
    cid_q->sdu_cid_aggr_info->last_dequeued_time=deq_time_in_us;
  }
  return 0;
}
