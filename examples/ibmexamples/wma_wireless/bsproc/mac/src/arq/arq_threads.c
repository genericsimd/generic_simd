/*-------------------------------------------------------------------
IBM Confidential

IBM Wireless M2M Platform

(C)Copyright IBM Corp. 2009,2010,2011

All Rights Reserved

File Name: arq_threads.c

Change Activity:

Date    	Description of Change        	By
---------------------------------------------------------------
01-Oct-2008 	     Created		   Umamaheshwari C Devi

---------------------------------------------------------------
*/
//#define NO_BIND_DISTINCT

#include <pthread.h>
#include <stdlib.h>
#include <unistd.h>
#include <assert.h>
#include "arq_threads.h"
#include "arq_conn.h"
#include "arq_const.h"
#include "debug.h"
#include "memmgmt.h"
#include "log.h"
#include "flog.h"

#define MAX_CONNS_PER_THREAD 1024

thread_group_t ul_threads;
thread_group_t dl_threads;

extern int param_NUM_ATTACHED_PROCS;
extern int g_sys_procs;

void ARQ_tx_proc_thread_groups_init()
{
	/*
	ul_threads.conns_by_thread = (thread_conns_map_t *)WiMAX_mac_malloc(sizeof(thread_conns_map_t)*MAX_RX_THREADS);
	assert (ul_threads.conns_by_thread != NULL);
	ul_threads.curr_thread_ct = 0;
	ul_threads.max_thread_ct = MAX_RX_THREADS;
	*/

	dl_threads.conns_by_thread = (thread_conns_map_t *)WiMAX_mac_malloc(sizeof(thread_conns_map_t)*MAX_TX_THREADS);
	assert (dl_threads.conns_by_thread != NULL);
	dl_threads.curr_thread_ct = 0;
	dl_threads.max_thread_ct = MAX_TX_THREADS;
	FLOG_DEBUG("Created UL and DL thread groups for processing received and transmitted blocks\n");
}

void ARQ_tx_proc_thread_groups_destroy()
{
	for (int i = 0; i < MAX_TX_THREADS; i++) {
		pthread_cond_signal(&(dl_threads.conns_by_thread[i].thread_cond));
		usleep(100);
		if (dl_threads.conns_by_thread[i].conn_list.dl_conns)
			WiMAX_mac_free (dl_threads.conns_by_thread[i].conn_list.dl_conns);
	}
	if (dl_threads.conns_by_thread) {
		WiMAX_mac_free(dl_threads.conns_by_thread);
	}
	dl_threads.curr_thread_ct = 0;
	dl_threads.max_thread_ct = 0;
}

void ARQ_tx_proc_thread_groups_delete()
{
	WiMAX_mac_free(dl_threads.conns_by_thread);
}

// TO DO: Make thread safe

void add_conn_to_tg (dl_connection_t *conn)
{
	int thread_idx = 0;
	int min_conns = 0xFFFF;
	int i;
	int ret_val;
	char desc[64];
    pthread_attr_t tattr;
    cpu_set_t cpuset;
	//int logical_map[] = {0, 1, 2, 3};
	int logical_map[] = {0, 2, 1, 3};

	FLOG_DEBUG("Assigning connection to a Tx processing thread\n");
	assert (conn != NULL);
	assert (conn->tx_blocks_q != NULL);

	// If MAX_TX_THREADS have not yet been created
	if (dl_threads.curr_thread_ct < dl_threads.max_thread_ct) {
		thread_conns_map_t *conns_by_thread;

		conns_by_thread = &(dl_threads.conns_by_thread[dl_threads.curr_thread_ct]);
		conns_by_thread->conn_list.dl_conns = (dl_connection_t **)WiMAX_mac_calloc (MAX_CONNS_PER_THREAD, sizeof(dl_connection_t *));
		assert (conns_by_thread->conn_list.dl_conns != NULL);
		conns_by_thread->num_conns = 1;
		conns_by_thread->conn_list.dl_conns[0] = conn;
		conns_by_thread->max_valid_conn_idx = 1;

		pthread_mutex_init (&(conns_by_thread->thread_mutex), NULL);
		sprintf(desc, "Mutex assigned to TX blocks processing thread: %d\n", dl_threads.curr_thread_ct);
		add_mutex_to_trace_record(&(conns_by_thread->thread_mutex), desc);
		pthread_cond_init (&(conns_by_thread->thread_cond), NULL);

		set_grp_cond_and_mutex_circq_array (conn->tx_blocks_q, &(conns_by_thread->thread_mutex), &(conns_by_thread->thread_cond));	

		FLOG_DEBUG("Creating a new DL thread\n");
#ifdef NO_BIND_DISTINCT
		ret_val = pthread_create (&(conns_by_thread->thread), NULL, tx_blocks_dequeue_thread, (void *)conns_by_thread);
#else
                 pthread_attr_init(&tattr);
                 CPU_ZERO(&cpuset);
                 //CPU_SET(param_NUM_ATTACHED_PROCS, &cpuset);
                 CPU_SET(logical_map[g_sys_procs-1-dl_threads.curr_thread_ct], &cpuset);
                 FLOG_INFO("param_NUM_ATTACHED_PROCS: %d proc: %d\n", param_NUM_ATTACHED_PROCS, logical_map[g_sys_procs-1-dl_threads.curr_thread_ct]);
                 pthread_attr_setaffinity_np(&tattr, sizeof(cpuset), &cpuset);

		ret_val = pthread_create (&(conns_by_thread->thread), &tattr, tx_blocks_dequeue_thread, (void *)conns_by_thread);
#endif
		assert (ret_val == 0);
#ifndef NO_BIND_DISTINCT
		pthread_attr_destroy(&tattr);
#endif
		pthread_detach(conns_by_thread->thread);

		//conns_by_thread->thread = conn->tx_blocks_deq_thread;

		dl_threads.curr_thread_ct++;

		return;
	}


	// Find the thread with least number of connections
	FLOG_DEBUG("Finding a thread with the least number of assigned connections\n");
	for (i = 0; i < dl_threads.curr_thread_ct; i++) {
		if (dl_threads.conns_by_thread[i].num_conns < min_conns) {
			thread_idx = i;
			min_conns = dl_threads.conns_by_thread[i].num_conns;
		}
	}

	assert (min_conns < MAX_CONNS_PER_THREAD);	
	FLOG_DEBUG("Thread with least number of connections is thread %d of the DL group\n", thread_idx);

	for (i = 0; i < dl_threads.conns_by_thread[thread_idx].max_valid_conn_idx; i++) {
		thread_conns_map_t *conns_by_thread = &(dl_threads.conns_by_thread[thread_idx]);

		assert (conns_by_thread != NULL);

		if (conns_by_thread->conn_list.dl_conns[i] == NULL) {
			conns_by_thread->conn_list.dl_conns[i] = conn;
			set_grp_cond_and_mutex_circq_array (conn->tx_blocks_q, &(conns_by_thread->thread_mutex), &(conns_by_thread->thread_cond));	
			conns_by_thread->num_conns++;
			return;
		}
	}

	dl_threads.conns_by_thread[thread_idx].conn_list.dl_conns[min_conns] = conn;
	dl_threads.conns_by_thread[thread_idx].max_valid_conn_idx++;
	dl_threads.conns_by_thread[thread_idx].num_conns++;
	return;

}

void remove_conn_from_tg (dl_connection_t *conn)
{
	for (int i = 0; i < dl_threads.curr_thread_ct; i++) {
		for (int j = 0; j < dl_threads.conns_by_thread[i].max_valid_conn_idx; j++) {
			if (dl_threads.conns_by_thread[i].conn_list.dl_conns[j] == conn) {
				dl_threads.conns_by_thread[i].conn_list.dl_conns[i] = NULL;
				dl_threads.conns_by_thread[i].num_conns--;
				if (j == dl_threads.conns_by_thread[i].max_valid_conn_idx-1) {
					dl_threads.conns_by_thread[i].max_valid_conn_idx--;
				}
			}
		}
	}
}
