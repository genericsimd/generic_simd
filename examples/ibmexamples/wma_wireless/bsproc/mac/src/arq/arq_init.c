/*-------------------------------------------------------------------
IBM Confidential

IBM Wireless M2M Platform

(C)Copyright IBM Corp. 2008,2009,2010,2011

All Rights Reserved

File Name: arq_init.c

Change Activity:

Date    	Description of Change        	By
---------------------------------------------------------------
01-Oct-2008 	     Created		   Umamaheshwari C Devi

---------------------------------------------------------------
*/
#include <stdlib.h>
#include <assert.h>
#include <time.h>
#include "arq_types.h"
#include "circq_array.h"
#include "arq_ds.h"
#include "arq_defines.h"
#include "arq_conn.h"
#include "arq_threads.h"
#include "ll.h"
#include "rbtree.h"
#include "binheap.h"
#include "debug.h"
#include "arq_const.h"
#include "memmgmt.h"
#include "log.h"
#include "flog.h"
#include "arq_ifaces.h"

#define TX_BLOCKS_Q_SIZE	ARQ_WINDOW_SIZE_DEF
#define CONN_MGMT_Q_SIZE	5

#define MAX_TRACE_BUFFERS	50

const int true = 0;
const int false = -1;

extern void set_ul_conn_arq_params (ul_connection_t *, int, int, struct timespec, struct timespec);
extern void set_dl_conn_arq_params (dl_connection_t *, int, int, struct timespec, struct timespec, struct timespec, float, boolean);
extern bin_heap_node_t *insert_dl_timer (dl_connection_t *, timer_type_t, int, struct timespec, struct timeval, bin_heap_node_t **);
extern bin_heap_node_t *insert_ul_timer (ul_connection_t *, timer_type_t, int, struct timespec, struct timeval, bin_heap_node_t **);
extern void add_conn_to_tg (dl_connection_t *conn);
extern void ARQ_tx_proc_thread_groups_destroy();

extern int param_NUM_ATTACHED_PROCS;
extern rbtree* uplink_conns_tree;
extern rbtree* dnlink_conns_tree;

void ARQ_init()
{
	FLOG_DEBUG("In ARQ_init\n");
	//init_trace_buffer_record(MAX_TRACE_BUFFERS); //-- This initialization function should be called from the main module
	ARQ_conns_init();
	ARQ_tx_proc_thread_groups_init();
	//start_log_reader("wmac.traces.out");
}

void ARQ_shutdown()
{
	FLOG_INFO("ARQ Shutdown Started\n");
	int rc;
	for (int idx = 0; idx < min(MAX_DL_TIMER_THREADS, param_NUM_ATTACHED_PROCS); idx++) {
		if (dl_timer_thread_done == 0)
		{
			//rc = pthread_join(arq_dl_timers[idx].timer_thread,NULL);
			rc = pthread_cancel(arq_dl_timers[idx].timer_thread);
			if (rc)
			{
				FLOG_ERROR("Error code from ARQ DL Timer thread join\n");
			}
			else
			{
				FLOG_INFO("Terminated ARQ DL Timeouts Thread\n");
			}
		}
		destroy_bin_heap(arq_dl_timers[idx].timer_list);
	}

	for (int idx = 0; idx < min(MAX_UL_TIMER_THREADS, param_NUM_ATTACHED_PROCS); idx++) {
		if (ul_timer_thread_done == 0)
		{
			//rc = pthread_join(arq_ul_timers[idx].timer_thread,NULL);
			rc = pthread_cancel(arq_ul_timers[idx].timer_thread);
			if (rc)
			{
				FLOG_ERROR("Error code from ARQ UL Timer thread join\n");
			}
			else
			{
				FLOG_INFO("Terminated ARQ UL Timeouts Thread\n");
			}
		}
		destroy_bin_heap(arq_ul_timers[idx].timer_list);
	}
	for (int cid = 0; cid < MAX_CIDS; cid++) {
		remove_dl_connection (cid);
		remove_ul_connection (cid);
	}
	ARQ_tx_proc_thread_groups_destroy();
	destroy_circq_array (arq_ul_mac_mgmt_msg_q);
	rbtree_destroy(dnlink_conns_tree);
	rbtree_destroy(uplink_conns_tree);
	FLOG_INFO("Completed ARQ Shutdown\n");	
}

// This function needs to be called when a new Uplink connection 
// is created
op_status_t ARQ_uplink_conn_init(short cid, short wnd_size, short block_size)
{
	static int next_ul_thread = 0;
	ul_connection_t *new_conn;

	FLOG_DEBUG("Initializing uplink connection with ID %d\n", cid);

	if (find_ul_connection(cid) != NULL) {
		return E_DUPLICATE_CONN;
	}

	new_conn = (ul_connection_t *)WiMAX_mac_calloc (1, sizeof(ul_connection_t));
	assert (new_conn != NULL);

	new_conn->cid = cid;
	new_conn->arq_sync_loss_timer = NULL;
	new_conn->is_reset = true;  // should be unset when some block is received
	new_conn->timer_thrd_idx = next_ul_thread;
	next_ul_thread = (next_ul_thread+1)%MAX_UL_TIMER_THREADS;

	// Set ARQ params
	//set_ul_conn_arq_params (new_conn, ARQ_WINDOW_SIZE_DEF, ARQ_BLOCK_SIZE_DEF,
							//ARQ_SYNC_LOSS_TIMEOUT_DEF, ARQ_PURGE_TIMEOUT_DEF);

	set_ul_conn_arq_params (new_conn, wnd_size, block_size, ARQ_SYNC_LOSS_TIMEOUT_DEF, ARQ_PURGE_TIMEOUT_DEF);

	//new_conn->conn_mgmt_q = create_circq_array (CONN_MGMT_Q_SIZE, sizeof (int));
	//assert (new_conn->conn_mgmt_q != NULL);

	new_conn->ul_block_buffer.ul_block_buffer = (ul_block_t *)WiMAX_mac_calloc(new_conn->ARQ_RX_WINDOW_SIZE, sizeof(ul_block_t));
	assert (new_conn->ul_block_buffer.ul_block_buffer != NULL);
	//new_conn->ul_block_buffer.arq_rx_wnd_start_idx = 0;
	new_conn->arq_last_cum_ack = ARQ_BSN_MODULUS;
	pthread_mutex_init (&(new_conn->ul_block_buffer.arq_wnd_lock), NULL);
	add_mutex_to_trace_record(&(new_conn->ul_block_buffer.arq_wnd_lock), "UL block buffer lock");
	FLOG_DEBUG("Created UL Blocks Buffer\n");

	add_new_ul_connection (new_conn);	
	//add_conn_to_tg (new_conn);

	struct timeval curr_time;
	gettimeofday(&curr_time, NULL);
	insert_ul_timer (new_conn, ARQ_ACK_GEN, 0, ARQ_ACK_INTERVAL_INIT, curr_time, &(new_conn->arq_ack_gen_timer));
#ifdef TEST_UL_SYNC_LOSS
	gettimeofday(&curr_time, NULL);
	struct timespec temptime;
	temptime.tv_sec=ARQ_SYNC_LOSS_TIMEOUT_DEF.tv_sec +1;
	temptime.tv_nsec = ARQ_SYNC_LOSS_TIMEOUT_DEF.tv_nsec;
	//new_conn->arq_rx_highest_bsn += 1;
	insert_ul_timer (new_conn, ARQ_SYNC_LOSS_TIMEOUT, 0, ARQ_SYNC_LOSS_TIMEOUT_DEF, curr_time, &(new_conn->arq_sync_loss_timer));
#endif
	return SUCCESS;
}

// This function needs to be called when a new downlink connection
// is created
op_status_t ARQ_downlink_conn_init(short cid, short wnd_size, short block_size, float blk_loss_prob, boolean is_frag_enabled)
{
	static int next_thread = 0;
	dl_connection_t *new_conn;

	FLOG_DEBUG("Initializing downlink connection with ID %d\n", cid);

	if (find_dl_connection(cid) != NULL) {
		return E_DUPLICATE_CONN;
	}

	new_conn = (dl_connection_t *)WiMAX_mac_calloc (1, sizeof(dl_connection_t));
	assert (new_conn != NULL);

	new_conn->cid = cid;
	new_conn->timer_thrd_idx = next_thread;
	FLOG_INFO("Thread for connection %d : %d\n", cid, next_thread);
	next_thread = (next_thread+1)%MAX_DL_TIMER_THREADS;

	// Set ARQ params
	FLOG_DEBUG("ARQ Initialization : ARQ_RETRY_TIMEOUT: %ld %ld ARQ_BLOCK_LIFETIME_DEF: sec %ld %ld nsec\n", 
				ARQ_RETRY_TIMEOUT_DEF.tv_sec, ARQ_RETRY_TIMEOUT_DEF.tv_nsec, ARQ_BLOCK_LIFETIME_DEF.tv_sec, ARQ_BLOCK_LIFETIME_DEF.tv_nsec);
	set_dl_conn_arq_params (new_conn, wnd_size, block_size, ARQ_BLOCK_LIFETIME_DEF, ARQ_SYNC_LOSS_TIMEOUT_DEF, ARQ_RETRY_TIMEOUT_DEF, 
							blk_loss_prob, is_frag_enabled);

	new_conn->tx_blocks_q = create_circq_array (TX_BLOCKS_Q_SIZE, sizeof (blocks_info_t));
	assert (new_conn->tx_blocks_q != NULL);
	FLOG_DEBUG("Created Transmission Blocks Queue\n");

	new_conn->retrans_q_aggr_info.num_bytes = 0;
	new_conn->retrans_q_aggr_info.num_blocks = 0;
	new_conn->retrans_q_aggr_info.num_consec_block_sequences = 0;

	new_conn->retrans_q = create_init_linked_list (sizeof(blocks_info_t));
	assert (new_conn->retrans_q != NULL);
	FLOG_DEBUG("Created Waiting for Retransmission Queue\n");

	pthread_mutex_init (&(new_conn->retrans_aggr_lock), NULL);
	add_mutex_to_trace_record(&(new_conn->retrans_aggr_lock), "Lock for aggregate info of Retransmission Queue");

	//new_conn->conn_mgmt_q = create_circq_array (CONN_MGMT_Q_SIZE, sizeof (int));
	//assert (new_conn->conn_mgmt_q != NULL);

	new_conn->block_buffer.block_buffer = (block_t *)WiMAX_mac_calloc(new_conn->ARQ_TX_WINDOW_SIZE, sizeof(block_t));
	assert (new_conn->block_buffer.block_buffer != NULL);
	new_conn->block_buffer.arq_tx_wnd_start_idx = 0;
	pthread_mutex_init (&(new_conn->block_buffer.arq_wnd_lock), NULL);
	add_mutex_to_trace_record(&(new_conn->block_buffer.arq_wnd_lock), "DL block buffer lock");
	FLOG_DEBUG("Created Blocks Buffer\n");

#ifdef TEST_DL_SYNC_LOSS
	struct timeval curr_time;
	gettimeofday(&curr_time, NULL);
	struct timespec temptime;
	temptime.tv_sec=ARQ_SYNC_LOSS_TIMEOUT_DEF.tv_sec +1;
	temptime.tv_nsec = ARQ_SYNC_LOSS_TIMEOUT_DEF.tv_nsec;
	new_conn->arq_tx_next_bsn += 1;
	insert_dl_timer (new_conn, ARQ_SYNC_LOSS_TIMEOUT, 0, ARQ_SYNC_LOSS_TIMEOUT_DEF, curr_time, &(new_conn->arq_sync_loss_timer));
#endif
	add_new_dl_connection (new_conn);	
	add_conn_to_tg (new_conn);

	return SUCCESS;
}

void ARQ_downlink_conn_teardown(short cid)
{
	dl_connection_t *conn = find_dl_connection(cid);
	conn = conn;
}
