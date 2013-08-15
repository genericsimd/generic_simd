/* ----------------------------------------------------------------------------
 IBM Confidential

 IBM Wireless M2M Platform

 (C)Copyright IBM Corp. 2009, 2010, 2011

 All Rights Reserved.

 File Name: metric_proc.c

 Change Activity:

 Date             Description of Change                            By
 -----------      ---------------------                            --------
 16-Aug.2011      Created                                          Zhu Zhenbo

 ----------------------------------------------------------------------------
 PROLOG END TAG zYx                                                         */


/** OS */
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>

//#define _GNU_SOURCE
#include <pthread.h>

/** Application */
#include "metric_proc.h"
#include "queue_util.h"
#include "thd_util.h"
#include "flog.h"
#include "prephy_proc.h"

#include "bs_cfg.h"
#include "bs_debug.h"
#include "trans.h"
#include <sys/time.h>
#include "trans_timer.h"

unsigned int ranging_success_count = 0;
unsigned int total_wmb_in_system = 0;
unsigned int ul_total_connection = 0;
unsigned int dl_total_connection = 0;
unsigned int crc_error_count = 0;
unsigned int pdu_total_count = 0;

double ul_total_bytes = 0;
double dl_total_bytes = 0;
double ul_mgmt_total_bytes = 0;
double dl_mgmt_total_bytes = 0;
double ul_app_total_bytes = 0;
double dl_app_total_bytes = 0;

static int update_metric_func (void *p_msg, size_t len, void *p_msg_info);

int init_bsperf_metric (void)
{
    int default_inum = 0;
    float default_fnum = 0.0f;
    int ret = 0;

    ret = set_global_param("CRCErrorCount", (void *)&(default_inum));

    if (ret != 0)
    {
        FLOG_WARNING("Set CRCErrorCount failed\n");
    }

    ret = set_global_param("RangingSuccessCount", (void *)&(default_inum));

    if (ret != 0)
    {
        FLOG_WARNING("Set RangingSuccessCount failed\n");
    }

    ret = set_global_param("PerWmbDLThroughput", (void *)&(default_fnum));

    if (ret != 0)
    {
        FLOG_WARNING("Set PerWmbDLThroughput failed\n");
    }

    ret = set_global_param("PerWmbULThroughput", (void *)&(default_fnum));

    if (ret != 0)
    {
        FLOG_WARNING("Set PerWmbULThroughput failed\n");
    }

    ret = set_global_param("PerConnDLThroughput", (void *)&(default_fnum));

    if (ret != 0)
    {
        FLOG_WARNING("Set PerConnDLThroughput failed\n");
    }

    ret = set_global_param("PerConnULThroughput", (void *)&(default_fnum));

    if (ret != 0)
    {
        FLOG_WARNING("Set PerConnULThroughput failed\n");
    }

    ret = set_global_param("ULMgmtThroughput", (void *)&(default_fnum));

    if (ret != 0)
    {
        FLOG_WARNING("Set ULMgmtThroughput failed\n");
    }

    ret = set_global_param("DLMgmtThroughput", (void *)&(default_fnum));

    if (ret != 0)
    {
        FLOG_WARNING("Set DLMgmtThroughput failed\n");
    }

    ret = set_global_param("ConnectionCount", (void *)&(default_inum));

    if (ret != 0)
    {
        FLOG_WARNING("Set ConnectionCount failed\n");
    }

    ret = set_global_param("PDUTotalCount", (void *)&(default_inum));

    if (ret != 0)
    {
        FLOG_WARNING("Set ConnectionCount failed\n");
    }

    FLOG_DEBUG("initial Perf Metric finish\n");

    return 0;
}


int metric_process (void)
{
    int ret = 0;
    int duration;
    void* p_timer = NULL;
    
    ret = get_global_param ("METRIC_UPDATE_DURATION", & (duration));

    if (ret != 0)
    {
        FLOG_WARNING ("get parameters METRIC_UPDATE_DURATION error\n");
    }
   
    #ifdef _NEW_TRANS_ENABLE_
    
    struct trans_timer_info st_timer_info;    

    st_timer_info.f_callback = update_metric_func;
    st_timer_info.p_data = NULL;
    st_timer_info.p_timer_list = &g_trans_timer_list;
    st_timer_info.uc_type = TRANS_TIMER_TYPE_CIRCLE;
    st_timer_info.uw_interval = duration;
    
    ret = trans_timer_add(&st_timer_info, &p_timer);    
    if (TRANS_SUCCESS != ret)
    {
        FLOG_ERROR("Call trans_timer_add error! ret = %d\r\n", ret);
        return 1;
    }

    #else
    struct timeval st_time_val;
    struct trans_timer_msg_info st_msg_info;

    gettimeofday(&st_time_val, NULL);
    
    /*Timeout -- 10s*/
    st_time_val.tv_sec = st_time_val.tv_sec + duration;
    //st_time_val.tv_usec = st_time_val.tv_usec;

    st_msg_info.us_serial_number = 0;
    st_msg_info.uw_src_moudle = TRANS_MOUDLE_BS;
    st_msg_info.uc_block_flag = TRANS_QUENE_BLOCK;
    st_msg_info.f_callback = update_metric_func;
    st_msg_info.p_user_info = NULL;

    ret = trans_timer_add(&st_time_val,
                             update_metric_func,
                             &st_msg_info,
                             sizeof (struct trans_timer_msg_info),
                             &st_msg_info,
                             &p_timer);

    if (TRANS_SUCCESS != ret)
    {
        FLOG_ERROR("Call trans_timer_add error! uw_ret = %d\r\n", ret);
        return 1;
    }
    
    #endif

    FLOG_DEBUG("initial Monitor Process finish\n");

    return 0;
}

int metric_release (void)
{
/* clean timer? */
    return 0;
}


static int update_metric_func(void *p_msg, size_t len, void *p_msg_info)
{
    int tmp_inum = 0;
    float tmp_fnum = 0.0f;
    int ret = 0;

    static double tmp_ul_total_bytes = 0;
    static double tmp_dl_total_bytes = 0;
    static double tmp_ul_mgmt_total_bytes = 0;
    static double tmp_dl_mgmt_total_bytes = 0;
    static double tmp_ul_app_total_bytes = 0;
    static double tmp_dl_app_total_bytes = 0;

    int duration;

    ret = get_global_param("RangingSuccessCount", (void *)&(tmp_inum));

    if (ret != 0)
    {
        FLOG_WARNING("Get RangingSuccessCount failed\n");
    }

    tmp_inum += ranging_success_count;
    ranging_success_count = 0;

    ret = set_global_param("RangingSuccessCount", (void *)&(tmp_inum));

    if (ret != 0)
    {
        FLOG_WARNING("Set RangingSuccessCount failed\n");
    }

    ret = get_global_param("CRCErrorCount", (void *)&(tmp_inum));

    if (ret != 0)
    {
        FLOG_WARNING("Get CRCErrorCount failed\n");
    }

//    tmp_inum += crc_error_count;
//    crc_error_count = 0;

    ret = set_global_param("CRCErrorCount", (void *)&(crc_error_count));

    if (ret != 0)
    {
        FLOG_WARNING("Set CRCErrorCount failed\n");
    }

    if (total_wmb_in_system == 0)
    {
        tmp_fnum = 0.0F;
    }else
    {
        tmp_fnum = (float) ( (dl_total_bytes - tmp_dl_total_bytes) * 8 /
                             (float)(total_wmb_in_system * METRIC_UPDATE_PERIOD * 1024) );
    }

    ret = set_global_param("PerWmbDLThroughput", (void *)&(tmp_fnum));

    if (ret != 0)
    {
        FLOG_WARNING("Set PerWmbDLThroughput failed\n");
    }



    if (total_wmb_in_system == 0)
    {
        tmp_fnum = 0.0F;
    }else
    {
        tmp_fnum = (float) ( (ul_total_bytes - tmp_ul_total_bytes) * 8 /
                             (float)(total_wmb_in_system * METRIC_UPDATE_PERIOD * 1024) );
    }

    ret = set_global_param("PerWmbULThroughput", (void *)&(tmp_fnum));

    if (ret != 0)
    {
        FLOG_WARNING("Set PerWmbULThroughput failed\n");
    }



    if (dl_total_connection == 0)
    {
        tmp_fnum = 0.0F;
    }else
    {
        tmp_fnum = (float) ( (dl_total_bytes - tmp_dl_total_bytes) * 8 /
                             (float)(dl_total_connection * METRIC_UPDATE_PERIOD * 1024) );
    }

    ret = set_global_param("PerConnDLThroughput", (void *)&(tmp_fnum));

    if (ret != 0)
    {
        FLOG_WARNING("Set PerConnDLThroughput failed\n");
    }



    if (ul_total_connection == 0)
    {
        tmp_fnum = 0.0F;
    }else
    {
        tmp_fnum = (float) ( (ul_total_bytes - tmp_ul_total_bytes) * 8 /
                             (float)(ul_total_connection * METRIC_UPDATE_PERIOD * 1024) );
    }

    ret = set_global_param("PerConnULThroughput", (void *)&(tmp_fnum));

    if (ret != 0)
    {
        FLOG_WARNING("Set PerConnULThroughput failed\n");
    }



    tmp_fnum = (float) ( (ul_mgmt_total_bytes - tmp_ul_mgmt_total_bytes) * 8 /
                         (float)(METRIC_UPDATE_PERIOD * 1024) );

    ret = set_global_param("ULMgmtThroughput", (void *)&(tmp_fnum));

    if (ret != 0)
    {
        FLOG_WARNING("Set ULMgmtThroughput failed\n");
    }



    tmp_fnum = (float) ( (dl_mgmt_total_bytes - tmp_dl_mgmt_total_bytes) * 8 /
                         (float)(METRIC_UPDATE_PERIOD * 1024) );

    ret = set_global_param("DLMgmtThroughput", (void *)&(tmp_fnum));

    if (ret != 0)
    {
        FLOG_WARNING("Set DLMgmtThroughput failed\n");
    }



    tmp_inum = ul_total_connection + dl_total_connection;

    ret = set_global_param("ConnectionCount", (void *)&(tmp_inum));

    if (ret != 0)
    {
        FLOG_WARNING("Set ConnectionCount failed\n");
    }

    ret = set_global_param("PDUTotalCount", (void *)&(pdu_total_count));

    if (ret != 0)
    {
        FLOG_WARNING("Set PDUTotalCount failed\n");
    }


    tmp_dl_total_bytes = dl_total_bytes;
    tmp_ul_total_bytes = ul_total_bytes;
    tmp_ul_mgmt_total_bytes = ul_mgmt_total_bytes;
    tmp_dl_mgmt_total_bytes = dl_mgmt_total_bytes;

    ret = get_global_param ("METRIC_UPDATE_DURATION", & (duration));

    if (ret != 0)
    {
        FLOG_WARNING ("get parameters METRIC_UPDATE_DURATION error\n");
    }
    
    #ifdef _NEW_TRANS_ENABLE_
    
    /*Do not add timer again*/    
    #else
    struct timeval st_time_val;
    void* p_timer_id = NULL;
    struct trans_timer_msg_info st_msg_info;
    
    gettimeofday(&st_time_val, NULL);
    
    /*Timeout -- 10s*/
    st_time_val.tv_sec = st_time_val.tv_sec + duration;
    //st_time_val.tv_usec = st_time_val.tv_usec;

    st_msg_info.us_serial_number = 0;
    st_msg_info.uw_src_moudle = TRANS_MOUDLE_BS;
    st_msg_info.uc_block_flag = TRANS_QUENE_BLOCK;
    st_msg_info.f_callback = update_metric_func;
    st_msg_info.p_user_info = NULL;

    ret = trans_timer_add(&st_time_val,
                             update_metric_func,
                             &st_msg_info,
                             sizeof (struct trans_timer_msg_info),
                             &st_msg_info,
                             &p_timer_id);

    if (TRANS_SUCCESS != ret)
    {
        FLOG_ERROR("Call trans_timer_add error! uw_ret = %d\r\n", ret);
        return 1;
    }

    #endif

    (void) p_msg;
    (void) len;
    (void) p_msg_info;
    (void) tmp_ul_app_total_bytes;
    (void) tmp_dl_app_total_bytes;

    FLOG_DEBUG("Update metric successfully\n");

    return 0;

}

