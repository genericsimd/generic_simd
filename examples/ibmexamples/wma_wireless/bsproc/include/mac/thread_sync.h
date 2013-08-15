/* ----------------------------------------------------------------------------
   IBM Confidential

   IBM Wireless M2M Platform   

   (C)Copyright IBM Corp. 2009, 2010, 2011

   All Rights Reserved.

   File Name: thread_sync.h

   Change Activity:

   Date             Description of Change                   By
   -----------      ---------------------					--------
   14-Apr.2009      Created                                 Smruti Sarangi

   ----------------------------------------------------------------------------
   PROLOG END TAG zYx                                                         */


#ifndef THREAD_SYNC_H_
#define THREAD_SYNC_H_

extern pthread_mutex_t sync_mutex;
extern int sync_flag;
extern int sync_count;
extern void decrement_sync_count();
extern int can_sync_continue();
#endif /* THREAD_SYNC_H_ */
