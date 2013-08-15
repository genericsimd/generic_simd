/* ----------------------------------------------------------------------------
   IBM Confidential

   IBM Wireless M2M Platform   

   (C)Copyright IBM Corp. 2008, 2009, 2010, 2011

   All Rights Reserved.

   File Name: circq_array.h

   Change Activity:

   Date             Description of Change                   By
   -----------      ---------------------					--------
   1-Oct.2008       Created                                 Uma Devi

   ----------------------------------------------------------------------------
   PROLOG END TAG zYx                                                         */

#ifndef _CIRCQ_ARRAY_H
#define _CIRCQ_ARRAY_H

#include <pthread.h>
#include "arq_types.h"


typedef struct {
	int array_size;  // array_size is in units of elements;
					 // Total memory allocated is hence array_size*elem_size
	int elem_size;
	char *array;
	int head_idx;
	int tail_idx;
	pthread_mutex_t q_mutex;
	pthread_cond_t cond_is_empty;

	pthread_mutex_t *grp_mutex; // This cond, mutex pair is used to signal a thread
	pthread_cond_t  *grp_cond;  // that is blocked and is waiting for one of a set 
	                           // of queues to become non-empty.

} circq_array_t;

#define EMPTY_QUEUE {	\
	.array_size 	= 0, 	\
	.array			= NULL, \
	.head_idx		= 0, 	\
	.tail_idx 		= 0,	\
	.q_mutex		= PTHREAD_MUTEX_INITIALIZER, \
	.cond_is_empty 	= PTHREAD_COND_INITIALIZER,  \
	}


extern circq_array_t *create_circq_array (int array_size, int elem_size);
extern void destroy_circq_array (circq_array_t *);
extern op_status_t enqueue_circq_array (circq_array_t *, void *, boolean, boolean);
extern op_status_t dequeue_circq_array (circq_array_t *, void *, boolean, boolean);
extern boolean is_empty_circq_array (circq_array_t *);
extern void set_grp_cond_and_mutex_circq_array (circq_array_t *, pthread_mutex_t *, pthread_cond_t *);

#endif  //_CIRCQ_ARRAY_H
