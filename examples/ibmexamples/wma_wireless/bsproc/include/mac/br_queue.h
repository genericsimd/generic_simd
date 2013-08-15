/* ----------------------------------------------------------------------------
   IBM Confidential

   IBM Wireless M2M Platform   

   (C)Copyright IBM Corp. 2008, 2009, 2010, 2011

   All Rights Reserved.

   File Name: br_queue.h

   Change Activity:

   Date             Description of Change                   By
   -----------      ---------------------					--------
   1-Oct.2008       Created                                 Parul Gupta

   ----------------------------------------------------------------------------
   PROLOG END TAG zYx                                                         */

#ifndef _BR_Q_
#define _BR_Q_

#include "mac.h"
#include "mac_connection.h"

//4 QoS classes except UGS needs BW requests
#define NUM_BR_Q 5

typedef struct{
  int last_dequeued_frame_number;
  int overall_deficit;
}ugs_cid_info;

ugs_cid_info *ul_ugs_cid_info;


// Element containing bandwidth request (BR) for a particular CID
typedef struct br_element bw_req_el;
struct br_element{
  int bandwidth_requested; //in Bytes
  int cid;
  bw_req_el *next;
  bw_req_el *prev;
};

// Queue that holds all the BR's
typedef struct{
  int br_num; //number of BR messages in the queue
  int total_br; //total bandwidth requested by all CID's, in bytes
  bw_req_el *head;
  bw_req_el *tail;
  //thread safe var
  pthread_mutex_t qmutex;
  pthread_cond_t notempty;
}br_queue;

//specifies the type of bandwidth request incremental(000) or aggregate(001)
typedef enum { BWR_INCREMENTAL=0, BWR_AGGREGATE=1 } BWR_Type;

// enqueue a BR message
int enqueue_bw_req_queue(br_queue** br_q_list, int cid, int num_bytes, BWR_Type type);

// dequeue a BR message
int dequeue_br_queue(br_queue* br_q, int num_el);

// cleanup served elements (bandwidth req=0) from the BR queue
int cleanup_br_queue(br_queue* br_q);

// allocate and initialize the bw req queues for each class (array of br_queue)
br_queue** br_queue_init();

// Release the memory allocated for BW req queues
int release_bwreq_queue();

bw_req_el* br_el_init(int cid, int num_bytes);

//NOTE: the following 3 methods (find_br_el, update_bw_req, and add_new_be_req)
// need to lock and unlock the queue
// the locking is called before invoking the function
// and unlocking is called after the function
// these 3 functions are split from a single function for
// more structured and smaller function body

//searches the br_queue for br_req_el for a specific cid
bw_req_el* find_br_el(br_queue* br_q, int cid);

// Add the incremental bandwidth requested for a cid to br_req_el for that cid
int update_bw_req(br_queue* br_q, bw_req_el* br_el, int num_bytes, BWR_Type type);

//Add a br_req_el for the cid alongwith the bandwidth requested
int add_new_bw_req(br_queue* br_q, int cid, int num_bytes);

//global variables

//Array of br_queue* containing br_queues for each QoS classs
extern br_queue** bwr_q_list;
#endif
