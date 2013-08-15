/* ----------------------------------------------------------------------------
 * IBM Confidential

 IBM Wireless M2M Platform

 (C)Copyright IBM Corp. 2009, 2010, 2011

 All Rights Reserved.

 File Name: trans_proc.c

 Change Activity:

 Date             Description of Change                            By
 -----------      ---------------------                            --------
 12-May 2011       Created                                         Zhu, Zhenbo

 ----------------------------------------------------------------------------
 PROLOG END TAG zYx                                                         */

#include <stdio.h>
#include <pthread.h>
#include <arpa/inet.h>
#include <sys/time.h>

#include "global.h"
#include "queue_util.h"
#include "flog.h"
#include "sem_util.h"
#include "bs_cfg.h"

#include "trans_proc.h"
#include "rru_proc.h"
#include "rru_adapter.h"
#include "trans.h"

#include "phy_dl_tx_interface.h"
#include "phy_proc.h"

/* Agent code */
#include "trans.h"
#include "trans_agent.h"
#include "trans_rrh.h"
#include "trans_debug.h"

#include "monitor_proc.h"

//#define _TX_RAW_DUMP_

struct trans_contorl_config g_trans_control_param;

static int get_trans_config (struct trans_contorl_config * trans_config);


#ifndef _SIM_RRH_

pthread_t g_wma_agent_conn_thd = 0;
static int   g_wma_connected = 0;

void *process_wma_agent_connect (void *arg __attribute__ ((unused)));

int connect_agent (void)
{
    pthread_attr_t tattr;

    pthread_attr_init (&tattr);

    pthread_create (&g_wma_agent_conn_thd, NULL, process_wma_agent_connect, NULL);

    pthread_attr_destroy (&tattr);

    return 0;
}

int close_agent (void)
{
    if ( g_wma_agent_conn_thd != 0 )
    {
        if ((g_wma_connected != 1))
        {
            pthread_cancel (g_wma_agent_conn_thd);
        }

        pthread_join (g_wma_agent_conn_thd, NULL);
    }
    
    g_wma_agent_conn_thd = 0;

    return 0;
}

int wait_agent(void)
{
    if ( g_wma_agent_conn_thd != 0 )
    {
        pthread_join (g_wma_agent_conn_thd, NULL);
    }

    return g_wma_connected;
}

void *process_wma_agent_connect (void *arg __attribute__ ((unused)))
{
    u_int32_t uw_ret = 0;
    
    if (pthread_setcanceltype (PTHREAD_CANCEL_ASYNCHRONOUS, NULL) != 0)
    {
        FLOG_WARNING ("Set pthread cancel");
        return NULL;
    }

    FLOG_INFO ("Connecting to WMA Agent\n");

    /*Connect to Agent*/
    uw_ret = trans_connect_agent();
    /*Error*/
    if (TRANS_SUCCESS != uw_ret)
    {
#if 0
        FLOG_ERROR("Call trans_connect_agent error! uw_ret = %d\r\n", uw_ret);
        FLOG_ERROR ("Connected to WMA Error\n");
#endif
        return NULL;
    }

    g_wma_connected = 1;

    return NULL;
}
#endif

int init_trans(char * device_id, char * agent_id)
{
    u_int32_t uw_ret = 0;
    int32_t w_ret = 0;

    struct trans_init_info st_init_info;
//    struct trans_thread_id st_thread_id;

    FLOG_INFO("Initial Trans utilites...\r\n");

    if (get_trans_config (&g_trans_control_param) != 0)
    {
        FLOG_ERROR ("Configure TRANS parameters failed");
        return 1;
    }

    memset(&st_init_info, 0, sizeof(struct trans_init_info));

    st_init_info.uc_server_id = g_trans_control_param.bbu_id;   /*SERVER ID*/

    /*RRU  IP*/
    st_init_info.uw_rrh_ip_addr = htonl(g_trans_control_param.dst_ip);
    /*RRU  mask*/
    st_init_info.uw_rrh_mask_addr = htonl(g_trans_control_param.rru_nic_mask);
    /*SERVER  IP*/
    st_init_info.uw_ser_ip_addr = htonl(g_trans_control_param.src_ip);

    st_init_info.us_ser_tcp_port = g_trans_control_param.bbu_tcp_port; /*SERVER  TCP Port*/
    st_init_info.us_ser_data_port = g_trans_control_param.src_port; /*SERVER  I/Q  Data Port*/
    st_init_info.uw_ser_broc_addr = htonl(g_trans_control_param.rru_broadcast_ip);/*SERVER BROADCAST IP */
    st_init_info.uw_rru_id = g_trans_control_param.rru_id;   /*RRU ID*/

    st_init_info.us_agent_tcp_port = g_trans_control_param.agent_server_port;
    st_init_info.uw_agent_ip_addr = htonl(g_trans_control_param.agent_ip);
    st_init_info.uw_agent_hb_time = g_trans_control_param.agent_heartbeat_timeout;

    st_init_info.us_monitor_port = g_trans_control_param.monitor_server_port;

    //strcpy((char *)st_init_info.a_agent_device_id, "188");
    strcpy((char *)st_init_info.a_agent_device_id, device_id);

    //strcpy((char *)st_init_info.a_agent_id, "0");
    strcpy((char *)st_init_info.a_agent_id, agent_id);

    strcpy((char *)st_init_info.a_rrh_nic_if, g_trans_control_param.rru_nic_if);
    
    //memset(st_init_info.a_mac, 11, 6);
    memcpy(st_init_info.a_mac, g_trans_control_param.my_mac, 6);

    memcpy(st_init_info.a_rrh_mac, g_trans_control_param.rru_mac, 6);

    uw_ret = trans_init(&st_init_info);
    if (0 != uw_ret)
    {
        FLOG_ERROR("Call trans_init error! uw_ret = %d\r\n", uw_ret);
        return 1;
    }
    
    w_ret = init_monitor();
    if (0 != w_ret)
    {
        FLOG_ERROR("Call init_monitor error! uw_ret = %d\r\n", uw_ret);
    }

    return 0;
}

int close_trans (void)
{
    int ret = 0;

#ifndef _SIM_RRH_
/*
    if ( close_rrh() != 0)
    {
        FLOG_ERROR ("Close RRH faild");
    }
*/
    if ( close_agent() != 0)
    {
        FLOG_ERROR ("Close BS_AGENT_CONNECT faild");
    }

    if ( trans_release() != 0)
    {
        FLOG_ERROR ("Close BS_TRANS faild");
    }
#endif

    return ret;

}


#ifdef _NEW_TRANS_ENABLE_


#ifdef _NEW_TRANS_GREN_ENABLE_

int set_rrh_basic_config(void)
{
    u_int32_t uw_ret = 0;
    int w_ret = 0;
    int a_value[2] = {0};
     
    int ad_cmd_time = 5000;

    FLOG_INFO("Basic config start...\r\n");

    a_value[0] = g_trans_control_param.rru_byte_order;
    
    uw_ret = trans_send_bs_msg_to_rrh(TRANS_SEND_RRH_CFG_BYTE_ORDER, 1, a_value);
    if (TRANS_SUCCESS != uw_ret)
    {
        FLOG_ERROR("Call trans_send_bs_msg_to_rrh for CFG_BYTE_ORDER error! uw_ret = %d\r\n", uw_ret);
    
        return TRANS_FAILD;
    } 
    
    /*QUERY_POWER*/
    uw_ret = trans_send_bs_msg_to_rrh(TRANS_SEND_RRH_QUERY_POWER, 0, a_value);
    if (0 != uw_ret)
    {
        FLOG_ERROR("Call trans_send_bs_msg_to_rrh for quary QUERY_POWER error! uw_ret = %d\r\n", uw_ret);
    
        return 1;
    }
    
    /*ADVANCED_CMD_TIM*/
    w_ret = get_global_param ("ADVANCED_CMD_TIME", & ( ad_cmd_time ));
    
    if (w_ret != 0)
    {
        FLOG_WARNING ("get parameters SYMBOL_NUM_IN_UL_FRAME error\n");
        ad_cmd_time = 5000;
    }
    
    a_value[0] = ad_cmd_time;
    a_value[1] = ad_cmd_time;
    
    uw_ret = trans_send_bs_msg_to_rrh(TRANS_SEND_RRH_CFG_DL_PRESEND_TIME, 1, a_value);
    
    if (0 != uw_ret)
    {
        FLOG_ERROR("Call trans_send_bs_msg_to_rrh for DL_PRESEND_TIME error");
        return 1;
    }

    
    /*GPS_ENABLE*/
    a_value[0] = g_trans_control_param.enable_gps;
    
    uw_ret = trans_send_bs_msg_to_rrh(TRANS_SEND_RRH_CFG_GPS_ENABLE, 1, a_value);
    if (TRANS_SUCCESS != uw_ret)
    {
        FLOG_ERROR("Call trans_send_bs_msg_to_rrh for CFG_GPS_ENABLE error! uw_ret = %d\r\n", uw_ret);
    
        return TRANS_FAILD;
    }
    
    if (g_trans_control_param.enable_gps == 1)
    {
        uw_ret = trans_send_bs_msg_to_rrh(TRANS_SEND_RRH_QUERY_GPS_LOCK, 1, a_value);
        if (TRANS_SUCCESS != uw_ret)
        {
            FLOG_ERROR("Call trans_send_bs_msg_to_rrh for QUERY_GPS_LOCK error! uw_ret = %d\r\n", uw_ret);
    
            return TRANS_FAILD;
        }
    
        if (1 != a_value[0])
        {
            FLOG_INFO("The GPS is not locked. %d\r\n", a_value[0]);
        }
    
        while (a_value[0] != 1)
        {
            uw_ret = trans_send_bs_msg_to_rrh(TRANS_SEND_RRH_QUERY_GPS_LOCK, 1, a_value);
            if (TRANS_SUCCESS != uw_ret)
            {
                FLOG_ERROR("Call trans_send_bs_msg_to_rrh for CFG_GPS_ENABLE error! uw_ret = %d\r\n", uw_ret);
    
                return TRANS_FAILD;
            }
            sleep(3);
            FLOG_INFO("GPS locking.....\r\n");
        }
    
        FLOG_INFO("GPS is locked.\r\n");
    }

    
    /*GPS INFO*/
    uw_ret = trans_send_bs_msg_to_rrh(TRANS_SEND_RRH_QUERY_GPS, 0, a_value);
    if (TRANS_SUCCESS != uw_ret)
    {
        FLOG_ERROR("Call trans_send_bs_msg_to_rrh for quary GPS error! uw_ret = %d\r\n", uw_ret);
    
        return TRANS_FAILD;
    }
    
    /*CFG_AGC*/
    a_value[0] = g_trans_control_param.enable_agc;                
    uw_ret = trans_send_bs_msg_to_rrh(TRANS_SEND_RRH_CFG_AGC_ENABLE, 1, a_value);
    if (0 != uw_ret)
    {
        FLOG_ERROR("Call trans_send_bs_msg_to_rrh for CFG_AGC error! uw_ret = %d\r\n", uw_ret);
        //g_rrh_config_state = SET_RRH_CONFIG_STEP_3;
        return 1;
    
    }
    
    /*CFG_CH_RX_PGC*/
    if (g_trans_control_param.enable_agc == 0)
    {
        a_value[0] = g_trans_control_param.chan1_rx_pgc;
        a_value[1] = g_trans_control_param.chan2_rx_pgc;
        uw_ret = trans_send_bs_msg_to_rrh(TRANS_SEND_RRH_CFG_CH_RX_PGC, 2, a_value);
        if (0 != uw_ret)
        {
            FLOG_ERROR("Call trans_send_bs_msg_to_rrh for CFG_CH_RX_PGC error! uw_ret = %d\r\n", uw_ret);
            //g_rrh_config_state = SET_RRH_CONFIG_STEP_3;
            return 1;
        }
    }
     
    /*CFG_PA_SWITCH*/
    a_value[0] = g_trans_control_param.chan1_pa_enable;
    a_value[1] = g_trans_control_param.chan2_pa_enable;
    
    uw_ret = trans_send_bs_msg_to_rrh(TRANS_SEND_RRH_CFG_PA_SWITCH, 2, a_value);
    if (TRANS_SUCCESS != uw_ret)
    {
        FLOG_ERROR("Call trans_send_bs_msg_to_rrh for CFG_PA_SWITCH error! uw_ret = %d\r\n", uw_ret);
    
        return TRANS_FAILD;
    }   

    /*CFG_RX_LEN*/
    a_value[0] = g_trans_control_param.rru_rx_len;                
    uw_ret = trans_send_bs_msg_to_rrh(TRANS_SEND_RRH_CFG_RX_LEN, 1, a_value);
    if (0 != uw_ret)
    {
        FLOG_ERROR("Call trans_send_bs_msg_to_rrh for CFG_RX_LEN error! uw_ret = %d\r\n", uw_ret);
        return 1;
    }
    
    
    /*CFG_TX_LEN*/
    a_value[0] = g_trans_control_param.rru_tx_len;                
    uw_ret = trans_send_bs_msg_to_rrh(TRANS_SEND_RRH_CFG_TX_LEN, 1, a_value);
    if (0 != uw_ret)
    {
        FLOG_ERROR("Call trans_send_bs_msg_to_rrh for CFG_TX_LEN error! uw_ret = %d\r\n", uw_ret);
    
        return 1;
    
    }

    /*CFG_TTG_RTG*/
    a_value[0] = g_trans_control_param.rru_ttg;      
    a_value[1] = g_trans_control_param.rru_rtg;                
    uw_ret = trans_send_bs_msg_to_rrh(TRANS_SEND_RRH_CFG_TTG_RTG, 2, a_value);
    if (0 != uw_ret)
    {
        FLOG_ERROR("Call trans_send_bs_msg_to_rrh for CFG_TTG_RTG error! uw_ret = %d\r\n", uw_ret);
        return 1;
    }
    
    /** should get some RRH inforamtion and set to DB here */
    
    FLOG_INFO("Basic config end\r\n");
    
    return 0;

}

#else

int set_rrh_basic_config(void)
{
    u_int32_t uw_ret = 0;
    int a_value[2] = {0};
    
    FLOG_INFO("Basic config start...\r\n");
    
    a_value[0] = g_trans_control_param.rru_byte_order;

    uw_ret = trans_send_bs_msg_to_rrh(TRANS_SEND_RRH_CFG_BYTE_ORDER, 1, a_value);
    if (TRANS_SUCCESS != uw_ret)
    {
        FLOG_ERROR("Call trans_send_bs_msg_to_rrh for CFG_BYTE_ORDER error! uw_ret = %d\r\n", uw_ret);
    
        return TRANS_FAILD;
    }

    uw_ret = trans_send_bs_msg_to_rrh(TRANS_SEND_RRH_QUERY_GPS, 0, a_value);
    if (TRANS_SUCCESS != uw_ret)
    {
        FLOG_ERROR("Call trans_send_bs_msg_to_rrh for quary GPS error! uw_ret = %d\r\n", uw_ret);
    
        return TRANS_FAILD;
    }
    
    uw_ret = trans_send_bs_msg_to_rrh(TRANS_SEND_RRH_QUERY_POWER, 0, a_value);
    if (TRANS_SUCCESS != uw_ret)
    {
        FLOG_ERROR("Call trans_send_bs_msg_to_rrh for quary QUERY_POWER error! uw_ret = %d\r\n", uw_ret);
    
        return TRANS_FAILD;
    }
    
    a_value[0] = g_trans_control_param.enable_gps;
    
    uw_ret = trans_send_bs_msg_to_rrh(TRANS_SEND_RRH_CFG_GPS_ENABLE, 1, a_value);
    if (TRANS_SUCCESS != uw_ret)
    {
        FLOG_ERROR("Call trans_send_bs_msg_to_rrh for CFG_GPS_ENABLE error! uw_ret = %d\r\n", uw_ret);
    
        return TRANS_FAILD;
    }
    
    if (g_trans_control_param.enable_gps == 1)
    {
        uw_ret = trans_send_bs_msg_to_rrh(TRANS_SEND_RRH_QUERY_GPS_LOCK, 1, a_value);
        if (TRANS_SUCCESS != uw_ret)
        {
            FLOG_ERROR("Call trans_send_bs_msg_to_rrh for QUERY_GPS_LOCK error! uw_ret = %d\r\n", uw_ret);
    
            return TRANS_FAILD;
        }
    
        if (1 != a_value[0])
        {
            FLOG_INFO("The GPS is not locked. %d\r\n", a_value[0]);
        }
    
        while (a_value[0] != 1)
        {
            uw_ret = trans_send_bs_msg_to_rrh(TRANS_SEND_RRH_QUERY_GPS_LOCK, 1, a_value);
            if (TRANS_SUCCESS != uw_ret)
            {
                FLOG_ERROR("Call trans_send_bs_msg_to_rrh for CFG_GPS_ENABLE error! uw_ret = %d\r\n", uw_ret);
    
                return TRANS_FAILD;
            }
            sleep(3);
            FLOG_INFO("GPS locking.....\r\n");
        }
    
        FLOG_INFO("GPS is locked.\r\n");
    }
    
    uw_ret = trans_send_bs_msg_to_rrh(TRANS_SEND_RRH_QUERY_GPS, 0, a_value);
    if (TRANS_SUCCESS != uw_ret)
    {
        FLOG_ERROR("Call trans_send_bs_msg_to_rrh for quary GPS error! uw_ret = %d\r\n", uw_ret);
    
        return TRANS_FAILD;
    }
    
    uw_ret = trans_send_bs_msg_to_rrh(TRANS_SEND_RRH_QUERY_POWER, 0, a_value);
    if (TRANS_SUCCESS != uw_ret)
    {
        FLOG_ERROR("Call trans_send_bs_msg_to_rrh for quary QUERY_POWER error! uw_ret = %d\r\n", uw_ret);
    
        return TRANS_FAILD;
    }
    
    a_value[0] = g_trans_control_param.rru_output_power;
    
    uw_ret = trans_send_bs_msg_to_rrh(TRANS_SEND_RRH_CFG_OUTPUT_POWER, 1, a_value);
    if (TRANS_SUCCESS != uw_ret)
    {
        FLOG_ERROR("Call trans_send_bs_msg_to_rrh for CFG_OUTPUT_POWER error! uw_ret = %d\r\n", uw_ret);
    
        //return TRANS_FAILD;
    }
    
    a_value[0] = g_trans_control_param.chan1_pa_enable;
    a_value[1] = g_trans_control_param.chan2_pa_enable;
    
    uw_ret = trans_send_bs_msg_to_rrh(TRANS_SEND_RRH_CFG_PA_SWITCH, 2, a_value);
    if (TRANS_SUCCESS != uw_ret)
    {
        FLOG_ERROR("Call trans_send_bs_msg_to_rrh for CFG_PA_SWITCH error! uw_ret = %d\r\n", uw_ret);
    
        return TRANS_FAILD;
    }
    
    /** should get some RRH inforamtion and set to DB here */
    
    FLOG_INFO("Basic config end\r\n");
    
    return 0;
}


#endif

int connect_rrh(void)
{
    u_int32_t uw_ret = 0;
    //int a_value[2] = {0};
    int w_ret = 0;
   
    FLOG_INFO("Connecting to RRH...\r\n");

#ifndef _SIM_RRH_

    uw_ret = trans_thread_create();
    if (0 != uw_ret)
    {
        FLOG_ERROR("Call trans_thread_create error! uw_ret = %d\r\n", uw_ret);
    
        return 1;
    }

    /*Connect to RRH*/
    uw_ret = trans_connect_rrh();
    if (0 != uw_ret)
    {
        FLOG_ERROR("Call trans_connect_rrh error! uw_ret = %d\r\n", uw_ret);
        return 1;
    }

    #ifndef _OLD_RRH_

    w_ret = set_rrh_basic_config();

    if (0 != uw_ret)
    {
        FLOG_ERROR("Call set_rrh_basic_config error! uw_ret = %d\r\n", uw_ret);
    
        return 1;
    }
    
        #if 0
        uw_ret = trans_send_bs_msg_to_rrh(TRANS_SEND_RRH_QUERY_GPS, 0, a_value);
        if (TRANS_SUCCESS != uw_ret)
        {
            FLOG_ERROR("Call trans_send_bs_msg_to_rrh for quary GPS error! uw_ret = %d\r\n", uw_ret);

            return TRANS_FAILD;
        }

        uw_ret = trans_send_bs_msg_to_rrh(TRANS_SEND_RRH_QUERY_POWER, 0, a_value);
        if (TRANS_SUCCESS != uw_ret)
        {
            FLOG_ERROR("Call trans_send_bs_msg_to_rrh for quary QUERY_POWER error! uw_ret = %d\r\n", uw_ret);

            return TRANS_FAILD;
        }

        a_value[0] = g_trans_control_param.enable_gps;
        
        uw_ret = trans_send_bs_msg_to_rrh(TRANS_SEND_RRH_CFG_GPS_ENABLE, 1, a_value);
        if (TRANS_SUCCESS != uw_ret)
        {
            FLOG_ERROR("Call trans_send_bs_msg_to_rrh for CFG_GPS_ENABLE error! uw_ret = %d\r\n", uw_ret);

            return TRANS_FAILD;
        }

        if (g_trans_control_param.enable_gps == 1)
        {
            uw_ret = trans_send_bs_msg_to_rrh(TRANS_SEND_RRH_QUERY_GPS_LOCK, 1, a_value);
            if (TRANS_SUCCESS != uw_ret)
            {
                FLOG_ERROR("Call trans_send_bs_msg_to_rrh for QUERY_GPS_LOCK error! uw_ret = %d\r\n", uw_ret);
        
                return TRANS_FAILD;
            }

            if (1 != a_value[0])
            {
                FLOG_INFO("The GPS is not locked. %d\r\n", a_value[0]);
            }

            while (a_value[0] != 1)
            {
                uw_ret = trans_send_bs_msg_to_rrh(TRANS_SEND_RRH_QUERY_GPS_LOCK, 1, a_value);
                if (TRANS_SUCCESS != uw_ret)
                {
                    FLOG_ERROR("Call trans_send_bs_msg_to_rrh for CFG_GPS_ENABLE error! uw_ret = %d\r\n", uw_ret);

                    return TRANS_FAILD;
                }
                sleep(3);
                FLOG_INFO("GPS locking.....\r\n");
            }

            FLOG_INFO("GPS is locked.\r\n");
        }

        uw_ret = trans_send_bs_msg_to_rrh(TRANS_SEND_RRH_QUERY_GPS, 0, a_value);
        if (TRANS_SUCCESS != uw_ret)
        {
            FLOG_ERROR("Call trans_send_bs_msg_to_rrh for quary GPS error! uw_ret = %d\r\n", uw_ret);

            return TRANS_FAILD;
        }

        uw_ret = trans_send_bs_msg_to_rrh(TRANS_SEND_RRH_QUERY_POWER, 0, a_value);
        if (TRANS_SUCCESS != uw_ret)
        {
            FLOG_ERROR("Call trans_send_bs_msg_to_rrh for quary QUERY_POWER error! uw_ret = %d\r\n", uw_ret);

            return TRANS_FAILD;
        }

        a_value[0] = g_trans_control_param.rru_output_power;

        uw_ret = trans_send_bs_msg_to_rrh(TRANS_SEND_RRH_CFG_OUTPUT_POWER, 1, a_value);
        if (TRANS_SUCCESS != uw_ret)
        {
            FLOG_ERROR("Call trans_send_bs_msg_to_rrh for CFG_OUTPUT_POWER error! uw_ret = %d\r\n", uw_ret);

            //return TRANS_FAILD;
        }

        a_value[0] = g_trans_control_param.chan1_pa_enable;
        a_value[1] = g_trans_control_param.chan2_pa_enable;

        uw_ret = trans_send_bs_msg_to_rrh(TRANS_SEND_RRH_CFG_PA_SWITCH, 2, a_value);
        if (TRANS_SUCCESS != uw_ret)
        {
            FLOG_ERROR("Call trans_send_bs_msg_to_rrh for CFG_PA_SWITCH error! uw_ret = %d\r\n", uw_ret);

            return TRANS_FAILD;
        }
        #endif
    
    #endif

#else
    (void) uw_ret;
    //(void) a_value;
#endif

    /** should get some RRH inforamtion and set to DB here */

    FLOG_INFO("Connected to RRH\r\n");

    if (ethrru_init () != 0)
    {
        FLOG_ERROR ("Starting RRU failed");
        return 1;
    }

    FLOG_INFO("Eth RRU initilized\r\n");

    return 0;
}


#else
int connect_rrh(void)
{
    u_int32_t uw_ret = 0;
    int a_value[2] = {0};
   
    FLOG_INFO("Connecting to RRH...\r\n");

#ifndef _SIM_RRH_

    /*Connect to RRH*/
    uw_ret = trans_connect_rrh();
    if (TRANS_SUCCESS != uw_ret)
    {
        FLOG_ERROR("Call trans_connect_rrh error! uw_ret = %d\r\n", uw_ret);
        return TRANS_FAILD;
    }

    uw_ret = trans_thread_create();
    if (TRANS_SUCCESS != uw_ret)
    {
        FLOG_ERROR("Call trans_thread_create error! uw_ret = %d\r\n", uw_ret);

        return TRANS_FAILD;
    }

#ifndef _OLD_RRH_
    a_value[0] = g_trans_control_param.enable_gps;
    
    uw_ret = trans_send_msg_to_rrh(TRANS_SEND_CFG_GPS_ENABLE, 1, a_value);
    if (TRANS_SUCCESS != uw_ret)
    {
        FLOG_ERROR("Call trans_send_msg_to_rrh for CFG_GPS_ENABLE error! uw_ret = %d\r\n", uw_ret);

        return TRANS_FAILD;
    }

    if (g_trans_control_param.enable_gps == 1)
    {
        uw_ret = trans_send_msg_to_rrh(TRANS_SEND_QUERY_GPS_LOCK, 1, a_value);
        if (TRANS_SUCCESS != uw_ret)
        {
            FLOG_ERROR("Call trans_send_msg_to_rrh for CFG_GPS_ENABLE error! uw_ret = %d\r\n", uw_ret);
    
            return TRANS_FAILD;
        }

        if (1 != a_value[0])
        {
            FLOG_INFO("The GPS is not locked. %d\r\n", a_value[0]);
        }

        while (a_value[0] != 1)
        {
            uw_ret = trans_send_msg_to_rrh(TRANS_SEND_QUERY_GPS_LOCK, 1, a_value);
            if (TRANS_SUCCESS != uw_ret)
            {
                FLOG_ERROR("Call trans_send_msg_to_rrh for CFG_GPS_ENABLE error! uw_ret = %d\r\n", uw_ret);

                return TRANS_FAILD;
            }
            sleep(3);
            FLOG_INFO("GPS locking.....\r\n");
        }

        FLOG_INFO("GPS is locked.\r\n");
    }

    uw_ret = trans_send_msg_to_rrh(TRANS_SEND_INIT_QUERY, 0, NULL);
    if (TRANS_SUCCESS != uw_ret)
    {
        FLOG_ERROR("Call trans_send_msg_to_rrh for quary error! uw_ret = %d\r\n", uw_ret);

        return TRANS_FAILD;
    }

    a_value[0] = g_trans_control_param.rru_output_power;

    uw_ret = trans_send_msg_to_rrh(TRANS_SEND_CFG_OUTPUT_POWER, 1, a_value);
    if (TRANS_SUCCESS != uw_ret)
    {
        FLOG_ERROR("Call trans_send_msg_to_rrh for CFG_OUTPUT_POWER error! uw_ret = %d\r\n", uw_ret);

        //return TRANS_FAILD;
    }

    a_value[0] = g_trans_control_param.chan1_pa_enable;
    a_value[1] = g_trans_control_param.chan2_pa_enable;

    uw_ret = trans_send_msg_to_rrh(TRANS_SEND_CFG_PA_SWITCH, 2, a_value);
    if (TRANS_SUCCESS != uw_ret)
    {
        FLOG_ERROR("Call trans_send_msg_to_rrh for CFG_PA_SWITCH error! uw_ret = %d\r\n", uw_ret);

        return TRANS_FAILD;
    }
#endif

    #else
    (void) uw_ret;
    (void) a_value;
    #endif

    /** should get some RRH inforamtion and set to DB here */

    FLOG_INFO("Connected to RRH\r\n");

    if (ethrru_init () != 0)
    {
        FLOG_ERROR ("Starting RRU failed");
        return 1;
    }

    FLOG_INFO("Eth RRU initilized\r\n");

    return 0;
}

#endif

int close_rrh(void)
{
    int ret = 0;

    if ( ethrru_close() != 0 )
    {
        FLOG_ERROR ("Close Eth RRU faild");
        ret = 1;
    }

    return ret;
}

static int get_trans_config (struct trans_contorl_config * trans_config)
{
    int ret;
    char tmp_string[128];
    int value = 0;
    
    char *token;
    int tmp_mac;
    char *search = ":";
    int i = 0;

    ret = get_global_param ("BBU_SERVER_ID", & ( trans_config->bbu_id ));

    if (ret != 0)
    {
        FLOG_WARNING ("get parameters BBU_SERVER_ID error\n");
    }

    ret = get_global_param ("RRU_BROADCAST_IP", tmp_string);

    if (ret != 0)
    {
        FLOG_WARNING ("get parameters RRU_BROADCAST_IP error\n");
    }

    trans_config->rru_broadcast_ip = ntohl (inet_addr (tmp_string));

    ret = get_global_param ("AGENT_HEARTBEAT_TIMEOUT", & (value) );

    if (ret != 0)
    {
        FLOG_WARNING ("get parameters AGENT_HEARTBEAT_TIMEOUT error\n");
    }

    trans_config->agent_heartbeat_timeout = (unsigned int)value;;

    ret = get_global_param ("RRU_ID", & ( value ));

    if (ret != 0)
    {
        FLOG_WARNING ("get parameters RRU_ID error\n");
    }

    trans_config->rru_id = (unsigned int)value;

    ret = get_global_param ("BBU_NIC_IP", tmp_string);

    if (ret != 0)
    {
        FLOG_WARNING ("get parameters BBU_NIC_IP error\n");
    }

    trans_config->src_ip = ntohl (inet_addr (tmp_string));

    ret = get_global_param ("RRU_NIC_IP", tmp_string);

    if (ret != 0)
    {
        FLOG_WARNING ("get parameters RRU_NIC_IP error\n");
    }

    trans_config->dst_ip = ntohl (inet_addr (tmp_string));

    ret = get_global_param ("BBU_NIC_DATA_PORT", & ( value ));

    if (ret != 0)
    {
        FLOG_WARNING ("get parameters BBU_NIC_DATA_PORT error\n");
    }

    trans_config->src_port = (unsigned short)(value + trans_config->bbu_id * 2);

    ret = get_global_param ("RRU_NIC_DATA_PORT", & ( value ));

    if (ret != 0)
    {
        FLOG_WARNING ("get parameters RRU_NIC_DATA_PORT error\n");
    }

    trans_config->dst_port = (unsigned short) value;

    ret = get_global_param ("RRU_NIC_MASK", tmp_string);

    if (ret != 0)
    {
        FLOG_WARNING ("get parameters RRU_NIC_MASK error\n");
    }

    trans_config->rru_nic_mask = ntohl (inet_addr (tmp_string));

    ret = get_global_param ("BBU_TCP_PORT", & ( value ));

    if (ret != 0)
    {
        FLOG_WARNING ("get parameters BBU_TCP_PORT error\n");
    }

    trans_config->bbu_tcp_port = (unsigned short) (value + trans_config->bbu_id * 2);

    ret = get_global_param ("RRU_NIC_IF", tmp_string);

    if (ret != 0)
    {
        FLOG_WARNING ("get parameters RRU_NIC_MASK error\n");
    }

    strcpy(trans_config->rru_nic_if, tmp_string);


    ret = get_global_param ("AGENT_IP", tmp_string);

    if (ret != 0)
    {
        FLOG_WARNING ("get parameters AGENT_IP error\n");
    }

    trans_config->agent_ip = ntohl (inet_addr (tmp_string));

    ret = get_global_param ("AGENT_SERVER_PORT", & ( value ));

    if (ret != 0)
    {
        FLOG_WARNING ("get parameters AGENT_SERVER_PORT error\n");
    }

    trans_config->agent_server_port = (unsigned short) value;

    ret = get_global_param ("MONITOR_SERVER_PORT", & ( value ));

    if (ret != 0)
    {
        FLOG_WARNING ("get parameters MONITOR_SERVER_PORT error\n");
    }

    trans_config->monitor_server_port = (unsigned short) (value + trans_config->bbu_id * 2);

    ret = get_global_param ("ENABLE_RRU_GPS", & ( value ));

    if (ret != 0)
    {
        FLOG_WARNING ("get parameters ENABLE_RRU_GPS error\n");
    }

    trans_config->enable_gps = value;

    ret = get_global_param ("RRU_OUTPUT_POWER", & ( value ));

    if (ret != 0)
    {
        FLOG_WARNING ("get parameters RRU_OUTPUT_POWER error\n");
    }

    trans_config->rru_output_power = value;

    ret = get_global_param ("CHAN1_PA_ENABLE", & ( value ));

    if (ret != 0)
    {
        FLOG_WARNING ("get parameters CHAN1_PA_ENABLE error\n");
    }

    trans_config->chan1_pa_enable = value;

    ret = get_global_param ("CHAN2_PA_ENABLE", & ( value ));

    if (ret != 0)
    {
        FLOG_WARNING ("get parameters CHAN2_PA_ENABLE error\n");
    }

    trans_config->chan2_pa_enable = value;

    ret = get_global_param ("IS_RX_FRAME_LEN", & ( value ));

    if (ret != 0)
    {
        FLOG_WARNING ("get parameters IS_RX_FRAME_LEN error\n");
    }

    trans_config->is_rxfrm_len = value;

    ret = get_global_param ("RRU_RX_LEN", & ( value ));

    if (ret != 0)
    {
        FLOG_WARNING ("get parameters RRU_RX_LEN error\n");
    }

    trans_config->rru_rx_len = value;

    ret = get_global_param ("RRU_TX_LEN", & ( value ));

    if (ret != 0)
    {
        FLOG_WARNING ("get parameters RRU_TX_LEN error\n");
    }

    trans_config->rru_tx_len = value;

    ret = get_global_param ("RRU_TTG", & ( value ));
    
    if (ret != 0)
    {
        FLOG_WARNING ("get parameters RRU_TTG error\n");
    }
    
    trans_config->rru_ttg = value;
    
    ret = get_global_param ("RRU_RTG", & ( value ));
    
    if (ret != 0)
    {
        FLOG_WARNING ("get parameters RRU_RTG error\n");
    }
    
    trans_config->rru_rtg = value;


    ret = get_global_param ("ENABLE_RRU_AGC", & ( value ));

    if (ret != 0)
    {
        FLOG_WARNING ("get parameters ENABLE_RRU_AGC error\n");
    }

    trans_config->enable_agc = value;

    ret = get_global_param ("CH0_RX_PGC", & ( value ));

    if (ret != 0)
    {
        FLOG_WARNING ("get parameters CH0_RX_PGC error\n");
    }

    trans_config->chan1_rx_pgc = value;

    ret = get_global_param ("CH1_RX_PGC", & ( value ));

    if (ret != 0)
    {
        FLOG_WARNING ("get parameters CH1_RX_PGC error\n");
    }

    trans_config->chan2_rx_pgc = value;

    /*0*/
    ret = get_global_param ("CARRIER0_ENABLE", & ( value ));
    
    if (ret != 0)
    {
        FLOG_WARNING ("get parameters CARRIER0_ENABLE error\n");
    }
    
    trans_config->carrier_info[0].carrier_enable = value;
    
    ret = get_global_param ("CARRIER0_FREQ", & ( value ));
    
    if (ret != 0)
    {
        FLOG_WARNING ("get parameters CARRIER0_FREQ error\n");
    }
    
    trans_config->carrier_info[0].carrier_freq = value;
    
    ret = get_global_param ("CARRIER0_BW", & ( value ));
    
    if (ret != 0)
    {
        FLOG_WARNING ("get parameters CARRIER0_BW error\n");
    }
    
    trans_config->carrier_info[0].carrier_bw = value;
    
    ret = get_global_param ("CARRIER0_POWER", & ( value ));
    
    if (ret != 0)
    {
        FLOG_WARNING ("get parameters CARRIER0_POWER error\n");
    }
    
    trans_config->carrier_info[0].carrier_pwr = value;
    
    
    /*1*/
    ret = get_global_param ("CARRIER1_ENABLE", & ( value ));
    
    if (ret != 0)
    {
        FLOG_WARNING ("get parameters CARRIER1_ENABLE error\n");
    }
    
    trans_config->carrier_info[1].carrier_enable = value;
    
    ret = get_global_param ("CARRIER1_FREQ", & ( value ));
    
    if (ret != 0)
    {
        FLOG_WARNING ("get parameters CARRIER1_FREQ error\n");
    }
    
    trans_config->carrier_info[1].carrier_freq = value;
    
    ret = get_global_param ("CARRIER1_BW", & ( value ));
    
    if (ret != 0)
    {
        FLOG_WARNING ("get parameters CARRIER1_BW error\n");
    }
    
    trans_config->carrier_info[1].carrier_bw = value;
    
    ret = get_global_param ("CARRIER1_POWER", & ( value ));
    
    if (ret != 0)
    {
        FLOG_WARNING ("get parameters CARRIER1_POWER error\n");
    }
    
    trans_config->carrier_info[1].carrier_pwr = value;
    
    /*2*/
    ret = get_global_param ("CARRIER2_ENABLE", & ( value ));
    
    if (ret != 0)
    {
        FLOG_WARNING ("get parameters CARRIER2_ENABLE error\n");
    }
    
    trans_config->carrier_info[2].carrier_enable = value;
    
    ret = get_global_param ("CARRIER2_FREQ", & ( value ));
    
    if (ret != 0)
    {
        FLOG_WARNING ("get parameters CARRIER2_FREQ error\n");
    }
    
    trans_config->carrier_info[2].carrier_freq = value;
    
    ret = get_global_param ("CARRIER2_BW", & ( value ));
    
    if (ret != 0)
    {
        FLOG_WARNING ("get parameters CARRIER2_BW error\n");
    }
    
    trans_config->carrier_info[2].carrier_bw = value;
    
    ret = get_global_param ("CARRIER2_POWER", & ( value ));
    
    if (ret != 0)
    {
        FLOG_WARNING ("get parameters CARRIER2_POWER error\n");
    }
    
    trans_config->carrier_info[2].carrier_pwr = value;
    
    /*3*/
    ret = get_global_param ("CARRIER3_ENABLE", & ( value ));
    
    if (ret != 0)
    {
        FLOG_WARNING ("get parameters CARRIER3_ENABLE error\n");
    }
    
    trans_config->carrier_info[3].carrier_enable = value;
    
    ret = get_global_param ("CARRIER3_FREQ", & ( value ));
    
    if (ret != 0)
    {
        FLOG_WARNING ("get parameters CARRIER3_FREQ error\n");
    }
    
    trans_config->carrier_info[3].carrier_freq = value;
    
    ret = get_global_param ("CARRIER3_BW", & ( value ));
    
    if (ret != 0)
    {
        FLOG_WARNING ("get parameters CARRIER3_BW error\n");
    }
    
    trans_config->carrier_info[3].carrier_bw = value;
    
    ret = get_global_param ("CARRIER3_POWER", & ( value ));
    
    if (ret != 0)
    {
        FLOG_WARNING ("get parameters CARRIER3_POWER error\n");
    }
    
    trans_config->carrier_info[3].carrier_pwr = value;

    /*RRU_BYTE_ORDER*/
    ret = get_global_param ("RRU_BYTE_ORDER", & ( value ));
    
    if (ret != 0)
    {
        FLOG_WARNING ("get parameters RRU_BYTE_ORDER error\n");
    }
    
    trans_config->rru_byte_order = value;
    
    /*MAC*/
    ret = get_global_param ("MY_MAC", tmp_string);
    
    if (ret != 0)
    {
        FLOG_WARNING ("get parameters MY_MAC error\n");
    }

    token = strtok (tmp_string, search);
    sscanf(token, "%x", &(tmp_mac));    
    trans_config->my_mac[0] = (unsigned char)tmp_mac;
    
    for (i = 1; i < 6; i++)
    {
        token = strtok (NULL, search);
        sscanf(token, "%x", &(tmp_mac));
        
        trans_config->my_mac[i] = (unsigned char)tmp_mac;
    }
    
    ret = get_global_param ("RRU_MAC", tmp_string);
    
    if (ret != 0)
    {
        FLOG_WARNING ("get parameters RRU_MAC error\n");
    }
    
    token = strtok (tmp_string, search);
    sscanf(token, "%x", &(tmp_mac));    
    trans_config->rru_mac[0] = (unsigned char)tmp_mac;
    
    for (i = 1; i < 6; i++)
    {
        token = strtok (NULL, search);
        sscanf(token, "%x", &(tmp_mac));
        
        trans_config->rru_mac[i] = (unsigned char)tmp_mac;
    }

    return 0;
}

