/* ----------------------------------------------------------------------------
   IBM Confidential

   IBM Wireless M2M Platform   

   (C)Copyright IBM Corp. 2008, 2009, 2010, 2011

   All Rights Reserved.

   File Name: perf_log.h

   Change Activity:

   Date             Description of Change                       By
   -----------      ---------------------                       --------
   1-Oct.2008       Created                                     Malolan Chetlur

   ----------------------------------------------------------------------------
   PROLOG END TAG zYx                                                         */

#ifndef _PERF_LOG_H_
#define _PERF_LOG_H_

#ifdef PERFLOG

typedef enum {
  METHOD_BEGIN=0,
  METHOD_END=1
} LogType;

typedef struct p_data perf_data;
struct p_data {
  unsigned long long int st_time;
  unsigned long long int sum_latency;
  unsigned long long int sum_latency_sq;
  int latency_min;
  int latency_max;
  int exceed_count;
  int count;
  int tid;
  perf_data* next;
};

extern perf_data* p_data_arr;

// names (string) of all code blocks corresponding to blk id
extern char perf_block_names[20][30];
extern char* perf_enabled_blocks;

extern void perf_log_init(void);
extern void plog(int method_name, LogType ltype);
extern void print_perf_data(void);
extern void perf_log_cleanup(void);
#include "perf_log_defns.h"

#else
//PERFLOGS not defined
#define perf_log_init(pfn);
#define plog(method_name, ltype);
#define print_perf_data();
#define perf_log_cleanup(void);
#define PLOG(X,Y);
#endif
#endif
