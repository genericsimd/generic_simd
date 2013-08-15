/* ----------------------------------------------------------------------------
   IBM Confidential

   IBM Wireless M2M Platform   

   (C)Copyright IBM Corp. 2008, 2009, 2010, 2011

   All Rights Reserved.

   File Name: arq_threads.h

   Change Activity:

   Date             Description of Change                            By
   -----------      ---------------------                            --------
   1-Oct.2008       Created                                          Uma Devi

   ----------------------------------------------------------------------------
   PROLOG END TAG zYx                                                         */

#ifndef _ARQ_THREADS_H
#define _ARQ_THREADS_H
#include "arq_conn.h"

typedef dl_connection_t *dl_conn_ptr_t;

typedef struct thread_conns_map {
	pthread_t	thread;
	int			num_conns;
	int 		max_valid_conn_idx;
	//int			*conn_list;
	union {
		dl_conn_ptr_t *dl_conns;
		dl_conn_ptr_t *ul_conns;
	} conn_list;
	pthread_mutex_t	 thread_mutex;
	pthread_cond_t   thread_cond;
} thread_conns_map_t;

typedef struct {
	thread_conns_map_t *conns_by_thread;
	int 			 curr_thread_ct;
	int				 max_thread_ct;
} thread_group_t;

extern thread_group_t ul_threads;
extern thread_group_t dl_threads;

extern void ARQ_tx_proc_thread_groups_init();
extern void *tx_blocks_dequeue_thread(void *);

#endif // _ARQ_THREADS_H
