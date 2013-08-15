/* ----------------------------------------------------------------------------
   IBM Confidential

   IBM Wireless M2M Platform   

   (C)Copyright IBM Corp. 2009, 2010, 2011

   All Rights Reserved.

   File Name: thread_sync.c

   Change Activity:

   Date             Description of Change                   By
   -----------      ---------------------					--------
   14-Apr.2009      Created                                 Smruti Sarangi

   ----------------------------------------------------------------------------
   PROLOG END TAG zYx                                                         */

#include <pthread.h>
#include "thread_sync.h"

void decrement_sync_count() {
	pthread_mutex_lock(&sync_mutex);
	sync_count--;
	pthread_mutex_unlock(&sync_mutex);
}
 int can_sync_continue() {
	int val;
	pthread_mutex_lock(&sync_mutex);
	val = sync_flag;
	pthread_mutex_unlock(&sync_mutex);
	return val;
}
