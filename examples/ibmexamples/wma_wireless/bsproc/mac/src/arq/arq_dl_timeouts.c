/*-------------------------------------------------------------------
IBM Confidential

IBM Wireless M2M Platform

(C)Copyright IBM Corp. 2008,2009,2010,2011

All Rights Reserved

File Name: arq_dl_timeouts.c

Change Activity:

Date    	Description of Change        	By
---------------------------------------------------------------
01-Oct-2008 	     Created		   Umamaheshwari C Devi

---------------------------------------------------------------
*/
//#define DDEBUG
//#define TRACE_LEVEL 0

#include <unistd.h>
#include <stdlib.h>
#include <syscall.h>
#include <sys/syscall.h>
#include <signal.h>
#include <time.h>
#include <sys/time.h>
#include <assert.h>
#include "arq_ds.h"
#include "arq_defines.h"
#include "arq_conn.h"
#include "arq_const.h"
#include "arq_ifaces.h"
#include "ll.h"
#include "debug.h"
#include "binheap.h"
#include "mac_sdu_queue.h"
#include "mac_connection.h"
#include "memmgmt.h"
#include "log.h"
#include "flog.h"
#include "mutex_macros.h"

void handle_dl_timeout(const timer_entry_t *);
extern bin_heap_node_t *insert_dl_timer (dl_connection_t *, timer_type_t, int, struct timespec, struct timeval, bin_heap_node_t **);
extern void insert_dummy_feedback_timer (dl_connection_t *, timer_type_t, const int, const struct timeval, const int, bin_heap_node_t **, double);
extern int timespec_cmp(void*, void*);
extern op_status_t delay_timer (bin_heap_t *timer_list, timer_t, unsigned short cid, struct timespec delta, struct timeval curr_time, bin_heap_node_t *node, int);

extern sdu_queue *dl_sdu_queue;
extern long num_allocated;
extern int sync_flag;
//extern long num_freed_per_thread[];
struct drand48_data rnd_state[MAX_DL_TIMER_THREADS];

void SIGALRM_handler(int dummy)
{
	//TRACE(4, "Dummy SIGALRM handler...\n");
	// Actual processing is done following a return from
	// nanosleep. nanosleep returns if either the sleep
	// duration elapses or the call is interrupted

	// This is in order to make the handler short and 
	// avoid the use of mutexes from the handler
}


void *arq_process_dl_timeouts(void *arg)
{
#define MAX_LAXITY 1000  // Should not exceed 1000000

	struct timespec req;	
	struct sigevent timer_event_spec;
	int idx;

#ifdef INTEGRATION_TEST
        long long int thrd_idx  = (long long int)arg;
#else
        int thrd_idx = (int)arg;
#endif

	FLOG_INFO("arq_process_dl_timeouts: thread index: %d\n", thrd_idx);

	create_init_trace_buffer(16384, "arq_process_dl_timeouts");

	timer_event_spec.sigev_value.sival_int = 0;
	timer_event_spec.sigev_value.sival_ptr = NULL;
	for (idx = 0; idx < __SIGEV_PAD_SIZE; idx++) {
		timer_event_spec._sigev_un._pad[idx] = 0;
	}
	timer_event_spec._sigev_un._sigev_thread._function = 0;
	timer_event_spec._sigev_un._sigev_thread._attribute = 0;

	FLOG_DEBUG("Creating timer processing thread\n");


	timer_event_spec.sigev_signo = SIGALRM;
	timer_event_spec.sigev_notify = SIGEV_THREAD_ID;
	timer_event_spec._sigev_un._tid = syscall(SYS_gettid);;

	syscall (SYS_timer_create, CLOCK_REALTIME, &timer_event_spec, &(arq_dl_timers[thrd_idx].timeout_timer));

	signal(SIGALRM, SIGALRM_handler);

	srand48_r(time(NULL), &rnd_state[thrd_idx]);

	req.tv_sec = 600;
	req.tv_nsec = 0;

	do {
		struct timeval curr_time, diff_time;
		struct timezone curr_timezone;

		FLOG_DEBUG("DL Timers thread: go to sleep...\n");

		int istimerset;
		struct timeval htime;
		timer_entry_t *head_timer_entry = NULL;
                void * phead_timer_entry = (void *)head_timer_entry;
		heap_peek_value (arq_dl_timers[thrd_idx].timer_list, (void **)&phead_timer_entry);
                head_timer_entry = (timer_entry_t *)phead_timer_entry;
		if (head_timer_entry) {
			TIMESPEC_TO_TIMEVAL(&htime, &(head_timer_entry->fireat));
		} else {
			timerclear(&htime);
		}

		istimerset = timerisset(&htime);

		if (istimerset) {
			gettimeofday (&curr_time, &curr_timezone);
			if (timercmp(&curr_time, &htime, <)) {
				timersub(&htime, &curr_time, &diff_time);
				req.tv_sec = diff_time.tv_sec; req.tv_nsec = diff_time.tv_usec*1000L;
				//printf("Sleeping for %ld sec and %ld usec\n", diff_time.tv_sec, diff_time.tv_usec);
				//printf("To be woken up at %ld sec %ld usec\n", htime.tv_sec, htime.tv_usec);
				if (req.tv_sec > 0 || req.tv_nsec > MAX_LAXITY) {
					nanosleep(&req, NULL);
				}
			}
		} else {
		    // No timer is pending
			req.tv_sec = 600;
			req.tv_nsec = 0;
			nanosleep(&req, NULL);
		}
		if (sync_flag == 0) pthread_exit(NULL);
		heap_peek_value (arq_dl_timers[thrd_idx].timer_list, (void **)&phead_timer_entry);
                head_timer_entry = (timer_entry_t *)phead_timer_entry;
		if (head_timer_entry) {
			TIMESPEC_TO_TIMEVAL(&htime, &(head_timer_entry->fireat));
		} else {
			timerclear(&htime);
		}
		// Get current time
		gettimeofday (&curr_time, &curr_timezone);

		if (timercmp(&curr_time, &htime, <)) {
			timersub(&htime, &curr_time, &diff_time);
			if (diff_time.tv_sec > 0 || diff_time.tv_usec > 100 /*MAX_LAXITY*/) {
				// If no timer is scheduled for expiry, get back to sleeping
				FLOG_DEBUG("Sleep interrupted for DL timer thread\n");
				continue;
			}
		}

		FLOG_DEBUG("handle_dl_timeout: Some DL timer fired\n");
		//printf("handle_dl_timeout: Some DL timer for %ld sec %ld usec fired at %ld sec %ld usec\n", curr_time.tv_sec, 
		//																	curr_time.tv_usec, htime.tv_sec, htime.tv_usec);
		
		// Dequeue the first entry

		//struct timespec delta, cum_delta;
		timer_entry_t *timer_entry = NULL;
                void * ptimer_entry = (void *)timer_entry;
		op_status_t ret_stat;

		ret_stat = heap_extract_top_value (timespec_cmp, arq_dl_timers[thrd_idx].timer_list, (void **)&ptimer_entry);
                timer_entry = (timer_entry_t *)ptimer_entry;
		/*
		TRACE4(4, "fireat: %d sec %d nsec, dl timers head time: %d sec %d usec\n", 
					timer_entry->fireat.tv_sec, timer_entry->fireat.tv_nsec, arq_dl_timers.head_time.tv_sec, arq_dl_timers.head_time.tv_usec);
		assert (timer_entry->fireat.tv_sec == arq_dl_timers.head_time.tv_sec && 
			timer_entry->fireat.tv_nsec-arq_dl_timers.head_time.tv_usec*1000 < 1000);
			*/

		if (ret_stat == SUCCESS) {
			handle_dl_timeout (timer_entry);
			do {
				do {
					if (sync_flag ==0) pthread_exit(NULL);
					timer_entry_t ref_time;
					gettimeofday (&curr_time, &curr_timezone);

					TIMEVAL_TO_TIMESPEC(&curr_time, &(ref_time.fireat));
					ret_stat = heap_extract_top_value_cond (timespec_cmp, arq_dl_timers[thrd_idx].timer_list, &ref_time, (void **)&ptimer_entry);
                                        timer_entry = (timer_entry_t *)ptimer_entry;
					FLOG_DEBUG("Firing time: %ld sec %ld nsec\n", timer_entry->fireat.tv_sec, timer_entry->fireat.tv_nsec);
					FLOG_DEBUG("Current time: %ld sec %ld usec\n", curr_time.tv_sec, curr_time.tv_usec);
					if (ret_stat != SUCCESS) {
						if (ret_stat == E_COND_NOT_MET) {
					//printf("Firing time: %d sec %d nsec  ", timer_entry->fireat.tv_sec, timer_entry->fireat.tv_nsec);
					//printf("Current time: %ld sec %ld usec\n", curr_time.tv_sec, curr_time.tv_usec);
							FLOG_DEBUG( "1: No more eligible timers in DL Delta List\n"); 
						} else {
							FLOG_DEBUG("1: DL Delta List empty\n"); 
						}
						break;
					}
					handle_dl_timeout (timer_entry);
				} while(1);

				/*
				if (ret_stat == E_Q_EMPTY) {
					break;
				}
				*/
				if (ret_stat != SUCCESS) {
					break;
				}

	/*
	static int call_count_2[] = {0, 0, 0, 0};
	static double mean_2[] = {0, 0, 0, 0};


	unsigned long long f_begin = readtsc();
	*/
				/**************************
				gettimeofday (&curr_time, &curr_timezone);
				struct timeval head_time_tmp;

				head_time_tmp.tv_sec = 0; head_time_tmp.tv_usec = 100;//100*MAX_DL_TIMER_THREADS;
				timersub(&(arq_dl_timers[thrd_idx].head_time), &head_time_tmp, &head_time_tmp);
				***************************/
				/*
				TRACE2(4, "DL timers head time: %d sec %d usec\n", arq_dl_timers.head_time.tv_sec, arq_dl_timers.head_time.tv_usec);
				TRACE2(4, "Current time: %d sec %d usec\n", curr_time.tv_sec, curr_time.tv_usec);
				TRACE2(4, "DL timers head time tmp: %d sec %d usec\n", head_time_tmp.tv_sec, head_time_tmp.tv_usec);
				*/

				/********************************
				if (timercmp(&head_time_tmp, &curr_time, >)) {
					static int arm_ct[] = {0, 0, 0, 0};
					// Arm the timer and go to sleep
					TRACE(4, "DL Timer thread: Arming the timer...\n");
					if (arm_ct[thrd_idx] % 1000 == 0) {
					printf("DL Timer thread: Arming the timer...\n");
					}
					arm_ct[thrd_idx]++;
					struct itimerspec timerspec;
					timerspec.it_interval.tv_nsec = timerspec.it_interval.tv_sec = 0;
					timerspec.it_value.tv_nsec = arq_dl_timers[thrd_idx].head_time.tv_usec*1000; timerspec.it_value.tv_sec = arq_dl_timers[thrd_idx].head_time.tv_sec;
					//TIMEVAL_TO_TIMESPEC(&(timerspec.it_value), &(conn->timer_list.head_time));
					syscall (SYS_timer_settime, arq_dl_timers[thrd_idx].timeout_timer, TIMER_ABSTIME, &timerspec, NULL);
					break;
				}
				**********************************/
	/*
	unsigned long long f_end = readtsc();
	call_count_2[thrd_idx]++;
	mean_2[thrd_idx] = (double)(call_count_2[thrd_idx]-1)*mean_2[thrd_idx]/call_count_2[thrd_idx]+(double)(f_end-f_begin)/call_count_2[thrd_idx];
	if (call_count_2[thrd_idx] % 10000 == 0) {
		printf("%d %lld %lld %lld %f\n", thrd_idx, f_begin, f_end, f_end-f_begin, mean_2[thrd_idx]);
	}
	*/
			} while (1);
		} else {
			if (ret_stat == E_COND_NOT_MET) {
				FLOG_DEBUG("2: No more eligible timers in DL Delta List\n");
			} else if (ret_stat == E_Q_EMPTY) {
				FLOG_DEBUG("2: DL Delta List empty\n");
			}
		}
	} while(sync_flag == 1);	
	FLOG_INFO("Terminated ARQ DL timeouts thread\n");
	dl_timer_thread_done = 1;
	pthread_exit (NULL);
}


void handle_dl_timeout(const timer_entry_t *p_te)
{
	if (sync_flag == 0) pthread_exit(NULL);
	int bl_offset_from_start;
	int bl_buff_idx;
	struct timeval tv;
	dl_connection_t *conn;


	conn = find_dl_connection (p_te->cid);
	int thrd_idx = conn->timer_thrd_idx;

	if (NULL == conn) {
		return;
	}


	if (p_te->timer_type == ARQ_SYNC_LOSS_TIMEOUT) {
		FLOG_DEBUG("handle_dl_timeout : CONN: %d Setting arq_sync_loss_timer %p to NULL\n", conn->cid, conn->arq_sync_loss_timer);
		FLOG_DEBUG("handle_dl_timeout : CONN: %d Setting arq_sync_loss_timer %p to NULL\n", conn->cid, conn->arq_sync_loss_timer);
		FLOG_WARNING("DL_SYNC_LOSS TIMEOUT: ");
		conn->arq_sync_loss_timer = NULL;
		if (conn->arq_tx_window_start != conn->arq_tx_next_bsn) { // Send RESET message only if at least one block is outstanding
			// Send ARQ Reset message
			ARQ_reset_message *reset_msg;

			FLOG_DEBUG( "handle_dl_timeout: DL SYNC LOSS TIMEOUT timer fired\n");

			//reset_msg = (ARQ_reset_message *)WiMAX_mac_calloc (1, sizeof(ARQ_reset_message));
			reset_msg = (ARQ_reset_message *)mac_sdu_malloc(sizeof(ARQ_reset_message), MMM_CLASS_TYPE);
			reset_msg->mgmt_msg_type = ARQ_RESET_MSG;
			reset_msg->cid = conn->cid;
			reset_msg->type = 0x00;
			reset_msg->direction = 0x0;
			// Post the message to the MAC management message queue
			int basic_cid = conn->cid-1024;
			int ret_val;

			ret_val = get_basic_cid(conn->cid, &basic_cid);

			if (0 == ret_val) {
				FLOG_DEBUG(" Enqueueing Reset Message of Type 0 For User %d\n",basic_cid);
				enqueue_transport_sdu_queue(dl_sdu_queue, basic_cid, sizeof(ARQ_reset_message), reset_msg);
			}
			else
			{
				FLOG_DEBUG("Connection not found for resetting\n");
			}
			conn->arq_sync_loss_timer = NULL;
			struct timeval htime;
			TIMESPEC_TO_TIMEVAL(&htime, &(p_te->fireat));
			insert_dl_timer (conn, ARQ_RESET_TIMEOUT, 0, ARQ_RESET_TIMEOUT_DEF, htime, &(conn->arq_sync_loss_timer));
			FLOG_DEBUG("Inserted Reset timer\n");
		}
		else {FLOG_WARNING("At window. No reset msg\n");}
		return;
	}

	pthread_mutex_lock(&(conn->block_buffer.arq_wnd_lock));
	bl_offset_from_start = mod((p_te->bsn-conn->arq_tx_window_start), ARQ_BSN_MODULUS);
	bl_buff_idx = (conn->block_buffer.arq_tx_wnd_start_idx+bl_offset_from_start)%conn->ARQ_TX_WINDOW_SIZE;
	pthread_mutex_unlock(&(conn->block_buffer.arq_wnd_lock));

	FLOG_DEBUG("In handle_dl_timeout: CONN ID: %d BSN: %d Offset from Wnd Start : %d Idx : %d\n", conn->cid, p_te->bsn, bl_offset_from_start, bl_buff_idx);
	FLOG_DEBUG("In handle_dl_timeout: CONN ID: %d BSN: %d timer type: %d\n", conn->cid, p_te->bsn, p_te->timer_type);

	if (bl_offset_from_start >= conn->ARQ_TX_WINDOW_SIZE) {
		FLOG_DEBUG("In handle_dl_timeout: Timer fired for block outside sliding window: CONN ID: %d BSN: %d Timer Type: %d\n", conn->cid, p_te->bsn, p_te->timer_type);
		return;
	}
	assert (bl_offset_from_start < conn->ARQ_TX_WINDOW_SIZE);
	assert (bl_offset_from_start >= 0);
	assert (bl_buff_idx >= 0);

	gettimeofday (&tv, NULL);
	FLOG_DEBUG("In handle_dl_timeout: CONN ID: %d BSN: %d CURR TIME : %ld sec %ld usec\n", 
						conn->cid, p_te->bsn, tv.tv_sec, tv.tv_usec);
	//volatile block_t *bl = &(conn->block_buffer.block_buffer[bl_buff_idx]);
#define bl (&(conn->block_buffer.block_buffer[bl_buff_idx]))
	struct timeval htime;
	TIMESPEC_TO_TIMEVAL(&htime, &(p_te->fireat));

	switch (p_te->timer_type) {
		case ARQ_RETRY_TIMEOUT: {
			blocks_info_t bl_info;
			bl->retry_timeout_timer = NULL;
			//TIMESPEC_TO_TIMEVAL (&(arq_dl_timers[thrd_idx].head_time), &(bl->retry_timeout_at.fireat))
			TIMESPEC_TO_TIMEVAL (&htime, &(bl->retry_timeout_at.fireat))

			// Double check that the block buffer need not be locked
			if (bl->state != ARQ_DONE && bl->state != ARQ_DISCARDED) { 
				bl->retry_timeout_timer = NULL;
				bl->state = ARQ_WAITING_FOR_RETX;
			
				assert (bl->bsn == p_te->bsn);
				bl_info.start_bsn = p_te->bsn;
				bl_info.size = bl->size;
				bl_info.data = bl->data;
				bl_info.btype = bl->type;

				// Add block to retransmission queue
				FLOG_DEBUG("handle_dl_timeout: RETRY TIMEOUT timer fired; adding to retransmission queue. CONN ID: %d BSN: %d\n", conn->cid, p_te->bsn); 
#ifdef TESTING_RETRY_TIMEOUT
				FLOG_INFO("In handle_dl_timeout: RETRY TIMEOUT timer has expired; adding to retransmission queue. CONN ID: %d BSN: %d\n", conn->cid, p_te->bsn);
#endif
				//enqueue_unique_linked_list(conn->retrans_q, bl->bsn, (void *)&bl_info);
				ARQ_enqueue_ReTX_q_unique (conn, bl->bsn, &bl_info);

				// Add a retry timeout timer for the block
#ifdef TESTING_RETRY_TIMEOUT
				FLOG_INFO("In handle_dl_timeout: RETRY TIMEOUT Enqueuing another Retry timeout\n");
#endif
				insert_dl_timer (conn, ARQ_RETRY_TIMEOUT, bl->bsn, conn->arq_retry_timeout, htime, &(bl->retry_timeout_timer));
			}
			else {
				bl->retry_timeout_timer = NULL;
				FLOG_DEBUG( "handle_dl_timeout: Retry Timeout timer set to null for CONN ID: %d BSN: %d\n", conn->cid, bl->bsn);
			}

			// Insert ARQ_SYNC_LOSS_TIMEOUT timer if it is not already present for the connection
			if (conn->arq_sync_loss_timer == NULL) {
#ifndef TEST_DL_SYNC_LOSS				
				insert_dl_timer (conn, ARQ_SYNC_LOSS_TIMEOUT, 0, ARQ_SYNC_LOSS_TIMEOUT_DEF, htime, &(conn->arq_sync_loss_timer));
#endif
				assert (conn->arq_sync_loss_timer != NULL);
				FLOG_DEBUG("handle_dl_timeout: CONN: %d inserted sync loss timer %p\n", conn->cid, conn->arq_sync_loss_timer);
#ifdef TESTING_RETRY_TIMEOUT				
				FLOG_INFO("handle_dl_timeout: CONN: %d inserted sync loss timer %p\n", conn->cid, conn->arq_sync_loss_timer);
#endif
			}
		}
		break;
		case ARQ_RESET_TIMEOUT:
		//Assuming One retry for RESET
			conn->arq_sync_loss_timer = NULL;
			FLOG_DEBUG("Arq reset timed out\n");
			ARQ_reset_message *rst_msg = (ARQ_reset_message*)mac_sdu_malloc(sizeof(ARQ_reset_message),5);
			rst_msg->mgmt_msg_type = ARQ_RESET_MSG;
			rst_msg->cid = conn->cid; 
			rst_msg->type = 0x00;
			rst_msg->direction = 0x0;
			int basic_cid ;
			int ret_val = get_basic_cid(conn->cid, &basic_cid);
			if (ret_val == 0)
			{
			FLOG_WARNING("RESET_TIMEOUT: Retrying reset message once\n");
				enqueue_transport_sdu_queue(dl_sdu_queue,basic_cid,sizeof(ARQ_reset_message),rst_msg);
			}
		break;
		case ARQ_DISCARD_TIMEOUT:
		case ARQ_BLOCK_LIFETIME: {
			if (bl->state != ARQ_DONE) {
				bl->state = ARQ_DISCARDED;
				if (p_te->timer_type == ARQ_BLOCK_LIFETIME) {
					TIMESPEC_TO_TIMEVAL (&htime, &(bl->lifetime_timeout_at.fireat));
					conn->block_buffer.block_buffer[bl_buff_idx].block_lifetime_timer = NULL;
					bl->block_lifetime_timer = NULL;
					FLOG_DEBUG("handle_dl_timeout: Block Lifetime Timer set to null for CONN ID: %d BSN: %d lt: %p block address: %p\n", conn->cid, bl->bsn, bl->block_lifetime_timer, bl);
#ifdef TEST_BLOCK_LIFETIME_WITH_DISCARD
					FLOG_INFO("handle_dl_timeout: Block Lifetime Timer set to null for CONN ID: %d BSN: %d lt: %p block address: %p\n", conn->cid, bl->bsn, bl->block_lifetime_timer, bl);
#endif
				} else {
					assert (bl->block_lifetime_timer == NULL);
					TIMESPEC_TO_TIMEVAL (&htime, &(bl->retry_timeout_at.fireat));
					bl->retry_timeout_timer = NULL;
					FLOG_DEBUG("handle_dl_timeout: Retry Timeout timer set to null for CONN ID: %d BSN: %d\n", conn->cid, bl->bsn);
				}

				// Remove the block from waiting for retransmission queue (if previously added)
				//unlink_and_discard_element_ll (conn->retrans_q, bl->bsn);
				ARQ_delete_ReTX_q (conn, bl->bsn, bl->size);

				// If timer is BLOCK_LIFETIME, then reset retry timeout timer
				if (p_te->timer_type == ARQ_BLOCK_LIFETIME && bl->retry_timeout_timer != NULL) {
					heap_delete (timespec_cmp, arq_dl_timers[thrd_idx].timer_list, &(bl->retry_timeout_timer));
					bl->retry_timeout_timer = NULL;
					FLOG_DEBUG("handle_dl_timeout: Retry timeout timer deleted and set to null for CONN ID: %d BSN: %d\n", conn->cid, bl->bsn);
					
#ifdef TEST_BLOCK_LIFETIME_WITH_DISCARD
					FLOG_INFO("handle_dl_timeout: Retry timeout timer deleted and set to null for CONN ID: %d BSN: %d\n", conn->cid, bl->bsn);
#endif
				}

				FLOG_DEBUG("handle_dl_timeout: DISCARD TIMEOUT/BLOCK LIFETIME timer fired; CONN ID: %d BSN: %d\n", conn->cid, p_te->bsn); 

				// Generate ARQ_Discard message
				ARQ_discard_message *dis_msg;

				//dis_msg = (ARQ_discard_message *)WiMAX_mac_calloc (1, sizeof(ARQ_discard_message));
				dis_msg = (ARQ_discard_message *)mac_sdu_malloc(sizeof(ARQ_discard_message), MMM_CLASS_TYPE);
				dis_msg->mgmt_msg_type = ARQ_DISCARD_MSG;
				dis_msg->cid = conn->cid;
				dis_msg->bsn = p_te->bsn;

				// Add the discard message to the MAC management message queue
				// Find the basic CID for this connection first
				int basic_cid = conn->cid-1024;
				int ret_val;

				ret_val = get_basic_cid(conn->cid, &basic_cid);

#ifdef TEST_BLOCK_LIFETIME_WITH_DISCARD
				FLOG_INFO("Block Lifetime expired. So sending DISCARD Message for BSN %d on Connection %d\n",p_te->bsn,conn->cid);
#endif
				if (0 == ret_val) {
					enqueue_transport_sdu_queue(dl_sdu_queue, basic_cid, sizeof(ARQ_discard_message), dis_msg);
				}
			
				if (p_te->timer_type == ARQ_BLOCK_LIFETIME) {
					// Delete the block feedback timer
					/*if (bl->dummy_feedback_timer != NULL) {
						heap_delete (timespec_cmp, arq_dl_timers[thrd_idx].timer_list, &(bl->dummy_feedback_timer));
						bl->dummy_feedback_timer = NULL;
					}*/
					double rval, addl_delay;
					// Insert a dummy feedback timer for the block
					drand48_r(&rnd_state[thrd_idx], &rval);
					drand48_r(&rnd_state[thrd_idx], &addl_delay);
					//insert_dummy_feedback_timer (conn, ARQ_DUMMY_DISCARD_FEEDBACK, bl->bsn, htime, num_retries, &(bl->dummy_feedback_timer), addl_delay);
				}

				// Insert a discard timeout timer for the block
				insert_dl_timer (conn, ARQ_DISCARD_TIMEOUT, bl->bsn, conn->arq_retry_timeout, htime, &(bl->retry_timeout_timer));

				} else {
					bl->retry_timeout_timer = NULL;
					bl->block_lifetime_timer = NULL;
					FLOG_DEBUG("handle_dl_timeout: Block Lifetime and Retry Timeout timers set to null for CONN ID: %d BSN: %d\n", conn->cid, bl->bsn);
				}
			}
			break;


		case ARQ_DUMMY_DISCARD_FEEDBACK:
		case ARQ_DUMMY_FEEDBACK: {
			TIMESPEC_TO_TIMEVAL (&htime, &(bl->feedback_at.fireat))
			if (bl->state != ARQ_DONE) {
				int bl_no;
				// Change the status of the block to DONE
				// Ensure that the block buffer need not be locked
				bl->state = ARQ_DONE;

				FLOG_DEBUG("handle_dl_timeout: DUMMY feedback timer fired CONN ID: %d BSN: %d\n", conn->cid, bl->bsn); 

				// Reset retry timeout and block lifetime timers
				//assert (bl->retry_timeout_timer != NULL);
				if (bl->retry_timeout_timer != NULL) {
					heap_delete (timespec_cmp, arq_dl_timers[thrd_idx].timer_list, &(bl->retry_timeout_timer));
					bl->retry_timeout_timer = NULL;
					FLOG_DEBUG("handle_dl_timeout: Retry Timeout timer deleted and set to null for CONN ID: %d BSN: %d\n", conn->cid, bl->bsn);
				}

				if (p_te->timer_type == ARQ_DUMMY_FEEDBACK) {
					//assert (bl->block_lifetime_timer != NULL);
					if (bl->block_lifetime_timer != NULL) {
						heap_delete (timespec_cmp, arq_dl_timers[thrd_idx].timer_list, &(bl->block_lifetime_timer));
						bl->block_lifetime_timer = NULL;
						FLOG_DEBUG("handle_dl_timeout: Block Lifetime timer deleted and set to null for CONN ID: %d BSN: %d\n", conn->cid, bl->bsn);
					}
				}

				// Remove the block from waiting for retransmission queue (if previously added)
				//unlink_and_discard_element_ll (conn->retrans_q, bl->bsn);
				ARQ_delete_ReTX_q (conn, bl->bsn, bl->size);
	


				// Adjust ARQ_TX_WINDOW_START
				//int no_of_blocks = (conn->arq_tx_next_bsn-conn->arq_tx_window_start)%ARQ_BSN_MODULUS;
				int no_of_blocks = mod((conn->arq_tx_next_bsn-conn->arq_tx_window_start), ARQ_BSN_MODULUS);
				FLOG_DEBUG("handle_dl_timeout: CONN ID: %d No. of outstanding blocks: %d\n", conn->cid, no_of_blocks);

				for (bl_no = 0; bl_no < no_of_blocks; bl_no++) {
					int blk_idx = (bl_no+conn->block_buffer.arq_tx_wnd_start_idx)%conn->ARQ_TX_WINDOW_SIZE;
					if (conn->block_buffer.block_buffer[blk_idx].state != ARQ_DONE) {
						break;
					} 
					conn->block_buffer.block_buffer[blk_idx].state = ARQ_NOT_SENT;
					FLOG_DEBUG( "handle_dl_timeout: CONN ID: %d bl_no: %d bl_idx: %d\n", conn->cid, bl_no, blk_idx);
					FLOG_DEBUG("handle_dl_timeout: CONN ID: %d bl_no: %d rt: %p lt: %p data address: %p size: %d\n", conn->cid, bl_no, 
								conn->block_buffer.block_buffer[blk_idx].retry_timeout_timer,
								conn->block_buffer.block_buffer[blk_idx].block_lifetime_timer,
								conn->block_buffer.block_buffer[blk_idx].data,
								conn->block_buffer.block_buffer[blk_idx].size);
					mac_sdu_free (conn->block_buffer.block_buffer[blk_idx].data,
								  conn->block_buffer.block_buffer[blk_idx].size, 
								  conn->block_buffer.block_buffer[blk_idx].type);
					//num_freed_per_thread[thrd_idx]++;
					conn->block_buffer.block_buffer[blk_idx].data = NULL;
					assert(conn->block_buffer.block_buffer[blk_idx].retry_timeout_timer == NULL);
					assert(conn->block_buffer.block_buffer[blk_idx].block_lifetime_timer == NULL);
				}

				if (bl_no > 0) {
					//conn->arq_tx_window_start = ((conn->arq_tx_window_start)+bl_no)%ARQ_BSN_MODULUS;
					pthread_mutex_lock(&(conn->block_buffer.arq_wnd_lock));
					conn->arq_tx_window_start = mod (((conn->arq_tx_window_start)+bl_no), ARQ_BSN_MODULUS);
					conn->block_buffer.arq_tx_wnd_start_idx = (conn->block_buffer.arq_tx_wnd_start_idx+bl_no)%conn->ARQ_TX_WINDOW_SIZE;
					pthread_mutex_unlock(&(conn->block_buffer.arq_wnd_lock));
					FLOG_DEBUG("handle_dl_timeout: CONN ID: %d ARQ_TX_WINDOW_START advanced to %d\n", conn->cid, conn->arq_tx_window_start);
					// Update ARQ_SYNC_LOSS_TIMEOUT
					op_status_t delay_ret_val = SUCCESS;
					// delete the current timer
					if (conn->arq_sync_loss_timer != NULL) {
						FLOG_DEBUG("handle_dl_timeout: CONN: %d updating sync loss timer value %p\n", conn->cid, conn->arq_sync_loss_timer);
						// Change the timestamp for the sync_loss_timer
						delay_ret_val = delay_timer (arq_dl_timers[thrd_idx].timer_list, arq_dl_timers[thrd_idx].timeout_timer, conn->cid, conn->arq_sync_loss_timeout, htime, conn->arq_sync_loss_timer, thrd_idx);
						assert(conn->arq_sync_loss_timer != NULL);
						//heap_delete (timespec_cmp, arq_dl_timers.timer_list, &(conn->arq_sync_loss_timer));
						//conn->arq_sync_loss_timer = NULL;
					} 
					if (NULL == conn->arq_sync_loss_timer /*|| SUCCESS != delay_ret_val*/) {
						// Add new sync loss timer
						insert_dl_timer (conn, ARQ_SYNC_LOSS_TIMEOUT, 0, ARQ_SYNC_LOSS_TIMEOUT_DEF, htime, &(conn->arq_sync_loss_timer));
						FLOG_DEBUG("handle_dl_timeout: CONN: %d inserted sync loss timer %p\n", conn->cid, conn->arq_sync_loss_timer);
					}
				}
			}	
		}
		break;

	default:
		break;
	}
}
