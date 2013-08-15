/*-------------------------------------------------------------------
IBM Confidential

IBM Wireless M2M Platform

(C)Copyright IBM Corp. 2009,2010,2011

All Rights Reserved

File Name: arq_rx_blocks_proc.c

Change Activity:

Date    	Description of Change        	By
---------------------------------------------------------------
01-Oct-2008 	     Created		   Umamaheshwari C Devi

---------------------------------------------------------------
*/
//#define DDEBUG
//#define TRACE_LEVEL 4

#include <unistd.h>
#include <syscall.h>
#include <sys/syscall.h>
#include <assert.h>
#include <pthread.h>
#include "arq_const.h"
#include "arq_conn.h"
#include "arq_ds.h"
#include "arq_types.h"
#include "arq_defines.h"
#include "debug.h"
#include "flog.h"
#include "mutex_macros.h"

bin_heap_node_t *insert_ul_timer (ul_connection_t *, timer_type_t, int, struct timespec, struct timeval, bin_heap_node_t **);
extern int timespec_cmp (void *, void *);
//extern timer_list_t ul_timers;

op_status_t notify_received_blocks (short cid, int bsn_start, int bsn_end)
{
	ul_connection_t *conn;
	int thrd_idx;


	conn = find_ul_connection(cid);
	thrd_idx = conn->timer_thrd_idx;

	assert (conn != NULL);

	pthread_mutex_lock(&(conn->ul_block_buffer.arq_wnd_lock));

	FLOG_DEBUG("In notify_received_blocks: CID: %d bsn_start: %d bsn_end: %d window start: %d highest bsn: %d\n", 
											cid, bsn_start, bsn_end, conn->arq_rx_window_start, conn->arq_rx_highest_bsn);

	// Verify that bsn_start is within the sliding window
	if (bsn_start >= ARQ_BSN_MODULUS || mod((bsn_start-conn->arq_rx_window_start), ARQ_BSN_MODULUS) >= conn->ARQ_RX_WINDOW_SIZE) {
		pthread_mutex_unlock(&(conn->ul_block_buffer.arq_wnd_lock));
		return E_START_BSN_OUT_OF_RANGE;
	}

	int bsn_allowed_end = mod (conn->arq_rx_window_start+ (MIN( mod(bsn_end-conn->arq_rx_window_start, ARQ_BSN_MODULUS), 
								conn->ARQ_RX_WINDOW_SIZE-1)), ARQ_BSN_MODULUS);

	int blocks_allowed = mod (bsn_allowed_end-bsn_start+1, ARQ_BSN_MODULUS);
	FLOG_DEBUG("In notify_received_blocks: bsn_allowed_end: %d blocks_allowed: %d\n", bsn_allowed_end, blocks_allowed);

	assert (blocks_allowed <= conn->ARQ_RX_WINDOW_SIZE);
	int offset_to_highest_bsn = mod(conn->arq_rx_highest_bsn-conn->arq_rx_window_start, ARQ_BSN_MODULUS);
	int offset_to_bsn_allowed_end = mod(bsn_allowed_end-conn->arq_rx_window_start, ARQ_BSN_MODULUS);
	boolean update_rx_highest_bsn = false;

	if (offset_to_bsn_allowed_end+1 > offset_to_highest_bsn) {
		FLOG_DEBUG("In notify_received_blocks: Will update highest bsn; current offset: %d new offset: %d\n", offset_to_highest_bsn,
																						offset_to_bsn_allowed_end+1);
		update_rx_highest_bsn = true;
	}

	// Add to FEEDBACK list

	struct timeval tv;
	gettimeofday(&tv, NULL);
	boolean insert_purge_timers = true;

	if (bsn_start == conn->arq_rx_window_start) {
		//pthread_mutex_lock(&(conn->ul_block_buffer.arq_wnd_lock));
		conn->arq_rx_window_start = mod (((conn->arq_rx_window_start)+blocks_allowed), ARQ_BSN_MODULUS);
		if (true == update_rx_highest_bsn) {
			conn->arq_rx_highest_bsn = (bsn_allowed_end+1)%ARQ_BSN_MODULUS;
			FLOG_DEBUG("In notify_received_blocks: ARQ_RX_HIGHEST_BSN advanced to %d for connection %d\n",
								conn->arq_rx_highest_bsn, conn->cid);
		}
		//conn->ul_block_buffer.arq_rx_wnd_start_idx = (conn->ul_block_buffer.arq_rx_wnd_start_idx+blocks_allowed)%conn->ARQ_RX_WINDOW_SIZE;
		//pthread_mutex_unlock(&(conn->ul_block_buffer.arq_wnd_lock));

		FLOG_DEBUG("In notify_received_blocks: ARQ_RX_WINDOW_START advanced to %d for connection %d\n",
								conn->arq_rx_window_start, conn->cid);
		// Reset Sync LOSS timer
		if (conn->arq_sync_loss_timer != NULL) {
			FLOG_DEBUG("About to free SYNC LOSS TIMER for CONN ID: %d\n", conn->cid);
			//delete_node_delta_list (ul_timers.timer_list, &(conn->arq_sync_loss_timer));
			heap_delete (timespec_cmp, arq_ul_timers[thrd_idx].timer_list,&(conn->arq_sync_loss_timer));
			/*
			struct timeval zero;
			zero.tv_sec = zero.tv_usec = 0;
			delay_ret_val = delay_timer (arq_ul_timers[thrd_idx].timer_list, arq_ul_timers[thrd_idx].timeout_timer, conn->cid, conn->arq_sync_loss_timeout, zero, conn->arq_sync_loss_timer, thrd_idx);
			*/
		}
		conn->arq_sync_loss_timer = NULL;
		// Add new sync loss timer
#ifndef TEST_DL_SYNC_LOSS
#ifndef TEST_UL_SYNC_LOSS
		insert_ul_timer (conn, ARQ_SYNC_LOSS_TIMEOUT, 0, conn->arq_sync_loss_timeout, tv, &(conn->arq_sync_loss_timer));
#endif
#endif
			//TRACE2(4, "New SYNC LOSS TIMER added to CONN ID: %d at st: %p\n", conn->cid, conn->arq_sync_loss_timer);

		FLOG_DEBUG("In notify_received_blocks: ARQ_SYNC_LOSS timer updated\n");
		insert_purge_timers = false;
	} 

	// Set PURGE TIMERS for the received blocks
	int bl_no;

	//int bsn_start_offset = mod((bsn_start-conn->arq_rx_window_start), ARQ_BSN_MODULUS);

	for (bl_no = 0; bl_no < blocks_allowed; bl_no++) {
		// Add purge timeout timer
		//int blk_idx = (bl_no+bsn_start_offset+conn->ul_block_buffer.arq_rx_wnd_start_idx)%conn->ARQ_RX_WINDOW_SIZE;
		int bl_bsn = (bsn_start+bl_no)%ARQ_BSN_MODULUS;
		int blk_idx = bl_bsn%conn->ARQ_RX_WINDOW_SIZE;
		if (true == insert_purge_timers) {
			if (true == update_rx_highest_bsn) {
				conn->arq_rx_highest_bsn = (bl_bsn+1)%ARQ_BSN_MODULUS;
				FLOG_DEBUG("In notify_received_blocks: ARQ_RX_HIGHEST_BSN advanced to %d for connection %d\n",
								conn->arq_rx_highest_bsn, conn->cid);
			}
			insert_ul_timer (conn, ARQ_PURGE_TIMEOUT, bl_bsn, conn->arq_purge_timeout, tv, 
							 &(conn->ul_block_buffer.ul_block_buffer[blk_idx].purge_timeout_timer));
			FLOG_DEBUG("In notify_received_blocks: PURGE timer inserted for %d blk %d  at pt: %p\n", conn->cid, bl_bsn,
												 conn->ul_block_buffer.ul_block_buffer[blk_idx].purge_timeout_timer);
			/*printf("In notify_received_blocks: PURGE timer inserted for %d blk %d  at pt: %p\n", conn->cid, bl_bsn,
												 conn->ul_block_buffer.ul_block_buffer[blk_idx].purge_timeout_timer);*/
			conn->ul_block_buffer.ul_block_buffer[blk_idx].state = ARQ_BLK_RECEIVED;
			conn->ul_block_buffer.ul_block_buffer[blk_idx].bsn = bl_bsn;
		} else { // If Window Start is advanced, cumulative ack will take care of acknowledging blocks received, so 
		         // it is not necessary to have the state as ARQ_BLK_RECEIVED
			conn->ul_block_buffer.ul_block_buffer[blk_idx].state = ARQ_BLK_NOT_RECEIVED;
			conn->ul_block_buffer.ul_block_buffer[blk_idx].bsn = bl_bsn;
		}
	}
	pthread_mutex_unlock (&(conn->ul_block_buffer.arq_wnd_lock));
	conn->is_reset = false;
	return SUCCESS;
}

bin_heap_node_t *insert_ul_timer (ul_connection_t *conn, timer_type_t tt, int bsn, struct timespec delta, struct timeval curr_time, bin_heap_node_t **new_node)
{
	bin_heap_node_t *timer_node = NULL;
	timer_entry_t *bl_timer;
	struct timeval new_head_time;
	struct timespec curr_time_ts, fireat;
	int thrd_idx = conn->timer_thrd_idx;

	TIMEVAL_TO_TIMESPEC(&curr_time, &curr_time_ts);
	timeradd_ts(&curr_time_ts, &delta, &fireat);

	switch (tt) {
		case ARQ_PURGE_TIMEOUT:
			bl_timer = &(conn->ul_block_buffer.ul_block_buffer[bsn%conn->ARQ_RX_WINDOW_SIZE].purge_timeout_at);
			break;
		case ARQ_RESET_TIMEOUT:
		case ARQ_SYNC_LOSS_TIMEOUT:
			bl_timer = &(conn->sync_loss_timeout_at);
			break;
		case ARQ_ACK_GEN:
			bl_timer = &(conn->ack_gen_at);
			break;
		default:
			assert(0);
	}

	bl_timer->bsn = bsn;
	bl_timer->timer_type = tt;
	bl_timer->fireat = fireat;
	bl_timer->cid = conn->cid;

	timer_entry_t *timer_entry = NULL;
        void * ptimer_entry = (void *)timer_entry;
	heap_peek_value (arq_ul_timers[thrd_idx].timer_list, (void **)(&ptimer_entry));
        timer_entry = (timer_entry_t *)ptimer_entry;

	TIMESPEC_TO_TIMEVAL(&new_head_time, &fireat);
	struct timeval old_head_time;
	if (timer_entry) {
		TIMESPEC_TO_TIMEVAL(&old_head_time, &(timer_entry->fireat));
	} else {
		timerclear(&old_head_time);
	}

	timer_node = heap_insert_value (timespec_cmp, arq_ul_timers[thrd_idx].timer_list, (void *)bl_timer, new_node);
	assert (timer_node != NULL);
	TIMESPEC_TO_TIMEVAL(&new_head_time, &fireat);
	if (timercmp(&new_head_time, &old_head_time, <) || !((timerisset(&(old_head_time))))) {
		FLOG_DEBUG("insert_ul_timer: CONN: %d BSN: %d Timer head time changed to %ld sec %ld usec\n", 
										conn->cid, bsn, new_head_time.tv_sec, new_head_time.tv_usec);
		struct itimerspec timerspec;
		set_itimer_spec (timerspec, new_head_time);
		syscall (SYS_timer_settime, arq_ul_timers[thrd_idx].timeout_timer, TIMER_ABSTIME, &timerspec, NULL);
	}
	return timer_node;
}
