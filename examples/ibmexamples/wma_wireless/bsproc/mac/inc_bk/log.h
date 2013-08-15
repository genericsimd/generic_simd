/* ----------------------------------------------------------------------------
   IBM Confidential

   IBM Wireless M2M Platform   

   (C)Copyright IBM Corp. 2008, 2009, 2010, 2011

   All Rights Reserved.

   File Name: log.h

   Change Activity:

   Date             Description of Change                   By
   -----------      ---------------------					--------
   1-Oct.2008       Created                                 Uma Devi

   ----------------------------------------------------------------------------
   PROLOG END TAG zYx                                                         */

#ifndef _LOG_H_
#define _LOG_H_
#include <pthread.h>
#include <stdio.h>
#include <sys/types.h>
#include <assert.h>
#include "arq_types.h"
#include "debug.h"
#include "util.h"

#define MAX_TRACE_BUFFERS   50

typedef enum {
	FRAME_CREATION_START = 1,
	FRAME_CREATION_END   = 2,
	LOCK_TRYING			 = 3,
	LOCK_ACQUIRED		 = 4,
	LOCK_RELEASED		 = 5,
	SDU_QUEUE_LENGTH	 = 6,
	SDU_ALLOCED			 = 7,
	SDU_FREED			 = 8,
	FRAME_COMPOSITION	 = 9,
	PERF_PROFILE_EVENT	 = 10,
	BURST_PROFILE_EVENT  = 11,
}trace_event_type_t;

typedef enum {
	PERF_START	    = 1,
	DL_MAP 		    = 2,
	SCHED_START     = 3,

} perf_profile_type_t;

typedef struct {
	trace_event_type_t	ev_type;
	pid_t			thread_id;
	long long		ev_ts;
	pthread_mutex_t	*lock_addr;
} lock_trace_info_t;

typedef struct {
	trace_event_type_t	ev_type;
	pid_t			thread_id;
	long long		ev_ts;
	int				sdu_q_len;
	int				cid;
} sdu_q_trace_info_t;

typedef struct {
  trace_event_type_t	ev_type;
  pid_t			thread_id;
  long long		ev_ts;
  void* sdu_start_addr;
  short class_type;
} sdu_mem_scope_info_t;

typedef struct {
	trace_event_type_t	ev_type;
	pid_t				thread_id;
	long long			ev_ts;
	int					frame_num;
} frame_trace_info_t;

typedef struct {
	trace_event_type_t	ev_type;
	int 				frame_id;
	int					event_id;
	long long 			ev_ts;
} perf_trace_info_t;

typedef struct {
	trace_event_type_t	ev_type;
	int 				frame_id;
	int					event_id;
	int					burst_id;
	int					num_bytes ;
	long long			ev_ts;
} burst_trace_info_t;

typedef struct {
	char		 *traces;
	size_t		 size;
	size_t		 curr_limit;
	int			 read_idx;
	int			 write_idx;
	int 		 num_faults;
	pid_t		thread_id;
	char		thread_desc[64];
} trace_buffer_t;

typedef struct {
	pthread_mutex_t *ma;
	char		mdesc[64];
} mutex_descriptor_t;

#define MAX_MDESCS	100

// Struct to maintain pointers to all trace buffers
// This is to facilitate their
// reading by a common reader thread.
typedef struct {
	trace_buffer_t **trace_buffers;
	int num_used;
	int max_buffers;
	mutex_descriptor_t mdescs[MAX_MDESCS];
	int num_md_used;
} trace_buffer_record_t;


#ifdef LOG_METRICS
extern pthread_key_t tbuffer_key;
extern trace_buffer_record_t all_trace_buffers;
extern pthread_t log_reader;

extern void init_trace_buffer_record(int max_buffers);
extern void free_trace_buffer_record(void);
extern void create_init_trace_buffer (size_t size, char *);
extern op_status_t read_trace_entry(trace_buffer_t *tb, char *out_buf);
extern op_status_t read_trace_info_struct(trace_buffer_t *tb, void **, trace_event_type_t *);
extern op_status_t read_trace_info_bulk(trace_buffer_t *tb, void **, size_t *, int *);
extern void add_mutex_to_trace_record (pthread_mutex_t *mutex, char *desc);
extern void start_log_reader(char *);
extern void release_log_reader();
extern void LOG_EVENT_perf_profile_event(perf_profile_type_t event_id);
extern void LOG_EVENT_burst_profile_event(int event_id, int burst_id, int num_bytes);
#else
#define init_trace_buffer_record(max_buffers) ;
#define read_trace_entry(tb, out_buf)  ;
#define read_trace_info_struct(tb, tis, pte)  ;
#define read_trace_info_bulk(tb, tis, bytes, new_read_idx)  ;
#define add_mutex_to_trace_record(mutex, desc) ;
#define create_init_trace_buffer(size, desc) ;
#define free_trace_buffer_record() ;
#define start_log_reader ;
#define  release_log_reader() ;
#define LOG_EVENT_perf_profile_event(event_id) ;
#define LOG_EVENT_burst_profile_event(event_id, burst_id, num_bytes) ;
#endif



#ifdef LOG_METRICS
#define NUM_TRACE_BUFFERS all_trace_buffers.num_used
#define NUM_TRACED_MUTEXES all_trace_buffers.num_md_used
#define TRACE_BUFFER(i)	  all_trace_buffers.trace_buffers[i]

/**/
/*
#define LOG_EVENT_lock_generic(ts, la, lock_type) {			\
									trace_buffer_t *tb = pthread_getspecific(tbuffer_key); \
									if (NULL != tb) { \
										int tmp_lim = tb->size; \
										TRACE4(4, "Thread Id: %d Buffer: %p, Write Idx: %d Read Idx: %d\n", tb->thread_id, tb, tb->write_idx, tb->read_idx); \
										if (tb->read_idx <= tb->write_idx) { \
											if ((tb->write_idx+sizeof(lock_trace_info_t)) >= tb->size) { \
												tmp_lim = tb->curr_limit = tb->write_idx+1; \
											} else { \
												tmp_lim = tb->size; \
											} \
										} \
										do {\
											if (mod(tb->read_idx-tb->write_idx-1, tmp_lim) >= sizeof(lock_trace_info_t)) { \
												lock_trace_info_t *lt = (lock_trace_info_t *)(&(tb->traces[(tb->write_idx+1)%tb->curr_limit])); \
												lt->ev_type = lock_type; \
												lt->thread_id = tb->thread_id; \
												lt->ev_ts = ts; \
												lt->lock_addr = la; \
												tb->write_idx = (tb->write_idx+sizeof(lock_trace_info_t))%tb->curr_limit; \
												break; \
											} \
										} while(1); \
									} \
								}


*/
#define LOG_EVENT_frame_generic(ts, fnum, etype) { \
									trace_buffer_t *tb = pthread_getspecific(tbuffer_key); \
									if (NULL != tb) { \
										int tmp_lim = tb->size; \
										/* TRACE4(4, "Thread Id: %d Buffer: %p, Write Idx: %d Read Idx: %d\n", tb->thread_id, tb, tb->write_idx, tb->read_idx); */\
										if (tb->read_idx <= tb->write_idx) { \
											if ((tb->write_idx+sizeof(frame_trace_info_t)) >= tb->size) { \
												tmp_lim = tb->curr_limit = tb->write_idx+1; \
											} else { \
												tmp_lim = tb->size; \
											} \
										} \
										if (mod(tb->read_idx-tb->write_idx-1, tmp_lim) >= sizeof(frame_trace_info_t)) { \
											frame_trace_info_t *ft = (frame_trace_info_t *)(&(tb->traces[(tb->write_idx+1)%tb->curr_limit])); \
											ft->ev_type = etype; \
											ft->thread_id = tb->thread_id; \
											ft->ev_ts = ts; \
											ft->frame_num = fnum; \
											tb->write_idx = (tb->write_idx+sizeof(frame_trace_info_t))%tb->curr_limit; \
										} else { \
											tb->num_faults++; \
											TRACE1(4, "Thread Id: %d\n", tb->thread_id); \
											assert(0); \
										} \
									} \
								}




/*
#define LOG_EVENT_sdu_len(ts, cid, qlen) {\
									trace_buffer_t *tb = pthread_getspecific(tbuffer_key); \
									if (NULL != tb) { \
										int tmp_lim = tb->size; \
										TRACE4(4, "Thread Id: %d Buffer: %p, Write Idx: %d Read Idx: %d\n", tb->thread_id, tb, tb->write_idx, tb->read_idx); \
										if (tb->read_idx <= tb->write_idx) { \
											if ((tb->write_idx+sizeof(sdu_q_trace_info_t)) >= tb->size) { \
												tmp_lim = tb->curr_limit = tb->write_idx+1; \
											} else { \
												tmp_lim = tb->size; \
											} \
										} \
										if (mod(tb->read_idx-tb->write_idx-1, tmp_lim) >= sizeof(sdu_q_trace_info_t)) { \
											sdu_q_trace_info_t *ft = (sdu_q_trace_info_t *)(&(tb->traces[(tb->write_idx+1)%tb->curr_limit])); \
											ft->ev_type = SDU_QUEUE_LENGTH; \
											ft->thread_id = tb->thread_id; \
											ft->ev_ts = ts; \
											ft->cid = cid; \
											ft->sdu_q_len = qlen; \
											tb->write_idx = (tb->write_idx+sizeof(sdu_q_trace_info_t))%tb->curr_limit; \
										} else { \
											tb->num_faults++; \
											TRACE1(4, "Thread Id: %d\n", tb->thread_id); \
											assert(0); \
										} \
									} \
								}

*/
/**/

/*
#define LOG_EVENT_sdu_mem_scope_generic(ts, sdu_addr, class_type, etype) {	\
									trace_buffer_t *tb = pthread_getspecific(tbuffer_key); \
									if (NULL != tb) { \
										int tmp_lim = tb->size; \
										TRACE4(4, "Thread Id: %d Buffer: %p, Write Idx: %d Read Idx: %d\n", tb->thread_id, tb, tb->write_idx, tb->read_idx); \
										if (tb->read_idx <= tb->write_idx) { \
											if ((tb->write_idx+sizeof(sdu_mem_scope_info_t)) >= tb->size) { \
												tmp_lim = tb->curr_limit = tb->write_idx+1; \
											} else { \
												tmp_lim = tb->size; \
											} \
										} \
										if (mod(tb->read_idx-tb->write_idx-1, tmp_lim) >= sizeof(sdu_mem_scope_info_t)) { \
											sdu_mem_scope_info_t *ft = (sdu_mem_scope_info_t *)(&(tb->traces[(tb->write_idx+1)%tb->curr_limit])); \
											ft->ev_type = etype; \
											ft->thread_id = tb->thread_id; \
											ft->ev_ts = ts; \
											ft->sdu_start_addr = sdu_addr; \
											ft->class_type = class_type; \
											tb->write_idx = (tb->write_idx+sizeof(sdu_mem_scope_info_t))%tb->curr_limit; \
										} else { \
											tb->num_faults++; \
											TRACE1(4, "Thread Id: %d\n", tb->thread_id); \
											assert(0); \
										} \
									} \
								}

*/
/**/


/*
static void inline LOG_EVENT_lock_generic(long long ts, pthread_mutex_t *la, trace_event_type_t lock_type) {
									trace_buffer_t *tb = pthread_getspecific(tbuffer_key);
									if (NULL == tb) { return; }
									int tmp_lim = tb->size;
									// *TRACE4(4, "Thread Id: %d Buffer: %p, Write Idx: %d Read Idx: %d\n", tb->thread_id, tb, tb->write_idx, tb->read_idx); * //
									if (tb->read_idx <= tb->write_idx) {
										if ((tb->write_idx+sizeof(lock_trace_info_t)) >= tb->size) {
											tmp_lim = tb->curr_limit = tb->write_idx+1;
										} else {
											tmp_lim = tb->size;
										}
									}
									if (mod(tb->read_idx-tb->write_idx-1, tmp_lim) >= sizeof(lock_trace_info_t)) {
										lock_trace_info_t *lt = (lock_trace_info_t *)(&(tb->traces[(tb->write_idx+1)%tb->curr_limit]));
										lt->ev_type = lock_type;
										lt->thread_id = tb->thread_id;
										lt->ev_ts = ts;
										lt->lock_addr = la;
										tb->write_idx = (tb->write_idx+sizeof(lock_trace_info_t))%tb->curr_limit;
										//printf("%d %d %lld %p\n", lt->ev_type, lt->thread_id, lt->ev_ts, lt->lock_addr);
									} else {
										tb->num_faults++;
										TRACE1(4, "Thread Id: %d\n", tb->thread_id);
										assert(0);
									}
								}

static void LOG_EVENT_frame_generic(long long ts, int fnum, trace_event_type_t etype) {
									trace_buffer_t *tb = pthread_getspecific(tbuffer_key);
									if (NULL == tb) { return; }
									int tmp_lim = tb->size;
									// *TRACE4(4, "Thread Id: %d Buffer: %p, Write Idx: %d Read Idx: %d\n", tb->thread_id, tb, tb->write_idx, tb->read_idx); * //
									if (tb->read_idx <= tb->write_idx) {
										if ((tb->write_idx+sizeof(frame_trace_info_t)) >= tb->size) {
											tmp_lim = tb->curr_limit = tb->write_idx+1;
										} else {
											tmp_lim = tb->size;
										}
									}
									if (mod(tb->read_idx-tb->write_idx-1, tmp_lim) >= sizeof(frame_trace_info_t)) {
										frame_trace_info_t *ft = (frame_trace_info_t *)(&(tb->traces[(tb->write_idx+1)%tb->curr_limit]));
										ft->ev_type = etype;
										ft->thread_id = tb->thread_id;
										ft->ev_ts = ts;
										ft->frame_num = fnum;
										tb->write_idx = (tb->write_idx+sizeof(frame_trace_info_t))%tb->curr_limit;
										//printf("%d %d %lld %d\n", ft->ev_type, ft->thread_id, ft->ev_ts, ft->frame_num);
									} else {
										tb->num_faults++;
										TRACE1(4, "Thread Id: %d\n", tb->thread_id);
										assert(0);
									}
								}

static inline void LOG_EVENT_sdu_len(long long ts, int cid, int qlen) {
									trace_buffer_t *tb = pthread_getspecific(tbuffer_key);
									if (NULL == tb) { return; }
									int tmp_lim = tb->size;
									// *TRACE4(4, "Thread Id: %d Buffer: %p, Write Idx: %d Read Idx: %d\n", tb->thread_id, tb, tb->write_idx, tb->read_idx); * //
									if (tb->read_idx <= tb->write_idx) {
										if ((tb->write_idx+sizeof(sdu_q_trace_info_t)) >= tb->size) {
											tmp_lim = tb->curr_limit = tb->write_idx+1;
										} else {
											tmp_lim = tb->size;
										}
									}
									if (mod(tb->read_idx-tb->write_idx-1, tmp_lim) >= sizeof(sdu_q_trace_info_t)) {
										sdu_q_trace_info_t *ft = (sdu_q_trace_info_t *)(&(tb->traces[(tb->write_idx+1)%tb->curr_limit]));
										ft->ev_type = SDU_QUEUE_LENGTH;
										ft->thread_id = tb->thread_id;
										ft->ev_ts = ts;
										ft->cid = cid;
										ft->sdu_q_len = qlen;
										tb->write_idx = (tb->write_idx+sizeof(sdu_q_trace_info_t))%tb->curr_limit;
										//printf("%d %d %lld %d %d\n", ft->ev_type, ft->thread_id, ft->ev_ts, ft->cid, ft->sdu_q_len);
									} else {
										tb->num_faults++;
										TRACE1(4, "Thread Id: %d\n", tb->thread_id);
										assert(0);
									}
								}
*/


/* static void LOG_EVENT_sdu_mem_scope_generic(long long ts, void* sdu_addr, short class_type, trace_event_type_t etype) { */
/*   trace_buffer_t *tb = pthread_getspecific(tbuffer_key); */
/*   if (NULL != tb) {			     */
/*     int tmp_lim = tb->size;						 */
/*     TRACE4(4, "Thread Id: %d Buffer: %p, Write Idx: %d Read Idx: %d\n", tb->thread_id, tb, tb->write_idx, tb->read_idx);  */
/*     if (tb->read_idx <= tb->write_idx) {				 */
/*       if ((tb->write_idx+sizeof(sdu_mem_scope_info_t)) >= tb->size) {	 */
/* 	tmp_lim = tb->curr_limit = tb->write_idx+1;  */
/*       } else {							        */
/* 	tmp_lim = tb->size;						 */
/*       }									 */
/*     }									 */
/*     if (mod(tb->read_idx-tb->write_idx-1, tmp_lim) >= sizeof(sdu_mem_scope_info_t)) {  */
/*       sdu_mem_scope_info_t *ft = (sdu_mem_scope_info_t *)(&(tb->traces[(tb->write_idx+1)%tb->curr_limit]));  */
/*       ft->ev_type = etype;						 */
/*       ft->thread_id = tb->thread_id;					 */
/*       ft->ev_ts = ts;							 */
/*       ft->sdu_start_addr = sdu_addr;					 */
/*       ft->class_type = class_type;					 */
/*       tb->write_idx = (tb->write_idx+sizeof(sdu_mem_scope_info_t))%tb->curr_limit;  */
/*       printf("log event sdum mem scope %d %d %p\n", etype, class_type, sdu_addr);  */
/*     } else {								 */
/*       tb->num_faults++;							 */
/*       TRACE1(4, "Thread Id: %d\n", tb->thread_id);			 */
/*       assert(0);							 */
/*     } */
/*   } */
/* } */

#define LOG_EVENT_lock_generic(ts, la, lock_type)
#define LOG_EVENT_lock_trying(ts, la)
#define LOG_EVENT_lock_acquired(ts, la)
#define LOG_EVENT_lock_released(ts, la)
#define LOG_EVENT_sdu_len(ts, cid, qlen)
#define LOG_EVENT_sdu_allocated(ts, sdu_addr, class_type)
#define LOG_EVENT_sdu_freed(ts, sdu_addr, class_type)

/*
#define LOG_EVENT_lock_trying(ts, la) 	LOG_EVENT_lock_generic(ts, la, LOCK_TRYING)
#define LOG_EVENT_lock_acquired(ts, la) LOG_EVENT_lock_generic(ts, la, LOCK_ACQUIRED)
#define LOG_EVENT_lock_released(ts, la) LOG_EVENT_lock_generic(ts, la, LOCK_RELEASED)
*/
#define LOG_EVENT_frame_started(ts, fnum) LOG_EVENT_frame_generic(ts, fnum, FRAME_CREATION_START)
#define LOG_EVENT_frame_created(ts, fnum) LOG_EVENT_frame_generic(ts, fnum, FRAME_CREATION_END)

/*
#define LOG_EVENT_sdu_allocated(ts, sdu_addr, class_type) LOG_EVENT_sdu_mem_scope_generic(ts, sdu_addr, class_type, SDU_ALLOCED);
#define LOG_EVENT_sdu_freed(ts, sdu_addr, class_type) LOG_EVENT_sdu_mem_scope_generic(ts, sdu_addr, class_type, SDU_FREED);
*/
/*
static inline void LOG_EVENT_lock_trying(long long ts, pthread_mutex_t *la)
{
	LOG_EVENT_lock_generic(ts, la, LOCK_TRYING);
}

static inline void LOG_EVENT_lock_acquired(long long ts, pthread_mutex_t *la)
{
	LOG_EVENT_lock_generic(ts, la, LOCK_ACQUIRED);
}

static inline void LOG_EVENT_lock_released(long long ts, pthread_mutex_t *la)
{
	LOG_EVENT_lock_generic(ts, la, LOCK_RELEASED);
}
*/
#else
#define LOG_EVENT_lock_generic(ts, la, lock_type)
#define LOG_EVENT_lock_trying(ts, la)
#define LOG_EVENT_lock_acquired(ts, la)
#define LOG_EVENT_lock_released(ts, la)
#define LOG_EVENT_frame_started(ts, la)
#define LOG_EVENT_frame_created(ts, la)
#define LOG_EVENT_frame_generic(ts, la, event_type)
#define LOG_EVENT_sdu_len(ts, cid, qlen)
#define LOG_EVENT_sdu_allocated(ts, sdu_addr, class_type)
#define LOG_EVENT_sdu_freed(ts, sdu_addr, class_type)
#endif

#endif //_LOG_H_
