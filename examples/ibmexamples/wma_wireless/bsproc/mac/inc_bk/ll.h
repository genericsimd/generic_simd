/* ----------------------------------------------------------------------------
   IBM Confidential

   IBM Wireless M2M Platform   

   (C)Copyright IBM Corp. 2008, 2009, 2010, 2011

   All Rights Reserved.

   File Name: ll.h

   Change Activity:

   Date             Description of Change                   By
   -----------      ---------------------					--------
   1-Oct.2008       Created									Uma Devi

   ----------------------------------------------------------------------------
   PROLOG END TAG zYx                                                         */

#ifndef _LL_H
#define _LL_H

#include <pthread.h>
#include "arq_types.h"


typedef struct ll_node_t {
	struct ll_node_t *next;
	struct ll_node_t *prev;
	long int  key;	// restricting key to be integral
	char data[0];
} ll_node_t;

typedef struct {
	size_t el_size;
	ll_node_t *head;
	ll_node_t *tail;
	pthread_mutex_t ll_mutex;
	pthread_cond_t cond_is_empty;
} linked_list_t;

#define EMPTY_LIST {	\
	.head 			= NULL, \
	.tail 			= NULL, \
	.ll_mutex		= PTHREAD_MUTEX_INITIALIZER, \
	.cond_is_empty 	= PTHREAD_COND_INITIALIZER,  \
	}


extern linked_list_t *create_init_linked_list (size_t);
extern void destroy_linked_list (linked_list_t *);
extern ll_node_t *enqueue_linked_list (linked_list_t *, long int, void *, boolean);
extern ll_node_t *enqueue_unique_linked_list (linked_list_t *, long int, void *);
ll_node_t *enqueue_arq_wnd_order_linked_list (linked_list_t *ll, long int key,  void *data, short arq_wnd_start);
extern op_status_t dequeue_linked_list (linked_list_t *, long int *, void *, boolean);
extern op_status_t find_element_ll (linked_list_t *, long int, void * );
extern op_status_t peek_ll (linked_list_t *, long int *, void *, boolean); // returns the key and data values of the
														 // head without dequeuing
extern op_status_t update_head_ll (linked_list_t *, long int, void *, boolean);
extern op_status_t unlink_element_ll (linked_list_t *, long int , void *);
extern op_status_t unlink_and_discard_element_ll (linked_list_t *, long int);
extern op_status_t delete_node_linked_list (linked_list_t *, ll_node_t *);
extern boolean is_ll_empty (linked_list_t *);



#endif  //_LL_H

