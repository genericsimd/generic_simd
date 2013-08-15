/* ----------------------------------------------------------------------------
 * IBM Confidential

   IBM Wireless M2M Platform

   (C)Copyright IBM Corp. 2009, 2010, 2011

   All Rights Reserved.

   File Name: bs_proc.c

   Change Activity:

   Date             Description of Change                            By
   -----------      ---------------------                            --------
   12-May 2011       Created                                         Zhu, Zhenbo

   ----------------------------------------------------------------------------
   PROLOG END TAG zYx                                                         */

#include <pthread.h>
#include <stdlib.h>
#include <sys/time.h>
#include <signal.h>

#include <sched.h>
#include <string.h>
#include <stdio.h>

#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#include <unistd.h>

#include "mac.h"
#include "queue_util.h"
#include "bs_cfg.h"
#include "bs_debug.h"
#include "global.h"

#include "flog.h"

#include "uart_util.h"
#include "monitor_proc.h"
#include "metric_proc.h"

#include "dump_util.h"
#include "prof_util.h"

/** from MAC layer */
#include "mac_proc.h"
#include "stube_proc.h"

/** from PHY layer */
#include "phy_proc.h"
#include "prephy_proc.h"
#include "initial_sensing_proc.h"

/** from CS layer */
#include "cs_proc.h"

/** from base support lib */
#include "witm_info.h"

/** from RRH */
#include "rru_proc.h"
#include "trans_proc.h"

//#define _FRAME_DUMP_

unsigned int app_init_flag = 0;
unsigned int adapter_init_flag = 0;

/** thread handler */
pthread_t dump_thread = 0;
pthread_t pkt_forward_thread = 0;

static int init_thread (void)
{
    if (monitor_process () != 0)
    {
        FLOG_FATAL ("Initial Monitor ERROR\n");

        return 1;
    }

    if (metric_process () != 0)
    {
        FLOG_FATAL ("Initial Metric ERROR\n");

        return 1;
    }

    if (rrh_tdd_tx_process () != 0)
    {
        FLOG_ERROR ("Initial RANGING ERROR\n");
        return 1;
    }

    if (phy_bs_process () != 0)
    {
        FLOG_FATAL ("Initial PHY thread ERROR\n");
        return 1;
    }
#ifndef _PHY_STUB_ENABLE_
    if (mac_bs_process () != 0)
    {
        FLOG_FATAL ("Initial MAC thread ERROR\n");
        return 1;
    }
#else
    if (start_stube_thread() != 0)
    {
        FLOG_FATAL ("Initial PHY stub ERROR\n");
        return 1;
    }
#endif

    if (pre_phy_process () != 0)
    {
        FLOG_FATAL ("Initial PHY thread ERROR\n");
        return 1;
    }

    if (rrh_tdd_rx_process () != 0)
    {
        FLOG_FATAL ("Initial RANGING ERROR\n");
        return 1;
    }

    if (pkt_forward_process () != 0)
    {
        FLOG_FATAL ("Initial Pkt Forward ERROR\n");

        return 1;
    }
#ifdef _IP_ENABEL_
    if (pkt_classify_process () != 0)
    {
        FLOG_FATAL ("Initial PKY Classify ERROR\n");

        return 1;
    }
#endif

    return 0;
}

static int release_thread (void)
{
    if (pkt_forward_release () != 0)
    {
        FLOG_FATAL ("Release Pkt Forward ERROR\n");

        return 1;
    }
#ifdef _IP_ENABEL_
    if (pkt_classify_release () != 0)
    {
        FLOG_FATAL ("Release Pkt Classift ERROR\n");

        return 1;
    }
#endif

    if (rrh_tdd_rx_release () != 0)
    {
        FLOG_FATAL ("Release RRH Rx ERROR\n");

        return 1;
    }

    if (pre_phy_release () != 0)
    {
        FLOG_FATAL ("Release PRE PHY ERROR\n");

        return 1;
    }

#ifndef _PHY_STUB_ENABLE_

    if (mac_bs_release () != 0)
    {
        FLOG_FATAL ("Release MAC ERROR\n");

        return 1;
    }
#else
    if (stop_stube_thread () != 0)
    {
        FLOG_FATAL ("Release MAC ERROR\n");

        return 1;
    }
#endif

    if (phy_bs_release () != 0)
    {
        FLOG_FATAL ("Release PHY ERROR\n");
        return 1;
    }

    if (rrh_tdd_tx_release () != 0)
    {
        FLOG_FATAL ("Release RRH Tx ERROR\n");

        return 1;
    }

    if (metric_release () != 0)
    {
        FLOG_FATAL ("Release Metric ERROR\n");

        return 1;
    }

    if (monitor_release () != 0)
    {
        FLOG_FATAL ("Release Monitor ERROR\n");

        return 1;
    }

#ifdef DSX_ENABLE
    extern pthread_t dsx_thread;
    FLOG_INFO("cancel dsx thread");
    pthread_cancel(dsx_thread);
#endif

#ifdef BR_ENABLE
    extern pthread_t br_thread;
    FLOG_INFO("cancel br thread");
    pthread_cancel(br_thread);
#endif
	
    return 0;
}

static int init_adapters (void)
{
#ifdef _DUMP_UTIL_ENABLE_
    if (init_dump () != 0)
    {
        FLOG_FATAL ("INIT DUMP ERROR");
        return 1;
    }
#endif
    if (init_queue () != 0)
    {
        FLOG_FATAL ("INIT QUEUE ERROR");
        //        application_deinit ();
        return 1;
    }

    if (init_cs_table() != 0)
    {
        FLOG_FATAL ("INIT CS TABLE");
        //        application_deinit ();
        return 1;
    }

#ifdef _UART_ENABLE_
    if (init_uart() != 0)
    {
        FLOG_FATAL ("INIT UART DEV ERROR");
    }
#endif

    if (init_bsperf_metric () != 0)
    {
        FLOG_FATAL ("INIT PERF METRIC ERROR");
        return 1;
    }

    adapter_init_flag = 1;

    return 0;
}

static int destroy_adapters (void)
{

    if (adapter_init_flag == 0)
    {
        FLOG_FATAL("Adapter not initialized");
        return 1;
    }

#ifdef _UART_ENABLE_
    if (release_uart () != 0)
    {
        FLOG_FATAL ("Release UART ERROR ");
        return 1;
    }
#endif

    if (release_cs_table () != 0)
    {
        FLOG_FATAL ("Release CS TABLE ERROR");
        return 1;
    }

    if (release_queue () != 0)
    {
        FLOG_FATAL ("Release QUEUE ERROR");
        return 1;
    }

    return 0;
}

static int init_application ()
{
    if (init_thread () != 0)
    {
        FLOG_FATAL ("Initial THREADS ERROR");
        //        application_deinit ();
        return 1;
    }

    app_init_flag = 1;


    PROF_INIT( MSC_CPU );

    return 0;
}

int release_application (void)
{
    if (app_init_flag == 0)
    {
        FLOG_ERROR("Application not initialized");
        return 0;
    }

    if (release_thread () != 0)
    {
        FLOG_FATAL ("Release THREADS ERROR\n");
    }

    //    print_perf_data ();

    return 0;
}


static int exit_mbmsproc (int cmd)
{
    FLOG_INFO ("Base Station Process exited with %d", cmd);

    (void)cmd;

#ifdef _DUMP_UTIL_ENABLE_
    if (release_dump () != 0)
    {
        FLOG_FATAL ("Release DUMP ERROR");
        return 1;
    }
#endif

//    DO_DUMP(DUMP_RX_ALL_RAW_ID, 0, 0, NULL);

    if (release_application () != 0)
    {
        FLOG_FATAL ("Release Application failed");
    }

    if (init_sensing_release() != 0)
    {
        FLOG_FATAL ("Release init sensing failed");
    }

    if (close_rrh() != 0)
    {
        FLOG_FATAL ("Delete RRH failed");
    }

    if (close_trans () != 0)
    {
        FLOG_FATAL ("Release Trans ERROR\n");

        return 1;
    }

    if (destroy_adapters () != 0)
    {
        FLOG_FATAL ("Release Adapter failed");
    }

    if (deinit_global_hook () != 0)
    {
        FLOG_FATAL ("Release global hook failed");
    }

    if (deinit_global_param () != 0)
    {
        FLOG_FATAL ("Release global parameters failed");
    }

    wmrt_exit ();

    PROF_TIMER_DUMP;
    PROF_DEINIT;
    exit(0);

    return 0;
}

void sig_handler( int sig __attribute__((unused)))
{
    exit_mbmsproc (0);
}


int main (int argc, char *argv[])
{
    struct sched_param param;
    int maxpri;
    struct queue_msg sig_msg;

    char * device_id = "188";
    char * agent_id = "0";

    maxpri = sched_get_priority_max(SCHED_METHOD);

    if(maxpri == -1)
    {
        FLOG_ERROR("sched_get_priority_max() failed\n");
        return 1;
    }

    param.sched_priority = maxpri;

    if (sched_setscheduler(getpid(), SCHED_METHOD, &param) == -1)
    {
        FLOG_ERROR("sched_setscheduler() failed\n");
        return 1;
    }

    LOG_INIT_CONSOLE_ONLY("WMA wireless");
/*
    if (wminfo_init (WMI_ROLEBS) != 0)
    {
        FLOG_FATAL ("Failed to initialize Managerial Runtime!");
        return 1;
    }
    if (wmrt_setsysexitfunc (&exit_mbmsproc) != 0)
    {
        FLOG_FATAL ("Failed to set sys-exit\n");
        return 1;
    }
*/
    signal( SIGINT, sig_handler); //enable when wmrt exit function not working

    if (argc < 2)
    {
        printf ("Usage : %s <absolute path of configuration file> [device ID] [Agent ID] \n",
                argv [0]);

        return 0;
    }else if (argc == 3)
    {
        device_id = argv[2];
    }else if (argc == 4)
    {
        device_id = argv[2];
        agent_id = argv[3];
    }

/*
     if (wmrt_regusrexitfunc (&exit_mbmsproc) != 0)
     {
     FLOG_DEBUG ("Failed in register user exit function");
     return -1;
     }
*/

    FLOG_DEBUG ("Base Station Process Started\n");

    /** start-up process */
    if (init_global_param (argv[1]) != 0)
    {
        FLOG_FATAL ("initial parameters error\n");
        exit_mbmsproc(0);
        return 1;
    }

    /** start-up process */
    if (init_global_hook () != 0)
    {
        FLOG_FATAL ("initial hook error\n");
        exit_mbmsproc(0);
        return 1;
    }

    if (init_adapters () != 0)
    {
        FLOG_FATAL ("Error in prepare adapter\n");
        exit_mbmsproc(0);
        return 1;
    }

    if (init_trans (device_id, agent_id) != 0)
    {
        FLOG_FATAL ("INIT PERF METRIC ERROR");
        return 1;
    }

    if (connect_rrh() != 0)
    {
        FLOG_FATAL ("connect to RRH faild error\n");
        exit_mbmsproc(0);
        return 1;
    }

    if (init_sensing_process() != 0 )
    {
        FLOG_FATAL ("initial sensing error\n");
        exit_mbmsproc(0);
        return 1;
    }

    /** running process */
    if (init_application () != 0)
    {
        FLOG_FATAL ("Error in application initializing\n");
        exit_mbmsproc(0);
        return 1;

    }

    FLOG_INFO ("Initialization stage finished\n");

    /** running process */
    sig_msg.my_type = exit_de_id;

#ifdef SYS_POWER_COMPILE
    while(1)
    {
	sleep(10);
        PROF_TIMER_DUMP;

    }
#else

    while(1)
	sleep(10);

    if (wmrt_dequeue (exit_de_id, &sig_msg, sizeof(struct queue_msg))
                      == -1)
    {
        FLOG_ERROR ("ENQUEUE ERROR\n");
    }
#endif

    exit_mbmsproc (0);

    return 0;
}
