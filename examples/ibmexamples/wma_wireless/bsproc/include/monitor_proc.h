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

#ifndef __MONITOR_PROC_H_
#define __MONITOR_PROC_H_

#include "bs_debug.h"

#define MAX_QUEUE_LEN 9

#define MIN_MONITOR_PKT_SIZE 128

struct monitor_cfg_trace
{
    char  a_cfg_name[64];
    int     w_on_off;
    int     w_sampling;
    
}__attribute__((packed));

struct monitor_reset_metric
{
    char  a_cfg_name[64];
    int   type;

}__attribute__((packed));

struct monitor_dump_metric
{
    char  a_cfg_name[64];
    int   on_off;

}__attribute__((packed));

struct monitor_trace_hdr
{
    char hook_name[64];
    int len;
}__attribute__((packed));

int monitor_process (void);
int monitor_release (void);

/* define the type of message through the pipe */
#define RRH_PERIOD_SENSING 10
#define WIRELESS_CPE_ENTRY 11
#define HOOK_DEBUG_TRACE 20

int send_peri_senssing_msg (char * p_active_band, float * p_result);
int send_network_entry_msg (char * mac_addr);

int hook_debug_trace(int debug_idx, void * p_buf, int buf_len, int inc);

int init_monitor(void);

#endif /* __MONITOR_PROC_H_ */

