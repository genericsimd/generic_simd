/* ----------------------------------------------------------------------------
   IBM Confidential

   IBM Wireless M2M Platform   

   (C)Copyright IBM Corp. 2008, 2009, 2010, 2011

   All Rights Reserved.

   File Name: arq_ds.h

   Change Activity:

   Date             Description of Change                            By
   -----------      ---------------------                            --------
   1-Oct.2008       Created                                          Uma Devi

   ----------------------------------------------------------------------------
   PROLOG END TAG zYx                                                         */

#ifndef _ARQ_DS_H
#define _ARQ_DS_H

#include <time.h>
#include <sys/time.h>
#include <pthread.h>
#include "arq_types.h"
#include "logical_packet.h"
#include "circq_array.h"
#include "binheap.h"
#include "arq_const.h"


typedef enum {
	UNFRAGMENTED=0,	
	LAST = 1,
	FIRST = 2,
	INTERMEDIATE = 3,
	} block_type_t;

typedef enum {
	ARQ_NOT_SENT = 0,
	ARQ_BLK_NOT_RECEIVED = 0, // NOT_SENT and NOT_RECEIVED are
							  // default states for DL and UL
							  // connections, respectively
	ARQ_OUTSTANDING = 1,
	ARQ_WAITING_FOR_RETX = 2,
	ARQ_DISCARDED = 3,
	ARQ_DONE = 4,
	ARQ_BLK_RECEIVED = 6,
	ARQ_ACK_SENT = 7,
	// Temporary book-keeping states
	ARQ_ACK_SEL_WILL_BE_SENT = 8,
	ARQ_ACK_BS_WILL_BE_SENT = 9,
} block_state_t;

typedef enum {
	ARQ_RETRY_TIMEOUT = 1,
	ARQ_BLOCK_LIFETIME = 2,
	ARQ_DISCARD_TIMEOUT = 3,
	ARQ_PURGE_TIMEOUT = 4,
	ARQ_SYNC_LOSS_TIMEOUT = 5,
	ARQ_DUMMY_FEEDBACK = 6,
	ARQ_DUMMY_DISCARD_FEEDBACK = 7,
	ARQ_ACK_GEN = 8,
	ARQ_RESET_TIMEOUT = 9,
} timer_type_t;

typedef struct {
	int start_bsn;
	int	size;
	char *data;
	fragment_type btype;
} blocks_info_t;


struct timer_entry_t{
	short cid;
	int bsn;
	struct timespec fireat;
	timer_type_t timer_type;
};

typedef struct timer_entry_t timer_entry_t;


typedef struct {
	bin_heap_t	*timer_list;
	pthread_t timer_thread;
	timer_t   timeout_timer;
}timer_list_t;
	

typedef struct {
	int bsn;
	//block_type_t type;
	fragment_type type;
	char *data;
	int size;
	block_state_t state;
	timer_entry_t retry_timeout_at;
	timer_entry_t lifetime_timeout_at;
	timer_entry_t feedback_at;
	bin_heap_node_t	*retry_timeout_timer;
	bin_heap_node_t	*block_lifetime_timer;
	bin_heap_node_t *dummy_feedback_timer;
} block_t;

typedef struct {
	block_t *block_buffer; // Is allocated ARQ_TX_WINDOW_SIZE elements at runtime
	int arq_tx_wnd_start_idx;
	pthread_mutex_t arq_wnd_lock;
} block_buffer_t;

typedef struct {
	int bsn;
	// int size;
	block_state_t state;
	timer_entry_t purge_timeout_at;
	bin_heap_node_t *purge_timeout_timer;
} ul_block_t;

typedef struct {
	ul_block_t *ul_block_buffer; // Is allocated ARQ_TX_WINDOW_SIZE elements at runtime
	//int arq_rx_wnd_start_idx;
	pthread_mutex_t arq_wnd_lock;
} ul_block_buffer_t;

typedef struct {
	unsigned char mgmt_msg_type;
	short cid;
	unsigned reserved : 5;
	unsigned bsn : 11;
} ARQ_discard_message;

typedef struct {
	unsigned char mgmt_msg_type;
	short cid;
	unsigned type : 2;
	unsigned direction:1;
	unsigned reserved : 6;
} ARQ_reset_message;

typedef struct {
	short cid;
	unsigned last 		: 1;
	unsigned ack_type 	: 2;
	unsigned bsn		: 11;
	unsigned no_ack_maps: 2;
	char data[0];
} ARQ_feedback_ie;

typedef struct {
	unsigned char mgmt_msg_type;
	//ARQ_feedback_ie acks[0];
	char acks[0]; // Will actually be an array of feedback ies
} ARQ_feedback_message;

typedef struct {
	void *message;
	boolean is_payload_only;
} ARQ_mgmt_msg_t;

typedef struct {
	size_t num_bytes;
	size_t num_blocks;
	size_t num_consec_block_sequences;
} ARQ_ReTX_Q_aggr_info;

typedef struct {
	pthread_t ul_mac_mgmt_thread;
	circq_array_t *ul_mac_mgmt_msg_q;
	timer_list_t dl_timers[MAX_DL_TIMER_THREADS];
	timer_list_t ul_timers[MAX_UL_TIMER_THREADS];
} ARQ_globals;

#endif // _ARQ_DS_H
