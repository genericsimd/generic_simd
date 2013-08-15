/* ----------------------------------------------------------------------------
   IBM Confidential

   IBM Wireless M2M Platform   

   (C)Copyright IBM Corp. 2008, 2009, 2010, 2011

   All Rights Reserved.

   File Name: mme_test.h

   Change Activity:

   Date             Description of Change                   By
   -----------      ---------------------					--------
   1-Oct.2008       Created                                 Partha Dutta

   ----------------------------------------------------------------------------
   PROLOG END TAG zYx                                                         */

#ifndef _MME_TEST_H_
#define _MME_TEST_H_


typedef struct mme_input mme_driver_input;

struct mme_input{
  mgt_msg_queue* queue;
  int thread_id;
};


extern void *enqueue_loop(void *input);

extern void *dequeue_loop(void *input);

extern void printQ_HtoT(mgt_msg_queue *myQ1);  // for testing

extern void printQ_TtoH(mgt_msg_queue *myQ1);  // for testing

#endif
