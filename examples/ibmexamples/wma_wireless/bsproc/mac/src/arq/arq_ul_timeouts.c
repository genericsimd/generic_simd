/*-------------------------------------------------------------------
IBM Confidential

IBM Wireless M2M Platform

(C)Copyright IBM Corp. 2009,2010,2011

All Rights Reserved

File Name: arq_ul_timeouts.c

Change Activity:

Date    	Description of Change        	By
---------------------------------------------------------------
01-Oct-2008 	     Created		   Umamaheshwari C Devi

---------------------------------------------------------------
*/
#include <unistd.h>
#include <syscall.h>
#include <sys/syscall.h>
#include <signal.h>
#include <time.h>
#include <sys/time.h>
#include <assert.h>
#include <string.h>
#include "arq_ds.h"
#include "arq_defines.h"
#include "arq_conn.h"
#include "arq_const.h"
#include "ll.h"
#include "debug.h"
#include "binheap.h"
#include "mac_sdu_queue.h"
#include "mac_connection.h"
#include "memmgmt.h"
#include "log.h"
#include "flog.h"

void handle_ul_timeout(const timer_entry_t *);
extern bin_heap_node_t *insert_ul_timer (ul_connection_t *, timer_type_t, int, struct timespec, struct timeval, bin_heap_node_t **);
void construct_selective_ack_map (ul_connection_t *, short, short, short *, short *, short *);
void construct_block_sequence_ack_map (ul_connection_t *, short, short, short *, short *, short *);
extern op_status_t delay_timer (bin_heap_t *timer_list, timer_t, unsigned short cid, struct timespec delta, struct timeval curr_time, bin_heap_node_t *node);

extern int timespec_cmp(void *, void *);
extern sdu_queue *dl_sdu_queue;
extern int sync_flag;
//extern timer_list_t ul_timers;    // List of all UL timers of all connections

void SIGALRM_UL_handler(int dummy)
{
	//TRACE(4, "Dummy UL SIGALRM handler...\n");
	// Actual processing is done following a return from
	// nanosleep. nanosleep returns if either the sleep
	// duration elapses or the call is interrupted

	// This is in order to make the handler short and 
	// avoid the use of mutexes from the handler
}


void *arq_process_ul_timeouts(void *arg)
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
	create_init_trace_buffer(8192, "arq_process_ul_timeouts");

	timer_event_spec.sigev_value.sival_int = 0;
	timer_event_spec.sigev_value.sival_ptr = NULL;
	for (idx = 0; idx < __SIGEV_PAD_SIZE; idx++) {
		timer_event_spec._sigev_un._pad[idx] = 0;
	}
	timer_event_spec._sigev_un._sigev_thread._function = 0;
	timer_event_spec._sigev_un._sigev_thread._attribute = 0;

	FLOG_DEBUG("Creating timer processing thread %ld\n", syscall(SYS_gettid));

	timer_event_spec.sigev_signo = SIGALRM;
	timer_event_spec.sigev_notify = SIGEV_THREAD_ID;
	timer_event_spec._sigev_un._tid = syscall(SYS_gettid);;

	syscall (SYS_timer_create, CLOCK_REALTIME, &timer_event_spec, &(arq_ul_timers[thrd_idx].timeout_timer));

	signal(SIGALRM, SIGALRM_UL_handler);

	req.tv_sec = 600;
	req.tv_nsec = 0;

	do {
		struct timeval curr_time, diff_time;
		struct timezone curr_timezone;

		int istimerset;
		struct timeval htime;
		timer_entry_t *head_timer_entry = NULL;
                void * phead_timer_entry = (void *)head_timer_entry;
		heap_peek_value (arq_ul_timers[thrd_idx].timer_list, (void **)&phead_timer_entry);
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
		heap_peek_value (arq_ul_timers[thrd_idx].timer_list, (void **)&phead_timer_entry);
                head_timer_entry = (timer_entry_t *)phead_timer_entry;
		if (head_timer_entry) {
			TIMESPEC_TO_TIMEVAL(&htime, &(head_timer_entry->fireat));
		} else {
			timerclear(&htime);
		}
		gettimeofday (&curr_time, &curr_timezone);
		FLOG_DEBUG("TIMERS: handle_ul_timeout: timer fired at Current Time: %ld sec %ld usec\n", curr_time.tv_sec, curr_time.tv_usec);

		if (timercmp(&curr_time, &htime, <)) {
			timersub(&htime, &curr_time, &diff_time);
			if (diff_time.tv_sec == 0 && diff_time.tv_usec > MAX_LAXITY) {
				// If no timer is scheduled for expiry, get back to sleeping
				FLOG_DEBUG("Sleep interrupted for UL\n");
				continue;
			}
		}


		FLOG_DEBUG("handle_ul_timeout: Some timer fired for UL\n");
		
		// Dequeue the first entry

		timer_entry_t *timer_entry = NULL;
                void * ptimer_entry = (void *)timer_entry;
		op_status_t ret_stat;

		ret_stat = heap_extract_top_value (timespec_cmp, arq_ul_timers[thrd_idx].timer_list, (void **)&ptimer_entry);
                timer_entry = (timer_entry_t *)ptimer_entry;

		if (ret_stat == SUCCESS) {
			handle_ul_timeout (timer_entry);
			do {
				do {
					timer_entry_t ref_time;
					gettimeofday (&curr_time, &curr_timezone);

					TIMEVAL_TO_TIMESPEC(&curr_time, &(ref_time.fireat));
					ret_stat = heap_extract_top_value_cond (timespec_cmp, arq_ul_timers[thrd_idx].timer_list, &ref_time, (void **)&ptimer_entry);
                                        timer_entry = (timer_entry_t *)ptimer_entry;
					if (ret_stat != SUCCESS) {
						if (ret_stat == E_COND_NOT_MET) {
							//TIMESPEC_TO_TIMEVAL(&(arq_ul_timers[thrd_idx].head_time), &(timer_entry->fireat));
							FLOG_DEBUG("No more eligible timers in UL Delta List\n");
						} else {
							//timerclear(&(ul_timers.head_time));
							FLOG_DEBUG("UL Delta List empty\n");
						}
						break;
					}
					handle_ul_timeout (timer_entry);
				} while(1);

				/*
				if (ret_stat == E_Q_EMPTY) {
					break;
				}
				*/
				if (ret_stat != SUCCESS) {
					break;
				}
			} while (1);
		} else {
			if (ret_stat == E_COND_NOT_MET) {
				FLOG_DEBUG("No more eligible timers in UL Delta List \n");
			} else if (ret_stat == E_Q_EMPTY) {
				FLOG_DEBUG("UL Delta List empty\n");
				//timerclear(&(ul_timers.head_time));
			}
		}
	} while(sync_flag == 1);	

	//timer_initialized = true;
	FLOG_INFO("Terminated ARQ UL Timeouts Thread\n");
	ul_timer_thread_done = 1;
	pthread_exit (NULL);
}


void handle_ul_timeout(const timer_entry_t *p_te)
{
	if (sync_flag == 0) pthread_exit(NULL);
	int bl_buff_idx;
	struct timeval tv;
	short cid;
	ul_connection_t *conn;
	int thrd_idx;
	struct timeval htime;

	cid = p_te->cid;
	conn = find_ul_connection (cid);
	thrd_idx = conn->timer_thrd_idx;
	if (conn == NULL) {
		return;
	}

	FLOG_DEBUG("handle_ul_timeout: CONN ID: %d timer code: %d\n", cid, p_te->timer_type);
	TIMESPEC_TO_TIMEVAL(&htime, &(p_te->fireat));

	if (p_te->timer_type == ARQ_SYNC_LOSS_TIMEOUT) {
#ifdef TEST_UL_SYNC_LOSS
		FLOG_WARNING("SYNC LOST IN UL TIMEOUT.");
#endif
		conn->arq_sync_loss_timer = NULL;
		TIMESPEC_TO_TIMEVAL (&(htime), &(conn->sync_loss_timeout_at.fireat));
		if (conn->arq_rx_window_start == conn->arq_rx_highest_bsn) { // Send RESET message only if there is no scope for window start 
																	 // to advance without receiving a new PDU
			// Send ARQ Reset message
			ARQ_reset_message *reset_msg;

			FLOG_DEBUG("handle_ul_timeout: UL SYNC LOSS TIMEOUT timer fired\n");

			reset_msg = (ARQ_reset_message *)mac_sdu_malloc ( sizeof(ARQ_reset_message),5);
			reset_msg->mgmt_msg_type = ARQ_RESET_MSG;
			reset_msg->cid = conn->cid;
			reset_msg->type = 0x00;
			reset_msg->direction = 0x1;
			// Post the message to the MAC management message queue
			int basic_cid = conn->cid-1024;
			int ret_val;

			ret_val = get_basic_cid(conn->cid, &basic_cid);
			if (ret_val == 0){
				FLOG_DEBUG(" UL_SYNC_LOSS: Sending Reset message of type 0 for Basic CID %d\n",basic_cid);
				enqueue_transport_sdu_queue(dl_sdu_queue, basic_cid, sizeof(ARQ_reset_message), reset_msg);
			}
			conn->arq_sync_loss_timer = NULL;
			struct timeval htime;
			TIMESPEC_TO_TIMEVAL(&htime, &(p_te->fireat));
			FLOG_DEBUG("UL_SYNC_LOSS: Setting Reset timeout\n");
			insert_ul_timer(conn,ARQ_RESET_TIMEOUT,0,ARQ_RESET_TIMEOUT_DEF,htime,&(conn->arq_sync_loss_timer));
		}
		return;
	} 
	else if (p_te->timer_type == ARQ_RESET_TIMEOUT){
			FLOG_DEBUG("RESET_TIMEOUT: Retrying send once\n");
			//Assuming one retry for Reset
			ARQ_reset_message *rst_msg = (ARQ_reset_message*)mac_sdu_malloc(sizeof(ARQ_reset_message),5);
			rst_msg->mgmt_msg_type = ARQ_RESET_MSG;
			rst_msg->cid = conn->cid;
			rst_msg->direction = 0x1;
			rst_msg->type = 0x00;
			int basic_cid;
			int ret_val = get_basic_cid(conn->cid, &basic_cid);
			if (ret_val == 0)
			{
				enqueue_transport_sdu_queue(dl_sdu_queue,basic_cid, sizeof(ARQ_reset_message),rst_msg);
			}
		

	}
	else if (p_te->timer_type == ARQ_PURGE_TIMEOUT) {

		assert (p_te->timer_type == ARQ_PURGE_TIMEOUT);

		FLOG_DEBUG("PURGE timer fired for CONN ID: %d\n", conn->cid);


		/*
		bl_buff_idx = (conn->ul_block_buffer.arq_rx_wnd_start_idx+bl_offset_from_start)%conn->ARQ_RX_WINDOW_SIZE;
		pthread_mutex_unlock(&(conn->ul_block_buffer.arq_wnd_lock));
		*/

		if (true == conn->is_reset) {
			return;
		}
#ifdef TESTING_PURGE_TIMEOUT
		FLOG_INFO( "PURGE timer fired for CONN ID: %d. BSN is %d. Win start is %d\n", conn->cid,p_te->bsn,conn->arq_rx_window_start);
#endif
		pthread_mutex_lock(&(conn->ul_block_buffer.arq_wnd_lock));

		bl_buff_idx = p_te->bsn%conn->ARQ_RX_WINDOW_SIZE;

		ul_block_t *bl = &(conn->ul_block_buffer.ul_block_buffer[bl_buff_idx]);

		TIMESPEC_TO_TIMEVAL(&(htime), &(bl->purge_timeout_at.fireat));

		bl->purge_timeout_timer = NULL;

		int bl_offset_from_start = mod((p_te->bsn-conn->arq_rx_window_start), ARQ_BSN_MODULUS);
		int bl_offset_from_highest_bsn = mod((conn->arq_rx_highest_bsn-p_te->bsn-1), ARQ_BSN_MODULUS);

		FLOG_DEBUG( "In handle_ul_timeout: CONN ID: %d BSN: %d Offset from Wnd Start : %d Idx : %d\n", conn->cid, p_te->bsn, bl_offset_from_start, bl_buff_idx);
		FLOG_DEBUG("In handle_ul_timeout: CONN ID: %d BSN: %d timer type: %d Wnd Start: %d highest bsn: %d\n", 
												conn->cid, p_te->bsn, p_te->timer_type, conn->arq_rx_window_start, conn->arq_rx_highest_bsn);

		if (bl_offset_from_start+1 >= conn->ARQ_RX_WINDOW_SIZE || bl_offset_from_highest_bsn+1 >= conn->ARQ_RX_WINDOW_SIZE) {
			FLOG_DEBUG("In handle_ul_timeout: Timer fired for block outside sliding window: CONN ID: %d BSN: %d Timer Type: %d\n", 
														conn->cid, p_te->bsn, p_te->timer_type);
			pthread_mutex_unlock(&(conn->ul_block_buffer.arq_wnd_lock));
			return;
		}

		assert (bl_offset_from_start < conn->ARQ_RX_WINDOW_SIZE);
		assert (bl_offset_from_start >= 0);
		assert (bl_buff_idx >= 0);

		gettimeofday (&tv, NULL);
		FLOG_DEBUG("In handle_ul_timeout: CONN ID: %d BSN: %d CURR TIME : %ld sec %ld usec\n", conn->cid, p_te->bsn, tv.tv_sec, tv.tv_usec);

		int wnd_start = conn->arq_rx_window_start;
		int no_of_blocks = mod((p_te->bsn-wnd_start), ARQ_BSN_MODULUS); // No. of blocks whose purge timers need to be checked

		FLOG_DEBUG("handle_ul_timeout: CONN ID: %d No. of blocks until block whose timer purged: %d\n", conn->cid, no_of_blocks);

		int bl_no;
		for (bl_no = 0; bl_no < no_of_blocks; bl_no++) {
			//int blk_idx = (bl_no+conn->ul_block_buffer.arq_rx_wnd_start_idx)%conn->ARQ_RX_WINDOW_SIZE;
			int blk_idx = (wnd_start+bl_no)%conn->ARQ_RX_WINDOW_SIZE;

			FLOG_DEBUG("handle_ul_timeout: CONN ID: %d bl_no: %d bl_idx: %d\n", conn->cid, bl_no, blk_idx);
			FLOG_DEBUG("handle_ul_timeout: CONN ID: %d bl_no: %d pt: %p block address: %p\n", conn->cid, bl_no, 
						conn->ul_block_buffer.ul_block_buffer[blk_idx].purge_timeout_timer,
						&(conn->ul_block_buffer.ul_block_buffer[blk_idx]));
			if (conn->ul_block_buffer.ul_block_buffer[blk_idx].purge_timeout_timer != NULL) {
				heap_delete (timespec_cmp, arq_ul_timers[thrd_idx].timer_list, &(conn->ul_block_buffer.ul_block_buffer[blk_idx].purge_timeout_timer));
			}
			conn->ul_block_buffer.ul_block_buffer[blk_idx].purge_timeout_timer = NULL;
			conn->ul_block_buffer.ul_block_buffer[blk_idx].state = ARQ_BLK_RECEIVED;
		}

		// Add blocks to FEEDBACK list

		bl->state = ARQ_BLK_RECEIVED;

		FLOG_DEBUG("handle_ul_timeout: CONN: %d ARQ_RX_WINDOW_START currently at %d\n", conn->cid, conn->arq_rx_window_start);
		//pthread_mutex_lock(&(conn->ul_block_buffer.arq_wnd_lock));
		conn->arq_rx_window_start = mod (((conn->arq_rx_window_start)+no_of_blocks+1), ARQ_BSN_MODULUS);
#ifdef TESTTING_PURGE_TIMEOUT
		FLOG_INFO("In Purge Timeout: Window updated to %d\n",conn->arq_rx_window_start);
#endif
		//conn->ul_block_buffer.arq_rx_wnd_start_idx = (conn->ul_block_buffer.arq_rx_wnd_start_idx+no_of_blocks+1)%conn->ARQ_RX_WINDOW_SIZE;
		pthread_mutex_unlock(&(conn->ul_block_buffer.arq_wnd_lock));
		FLOG_DEBUG("handle_ul_timeout: CONN: %d ARQ_RX_WINDOW_START advanced to %d\n", conn->cid, conn->arq_rx_window_start);
		// Update ARQ_SYNC_LOSS_TIMEOUT
		// delete the current timer
		op_status_t delay_ret_val;
		if (conn->arq_sync_loss_timer != NULL) {
			FLOG_DEBUG("About to free SYNC LOSS TIMER for CONN ID: %d\n", conn->cid);
			//heap_delete (timespec_cmp, ul_timers.timer_list, &(conn->arq_sync_loss_timer));
			delay_ret_val = delay_timer (arq_ul_timers[thrd_idx].timer_list, arq_ul_timers[thrd_idx].timeout_timer, conn->cid, conn->arq_sync_loss_timeout, htime, conn->arq_sync_loss_timer);
		}
		if (NULL == conn->arq_sync_loss_timer || SUCCESS != delay_ret_val) {
			// Add new sync loss timer
			insert_ul_timer (conn, ARQ_SYNC_LOSS_TIMEOUT, 0, conn->arq_sync_loss_timeout, htime, &(conn->arq_sync_loss_timer));
			FLOG_DEBUG("New SYNC LOSS TIMER added to CONN ID: %d at st: %p\n", conn->cid, conn->arq_sync_loss_timer);
		}
	} else if (ARQ_ACK_GEN == p_te->timer_type) {

		// Insert the next ARQ_ACK_GEN timer
		gettimeofday(&tv, NULL);
		insert_ul_timer(conn, ARQ_ACK_GEN, 0, ARQ_ACK_INTERVAL, htime, &(conn->arq_ack_gen_timer));
		FLOG_DEBUG("handle_ul_timeout: CONN ID: %d ARQ_ACK_GEN timer inserted at %p\n", conn->cid, conn->arq_ack_gen_timer);
		TIMESPEC_TO_TIMEVAL(&(htime), &(conn->ack_gen_at.fireat));

		short start_bsn;
		short cum_start_bsn;
		short ack_type;
		short rx_window_start, rx_highest_bsn; // Take a snap-shot and work with it to avoid race conditions

		pthread_mutex_lock(&(conn->ul_block_buffer.arq_wnd_lock));
		rx_window_start = conn->arq_rx_window_start;
		rx_highest_bsn = conn->arq_rx_highest_bsn;
		pthread_mutex_unlock(&(conn->ul_block_buffer.arq_wnd_lock));

		FLOG_DEBUG("handle_ul_timeout: CONN ID: %d UL ARQ_ACK_GEN timer fired ARQ window start: %d ARQ highest bsn: %d\n", 
													conn->cid, rx_window_start, rx_highest_bsn);
	
		assert (mod((rx_highest_bsn-rx_window_start), ARQ_BSN_MODULUS) <= conn->ARQ_RX_WINDOW_SIZE);

		short wnd_start_minus1 = mod(rx_window_start-1, ARQ_BSN_MODULUS); 

		// The second condition is to handle the initial case when no cumulative ACK has been sent
		if (conn->arq_last_cum_ack != wnd_start_minus1 && (conn->arq_last_cum_ack != ARQ_BSN_MODULUS || rx_window_start != 0)) {
			// Cumulative ACK case
			start_bsn = wnd_start_minus1;
			cum_start_bsn = wnd_start_minus1;
			FLOG_DEBUG("handle_ul_timeout: CONN ID: %d Some sort of cumulative ack with start BSN %d\n", conn->cid, start_bsn);
			if (rx_window_start == rx_highest_bsn) {
				// Pure cumulative ACK; no selective ACKs
				ack_type = ARQ_CUMULATIVE_ACK;
				conn->arq_last_cum_ack = start_bsn;
				FLOG_DEBUG("handle_ul_timeout: CONN ID: %d ack type is ARQ_CUMULATIVE_ACK\n", conn->cid);
			} else {
				// Cumulative and Selective
				ack_type = 4;  // Need to decide between types 2 and 3
				conn->arq_last_cum_ack = start_bsn;
				FLOG_DEBUG("handle_ul_timeout: CONN ID: %d ack type TO BE DECIDED\n", conn->cid);
			}
		} else {
			if (rx_window_start == rx_highest_bsn) {
				FLOG_DEBUG("handle_ul_timeout: CONN ID: %d Nothing to ACK, returning...\n", conn->cid);
				// Nothing new to ACK
				return;
			}
			// Selective ACKs only -- hence selective ack map needs to be constructed
			ack_type = ARQ_SELECTIVE_ACK;
			// Skip all NOT RECEIVED and previously ACKed blocks
			short bsn;
			for (bsn = rx_window_start; bsn != rx_highest_bsn; bsn = (bsn+1)%ARQ_BSN_MODULUS) {
				int blk_idx = bsn%conn->ARQ_RX_WINDOW_SIZE;
				if (conn->ul_block_buffer.ul_block_buffer[blk_idx].state == ARQ_BLK_RECEIVED) {
					break;
				}
			}
			start_bsn = bsn;
			FLOG_DEBUG("handle_ul_timeout: CONN ID: %d Start BSN for SELECTIVE ACK case: %d\n", conn->cid, start_bsn);
		}
		short num_possible_blks = mod((rx_highest_bsn-start_bsn), ARQ_BSN_MODULUS);

		ARQ_feedback_message *ack_msg;

		int max_feedback_ies = num_possible_blks/63+1; // 63 is the minimum number of blocks whose status can be communicated using
													 // four shorts

		if (0 == num_possible_blks) {
			FLOG_DEBUG("handle_ul_timeout: CONN ID: %d num_possible_blocks IS ZERO!!\n", conn->cid);
			if (ARQ_SELECTIVE_ACK == ack_type) {
				return;
			} else {
				ack_type = ARQ_CUMULATIVE_ACK;
			}
		}

		FLOG_DEBUG("handle_ul_timeout: CONN ID: %d ARQ Wnd Start: %d Start BSN: %d RX highest BSN: %d\n",
												conn->cid, rx_window_start, start_bsn, rx_highest_bsn);
		FLOG_DEBUG("handle_ul_timeout: CONN ID: %d Num Possible Blocks: %d Max Feedback IEs: %d\n",
												conn->cid, num_possible_blks, max_feedback_ies);
		int msg_alloc_size = sizeof(ARQ_feedback_message);


		if (ARQ_CUMULATIVE_ACK == ack_type) {
			msg_alloc_size += sizeof(ARQ_feedback_ie);
		} else {
			msg_alloc_size += max_feedback_ies*(sizeof(ARQ_feedback_ie)+4*sizeof(short)); // 4 is the maximum number of ACK MAPs that can
																						  // be present in a single Feedback_IE
		}
			
		ack_msg = (ARQ_feedback_message *)calloc(1, msg_alloc_size);
		assert (ack_msg != NULL);
		FLOG_DEBUG("handle_ul_timeout: CONN ID: %d Allocated %d bytes for ARQ_Feedback_IE at %p\n", conn->cid, msg_alloc_size, ack_msg);

		// Set fixed fields
		ack_msg->mgmt_msg_type = ARQ_FEEDBACK_MSG;
		ARQ_feedback_ie *fb_ie = (ARQ_feedback_ie*)(ack_msg->acks);
		fb_ie->cid = conn->cid;
		fb_ie->ack_type = ack_type;
		fb_ie->bsn = start_bsn;
		fb_ie->last = ARQ_CUMULATIVE_ACK == ack_type ? 1 : 0;  // Should be updated to 1 if only one Feedback IE is included

		// Generate selective maps for types other than ARQ_CUMULATIVE_ACK
		int ack_msg_offset = sizeof(ARQ_feedback_ie);
		int num_feedback_ies = 1;
		int used_size = msg_alloc_size;

		if (ack_type != ARQ_CUMULATIVE_ACK) {
			short total_blocks_covered = 0;
			ARQ_feedback_ie *fb_ie = (ARQ_feedback_ie*)(ack_msg->acks);
			short next_start_bsn = 0;
			do {
				short bitmap[4]; // to hold the ACK MAPs
				short bs_bitmap[4];

				short num_shorts_used_sel_map, num_blocks_covered_sel_map;
				short num_shorts_used_seq_map, num_blocks_covered_seq_map;
				
				construct_selective_ack_map (conn, start_bsn, rx_highest_bsn, bitmap, &num_shorts_used_sel_map, &num_blocks_covered_sel_map);
				FLOG_DEBUG("handle_ul_timeout: CONN ID: %d For FEEDBACK_IE %d Selective ACK MAP Bitmap Size:%d Blocks covered: %d\n", 
												conn->cid, num_feedback_ies, num_shorts_used_sel_map, num_blocks_covered_sel_map);
				next_start_bsn = (start_bsn+num_blocks_covered_sel_map)%ARQ_BSN_MODULUS;
				if (ARQ_SELECTIVE_ACK != ack_type) { 
					// This branch will be taken only once, just the first time
					construct_block_sequence_ack_map (conn, (start_bsn+1)%ARQ_BSN_MODULUS, rx_highest_bsn,  bs_bitmap, &num_shorts_used_seq_map, 
															&num_blocks_covered_seq_map);
					FLOG_DEBUG("handle_ul_timeout: CONN ID: %d For FEEDBACK IE %d Block Sequence ACK MAP Bitmap Size:%d Blocks covered: %d\n", 
												conn->cid, num_feedback_ies, num_shorts_used_seq_map, num_blocks_covered_seq_map);
					//num_blocks_covered_seq_map = 0; // This statement to be removed later
					if (((num_blocks_covered_seq_map+1) > num_blocks_covered_sel_map) || (((num_blocks_covered_seq_map+1 == num_blocks_covered_sel_map) && (num_shorts_used_seq_map <= num_shorts_used_sel_map)))) {
						fb_ie->ack_type = ack_type = ARQ_CUM_BLK_SEQ_SEL_ACK;
						FLOG_DEBUG("handle_ul_timeout: CONN ID: %d For FEEDBCK IE %d ACK_TYPE set to ARQ_CUM_BLK_SEQ_SEL_ACK\n", conn->cid, num_feedback_ies);
						num_possible_blks--;
						fb_ie->bsn = (start_bsn+1)%ARQ_BSN_MODULUS;
					} else {
						fb_ie->ack_type = ack_type = ARQ_CUM_SEL_ACK;
						FLOG_DEBUG("handle_ul_timeout: CONN ID: %d For FEEDBACK IE %d ACK_TYPE set to ARQ_CUM_SEL_ACK\n", 
												conn->cid, num_feedback_ies);
					}
					next_start_bsn = (start_bsn+num_blocks_covered_seq_map)%ARQ_BSN_MODULUS;
				}

				if (ARQ_SELECTIVE_ACK == ack_type || ARQ_CUM_SEL_ACK == ack_type) {
					// If CUM SEL, first bit should be set to 1
					if (ARQ_CUM_SEL_ACK == ack_type) {
						bitmap[0] |= 0x80;	
					}
					FLOG_DEBUG("handle_ul_timeout: CONN ID: %d copying bitmap %d %d %d %d to %p\n", conn->cid, 
												(unsigned short)bitmap[0], (unsigned short)bitmap[1], (unsigned short)bitmap[2],(unsigned short)bitmap[3],
												((char *)ack_msg->acks)+ack_msg_offset);
					memcpy(((char *)(ack_msg->acks)+ack_msg_offset), bitmap, num_shorts_used_sel_map*sizeof(short));
					ack_msg_offset += num_shorts_used_sel_map*sizeof(short);
					total_blocks_covered += num_blocks_covered_sel_map;
					fb_ie->no_ack_maps = num_shorts_used_sel_map;
					next_start_bsn = (start_bsn+num_blocks_covered_sel_map)%ARQ_BSN_MODULUS;
				} else {
					//assert (false);
					//assert (0);
					FLOG_DEBUG("handle_ul_timeout: CONN ID: %d copying bs_bitmap %d %d %d %d to %p\n", conn->cid, 
									(unsigned short)bs_bitmap[0], (unsigned short)bs_bitmap[1], (unsigned short)bs_bitmap[2],(unsigned short)bs_bitmap[3],
									((char *)ack_msg->acks)+ack_msg_offset);
					memcpy(((char *)ack_msg->acks)+ack_msg_offset, bs_bitmap, num_shorts_used_seq_map*sizeof(short));
					ack_msg_offset += num_shorts_used_seq_map*sizeof(short);
					total_blocks_covered += num_blocks_covered_seq_map;
					fb_ie->no_ack_maps = num_shorts_used_seq_map;
					next_start_bsn = (start_bsn+1+num_blocks_covered_seq_map)%ARQ_BSN_MODULUS;
				}

				// Convert ACK_WILL_BE_SENT's to ACK_SENT's

				int temp_bsn;
				int temp_idx = 0;
				int temp_idx_limit;
				if (ARQ_CUM_BLK_SEQ_SEL_ACK == ack_type) {
					temp_idx_limit = num_blocks_covered_seq_map+1;
				} else {
					temp_idx_limit = num_blocks_covered_sel_map;
				}
				for(temp_bsn = start_bsn; temp_bsn != rx_highest_bsn; temp_bsn++) {
				if (sync_flag == 0) {pthread_exit(NULL);}
					if (ARQ_ACK_SEL_WILL_BE_SENT == conn->ul_block_buffer.ul_block_buffer[temp_bsn%conn->ARQ_RX_WINDOW_SIZE].state ||
						ARQ_ACK_BS_WILL_BE_SENT == conn->ul_block_buffer.ul_block_buffer[temp_bsn%conn->ARQ_RX_WINDOW_SIZE].state) {
						if (temp_idx < temp_idx_limit) {
							conn->ul_block_buffer.ul_block_buffer[temp_bsn%conn->ARQ_RX_WINDOW_SIZE].state = ARQ_ACK_SENT;
							temp_idx++;
						}
						else {
							conn->ul_block_buffer.ul_block_buffer[temp_bsn%conn->ARQ_RX_WINDOW_SIZE].state = ARQ_BLK_RECEIVED;
						}
					}
				}
				
				FLOG_DEBUG("handle_ul_timeout: CONN ID: %d total_blocks_covered: %d\n", conn->cid, total_blocks_covered);
				assert (total_blocks_covered <= num_possible_blks);
				// It's possible for toal_blocks_covered to exceed num_possible_blocks as rx_highest_bsn might shift

				if (total_blocks_covered >= num_possible_blks) {
					FLOG_DEBUG("handle_ul_timeout: CONN ID: %d Last feedback is %d\n", conn->cid, num_feedback_ies);
					fb_ie->last = 1;
					break;
				}
				fb_ie = (ARQ_feedback_ie *)((char *)(ack_msg->acks)+ack_msg_offset);
				fb_ie->ack_type = ack_type = ARQ_SELECTIVE_ACK;
				fb_ie->last = 0;
				fb_ie->cid = conn->cid;
				fb_ie->bsn = next_start_bsn;
				start_bsn = next_start_bsn;

				ack_msg_offset += sizeof(ARQ_feedback_ie);
				num_feedback_ies++;
				
			} while(1);

			// Resize to the correct size
			used_size = sizeof(ARQ_feedback_message)+ack_msg_offset;
			assert (used_size <= msg_alloc_size);

			ack_msg = (ARQ_feedback_message *)realloc(ack_msg, used_size);
			num_feedback_ies++;
		}
		// Insert the message into the SDU queue
		int my_basic_cid;
		int ret_val = get_basic_cid(conn->cid, &my_basic_cid);
		if (ret_val == 0)
		{
			ARQ_feedback_message *tmpp = (ARQ_feedback_message*) mac_sdu_malloc(used_size, 5);
			memcpy(tmpp,ack_msg, used_size);
			//printf("Enqueue fb\n");
			enqueue_transport_sdu_queue(dl_sdu_queue, my_basic_cid, used_size, (void*)tmpp);
		}
		else
		{
			FLOG_DEBUG("Error : ARQ Connection not found. Ignoring ack message \n");
		}
		free(ack_msg);


	}
}


unsigned short masks[] = {0x8000, 0x4000, 0x2000, 0x1000, 0x0800, 0x0400, 0x0200, 0x0100,
						  0x0080, 0x0040, 0x0020, 0x0010, 0x0008, 0x0004, 0x0002, 0x0001};

// Return the number of shorts used up in bitmap
// Bitmap should be an array of size at least four
void construct_selective_ack_map (ul_connection_t *conn, short start_bsn, short rx_highest_bsn, short *bitmap, short *num_shorts_used, short *num_blocks_covered)
{
	short array_idx, offset;
	short num_blocks = MIN(mod(rx_highest_bsn-start_bsn, ARQ_BSN_MODULUS), 4*16-1); // No. of ACK Maps * Size of each ACK MAP -1
	int i;
	
	assert (num_blocks <= 63);
	// Clear the bitmap
	bitmap[0] = bitmap[1] = bitmap[2] = bitmap[3] = 0;

	FLOG_DEBUG("In construct_selective_ack_map: CONN ID: %d start_bsn: %d highest_bsn: %d mod: %d\n", conn->cid, start_bsn, rx_highest_bsn, mod(rx_highest_bsn-start_bsn, ARQ_BSN_MODULUS));

	// i starts from 1, since start_bsn is assumed to be received
	for (i = 0; i <= num_blocks; i++) {
		int bl_no = (start_bsn+i)%ARQ_BSN_MODULUS;
		int blk_idx = bl_no%conn->ARQ_RX_WINDOW_SIZE;

		if (ARQ_BLK_NOT_RECEIVED == conn->ul_block_buffer.ul_block_buffer[blk_idx].state ||
			ARQ_ACK_SENT == conn->ul_block_buffer.ul_block_buffer[blk_idx].state){
			continue;
		}

		conn->ul_block_buffer.ul_block_buffer[blk_idx].state = ARQ_ACK_SEL_WILL_BE_SENT;
		array_idx = i/16;
		offset = i%16;
		bitmap[array_idx] |= masks[offset];
	}
	*num_shorts_used = (num_blocks/16);	
	if (16 * (*num_shorts_used < num_blocks)) {
		(*num_shorts_used)++;
	}
	*num_blocks_covered = num_blocks;
	FLOG_DEBUG("In construct_selective_ack_map: CONN ID: %d Bitmap Size: %d Blocks Covered: %d\n", 
							conn->cid, *num_shorts_used, *num_blocks_covered);
	FLOG_DEBUG("In construct_selective_ack_map: CONN ID: %d Bitmap: %u %u %u %u\n", 
							conn->cid, (unsigned short)bitmap[0], (unsigned short)bitmap[1], (unsigned short)bitmap[2], (unsigned short)bitmap[3]);
}


void construct_block_sequence_ack_map (ul_connection_t *conn, short start_bsn, short rx_highest_bsn,short *bitmap, short *num_shorts_used, short *num_blocks_covered)
{
#define MAX_BLOCKS_TWO_SEQ 63
#define MAX_BLOCKS_THREE_SEQ 15

	short bsn = start_bsn;
	short blk_idx = bsn % conn->ARQ_RX_WINDOW_SIZE;
	short seq_idx = 0;
	int total_blocks_limit;

	*num_blocks_covered = 0;

	// Clear the bitmap
	bitmap[0]= bitmap[1] = bitmap[2] = bitmap[3] = 0;

	// Skip all the leading blocks that are not in the ARQ_BLK_RECEIVED state
	while (bsn != rx_highest_bsn && conn->ul_block_buffer.ul_block_buffer[blk_idx].state == ARQ_BLK_NOT_RECEIVED) {
		bsn = (bsn+1)%ARQ_BSN_MODULUS;
		blk_idx = bsn % conn->ARQ_RX_WINDOW_SIZE;
		(*num_blocks_covered)++;
	}

	total_blocks_limit = (rx_highest_bsn-start_bsn)%ARQ_BSN_MODULUS;

	boolean flag;
	int total_blocks_count = 0;

	for (seq_idx = 0; seq_idx < 4; seq_idx++) { // for each of the maximum number of ACK MAPS
		short sub_seq_idx = 0;
		short blk_count[3]; 	// The number of blocks in the at most three sub-sequeunces
							    // for this sequence
		short seq_type = 2; // 0 - 2-block seq, 1 - 3-block seq, 2 - in the deciding mode
		short block_limit;
		short blk_seq_state[3];
		short tmp_blk_count;

		do{
			tmp_blk_count = 0;
			// Initialize
			blk_seq_state[sub_seq_idx] = 0;
			blk_count[sub_seq_idx] = 0;

			block_limit = (0 == seq_type || 2 == seq_type) ? MAX_BLOCKS_TWO_SEQ : MAX_BLOCKS_THREE_SEQ;

			flag = (conn->ul_block_buffer.ul_block_buffer[blk_idx].state == ARQ_BLK_RECEIVED ||
					conn->ul_block_buffer.ul_block_buffer[blk_idx].state == ARQ_ACK_SEL_WILL_BE_SENT ||
					conn->ul_block_buffer.ul_block_buffer[blk_idx].state == ARQ_ACK_BS_WILL_BE_SENT) 
										? true : false;

			if (true == flag) {
				while(tmp_blk_count < block_limit 
						&& bsn != rx_highest_bsn && 
						(conn->ul_block_buffer.ul_block_buffer[blk_idx].state == ARQ_BLK_RECEIVED ||
						 conn->ul_block_buffer.ul_block_buffer[blk_idx].state == ARQ_ACK_SEL_WILL_BE_SENT ||
						 conn->ul_block_buffer.ul_block_buffer[blk_idx].state == ARQ_ACK_BS_WILL_BE_SENT)) {
					conn->ul_block_buffer.ul_block_buffer[bsn%conn->ARQ_RX_WINDOW_SIZE].state = ARQ_ACK_BS_WILL_BE_SENT;
					bsn = (bsn+1)%ARQ_BSN_MODULUS;
					blk_idx = bsn % conn->ARQ_RX_WINDOW_SIZE;
					tmp_blk_count++;
				}
			} else {
				while(tmp_blk_count < block_limit 
						&& bsn != rx_highest_bsn && 
						conn->ul_block_buffer.ul_block_buffer[blk_idx].state != ARQ_BLK_RECEIVED &&
						conn->ul_block_buffer.ul_block_buffer[blk_idx].state != ARQ_ACK_SEL_WILL_BE_SENT &&
						conn->ul_block_buffer.ul_block_buffer[blk_idx].state != ARQ_ACK_BS_WILL_BE_SENT) {
					bsn = (bsn+1)%ARQ_BSN_MODULUS;
					blk_idx = bsn % conn->ARQ_RX_WINDOW_SIZE;
					tmp_blk_count++;
				}
			}
			//block_seq_end = mod (bsn-1)%ARQ_BSN_MODULUS;

			total_blocks_count += tmp_blk_count;

			// If undecided, see if we can decide now
			if (2 == seq_type) {
				if ((0 == sub_seq_idx && tmp_blk_count > MAX_BLOCKS_THREE_SEQ) ||
					(1 == sub_seq_idx && tmp_blk_count > MAX_BLOCKS_THREE_SEQ)) {
					seq_type = 0;
				} else if (1 == sub_seq_idx) {
					seq_type = 1;
				}
			}

			blk_count[sub_seq_idx] = tmp_blk_count;
			blk_seq_state[sub_seq_idx] = (true == flag ? 1 : 0);

			if ((0 == seq_type && 1 == sub_seq_idx) || (1 == seq_type && 2 == sub_seq_idx)) {
				break;
			}


			sub_seq_idx++;

		} while (1);

		short sub_seq_length = 6;
		short num_sub_seqs = 2;
		short resvd_bits = 1;

		if (1 == seq_type) {
			sub_seq_length = 4;
			num_sub_seqs = 3;
			resvd_bits = 0;
			bitmap[seq_idx] = 0x8000;
		}

		boolean quit = false;
		for (sub_seq_idx = 0; sub_seq_idx < num_sub_seqs && false == quit; sub_seq_idx++) {
			bitmap[seq_idx] |= (blk_count[sub_seq_idx] << (sub_seq_length*((num_sub_seqs-1)-sub_seq_idx) + resvd_bits));
			bitmap[seq_idx] |= (blk_seq_state[sub_seq_idx] << (sub_seq_length*num_sub_seqs+(num_sub_seqs-1-sub_seq_idx)+resvd_bits));
			*num_blocks_covered += blk_count[sub_seq_idx];
			if (0 == blk_count[sub_seq_idx]) {
				quit = true;
			}
		}
		if (true == quit) {
			*num_shorts_used = seq_idx+1;
			FLOG_DEBUG("In construct_block_sequence_ack_map: CONN ID: %d Bitmap Size: %d Blocks Covered: %d\n", 
									conn->cid, *num_shorts_used, *num_blocks_covered);
			FLOG_DEBUG("In construct_block_sequence_ack_map: CONN ID: %d Bitmap: %d %d %d %d\n", 
									conn->cid, bitmap[0], bitmap[1], bitmap[2], bitmap[3]);
			return;
		}
	}
	*num_shorts_used = seq_idx;
	FLOG_DEBUG("In construct_block_sequence_ack_map: CONN ID: %d Bitmap Size: %d Blocks Covered: %d\n", 
							conn->cid, *num_shorts_used, *num_blocks_covered);
	FLOG_DEBUG("In construct_block_sequence_ack_map: CONN ID: %d Bitmap: %d %d %d %d\n", 
							conn->cid, bitmap[0], bitmap[1], bitmap[2], bitmap[3]);
	return;
}
