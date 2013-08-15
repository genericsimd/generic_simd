/* ----------------------------------------------------------------------------
   IBM Confidential

   IBM Wireless M2M Platform   

   (C)Copyright IBM Corp. 2008, 2009, 2010, 2011

   All Rights Reserved.

   File Name: logreader.c

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
#include <pthread.h>
#include <sched.h>
#include <syscall.h>
//#include "memmgmt.h"
#include "log.h"
#include "util.h"
#include "debug.h"

#define WiMAX_mac_malloc malloc
#define WiMAX_mac_free   free

#define LOG_FILE "./logs.out"

char log_file[256]; // to be replaced by MAX_PATH_LEN?
char meta_file[256];

extern int param_NUM_ATTACHED_PROCS; // No. of processors that we want to bind our threads to
extern int g_sys_procs;
extern int sync_flag;

#ifdef LOG_METRICS
pthread_t	log_reader;
void *read_logs(void *arg)
{
    if (pthread_setcanceltype (PTHREAD_CANCEL_ASYNCHRONOUS, NULL) != 0)
    {
        return NULL;
    }
	char trace_line[256];
	op_status_t ret_val = SUCCESS;
	int num_buffers, prev_num_buffers;
	int num_mutexes, prev_num_mutexes;
	cpu_set_t cpuset;
	/*
	int bind_to_proc = min(g_sys_procs-1, param_NUM_ATTACHED_PROCS);


	CPU_ZERO(&cpuset);
	CPU_SET(bind_to_proc, &cpuset);
	sched_setaffinity(syscall(SYS_gettid), sizeof(cpuset), &cpuset);
	pthread_setaffinity_np (log_reader, sizeof(cpuset), &cpuset);

	CPU_ZERO(&cpuset);
	*/
	sched_getaffinity(syscall(SYS_gettid), sizeof(cpuset), &cpuset);
	for (int cpu = 0; cpu < 8; cpu++) {
		if (CPU_ISSET(cpu, &cpuset)) {
		    FLOG_INFO("log reader has affinity to processor %d\n", cpu);		
		}
	}

	FILE *fp = fopen(log_file, "w");
	FILE *meta_fp = fopen(meta_file, "w");

	prev_num_buffers = prev_num_mutexes = 0;

	do {
		num_buffers = NUM_TRACE_BUFFERS;
		num_mutexes = NUM_TRACED_MUTEXES;

		for (int i = prev_num_buffers; i < num_buffers; i++) {
			trace_buffer_t *tb = (trace_buffer_t *)TRACE_BUFFER(i);	
			if (tb != NULL) {
				sprintf(trace_line, "TRACE BUFF: Thread ID: %d Desc: %s\n", tb->thread_id, tb->thread_desc);
				fputs(trace_line, meta_fp);
				fflush(meta_fp);
			}
		}

		for (int i = prev_num_mutexes; i < num_mutexes; i++) {
				sprintf(trace_line, "TRACE RECORD: Mutex Address: %p Desc: %s\n", all_trace_buffers.mdescs[i].ma, 
																				  all_trace_buffers.mdescs[i].mdesc);
				fputs(trace_line, meta_fp);
				fflush(meta_fp);
		}

		for (int i = 0; i < NUM_TRACE_BUFFERS; i++) {
			trace_buffer_t *tb = (trace_buffer_t *)TRACE_BUFFER(i);	
			if (NULL == tb || tb->read_idx == tb->write_idx /*(mod(tb->write_idx-tb->read_idx, tb->curr_limit) < 512)*/) {
				continue;
			}
			assert (tb != NULL);
			do {
				//ret_val = read_trace_entry(tb, trace_line);
				void *tis;
				//trace_event_type_t et;
				size_t num_bytes;
				int new_read_idx;
				//ret_val = read_trace_info_struct(tb, &tis, &et);
				ret_val = read_trace_info_bulk (tb, &tis, &num_bytes, &new_read_idx);
				if (ret_val != SUCCESS) {
					break;
				}
				tb->read_idx = new_read_idx;
				//fputs(trace_line, fp);
				/*
				switch(et) {
					case LOCK_TRYING:
					case LOCK_ACQUIRED:
					case LOCK_RELEASED:
						fwrite(tis, sizeof(lock_trace_info_t), 1, fp);
						break;
					case SDU_QUEUE_LENGTH:
						fwrite(tis, sizeof(sdu_q_trace_info_t), 1, fp);
						break;
					case FRAME_CREATION_START:
					case FRAME_CREATION_END:
						fwrite(tis, sizeof(frame_trace_info_t), 1, fp);
						break;
					default:
						break;
					}
				*/
				fwrite((char *)tis, sizeof(char), num_bytes, fp);
				break;
			}  while(1);
		}
		prev_num_mutexes = num_mutexes;
		prev_num_buffers = num_buffers;
	}while (sync_flag == 1);
	fclose(fp);
	fclose(meta_fp);
	return NULL;
}

void start_log_reader(char *lfile)
{
	int ret_val;
	pthread_attr_t tattr;
	/*
	pthread_t tid;
	int newprio;
	struct sched_param parm;
	int spolicy;
	*/
//	int bind_to_proc = min(g_sys_procs-1, param_NUM_ATTACHED_PROCS);
	int bind_to_proc = g_sys_procs-1;

	cpu_set_t cpuset;

	strcpy(log_file, lfile);

	strcpy(meta_file, log_file);
	char *fname_beg = strrchr(meta_file, '/');
	char *prefix = "meta.";
	int fnlen;
	if (NULL != fname_beg) {
		fname_beg++;
		fnlen = strlen(fname_beg);
	} else {
		fnlen = strlen(meta_file);
		fname_beg = meta_file;
	}
	memmove(fname_beg+strlen(prefix), fname_beg, fnlen);
	memcpy(fname_beg, prefix, strlen(prefix));
	*(fname_beg+strlen(prefix)+fnlen)='\0';

	ret_val = pthread_attr_init(&tattr);

	//ret_val = pthread_attr_getschedpolicy (&tattr, &spolicy);
	//spolicy = SCHED_RR;
	//pthread_attr_setschedpolicy (&tattr, spolicy);

	//ret_val = pthread_attr_getschedpolicy (&tattr, &spolicy);
	//ret_val = pthread_attr_getschedparam (&tattr, &parm);

	//newprio = sched_get_priority_max(spolicy);
	//parm.sched_priority = newprio;


	//ret_val = pthread_attr_setschedparam (&tattr, &parm);
	CPU_ZERO(&cpuset);
	CPU_SET(bind_to_proc, &cpuset);
	pthread_attr_setaffinity_np(&tattr, sizeof(cpuset), &cpuset);
	ret_val = pthread_create(&log_reader, &tattr, read_logs, NULL);
	assert (ret_val == 0);

//	ret_val = pthread_setschedprio(log_reader, 20);
//	assert(ret_val == 0);
	pthread_attr_destroy(&tattr);
}

void release_log_reader()
{
	pthread_cancel(log_reader);
}
#else
#define read_logs(arg) ;
#endif
