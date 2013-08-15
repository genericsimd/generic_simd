/* ----------------------------------------------------------------------------
 IBM Confidential

 IBM Wireless M2M Platform

 (C)Copyright IBM Corp. 2009, 2010, 2011

 All Rights Reserved.

 File Name: monitor_proc.h

 Change Activity:

 Date             Description of Change                            By
 -----------      ---------------------                            --------
 16-Aug.2011      Created                                          Zhu Zhenbo

 ----------------------------------------------------------------------------
 PROLOG END TAG zYx                                                         */

#ifndef __METRIC_PROC_H_
#define __METRIC_PROC_H_

#include "bs_debug.h"

/* metric update period in second */
#define METRIC_UPDATE_PERIOD 90

extern unsigned int ranging_success_count;

extern double ul_total_bytes;
extern double dl_total_bytes;
extern double ul_mgmt_total_bytes;
extern double dl_mgmt_total_bytes;
extern double ul_app_total_bytes;
extern double dl_app_total_bytes;

extern unsigned int crc_error_count;
extern unsigned int pdu_total_count;

extern unsigned int total_wmb_in_system;
extern unsigned int ul_total_connection;
extern unsigned int dl_total_connection;

int init_bsperf_metric (void);
int metric_process (void);
int metric_release (void);

#endif /* __METRIC_PROC_H_ */

