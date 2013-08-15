/* ----------------------------------------------------------------------------
   IBM Confidential

   IBM Wireless M2M Platform   

   (C)Copyright IBM Corp. 2008, 2009, 2010, 2011

   All Rights Reserved.

   File Name: perf_log_defns.h

   Change Activity:

   Date             Description of Change                       By
   -----------      ---------------------                       --------
   1-Oct.2008       Created                                     Malolan Chetlur

   ----------------------------------------------------------------------------
   PROLOG END TAG zYx                                                         */

#ifndef _PERF_LOG_DEFNS_
#define _PERF_LOG_DEFNS_
#ifdef PERFLOG

// const id of all code blocks profiled

//PLOG_DL_SCHEDULER profile bs_scheduling call in mac_dl_cps_controller
#define PLOG_DL_SCHEDULER 0 

//PLOG_DL_BURSTPROC profiles burst processing code in mac_dl_cps_controller
#define PLOG_DL_BURSTPROC 1

//PLOG_SDUQ_ENQUEUE profiles eqnueue in sdu_cid_queue
#define PLOG_SDUQ_ENQUEUE 2

//PLOG_SDUQ_DEQUEUE profiles dequeue in sdu_cid_queue
#define PLOG_SDUQ_DEQUEUE 3

//PLOG_SDUQ_PEEK profiles peek in sdu_cid_queue
#define PLOG_SDUQ_PEEK 4

//PLOG_FRAME_LATENCY measure frame creation latency
#define PLOG_FRAME_LATENCY 5

#define PLOG(X,Y) plog(X,Y);

#endif //ifdef PERFLOG
#endif //ifndef _PERF_LOG_DEFNS_
