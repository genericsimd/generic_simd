/* ----------------------------------------------------------------------------
   IBM Confidential

   IBM Wireless M2M Platform   

   (C)Copyright IBM Corp. 2008, 2009, 2010, 2011

   All Rights Reserved.

   File Name: log.c

   Change Activity:

   Date             Description of Change                   By
   -----------      ---------------------					--------
   1-Oct.2008       Created                                 Uma Devi

   ----------------------------------------------------------------------------
   PROLOG END TAG zYx                                                         */

#include <stdio.h>
#include <stdlib.h>
#include <sys/syscall.h>
#include <unistd.h>
#include <assert.h>
#include <string.h>
//#include "memmgmt.h"
#include "log.h"
#include "util.h"
#include "debug.h"

#include "sdu_cid_queue.h"

#define WiMAX_mac_malloc malloc
#define WiMAX_mac_free   free

#ifdef LOG_METRICS
trace_buffer_record_t all_trace_buffers;
pthread_key_t	tbuffer_key;
pthread_key_t   tperf_key; /* key to store performance data */
long long base_time = -1;
void init_trace_buffer_record(int max_buffers)
{
	if (NULL != all_trace_buffers.trace_buffers) {
		FLOG_ERROR("TRACE BUFFER RECORD already initialized\n");
		assert (0);
	}

	all_trace_buffers.trace_buffers = (trace_buffer_t **)WiMAX_mac_malloc(max_buffers*sizeof(trace_buffer_t *));

	assert (all_trace_buffers.trace_buffers != NULL);

	all_trace_buffers.max_buffers = max_buffers;
	all_trace_buffers.num_used = 0;

	assert (pthread_key_create(&tbuffer_key, NULL) == 0);
	assert (pthread_key_create(&tperf_key, NULL) == 0);
}

void free_trace_buffer_record(void)
{
	for (int i = 0; i < all_trace_buffers.num_used; i++) {
		WiMAX_mac_free(all_trace_buffers.trace_buffers[i]->traces);
		WiMAX_mac_free(all_trace_buffers.trace_buffers[i]);
		all_trace_buffers.trace_buffers[i] = NULL;
	}

	WiMAX_mac_free (all_trace_buffers.trace_buffers);
	all_trace_buffers.trace_buffers = NULL;

}

void create_init_trace_buffer (size_t size, char *desc)
{
	if(base_time < 0)
		base_time = readtsc();
	trace_buffer_t *tb;

	FLOG_DEBUG("TRACE BUFFER: in create_init_trace_buffer\n");

	tb = (trace_buffer_t *)WiMAX_mac_malloc(sizeof(trace_buffer_t));
	assert (tb != NULL);

	tb->traces = (char *)WiMAX_mac_malloc(size);
	assert (tb->traces != NULL);

	tb->size = size;
	tb->curr_limit = size;
	tb->read_idx = tb->write_idx = 0;
	tb->num_faults = 0;
	tb->thread_id = syscall(SYS_gettid);
	strcpy(tb->thread_desc, desc);

	pthread_setspecific(tbuffer_key, tb);
	int my_slot = fetch_and_incr(&all_trace_buffers.num_used);
	FLOG_DEBUG("TRACE BUFFER: create_init_trace_buffer: my_slot: %d\n", my_slot);
	FLOG_DEBUG("TRACE_BUFFER: create_init_trace_buffer: thread id: %d desc: %s\n", tb->thread_id,
			desc);
	assert (my_slot < all_trace_buffers.max_buffers);
	all_trace_buffers.trace_buffers[my_slot] = tb;
}

void add_mutex_to_trace_record (pthread_mutex_t *mutex, char *desc)
{
	return;
	/*	int my_slot = fetch_and_incr(&all_trace_buffers.num_md_used);
	FLOG_DEBUG("TRACE BUFFER: add_mutex_to_trace_record: my_slot: %d desc: %s\n", my_slot, desc);
	assert (my_slot < MAX_MDESCS);
	all_trace_buffers.mdescs[my_slot].ma = mutex;
	if (desc != NULL) {
		strcpy(all_trace_buffers.mdescs[my_slot].mdesc, desc);
	} else {
		strcpy(all_trace_buffers.mdescs[my_slot].mdesc, "");
	}*/
}

// Notes for me: The reader should print the owners of all locks before writing traces to file.

// Retrun the contents of next unread entry in out_buf
// out_buf should have been allocated sufficiently by the caller
op_status_t read_trace_entry(trace_buffer_t *tb, char *out_buf)
{
	op_status_t ret_val = E_Q_EMPTY;

	assert (tb != NULL);

	if (tb->read_idx != tb->write_idx) {
		int new_read_idx;
		int rw_gap = mod(tb->write_idx-tb->read_idx, tb->curr_limit);

		trace_event_type_t et = tb->traces[(tb->read_idx+1)%tb->curr_limit];
		switch (et) {
		case LOCK_TRYING:
		case LOCK_ACQUIRED:
		case LOCK_RELEASED:
			assert (rw_gap >= sizeof(lock_trace_info_t));
			lock_trace_info_t *lti = (lock_trace_info_t *)(&(tb->traces[(tb->read_idx+1)%tb->curr_limit]));
			sprintf(out_buf, "%d %d %lld %p\n", lti->ev_type, lti->thread_id, lti->ev_ts, lti->lock_addr);
			new_read_idx = (tb->read_idx+sizeof(lock_trace_info_t))%tb->curr_limit;
			break;

		case SDU_QUEUE_LENGTH:
			assert (rw_gap >= sizeof(sdu_q_trace_info_t));
			sdu_q_trace_info_t *sti = (sdu_q_trace_info_t *)(&(tb->traces[(tb->read_idx+1)%tb->curr_limit]));
			sprintf(out_buf, "%d %d %lld %d %d\n", sti->ev_type, sti->thread_id, sti->ev_ts, sti->cid, sti->sdu_q_len);
			new_read_idx = (tb->read_idx+sizeof(sdu_q_trace_info_t))%tb->curr_limit;
			break;

		case FRAME_CREATION_START:
		case FRAME_CREATION_END:
			assert (rw_gap >= sizeof(frame_trace_info_t));
			frame_trace_info_t *fti = (frame_trace_info_t *)(&(tb->traces[(tb->read_idx+1)%tb->curr_limit]));
			sprintf(out_buf, "%d %d %lld %d\n", fti->ev_type, fti->thread_id, fti->ev_ts, fti->frame_num);
			new_read_idx = (tb->read_idx+sizeof(frame_trace_info_t))%tb->curr_limit;
			break;
		case PERF_PROFILE_EVENT:
			assert (rw_gap >= sizeof(perf_trace_info_t));
			perf_trace_info_t *pti = (perf_trace_info_t *)(&(tb->traces[(tb->read_idx+1)%tb->curr_limit]));
			sprintf(out_buf, "%d %d %lld\n", pti->frame_id, pti->event_id, pti->ev_ts);
			new_read_idx = (tb->read_idx+sizeof(perf_trace_info_t))%tb->curr_limit;
			break;
		default:
			FLOG_ERROR("Trace Read: Thread Id: %d Unknown event type: %d\n", tb->thread_id, et);
			assert (0);
			break;
		}
		if (new_read_idx < tb->read_idx) {
			tb->curr_limit = tb->size;
		}
		tb->read_idx = new_read_idx;
		ret_val = SUCCESS;
	} else {
		ret_val = E_Q_EMPTY;
	}
	return ret_val;
}


op_status_t read_trace_info_struct(trace_buffer_t *tb, void **pti, trace_event_type_t *pet)
{
	op_status_t ret_val = E_Q_EMPTY;

	assert (tb != NULL);

	if (tb->read_idx != tb->write_idx) {
		int new_read_idx;
		int rw_gap = mod(tb->write_idx-tb->read_idx, tb->curr_limit);

		trace_event_type_t et = tb->traces[(tb->read_idx+1)%tb->curr_limit];
		*pet = et;
		switch (et) {
		case LOCK_TRYING:
		case LOCK_ACQUIRED:
		case LOCK_RELEASED: {
			lock_trace_info_t *lti = (lock_trace_info_t *)(&(tb->traces[(tb->read_idx+1)%tb->curr_limit]));
			*pti = lti;
			new_read_idx = (tb->read_idx+sizeof(lock_trace_info_t))%tb->curr_limit;
			break;
		}

		case SDU_QUEUE_LENGTH: {
			sdu_q_trace_info_t *sti = (sdu_q_trace_info_t *)(&(tb->traces[(tb->read_idx+1)%tb->curr_limit]));
			*pti = sti;
			new_read_idx = (tb->read_idx+sizeof(sdu_q_trace_info_t))%tb->curr_limit;
			break;
		}

		case FRAME_CREATION_START:
		case FRAME_CREATION_END: {
			frame_trace_info_t *fti = (frame_trace_info_t *)(&(tb->traces[(tb->read_idx+1)%tb->curr_limit]));
			*pti = fti;
			new_read_idx = (tb->read_idx+sizeof(frame_trace_info_t))%tb->curr_limit;
			break;
		}
		case PERF_PROFILE_EVENT:
		{
			perf_trace_info_t *perf_ti = (perf_trace_info_t *)(&(tb->traces[(tb->read_idx+1)%tb->curr_limit]));
			*pti = perf_ti;
			new_read_idx = (tb->read_idx+sizeof(frame_trace_info_t))%tb->curr_limit;
			break;
		}
		default:
			FLOG_ERROR("Trace Read: Thread Id: %d Unknown event type: %d\n", tb->thread_id, et);
			assert (0);
			break;
		}
		perf_trace_info_t *a;
		if (new_read_idx < tb->read_idx) {
			tb->curr_limit = tb->size;
		}
		tb->read_idx = new_read_idx;
		ret_val = SUCCESS;
	} else {
		ret_val = E_Q_EMPTY;
	}
	return ret_val;
}


op_status_t read_trace_info_bulk(trace_buffer_t *tb, void **pti, size_t *num_bytes, int *new_read_idx)
{
	op_status_t ret_val = E_Q_EMPTY;

	assert (tb != NULL);
	int write_idx = tb->write_idx;

	if (tb->read_idx != write_idx) {
		*pti = (void *)(&(tb->traces[(tb->read_idx+1)%tb->curr_limit]));
		if (write_idx < tb->read_idx) {
			if (tb->read_idx < tb->curr_limit-1) {
				*num_bytes = tb->curr_limit-tb->read_idx-1;
				*new_read_idx = tb->curr_limit-1;
			} else {
				assert (tb->read_idx == tb->curr_limit-1);
				*num_bytes = write_idx+1;
				tb->curr_limit = tb->size;
				*new_read_idx = write_idx;
			}
		} else {
			*num_bytes = write_idx-tb->read_idx;
			*new_read_idx = write_idx;
		}
		ret_val = SUCCESS;
	} else {
		ret_val = E_Q_EMPTY;
	}
	return ret_val;
}

void LOG_EVENT_perf_profile_event(perf_profile_type_t event_id) {
	// get the frame and event id
	long long ev_ts = readtsc() - base_time;
	int frame_id = (int)get_current_frame_number();

	// write to the trace buffer
	trace_buffer_t *tb = pthread_getspecific(tbuffer_key);
	if (NULL != tb) {
		int tmp_lim = tb->size;
		if (tb->read_idx <= tb->write_idx) {
			if ((tb->write_idx+sizeof(perf_trace_info_t)) >= tb->size) {
				tmp_lim = tb->curr_limit = tb->write_idx+1;
			} else {
				tmp_lim = tb->size;
			}
		}
		if (mod(tb->read_idx-tb->write_idx-1, tmp_lim) >= sizeof(perf_trace_info_t)) {
			perf_trace_info_t *pti = (perf_trace_info_t *)(&(tb->traces[(tb->write_idx+1)%tb->curr_limit]));
			pti->ev_type = PERF_PROFILE_EVENT;
			pti->frame_id = frame_id;
			pti->event_id = event_id;
			pti->ev_ts = ev_ts;
			tb->write_idx = (tb->write_idx+sizeof(perf_trace_info_t))%tb->curr_limit;
		} else {
			tb->num_faults++;
			TRACE1(4, "Thread Id: %d\n", tb->thread_id);
			assert(0);
		}
	}
}
void LOG_EVENT_burst_profile_event(int event_id, int burst_id, int num_bytes) {
	// get the frame and event id
	long long ev_ts = readtsc() - base_time;

	int frame_id = (int)get_current_frame_number();

	// write to the trace buffer
	trace_buffer_t *tb = pthread_getspecific(tbuffer_key);
	if (NULL != tb) {
		int tmp_lim = tb->size;
		if (tb->read_idx <= tb->write_idx) {
			if ((tb->write_idx+sizeof(burst_trace_info_t)) >= tb->size) {
				tmp_lim = tb->curr_limit = tb->write_idx+1;
			} else {
				tmp_lim = tb->size;
			}
		}
		if (mod(tb->read_idx-tb->write_idx-1, tmp_lim) >= sizeof(burst_trace_info_t)) {
			burst_trace_info_t *pti = (burst_trace_info_t *)(&(tb->traces[(tb->write_idx+1)%tb->curr_limit]));
			pti->ev_type = BURST_PROFILE_EVENT;
			pti->frame_id = frame_id;
			pti->event_id = event_id;
			pti->ev_ts = ev_ts ;
			pti->burst_id = burst_id;
			pti->num_bytes = num_bytes;
			tb->write_idx = (tb->write_idx+sizeof(burst_trace_info_t))%tb->curr_limit;
		} else {
			tb->num_faults++;
			FLOG_ERROR("Thread Id: %d\n", tb->thread_id);
			assert(0);
		}
	}
}


#endif
