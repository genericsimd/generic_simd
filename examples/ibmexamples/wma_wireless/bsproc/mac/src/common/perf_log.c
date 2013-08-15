/* ----------------------------------------------------------------------------
   IBM Confidential

   IBM Wireless M2M Platform   

   (C)Copyright IBM Corp. 2009, 2010, 2011

   All Rights Reserved.

   File Name: perf_log.c

   Change Activity:

   Date             Description of Change                   By
   -----------      ---------------------					--------
   1-Apr.2009       Created                                 Malolan Chetlur

   ----------------------------------------------------------------------------
   PROLOG END TAG zYx                                                         */
#ifdef PERFLOG
//#define USE_GTOD

#include <stdio.h>
#include <stdlib.h>
#include <sys/time.h>
#include <unistd.h>
#include <syscall.h>
#include <assert.h>
#include <limits.h>
#include <math.h>
#include <string.h>
#include "perf_log.h"
#include "util.h"
#include "memmgmt.h"

#include "sdu_cid_queue.h"

perf_data* p_data_arr=NULL;
char perf_block_names[20][30] = {"DL Scheduler", "Burst Proc", "SDUQ Enqueue", "SDUQ Dequeue", "SDUQ Peek", "Frame Creation Latency"};
char* perf_enabled_blocks=NULL;

//extern int frame_number;
extern char param_PERF_ENABLED_BLOCKS[200];

const int duration[9] = {0, 2000, 2500, 4000, 5000, 8000, 10000, 12500, 20000};
#define FRAME_DURATION_CODE 8

void perf_log_init() {
  p_data_arr=(perf_data*) mac_malloc(sizeof(perf_data)*20);
  assert(p_data_arr!=NULL);
  memset(p_data_arr, 0, sizeof(perf_data)*20);
  int ii=0;
  for(ii=0; ii<20; ii++) {
    p_data_arr[ii].latency_min=INT_MAX;
  }
  perf_enabled_blocks=param_PERF_ENABLED_BLOCKS;
}

void plog(int blk_name, LogType ltype) {

  if(perf_enabled_blocks[blk_name]!='1') return;

  unsigned long long int log_time=0;

  if(ltype==METHOD_END) {
#ifdef USE_GTOD
    l_time=gettimeofday(&tv, NULL);
    log_time=tv.tv_sec * 1000000 + tv.tv_usec;
#else
    log_time=readtsc();
#endif
  }
  int tid= syscall(SYS_gettid);
  perf_data* p_data=p_data_arr+blk_name;
  perf_data* prev_pd=p_data;

  while(p_data!=NULL) {
    if(p_data->tid==0) {
      p_data->tid=tid;
    }
    if(p_data->tid==tid) {
      //fill perf data and return
      if(ltype==METHOD_BEGIN) {
#ifdef USE_GTOD
	l_time=gettimeofday(&tv, NULL);
	log_time=tv.tv_sec * 1000000 + tv.tv_usec;
#else
	log_time=readtsc();
#endif
	p_data->st_time=log_time;
      }
      else { //METHOD_END
	unsigned long long int diff=log_time-p_data->st_time;
	p_data->sum_latency+=diff;
	p_data->sum_latency_sq += (diff*diff);
	if(p_data->latency_min > diff) {
	  p_data->latency_min=(int)diff;
	}
	if(p_data->latency_max < diff) {
	  p_data->latency_max=(int)diff;
	}
        if ( diff > (unsigned long long int)duration[FRAME_DURATION_CODE] )
        {
            p_data->exceed_count ++;
        }
	p_data->count++;
      }
      return;
    }
    else {
      prev_pd=p_data;
      p_data=p_data->next;
    }
  }
  //if we are here; add perf data for this tid
  assert(ltype==METHOD_BEGIN);
  perf_data *pd=(perf_data*) mac_malloc(sizeof(perf_data));
  memset(pd, 0, sizeof(perf_data));
  //fill perf data
  pd->tid=tid;
  prev_pd->next=pd;
  pd->latency_min=INT_MAX;
#ifdef USE_GTOD
  l_time=gettimeofday(&tv, NULL);
  log_time=tv.tv_sec * 1000000 + tv.tv_usec;
#else
  log_time=readtsc();
#endif
  pd->st_time=log_time;
}

void print_perf_data(void) {
  int ii=0;
  for(ii=0; ii<20; ii++) {
    perf_data* pd=p_data_arr+ii;
    unsigned long long int sum_lat=0;
    unsigned long long int sum_lat_sq=0;
    int min_lat=INT_MAX;
    int max_lat=0;
    int total_count=0;
    int exceed_count = 0;
    int tid = pd->tid;

    while(pd!=NULL) {
      if(pd->count>0) {
/* 	printf("Latency info for: %s...\n", perf_block_names[ii]); */
/* 	printf("Thd id:%d, count:%d, max latency:%d, min latency:%d, sum_latency:%lld, sum of latency squares:%lld\n", */
/* 	       pd->tid, pd->count, pd->latency_max, pd->latency_min, pd->sum_latency, pd->sum_latency_sq); */
	sum_lat+=pd->sum_latency;
	sum_lat_sq +=pd->sum_latency_sq;
	if(min_lat>pd->latency_min) {
	  min_lat=pd->latency_min;
	}
	if(max_lat<pd->latency_max) {
	  max_lat=pd->latency_max;
	}
	total_count+=pd->count;
        exceed_count+=pd->exceed_count;
      }
      pd=pd->next;
    }
    if(total_count>0) {
      double avg_lat=(sum_lat*1.0)/(total_count*1.0);
      double sd_lat=sqrt((sum_lat_sq*1.0)/(total_count*1.0) - (avg_lat*avg_lat));
//      printf("TID %d: Latency info for: %s: ", tid, perf_block_names[ii]);
      FLOG_INFO("TID %d: Latency info for: %s: count:%d, max latency:%d, min latency:%d, avg latency:%e, S.D of latency:%e\n", tid, perf_block_names[ii], total_count, max_lat, min_lat, avg_lat, sd_lat);
	  FLOG_INFO("TID %d: Percentage info for: %s: exceed_count:%d, succedd radio %f\n", tid, perf_block_names[ii], exceed_count, ((float)total_count - (float)exceed_count) / (float)total_count);
    }
  }
}

void perf_log_cleanup(void) {
  //An array of perf_data is allocated 
  //if there are more threads for a block, each element contains a list of pd
  // delete the list execept thefirst element created as array
  int ii=0;
  for(ii=0; ii<20; ii++) {
    perf_data* pd=p_data_arr+ii;
    perf_data* pd_list=pd->next;

    while(pd_list!=NULL) {
      perf_data* nxt_pd=pd_list->next;
      mac_free(sizeof(perf_data), pd_list);
      pd_list=nxt_pd;
    }
  }
  //free the array of perf_data allocated
  mac_free(sizeof(perf_data)*20, p_data_arr);
}

#endif
