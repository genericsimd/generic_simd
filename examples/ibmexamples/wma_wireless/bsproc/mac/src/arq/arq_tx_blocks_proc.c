/*-------------------------------------------------------------------
IBM Confidential

IBM Wireless M2M Platform

(C)Copyright IBM Corp. 2009,2010,2011

All Rights Reserved

File Name: arq_tx_blocks_proc.c

Change Activity:

Date    	Description of Change        	By
---------------------------------------------------------------
01-Oct-2008 	     Created		   Umamaheshwari C Devi

---------------------------------------------------------------
*/
//#define DDEBUG
//#define TRACE_LEVEL 4

#include <stdlib.h>
#include <pthread.h>
#include <assert.h>
#include <time.h>
#include <unistd.h>
#include <syscall.h>
#include <math.h>
#include "arq_ds.h"
#include "circq_array.h"
#include "util.h"
#include "arq_threads.h"
#include "arq_conn.h"
#include "arq_const.h"
#include "ll.h"
#include "binheap.h"
#include "arq_defines.h"
#include "debug.h"
#include "log.h"
#include "flog.h"

#define ONE_BILLION 1000000000L
#define ONE_MILLION 1000000L
#define MIN_FEEDBACK_TIME 5000000L

fragment_type find_block_type (fragment_type, unsigned short, unsigned short);

bin_heap_node_t *insert_dl_timer (dl_connection_t *, timer_type_t, int, struct timespec, struct timeval, bin_heap_node_t **);
void insert_dummy_feedback_timer (dl_connection_t *, timer_type_t, const int, const struct timeval, const int, bin_heap_node_t **, double);

extern int timespec_cmp(void *, void *);
extern int copy_timer_entry(void *, void *);
extern int sync_flag;

void *tx_blocks_dequeue_thread (void *thread_conns)
{
	thread_conns_map_t *conns_by_thread;
	int i;
	boolean all_qs_empty;
	pid_t my_tid;
	int num_retries[MAX_CONNS_PER_THREAD];  // Number of retries for an ARQ block; computed according to block loss probability.
										    // Should be the same for all blocks of an SDU if fragmentation is disabled.
	char desc[64];
	struct drand48_data tx_rstate;

	my_tid = syscall(SYS_gettid);
	FLOG_DEBUG("New Tx blocks dequeue thread started; thread id: %d\n", my_tid);
	assert (thread_conns != NULL);
	sprintf(desc, "tx_blocks_dequeue_thread with id %d\n", my_tid);
	create_init_trace_buffer(8192, desc);

	//pthread_mutex_init (&dl_timer_lock, NULL);

	srand48_r(time(NULL), &tx_rstate);

	//cpu_set_t cpuset;

	//CPU_ZERO(&cpuset);

	//pthread_getaffinity_np (dl_threads.conns_by_thread[0].thread, sizeof(cpuset), &cpuset);
	//sched_getaffinity (my_tid, sizeof(cpuset), &cpuset);
	//for (int cpu = 0; cpu < 4 /*CPU_SETSIZE*/; cpu++) {
		//if (CPU_ISSET(cpu, &cpuset)) {
			//printf("Thread %d tx_blocks proc has affinity to CPU %d\n", my_tid, cpu);
		//}
	//}

	conns_by_thread = (thread_conns_map_t *)thread_conns;

	// Initialize num_retries for all connections to 0
	for (i = 0; i < conns_by_thread->max_valid_conn_idx; i++) {
		num_retries[i] = 0;
		//mean[i] = 0;
		//count[i] = 0;
		//prev_tsc[i] = 0;
	}

	while(sync_flag == 1) {
		pthread_mutex_lock(&(conns_by_thread->thread_mutex));
		all_qs_empty = true;
		for (i = 0; i < conns_by_thread->max_valid_conn_idx; i++) {
			if (conns_by_thread->conn_list.dl_conns[i] != NULL) {
				circq_array_t *tx_blocks_q = conns_by_thread->conn_list.dl_conns[i]->tx_blocks_q;
				assert (tx_blocks_q != NULL);
				if (is_empty_circq_array(tx_blocks_q) == false) {
					FLOG_DEBUG("tx_blocks_dequeue_thread: %d: found a non-empty tx_blocks Q\n", my_tid);
					all_qs_empty = false;
					break;
				}
			}
		}
		if (sync_flag == 0) break;
		if (all_qs_empty == true) {
			FLOG_DEBUG("tx_blocks_dequeue_thread: %d: All tx_blocks queues empty, blocking...\n", my_tid);
			//pthread_mutex_lock(&(conns_by_thread->thread_mutex));
			pthread_cond_wait (&(conns_by_thread->thread_cond), &(conns_by_thread->thread_mutex));
			//pthread_mutex_unlock(&(conns_by_thread->thread_mutex));
			FLOG_DEBUG("tx_blocks_dequeue_thread: %d: Signaled...\n", my_tid);
		}
		if (sync_flag == 0) break;
		pthread_mutex_unlock(&(conns_by_thread->thread_mutex));

		struct timeval curr_time;
		
		for (i = 0; i < conns_by_thread->max_valid_conn_idx; i++) {
			dl_connection_t *conn = conns_by_thread->conn_list.dl_conns[i];
			if (conn != NULL) {
				blocks_info_t tx_block;
				op_status_t ret_val;
				circq_array_t *tx_blocks_q = conn->tx_blocks_q;

				if (is_empty_circq_array(tx_blocks_q) == true) {
					FLOG_DEBUG("tx_blocks_dequeue_thread: %d Empty queue for conn %d\n", my_tid, conn->cid);
					continue;
				}
				do {
					int num_blocks;
					int j;

					ret_val = dequeue_circq_array(tx_blocks_q, (void *)&tx_block, false, false);	

					if (ret_val != SUCCESS) {
						break;
					}

					gettimeofday(&curr_time, NULL);
					FLOG_DEBUG("tx_blocks_dequeue_thread: %d CURR TIME sec --  %ld usec -- %ld ns\n", my_tid, curr_time.tv_sec, curr_time.tv_usec);
					//printf("tx_blocks_dequeue_thread: %d CONN: %d CURR TIME sec --  %ld usec -- %ld ns\n", my_tid, conn->cid, curr_time.tv_sec, curr_time.tv_usec);

					// Process transmitted blocks
					num_blocks = tx_block.size/conn->ARQ_BLOCK_SIZE;
					if (num_blocks*conn->ARQ_BLOCK_SIZE != tx_block.size) {
						num_blocks++;
					}
					FLOG_DEBUG("tx_blocks_dequeue_thread: %d: Dequeued blocks_info: num_blocks: %d start_bsn: %d size: %d\n", my_tid, num_blocks, tx_block.start_bsn, tx_block.size);
					//printf("tx_blocks_dequeue_thread: %d: Dequeued blocks_info: num_blocks: %d start_bsn: %d size: %d\n", my_tid, num_blocks, tx_block.start_bsn, tx_block.size);
					assert (tx_block.size > 0);
					assert (tx_block.start_bsn >= 0 && tx_block.start_bsn < ARQ_BSN_MODULUS);
					for (j = 0; j < num_blocks; j++) {
						int bsn;
						int bsn_offset_from_start, bsn_idx;
						//volatile block_t *block;

						bsn = (tx_block.start_bsn+j)%ARQ_BSN_MODULUS;

						//bsn_offset_from_start = (bsn-conn->arq_tx_window_start)%ARQ_BSN_MODULUS;
						int wnd_st;
						pthread_mutex_lock(&(conn->block_buffer.arq_wnd_lock));
						bsn_offset_from_start = mod((bsn-conn->arq_tx_window_start), ARQ_BSN_MODULUS);
						wnd_st = conn->arq_tx_window_start;
						bsn_idx = (conn->block_buffer.arq_tx_wnd_start_idx+bsn_offset_from_start)%conn->ARQ_TX_WINDOW_SIZE;
						pthread_mutex_unlock(&(conn->block_buffer.arq_wnd_lock));
						if (bsn_offset_from_start > 100) {
						FLOG_DEBUG( "tx_blocks_dequeue_thread: CONN: %d BSN: %d ARQ_TX_WINDOW_START: %d OFFSET from WND_START: %d\n", conn->cid, wnd_st, bsn, bsn_offset_from_start);
						}
						FLOG_DEBUG("tx_blocks_dequeue_thread: arq_tx_window_start: %d\n", wnd_st);

						//printf("For bsn is %d bsn offset from start %d , window start is %d window size %d\n", bsn, bsn_offset_from_start, conn->arq_tx_window_start,conn->ARQ_TX_WINDOW_SIZE);
						//assert(bsn_offset_from_start < conn->ARQ_TX_WINDOW_SIZE);

						//block = &(conn->block_buffer.block_buffer[bsn_idx]);
						#define block (&(conn->block_buffer.block_buffer[bsn_idx]))
						block->bsn = bsn;
						block->data = tx_block.data+j*conn->ARQ_BLOCK_SIZE;
						if (j <= (tx_block.size/conn->ARQ_BLOCK_SIZE)-1) { 
							block->size = conn->ARQ_BLOCK_SIZE;
						} else {
							block->size = tx_block.size%conn->ARQ_BLOCK_SIZE;
						}
						block->state = ARQ_OUTSTANDING;
						block->type = find_block_type (tx_block.btype, j, num_blocks);
						FLOG_DEBUG( "tx_blocks_dequeue_thread: %d BSN: %d source type: %d block_type: %d\n", my_tid, bsn, tx_block.btype, block->type);

						
						FLOG_DEBUG("tx_blocks_dequeue_thread: %d Existing Retry Timeout Timer: %p\n", my_tid, block->retry_timeout_timer);
						//assert (block->retry_timeout_timer == NULL);
						if (block->retry_timeout_timer == NULL) insert_dl_timer (conn, ARQ_RETRY_TIMEOUT, bsn, conn->arq_retry_timeout, curr_time, &(block->retry_timeout_timer));
						FLOG_DEBUG("tx_blocks_dequeue_thread: %d BSN: %d ARQ_RETRY_TIMEOUTset at offset sec --  %ld nsec -- %ld ns\n", 
										my_tid, bsn, conn->arq_retry_timeout.tv_sec, conn->arq_retry_timeout.tv_nsec);
						FLOG_DEBUG("tx_blocks_dequeue_thread: %d Retry Timeout Timer: %p\n", my_tid, block->retry_timeout_timer);
						//assert (block->retry_timeout_timer != NULL);
							

						FLOG_DEBUG("tx_blocks_dequeue_thread: %d Existing Block Lifetime Timer: %p\n", my_tid, block->block_lifetime_timer);
						//assert (block->block_lifetime_timer == NULL);
						if (block->block_lifetime_timer == NULL) insert_dl_timer (conn, ARQ_BLOCK_LIFETIME, bsn, conn->arq_block_lifetime, curr_time, &(block->block_lifetime_timer));
						FLOG_DEBUG("tx_blocks_dequeue_thread: %d BSN: %d ARQ_BLOCK_LIFETIME set at offset sec --  %ld nsec -- %ld ns\n", 
										my_tid, bsn, conn->arq_block_lifetime.tv_sec, conn->arq_block_lifetime.tv_nsec);
						FLOG_DEBUG("tx_blocks_dequeue_thread: %d Block Lifetime Timer: %p\n", my_tid, block->block_lifetime_timer);
						//assert (block->block_lifetime_timer != NULL);

						// Part of ARQ driver -- will be eventually discarded
						if (true == conn->is_frag_enabled || NO_FRAGMENTATION == block->type || FIRST_FRAGMENT == block->type) {
							if (conn->block_loss_prob < 10e-9) {
								num_retries[i] = 0;
							} else {
								double rval;
								drand48_r(&tx_rstate, &rval);
								num_retries[i] = log(1.0-rval)/log(conn->block_loss_prob);
							}
						} 
						FLOG_DEBUG("tx_blocks_dequeue_thread: CONN ID %d BSN: %d Num Retries: %d\n", conn->cid, bsn, num_retries[i]);
						double rval;
						drand48_r(&tx_rstate, &rval);
						//insert_dummy_feedback_timer (conn, ARQ_DUMMY_FEEDBACK, bsn, curr_time, num_retries[i], &(block->dummy_feedback_timer), rval);
						
					}
					if (num_blocks > 0) {
						conn->arq_tx_next_bsn = (tx_block.start_bsn+num_blocks)%ARQ_BSN_MODULUS;
					}
				} while (1);
			}
		}
	}
	FLOG_INFO("Terminated Transmit Blocks Dequeue Thread\n");
	pthread_exit(NULL);
	return NULL;
}


bin_heap_node_t *insert_dl_timer (dl_connection_t *conn, timer_type_t tt, int bsn, struct timespec delta, struct timeval curr_time, bin_heap_node_t **new_node)
{
	//pthread_mutex_lock (&dl_timer_lock);
	bin_heap_node_t *timer_node = NULL;
	timer_entry_t *bl_timer;
	struct timeval new_head_time;
	struct timespec curr_time_ts, fireat;
	int thrd_idx = conn->timer_thrd_idx;


	TIMEVAL_TO_TIMESPEC(&curr_time, &curr_time_ts);
	timeradd_ts(&curr_time_ts, &delta, &fireat);

	switch (tt) {
		case ARQ_RETRY_TIMEOUT:
		case ARQ_DISCARD_TIMEOUT:
			bl_timer = &(conn->block_buffer.block_buffer[bsn%conn->ARQ_TX_WINDOW_SIZE].retry_timeout_at);
			break;
		case ARQ_BLOCK_LIFETIME:
			bl_timer = &(conn->block_buffer.block_buffer[bsn%conn->ARQ_TX_WINDOW_SIZE].lifetime_timeout_at);
			break;
		case ARQ_DUMMY_FEEDBACK:
		case ARQ_DUMMY_DISCARD_FEEDBACK:
			bl_timer = &(conn->block_buffer.block_buffer[bsn%conn->ARQ_TX_WINDOW_SIZE].feedback_at);
			break;
		case ARQ_RESET_TIMEOUT:
		case ARQ_SYNC_LOSS_TIMEOUT:
			bl_timer = &(conn->sync_loss_timeout_at);
			break;
		default:
			assert(0);
			break;
	}

	bl_timer->bsn = bsn;
	bl_timer->timer_type = tt;
	bl_timer->fireat = fireat;
	bl_timer->cid = conn->cid;

	FLOG_DEBUG("insert_dl_timer: inserting timer type %d for CONN: %d BSN: %d to fire at %ld sec %ld nsec\n", 
								tt, conn->cid, bsn, fireat.tv_sec, fireat.tv_nsec);
	/**/
//	printf("insert_dl_timer: thrd_id: %d inserting timer type %d for CONN: %d BSN: %d to fire at %ld sec %ld nsec\n", thrd_idx, tt, conn->cid, bsn, fireat.tv_sec, fireat.tv_nsec);
	/**/
	timer_entry_t *timer_entry = NULL;
        void * ptimer_entry = (void *)timer_entry;

	heap_peek_value (arq_dl_timers[thrd_idx].timer_list, (void **)(&ptimer_entry));
        timer_entry = (timer_entry_t *)ptimer_entry;
	timer_node = heap_insert_value (timespec_cmp, arq_dl_timers[thrd_idx].timer_list, (void *)bl_timer, new_node);
	assert (timer_node != NULL);
	TIMESPEC_TO_TIMEVAL(&new_head_time, &fireat);
	struct timeval old_head_time;
	if (timer_entry) {
		TIMESPEC_TO_TIMEVAL(&old_head_time, &(timer_entry->fireat));
	} else {
		timerclear(&old_head_time);
	}

	if (timercmp(&new_head_time, &old_head_time, <) /*|| timercmp(&new_head_time, &old_head_time, =)*/ || !timerisset(&old_head_time)) { 
		FLOG_DEBUG("tx_blocks_dequeue_thread: CONN: %d BSN: %d Timer head time changed to %ld sec %ld usec\n", conn->cid, bsn, new_head_time.tv_sec, new_head_time.tv_usec);
		//printf("tx_blocks_dequeue_thread: CONN: %d BSN: %d Timer head time changed to %ld sec %ld usec from %ld sec %ld usec\n", conn->cid, bsn, new_head_time.tv_sec, new_head_time.tv_usec, old_head_time.tv_sec, old_head_time.tv_usec);
		struct itimerspec timerspec;
		set_itimer_spec (timerspec, new_head_time);
		syscall (SYS_timer_settime, arq_dl_timers[thrd_idx].timeout_timer, TIMER_ABSTIME, &timerspec, NULL);
	}
	return timer_node;
}


op_status_t delay_timer (bin_heap_t *timer_list, timer_t wakeup_timer, unsigned short cid, struct timespec delta, struct timeval curr_time, bin_heap_node_t *node, int thrd_idx)
{
	timer_entry_t *bl_timer;
	timer_entry_t new_timer;
	struct timeval new_head_time, firetime;
	struct timespec curr_time_ts, fireat;
	op_status_t ret_val = SUCCESS;

	bl_timer = (timer_entry_t *)(node->value);

	if (timerisset(&curr_time)) {
		TIMEVAL_TO_TIMESPEC(&curr_time, &curr_time_ts);
		timeradd_ts(&curr_time_ts, &delta, &fireat);
	} else {
		timeradd_ts(&(bl_timer->fireat), &delta, &fireat);
	}

	FLOG_DEBUG("delay_dl_timer: delaying timer for CONN: %d BSN: %d from %ld sec %ld nsec to %ld sec %ld nsec\n", 
							cid, bl_timer->bsn, bl_timer->fireat.tv_sec, bl_timer->fireat.tv_nsec, fireat.tv_sec, fireat.tv_nsec);


	//Increase only if the new value is really higher
	if (timercmp_ts(&fireat, &(bl_timer->fireat), >)) {
		new_timer = *bl_timer;
		new_timer.fireat = fireat;

		timer_entry_t *past_timer = NULL;
                void * ppast_timer = (void *)past_timer;
		ret_val = heap_peek_value (timer_list, (void **)&ppast_timer);
                past_timer = (timer_entry_t *)ppast_timer;
		ret_val = heap_increase (timespec_cmp, copy_timer_entry, timer_list, node, &new_timer);
		if (SUCCESS == ret_val) {
			timer_entry_t *next_timer = NULL;
                        void * pnext_timer = (void *)next_timer;
			ret_val = heap_peek_value (timer_list, (void **)&pnext_timer);
                        next_timer = (timer_entry_t *)pnext_timer;
			if (SUCCESS == ret_val) {
				TIMESPEC_TO_TIMEVAL(&new_head_time, &(next_timer->fireat));
				TIMESPEC_TO_TIMEVAL(&firetime, &fireat);
				assert (!(timercmp(&new_head_time, &firetime, >)));
				if (timercmp(&new_head_time, &(firetime), =)) { // || !((timerisset(&(arq_dl_timers[thrd_idx].head_time))))) 
					FLOG_DEBUG("delay_dl_timer: CONN: %d Timer head time changed to %ld sec %ld usec\n", 
											cid, new_head_time.tv_sec, new_head_time.tv_usec);
					//printf("delay_dl_timer: CONN: %d Timer head time changed to %ld sec %ld usec from %ld sec %ld usec\n", 
											//cid, new_head_time.tv_sec, new_head_time.tv_usec, past_timer->fireat.tv_sec, past_timer->fireat.tv_nsec/1000);
					struct itimerspec timerspec;
					set_itimer_spec (timerspec, new_head_time);
					//arq_dl_timers[thrd_idx].head_time = new_head_time;
					syscall (SYS_timer_settime, wakeup_timer, TIMER_ABSTIME, &timerspec, NULL);
				}
			}
		}
	} else {
		//assert (0);
	}
	return ret_val;
}

void insert_dummy_feedback_timer (dl_connection_t *conn, timer_type_t tt, const int bsn, const struct timeval curr_time, const int num_retries, bin_heap_node_t **timer_node, double addl_delay)
{

	// Part of ARQ driver -- will be eventually discarded

	FLOG_DEBUG("insert_dummy_feedback_timer:BSN: %d Num Retries: %d\n", bsn, num_retries);
	long long int  retry_timeout_val = conn->arq_retry_timeout.tv_sec*ONE_BILLION + conn->arq_retry_timeout.tv_nsec;
	long long int  blk_feedback_time = num_retries*retry_timeout_val + MIN_FEEDBACK_TIME +
											(long long int)((double)(addl_delay*(double)(conn->arq_retry_timeout.tv_sec*ONE_BILLION + 
																						conn->arq_retry_timeout.tv_nsec - MIN_FEEDBACK_TIME)));
	FLOG_DEBUG("insert_dummy_feedback_timer: BSN: %d feed_back_time %lld ns\n",  bsn, blk_feedback_time);

	struct timespec blk_feedback_ts;
	blk_feedback_ts.tv_sec = (long int) (blk_feedback_time/ONE_BILLION);
	blk_feedback_ts.tv_nsec = (long int) (blk_feedback_time%ONE_BILLION);
	insert_dl_timer (conn, tt, bsn, blk_feedback_ts, curr_time, timer_node);
	FLOG_DEBUG( "insert_dummy_feedback_timer: BSN : %d DUMMY ARQ FEEDBACK set at offset sec --  %ld nsec -- %ld ns\n", 
					bsn, blk_feedback_ts.tv_sec, blk_feedback_ts.tv_nsec);
}

fragment_type find_block_type (fragment_type src_type, unsigned short curr_block, unsigned short total_blocks)
{
	switch (src_type) {
		case	NO_FRAGMENTATION:
		case	FIRST_FRAGMENT:
			if (curr_block == 0) {
				if (total_blocks == 1) {
					return src_type;
				} else if (total_blocks > 1) {
					return FIRST_FRAGMENT;
				}
			} else if (curr_block == total_blocks-1) {
				return LAST_FRAGMENT;
			} else if (curr_block < total_blocks-1) {
				return CONTINUING_FRAGMENT;
			} else {
				assert (0);
			}
			break;
		case	LAST_FRAGMENT:
			if (curr_block < total_blocks-1) {
				return CONTINUING_FRAGMENT;
			} else if (curr_block == total_blocks-1) {
				return LAST_FRAGMENT;
			} else {
				assert (0);
			}
			break;
		case	CONTINUING_FRAGMENT:
			return CONTINUING_FRAGMENT;
		default:
			assert(0);
	}
	return 101; // Should be replaced by a type which says UNKNOWN_TYPE
}
