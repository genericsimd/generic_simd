/* ----------------------------------------------------------------------------
   IBM Confidential

   IBM Wireless M2M Platform   

   (C)Copyright IBM Corp. 2008, 2009, 2010, 2011

   All Rights Reserved.

   File Name: deltalist.h

   Change Activity:

   Date             Description of Change                   By
   -----------      ---------------------					--------
   1-Oct.2008       Created                                 Uma Devi

   ----------------------------------------------------------------------------
   PROLOG END TAG zYx                                                         */
#ifndef _DELTALIST_H
#define _DELTALIST_H

#include <pthread.h>
#include "arq_types.h"
#include "arq_ds.h"


typedef struct delta_list_node_t {
	struct delta_list_node_t *next;
	struct delta_list_node_t *prev;
	struct timer_entry_t timer_entry;
	struct delta_list_node_t **cross_ref_ptr;
} delta_list_node_t;

typedef struct {
	delta_list_node_t *head;
	delta_list_node_t *tail;
	pthread_mutex_t dl_mutex;
	pthread_cond_t cond_is_empty;
} delta_list_t;

#define EMPTY_DELTA_LIST {	\
	.head 			= NULL, \
	.tail 			= NULL, \
	.dl_mutex		= PTHREAD_MUTEX_INITIALIZER, \
	.cond_is_empty 	= PTHREAD_COND_INITIALIZER,  \
	}


extern delta_list_t *create_init_delta_list (void);
extern void destroy_delta_list (delta_list_t *);
extern delta_list_node_t *enqueue_delta_list (delta_list_t *, long int, void *, boolean);
extern op_status_t dequeue_cond_delta_list (delta_list_t *, timer_entry_t *, struct timespec);
extern op_status_t delete_node_delta_list (delta_list_t *, delta_list_node_t **);
extern void delete_nodes_dl (delta_list_t *);
extern delta_list_node_t *insert_delta_list (delta_list_t *, timer_entry_t *, const struct timeval, const struct timeval, struct timeval *, delta_list_node_t **);
extern op_status_t set_and_get_head_timer_val_delta_list (delta_list_t *, struct timespec, struct timespec *);


#endif  //_DELTALIST_H
