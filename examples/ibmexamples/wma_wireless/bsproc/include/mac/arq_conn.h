/* ----------------------------------------------------------------------------
   IBM Confidential

   IBM Wireless M2M Platform   

   (C)Copyright IBM Corp. 2008, 2009, 2010, 2011

   All Rights Reserved.

   File Name: arq_conn.h

   Change Activity:

   Date             Description of Change                            By
   -----------      ---------------------                            --------
   1-Oct.2008       Created                                          Uma Devi

   ----------------------------------------------------------------------------
   PROLOG END TAG zYx                                                         */

#ifndef _ARQ_CONN_H
#define _ARQ_CONN_H
#include <time.h>
#include <pthread.h>
#include "circq_array.h"
#include "ll.h"
#include "arq_ds.h"

typedef struct {
	short cid;
	circq_array_t *tx_blocks_q;
	circq_array_t *conn_mgmt_q;
	linked_list_t *retrans_q;
	ARQ_ReTX_Q_aggr_info retrans_q_aggr_info;
	pthread_mutex_t retrans_aggr_lock;
	block_buffer_t	block_buffer;
	//timer_list_t timer_list;
	bin_heap_node_t *arq_sync_loss_timer;
	timer_entry_t	sync_loss_timeout_at;
	short			timer_thrd_idx;

	short	ARQ_BLOCK_SIZE;    
	short ARQ_TX_WINDOW_SIZE;
	struct timespec arq_block_lifetime; 
	struct timespec  arq_sync_loss_timeout;
	struct timespec  arq_retry_timeout; 
	boolean is_frag_enabled;
	float 	block_loss_prob;

	short arq_tx_window_start;
	short arq_tx_next_bsn;
} dl_connection_t;

typedef struct {
	short cid;
	circq_array_t *conn_mgmt_q;
	ul_block_buffer_t	ul_block_buffer;
	timer_list_t timer_list;
	//pthread_t    timer_thread;
	bin_heap_node_t *arq_sync_loss_timer;
	timer_entry_t sync_loss_timeout_at;
	bin_heap_node_t *arq_ack_gen_timer;
	timer_entry_t ack_gen_at;
	boolean 	is_reset;
	short 		timer_thrd_idx;

	short	ARQ_BLOCK_SIZE;    
	short ARQ_RX_WINDOW_SIZE;
	struct timespec  arq_sync_loss_timeout;
	struct timespec  arq_purge_timeout; 
	struct timespec  arq_retry_timeout;

	short arq_rx_window_start;
	short arq_rx_highest_bsn;
	short arq_last_cum_ack;
} ul_connection_t;


extern void ARQ_conns_init();
extern void add_new_dl_connection(dl_connection_t *new_conn);


extern dl_connection_t *find_dl_connection(short cid);
extern ul_connection_t *find_ul_connection(short cid);
extern void remove_dl_connection (short cid);
extern void remove_ul_connection (short cid);
extern void ARQ_downlink_conn_teardown (short cid);
extern void add_new_dl_connection(dl_connection_t *new_conn);
extern void add_new_ul_connection(ul_connection_t *new_conn);




extern linked_list_t *find_retrans_q(short cid);
extern circq_array_t *find_tx_blocks_q(short cid);

extern op_status_t enqueue_tx_blocks (short cid, void *, boolean, boolean);
extern op_status_t dequeue_retrans_q (short cid, int *, void **, boolean);

extern void set_arq_tx_next_bsn (short cid, int next_bsn);

#define set_arq_tx_next_bsn (cid, next_bsn) \
		{\
			dl_connection_t *conn = find_dl_connection(cid); \
			assert (conn != NULL); \
			conn->arq_tx_next_bsn = next_bsn; \
		}
#define get_arq_tx_window_start (cid) \
		{\
			dl_connection_t *conn; \
			(conn = find_dl_connection (cid)) ? conn->arq_tx_window_start : -1 ; \
		}
				

#endif //_ARQ_CONN_H
