/* ----------------------------------------------------------------------------
   IBM Confidential

   IBM Wireless M2M Platform   

   (C)Copyright IBM Corp. 2008, 2009, 2010, 2011

   All Rights Reserved.

   File Name: mac_dl_cps_controller.c

   Change Activity:

   Date             Description of Change                   By
   -----------      ---------------------					--------
   3-Aug.2008       Created                                 Chen Lin

   ----------------------------------------------------------------------------
   PROLOG END TAG zYx                                                         */

#define BURST_THREADS_EQUAL_NUM_PROCS
#define NO_BIND_DISTINCT
//#define DDEBUG
//#define TRACE_LEVEL 4
#include <pthread.h>
#include <string.h>
#include <sched.h>
#include <sys/syscall.h>

#ifdef __ENCRYPT__
#include <openssl/aes.h>
u_char **key_array;
FILE *keyfile;
#endif

/** add by zzb for integration */
#include "dlmap_builder.h"

//#define DUMP_BURST_FCH


#ifdef INTEGRATION_TEST

#include "bs_cfg.h"
#include "flog.h"
#include "adapter_bs_dl_transformer_phy.h"
#include "queue_util.h"
#include "sem_util.h"
#include "rru_proc.h"
#include "initial_sensing_proc.h"
#include "prephy_proc.h"

pthread_mutex_t cbr_mutex;
pthread_cond_t cbr_cond;

#endif


/** end by zzb for integration */

#include <stdio.h>
#include <limits.h>
#include "mac_dl_cps_controller.h"
#include "memmgmt.h"
#include "util.h"
#include "log.h"
#include "debug.h"
#include "perf_log.h"
#include "thread_sync.h"
#include <errno.h>
#include "phy_params.h"
#include "mac_headermsg_builder.h"
#include "mac_ss_ranging.h"
#include "mac_ss_ranging_utm.h"

#include "sdu_cid_queue.h"

//extern void* PANF(void* paf);
extern void* BurstFunc(void* paf);
extern int param_NUM_ATTACHED_PROCS;
extern int param_BIND_PROC_INDEX;

//#define PHASE_ENABLED
#ifdef PHASE_ENABLED
extern pthread_mutex_t critical_phase_mutex;
extern pthread_cond_t critical_phase_done;
extern int not_critical_phase;
#endif


//Global Variabled to trac Scheduler exceeding processing time
struct timeval after_scheduler_execution_time;
double after_scheduler_execution_time_in_us;
double min_exceeding_frame_time=99999999;
double max_exceeding_frame_time=0;
double total_exceeding_time=0;

long long int min_exceeding_frame_number=INT_MAX;
long long int max_exceeding_frame_number=INT_MAX;
int exceeding_time_count=0;

burst_args burst_thread_arg[MAX_NUM_BURSTS];
burst_thrd_info* burst_processing_thrd=NULL;


pthread_mutex_t process_cps_thread_mutex;
pthread_mutex_t burst_arr_mutex, all_thread_mutex;
int burst_arr[MAX_NUM_BURSTS];

pthread_cond_t child_burst_threads_done, all_thread_cond;
//No of active burst threads
int total_burst_threads;

//No. of bursts in current fram
int max_valid_bursts;

//Should the burst threads be alive
int threads_alive=1;
int num_bp_threads=0;

/** add by zzb for integration */
extern dts_info * gp_int_info;
/** end by zzb for integration */

extern dts_info int_info;
extern int interference_flag; //set by UL to indicate it got Interference info this frame. So this needs to be passed back to phy now.
extern int spectrum_measure_flag;
extern pthread_mutex_t spectrum_measure_flag_lock;
extern pthread_mutex_t int_info_lock;

void init_pi_mutex(pthread_mutex_t *m)
{
	pthread_mutexattr_t attr;
	int ret;
	int protocol;

	if ((ret = pthread_mutexattr_init(&attr)) != 0) {
		FLOG_FATAL("Failed to init mutexattr: %d (%s)\n", ret, strerror(ret));
	}
	if ((ret = pthread_mutexattr_setprotocol(&attr, PTHREAD_PRIO_INHERIT)) != 0) {
		FLOG_FATAL("Can't set protocol prio inherit: %d (%s)\n", ret, strerror(ret));
	}
	if ((ret = pthread_mutexattr_getprotocol(&attr, &protocol)) != 0) {
		FLOG_FATAL("Can't get mutexattr protocol: %d (%s)\n", ret, strerror(ret));
	}
	if ((ret = pthread_mutex_init(m, &attr)) != 0) {
		FLOG_FATAL("Failed to init mutex: %d (%s)\n", ret, strerror(ret));
	}
}


void init_burst_threads(void) {

#ifdef INTEGRATION_TEST
    long long int i = 0;
#else
	int i=0;
#endif
	int rc=0;

	//initialize the mutexes and cond variables for bursts burst_thread_arg
	// and global variables
	total_burst_threads=0;
	max_valid_bursts=0;
	init_pi_mutex(&(process_cps_thread_mutex));
	init_pi_mutex(&(burst_arr_mutex));
	init_pi_mutex(&(all_thread_mutex));

	pthread_cond_init(&(child_burst_threads_done), NULL);
	pthread_cond_init(&(all_thread_cond), NULL);
#ifdef SS_TX
        num_bp_threads = 1;
#else
  #ifdef BURST_THREADS_EQUAL_NUM_PROCS
        num_bp_threads=param_NUM_ATTACHED_PROCS;
  #else
        num_bp_threads=MAX_NUM_BURSTS;
  #endif
#endif

	for(i=0; i<MAX_NUM_BURSTS; i++) {
		burst_thread_arg[i].dl_sduq=NULL;
		burst_thread_arg[i].cur_burst=NULL;
		burst_thread_arg[i].pdu_list=NULL;
		burst_thread_arg[i].le_tobe_discard=NULL;
		burst_thread_arg[i].status=0;
	}

	for(int i=0; i < MAX_NUM_BURSTS ; i++)
		burst_arr[i] = 0;

	//after initialization -- start all the threads
	//  burst_processing_thrd=(burst_thrd_info*) mac_malloc(sizeof(burst_thrd_info)*(MAX_NUM_BURSTS));
	burst_processing_thrd=(burst_thrd_info*) mac_malloc(sizeof(burst_thrd_info)*(num_bp_threads));
	for(i=0; i<num_bp_threads; i++) {

		init_pi_mutex(&(burst_processing_thrd[i].burst_mutex));
		pthread_cond_init(&(burst_processing_thrd[i].ready_to_process), NULL);
		pthread_attr_t tattr;
        rc = pthread_attr_init(&tattr);
		pthread_attr_setdetachstate(&tattr, PTHREAD_CREATE_DETACHED);
#ifdef NO_BIND_DISTINCT
		rc=pthread_create(&(burst_processing_thrd[i].burst_thrd), &tattr, BurstFunc, (void*) i);
#else
		cpu_set_t cpuset;
        CPU_ZERO(&cpuset);
        CPU_SET(i%param_NUM_ATTACHED_PROCS + param_BIND_PROC_INDEX, &cpuset);
        pthread_attr_setaffinity_np(&tattr, sizeof(cpuset), &cpuset);

		rc=pthread_create(&(burst_processing_thrd[i].burst_thrd), &tattr, BurstFunc, (void*) i);
#endif
		struct sched_param  parm;
		parm.sched_priority = 30;
		int sched_err = pthread_setschedparam((burst_processing_thrd[i].burst_thrd), SCHED_RR, &parm);
		if(sched_err ) {
			FLOG_ERROR("Unable to Spawn threads for Burst functions with SCHED_RR\n");
		}
		pthread_detach(burst_processing_thrd[i].burst_thrd);
		pthread_attr_destroy(&tattr);
		burst_processing_thrd[i].func=NOTHING;
		burst_processing_thrd[i].resume_status=0;
	}
}

int release_burst_threads()
{
	int i = 0;
	for(i=0; i<num_bp_threads; i++)
	{
		pthread_cancel(burst_processing_thrd[i].burst_thrd);
		pthread_cond_destroy(&(burst_processing_thrd[i].ready_to_process));
		pthread_mutex_destroy(&(burst_processing_thrd[i].burst_mutex));
	}
	free(burst_processing_thrd);
	pthread_mutex_destroy(&(process_cps_thread_mutex));
	pthread_mutex_destroy(&(burst_arr_mutex));
	pthread_mutex_destroy(&(all_thread_mutex));

	pthread_cond_destroy(&(child_burst_threads_done));
	pthread_cond_destroy(&(all_thread_cond));
	return 0;
}

int release_sdu_payload(logical_element* le_tobe_discard){
	logical_element* le;
	logical_element* pre_le;
	le = le_tobe_discard;
	while (le)
	{
		pre_le = le;
		le = pre_le->next;
		if(pre_le->data != NULL)
		{
			mac_sdu_free((void*) pre_le->data, pre_le->length, pre_le->blk_type);
		}
		free(pre_le);

		pre_le = NULL;
	}

	return 0;
}

int print_logical_subframe_map(logical_dl_subframe_map* subframe_map){
	logical_burst_map* cur_burst_map;
	logical_pdu_map* cur_pdu_map;
	logical_packet* cur_lp;
	logical_element* cur_le;
	int i, j, sum;
	int aggr = 0;
	if (subframe_map)
	{
		i = 0;
		cur_burst_map = subframe_map->burst_header;
		printf("*************************************************\n");
		printf("Frame Number: %ld\n", get_current_frame_number());
		while (cur_burst_map)
		{
			i++;
			j = 0;
			sum =0;
			printf("The %d burst, the bytes num is: %d \n", i, cur_burst_map->burst_bytes_num);
			cur_pdu_map = cur_burst_map->pdu_map_header;
			while (cur_pdu_map)
			{
				j++;
				// do not need to release the logical packet container which has already been released when doing the frag and pack
				if (cur_pdu_map->transport_sdu_map)
				{
					printf("    The %d  transport pdu map,connection %d, bytes num is: %d \n", j, cur_pdu_map->transport_sdu_map->cid, cur_pdu_map->transport_sdu_map->num_bytes);
					sum += cur_pdu_map->transport_sdu_map->num_bytes;
					assert(cur_pdu_map->transport_sdu_map->num_bytes>0);
				}
				int sum_mm = 0;
				if (cur_pdu_map->mac_msg_map)
				{
					cur_lp = cur_pdu_map->mac_msg_map->mac_mgmt_msg_sdu;
					while (cur_lp)
					{
						sum_mm+=cur_lp->element_head->length;
						cur_lp = cur_lp->next;
					}
					printf("    The %d  mac message pdu map,connection %d, bytes num is: %d \n", j, cur_pdu_map->cid, sum_mm);
					sum += sum_mm;
				}
				int sum_arq = 0, count_lp = 0, count_le = 0;
				if (cur_pdu_map->arq_sdu_map)
				{
					cur_lp = cur_pdu_map->arq_sdu_map->arq_retransmit_block;
					while (cur_lp)
					{
						count_lp++;
						cur_le = cur_lp->element_head;
						while (cur_le)
						{
							count_le++;
							sum_arq+=cur_le->length;
							cur_le = cur_le->next;
						}
						cur_lp = cur_lp->next;
					}
					printf("    The %d retransmitted arq block map,connection %d, bytes num is: %d \n", j, cur_pdu_map->cid, sum);
					//		    assert(sum_arq>0);
					sum += sum_arq;
				}
				cur_pdu_map = cur_pdu_map->next;
			}
			assert(sum<cur_burst_map->burst_bytes_num);
			cur_burst_map = cur_burst_map->next;
			aggr += sum;;
		}

	}
	/*
	if (get_current_frame_number() % 500 == 0) {
		printf("P&F: Num Bursts: %d Total Bytes: %d\n", subframe_map->num_bursts, aggr);
	}
	 */
	//fflush(stdout);
	return 0;
}


void* dl_processing(void* arg)
{
	int i=0;
	mac_dl_cps_args* dl_args = (mac_dl_cps_args*) arg;
	sdu_queue* dl_sduq = dl_args->dl_sduq;
	subframe_queue* dl_subframeq = dl_args->dl_subframeq;
	br_queue** br_q_list = dl_args->br_q_list;
	logical_dl_subframe_map * frame_map = NULL;
	physical_subframe* phy_subframe=NULL;
	//phy_burst* phyburst=NULL;
	phy_burst* pre_phyburst=NULL;
	//logical_packet* pdu_list=NULL;
	logical_burst_map* cur_burst=NULL;
	// pthread_cond_t* scheduler_call = dl_args->scheduler_call;
	// pthread_mutex_t* scheduler_call_lock =dl_args->scheduler_call_lock;
	struct timeval before_execution_time;
	struct timeval after_execution_time;
	double before_execution_time_in_us;
	double after_execution_time_in_us;
	double next_execution_time_in_us;
	double dl_duration;
	double time_diff;
	logical_element* le_tobe_discard=NULL;
	int first_frame = 1;


    /** add by zzb for integration */

#ifdef INTEGRATION_TEST
    if (pthread_setcanceltype (PTHREAD_CANCEL_ASYNCHRONOUS, NULL) != 0)
    {
        printf ("\n");

        return NULL;
    }

    int islotnum = 0;
    int icount = 0;
    struct ofdma_map ofdma_frame;
    struct phy_dl_slotsymbol *p_phy = NULL;
    struct queue_msg queue_obj;
    queue_obj.my_type = mac_en_id[0];

    int iresult;
    unsigned int sensing_idx = 0;
    int ret;

#endif
    /** end by zzb for integration */

	/** For MAC DL-MAC, UL_MAP, Broadcast message */
    //unsigned char* p_tmp_fch_hdr = NULL;
    //unsigned char* p_tmp_hdr = NULL;
    //unsigned int tmp_fch_len = 0;
    //unsigned int tmp_len = 0;

	create_init_trace_buffer(16384, "dl_cps_processing");

	dl_duration = frame_duration[FRAME_DURATION_CODE] * 1000;
	// first sleep the given period of time;
	usleep(dl_duration);
	int print_burst_flag = TRUE;
	while(can_sync_continue())
	{
		LOG_EVENT_perf_profile_event(0);

/** add by zzb for integration */
#ifdef INTEGRATION_TEST
            if (wait_sem (tx_s_handle) != 0 )
            {
                FLOG_ERROR ("get SEM error\n");
            }

            if (g_periodic_sensing_enable == 0)
            {
                sensing_idx = 0;
            }else
            {
                sensing_idx = (unsigned int)get_current_frame_number();
            }

            if( ( (sensing_idx != 0) && ( (sensing_idx % (g_spectrum.periodic_duration * 50)) == 0 )) ||
                (g_periodic_sensing_reset == 1) )
            {
                pthread_mutex_lock(&spectrum_measure_flag_lock);
                spectrum_measure_flag = 1;
                pthread_mutex_unlock(&spectrum_measure_flag_lock);

                if (g_periodic_sensing_reset == 1)
                {
                    g_periodic_sensing_drop = 1;
                    g_periodic_sensing_reset = 0;
                }
            }

#endif
/** end by zzb for integration */

#ifdef PHASE_ENABLED
		pthread_mutex_lock(&critical_phase_mutex);
		not_critical_phase=0;
		pthread_mutex_unlock(&critical_phase_mutex);
#endif

		// waiting for some signal
		// pthread_mutex_lock(scheduler_call_lock);
		// pthread_cond_wait(scheduler_call, scheduler_call_lock);
#ifndef INTEGRATION_TEST
		gettimeofday (&before_execution_time, NULL);
		before_execution_time_in_us = (double) before_execution_time.tv_sec * 1000000 + before_execution_time.tv_usec;
#endif
		// printf("current time is %lf \n",  before_execution_time_in_us);
		if (first_frame){
#ifdef INTEGRATION_TEST
                    pthread_mutex_lock(&cbr_mutex);
                    pthread_cond_signal(&cbr_cond);
                    pthread_mutex_unlock(&cbr_mutex);
#else
			next_execution_time_in_us = before_execution_time_in_us + dl_duration;
#endif
			first_frame = 0;
		}
		else
		{
#ifndef INTEGRATION_TEST
			next_execution_time_in_us += dl_duration;
#endif
		}

		LOG_EVENT_frame_generic(readtsc(),get_current_frame_number(),FRAME_CREATION_START);
		// initialize the frame_map that generated last time
		// For SS side, In frame_map, only the logical_burst_map will be populated.
		frame_map=(logical_dl_subframe_map*)mac_malloc(sizeof(logical_dl_subframe_map));

		// Initialize the Dl subframe map
		dl_subframe_map_init(frame_map);

		LOG_EVENT_perf_profile_event(2);
		PLOG(PLOG_FRAME_LATENCY, METHOD_BEGIN);
		PLOG(PLOG_DL_SCHEDULER, METHOD_BEGIN);
pthread_mutex_lock(&int_info_lock);
/** add by zzb for integration */
        if ( (interference_flag == 1) && (gp_int_info != NULL) )
        {
            int_info.num_dl_interference = gp_int_info->num_dl_interference;
            int_info.num_ul_interference = gp_int_info->num_ul_interference;
            if (gp_int_info->is_active != NULL)
            {
                memcpy (int_info.is_active, gp_int_info->is_active,
                        sizeof ( int_info.is_active ));
            }

        }

        int num_dl_subch = NUM_DL_SUBCHANNELS - int_info.num_dl_interference;
        int num_ul_subch = NUM_UL_SUBCHANNELS - int_info.num_ul_interference;

/** end by zzb for integration */
pthread_mutex_unlock(&int_info_lock);
		pthread_rwlock_rdlock(&conn_info_rw_lock);
		//printf("NDI %d NUI %d\n",int_info.num_dl_interference, int_info.num_ul_interference);
		bs_scheduling(dl_sduq,  br_q_list, frame_map, num_dl_subch,num_ul_subch);
		

		PLOG(PLOG_DL_SCHEDULER, METHOD_END);
		LOG_EVENT_perf_profile_event(3);

		/** add by zzb for integration */

#ifndef SS_TX
#ifndef INTEGRATION_TEST
		print_dlmap (frame_map->dl_map);
#endif
		frame_map->fch->p_dts_info = &int_info;

#endif

		/** end by zzb for integration */

		// initialize the physical subframe
		initialize_subframe(&phy_subframe);
		phy_subframe->bursts_num = frame_map->num_bursts;
		phy_subframe->frame_num = (unsigned int)get_current_frame_number();

#ifdef DUMP_BURST_FCH
    print_dlmap (frame_map->dl_map);
    print_logical_subframe_map(frame_map);

    phy_subframe->frame_num = 0;
    frame_map->dl_map->frame_number = 0;
#endif

	    /** Building FCH & DL_MAP */
        /* Need free together with the phy_subframe! */
	phy_subframe->interference_info = NULL;
#ifndef SS_TX
        phy_subframe->fch_dl_map =
            malloc (MAX_FCH_LEN +
                       (frame_map->fch->dl_map_length) * BYTES_PER_SLOT);

        if (phy_subframe->fch_dl_map == NULL)
        {
            FLOG_FATAL("malloc failed\n");
        }

        memset (phy_subframe->fch_dl_map, 0, MAX_FCH_LEN +
                       (frame_map->fch->dl_map_length) * BYTES_PER_SLOT);

        /** add by zzb for integration */
                if (frame_map->burst_header != NULL)
                {
                    phy_subframe->raw_ul_map = malloc (
                            frame_map->burst_header->burst_bytes_num);

                    if (phy_subframe->raw_ul_map == NULL)
                    {
                        ERROR_TRACE ("malloc failed\n");
                    }

                    memset(phy_subframe->raw_ul_map, 0, frame_map->burst_header->burst_bytes_num);
                }
                else
                {
                    ERROR_TRACE ("No UL_MAP burst found in frame map");
                }

        /** end by zzb for integration */
	
        pthread_mutex_lock(&spectrum_measure_flag_lock);
	if (spectrum_measure_flag == 1)
	{
		phy_subframe->sense_flag = 1;
		//phy_subframe->interference_info = NULL;
		spectrum_measure_flag = 0;
	}
	else
	{
		phy_subframe->sense_flag = 0;
		//phy_subframe->interference_info = &int_info;
	}
        pthread_mutex_unlock(&spectrum_measure_flag_lock);

/** add by zzb for integration */

        pthread_mutex_lock(&int_info_lock);

	if (interference_flag == 1)
	{
		interference_flag = 0;
		phy_subframe->interference_info = gp_int_info;
	}
	else
	{
		phy_subframe->interference_info = NULL;
	}

        pthread_mutex_unlock(&int_info_lock);	

/** end by zzb for integration */

		// Build_frame_msg takes the frame_map->dlmap and frame_map->fch and 
		// populate the phy_subframe->fch_dl_map. The Frame_map->ulmap is taken 
		// and appended as the last burst in the frame_map->logical_burst_map. After 
		// this, concatenate function will take the logical bursts and prepare
		// the byte sequence in phy_subframe->burst_header->paylaod
	

        

	if (build_frame_msg( frame_map, phy_subframe ) != 0)
        {
            FLOG_ERROR("In dl_processing: build frame Messages error\n");
        }
#ifndef INTEGRATION_TEST
        print_logical_subframe_map(frame_map);
#endif

#else
	#ifdef SS_RX 
		#ifdef RANGING_ENABLED
		//ranging module works properly only if compiled as a BS instance/SS instance
		// not for BS-TX+SS-RX or SS-TX+BS-RX loopback test mode
		phy_subframe->p_ranging_info = malloc(sizeof(ranging_info));
		memset(phy_subframe->p_ranging_info, 0, sizeof(ranging_info));
		set_rng_info(phy_subframe->p_ranging_info);
/*		#ifdef RANGING_TEST
		printf("Done setting p_ranging_info for frame %d, ms_ranging_flag: %d\n", frame_number, phy_subframe->p_ranging_info->ms_ranging_flag);
		if((phy_subframe->p_ranging_info->num_ranging_adjust_ie == 1) && (phy_subframe->p_ranging_info->p_ranging_ie_list != NULL))
		{
			printf("Corrections in timing: %d, power: %d, frequency: %d\n", \
			phy_subframe->p_ranging_info->p_ranging_ie_list->timing_adjust, \
			phy_subframe->p_ranging_info->p_ranging_ie_list->power_adjust, \
			phy_subframe->p_ranging_info->p_ranging_ie_list->frequency_adjust); 
		}
		#endif*/
		#endif
	#endif
#endif

		pre_phyburst = NULL;
		cur_burst = frame_map->burst_header;
		le_tobe_discard = NULL;
		// generate the physical frame

		FLOG_DEBUG("MULTI: STARTING CPS --total burst %d: \n", total_burst_threads);
		PLOG(PLOG_DL_BURSTPROC, METHOD_BEGIN);
		// lock cps thread mutex
		pthread_mutex_lock(&(process_cps_thread_mutex));
#ifdef SS_TX
        num_bp_threads = 1;
#else
  #ifdef BURST_THREADS_EQUAL_NUM_PROCS
        num_bp_threads=param_NUM_ATTACHED_PROCS;
  #else
        num_bp_threads=frame_map->num_bursts;
  #endif
#endif

		max_valid_bursts=frame_map->num_bursts;

		// unlock cps thread mutex
		//pthread_mutex_unlock(&(process_cps_thread_mutex));

		FLOG_DEBUG("MULTI: STARTING FRAG PACK --total burst %d: \n", total_burst_threads);
		assert(MAX_NUM_BURSTS >= frame_map->num_bursts);
		for(i=0;  i<frame_map->num_bursts; i++) {
			// initialize the thread args
			burst_thread_arg[i].dl_sduq=dl_sduq;
			burst_thread_arg[i].cur_burst=cur_burst;
			burst_thread_arg[i].pdu_list=NULL;
			burst_thread_arg[i].le_tobe_discard=NULL;
			burst_thread_arg[i].status=-1;
			cur_burst = cur_burst->next;
		}

		//LOG_EVENT_frame_generic(readtsc(),get_current_frame_number(),FRAME_CONC_START);
		//Invoke the burst threads
		total_burst_threads = num_bp_threads;
		pthread_mutex_lock(&burst_arr_mutex);
		for(int idx =0; idx < num_bp_threads; idx++)
			burst_arr[idx] = 0;
		pthread_mutex_unlock(&burst_arr_mutex);

		for(i=0; i<num_bp_threads;i++) {
			pthread_mutex_lock(&(burst_processing_thrd[i].burst_mutex));
			burst_processing_thrd[i].func=PACK_AND_FRAG;
			burst_processing_thrd[i].resume_status=-1;

			//pthread_cond_broadcast(&(burst_processing_thrd[i].ready_to_process));
			pthread_mutex_unlock(&(burst_processing_thrd[i].burst_mutex));
			FLOG_DEBUG("MULTI: signal to process p&f --total burst: %d \n", total_burst_threads);
		}

		pthread_mutex_lock(&all_thread_mutex);
			LOG_EVENT_perf_profile_event(4);
			// signal child burst threads
			pthread_cond_broadcast(&all_thread_cond);
		pthread_mutex_unlock(&all_thread_mutex);

		while(total_burst_threads>0) {
			// wait for signal from last completing child
			pthread_cond_wait(&child_burst_threads_done, &(process_cps_thread_mutex));
		}

		//LOG_EVENT_frame_generic(readtsc(),get_current_frame_number(),FRAME_CONC_END);
		// unlock cps thread mutex
		pthread_mutex_unlock(&(process_cps_thread_mutex));
		pthread_rwlock_unlock(&conn_info_rw_lock);
		PLOG(PLOG_DL_BURSTPROC, METHOD_END);
		PLOG(PLOG_FRAME_LATENCY, METHOD_END);

		FLOG_DEBUG("MULTI: DONE FRAG PACK --total burst: %d\n", total_burst_threads);
#ifdef PHASE_ENABLED
		pthread_mutex_lock(&critical_phase_mutex);
		not_critical_phase=1;
		pthread_cond_broadcast(&critical_phase_done);
		pthread_mutex_unlock(&critical_phase_mutex);
#endif

		//void* status;
		//int rc=0;

		LOG_EVENT_perf_profile_event(5);
		cur_burst = frame_map->burst_header;
		//printf("P&F: Frame No: %d Num Bursts: %d\n", get_current_frame_number(), frame_map->num_bursts);
		fflush(stdout);
		for (i=0; i<frame_map->num_bursts; i++)
		{
			if(!burst_thread_arg[i].status) {
				/* 	    phyburst = (phy_burst *) malloc(sizeof(phy_burst)); */
				/* 	    phyburst->length = cur_burst->burst_bytes_num; */
				/* 	    phyburst->map_burst_index= cur_burst->map_burst_index; */
				/* 	    phyburst->burst_payload = (u_char *) malloc(phyburst->length); */
				/* 	    if (pdu_list_arr[i] == NULL) */
				/* 	      phyburst->length = 0; */
				/* 	    // concatenation */
				/* 	    concatenation(pdu_list_arr[i], phyburst->burst_payload, phyburst->length); */
				if (pre_phyburst == NULL){
					phy_subframe->burst_header = burst_thread_arg[i].phyburst;
					pre_phyburst = burst_thread_arg[i].phyburst;
				}
				else {
					pre_phyburst->next = burst_thread_arg[i].phyburst;
					pre_phyburst = burst_thread_arg[i].phyburst;
				}
				cur_burst = cur_burst->next;
			}
		}

		for (i=0; i<frame_map->num_bursts; i++) {
			release_logical_pdu_list(burst_thread_arg[i].pdu_list);
			release_sdu_payload(burst_thread_arg[i].le_tobe_discard);
		}

		if (pre_phyburst != NULL)
		{
			pre_phyburst->next = NULL;
		}
#ifdef PRINT_DATA
	if (print_burst_flag)
	{
	FILE *fp;
	int j = 0, kk = 0, val = 0;
	phy_burst *tb = phy_subframe->burst_header;
	char file_name[20];

	fp = fopen("fch_dlmap", "w");
	for(j = 0; j < phy_subframe->fch_dl_map_len; j++)
	{
		val = *((u_char*)(phy_subframe->fch_dl_map) + j);
		for(kk = 7; kk >= 0; kk--)
		{
			fprintf(fp, "%d\n", (val>>kk)&1);
		} 
	}
	fclose(fp);

	for(i = 0; (i < phy_subframe->bursts_num) && (tb != NULL); i++)
	{
		sprintf(file_name, "%d.burst", i);
		fp = fopen(file_name, "w");
        	if (!fp)
        	{
          		FLOG_FATAL("Error opening file %s\n", file_name);
        	}
        	else
        	{
          		FLOG_INFO("Opened file %s\n", file_name);
        	}
		for(j = 0; j < tb->length; j++)
		{
			val = tb->burst_payload[j];
			for(kk = 7; kk >= 0; kk--)
			{
				fprintf(fp, "%d\n", (val>>kk)&1);
			} 
		}
		tb = tb->next;
		fclose(fp);
	} 

	// Print only the bursts in the first frame. Unset print_burst_flag after
	// that. this is because diff. frames will have diff number of bursts and since
	// burst files are overwritten, we can have files containing a set of bursts 
	// from diff frame. Alternately, delete *.burst beore printing each time
	print_burst_flag = FALSE;
	}
#endif

#ifdef DUMP_BURST_FCH

FILE * ul_burst = NULL;
FILE * fch_dlmap = NULL;

ul_burst = fopen("DL_BURST.dump", "wb+");
fch_dlmap = fopen("FCH_DLMAP.dump", "wb+");

fprintf(ul_burst, "-------- start frame No %d dump --------\n", phy_subframe->frame_num);
fprintf(ul_burst, "-------- burst in total %d--------\n", phy_subframe->bursts_num);

int i;
//printf("in DL cps, fch_dl_map_len: %d\n", phy_subframe->fch_dl_map_len);
fprintf(fch_dlmap, "-------- start frame No %d dump --------\n", phy_subframe->frame_num);

for (i=0; i<phy_subframe->fch_dl_map_len; i++)
{
     //   fprintf(ul_burst, "-------- start frame No %d dump --------\n", phy_subframe->frame_num);
                fprintf(fch_dlmap, "%d\n", ((*((unsigned char *)(phy_subframe->fch_dl_map +i)) >> 7 ) & 0x01));
         //fprintf(fch_dlmap, "%d\n", (((unsigned char *)phy_subframe->fch_dl_map)[i]) >> 7 ) & 0x01))
                fprintf(fch_dlmap, "%d\n", ((*((unsigned char *)(phy_subframe->fch_dl_map +i)) >> 6 ) & 0x01));
                fprintf(fch_dlmap, "%d\n", ((*((unsigned char *)(phy_subframe->fch_dl_map +i)) >> 5 ) & 0x01));
                fprintf(fch_dlmap, "%d\n", ((*((unsigned char *)(phy_subframe->fch_dl_map +i)) >> 4 ) & 0x01));

                fprintf(fch_dlmap, "%d\n", ((*((unsigned char *)(phy_subframe->fch_dl_map +i)) >> 3 ) & 0x01));
                fprintf(fch_dlmap, "%d\n", ((*((unsigned char *)(phy_subframe->fch_dl_map +i)) >> 2 ) & 0x01));
                fprintf(fch_dlmap, "%d\n", ((*((unsigned char *)(phy_subframe->fch_dl_map +i)) >> 1 ) & 0x01));
                fprintf(fch_dlmap, "%d\n", ((*((unsigned char *)(phy_subframe->fch_dl_map +i))) & 0x01));
}

phy_burst * p_tmp_burst = phy_subframe->burst_header;


while (p_tmp_burst != NULL)
{
    fprintf(ul_burst, "-------- burst idx %d, length %d --------\n", p_tmp_burst->map_burst_index, p_tmp_burst->length);

    for (i = 0; i < p_tmp_burst->length; i++)
    {
        fprintf(ul_burst, "%d\n", ( ( p_tmp_burst->burst_payload[i] ) >> 7) & 0x01);
        fprintf(ul_burst, "%d\n", ( ( p_tmp_burst->burst_payload[i] ) >> 6) & 0x01);
        fprintf(ul_burst, "%d\n", ( ( p_tmp_burst->burst_payload[i] ) >> 5) & 0x01);
        fprintf(ul_burst, "%d\n", ( ( p_tmp_burst->burst_payload[i] ) >> 4) & 0x01);

        fprintf(ul_burst, "%d\n", ( ( p_tmp_burst->burst_payload[i] ) >> 3) & 0x01);
        fprintf(ul_burst, "%d\n", ( ( p_tmp_burst->burst_payload[i] ) >> 2) & 0x01);
        fprintf(ul_burst, "%d\n", ( ( p_tmp_burst->burst_payload[i] ) >> 1) & 0x01);
        fprintf(ul_burst, "%d\n", ( ( p_tmp_burst->burst_payload[i] ) ) & 0x01);
    }

    p_tmp_burst = p_tmp_burst->next;
}

fclose(fch_dlmap);
fclose(ul_burst);

phy_subframe->frame_num = (unsigned int)get_current_frame_number();
frame_map->dl_map->frame_number = (unsigned int)get_current_frame_number();

#endif

		/** add by zzb for integration */

#ifdef INTEGRATION_TEST
		        iresult = phy_subframe_send_transform_dl (phy_subframe, &ofdma_frame,
		                frame_map->dl_map);

		        if (iresult == -1)
		        {
		            FLOG_ERROR ("Transform error\n");
		        }

		        for (islotnum = 0; islotnum < ofdma_frame.slot_num; islotnum++)
		        {
		            adapter_transform_symbolslot_1 (&ofdma_frame, islotnum,frame_map->dl_map->frame_number,
		                    &p_phy);

		            if (islotnum + 1 == ofdma_frame.slot_num)
		                p_phy->dl_subframe_end_flag = 1;
		            //newobj->buf = p_phy;
		            queue_obj.p_buf = p_phy;
		            p_phy = NULL;
/*
		            dump_slot_symbol_data(fp1,queue_obj->p_buf,itimes++);
		            dump_phy_dl_slot_bench(fp2,queue_obj->p_buf);
*/

		            if ( wmrt_enqueue( mac_en_id[0], &queue_obj,
		                sizeof(struct queue_msg)) == -1)
		            {
		                FLOG_ERROR("enqueue error\n");
		            }
		        }
		/** end by zzb for integration */
		#endif
#ifndef INTEGRATION_TEST
		enqueue_subframe(dl_subframeq, phy_subframe);
#endif

#ifndef SS_TX
// At the SS, UCD, DCD, ULMAP, DLMAP are received and decoded in the Rx (SS-UL) chain, 
// unlike the BS where these are generated in the DL. Hence calls to these functions will be in SS UL
		// save the dlmap and ulmap 
		set_dl_map_msg(frame_map->dl_map, get_current_frame_number());
		set_ul_map_msg(frame_map->ul_map, get_current_frame_number());
#endif

		//free_dl_subframe_map(frame_map);

		// release the logical subframe map
		if (frame_map)
		{
			release_logical_subframe_map(frame_map);
		}
		//        pthread_mutex_unlock (scheduler_call_lock);

		LOG_EVENT_frame_generic(readtsc(),get_current_frame_number(),FRAME_CREATION_END);
		LOG_EVENT_perf_profile_event(1);
/** add by zzb for integration */

#ifdef INTEGRATION_TEST
        release_subframe(phy_subframe);

        pthread_mutex_lock(&int_info_lock);

        if (gp_int_info != NULL)
        {
            free (gp_int_info);
            gp_int_info = NULL;
        }

        pthread_mutex_unlock(&int_info_lock);
#else
		gettimeofday (&after_execution_time, NULL);
		after_execution_time_in_us = (double) after_execution_time.tv_sec * 1000000 + after_execution_time.tv_usec;
		time_diff = next_execution_time_in_us - after_execution_time_in_us;

		if (time_diff > 0)
		{
			usleep(time_diff);
		}
#endif
/** end by zzb for integration */
		update_current_frame_number(1);
		if (get_current_frame_number() < 0)
		{
		    set_current_frame_number(0);
		}
	}
	threads_alive = 0;
	FLOG_INFO("Terminated DL cps thread\n");fflush(stdout);
	release_burst_threads();
	pthread_exit((void*)0);
	return NULL;
}

int dl_cps_controller(br_queue **br_q_list){
	subframe_queue* dl_subframeq;
	int ret;
	//pthread_cond_t* scheduler_cond;
	//pthread_mutex_t* scheduler_mutex;
	// begin the initialization of dl send socket

	// obtain the dl sduq
	sdu_queue* dl_sduq;
	get_sduq(&dl_sduq, 1);

#ifndef INTEGRATION_TEST
	// initialize the downlink physical subframe queue
	initialize_subframe_queue(&dl_subframeq, 1);
#endif
	//get_timer_schedulertrigger_cond(&scheduler_cond, &scheduler_mutex);

	// start the downlink processing thread
	dl_arg.dl_sduq = dl_sduq;
	dl_arg.dl_subframeq = dl_subframeq;
	dl_arg.br_q_list = br_q_list;
	// dl_arg.scheduler_call = scheduler_cond;
	// dl_arg.scheduler_call_lock = scheduler_mutex;

	pthread_attr_t tattr;
    pthread_attr_init(&tattr);
    pthread_attr_setdetachstate(&tattr, PTHREAD_CREATE_JOINABLE);
	ret = pthread_create(&dl_processing_thread, &tattr, dl_processing, (void *) &dl_arg);
	if(ret)
	{
        FLOG_FATAL("Cannot create thread to start downlink processing");
	}
	pthread_attr_destroy(&tattr);
	return 0;
}

int release_dl_cps_controller(){
	//pthread_cancel(dl_processing_thread);
#ifdef INTEGRATION_TEST
//    pthread_cancel (dl_processing_thread);
    pthread_join (dl_processing_thread, NULL);
#endif

#ifndef INTEGRATION_TEST
	release_subframe_queue(dl_arg.dl_subframeq, 1);
#endif
	return 0;
}

/*void* PANF(void* paf_args) {
  pack_frag_args* paf=(pack_frag_args*)paf_args;
  fragpack(paf->dl_sduq, paf->cur_burst, &(paf->pdu_list), &(paf->le_tobe_discard), &(paf->status));
  pthread_exit((void*)0);
  }*/
int get_resume_status(burst_thrd_info* b_p_t ) {
	int res_val;
	pthread_mutex_t *pmut = &(b_p_t->burst_mutex);
	pthread_mutex_lock(pmut);
	res_val = b_p_t->resume_status;
	pthread_mutex_unlock(pmut);
	return res_val;
}
void* BurstFunc(void* b_args) {
    if (pthread_setcanceltype (PTHREAD_CANCEL_DEFERRED, NULL) != 0)
    {
        return NULL;
    }
#ifdef INTEGRATION_TEST
	long long int myid=(long long int)b_args;
#else
        int myid=(int)b_args;
#endif
//	cpu_set_t cpuset;
	int my_old_id = myid;
	int continuing = 0;
//	CPU_ZERO(&cpuset);

/*
#ifdef __ENCRYPT__
//make this a file input
char key[KEYLEN+1]={102,102,102,102,102,102,102,102,102,102,102,102,102,102,102,102,'\0'};
#endif
*/

	/* create trace buffer */
/*
    create_init_trace_buffer(65536, "burst thread");

	pthread_getaffinity_np ( burst_processing_thrd[myid].burst_thrd, sizeof(cpuset),
			&cpuset);
	for (int cpu = 0; cpu < 8 ; cpu++) {
		if (CPU_ISSET(cpu, &cpuset)) {
			FLOG_INFO("Thread %ld child %d has affinity to CPU %d\n", syscall(SYS_gettid), myid, cpu);
		}
	}
*/
	int preempted = 0;

	while(threads_alive) {

		burst_thrd_info* b_p_t=&(burst_processing_thrd[myid]);

		LOG_EVENT_burst_profile_event(0, myid, my_old_id);

		// lock burst mutex
		// get_resume_status checks if my own work is finished & second check
		// is to see i have not been preempted
		while( (get_resume_status(b_p_t) != -1)|| preempted ) {
			LOG_EVENT_burst_profile_event(14, myid, my_old_id);

			// wait for signal from main cps thread
			pthread_mutex_lock(&all_thread_mutex);
			pthread_cond_wait (&all_thread_cond, &all_thread_mutex);
			LOG_EVENT_burst_profile_event(11, myid, my_old_id);
			LOG_EVENT_perf_profile_event(11);
			pthread_mutex_unlock(&all_thread_mutex);
			LOG_EVENT_burst_profile_event(1, myid, my_old_id);
			preempted = 0;
		}
		LOG_EVENT_burst_profile_event(12, myid, my_old_id);

		// do the check for itself
		int cont_flag = 0;
		pthread_mutex_lock(&burst_arr_mutex);
			if(!continuing) {
				if(burst_arr[myid]) {
					cont_flag = 1;
					preempted = 1;
				}
				else
					burst_arr[myid] = 1;
			}
		pthread_mutex_unlock(&burst_arr_mutex);
		if(cont_flag)
			continue;
		LOG_EVENT_burst_profile_event(13, myid, my_old_id);

		// trace
		FLOG_DEBUG("MULTI: start processing p&f --total burst: %d\n", total_burst_threads);
#ifdef SS_TX
        num_bp_threads = 1;
#else
        #ifdef BURST_THREADS_EQUAL_NUM_PROCS
                num_bp_threads=param_NUM_ATTACHED_PROCS;
        #else
                num_bp_threads=max_valid_bursts;
        #endif
#endif

		int i=0;

		for(i=0; i<max_valid_bursts; i++) {
			if((i%(num_bp_threads))==myid) {
				//this burst is my responsibility to process
				//do processing
				burst_args* ba=&(burst_thread_arg[i]);

				if(b_p_t->func==PACK_AND_FRAG) {
					fragpack(ba->dl_sduq, ba->cur_burst, &(ba->pdu_list), &(ba->le_tobe_discard), &(ba->status));
					if(!ba->status) {
						ba->phyburst = (phy_burst *) malloc(sizeof(phy_burst));
						ba->phyburst->length = ba->cur_burst->burst_bytes_num;
						ba->phyburst->map_burst_index= ba->cur_burst->map_burst_index;
						ba->phyburst->burst_payload = (u_char *) malloc(ba->phyburst->length);
						if (ba->phyburst->length >0) memset(ba->phyburst->burst_payload, 0, ba->phyburst->length);

						pthread_mutex_lock(&all_thread_mutex);
                        concatenation(ba->pdu_list, ba->phyburst->burst_payload, ba->phyburst->length);
						pthread_mutex_unlock(&all_thread_mutex);
					}
				}
				else {
					//do nothing
				}
			}
		}
		LOG_EVENT_burst_profile_event(2, myid, my_old_id);

		pthread_mutex_lock(&(b_p_t->burst_mutex));
		b_p_t->resume_status=0;
		pthread_mutex_unlock(&(b_p_t->burst_mutex));
		// lock cps thread mutex
		pthread_mutex_lock(&(process_cps_thread_mutex));
		// signal cps thread
		total_burst_threads--;
		FLOG_DEBUG("MULTI: finished processing p&f --total burst: %d\n", total_burst_threads);
		if(total_burst_threads==0) {
			FLOG_DEBUG("MULTI: all threads done p&f: %d\n", total_burst_threads);
			pthread_cond_signal(&child_burst_threads_done);
			LOG_EVENT_burst_profile_event(5, myid, my_old_id);
		}
		// unlock cps thread
		pthread_mutex_unlock(&(process_cps_thread_mutex));
		LOG_EVENT_burst_profile_event(3, myid, my_old_id);

		// unlock burst mutex
		LOG_EVENT_burst_profile_event(4, myid, my_old_id);

		// check for more work

		pthread_mutex_lock(&burst_arr_mutex);
		int flag = 0;
		for(int i=0; i < num_bp_threads; i++) {
			if(burst_arr[i] == 0) {
				continuing = 1;
				myid = i;
				flag = 1;
				burst_arr[i] = 1;
				LOG_EVENT_burst_profile_event(70, myid, my_old_id);
				break;
			}
		}
		if(! flag) {
			myid = my_old_id;
			continuing = 0;
		}
		pthread_mutex_unlock(&burst_arr_mutex);

	}
	FLOG_INFO("returning from BurstFunc\n");
	pthread_exit((void*)0);
}


