/* ----------------------------------------------------------------------------
   IBM Confidential

   IBM Wireless M2M Platform   

   (C)Copyright IBM Corp. 2008, 2009, 2010, 2011

   All Rights Reserved.

   File Name: sdu_cid_queue.h

   Change Activity:

   Date             Description of Change                       By
   -----------      ---------------------                       --------
   1-Oct.2008       Created                                     Malolan Chetlur

   ----------------------------------------------------------------------------
   PROLOG END TAG zYx                                                         */

#ifndef _SDU_CID_Q_
#define _SDU_CID_Q_

#include "mac.h"
#include "logical_packet.h"
#include "arq_ds.h"

// Structure containing the aggregate information about the sdu_cid_queue
typedef struct sdu_cid_aggr_info sdu_cid_overall_info;
struct  sdu_cid_aggr_info {

  // This contains the information about the overall sdu-cid  queue
  int overall_bytes;
  int sfid;
  int serviceclassid;
  int cid;

  // next_bsn specifies the starting block number of the next
  // sdu that will be enqueued
  int next_bsn;

  //The following are used by the scheduler
  // the frame number dequeue operation performed on this transport cid
  // currently recorded for ugs cid only
  int last_dequeued_frame_number;
  double last_dequeued_time;

  // the overall deficit in number of bytes to satisfy
  // the minimum transfer rate for ugs
  int overall_deficit;

  //.... more to be added
};

// Queue that holds all the sdu of a CID
typedef struct mac_sdu_cid_queue sdu_cid_queue; 
struct mac_sdu_cid_queue{
  int sdu_num;
  logical_packet *head;
  logical_packet *tail;
  sdu_cid_overall_info* sdu_cid_aggr_info;
  //thread safe var
  pthread_mutex_t qmutex;
  pthread_cond_t notempty;
  FILE *ul_fp;
  FILE *dl_fp;
  sdu_cid_queue* next;
};


// enqueue num_bytes of data at location mem into cid 's queue
extern int enqueue_sdu_cid_queue(sdu_cid_queue* cid_q, int cid, int num_bytes, void* mem);
extern int update_overall_deficit_info(sdu_cid_queue* cid_q, int cid, int dequeued_bytes);

// dequeue num_bytes of data from cid_queue and return the list of logical_packets and sdu_list
extern int dequeue_sdu_cid_queue(sdu_cid_queue* cid_q, int cid, int num_bytes, logical_packet** sdu_list);

// allocate and initialize the cid_queue
extern sdu_cid_queue* mac_sdu_cid_queue_init(int cid);

// peek cid_q and determine for requested num_bytes of data
// the number of sdus and their max and min size that will be dequeues
extern int peek_cid_queue(sdu_cid_queue* cid_q, int cid, int num_bytes,
			  int* num_sdu, int* min_sdu_size, int* max_sdu_size);

int fragment_tail_packet(sdu_cid_queue* cid_q, int cid, int encountered_bytes, logical_packet** lp_list_tail, int needed_size, int remaining_size, int* nxt_bsn);

int add_tail_packet_back_to_queue(sdu_cid_queue* cid_q, int cid, int encountered_bytes, logical_packet** lp_list_head, logical_packet** lp_list_tail, int* nxt_bsn);

int partition_sdu_cid_queue(sdu_cid_queue* cid_q, int cid, int num_bytes, logical_packet** lp_list_head, logical_packet** lp_list_tail, int* nxt_bsn, int* num_packets);

blocks_info_t* create_transmitted_blocks(logical_packet* lp_list_head, int num_packets);
void enqueue_transmitted_blocks(int cid, blocks_info_t* transmitted_blocks, int num_packets);

int get_current_frame_number();
int set_current_frame_number(int frm_num);
int update_current_frame_number(int add);

int peek_cid_queue_with_locks(sdu_cid_queue* cid_q, int cid, int num_bytes,  int* num_sdu, int* min_sdu_size, int* max_sdu_size);
int peek_cid_queue_without_locks(sdu_cid_queue* cid_q, int cid, int num_bytes, int* num_sdu, int* min_sdu_size, int* max_sdu_size);
#endif
