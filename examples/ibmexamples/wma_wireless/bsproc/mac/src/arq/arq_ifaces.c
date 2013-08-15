/*-------------------------------------------------------------------
IBM Confidential

IBM Wireless M2M Platform

(C)Copyright IBM Corp. 2008,2009,2010,2011

All Rights Reserved

File Name: arq_ifaces.c

Change Activity:

Date    	Description of Change        	By
---------------------------------------------------------------
01-Oct-2008 	     Created		   Umamaheshwari C Devi

---------------------------------------------------------------
*/
#include <string.h>
#include <assert.h>
#include "circq_array.h"
#include "arq_ds.h"
#include "arq_defines.h"
#include "arq_conn.h"
#include "mac_sdu_queue.h"
#include "debug.h"
#include "memmgmt.h"
#include "log.h"
#include "flog.h"
#include "mutex_macros.h"

op_status_t ARQ_enqueue_tx_blocks_q(short cid, blocks_info_t *element)
{
	op_status_t ret_status;
	dl_connection_t *dl_conn;
	circq_array_t *circ_q;

	FLOG_DEBUG("In ARQ_enqueue_tx_blocks_q...\n");
	dl_conn = find_dl_connection(cid);
	assert (dl_conn != NULL);

	circ_q = dl_conn->tx_blocks_q;
	assert (circ_q != NULL);

	ret_status = enqueue_circq_array(circ_q, (void *)element, false, true);
	return ret_status;
}

op_status_t ARQ_set_tx_next_bsn(short cid, int next_bsn)
{
	dl_connection_t *dl_conn;

	dl_conn = find_dl_connection(cid);
	assert (dl_conn != NULL);

	dl_conn->arq_tx_next_bsn = next_bsn;

	return SUCCESS;
}

int ARQ_get_tx_window_start(short cid)
{
	dl_connection_t *dl_conn;

	dl_conn = find_dl_connection(cid);
	assert (dl_conn != NULL);

	return dl_conn->arq_tx_window_start;
}

int ARQ_get_tx_window_size(short cid)
{
	dl_connection_t *dl_conn;

	dl_conn = find_dl_connection(cid);
	assert (dl_conn != NULL);

	return dl_conn->ARQ_TX_WINDOW_SIZE;
}

int ARQ_get_rx_window_start(short cid)
{
	ul_connection_t *ul_conn;

	ul_conn = find_ul_connection(cid);
	assert (ul_conn != NULL);

	return ul_conn->arq_rx_window_start;
}

int ARQ_get_rx_window_size(short cid)
{
	ul_connection_t *ul_conn;

	ul_conn = find_ul_connection(cid);
	assert (ul_conn != NULL);

	return ul_conn->ARQ_RX_WINDOW_SIZE;
}

int ARQ_get_tx_block_size(short cid)
{
	dl_connection_t *dl_conn;

	dl_conn = find_dl_connection(cid);
	assert (dl_conn != NULL);

	return dl_conn->ARQ_BLOCK_SIZE;
}

int ARQ_get_rx_block_size(short cid)
{
	ul_connection_t *ul_conn;

	ul_conn = find_ul_connection(cid);
	assert (ul_conn != NULL);

	return ul_conn->ARQ_BLOCK_SIZE;
}


void ARQ_enqueue_ReTX_q_unique(dl_connection_t *conn, long int key, blocks_info_t *bl_info)
{

	int num_blks = bl_info->size/conn->ARQ_BLOCK_SIZE;

	if (num_blks*conn->ARQ_BLOCK_SIZE < bl_info->size) {
		num_blks++;
	}
	pthread_mutex_lock(&conn->retrans_aggr_lock);
	//if (enqueue_unique_linked_list(conn->retrans_q, key, (void *)bl_info) != NULL) {
	if (enqueue_arq_wnd_order_linked_list(conn->retrans_q, key, (void *)bl_info, conn->arq_tx_window_start) != NULL) {
		conn->retrans_q_aggr_info.num_bytes += bl_info->size;
		conn->retrans_q_aggr_info.num_blocks += num_blks;
	}
	FLOG_DEBUG("ARQ_enqueue_ReTX_q_unique: total num_bytes %d total num_blocks: %d\n", conn->retrans_q_aggr_info.num_bytes,
														conn->retrans_q_aggr_info.num_blocks);
	pthread_mutex_unlock(&conn->retrans_aggr_lock);
}

// Size should be passed in.. can be obtained from block buffer...
op_status_t ARQ_delete_ReTX_q(dl_connection_t *conn, short bsn, size_t size)
{
	op_status_t ret_val;
	pthread_mutex_lock(&conn->retrans_aggr_lock);

	if ((ret_val = unlink_and_discard_element_ll(conn->retrans_q, bsn)) == SUCCESS) {
		conn->retrans_q_aggr_info.num_bytes -= size;
		int num_blks = size/conn->ARQ_BLOCK_SIZE;

		if (num_blks*conn->ARQ_BLOCK_SIZE < size) {
			num_blks++;
		}
		conn->retrans_q_aggr_info.num_blocks -= num_blks;
	}

	FLOG_DEBUG("ARQ_delete_ReTX_q: total num_bytes %d total num_blocks: %d\n", conn->retrans_q_aggr_info.num_bytes,
														conn->retrans_q_aggr_info.num_blocks);
	pthread_mutex_unlock(&conn->retrans_aggr_lock);

	return SUCCESS;
}

ARQ_ReTX_Q_aggr_info ARQ_get_ReTX_queue_aggr_info(short cid)
{
	ARQ_ReTX_Q_aggr_info aggr_info = {0,0,0};
	dl_connection_t *conn = find_dl_connection(cid);

	assert(conn != NULL);

	if (conn != NULL) {
		pthread_mutex_lock (&conn->retrans_aggr_lock);
		aggr_info = conn->retrans_q_aggr_info;
		FLOG_DEBUG("ARQ_get_ReTX_q_aggr_info: total num_bytes %d total num_blocks: %d\n", aggr_info.num_bytes,
														aggr_info.num_blocks);
		pthread_mutex_unlock (&conn->retrans_aggr_lock);
	}
	return aggr_info;
}

void ARQ_dequeue_ReTX_q_cond (short cid, size_t num_bytes, size_t num_blocks, logical_packet **block_seq)
{
}

op_status_t ARQ_dequeue_ReTX_q (short cid, blocks_info_t *bl_info)
{
	op_status_t ret_val = E_INVALID_CONN;
	dl_connection_t *conn = find_dl_connection (cid);

	assert (conn != NULL);

	FLOG_DEBUG("In ARQ_dequeue_ReTX_q...\n");

	if (NULL != conn) {
		long bsn;
		pthread_mutex_lock (&conn->retrans_aggr_lock);
		ret_val = dequeue_linked_list (conn->retrans_q, &bsn, (void *)bl_info, true);
		if (SUCCESS == ret_val) {
			conn->retrans_q_aggr_info.num_bytes -= bl_info->size;
			int num_blks;
			num_blks = bl_info->size/conn->ARQ_BLOCK_SIZE;

			if (num_blks*conn->ARQ_BLOCK_SIZE < bl_info->size) {
				num_blks++;
			}
			conn->retrans_q_aggr_info.num_blocks -= num_blks;
		}
		FLOG_DEBUG("ARQ_dequeue_ReTX_q: total num_bytes %d total num_blocks: %d\n", conn->retrans_q_aggr_info.num_bytes,
														conn->retrans_q_aggr_info.num_blocks);
		pthread_mutex_unlock (&conn->retrans_aggr_lock);
		if (SUCCESS == ret_val) {
			block_t *bl = &(conn->block_buffer.block_buffer[bsn%conn->ARQ_TX_WINDOW_SIZE]);
			if (bl->bsn == bsn) {
				bl->state = ARQ_OUTSTANDING;
			}
		}
	}
	return ret_val;
}

int ARQ_num_conn_waiting_for_ReTX(short cid_begin, short cid_end)
{
	int i;
	int num_waiting = 0;

	for (i = cid_begin; i <= cid_end; i++) {
		dl_connection_t *conn;

		conn = find_dl_connection(i);
		if (conn!= NULL && false == is_ll_empty(conn->retrans_q)) {
			num_waiting++;
		}
	}
	FLOG_DEBUG("ARQ_num_conn_waiting_for_ReTX: Number of connections waiting for ReTX: %d\n", num_waiting);
	return num_waiting;
}

op_status_t ARQ_enqueue_mgmt_msg (void *msg, size_t size, boolean is_payload_only)
{
	char *arq_msg = (char *)WiMAX_mac_malloc(size);

	assert (NULL != arq_msg);

	memcpy(arq_msg, msg, size);

	ARQ_mgmt_msg_t enq_msg;
	enq_msg.message = arq_msg;
	enq_msg.is_payload_only = is_payload_only;

	return enqueue_circq_array (arq_ul_mac_mgmt_msg_q, (void *)&enq_msg, false, false);	
}


int timespec_cmp(void* _a, void* _b)
{
	struct timespec *a, *b;
	a = (struct timespec*)&(((timer_entry_t *)_a)->fireat);
	b = (struct timespec*)&(((timer_entry_t *)_b)->fireat);
	return (timercmp_ts(a, b, <));
}

int copy_timer_entry(void *a, void *b)
{
	memcpy(a, b, sizeof (timer_entry_t));
	return 0;
}

int print_timer_entry(void *a)
{
	timer_entry_t *te = (timer_entry_t*)a;
	FLOG_DEBUG("value: CONN: %d BSN: %d timer type: %d fireat %ld sec %ld nsec\n", 
				te->cid, te->bsn, te->timer_type, te->fireat.tv_sec, te->fireat.tv_nsec);

        (void)te;
	return 0;
}

/*
int get_basic_cid (int cid, int *b_cid)
{
	if (cid <= 1024) {
		*b_cid = cid+1024;
	} else {
		*b_cid = cid-1024;
	}
	return *b_cid;
}

int enqueue_transport_sdu_queue(sdu_queue* sdu_q, int cid, size_t num_bytes, void* physicalSdu) 
{
	return 0;
}
*/
