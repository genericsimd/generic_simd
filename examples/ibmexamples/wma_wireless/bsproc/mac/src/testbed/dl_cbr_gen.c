/* ----------------------------------------------------------------------------
   IBM Confidential

   IBM Wireless M2M Platform   

   (C)Copyright IBM Corp. 2008, 2009, 2010, 2011

   All Rights Reserved.

   File Name: dl_cbr_gen.c

   Change Activity:

   Date             Description of Change                   By
   -----------      ---------------------					--------
   1-Oct.2008       Created                                 Partha Dutta

   ----------------------------------------------------------------------------
   PROLOG END TAG zYx                                                         */

#include "CS.h"
#include "dl_exp_params.h"
#include "dl_cbr_gen.h"
#include "mac_dsd_test.h"
#include<sys/time.h>
#include "debug.h"

#ifdef PERFLOG
extern void print_perf_data();
#endif

extern double min_exceeding_frame_time;
extern double max_exceeding_frame_time;
extern long long int min_exceeding_frame_number;
extern long long int max_exceeding_frame_number;
extern int exceeding_time_count;

/** add by zzb for integration */
#ifdef INTEGRATION_TEST
extern pthread_mutex_t cbr_mutex;
extern pthread_cond_t cbr_cond;
#endif
/** end by zzb for integration */

//#define PHASE_ENABLED
#ifdef PHASE_ENABLED
extern pthread_mutex_t critical_phase_mutex;
extern pthread_cond_t critical_phase_done;
extern int not_critical_phase;
#endif

cbr_param* cbr_param_list_head;
extern int threads_alive;
int cbr_init(){

	cbr_param_list_head = NULL;
	return 0;
}

int close_files()
{
  cbr_param * param = cbr_param_list_head;
  while (param != NULL)
  {
    fclose(param->fp);
    param = param->next;
  }
  return 0;
}

int add_cid_cbr(int cid, int initial_offset, int duration, cbr_type type){

	cbr_param* param = (cbr_param*) mac_malloc(sizeof(cbr_param));
	if(!param){
		FLOG_FATAL("Cannot allocate memory in add_cid_cbr");
		return(-1);
	}

	param->cid = cid;
	param->next_tx_time = initial_offset;
	param->type = type;
	param->duration = duration;

	//list
	param->next = cbr_param_list_head;
	cbr_param_list_head = param;

	// set type params
	if(type == G711) {set_G711_param(param);}
	if(type == G723) {set_G723_param(param);}
	return(0);
}

int add_cid_cbr_vanilla(int cid, int initial_offset, int duration, int packet_size, int delay){
	cbr_param* param = (cbr_param*) mac_malloc(sizeof(cbr_param));
	if(!param){
		FLOG_FATAL("Cannot allocate memory in add_cid_cbr");
		return(-1);
	}

	param->cid = cid;
	param->next_tx_time = initial_offset;
	param->type = VANILLA;
	param->duration = duration;
	param->packet_size = packet_size;
	param->delay = delay;
	param->bit_rate = (int) (packet_size * 8000)/(delay);
#ifdef PRINT_DATA	
	char* file_name=(char*)malloc(16);
	sprintf(file_name,"%d.data", cid);
	param->fp = fopen(file_name, "w");
	if (!param->fp)
	{
	  FLOG_FATAL("Error opening file %s\n", file_name);
	}
	else
	{
	  FLOG_INFO("Opened file %s\n", file_name);
	}
        free(file_name);
#endif	

	//list
	param->next = cbr_param_list_head;
	cbr_param_list_head = param;


	return(0);

}



int set_G711_param(cbr_param* param){
	param->packet_size = 200;
	param->delay = 20;
	param->bit_rate = 80000; // (200*8)/(20/1000)
	return 0;
}

int set_G723_param(cbr_param* param){
	param->packet_size = 64;
	param->delay = 30;
	param->bit_rate = 17066; // (64*8)/(30/1000)
	return 0;
}



// IMPORTANT: all time calculation in micro seconds but remember that the cbr_params field are in milli seconds


#ifndef INTEGRATION_TEST
void* CBR(void* ignore){

	FLOG_DEBUG("Starting a CBR generator ...");

	long int my_clock = 0;  // in micro_sec
	long int max_duration = param_DL_EXP_DURATION;  // maximum duration over all cbrs
	long int sleep_interval = 0;  // how long to sleep before the next packet send
	long int adjusted_sleep_interval = 0; // sleep interval adjusted by time taken for sends
	cbr_param* tmp_param = cbr_param_list_head;

	// TODO: for testing
	long int packet_sent_count=0;

	//Needed for instrumentation
	create_init_trace_buffer(16384, "dl_cbr_gen");


	// calculate max_duration
	while(tmp_param != NULL){  // TODO: if cbrs are dynamically added then this calculation should be inside the send loop
		if(max_duration < 1000*tmp_param->duration){
			max_duration = 1000*tmp_param->duration;
		}
		tmp_param = tmp_param->next;
	}

	struct timeval start, cur, loop_start, tmp_start, tmp_end;
	struct timezone tzp;
	long int prev_sleep_duration, malloc_time, enq_time;

	// get start time in microsecond
	gettimeofday(&start, &tzp);
	gettimeofday(&cur, &tzp);

	long int time_for_sends = 0;  // measuring total time spent inside the loop
	int ii = 0, rand_byte = 0;
	char *packet_ptr;

	// main send loop

	while (my_clock < max_duration){



		tmp_param = cbr_param_list_head;
		sleep_interval = max_duration;

		gettimeofday(&loop_start, &tzp);

		prev_sleep_duration = ((loop_start.tv_sec-cur.tv_sec)*1000000L  + (loop_start.tv_usec-cur.tv_usec));  // time slept at the end of the previous loop

		pthread_rwlock_rdlock(&conn_info_rw_lock);
		while(tmp_param != NULL){

#ifdef PHASE_ENABLED
			pthread_mutex_lock(&critical_phase_mutex);
			while(not_critical_phase==0) {
				pthread_cond_wait(&critical_phase_done, &critical_phase_mutex);
			}
			pthread_mutex_unlock(&critical_phase_mutex);
#endif

			if(my_clock <= (tmp_param->duration*1000)){


				if(my_clock >= (tmp_param->next_tx_time*1000)){

					gettimeofday(&tmp_start, &tzp);
					short class_type=0;
					if(is_be_cid(tmp_param->cid)) {
						class_type=4;
					}


					char* tmp_packet = (char*) mac_sdu_malloc(tmp_param->packet_size, class_type);
					packet_ptr = tmp_packet;
					// Generate a random byte sequence, fill up the packet and also dump it in a file for matching
					for (ii = 0; ii < tmp_param->packet_size; ii++)
					{
					  //rand_byte = (8 * (rand() / (RAND_MAX + 1.0)));
					  // For FOAK integration testing, fixing data payload bits to all 1
					  rand_byte = 255;

#ifdef PRINT_DATA					  
					  fprintf(tmp_param->fp, "%d\n", rand_byte);
#endif					  
					  (*packet_ptr) = rand_byte;
					  packet_ptr++;
					} 
					gettimeofday(&tmp_end, &tzp);
					malloc_time= ((tmp_end.tv_sec-tmp_start.tv_sec)*1000000L  + (tmp_end.tv_usec-tmp_start.tv_usec));
					if(!tmp_packet){
						FLOG_FATAL("1: Memory allocation failed in CBR()");
					}

					//enqueue the packet to the sdu queue
					gettimeofday(&tmp_start, &tzp);
					enqueue_transport_sdu_queue(dl_sdu_queue, tmp_param->cid, tmp_param->packet_size, tmp_packet);
					gettimeofday(&tmp_end, &tzp);
					enq_time= ((tmp_end.tv_sec-tmp_start.tv_sec)*1000000L  + (tmp_end.tv_usec-tmp_start.tv_usec));

					//TODO: for testing
					packet_sent_count++;


					// when should we send the next packet for this connection
					tmp_param->next_tx_time += tmp_param->delay;
				}
				// update sleep_interval
				if(sleep_interval > ((1000*tmp_param->next_tx_time) - my_clock)){
					sleep_interval = (1000*tmp_param->next_tx_time) - my_clock;
				}
			}

			tmp_param = tmp_param->next;
		}
		pthread_rwlock_unlock(&conn_info_rw_lock);

		if(my_clock+sleep_interval > max_duration){
			break;
		}

		// get current time in microsecond
		gettimeofday(&cur, &tzp);

		// sleep
		adjusted_sleep_interval = (my_clock + sleep_interval) - ((cur.tv_sec-start.tv_sec)*1000000L  + (cur.tv_usec-start.tv_usec));


		// time spent inside the loop
		time_for_sends += ((cur.tv_sec-loop_start.tv_sec)*1000000L  + (cur.tv_usec-loop_start.tv_usec));


		if(adjusted_sleep_interval > 0){
			usleep(adjusted_sleep_interval);
		}

		// update my_clock
		my_clock += sleep_interval;
	}

	FLOG_INFO("Number of packets sent = %ld \n", packet_sent_count);fflush(stdout);

#ifdef PERFLOG
	print_perf_data();
#endif

	pthread_exit(NULL);
}
#endif

#ifdef INTEGRATION_TEST
void* CBR(void* ignore){

    if (pthread_setcanceltype (PTHREAD_CANCEL_ASYNCHRONOUS, NULL) != 0)
    {
        printf ("\n");

        return NULL;
    }

	FLOG_DEBUG("Starting a CBR generator ...");

pthread_mutex_init(&cbr_mutex, NULL);
pthread_cond_init (&cbr_cond, NULL);

	long int my_clock = 0;  // in micro_sec
	long int max_duration = 0;  // maximum duration over all cbrs
	long int sleep_interval = 0;  // how long to sleep before the next packet send
	long int adjusted_sleep_interval = 0; // sleep interval adjusted by time taken for sends
	cbr_param* tmp_param = cbr_param_list_head;

	// TODO: for testing
	long int packet_sent_count=0;

	//Needed for instrumentation
	create_init_trace_buffer(16384, "dl_cbr_gen");

    pthread_mutex_lock(&cbr_mutex);
    pthread_cond_wait(&cbr_cond, &cbr_mutex);
    pthread_mutex_unlock(&cbr_mutex);


	// calculate max_duration
	while(tmp_param != NULL){  // TODO: if cbrs are dynamically added then this calculation should be inside the send loop
		if(max_duration < 1000*tmp_param->duration){
			max_duration = 1000*tmp_param->duration;
		}
		//printf("\n tmp_params cid: %d delay %d duration %d\n", tmp_param->cid, tmp_param->delay, tmp_param->duration);
		tmp_param = tmp_param->next;
	}

	//printf("\nmax_duration = %d micro sec\n", max_duration);


	struct timeval start, cur, loop_start, loop_end, tmp_start, tmp_end;
	struct timezone tzp;
	long int start_time, cur_time, prev_sleep_duration, malloc_time, enq_time;

	// get start time in microsecond
	gettimeofday(&start, &tzp);
	gettimeofday(&cur, &tzp);

	long int time_for_sends = 0;  // measuring total time spent inside the loop
	int ii = 0, rand_byte = 0;
	char *packet_ptr;

	// main send loop

//	while (my_clock <= max_duration){
        while(1)
        {
		tmp_param = cbr_param_list_head;
		sleep_interval = max_duration;

		gettimeofday(&loop_start, &tzp);

		prev_sleep_duration = ((loop_start.tv_sec-cur.tv_sec)*1000000L  + (loop_start.tv_usec-cur.tv_usec));  // time slept at the end of the previous loop

		while(tmp_param != NULL){

					short class_type=0;
					if(is_be_cid(tmp_param->cid)) {
						class_type=4;
					}


					char* tmp_packet = (char*) mac_sdu_malloc(tmp_param->packet_size, class_type);
					packet_ptr = tmp_packet;
					// Generate a random byte sequence, fill up the packet and also dump it in a file for matching
					for (ii = 0; ii < tmp_param->packet_size; ii++)
					{
//					  rand_byte = (8 * (rand() / (RAND_MAX + 1.0)));
                                            rand_byte = 255;

					  //printf("%d Random byte for cid: %d is: %d\n", ii, tmp_param->cid, rand_byte);
//					  fprintf(tmp_param->fp, "%d\n", rand_byte);
					  (*packet_ptr) = rand_byte;
					  packet_ptr++;
					} 

					//enqueue the packet to the sdu queue
					//printf("\nEnqueue trying: my_clock:%d cid:%d size:%d \n", my_clock, tmp_param->cid, tmp_param->packet_size);
					enqueue_transport_sdu_queue(dl_sdu_queue, tmp_param->cid, tmp_param->packet_size, tmp_packet);
					//	  if((packet_sent_count%10) == 0){
					//  printf("CBR packet sent of size: %d", tmp_param->packet_size);
					//}
					//TODO: for testing
					packet_sent_count++;


					// when should we send the next packet for this connection
					tmp_param->next_tx_time += tmp_param->delay;

			tmp_param = tmp_param->next;
		}

                usleep(21000);
	}

	threads_alive=0;

	//printf("\n Error = %ld \n", time_for_sends);
	pthread_exit(NULL);
}
#endif


