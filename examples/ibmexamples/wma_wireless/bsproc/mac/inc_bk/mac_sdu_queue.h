/* ----------------------------------------------------------------------------
   IBM Confidential

   IBM Wireless M2M Platform   

   (C)Copyright IBM Corp. 2008, 2009, 2010, 2011

   All Rights Reserved.

   File Name: mac_sdu_queue.h

   Change Activity:

   Date             Description of Change                   By
   -----------      ---------------------					--------
   1-Oct.2008       Created                                 Malolan Chetlur

   ----------------------------------------------------------------------------
   PROLOG END TAG zYx                                                         */

#ifndef _MAC_SDU_Q_
#define _MAC_SDU_Q_

#include <stdlib.h>
#include "string.h"
#include "sdu_cid_queue.h"


//Queue maintained in BS for DL for all the (transport) CIDs

typedef struct mac_sdu_queue sdu_queue;

struct mac_sdu_queue {
  // Other aggr info of all sdu_cid_queues
  int overall_sdu_size;
  int num_cids; //not used anywhere right now
  int num_ss;//not used anywhere right now
  int num_be_cids_with_sdu_data;
  // Array of queue per CID
  sdu_cid_queue** sdu_cid_q;

};

// allocate and intialize the  sdu_queue
extern void mac_sdu_queue_init();

// dequeue from sdu_queue num_bytes ; return the corresponding list of 
// logical_packets as sdu_list
extern int dequeue_sduq(sdu_queue* sdu_q, int cid, size_t num_bytes, logical_packet** sdu_list);

// enqueue to sdu_queue data of size num_bytes at location physicalSdu
extern int enqueue_transport_sdu_queue(sdu_queue* sdu_q, int cid, size_t num_bytes, void* physicalSdu);

//peek the sdu cid queue and for requested num_bytes
// determine -- how many num_sdus are needed and their max and min size
extern int peek_sdu_queue(sdu_queue* sdu_q, int cid, int num_bytes, int* num_sdu, int* min_sdu_size, int* max_sdu_size);
extern sdu_cid_queue* get_sdu_cid_queue(sdu_queue* sdu_q, int cid);

extern BOOL is_cid_in_sdu_queue(int cid);


//sdu_queue* dl_sdu_queue;

sdu_queue* ul_sdu_queue;


extern int initialize_sduq(sdu_queue** sduq, u_int8_t is_dl);

extern int get_sduq(sdu_queue** sduq, u_int8_t is_dl);

//extern int dequeue_sduq(sdu_queue* sduq, int cid, int bytes_num, logical_packet** sdulist);

extern int enqueue_sduq(sdu_queue* sduq, int cid, logical_packet* sdu);

extern int add_sdu_cid_queue(sdu_queue* sduq, int cid);

extern int del_sdu_cid_queue(sdu_queue* sduq, int cid);

extern int release_sduq(sdu_queue* sduq, u_int8_t is_dl);

extern int dequeue_ul_sduq(sdu_queue* sduq);

void mac_sdu_queue_finalize();
#endif
