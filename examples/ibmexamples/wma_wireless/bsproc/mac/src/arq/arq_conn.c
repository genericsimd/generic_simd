/*-------------------------------------------------------------------
IBM Confidential

IBM Wireless M2M Platform

(C)Copyright IBM Corp. 2008,2009,2010,2011

All Rights Reserved

File Name: arq_conn.c

Change Activity:

Date    	Description of Change        	By
---------------------------------------------------------------
01-Oct-2008 	     Created		   Umamaheshwari C Devi

---------------------------------------------------------------
*/

#define NO_BIND_DISTINCT

#include <assert.h>
#include <time.h>
#include <unistd.h>
#include <sys/time.h>
#include <sys/syscall.h>
#include "arq_ds.h"
#include "arq_conn.h"
#include "rbtree.h"
#include "debug.h"
#include "arq_defines.h"
#include "util.h"
#include "arq_const.h"
#include "memmgmt.h"
#include "flog.h"

ARQ_globals globals;

rbtree* uplink_conns_tree;
rbtree* dnlink_conns_tree;

extern void *arq_process_ul_timeouts(void *);
extern void *arq_process_ul_mgmt_msgs(void *);
extern void *arq_process_dl_timeouts(void *);
extern void remove_conn_from_tg (dl_connection_t *);

static int compare_int(void* left, void* right);

extern int param_NUM_ATTACHED_PROCS;
extern int g_sys_procs;

// This map is system specific and has to 
// be different for the CRL box
//int logical_proc_map_CRL[] = {0, 4, 1, 5, 2, 6, 3, 7};
int logical_proc_map[] = {0, 2, 1, 3};
//int logical_proc_map[] = {0, 1, 2, 3};

/*
int compare_int(void* leftp, void* rightp) 
{
#ifdef INTEGRATION_TEST
    long long int left = *(long long int*)(leftp);
	long long int right = *(long long int*)(rightp);
#else
    int left = *(int*)(leftp);
	int right = *(int*)(rightp);
#endif
	if (left < right)
		return -1;
	else if (left > right)
		return 1;
	else {
		assert (left == right);
		return 0;
	}
}
*/
void ARQ_conns_init()
{
	int ret_val;
        cpu_set_t cpuset;
#ifdef INTEGRATION_TEST
	long long int idx;
#else
	int idx;
#endif
	FLOG_DEBUG("In ARQ_conns_init\n");
	uplink_conns_tree = rbtree_create();
	assert (uplink_conns_tree != NULL);
	FLOG_DEBUG("Uplink connections tree created\n");

	for (idx = 0; idx < min(MAX_UL_TIMER_THREADS, param_NUM_ATTACHED_PROCS); idx++) {
		arq_ul_timers[idx].timer_list = create_init_bin_heap();
		assert (arq_ul_timers[idx].timer_list != NULL);

		FLOG_DEBUG("Created UL Timer List\n");
		// Clear head time
		//timerclear(&(ul_timers.head_time));
		dl_timer_thread_done = 0;
		ul_timer_thread_done = 0;
		pthread_attr_t tattr;
        pthread_attr_init(&tattr);
		pthread_attr_setdetachstate(&tattr, PTHREAD_CREATE_JOINABLE);

#ifdef NO_BIND_DISTINCT
		ret_val = pthread_create (&(arq_ul_timers[idx].timer_thread),  &tattr, arq_process_ul_timeouts, NULL);
#else
        CPU_ZERO(&cpuset);
        CPU_SET(logical_proc_map[g_sys_procs-1-idx], &cpuset);
        pthread_attr_setaffinity_np(&tattr, sizeof(cpuset), &cpuset);

		ret_val = pthread_create (&(arq_ul_timers[idx].timer_thread),  &tattr, arq_process_ul_timeouts, NULL);
#endif
		 assert (0 == ret_val);
		 //pthread_detach((arq_ul_timers[idx].timer_thread));
		 pthread_attr_destroy(&tattr);
	}

	arq_ul_mac_mgmt_msg_q = create_circq_array (ARQ_MGMT_MSG_Q_SIZE, sizeof(ARQ_mgmt_msg_t));
	assert (arq_ul_mac_mgmt_msg_q != NULL);
/*
	ret_val = pthread_create (&arq_ul_mac_mgmt_thread, NULL, arq_process_ul_mgmt_msgs, NULL);
	assert (0 == ret_val);
	pthread_detach(arq_ul_mac_mgmt_thread);
	TRACE(4, "Created UL MAC mgmt message processing thread\n");
*/
	dnlink_conns_tree = rbtree_create();
	assert (dnlink_conns_tree != NULL);
	FLOG_DEBUG("Downlink connections tree created\n");

	// Create the MAXIMUM configured downlink timer lists and
	// threads to process them
	for (idx = 0; idx < min(MAX_DL_TIMER_THREADS, param_NUM_ATTACHED_PROCS); idx++) {
		arq_dl_timers[idx].timer_list = create_init_bin_heap ();
		assert (arq_dl_timers[idx].timer_list != NULL);
		FLOG_DEBUG("Created DL Timer List\n");
		//timerclear(&(arq_dl_timers[idx].head_time));

		//pthread_mutex_init(&(arq_dl_timers[idx].head_time_mutex), NULL);
/*
		 pthread_attr_init(&tattr);
		 CPU_ZERO(&cpuset);
		 CPU_SET(logical_proc_map[g_sys_procs-1-idx], &cpuset);
		 pthread_attr_setaffinity_np(&tattr, sizeof(cpuset), &cpuset);
*/
		pthread_attr_t tattr;
        pthread_attr_init(&tattr);
		pthread_attr_setdetachstate(&tattr, PTHREAD_CREATE_JOINABLE);
#ifdef NO_BIND_DISTINCT
		ret_val = pthread_create (&arq_dl_timers[idx].timer_thread,  &tattr, arq_process_dl_timeouts, (void*)idx);
#else
        CPU_ZERO(&cpuset);
		CPU_SET(logical_proc_map[g_sys_procs-1-idx], &cpuset);
        pthread_attr_setaffinity_np(&tattr, sizeof(cpuset), &cpuset);

		ret_val = pthread_create (&arq_dl_timers[idx].timer_thread,  &tattr, arq_process_dl_timeouts, idx);
#endif
		assert (0 == ret_val);
//#ifndef NO_BIND_DISTINCT
		CPU_ZERO(&cpuset);
		pthread_getaffinity_np(arq_dl_timers[idx].timer_thread, sizeof(cpuset), &cpuset);
		int cpu;
		for ( cpu = 0; cpu < 4; cpu++) {
			if (CPU_ISSET(cpu, &cpuset)) {
			  FLOG_DEBUG("timer thread %ld has affinity to processor %d\n", syscall(SYS_gettid), cpu);
			  //printf("timer thread %ld has affinity to processor %d\n", syscall(SYS_gettid), cpu);
		  }
	   }
//#endif
	   //pthread_detach(arq_dl_timers[idx].timer_thread);
	   pthread_attr_destroy(&tattr);
	}

}

void add_new_dl_connection(dl_connection_t *new_conn)
{
#ifdef INTEGRATION_TEST
	long long int cid;
#else
	int cid;
#endif
	assert (new_conn != NULL);

	cid = new_conn->cid;
	assert (rbtree_lookup(dnlink_conns_tree, (void *)(cid)) == NULL);
	rbtree_insert(dnlink_conns_tree, (void *)(cid), new_conn);
	FLOG_INFO("Inserted ARQ Connection for DL\n");
	assert (rbtree_lookup(dnlink_conns_tree, (void *)(cid)) != NULL);
}

void add_new_ul_connection(ul_connection_t *new_conn)
{
#ifdef INTEGRATION_TEST
	long long int cid;
#else
	int cid;
#endif
	assert (new_conn != NULL);

	cid = new_conn->cid;
	assert (rbtree_lookup(uplink_conns_tree, (void *)(cid)) == NULL);
	rbtree_insert(uplink_conns_tree, (void *)(cid), new_conn);
	FLOG_INFO("Inserted ARQ Connection for UL\n");
	assert (rbtree_lookup(uplink_conns_tree, (void *)(cid))  != NULL);
}

dl_connection_t *find_dl_connection(short cid)
{
	dl_connection_t *connection;
#ifdef INTEGRATION_TEST
	long long int icid = cid;
#else
	int icid = cid;
#endif

	connection = (dl_connection_t *)rbtree_lookup(dnlink_conns_tree, (void *)icid);

	return connection;
}

ul_connection_t *find_ul_connection(short cid)
{
	ul_connection_t *connection;
#ifdef INTEGRATION_TEST
	long long int icid = cid;
#else
	int icid = cid;
#endif

	connection = (ul_connection_t *)rbtree_lookup(uplink_conns_tree, (void *)icid);

	return connection;
}

void remove_dl_connection (short cid)
{
	dl_connection_t *connection;
#ifdef INTEGRATION_TEST
	long long int icid = cid;
#else	
	int icid = cid;
#endif

	connection = (dl_connection_t *)rbtree_lookup(dnlink_conns_tree, (void *)icid);
	if (NULL == connection) {
		return;
	}

	destroy_circq_array (connection->tx_blocks_q);
	destroy_linked_list (connection->retrans_q);
	//free (connection->conn_mgmt_q);
	WiMAX_mac_free(connection->block_buffer.block_buffer);
	pthread_mutex_destroy (&(connection->retrans_aggr_lock));
	pthread_mutex_destroy (&(connection->block_buffer.arq_wnd_lock));
	remove_conn_from_tg(connection);

	free (connection);

	rbtree_delete(dnlink_conns_tree, (void*)icid);
	FLOG_INFO("Deleted ARQ Connection for DL\n");

}

void remove_ul_connection (short cid)
{
	ul_connection_t *connection;
#ifdef INTEGRATION_TEST
	long long int icid = cid;
#else
	int icid = cid;
#endif

	connection = (ul_connection_t *)rbtree_lookup(uplink_conns_tree, (void *)icid);
	if (NULL == connection) {
		return;
	}

	WiMAX_mac_free(connection->ul_block_buffer.ul_block_buffer);
	pthread_mutex_destroy (&(connection->ul_block_buffer.arq_wnd_lock));

	free(connection);

	rbtree_delete(uplink_conns_tree, (void*)icid);

	FLOG_INFO("Deleted ARQ Connection for DL\n");
}

void set_dl_conn_arq_params (dl_connection_t *dl_conn, int arq_wnd_sz, int arq_blk_sz, struct timespec arq_blk_lifetime, struct timespec arq_sync_loss, struct timespec arq_retry_timeout, float block_loss_prob, boolean is_frag_enabled)
{
	assert(dl_conn != NULL);

	dl_conn->ARQ_TX_WINDOW_SIZE = arq_wnd_sz;
	dl_conn->ARQ_BLOCK_SIZE = arq_blk_sz;
	dl_conn->arq_block_lifetime = arq_blk_lifetime;
	dl_conn->arq_sync_loss_timeout = arq_sync_loss;
	dl_conn->arq_retry_timeout = arq_retry_timeout;
	dl_conn->is_frag_enabled = is_frag_enabled;
	dl_conn->block_loss_prob = block_loss_prob;

	FLOG_DEBUG("ARQ Initialization : arq_retry_timeout: %ld %ld sec %ld %ld nsec\n", 
				arq_retry_timeout.tv_sec, dl_conn->arq_retry_timeout.tv_sec, arq_retry_timeout.tv_nsec, dl_conn->arq_retry_timeout.tv_nsec);
	FLOG_DEBUG("ARQ Initialization : arq_block_lifetime: %ld %ld sec %ld %ld nsec\n", 
				arq_blk_lifetime.tv_sec, dl_conn->arq_block_lifetime.tv_sec, arq_blk_lifetime.tv_nsec, dl_conn->arq_block_lifetime.tv_nsec);
}

void set_ul_conn_arq_params (ul_connection_t *ul_conn, int arq_wnd_sz, int arq_blk_sz, struct timespec arq_sync_loss, struct timespec arq_purge_timeout)
{
	assert(ul_conn != NULL);

	ul_conn->ARQ_RX_WINDOW_SIZE = arq_wnd_sz;
	ul_conn->ARQ_BLOCK_SIZE = arq_blk_sz;
	ul_conn->arq_sync_loss_timeout = arq_sync_loss;
	ul_conn->arq_purge_timeout = arq_purge_timeout;
}
