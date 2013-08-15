/* ----------------------------------------------------------------------------
 IBM Confidential

 IBM Wireless M2M Platform

 (C)Copyright IBM Corp. 2009, 2010, 2011

 All Rights Reserved.

 File Name: trans.c

 Change Activity:

 Date             Description of Change                            By
 -----------      ---------------------                            --------
 23-Mar.2011      Created                                          E Wulan

 ----------------------------------------------------------------------------
 PROLOG END TAG zYx                                                         */


#include <sys/types.h>
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <syslog.h>
#include <signal.h>

#include <flog.h>
#ifndef TRANS_MONITOR_TEST_COMPILE
#include "queue_util.h"
#include "thd_util.h"
#endif

#include <trans.h>
#include <trans_common.h>
#include <trans_transaction.h>
#include <trans_list.h>
#include <trans_device.h>
#include <trans_timer.h>
#include <trans_agent.h>
#include <trans_wireless.h>
#include <trans_action.h>
#include <trans_rrh.h>
#include <trans_debug.h>
#include <trans_monitor.h>

#include <malloc.h>
#include <pthread.h>
//#include <semaphore.h>
//#include <sched.h>
//#include <mutex.h>

#include <sys/socket.h>
#include <netinet/in.h>

u_int32_t g_enable_metric = 0;

//pthread_mutex_t  g_ad_msg_thread_mutex;
//pthread_cond_t    g_ad_msg_thread_cond;

//int32_t g_trans_moudle_socket_fd[TRANS_MOUDLE_BUF] = {0};

/*If the thread has been creat*/
static u_int8_t    g_trans_thread_flag = 0;

//u_int8_t  *g_trans_rev_buf = NULL;
//u_int8_t  *g_trans_send_buf = NULL;

struct trans_thread_id  g_trans_thread_id;

struct trans_register_func  g_trans_register_exe_func[TRANS_REGISTER_FUN_BUF];
struct trans_register_func  g_trans_register_delete_func[TRANS_REGISTER_FUN_BUF];

u_int32_t trans_timer_thread();

u_int32_t turn_metric (u_int32_t on_off)
{
    if (on_off == 0)
    {
        g_enable_metric = 0;
    }else if (on_off == 1)
    {
        g_enable_metric = 1;
    }else if (on_off == 2)
    {
        return g_enable_metric;
    }else
    {
        return 2;
    }

    return 0;
}

/*****************************************************************************+
* Function: trans_init()
* Description: init
* Parameters:
*           *p_init_info : init info
* Return Values:
*           TRANS_FAILD : 1
*           TRANS_SUCCESS  : 0
*
*  
*  Data:    2011-03-23
* 
+*****************************************************************************/
u_int32_t trans_init(struct trans_init_info *p_init_info)
{
    u_int32_t uw_ret = 0;
    u_int32_t uw_index = 0;

    if (NULL == p_init_info)
    {
        FLOG_ERROR("NULL PTR! \r\n");
        return TRANS_FAILD;
    }  
    
    signal(SIGPIPE,SIG_IGN);

    memset((u_int8_t*)&g_trans_thread_id, 0, sizeof(struct trans_thread_id));

    for (uw_index = 0; uw_index < TRANS_REGISTER_FUN_BUF; uw_index++)
    {
        g_trans_register_exe_func[uw_index].uc_use_flag = 0;
        g_trans_register_exe_func[uw_index].f_callback = NULL;
        
        g_trans_register_delete_func[uw_index].uc_use_flag = 0;
        g_trans_register_delete_func[uw_index].f_callback = NULL;
    }
    
    #if 0
    /* Allocate a memory.  */
    g_trans_rev_buf = (u_int8_t *)malloc(TRANS_REV_MSG_MAX_LEN);
    if (NULL == g_trans_rev_buf)
    {
        FLOG_ERROR("malloc g_trans_rev_buf error! \r\n");
        return TRANS_FAILD;   
    }
    
    g_trans_send_buf = (u_int8_t *)malloc(TRANS_SEND_MSG_MAX_LEN);
    if (NULL == g_trans_send_buf)
    {
        FLOG_ERROR("malloc g_trans_send_buf error! \r\n");
        return TRANS_FAILD;   
    } 
    #endif

    g_trans_rrh_socket = -1;
    g_trans_agent_socket.w_agent_socket = -1;

    uw_ret = trans_device_init(p_init_info->a_mac);
    /*Error*/
    if (TRANS_SUCCESS != uw_ret)
    {
        FLOG_ERROR("Call trans_device_init error! \r\n");
    
        return uw_ret;        
    }  

    uw_ret = trans_common_init();
    /*Error*/
    if (TRANS_SUCCESS != uw_ret)
    {
        FLOG_ERROR("Call trans_common_init error! \r\n");
    
        return uw_ret;        
    }  

    uw_ret = trans_transaction_init();
    /*Error*/
    if (TRANS_SUCCESS != uw_ret)
    {
        FLOG_ERROR("Call trans_transaction_init error! \r\n");
    
        return uw_ret;        
    } 

    uw_ret = trans_timer_init();
    /*Error*/
    if (TRANS_SUCCESS != uw_ret)
    {
        FLOG_ERROR("Call trans_timer_init error! \r\n");
    
        return uw_ret;        
    }  

    uw_ret = trans_action_init();
    /*Error*/
    if (TRANS_SUCCESS != uw_ret)
    {
        FLOG_ERROR("Call trans_action_init error! \r\n");
    
        return uw_ret;        
    } 
    
    #ifdef TRANS_RRH_COMPILE
    uw_ret = trans_rrh_init(p_init_info);
    /*Error*/
    if (TRANS_SUCCESS != uw_ret)
    {
        FLOG_ERROR("Call trans_rrh_init error! \r\n");

        return uw_ret;        
    } 
    #endif

    #ifdef TRANS_AGENT_COMPILE
    uw_ret = trans_agent_init(p_init_info);
    /*Error*/
    if (TRANS_SUCCESS != uw_ret)
    {
        FLOG_ERROR("Call trans_agent_init error! \r\n");
    
        return uw_ret;        
    }  
    #endif

    #ifdef TRANS_MONITOR_COMPILE
    uw_ret = trans_monitor_init(p_init_info);
    /*Error*/
    if (TRANS_SUCCESS != uw_ret)
    {
        FLOG_ERROR("Call trans_monitor_init error! \r\n");
    
        return uw_ret;        
    }  
    #endif


    #if (defined TRANS_BS_COMPILE) || (defined TRANS_MS_COMPILE)
    uw_ret = trans_wireless_init();
    /*Error*/
    if (TRANS_SUCCESS != uw_ret)
    {
        FLOG_ERROR("Call trans_bs_init error! \r\n");
    
        return uw_ret;        
    } 
    #endif

    /*Timer Thread*/
    uw_ret = trans_timer_thread();

    if (TRANS_SUCCESS != uw_ret)
    {
        FLOG_ERROR("Call trans_timer_thread error! uw_ret = %d\r\n", uw_ret);
        return uw_ret;
    }

    //LOG_INIT_CONSOLE_SYSLOG( "flog" )

    //LOG_INIT_CONSOLE_ONLY( "BS_Agent" )

    //LOG_INIT_SYSLOG_ONLY( "flog" )

    //memset((u_int8_t*)g_trans_moudle_socket_fd, 0, (sizeof(int32_t)*ADAPT_MOUDLE_BUF));

    //pthread_mutex_init(&g_trans_msg_thread_mutex, NULL);
    //pthread_cond_init(&g_trans_msg_thread_cond, NULL);

    return TRANS_SUCCESS;
}


/*****************************************************************************+
* Function: trans_connect_rrh()
* Description: 
* Parameters:
*           NONE
* Return Values:
*           TRANS_FAILD : 1
*           TRANS_SUCCESS  : 0
*  
*  Data:    2011-03-23
* 
+*****************************************************************************/
u_int32_t trans_connect_rrh()
{
    #ifdef TRANS_RRH_COMPILE
    u_int32_t uw_ret = 0;

    #if (defined TRANS_RRH_NEW_CONNECT) && ((defined TRANS_BS_COMPILE) || (defined TRANS_UI_COMPILE))
    int w_value = 0;
    #endif

    FLOG_DEBUG_TRANS(g_trans_debug_com, "Enter \r\n");

    #ifndef TRANS_RRH_NEW_CONNECT
    
    #ifdef TRANS_RRH_RAW_SOCKET
    uw_ret = trans_rrh_udp_raw_socket();
    /*Error*/
    if (TRANS_SUCCESS != uw_ret)
    {
        FLOG_ERROR("Call trans_rrh_udp_raw_socket error! uw_ret =%d\r\n", uw_ret);
        FLOG_ERROR("Failed to receive the broadcasting messages\n");
    
        return uw_ret;        
    } 
    
    #else
    
    uw_ret = trans_rrh_udp_socket();
    /*Error*/
    if (TRANS_SUCCESS != uw_ret)
    {
        FLOG_ERROR("Call trans_rrh_udp_socket error! uw_ret =%d\r\n", uw_ret);
        FLOG_ERROR("Failed to receive the broadcasting messages\n");
    
        return uw_ret;        
    } 

    #endif

    #endif

    uw_ret = trans_rrh_tcp_socket();
    /*Error*/
    if (TRANS_SUCCESS != uw_ret)
    {
        FLOG_ERROR("Call trans_rrh_tcp_socket error! uw_ret =%d\r\n", uw_ret);    
        return uw_ret;        
    } 

    //pthread_cond_signal(&g_trans_msg_thread_cond);

    /*Send HeartBeat Msg To RRH per 3s*/    
    uw_ret = trans_rrh_heartbeat_timer();
    /*Error*/
    if (0 != uw_ret)
    {
        FLOG_ERROR("Call trans_rrh_heartbeat_timer error! uw_ret =%d\r\n", uw_ret);
        //return ;
    }

    #if (defined TRANS_RRH_NEW_CONNECT) && ((defined TRANS_BS_COMPILE) || (defined TRANS_UI_COMPILE))
    /*Send message for IQ port*/
    w_value = g_trans_rrh_eqp_config.us_ser_data_udp_port;
    uw_ret = trans_wireless_send2_rrh_data_port(NULL, 1, &(w_value));
    /*Error*/
    if (0 != uw_ret)
    {
        FLOG_ERROR("Call trans_wireless_send2_rrh_data_port error! uw_ret =%d\r\n", uw_ret);
        return uw_ret;        
    }
    #endif
   
    FLOG_DEBUG_TRANS(g_trans_debug_com, "Exit \r\n");
    #endif
    
    return TRANS_SUCCESS;
}

/*****************************************************************************+
* Function: trans_connect_agent()
* Description: 
* Parameters:
*           NONE
* Return Values:
*           TRANS_FAILD : 1
*           TRANS_SUCCESS  : 0
*
*  
*  Data:    2011-04-23
* 
+*****************************************************************************/
u_int32_t trans_connect_agent()
{
    #ifdef TRANS_AGENT_COMPILE
    u_int32_t uw_ret = 0;
    u_int32_t uw_connect_num = 0;
    
    FLOG_DEBUG_TRANS(g_trans_debug_com, "Enter \r\n");

    if (0 == TRANS_AGENT_CONNECT_NUM)
    {
        uw_connect_num = 1;
        while (uw_connect_num)
        {
            uw_ret = trans_agent_tcp_socket();
            /*Error*/
            if (TRANS_SUCCESS != uw_ret)
            {
                FLOG_ERROR("Call trans_agent_tcp_socket error! uw_ret = %d \r\n", uw_ret);
            
            } 
            else
            {
                break;
        
            }

            sleep(3);
        } 
    }
    else
    {
        uw_connect_num = TRANS_AGENT_CONNECT_NUM;
        //uw_connect_num = 3;
        while (uw_connect_num)
        {
            uw_ret = trans_agent_tcp_socket();
            /*Error*/
            if (TRANS_SUCCESS != uw_ret)
            {
                FLOG_ERROR("Call trans_agent_tcp_socket error! uw_ret = %d \r\n", uw_ret);
            
            } 
            else
            {
                break;
        
            }
        
            uw_connect_num--;
            sleep(3);
        } 
    }

    if ((TRANS_SUCCESS != uw_ret) || (0 == uw_connect_num))
    {
        FLOG_ERROR("Connect to Agent error! uw_ret = %d\r\n", uw_ret);        
        return TRANS_FAILD;
    }

    uw_ret = trans_init_finish_inform_agent();        
    /*Error*/
    if (TRANS_SUCCESS != uw_ret)
    {
        FLOG_ERROR("Call trans_init_finish_inform_agent error! uw_ret = %d\r\n", uw_ret);        
        return TRANS_FAILD;
    }  
    
    FLOG_DEBUG_TRANS(g_trans_debug_com, "Exit \r\n");

    #else

    FLOG_WARNING("Aegnt Disabled!");

    #endif
    
    return TRANS_SUCCESS;
}

/*****************************************************************************+
* Function: trans_connect_peer()
* Description: 
* Parameters:
*           uw_ip_addr : Ip address
* Return Values:
*           TRANS_FAILD : 1
*           TRANS_SUCCESS  : 0
*
*  
*  Data:    2011-04-23
* 
+*****************************************************************************/
u_int32_t trans_connect_peer(u_int32_t uw_ip_addr, u_int16_t us_port)
{
    //#ifdef TRANS_MONITOR_COMPILE
    #if ((defined TRANS_BS_COMPILE) || (defined TRANS_MS_COMPILE)) && (defined TRANS_MONITOR_COMPILE)
    u_int32_t uw_ret = 0;

    FLOG_DEBUG_TRANS(g_trans_debug_com, "Enter \r\n");
    
    uw_ret = trans_wireless_tcp_socket(uw_ip_addr, us_port);
    /*Error*/
    if (TRANS_SUCCESS != uw_ret)
    {
        FLOG_ERROR("Call trans_rrh_tcp_socket error! uw_ret =%d\r\n", uw_ret);
    
        return uw_ret;        
    } 

    
    FLOG_DEBUG_TRANS(g_trans_debug_com, "Exit \r\n");
    #else

    (void)uw_ip_addr;
    (void)us_port;
    
    #endif
    
    return TRANS_SUCCESS;
}


/*****************************************************************************+
* Function: trans_monitor_thread()
* Description: 
* Parameters:
*           NONE
* Return Values:
*           TRANS_FAILD : 1
*           TRANS_SUCCESS  : 0
*
*  
*  Data:    2011-09-19
* 
+*****************************************************************************/
u_int32_t trans_monitor_thread()
{
    #ifdef TRANS_MONITOR_COMPILE
    pthread_t st_thread;
    int32_t w_ret = 0;
    
    pthread_attr_t tattr;
    pthread_attr_init (&tattr);

    #ifndef TRANS_MONITOR_TEST_COMPILE
    w_ret = set_thread_pri(&tattr, SCHED_OTHER, MIN_PRI, 0);

    if (w_ret != 0)
    {
        FLOG_WARNING("set thread scheduling error. w_ret = %d.\r\n", w_ret);
    }
    #endif

    /*Create   thread*/
    w_ret = pthread_create (&st_thread, &tattr, (void *)&trans_monitor_tcp_connect, NULL);
    /*Error*/
    if (0 != w_ret)
    {
        FLOG_ERROR("Call pthread_create error! w_ret = %d\r\n", w_ret);        
        return TRANS_FAILD;
    
    }
 
    memcpy(&(g_trans_thread_id.monitor_thread), &st_thread, sizeof(pthread_t)); 
    
    pthread_attr_destroy (&tattr);
    #endif
    
    return TRANS_SUCCESS;

}

/*****************************************************************************+
* Function: trans_msg_rev_thread()
* Description: 
* Parameters:
*           NONE
* Return Values:
*           TRANS_FAILD : 1
*           TRANS_SUCCESS  : 0
*
*  
*  Data:    2011-03-23
* 
+*****************************************************************************/
u_int32_t trans_msg_rev_thread()
{
    pthread_t st_thread;
    int32_t w_ret = 0;
    
    pthread_attr_t tattr;
    pthread_attr_init (&tattr);
    
    #ifndef TRANS_MONITOR_TEST_COMPILE
    w_ret = set_thread_pri(&tattr, SCHED_OTHER, MIN_PRI, 0);
    
    if (w_ret != 0)
    {
        FLOG_WARNING("set thread scheduling error. w_ret = %d.\r\n", w_ret);
    }
    #endif
    
    /*Create   thread*/
    w_ret = pthread_create (&st_thread, &tattr, (void *)&trans_common_msg_receive, NULL);
    /*Error*/
    if (0 != w_ret)
    {
        FLOG_ERROR("Call pthread_create error! w_ret = %d\r\n", w_ret);        
        return TRANS_FAILD;
    
    }
    
    memcpy(&(g_trans_thread_id.msg_rev_thread), &st_thread, sizeof(pthread_t)); 

    pthread_attr_destroy (&tattr);

    return TRANS_SUCCESS;

}

/*****************************************************************************+
* Function: trans_msg_pro_thread()
* Description: 
* Parameters:
*           NONE
* Return Values:
*           TRANS_FAILD : 1
*           TRANS_SUCCESS  : 0
*
*  
*  Data:    2011-03-23
* 
+*****************************************************************************/
u_int32_t trans_msg_pro_thread()
{
    pthread_t st_thread;
    int32_t w_ret = 0;
    
    pthread_attr_t tattr;
    pthread_attr_init (&tattr);
    
    #ifndef TRANS_MONITOR_TEST_COMPILE
    w_ret = set_thread_pri(&tattr, SCHED_OTHER, MIN_PRI, 0);
    
    if (w_ret != 0)
    {
        FLOG_WARNING("set thread scheduling error. w_ret = %d.\r\n", w_ret);
    }
    #endif
    
    /*Create   thread*/
    w_ret = pthread_create (&st_thread, &tattr, (void *)&trans_common_msg_process, NULL);
    /*Error*/
    if (0 != w_ret)
    {
        FLOG_ERROR("Call pthread_create error! w_ret = %d\r\n", w_ret);        
        return TRANS_FAILD;
    
    }
    
    memcpy(&(g_trans_thread_id.msg_pro_thread), &st_thread, sizeof(pthread_t)); 

    pthread_attr_destroy (&tattr);

    return TRANS_SUCCESS;

}

/*****************************************************************************+
* Function: trans_action_thread()
* Description: 
* Parameters:
*            NONE
* Return Values:
*           TRANS_FAILD : 1
*           TRANS_SUCCESS  : 0
*
*  
*  Data:    2011-03-23
* 
+*****************************************************************************/
u_int32_t trans_action_thread()
{
    pthread_t st_thread;    
    int32_t w_ret = 0;
    
    pthread_attr_t tattr;
    pthread_attr_init (&tattr);
    
    #ifndef TRANS_MONITOR_TEST_COMPILE
    w_ret = set_thread_pri(&tattr, SCHED_OTHER, MIN_PRI, 0);
    
    if (w_ret != 0)
    {
        FLOG_WARNING("set thread scheduling error. w_ret = %d.\r\n", w_ret);
    }
    #endif

    /*Create   thread*/
    w_ret = pthread_create(&st_thread, &tattr, (void *)&trans_action_handler, &g_trans_action_list);
    /*Error*/
    if (0 != w_ret)
    {
        FLOG_ERROR("Call pthread_create timer_thread error! w_ret = %d\r\n", w_ret);        
        return TRANS_FAILD;
    
    }

    memcpy(&(g_trans_thread_id.action_thread), &st_thread, sizeof(pthread_t));

    pthread_attr_destroy (&tattr);

    return TRANS_SUCCESS;

}

/*****************************************************************************+
* Function: trans_timer_thread()
* Description: 
* Parameters:
*            NONE
* Return Values:
*           TRANS_FAILD : 1
*           TRANS_SUCCESS  : 0
*
*  
*  Data:    2011-03-23
* 
+*****************************************************************************/
u_int32_t trans_timer_thread()
{
    pthread_t st_thread;    
    int32_t w_ret = 0;
   
    pthread_attr_t tattr;
    pthread_attr_init (&tattr);

    #ifndef TRANS_MONITOR_TEST_COMPILE
    w_ret = set_thread_pri(&tattr, SCHED_OTHER, MIN_PRI, 0);

    if (w_ret != 0)
    {
        FLOG_WARNING("set thread scheduling error. w_ret = %d.\r\n", w_ret);
    }
    #endif

    /*Create   thread*/
    w_ret = pthread_create(&st_thread, &tattr, (void *)&trans_timer_handler, &g_trans_timer_list);
    /*Error*/
    if (0 != w_ret)
    {
        FLOG_ERROR("Call pthread_create timer_thread error! w_ret = %d\r\n", w_ret);        
        return TRANS_FAILD;
    
    }

    memcpy(&(g_trans_thread_id.timer_thread), &st_thread, sizeof(pthread_t));
    
    pthread_attr_destroy (&tattr);

    return TRANS_SUCCESS;

}


/*****************************************************************************+
* Function: trans_init_finish_inform_agent()
* Description: Adapt init finished, send the message to inform the agent
* Parameters:
*            NONE
* Return Values:
*           TRANS_FAILD : 1
*           TRANS_SUCCESS  : 0
*
*  
*  Data:    2011-05-16
* 
+*****************************************************************************/
u_int32_t trans_init_finish_inform_agent()
{
    #ifdef TRANS_AGENT_COMPILE      
    u_int32_t uw_ret = 0;
    
    /*Send Connection message to Agent*/
    uw_ret = trans_agent_send_conn_msg();
    /*Error*/
    if (TRANS_SUCCESS != uw_ret)
    {
        FLOG_ERROR("Call trans_agent_send_conn_msg error! uw_ret = %d\r\n", uw_ret);   
        return uw_ret;
    }     
    
    /*Send State Change message to Agent*/
    uw_ret = trans_agent_send_state_msg();   
    /*Error*/
    if (TRANS_SUCCESS != uw_ret)
    {
      
        FLOG_ERROR("Call trans_agent_send_state_msg error! uw_ret = %d\r\n", uw_ret);   
        return uw_ret;
    } 

    #ifdef TRANS_BS_COMPILE
    uw_ret = trans_agent_hb_timer();
    /*Error*/
    if (TRANS_SUCCESS != uw_ret)
    {
        FLOG_ERROR("Call trans_agent_hb_timer error! uw_ret = %d\r\n", uw_ret); 
        return uw_ret;
    } 
    #endif

    #if 0
    /****************Agent Test************/
    struct trans_agent_alert_info        st_alert_info;
    struct trans_send_msg_to_agent st_msg_info;

    st_msg_info.f_callback = NULL;
    st_msg_info.p_reqs_msg = "alert";
    
    st_alert_info.us_alarm_id = 0x0205;
    st_alert_info.w_alarm_value = 345;
    
    st_msg_info.p_resp_msg = &st_alert_info;
    st_msg_info.p_info = NULL;
        
    uw_ret = trans_send_rrh_msg(TRANS_SEND_TO_AGENT, &st_msg_info);
    /*Error*/
    if (TRANS_SUCCESS != uw_ret)
    {
        FLOG_ERROR("Call trans_send_rrh_msg error! uw_ret = %d\r\n", uw_ret);
        return TRANS_FAILD;
    }
    
    FLOG_INFO("***********Send alarm OK **************** \r\n");
    //#endif 



    /****************Agent Test For periodic************/
    struct trans_periodic_sensing_info   st_periodic_info;
    int        a_per_interref[21] = {0};
    float     a_per_sensing[1024] = {0};
    
    st_periodic_info.p_per_interref = a_per_interref;
    st_periodic_info.p_per_sensing = a_per_sensing;
    
    uw_ret = trans_send_bs_msg_to_agent(TRANS_SEND_AGENT_PERIODIC, 0, &st_periodic_info);
    /*Error*/
    if (TRANS_SUCCESS != uw_ret)
    {
        FLOG_ERROR("Call trans_send_bs_msg_to_agent for AGENT_PERIODIC error! uw_ret = %d\r\n", uw_ret);
    }

    sleep (10);

    /****************Agent Test For topologyUpdate************/
    uw_ret = trans_send_bs_msg_to_agent(TRANS_SEND_AGENT_TOPOLGY, 0, "1234567");
    /*Error*/
    if (TRANS_SUCCESS != uw_ret)
    {
        FLOG_ERROR("Call trans_send_bs_msg_to_agent for AGENT_TOPOLGY error! uw_ret = %d\r\n", uw_ret);
    }


    /****************Agent Test************/
    #endif
    
    #endif
    return TRANS_SUCCESS;
  
}

//#ifdef TRANS_BS_COMPILE 

/*****************************************************************************+
* Function: trans_send_bs_msg_to_rrh()
* Description: 
* Parameters:
*           NONE
* Return Values:
*           TRANS_FAILD : 1
*           TRANS_SUCCESS  : 0
*
*  
*  Data:    2011-06-22
* 
+*****************************************************************************/
u_int32_t trans_send_bs_msg_to_rrh(u_int16_t us_type, 
                           size_t uc_num,
                           void * p_value)

{
    u_int32_t uw_ret = TRANS_SUCCESS;

    #if 0
    if (NULL == p_value)
    {
        FLOG_ERROR("NULL PTR! \r\n");
        return TRANS_FAILD;      
    } 
    #endif

    if (0 >= g_trans_rrh_socket)
    {
        FLOG_ERROR("RRH socket error,  can not send the message! \r\n");
        return TRANS_FAILD;      
    } 

    switch (us_type)
    {
        //#ifdef TRANS_RRH_COMPILE 
        #if (defined TRANS_RRH_COMPILE) && (defined TRANS_BS_COMPILE)
        /*CHANNEL_MODE : 1, 2, 3*/
        case TRANS_SEND_RRH_CFG_CHANNEL_MODE:
    
            uw_ret = trans_wireless_send2_rrh_ch_mode(NULL, uc_num, p_value);
            /*Error*/
            if (TRANS_SUCCESS != uw_ret)
            {
                FLOG_ERROR("Call trans_wireless_send2_rrh_ch_mode error! uw_ret = %d\r\n", uw_ret);        
    
            }
    
            break;   
    
        /*CHANNEL_FREQ */
        case TRANS_SEND_RRH_CFG_CHANNEL_FREQ:
           
            uw_ret = trans_wireless_send2_rrh_ch_freq(NULL, uc_num, p_value);
            /*Error*/
            if (TRANS_SUCCESS != uw_ret)
            {
                FLOG_ERROR("Call trans_wireless_send2_rrh_ch_freq error! uw_ret = %d\r\n", uw_ret);        
    
            }
    
            break;
    
        /*CHANNEL_FLAG : 0; 1*/
        case TRANS_SEND_RRH_CFG_CHANNEL_FLAG:
    
            uw_ret = trans_wireless_send2_rrh_ch_flag(NULL, uc_num, p_value);
            /*Error*/
            if (TRANS_SUCCESS != uw_ret)
            {
                FLOG_ERROR("Call trans_wireless_send2_rrh_ch_flag error! uw_ret = %d\r\n", uw_ret);        
             
            }
    
            break;
    
        /*DL_PRESEND_TIME*/
        case TRANS_SEND_RRH_CFG_DL_PRESEND_TIME:
        
            uw_ret = trans_wireless_send2_rrh_dl_pre_time(NULL, uc_num, p_value);
            /*Error*/
            if (TRANS_SUCCESS != uw_ret)
            {
                FLOG_ERROR("Call trans_wireless_send2_rrh_dl_pre_time error! uw_ret = %d\r\n", uw_ret);        
             
            }
    
            break;
    
        /*TX_LEN*/
        case TRANS_SEND_RRH_CFG_TX_LEN:
        
            uw_ret = trans_wireless_send2_rrh_tx_len(NULL, uc_num, p_value);
            /*Error*/
            if (TRANS_SUCCESS != uw_ret)
            {
                FLOG_ERROR("Call trans_wireless_send2_rrh_tx_len error! uw_ret = %d\r\n", uw_ret);        
             
            }
    
            break;
    
        /*RX_LEN*/
        case TRANS_SEND_RRH_CFG_RX_LEN:
        
            uw_ret = trans_wireless_send2_rrh_rx_len(NULL, uc_num, p_value);
            /*Error*/
            if (TRANS_SUCCESS != uw_ret)
            {
                FLOG_ERROR("Call trans_wireless_send2_rrh_len error! uw_ret = %d\r\n", uw_ret);        
             
            }
            else
    
            break;
    
        /*GPS_ENABLE*/
        case TRANS_SEND_RRH_CFG_GPS_ENABLE:
            
            uw_ret = trans_wireless_send2_rrh_gps_enable(NULL, uc_num, p_value);
            /*Error*/
            if (TRANS_SUCCESS != uw_ret)
            {
                FLOG_ERROR("Call trans_wireless_send2_rrh_gps_enable error! uw_ret = %d\r\n", uw_ret);        
          
            }
    
            break;  
    
         /*OUTPUT_POWER*/
        case TRANS_SEND_RRH_CFG_OUTPUT_POWER:
            
            uw_ret = trans_wireless_send2_rrh_output_pow(NULL, uc_num, p_value);
            /*Error*/
            if (TRANS_SUCCESS != uw_ret)
            {
                FLOG_ERROR("Call trans_wireless_send2_rrh_output_pow error! uw_ret = %d\r\n", uw_ret);        
           
            }
            break;        
    
         /*PA_SWITCH*/
        case TRANS_SEND_RRH_CFG_PA_SWITCH:
            
            uw_ret = trans_wireless_send2_rrh_pa_switch(NULL, uc_num, p_value);
            /*Error*/
            if (TRANS_SUCCESS != uw_ret)
            {
                FLOG_ERROR("Call trans_wireless_send2_rrh_pa_switch error! uw_ret = %d\r\n", uw_ret);        
         
            }
      
            break;   
    
         /*AGC*/
        case TRANS_SEND_RRH_CFG_AGC_ENABLE:
        
            uw_ret = trans_wireless_send2_rrh_agc_enable(NULL, uc_num, p_value);
            /*Error*/
            if (TRANS_SUCCESS != uw_ret)
            {
                FLOG_ERROR("Call trans_wireless_send2_rrh_agc_enable error! uw_ret = %d\r\n", uw_ret);        
            
            }
        
            break;   
            
         /*CHAN1_RX_PGC*/
        case TRANS_SEND_RRH_CFG_CH_RX_PGC:
        
            uw_ret = trans_wireless_send2_rrh_ch_rx_pgc(NULL, uc_num, p_value);
            /*Error*/
            if (TRANS_SUCCESS != uw_ret)
            {
                FLOG_ERROR("Call trans_wireless_send2_rrh_ch_rx_pgc error! uw_ret = %d\r\n", uw_ret);        
            
            }
    
            break; 

         /*Carrier info*/
        case TRANS_SEND_RRH_CFG_CARR_INFO:
        
            uw_ret = trans_wireless_send2_rrh_carrier_info(NULL, uc_num, p_value);
            /*Error*/
            if (TRANS_SUCCESS != uw_ret)
            {
                FLOG_ERROR("Call trans_wireless_send2_rrh_carrier_info error! uw_ret = %d\r\n", uw_ret);        
            
            }
        
            break; 
            
         /*Byte order*/
        case TRANS_SEND_RRH_CFG_BYTE_ORDER:
        
            uw_ret = trans_wireless_send2_rrh_byte_order(NULL, uc_num, p_value);
            /*Error*/
            if (TRANS_SUCCESS != uw_ret)
            {
                FLOG_ERROR("Call trans_wireless_send2_rrh_byte_order error! uw_ret = %d\r\n", uw_ret);        
            
            }
        
            break; 

         /*TTG  &  RTG*/
        case TRANS_SEND_RRH_CFG_TTG_RTG:
        
            uw_ret = trans_wireless_send2_rrh_ttg_rtg(NULL, uc_num, p_value);
            /*Error*/
            if (TRANS_SUCCESS != uw_ret)
            {
                FLOG_ERROR("Call trans_wireless_send2_rrh_ttg_rtg error! uw_ret = %d\r\n", uw_ret);        
            
            }
        
            break; 

    
        /*Initial Step Query----basic information */
        case TRANS_SEND_RRH_QUERY_GPS:
        
            uw_ret = trans_wireless_send2_rrh_q_gps(NULL, uc_num, p_value);
            if (TRANS_SUCCESS != uw_ret) 
            {   
                FLOG_ERROR("Call trans_wireless_send2_rrh_q_gps error! uw_ret = %d\r\n", uw_ret);
            }   
    
           break;
            
        case TRANS_SEND_RRH_QUERY_POWER:
            
            uw_ret = trans_wireless_send2_rrh_q_power(NULL, uc_num, p_value);
            if (TRANS_SUCCESS != uw_ret) 
            {   
                FLOG_ERROR("Call trans_wireless_send2_rrh_q_power error! uw_ret = %d\r\n", uw_ret);
            }  
    
            break;
    
    
        case TRANS_SEND_RRH_QUERY_GPS_LOCK:
        
            uw_ret = trans_wireless_send2_rrh_q_gps_lock(NULL, uc_num, p_value);
            if (TRANS_SUCCESS != uw_ret) 
            {   
                FLOG_ERROR("Call trans_wireless_send2_rrh_q_gps_lock error! uw_ret = %d\r\n", uw_ret);
            }  
        
            break;

        #endif
                        
        default:
            
            FLOG_ERROR("Unknow type error! uc_type = %d\r\n", us_type);    
            uw_ret = TRANS_FAILD;

            (void)uc_num;
            (void)p_value;
    
            break;
    
    }  
    
    return uw_ret;

}

/*****************************************************************************+
* Function: trans_send_msg_to_agent()
* Description: 
* Parameters:
*           NONE
* Return Values:
*           TRANS_FAILD : 1
*           TRANS_SUCCESS  : 0
*
*  
*  Data:    2011-06-22
* 
+*****************************************************************************/
u_int32_t trans_send_bs_msg_to_agent(u_int16_t us_type, 
                           size_t len,
                           void * p_value)
{
    u_int32_t uw_ret = TRANS_SUCCESS;
    (void) len;
    
    #if (defined TRANS_AGENT_COMPILE) && (defined TRANS_BS_COMPILE)
    
    if (NULL == p_value)
    {
        FLOG_ERROR("NULL PTR! \r\n");
        return TRANS_FAILD;      
    }  
    
    if (0 >= g_trans_agent_socket.w_agent_socket)
    {
        FLOG_ERROR("Agent socket error,  can not send the message! \r\n");
        return TRANS_FAILD;      
    } 

    switch (us_type)
    {
        //#ifdef TRANS_AGENT_COMPILE
        
        case TRANS_SEND_AGENT_SPECTRUM:
            
            uw_ret = trans_wireless_send2_agent_spectrum(NULL, 0, p_value);
            if (TRANS_SUCCESS != uw_ret) 
            {   
                FLOG_ERROR("Call trans_wireless_send2_agent_spectrum error! uw_ret = %d\r\n", uw_ret);
            }  
        
            break;
            
        case TRANS_SEND_AGENT_PERIODIC:
            
            uw_ret = trans_wireless_send2_agent_periodic(NULL, 0, p_value);
            if (TRANS_SUCCESS != uw_ret) 
            {   
                FLOG_ERROR("Call trans_wireless_send2_agent_periodic error! uw_ret = %d\r\n", uw_ret);
            }  
        
            break;
            
        case TRANS_SEND_AGENT_TOPOLGY:
            
            uw_ret = trans_wireless_send2_agent_topology(NULL, 0, p_value);
            if (TRANS_SUCCESS != uw_ret) 
            {   
                FLOG_ERROR("Call trans_wireless_send2_agent_topology error! uw_ret = %d\r\n", uw_ret);
            }  
        
            break;

        default:
            
            FLOG_ERROR("Unknow type error! uc_type = %d\r\n", us_type);    
            uw_ret = TRANS_FAILD;
    
            break;
    
    }  

    #else

    (void)p_value;
    (void)us_type;

    
    #endif
    
    return uw_ret;
    
}

/*****************************************************************************+
* Function: trans_end_bs_msg_to_monitor()
* Description: 
* Parameters:
*           NONE
* Return Values:
*           TRANS_FAILD : 1
*           TRANS_SUCCESS  : 0
*
*  
*  Data:    2011-06-22
* 
+*****************************************************************************/
u_int32_t trans_send_bs_msg_to_monitor(u_int16_t us_type, 
                            u_int8_t uc_ack_flag,
                            u_int32_t uw_transaction, 
                            size_t len,
                            void * p_value)
{
    u_int32_t uw_ret = TRANS_SUCCESS;
    
    if (NULL == p_value)
    {
        FLOG_ERROR("NULL PTR! \r\n");
        return TRANS_FAILD;      
    }  
    
    switch (us_type)
    {
        //#ifdef TRANS_MONITOR_COMPILE
        #if (defined TRANS_MONITOR_COMPILE) && (defined TRANS_BS_COMPILE)
        case TRANS_SEND_MONITOR_HOOK_RESP:
            
            uw_ret = trans_wireless_send2_monitor_hook_resp(
                                                uc_ack_flag, 
                                                uw_transaction, 
                                                len, 
                                                p_value);
            if (TRANS_SUCCESS != uw_ret) 
            {   
                FLOG_ERROR("Call trans_wireless_send2_monitor_hook_resp error! uw_ret = %d\r\n", uw_ret);
            }  
        
            break;

        #endif

        default:
            
            (void)uc_ack_flag;
            (void)uw_transaction;
            (void)len;
            FLOG_ERROR("Unknow type error! uc_type = %d\r\n", us_type);    
            uw_ret = TRANS_FAILD;
    
            break;
    
    }  
    
    return uw_ret;
    
}

/*****************************************************************************+
* Function: trans_send_ms_msg_to_monitor()
* Description: 
* Parameters:
*           NONE
* Return Values:
*           TRANS_FAILD : 1
*           TRANS_SUCCESS  : 0
*
*  
*  Data:    2012-03-30
* 
+*****************************************************************************/
u_int32_t trans_send_ms_msg_to_monitor(u_int16_t us_type, 
                            u_int8_t uc_ack_flag,
                            u_int32_t uw_transaction, 
                            size_t len,
                            void * p_value)
{
    u_int32_t uw_ret = TRANS_SUCCESS;
    
    if (NULL == p_value)
    {
        FLOG_ERROR("NULL PTR! \r\n");
        return TRANS_FAILD;      
    }  
    
    switch (us_type)
    {
        //#ifdef TRANS_MONITOR_COMPILE
        #if (defined TRANS_MONITOR_COMPILE) && (defined TRANS_MS_COMPILE)
        case TRANS_SEND_MONITOR_HOOK_RESP:
            
            uw_ret = trans_wireless_send2_monitor_hook_resp(
                                                uc_ack_flag, 
                                                uw_transaction, 
                                                len, 
                                                p_value);
            if (TRANS_SUCCESS != uw_ret) 
            {   
                FLOG_ERROR("Call trans_wireless_send2_monitor_hook_resp error! uw_ret = %d\r\n", uw_ret);
            }  
        
            break;

        #endif

        default:
            
            (void)uc_ack_flag;
            (void)uw_transaction;
            (void)len;
            FLOG_ERROR("Unknow type error! uc_type = %d\r\n", us_type);    
            uw_ret = TRANS_FAILD;
    
            break;
    
    }  
    
    return uw_ret;
    
}


//#endif


/*****************************************************************************+
* Function: trans_thread_create()
* Description: Adapt moudle create thread
* Parameters:
*       
* Return Values:
*           TRANS_FAILD : 1
*           TRANS_SUCCESS  : 0
*
*  
*  Data:    2011-05-16
* 
+*****************************************************************************/
u_int32_t trans_thread_create()
{
    u_int32_t uw_ret = 0;

    uw_ret = trans_msg_rev_thread();  
    if (TRANS_SUCCESS != uw_ret)
    {
        FLOG_ERROR("Call trans_msg_rev_thread error! uw_ret = %d\r\n", uw_ret);     
        return uw_ret;
    }

    uw_ret = trans_msg_pro_thread();  
    if (TRANS_SUCCESS != uw_ret)
    {
        FLOG_ERROR("Call trans_msg_pro_thread error! uw_ret = %d\r\n", uw_ret);     
        return uw_ret;
    }    
    
    uw_ret = trans_action_thread();  
    if (TRANS_SUCCESS != uw_ret)
    {
        FLOG_ERROR("Call trans_action_thread error! uw_ret = %d\r\n", uw_ret);     
        return uw_ret;
    }

    uw_ret = trans_monitor_thread();
    if (TRANS_SUCCESS != uw_ret)
    {
        FLOG_ERROR("Call trans_monitor_thread error! uw_ret = %d\r\n", uw_ret);     
        return uw_ret;
    }
    
    g_trans_thread_flag = 1;

    return TRANS_SUCCESS;
}

/*****************************************************************************+
* Function: trans_register_func_callback()
* Description: Register Callback Function
* Parameters:
*           us_register_type : enum trans_register_fun_enum 
*           p_register_name : 
*                   (1) if uc_register_type == TRANS_REGISTER_FUN_MONITOR_OP
*                       p_register_name must be a number
*                   (2) 
*
*           f_exe_func : callback function is called when execute operation "p_register_name"
*           f_delete_func : callback function is called when delete operation "p_register_name"
* Return Values:
*           NONE
*
*  
*  Data:    2011-09-05
+*****************************************************************************/
u_int32_t trans_register_func_callback(u_int16_t us_register_type,
                                    void * p_register_name,  
                                    fun_callback f_exe_func,
                                    fun_callback f_delete_func)
{
    u_int16_t  us_register_name = TRANS_REGISTER_FUN_BUF;
    u_int16_t  us_operation = 0;
    
    switch (us_register_type)
    {
        case TRANS_REGISTER_FUN_MONITOR_OP:
    
            us_register_name = *((u_int16_t * )p_register_name);

            us_operation = TRANS_REGISTER_FUN_MONITOR_OP + us_register_name;
                            
            break;
            
        case TRANS_REGISTER_FUN_RRH_MSG_PRO:
        
            us_register_name = *((u_int16_t * )p_register_name);
        
            us_operation = TRANS_REGISTER_FUN_RRH_MSG_PRO + us_register_name;
                            
            break;
            
        case TRANS_REGISTER_FUN_AGENT_MSG_PRO:
        
            us_register_name = *((u_int16_t * )p_register_name);
        
            us_operation = TRANS_REGISTER_FUN_AGENT_MSG_PRO + us_register_name;
                            
            break;

        case TRANS_REGISTER_FUN_MONITOR_MSG_PRO:
        
            us_register_name = *((u_int16_t * )p_register_name);
        
            us_operation = TRANS_REGISTER_FUN_MONITOR_MSG_PRO + us_register_name;
                            
            break;
            
        case TRANS_REGISTER_FUN_WIRELESS_MSG_PRO:
        
            us_register_name = *((u_int16_t * )p_register_name);
        
            us_operation = TRANS_REGISTER_FUN_WIRELESS_MSG_PRO + us_register_name;
                            
            break;

        default:
    
            FLOG_ERROR("Type error! us_register_type = %d\r\n", us_register_type);
            
            return TRANS_FAILD;

    }

    if (TRANS_REGISTER_FUN_BUF < us_operation)
    {
        FLOG_ERROR("Out of rang : register_name = %d!\r\n", us_register_name);  
        return TRANS_FAILD;
    }

    if (NULL != f_exe_func)
    {
        if (1 == g_trans_register_exe_func[us_operation].uc_use_flag)
        {
            FLOG_ERROR("%d has been register in exe_func!\r\n", us_register_name);  
            return TRANS_FAILD;
        }
        else
        {
            g_trans_register_exe_func[us_operation].uc_use_flag = 1;
            g_trans_register_exe_func[us_operation].f_callback = f_exe_func;
        }
    }
    
    if (NULL != f_delete_func)
    {
        if (1 == g_trans_register_delete_func[us_operation].uc_use_flag)
        {
            FLOG_ERROR("%d has been register in delete_func!\r\n", us_register_name);  
            return TRANS_FAILD;
        }
        else
        {
            g_trans_register_delete_func[us_operation].uc_use_flag = 1;
            g_trans_register_delete_func[us_operation].f_callback = f_delete_func;
        
        }
    }
    
    return TRANS_SUCCESS;

}


/*****************************************************************************+
* Function: trans_thread_release()
* Description: Adapt moudle release
* Parameters:
*           NONE
* Return Values:
*           TRANS_FAILD : 1
*           TRANS_SUCCESS  : 0
*
*  
*  Data:    2011-05-16
* 
+*****************************************************************************/
u_int32_t trans_release()
{
    u_int32_t uw_ret = 0;
    
    FLOG_DEBUG_TRANS(g_trans_debug_com, "Enter \r\n");
    
    if (0 == g_trans_thread_flag)
    {
        FLOG_ERROR("Adapt module is not create!\r\n ");  
        
        return TRANS_FAILD;
    }
  
    /*Check thread is ont 0*/
    /*Cancel thread*/
    pthread_cancel (g_trans_thread_id.timer_thread);
    pthread_join (g_trans_thread_id.timer_thread, NULL);
    
    pthread_cancel (g_trans_thread_id.msg_rev_thread);
    pthread_join (g_trans_thread_id.msg_rev_thread, NULL);
    
    pthread_cancel (g_trans_thread_id.msg_pro_thread);
    pthread_join (g_trans_thread_id.msg_pro_thread, NULL);

    pthread_cancel (g_trans_thread_id.action_thread);
    pthread_join (g_trans_thread_id.action_thread, NULL);

    #ifdef TRANS_MONITOR_COMPILE
    /*Cancel thread*/
    pthread_cancel (g_trans_thread_id.monitor_thread);
    pthread_join (g_trans_thread_id.monitor_thread, NULL);
    #endif 
    
    #if 0
    /*Free memory*/
    free(g_trans_rev_buf);
    free(g_trans_send_buf);
    
    g_trans_rev_buf = NULL;
    g_trans_send_buf = NULL;
    #endif
    
    g_trans_thread_flag = 0;

    uw_ret = trans_device_release();
    /*Error*/
    if (TRANS_SUCCESS != uw_ret)
    {
        FLOG_ERROR("Call trans_device_release error! \r\n");
    
        return uw_ret;        
    }  
    
    uw_ret = trans_common_release();
    /*Error*/
    if (TRANS_SUCCESS != uw_ret)
    {
        FLOG_ERROR("Call trans_common_release error! \r\n");
    
        return uw_ret;        
    }  

    uw_ret = trans_transaction_release();
    /*Error*/
    if (TRANS_SUCCESS != uw_ret)
    {
        FLOG_ERROR("Call trans_transaction_release error! \r\n");
    
        return uw_ret;        
    }  

    uw_ret = trans_timer_release();
    if (TRANS_SUCCESS != uw_ret)
    {
        FLOG_ERROR("Call trans_timer_release error! uw_ret = %d\r\n", uw_ret);     
        return uw_ret;
    }
    
    uw_ret = trans_action_release();
    if (TRANS_SUCCESS != uw_ret)
    {
        FLOG_ERROR("Call trans_action_release error! uw_ret = %d\r\n", uw_ret);     
        return uw_ret;
    }
    
    #ifdef TRANS_RRH_COMPILE
    uw_ret = trans_rrh_release();
    if (TRANS_SUCCESS != uw_ret)
    {
        FLOG_ERROR("Call trans_rrh_release error! uw_ret = %d\r\n", uw_ret);     
        return uw_ret;
    }
    #endif

    #ifdef TRANS_AGENT_COMPILE
    uw_ret = trans_agent_release();
    if (TRANS_SUCCESS != uw_ret)
    {
        FLOG_ERROR("Call trans_agent_release error! uw_ret = %d\r\n", uw_ret);     
        return uw_ret;
    }
    #endif

    #ifdef TRANS_MONITOR_COMPILE
    uw_ret = trans_monitor_release();
    /*Error*/
    if (TRANS_SUCCESS != uw_ret)
    {
        FLOG_ERROR("Call trans_monitor_init error! \r\n");
    
        return uw_ret;        
    }  
    #endif
    
    #if (defined TRANS_BS_COMPILE) || (defined TRANS_MS_COMPILE)
    uw_ret = trans_wireless_release();
    if (TRANS_SUCCESS != uw_ret)
    {
        FLOG_ERROR("Call trans_bs_release error! uw_ret = %d\r\n", uw_ret);     
        return uw_ret;
    }
    #endif    
    
    FLOG_DEBUG_TRANS(g_trans_debug_com, "Exit \r\n");

    return TRANS_SUCCESS;
}


