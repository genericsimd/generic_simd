/* ----------------------------------------------------------------------------
   IBM Confidential

   IBM Wireless M2M Platform   

   (C)Copyright IBM Corp. 2008, 2009, 2010, 2011

   All Rights Reserved.

   File Name: CS.c

   Change Activity:

   Date             Description of Change                   By
   -----------      ---------------------					--------
   1-Oct.2008       Created                                 Malolan Chetlur

   ----------------------------------------------------------------------------
   PROLOG END TAG zYx                                                         */

#include "CS.h"
#include "debug.h"

void* CS(void* param) {
  FLOG_DEBUG("Starting MAC CS layer threads ...");
  //mac_sdu_queue_init();
  //ht_init();
  
  // Enqueue SDUs to mac sdu queue
  size_t total_bytes=0;
  int i=0;
  for(i=0; i<100; i++) {
    //get randomly a connection id
    int cid=rand()%10;

    //create a mac sdu packet
    size_t num_bytes=rand()%1000 +1;
    total_bytes += num_bytes;
    void* sdu = (void*)mac_malloc(num_bytes);
    FLOG_DEBUG(" Arrival of packet # %d cid: %d num_bytes: %d\n", i, cid, num_bytes);

    //enqueue it to the sdu queue
    enqueue_transport_sdu_queue(dl_sdu_queue, cid, num_bytes, sdu);
    //int cid_q_indx=ht_get_key(cid);
    //sdu_cid_queue* cid_q=dl_sdu_queue->sdu_cid_q[cid_q_indx];

    //sleep for some time
    msleep(100);

  }
  FLOG_DEBUG("Total bytes enqueued: %d\n", total_bytes);
  return NULL;
}

int micro_sleep(unsigned long micro_sec)
{
  struct timespec req={0};
  time_t sec=(int)(micro_sec/1000000L);
  micro_sec=micro_sec-(sec*1000000L);
  req.tv_sec=sec;
  req.tv_nsec=micro_sec*1000L;
  while(nanosleep(&req,&req)==-1)
    continue;
  return 1;
}  


int msleep(unsigned long milisec)
{
  struct timespec req={0};
  time_t sec=(int)(milisec/1000);
  milisec=milisec-(sec*1000);
  req.tv_sec=sec;
  req.tv_nsec=milisec*1000000L;
  while(nanosleep(&req,&req)==-1)
    continue;
  return 1;
}  
