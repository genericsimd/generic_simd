/*****************************************************************************+
*
*  File Name: trans.c
*
*  Function: TRANS MODULE
*
*  
*  Data:    2011-03-23
*  Modify:
*
+*****************************************************************************/


#include <sys/types.h>
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <syslog.h>
#include <signal.h>

#include <flog.h>
#include "queue_util.h"
#include "thd_util.h"

#include <trans.h>
#include <trans_timer.h>
#include <trans_list.h>
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

int32_t g_trans_moudle_socket_fd[TRANS_MOUDLE_BUF] = {0};

/*If the thread has been creat*/
static u_int8_t    g_trans_thread_flag = 0;

u_int8_t  *g_trans_rev_buf = NULL;
u_int8_t  *g_trans_send_buf = NULL;

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

    g_trans_rrh_socket = -1;
    g_trans_agent_socket.w_agent_socket = -1;
    g_trans_monitor_socket.w_monitor_socket = -1;

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

    /*Adapt Timer Thread*/
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
* Function: trans_msg_en_quene()
* Description: 
* Parameters:
*           NONE
* Return Values:
*           TRANS_FAILD : 1
*           TRANS_SUCCESS  : 0
*
*  
*  Data:    2011-07-12
* 
+*****************************************************************************/
u_int32_t trans_msg_en_quene(void *p_msg, struct trans_en_queue_msg *p_en_quene)
{
    struct queue_msg st_quene_msg;
    struct trans_queue_msg *p_quene_msg =NULL;
    u_int32_t   uw_len = 0;
    
    FLOG_DEBUG("Enter \r\n");  
      
    if ((NULL == p_msg) || (NULL == p_en_quene))
    {
        FLOG_ERROR("NULL PTR! \r\n");
        return TRANS_FAILD;  
    }
    
    /* Allocate a memory.  */
    p_quene_msg = (struct trans_queue_msg *)malloc(SIZEOF_TRANS_QUENE_MSG);
    
    if (NULL == p_quene_msg)
    {
        FLOG_ERROR("1 malloc p_quene_msg error! \r\n");
        return TRANS_FAILD;   
    }

    memset((u_int8_t*)p_quene_msg, 0, SIZEOF_TRANS_QUENE_MSG); 

    uw_len = p_en_quene->uw_len;

    if (0 != uw_len)
    {
        /* Allocate a memory.  */
        p_quene_msg->p_msg = (u_int8_t *)malloc(uw_len);
        if (NULL == p_quene_msg->p_msg)
        {
            FLOG_ERROR("2 malloc p_quene_msg->p_msg error! \r\n");
            return TRANS_FAILD;   
        }
    }
    else
    {
        p_quene_msg->p_msg = NULL;
    }   
   
    p_quene_msg->uc_result = p_en_quene->uc_result;
    p_quene_msg->uw_src_moudle = p_en_quene->uw_src_moudle;
    p_quene_msg->uw_len= uw_len;

    FLOG_DEBUG("%d, %d, %d\r\n", p_quene_msg->uc_result, p_quene_msg->uw_src_moudle, p_quene_msg->uw_len); 
    
    memcpy(p_quene_msg->p_msg, (u_int8_t *)p_msg, uw_len); 
    
    st_quene_msg.my_type = bsagent_en_id;
    st_quene_msg.p_buf = p_quene_msg;
    
    //trans_debug_msg_print(p_quene_msg->p_msg, 10, g_trans_debug_com);

    if (wmrt_enqueue (bsagent_en_id, &st_quene_msg, sizeof(struct queue_msg))
            == -1)
    {
        FLOG_ERROR ("ENQUEUE ERROR\n");
    }
    
    FLOG_DEBUG("Exit \r\n"); 

    return TRANS_SUCCESS;
}

/*****************************************************************************+
* Function: trans_msg_de_quene()
* Description: 
* Parameters:
*           NONE
* Return Values:
*           TRANS_FAILD : 1
*           TRANS_SUCCESS  : 0
*
*  
*  Data:    2011-07-12
* 
+*****************************************************************************/
u_int32_t trans_msg_de_quene(u_int8_t *p_result)
{
    struct queue_msg st_quene_msg;
    struct trans_queue_msg *p_quene_msg =NULL;
    
    FLOG_DEBUG("Enter \r\n");   
     
    if (NULL == p_result )
    {
        FLOG_ERROR("NULL PTR! \r\n");
        return TRANS_FAILD;  
    }

    st_quene_msg.my_type = bsagent_de_id;
    
    if (wmrt_dequeue (bsagent_de_id, &st_quene_msg, sizeof(struct queue_msg))
            == -1)
    {
        FLOG_ERROR ("DEQUEUE ERROR\n");
    }
    
    if (NULL == st_quene_msg.p_buf)
    {
        FLOG_ERROR("2 NULL PTR! \r\n");
        return TRANS_FAILD;      
    } 
    
    p_quene_msg = (struct trans_queue_msg *)(st_quene_msg.p_buf);
    
    *p_result = p_quene_msg->uc_result;

    FLOG_DEBUG("%d, %d, %d\r\n", p_quene_msg->uc_result, p_quene_msg->uw_src_moudle, p_quene_msg->uw_len);

    //trans_debug_msg_print(p_quene_msg->p_msg, 10, g_trans_debug_com);
    
    if ((0 == p_quene_msg->uw_len) || (NULL != p_quene_msg->p_msg))
    {
        free(p_quene_msg->p_msg);
    }
    
    free(p_quene_msg);

    FLOG_DEBUG("Exit \r\n"); 
   
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

    FLOG_DEBUG("Enter \r\n");

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

    uw_ret = trans_rrh_tcp_socket();
    /*Error*/
    if (TRANS_SUCCESS != uw_ret)
    {
        FLOG_ERROR("Call trans_rrh_tcp_socket error! uw_ret =%d\r\n", uw_ret);
    
        return uw_ret;        
    } 

    //pthread_cond_signal(&g_trans_msg_thread_cond);

    /*Close GPS Alert*/
    #if 0
    struct trans_send_cfg_to_rrh st_cfg_rrh;
    u_int16_t  us_param_type = 0x0205;
    u_int32_t  uw_param_value = 0;
    
    st_cfg_rrh.f_callback = trans_rrh_noblock_msg_timer_func;
    st_cfg_rrh.p_param_type = &us_param_type; 
    st_cfg_rrh.p_info = NULL;
    st_cfg_rrh.p_param_value = &uw_param_value;
    st_cfg_rrh.us_param_num = 1;
    st_cfg_rrh.uw_timeout = 10;
    
    uw_ret = trans_send_wireless_msg(TRANS_SEND_TO_RRH_CFG, &st_cfg_rrh);
    /*Error*/
    if (TRANS_SUCCESS != uw_ret)
    {
        FLOG_ERROR("Call trans_send_wireless_msg error! uw_ret =%d\r\n", uw_ret);
    
        //return uw_ret;        
    } 
    #endif
    
    FLOG_DEBUG("Exit \r\n");
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
    
    FLOG_DEBUG("Enter \r\n");

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
        FLOG_ERROR("Call trans_agent_send_state_msg error! uw_ret = %d\r\n", uw_ret);        
        return TRANS_FAILD;
    }  
    
    FLOG_DEBUG("Exit \r\n");

    #else

    FLOG_WARNING("Aegnt Disabled!");

    #endif
    
    return TRANS_SUCCESS;
}


/*****************************************************************************+
* Function: trans_msg_process()
* Description: 
* Parameters:
*           NONE
* Return Values:
*           NONE
*
*  
*  Data:    2011-03-23
* 
+*****************************************************************************/
void trans_msg_process()
{
    u_int32_t uw_ret = 0;
    struct trans_thread_info st_thread_info;
    //struct trans_thread_info *p_thread_info = NULL;
    
    int32_t  w_ret = 0;
    struct timeval st_time_val;
    
    fd_set readfds;
    int32_t w_max_socket = 0;
    
    u_int8_t  *p_rev_buf = NULL;
    u_int8_t  *p_send_buf = NULL;
        
    FLOG_DEBUG("Enter \r\n");

    w_ret = pthread_setcanceltype (PTHREAD_CANCEL_ASYNCHRONOUS, NULL);
    if (0 != w_ret)
    {
        FLOG_ERROR("Call pthread_setcanceltype error! w_ret =%d\r\n", w_ret);    
        return ;
    }

    #if 0
    #ifdef TRANS_RRH_COMPILE
    if (g_trans_rrh_socket <= 0)
    {
        //close (g_rrh_server_socket.w_sockFd);
        FLOG_ERROR("Socket ID error ! w_rrh_sockfd = %d\r\n", g_trans_rrh_socket);
        
        return;
    }
    #endif
        
    #ifdef TRANS_AGENT_COMPILE
    if (g_trans_agent_socket.w_agent_socket<= 0)
    {
        //close (g_rrh_server_socket.w_sockFd);
        FLOG_INFO("Socket ID error ! w_agent_sockfd = %d\r\n", g_trans_agent_socket.w_agent_socket);
        
        //return;
    }
    #endif    

    #ifdef TRANS_MONITOR_COMPILE
    
    pthread_mutex_lock (&(g_trans_monitor_socket.m_mutex));
    
    if (g_trans_monitor_socket.w_monitor_socket <= 0)
    {
        //close (g_rrh_server_socket.w_sockFd);
        FLOG_INFO("Socket ID error ! w_monitor_socket = %d\r\n", g_trans_monitor_socket.w_monitor_socket);
        
        //return;
    }
    
    pthread_mutex_unlock(&(g_trans_monitor_socket.m_mutex));            
    
    #endif  
    #endif


    #ifdef TRANS_RRH_COMPILE
    /*Send HeartBeat Msg To RRH per 3s*/    
    w_ret = trans_rrh_heartbeat_timer_func(NULL, 0, NULL);
    /*Error*/
    if (0 != w_ret)
    {
        FLOG_ERROR("Call trans_rrh_heartbeat_timer_func error! w_ret =%d\r\n", w_ret);
        //return ;
    }
    #endif    
    
    p_rev_buf = g_trans_rev_buf;
    p_send_buf = g_trans_send_buf;

    /*Continue to receive messages until the connetction broken*/
    while (1)
    {

        #if 0
        #if (!defined TRANS_AGENT_COMPILE) && (defined TRANS_RRH_COMPILE)
        w_max_socket = g_trans_rrh_socket;      
        st_thread_info.w_rrh_sockfd = g_trans_rrh_socket;
        #endif
        
        #if (!defined TRANS_RRH_COMPILE) && (defined TRANS_AGENT_COMPILE)
        w_max_socket = g_trans_agent_socket.w_agent_socket; 
        st_thread_info.w_agent_sockfd = g_trans_agent_socket.w_agent_socket; 
        #endif

        #if (defined TRANS_RRH_COMPILE) && (defined TRANS_AGENT_COMPILE)&&(!defined TRANS_MONITOR_COMPILE)
        w_max_socket = TRANS_MAX(g_trans_rrh_socket, g_trans_agent_socket.w_agent_socket);
       
        st_thread_info.w_rrh_sockfd = g_trans_rrh_socket;
        st_thread_info.w_agent_sockfd = g_trans_agent_socket.w_agent_socket; 
        #endif
        #endif

        #if (defined TRANS_RRH_COMPILE) || (defined TRANS_AGENT_COMPILE) || (defined TRANS_MONITOR_COMPILE)

        st_thread_info.w_rrh_sockfd = g_trans_rrh_socket;
        
        pthread_mutex_lock (&(g_trans_agent_socket.m_mutex));

        w_max_socket = TRANS_MAX(g_trans_rrh_socket, g_trans_agent_socket.w_agent_socket);

        st_thread_info.w_agent_sockfd = g_trans_agent_socket.w_agent_socket; 
       
        pthread_mutex_unlock(&(g_trans_agent_socket.m_mutex));        
        
        pthread_mutex_lock (&(g_trans_monitor_socket.m_mutex));
        
        w_max_socket = TRANS_MAX(w_max_socket, g_trans_monitor_socket.w_monitor_socket);

        st_thread_info.w_monitor_sockfd = g_trans_monitor_socket.w_monitor_socket;

        pthread_mutex_unlock(&(g_trans_monitor_socket.m_mutex));            
        
        #endif

        FLOG_DEBUG("w_max_socket = %d. \r\n", w_max_socket);

        /*No connection*/
        if (0 >= w_max_socket)
        {
            sleep(1);
            
            continue;
        }

        FD_ZERO(&readfds);
        
        #ifdef TRANS_RRH_COMPILE
        if (st_thread_info.w_rrh_sockfd > 0)
        {
            FD_SET(st_thread_info.w_rrh_sockfd, &readfds);
        }         
        #endif
        
        #ifdef TRANS_AGENT_COMPILE
        if (st_thread_info.w_agent_sockfd > 0)
        {
            FD_SET(st_thread_info.w_agent_sockfd, &readfds);
        }        
        #endif

        #ifdef TRANS_MONITOR_COMPILE
        if (st_thread_info.w_monitor_sockfd > 0)
        {
            FD_SET(st_thread_info.w_monitor_sockfd, &readfds);
        }
        #endif

//        gettimeofday(&st_time_val, NULL);

        st_time_val.tv_sec = 1;
     
        w_ret = select(w_max_socket + 1, &readfds, NULL, NULL, &st_time_val);

//        w_ret = select(w_max_socket + 1, &readfds, NULL, NULL, NULL);

        if(w_ret <0)
        {
            /*???????If  error , what?????*/
            FLOG_ERROR("select error! w_ret = %d\r\n", w_ret);
            //close(w_socket);
            return ;
        }
        else if (0 == w_ret)
        {
            //FLOG_INFO("select timeout \r\n");
            continue;
        }

        memset((u_int8_t*)p_rev_buf, 0, TRANS_REV_MSG_MAX_LEN);
        memset((u_int8_t*)p_send_buf, 0, TRANS_SEND_MSG_MAX_LEN);      

        #ifdef TRANS_AGENT_COMPILE
        /*Msg rev from AGENT-> Msg Process -> Msg send */
        if (st_thread_info.w_agent_sockfd > 0)
        {
            if((FD_ISSET(st_thread_info.w_agent_sockfd, &readfds)))
            {
                uw_ret = trans_agent_rev_msg(p_rev_buf, st_thread_info.w_agent_sockfd);
                /*Error*/
                if (TRANS_SUCCESS != uw_ret)
                {
                    /*???????If  error , Do what?????*/        
                    pthread_mutex_lock (&(g_trans_agent_socket.m_mutex));                   
                    g_trans_agent_socket.w_agent_socket = -1;                     
                    pthread_mutex_unlock(&(g_trans_agent_socket.m_mutex)); 

                    FLOG_ERROR("Call trans_agent_rev_msg error! uw_ret = %d\r\n", uw_ret);

                    FLOG_WARNING("Lost the connection with Agent. \r\n");
                    //return ;
                }  
                else
                {
                    uw_ret = trans_agent_rev_msg_process(p_rev_buf, p_send_buf, &st_thread_info);
                    /*Error*/
                    if (TRANS_SUCCESS != uw_ret)
                    {
                        /*???????If  error , Do what?????*/                    
                        FLOG_ERROR("Call trans_agent_rev_msg_process error! uw_ret = %d\r\n", uw_ret);
                        //return ;                
                    }   
                }            
            } 
        }
        #endif
       
        #ifdef TRANS_RRH_COMPILE
        /*Msg rev from RRH-> Msg Process -> Msg send */
        if (st_thread_info.w_rrh_sockfd > 0)
        {
            if((FD_ISSET(st_thread_info.w_rrh_sockfd, &readfds)))
            {
                uw_ret = trans_rrh_rev_msg(p_rev_buf, st_thread_info.w_rrh_sockfd);
                /*Error*/
                if (TRANS_SUCCESS != uw_ret)
                {
                    /*???????If  error , Do what?????*/                    
                    FLOG_ERROR("Call trans_rrh_rev_msg error! uw_ret = %d\r\n", uw_ret);
                    FLOG_ERROR("Connection RRH failed\r\n");
                    return ;
                }  
                else
                {
                    uw_ret = trans_rrh_rev_msg_process(p_rev_buf, p_send_buf, &st_thread_info);
                    /*Error*/
                    if (TRANS_SUCCESS != uw_ret)
                    {
                        /*???????If  error , Do what?????*/                    
                        FLOG_ERROR("Call trans_rrh_rev_msg_process error! uw_ret = %d\r\n", uw_ret);
                        //return ;                
                    }   
                }             
            }
        }
        #endif

        #ifdef TRANS_MONITOR_COMPILE
        struct trans_msg_info st_msg_info;
        /*Msg rev from Monitor-> Msg Process -> Msg send */
        if (st_thread_info.w_monitor_sockfd > 0)
        {
            if((FD_ISSET(st_thread_info.w_monitor_sockfd, &readfds)))
            {
                uw_ret = trans_monitor_rev_msg(p_rev_buf, st_thread_info.w_monitor_sockfd);
                /*Error*/
                if (TRANS_SUCCESS != uw_ret)
                {
                    /*???????If  error , Do what?????*/                    
                    FLOG_ERROR("Call trans_monitor_rev_msg error! uw_ret = %d\r\n", uw_ret);
                    FLOG_WARNING("Lost the connection with Monitor. \r\n");

                    /*Connection failed Process*/
                    //trans_monitor_tcp_connect_failed();
                    trans_monitor_conn_failed();
                    //return ;
                }  
                else
                {
                    st_msg_info.p_rev_msg = p_rev_buf;
                    st_msg_info.p_thread_info = &st_thread_info;
                    
                    uw_ret = trans_monitor_rev_msg_process(&st_msg_info, TRANS_REV_MSG_MAX_LEN, p_send_buf);
                    /*Error*/
                    if (TRANS_SUCCESS != uw_ret)
                    {
                        /*???????If  error , Do what?????*/                    
                        FLOG_ERROR("Call trans_monitor_rev_msg_process error! uw_ret = %d\r\n", uw_ret);
                        //return ;                
                    }   
                }            
            } 
        }
        #endif
   
    }

    FLOG_DEBUG("Exit \r\n");

    return ;
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

    w_ret = set_thread_pri(&tattr, SCHED_OTHER, MIN_PRI, 0);

    if (w_ret != 0)
    {
        FLOG_WARNING("set thread scheduling error. w_ret = %d.\r\n", w_ret);
    }

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
* Function: trans_msg_thread()
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
u_int32_t trans_msg_thread()
{
    pthread_t st_thread;
    int32_t w_ret = 0;
    
    pthread_attr_t tattr;
    pthread_attr_init (&tattr);
    
    w_ret = set_thread_pri(&tattr, SCHED_OTHER, MIN_PRI, 0);
    
    if (w_ret != 0)
    {
        FLOG_WARNING("set thread scheduling error. w_ret = %d.\r\n", w_ret);
    }
    
    /*Create   thread*/
    w_ret = pthread_create (&st_thread, &tattr, (void *)&trans_msg_process, NULL);
    /*Error*/
    if (0 != w_ret)
    {
        FLOG_ERROR("Call pthread_create error! w_ret = %d\r\n", w_ret);        
        return TRANS_FAILD;
    
    }
    
    memcpy(&(g_trans_thread_id.msg_thread), &st_thread, sizeof(pthread_t)); 

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
    
    w_ret = set_thread_pri(&tattr, SCHED_OTHER, MIN_PRI, 0);
    
    if (w_ret != 0)
    {
        FLOG_WARNING("set thread scheduling error. w_ret = %d.\r\n", w_ret);
    }

    /*Create   thread*/
    w_ret = pthread_create(&st_thread, &tattr, (void *)&trans_action_handler, g_trans_action_list);
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

    w_ret = set_thread_pri(&tattr, SCHED_OTHER, MIN_PRI, 0);

    if (w_ret != 0)
    {
        FLOG_WARNING("set thread scheduling error. w_ret = %d.\r\n", w_ret);
    }

    /*Create   thread*/
    w_ret = pthread_create(&st_thread, &tattr, (void *)&trans_timer_handler, g_p_timer_list);
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
    u_int8_t  *p_send_buf = NULL;
    u_int32_t uw_send_len = 4096;
    
    p_send_buf = (u_int8_t *)malloc(uw_send_len);
    if (NULL == p_send_buf)
    {
        FLOG_ERROR("malloc p_send_buf error! \r\n");
        return TRANS_FAILD;   
    }       
    
    memset((u_int8_t*)p_send_buf, 0, uw_send_len);      
    
    /*Send Connection message to Agent*/
    uw_ret = trans_agent_send_conn_msg(p_send_buf, g_trans_agent_socket.w_agent_socket);
    /*Error*/
    if (TRANS_SUCCESS != uw_ret)
    {
        free(p_send_buf);
        
        FLOG_ERROR("Call trans_agent_send_conn_msg error! uw_ret = %d\r\n", uw_ret);   
        return uw_ret;
    }     
    
    memset((u_int8_t*)p_send_buf, 0, uw_send_len);
    /*Send State Change message to Agent*/
    uw_ret = trans_agent_send_state_msg(p_send_buf, g_trans_agent_socket.w_agent_socket);   
    /*Error*/
    if (TRANS_SUCCESS != uw_ret)
    {
        free(p_send_buf);
        
        FLOG_ERROR("Call trans_agent_send_state_msg error! uw_ret = %d\r\n", uw_ret);   
        return uw_ret;
    } 

    #ifdef TRANS_BS_COMPILE
    memset((u_int8_t*)p_send_buf, 0, uw_send_len);
    uw_ret = trans_agent_send_hb_msg(p_send_buf, g_trans_agent_socket.w_agent_socket);
    /*Error*/
    if (TRANS_SUCCESS != uw_ret)
    {
        FLOG_ERROR("Call trans_agent_send_hb_msg error! uw_ret = %d\r\n", uw_ret); 

        free(p_send_buf);
        return uw_ret;
    } 
    #endif

    #if 0
    /****************Agent Test************/
    memset((u_int8_t*)p_send_buf, 0, uw_send_len);

    struct trans_agent_alert_info        st_alert_info;
    
    st_alert_info.us_alarm_id = 0x0205;
    st_alert_info.w_alarm_value = 345;
    
    struct trans_send_msg_to_agent st_msg_info;
    st_msg_info.f_callback = NULL;
    st_msg_info.uc_block_flag = TRANS_QUENE_NO_BLOCK;
    st_msg_info.uc_ack_flag = TRANS_ACK_FLAG_OK;
    st_msg_info.uw_resp_len = 6;
    st_msg_info.p_resp_msg = &st_alert_info;
    st_msg_info.p_reqs_msg = "alert";
    uw_ret = trans_send_rrh_msg(TRANS_SEND_TO_AGENT, &st_msg_info);
    /*Error*/
    if (TRANS_SUCCESS != uw_ret)
    {
        FLOG_ERROR("Call trans_send_rrh_msg error! uw_ret = %d\r\n", uw_ret);
    }
    
    FLOG_INFO("***********Send alarm OK **************** \r\n");
    
    struct trans_agent_spectrum_info st_spectrum_info;
    memset((u_int8_t*)&st_spectrum_info, 0, SIZEOF_TRANS_AGENT_SPECTRUM_INFO); 
    //st_spectrum_info.a_sensing = ;    
    st_spectrum_info.f_latitude = 103.12345678;
    st_spectrum_info.f_longitude = 23.12345678;
    
    FLOG_INFO("***********Set st_spectrum_info OK **************** \r\n");
    
    uw_ret = trans_agent_send_spectrum_msg(&st_spectrum_info, 
                            p_send_buf, g_trans_agent_socket);
    /*Error*/
    if (TRANS_SUCCESS != uw_ret)
    {
        FLOG_ERROR("Call trans_agent_send_spectrum_msg error! uw_ret = %d\r\n", uw_ret);        
    }
    
    //free(p_send_buf);
    #endif

    #if 0
    /****************Agent Test For periodic************/
    memset((u_int8_t*)p_send_buf, 0, uw_send_len);
    
    struct trans_periodic_sensing_info   st_periodic_info;
    int        a_per_interref[21] = {0};
    float     a_per_sensing[1024] = {0};
    
    st_periodic_info.p_per_interref = a_per_interref;
    st_periodic_info.p_per_sensing = a_per_sensing;
    
    struct trans_send_msg_to_agent st_msg_info;
    st_msg_info.f_callback = NULL;
    st_msg_info.uc_block_flag = TRANS_QUENE_NO_BLOCK;
    st_msg_info.uc_ack_flag = TRANS_ACK_FLAG_OK;
    st_msg_info.uw_resp_len = 8;
    st_msg_info.p_resp_msg = &st_periodic_info;
    st_msg_info.p_reqs_msg = "periodic";
    uw_ret = trans_send_wireless_msg(TRANS_SEND_TO_AGENT, &st_msg_info);
    /*Error*/
    if (TRANS_SUCCESS != uw_ret)
    {
        FLOG_ERROR("Call trans_send_rrh_msg error! uw_ret = %d\r\n", uw_ret);
    
    }
    #endif

    #if 0
    /****************Agent Test For topologyUpdate************/
    
    struct trans_send_msg_to_agent *p_msg_info = (struct trans_send_msg_to_agent *)malloc(SIZEOF_TRANS_SEND_MSG_TO_AGENT);
    
    p_msg_info->f_callback = NULL;
    p_msg_info->uc_block_flag = TRANS_QUENE_NO_BLOCK;
    p_msg_info->uc_ack_flag = TRANS_ACK_FLAG_OK;
    p_msg_info->uw_resp_len = 8;
    p_msg_info->p_resp_msg = "1234567";
    p_msg_info->p_reqs_msg = "topologyUpdate";
    
    uw_ret = trans_send_wireless_msg(TRANS_SEND_TO_AGENT, p_msg_info);
    /*Error*/
    if (TRANS_SUCCESS != uw_ret)
    {
        FLOG_ERROR("Call trans_send_wireless_msg error! uw_ret = %d\r\n", uw_ret);
    }

    free(p_msg_info);
    #endif


    /****************Agent Test************/

    free(p_send_buf);
 
    #endif
    return TRANS_SUCCESS;
  
}

#if 0
/*****************************************************************************+
* Function: trans_send_msg_to_agent()
* Description: Send message to Agent
* Parameters:
*           NONE
* Return Values:
*           TRANS_FAILD : 1
*           TRANS_SUCCESS  : 0
*
*  
*  Data:    2011-08-03
* 
+*****************************************************************************/
u_int32_t trans_send_msg_to_agent(u_int8_t uc_type, void *p_value)
{
    u_int32_t uw_ret = 0;
    
    #ifdef TRANS_AGENT_COMPILE 
    
    u_int8_t *p_msg = NULL; 
    u_int32_t uw_len = 8192;
    
    p_msg = (u_int8_t *)malloc(uw_len);
    if (NULL == p_msg)
    {
        FLOG_ERROR("malloc p_msg error! \r\n");
        return TRANS_FAILD;   
    }  
    
    switch (uc_type)
    {
        /*Spectrum*/
        case TRANS_SEND_SPECTRUM_TO_AGENT:
            
            memset((u_int8_t*)p_msg, 0, uw_len);
            
            uw_ret = trans_agent_forward_spectrum_msg(p_msg, p_value);
            /*Error*/
            if (TRANS_SUCCESS != uw_ret)
            {
                FLOG_ERROR("Call trans_agent_forward_spectrum_msg error! uw_ret = %d\r\n", uw_ret);        
                
            }
    
            break;   
    
        /*Error*/
        default:
            
            FLOG_ERROR("Unknow type error! uc_type = %d\r\n", uc_type);    
            uw_ret = TRANS_FAILD;
    
            break;
    
    }  
    
    free(p_msg);

    #endif

    (void) uc_type;
    (void) p_value;    
    
    return uw_ret;
}
#endif
/*****************************************************************************+
* Function: trans_send_msg_to_rrh()
* Description: Send message to RRH 
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
u_int32_t trans_send_msg_to_rrh(u_int8_t uc_type, u_int8_t uc_num, void *p_value)
{
    u_int32_t uw_ret = 0;
    
    #ifdef TRANS_RRH_COMPILE
    
    u_int8_t *p_msg = NULL; 
    u_int32_t uw_len = TRANS_RRH_SEND_MSG_MAX_LEN;

    p_msg = (u_int8_t *)malloc(uw_len);
    if (NULL == p_msg)
    {
        FLOG_ERROR("malloc p_msg error! \r\n");
        return TRANS_FAILD;   
    }  

    switch (uc_type)
    {
        /*CHANNEL_MODE : 1, 2, 3*/
        case TRANS_SEND_CFG_CHANNEL_MODE:
            
            memset((u_int8_t*)p_msg, 0, uw_len);
            
            uw_ret = trans_rrh_send_msg_ch_mode(p_msg, uc_num, p_value);
            /*Error*/
            if (TRANS_SUCCESS != uw_ret)
            {
                FLOG_ERROR("Call trans_rrh_send_msg_ch_mode error! uw_ret = %d\r\n", uw_ret);        
                
            }

            break;   

        /*CHANNEL_FREQ */
        case TRANS_SEND_CFG_CHANNEL_FREQ:
            
            memset((u_int8_t*)p_msg, 0, uw_len); 
            
            uw_ret = trans_rrh_send_msg_ch_freq(p_msg, uc_num, p_value);
            /*Error*/
            if (TRANS_SUCCESS != uw_ret)
            {
                FLOG_ERROR("Call trans_rrh_send_msg_ch_freq error! uw_ret = %d\r\n", uw_ret);        
                
            }
        
            break;

        /*CHANNEL_FLAG : 0; 1*/
        case TRANS_SEND_CFG_CHANNEL_FLAG:
            
            memset((u_int8_t*)p_msg, 0, uw_len);

            uw_ret = trans_rrh_send_msg_ch_flag(p_msg, uc_num, p_value);
            /*Error*/
            if (TRANS_SUCCESS != uw_ret)
            {
                FLOG_ERROR("Call trans_rrh_send_msg_ch_flag error! uw_ret = %d\r\n", uw_ret);        
                
            }
        
            break;

        /*DL_PRESEND_TIME*/
        case TRANS_SEND_CFG_DL_PRESEND_TIME:
            
            memset((u_int8_t*)p_msg, 0, uw_len);
        
            uw_ret = trans_rrh_send_msg_dl_pre_time(p_msg, uc_num, p_value);
            /*Error*/
            if (TRANS_SUCCESS != uw_ret)
            {
                FLOG_ERROR("Call trans_rrh_send_msg_dl_pre_time error! uw_ret = %d\r\n", uw_ret);        
                
            }
        
            break;

        /*TX_LEN*/
        case TRANS_SEND_CFG_TX_LEN:
            
            memset((u_int8_t*)p_msg, 0, uw_len);
        
            uw_ret = trans_rrh_send_msg_tx_len(p_msg, uc_num, p_value);
            /*Error*/
            if (TRANS_SUCCESS != uw_ret)
            {
                FLOG_ERROR("Call trans_rrh_send_msg_tx_len error! uw_ret = %d\r\n", uw_ret);        
                
            }
        
            break;

        /*RX_LEN*/
        case TRANS_SEND_CFG_RX_LEN:
            
            memset((u_int8_t*)p_msg, 0, uw_len);
        
            uw_ret = trans_rrh_send_msg_rx_len(p_msg, uc_num, p_value);
            /*Error*/
            if (TRANS_SUCCESS != uw_ret)
            {
                FLOG_ERROR("Call trans_rrh_send_msg_rx_len error! uw_ret = %d\r\n", uw_ret);        
                
            }
        
            break;

        /*GPS_ENABLE*/
        case TRANS_SEND_CFG_GPS_ENABLE:
            
            memset((u_int8_t*)p_msg, 0, uw_len);
            
            uw_ret = trans_rrh_send_msg_gps_enable(p_msg, uc_num, p_value);
            /*Error*/
            if (TRANS_SUCCESS != uw_ret)
            {
                FLOG_ERROR("Call trans_rrh_send_msg_gps_enable error! uw_ret = %d\r\n", uw_ret);        
                
            }
        
            break;  

         /*OUTPUT_POWER*/
        case TRANS_SEND_CFG_OUTPUT_POWER:
       
            memset((u_int8_t*)p_msg, 0, uw_len);
            
            uw_ret = trans_rrh_send_msg_output_pow(p_msg, uc_num, p_value);
            /*Error*/
            if (TRANS_SUCCESS != uw_ret)
            {
                FLOG_ERROR("Call trans_rrh_send_msg_output_pow error! uw_ret = %d\r\n", uw_ret);        
                
            }
        
            break;        

         /*PA_SWITCH*/
        case TRANS_SEND_CFG_PA_SWITCH:
        
            memset((u_int8_t*)p_msg, 0, uw_len);
            
            uw_ret = trans_rrh_send_msg_pa_switch(p_msg, uc_num, p_value);
            /*Error*/
            if (TRANS_SUCCESS != uw_ret)
            {
                FLOG_ERROR("Call trans_rrh_send_msg_pa_switch error! uw_ret = %d\r\n", uw_ret);        
                
            }
        
            break;   
         /*AGC*/
        case TRANS_SEND_CFG_AGC:
        
            memset((u_int8_t*)p_msg, 0, uw_len);
            
            uw_ret = trans_rrh_send_msg_agc(p_msg, uc_num, p_value);
            /*Error*/
            if (TRANS_SUCCESS != uw_ret)
            {
                FLOG_ERROR("Call trans_rrh_send_msg_pa_switch error! uw_ret = %d\r\n", uw_ret);        
                
            }
        
            break;   
         /*CHAN1_RX_PGC*/
        case TRANS_SEND_CFG_CH_RX_PGC:
        
            memset((u_int8_t*)p_msg, 0, uw_len);
            
            uw_ret = trans_rrh_send_msg_ch_rx_pgc(p_msg, uc_num, p_value);
            /*Error*/
            if (TRANS_SUCCESS != uw_ret)
            {
                FLOG_ERROR("Call trans_rrh_send_msg_pa_switch error! uw_ret = %d\r\n", uw_ret);        
                
            }
        
            break; 

        /*Initial Step Query----basic information */
        case TRANS_SEND_INIT_QUERY:
            
            memset((u_int8_t*)p_msg, 0, uw_len);
        
            uw_ret = trans_rrh_send_msg_q_gps(p_msg);
            if (TRANS_SUCCESS != uw_ret) 
            {   
                FLOG_ERROR("Call trans_rrh_send_msg_q_gps error! uw_ret = %d\r\n", uw_ret);
                free(p_msg);
                return 2;     
            }   

           memset((u_int8_t*)p_msg, 0, uw_len);
           
           uw_ret = trans_rrh_send_msg_q_power(p_msg);
           if (TRANS_SUCCESS != uw_ret) 
           {   
               FLOG_ERROR("Call trans_rrh_send_msg_q_power error! uw_ret = %d\r\n", uw_ret);
               free(p_msg);
               return 3;     
           }  
           
           break;
            
        case TRANS_SEND_QUERY_POWER:

            memset((u_int8_t*)p_msg, 0, uw_len);
            
            uw_ret = trans_rrh_send_msg_q_power(p_msg);
            if (TRANS_SUCCESS != uw_ret) 
            {   
                FLOG_ERROR("Call trans_rrh_send_msg_q_power error! uw_ret = %d\r\n", uw_ret);
                free(p_msg);
                return 4;     
            }  

            break;

        case TRANS_SEND_QUERY_GPS_LOCK:
        
            memset((u_int8_t*)p_msg, 0, uw_len);
            
            uw_ret = trans_rrh_send_msg_q_gps_lock(p_msg, p_value);
            if (TRANS_SUCCESS != uw_ret) 
            {   
                FLOG_ERROR("Call trans_rrh_send_msg_q_gps_lock error! uw_ret = %d\r\n", uw_ret);
                free(p_msg);
                return 5;     
            }  
        
            break;
            
        default:
            
            FLOG_ERROR("Unknow type error! uc_type = %d\r\n", uc_type);    
            uw_ret = TRANS_FAILD;

            break;
    
    }  

    free(p_msg);

    #else
    uc_type = uc_type;
    uc_num =  uc_num;
    (void)p_value;
    #endif

    return uw_ret;
}


/*****************************************************************************+
* Function: trans_send_action_msg()
* Description: Send message to Action 
* Parameters:
*           uc_send_type :  enum trans_send_msg_enum 
*           p_send_info :     struct trans_send_query_to_rrh
*                                      struct trans_send_cfg_to_rrh     
*                                      struct trans_send_msg_to_monitor
* 
* Return Values:
*           TRANS_FAILD : 1
*           TRANS_SUCCESS  : 0
*
*  
*  Data:    2011-09-05
* 
+*****************************************************************************/
u_int32_t trans_send_action_msg(u_int8_t uc_send_type, void * p_send_info)
{
    u_int32_t uw_ret = 0;
    u_int8_t *p_send_msg = NULL; 
    u_int32_t uw_len = TRANS_ACTION_SEND_MSG_MAX_LEN;
    
    p_send_msg = (u_int8_t *)malloc(uw_len);
    if (NULL == p_send_msg)
    {
        FLOG_ERROR("malloc p_send_msg error! \r\n");
        return TRANS_FAILD;   
    }  
    
    /*memset*/
    memset((u_int8_t*)p_send_msg, 0, uw_len);

    switch (uc_send_type)
    {
        #ifdef TRANS_RRH_COMPILE
        /*Send Message to RRH for Query */
        case TRANS_SEND_TO_RRH_QUERY:
    
            uw_ret = trans_rrh_send_action_query(p_send_info, uw_len, p_send_msg);
            if (TRANS_SUCCESS != uw_ret)
            {
                FLOG_ERROR("Call trans_rrh_send_action_query error! uw_ret = %d\r\n", uw_ret);
            }
            
            break;
            
        /*Send Message to RRH for Config */
        case TRANS_SEND_TO_RRH_CFG:

            uw_ret = trans_rrh_send_action_cfg(p_send_info, uw_len, p_send_msg);
            if (TRANS_SUCCESS != uw_ret)
            {
                FLOG_ERROR("Call trans_rrh_send_action_cfg error! uw_ret = %d\r\n", uw_ret);
            }

            break;
        #endif
        
        #ifdef TRANS_MONITOR_COMPILE
        /*Send Message to Monitor*/
        case TRANS_SEND_TO_MONITOR:

            uw_ret = trans_monitor_send_action_msg(p_send_info, uw_len, p_send_msg);
            if (TRANS_SUCCESS != uw_ret)
            {
                FLOG_ERROR("Call trans_monitor_send_action_msg error! uw_ret = %d\r\n", uw_ret);
            }

            break;
        #endif
        
        /*Send Message to Agent*/
        case TRANS_SEND_TO_AGENT:
        /*Send Message to Action*/
        //case TRANS_SEND_TO_ACTION:            
        /*Send Message to Wireless*/
        //case TRANS_SEND_TO_WIRELESS:             
                    
        default:
    
            FLOG_ERROR("Send type error! send_type = %d\r\n", uc_send_type);
            uw_ret = TRANS_FAILD;
    
    }
    
    free(p_send_msg);
    
    return uw_ret;
}

/*****************************************************************************+
* Function: trans_send_wireless_msg()
* Description: Send message to Wireless 
* Parameters:
*           uc_send_type :  enum trans_send_msg_enum 
*           p_send_info :     struct trans_send_query_to_rrh
*                                      struct trans_send_cfg_to_rrh     
*                                      struct trans_send_msg_to_monitor
*                                      struct trans_send_msg_to_agent
* Return Values:
*           TRANS_FAILD : 1
*           TRANS_SUCCESS  : 0
*
*  
*  Data:    2011-09-05
* 
+*****************************************************************************/
u_int32_t trans_send_wireless_msg(u_int8_t uc_send_type, void * p_send_info)
{
    u_int32_t uw_ret = 0;
    u_int8_t *p_send_msg = NULL; 
    u_int32_t uw_len = TRANS_WIRELESS_SEND_MSG_MAX_LEN;
    
    p_send_msg = (u_int8_t *)malloc(uw_len);
    if (NULL == p_send_msg)
    {
        FLOG_ERROR("malloc p_send_msg error! \r\n");
        return TRANS_FAILD;   
    }  
    
    /*memset*/
    memset((u_int8_t*)p_send_msg, 0, uw_len);
    
    switch (uc_send_type)
    {
        /*Send Message to RRH for Query */
        case TRANS_SEND_TO_RRH_QUERY:
        #ifdef TRANS_RRH_COMPILE
            uw_ret = trans_rrh_send_wireless_query(p_send_info, uw_len, p_send_msg);
            if (TRANS_SUCCESS != uw_ret)
            {
                FLOG_ERROR("Call trans_rrh_send_wireless_query error! uw_ret = %d\r\n", uw_ret);
            }
        #endif   
            break;
            
        /*Send Message to RRH for Config */
        case TRANS_SEND_TO_RRH_CFG:
        #ifdef TRANS_RRH_COMPILE
            uw_ret = trans_rrh_send_wireless_cfg(p_send_info, uw_len, p_send_msg);
            if (TRANS_SUCCESS != uw_ret)
            {
                FLOG_ERROR("Call trans_rrh_send_wireless_cfg error! uw_ret = %d\r\n", uw_ret);
            }
        #endif
            break;

        /*Send Message to Monitor*/
        case TRANS_SEND_TO_MONITOR:
        #ifdef TRANS_MONITOR_COMPILE
            uw_ret = trans_monitor_send_wireless_msg(p_send_info, uw_len, p_send_msg);
            if (TRANS_SUCCESS != uw_ret)
            {
                FLOG_ERROR("Call trans_monitor_send_wireless_msg error! uw_ret = %d\r\n", uw_ret);
            }
        #endif
            break;            
        
        /*Send Message to Agent*/
        case TRANS_SEND_TO_AGENT:
        #ifdef TRANS_AGENT_COMPILE    
            uw_ret = trans_agent_send_msg(p_send_info, uw_len, p_send_msg);
            if (TRANS_SUCCESS != uw_ret)
            {
                FLOG_ERROR("Call trans_agent_send_msg error! uw_ret = %d\r\n", uw_ret);
            }
        #endif   
            break;

        /*Send Message to Action*/
        //case TRANS_SEND_TO_ACTION:            
        /*Send Message to Wireless*/
        //case TRANS_SEND_TO_WIRELESS:             
                    
        default:
    
            FLOG_ERROR("Send type error! send_type = %d\r\n", uc_send_type);
            uw_ret = TRANS_FAILD;
    
    }
    
    free(p_send_msg);
    
    return uw_ret;

}

/*****************************************************************************+
* Function: trans_send_rrh_msg()
* Description: Send message to RRH 
* Parameters:
*           uc_send_type :  enum trans_send_msg_enum 
*           p_send_info :     struct trans_send_query_to_rrh
*                                      struct trans_send_cfg_to_rrh     
*                                      struct trans_send_msg_to_monitor
*                                      struct trans_send_msg_to_agent
* Return Values:
*           TRANS_FAILD : 1
*           TRANS_SUCCESS  : 0
*
*  
*  Data:    2011-09-05
* 
+*****************************************************************************/
u_int32_t trans_send_rrh_msg(u_int8_t uc_send_type, void * p_send_info)
{
    u_int32_t uw_ret = 0;
    u_int8_t *p_send_msg = NULL; 
    u_int32_t uw_len = TRANS_RRH_SEND_MSG_MAX_LEN;
    
    p_send_msg = (u_int8_t *)malloc(uw_len);
    if (NULL == p_send_msg)
    {
        FLOG_ERROR("malloc p_send_msg error! \r\n");
        return TRANS_FAILD;   
    }  
    
    /*memset*/
    memset((u_int8_t*)p_send_msg, 0, uw_len);
    
    switch (uc_send_type)
    {
        #ifdef TRANS_MONITOR_COMPILE
        /*Send Message to Monitor*/
        case TRANS_SEND_TO_MONITOR:
            uw_ret = trans_monitor_send_rrh_msg(p_send_info, uw_len, p_send_msg);
            if (TRANS_SUCCESS != uw_ret)
            {
                FLOG_ERROR("Call trans_monitor_send_rrh_msg error! uw_ret = %d\r\n", uw_ret);
            }
    
            break;
        #endif
            
        #ifdef TRANS_AGENT_COMPILE
        /*Send Message to Agent*/
        case TRANS_SEND_TO_AGENT:
            
            uw_ret = trans_agent_send_msg(p_send_info, uw_len, p_send_msg);
            if (TRANS_SUCCESS != uw_ret)
            {
                FLOG_ERROR("Call trans_agent_send_msg error! uw_ret = %d\r\n", uw_ret);
            }
            
            break;
        #endif

        /*Send Message to Action*/
        //case TRANS_SEND_TO_ACTION:   
        /*Send Message to Wireless*/
        //case TRANS_SEND_TO_WIRELESS:   
        /*Send Message to RRH for Query */
        case TRANS_SEND_TO_RRH_QUERY:
        /*Send Message to RRH for Config */
        case TRANS_SEND_TO_RRH_CFG:
                    
        default:
    
            p_send_info = p_send_info;
            FLOG_ERROR("Send type error! send_type = %d\r\n", uc_send_type);
            uw_ret = TRANS_FAILD;
    
    }
    
    free(p_send_msg);
  
    return uw_ret;

}


/*****************************************************************************+
* Function: trans_send_monitor_msg()
* Description: Send message to Monitor 
* Parameters:
*           uc_send_type :  enum trans_send_msg_enum 
*           p_send_info :     struct trans_send_query_to_rrh
*                                      struct trans_send_cfg_to_rrh     
*                                      struct trans_send_msg_to_monitor
* 
* Return Values:
*           TRANS_FAILD : 1
*           TRANS_SUCCESS  : 0
*
*  
*  Data:    2011-09-05
* 
+*****************************************************************************/
u_int32_t trans_send_monitor_msg(u_int8_t uc_send_type, void * p_send_info)
{
    u_int32_t uw_ret = 0;
    u_int8_t *p_send_msg = NULL; 
    u_int32_t uw_len = TRANS_MONITOR_SEND_MSG_MAX_LEN;
    
    p_send_msg = (u_int8_t *)malloc(uw_len);
    if (NULL == p_send_msg)
    {
        FLOG_ERROR("malloc p_send_msg error! \r\n");
        return TRANS_FAILD;   
    }  

    /*memset*/
    memset((u_int8_t*)p_send_msg, 0, uw_len);

    switch (uc_send_type)
    {
        #ifdef TRANS_RRH_COMPILE
        /*Send Message to RRH for Query */
        case TRANS_SEND_TO_RRH_QUERY:

            uw_ret = trans_rrh_send_monitor_query(p_send_info, uw_len, p_send_msg);
            if (TRANS_SUCCESS != uw_ret)
            {
                FLOG_ERROR("Call trans_rrh_send_monitor_query error! uw_ret = %d\r\n", uw_ret);

            }
            break;
            
        /*Send Message to RRH for Config */
        case TRANS_SEND_TO_RRH_CFG:

            uw_ret = trans_rrh_send_monitor_cfg(p_send_info, uw_len, p_send_msg);
            if (TRANS_SUCCESS != uw_ret)
            {
                FLOG_ERROR("Call trans_rrh_send_monitor_cfg error! uw_ret = %d\r\n", uw_ret);
            
            }

            break;
        #endif

        #ifdef TRANS_MONITOR_COMPILE
        /*Send Message to Monitor*/
        case TRANS_SEND_TO_MONITOR:
        
            uw_ret = trans_monitor_send_monitor_msg(p_send_info, uw_len, p_send_msg);
            if (TRANS_SUCCESS != uw_ret)
            {
                FLOG_ERROR("Call trans_monitor_send_monitor_msg error! uw_ret = %d\r\n", uw_ret);
            }
        
            break;
        #endif
            
        /*Send Message to Agent*/
        case TRANS_SEND_TO_AGENT:
        /*Send Message to Action*/
        //case TRANS_SEND_TO_ACTION:            
        /*Send Message to Wireless*/
        //case TRANS_SEND_TO_WIRELESS:             
                    
        default:
        
            FLOG_ERROR("Send type error! send_type = %d\r\n", uc_send_type);
            uw_ret = TRANS_FAILD;
    
    }

    free(p_send_msg);

    return uw_ret;
}

/*****************************************************************************+
* Function: trans_send_agent_msg()
* Description: Send message to Agent
* Parameters:
*           uc_send_type :  enum trans_send_msg_enum 
*           p_send_info :     struct trans_send_query_to_rrh
*                                      struct trans_send_cfg_to_rrh     
*                                      struct trans_send_msg_to_monitor
*                                      struct trans_send_msg_to_agent
* 
* Return Values:
*           TRANS_FAILD : 1
*           TRANS_SUCCESS  : 0
*
*  
*  Data:    2011-09-05
* 
+*****************************************************************************/
u_int32_t trans_send_agent_msg(u_int8_t uc_send_type, void * p_send_info)
{
    u_int32_t uw_ret = 0;
    u_int8_t *p_send_msg = NULL; 
    u_int32_t uw_len = TRANS_AGENT_SEND_MSG_MAX_LEN;
    
    p_send_msg = (u_int8_t *)malloc(uw_len);
    if (NULL == p_send_msg)
    {
        FLOG_ERROR("malloc p_send_msg error! \r\n");
        return TRANS_FAILD;   
    }  
    
    /*memset*/
    memset((u_int8_t*)p_send_msg, 0, uw_len);

    switch (uc_send_type)
    {
        #ifdef TRANS_RRH_COMPILE
        /*Send Message to RRH for Query */
        case TRANS_SEND_TO_RRH_QUERY:
    
            uw_ret = trans_rrh_send_agent_query(p_send_info, uw_len, p_send_msg);
            if (TRANS_SUCCESS != uw_ret)
            {
                FLOG_ERROR("Call trans_rrh_send_agent_query error! uw_ret = %d\r\n", uw_ret);
            }
            
            break;
        #endif
            
        /*Send Message to RRH for Config */
        case TRANS_SEND_TO_RRH_CFG:
        /*Send Message to Monitor*/
        case TRANS_SEND_TO_MONITOR:
        /*Send Message to Agent*/
        case TRANS_SEND_TO_AGENT:
        /*Send Message to Action*/
        //case TRANS_SEND_TO_ACTION:            
        /*Send Message to Wireless*/
        //case TRANS_SEND_TO_WIRELESS:             
                    
        default:
    
            FLOG_ERROR("Send type error! send_type = %d\r\n", uc_send_type);
            uw_ret = TRANS_FAILD;
    
    }
    
    free(p_send_msg);
    
    return uw_ret;
}

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

    uw_ret = trans_msg_thread();  
    if (TRANS_SUCCESS != uw_ret)
    {
        FLOG_ERROR("Call trans_msg_thread error! uw_ret = %d\r\n", uw_ret);     
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

            if (TRANS_REGISTER_FUN_BUF <= us_operation)
            {
                FLOG_ERROR("Out of rang : register_name = %d!\r\n", us_register_name);  
                return TRANS_FAILD;
            }
            
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
                            
            break;
    
        
        default:
    
            FLOG_ERROR("Type error! us_register_type = %d\r\n", us_register_type);
    
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
    
    FLOG_DEBUG("Enter \r\n");
    
    if (0 == g_trans_thread_flag)
    {
        FLOG_ERROR("Adapt module is not create!\r\n ");  
        
        return TRANS_FAILD;
    }
  
    /*Check thread is ont 0*/
    /*Cancel thread*/
    pthread_cancel (g_trans_thread_id.timer_thread);
    pthread_join (g_trans_thread_id.timer_thread, NULL);
    
    pthread_cancel (g_trans_thread_id.msg_thread);
    pthread_join (g_trans_thread_id.msg_thread, NULL);

    pthread_cancel (g_trans_thread_id.action_thread);
    pthread_join (g_trans_thread_id.action_thread, NULL);

    #ifdef TRANS_MONITOR_COMPILE
    /*Cancel thread*/
    pthread_cancel (g_trans_thread_id.monitor_thread);
    pthread_join (g_trans_thread_id.monitor_thread, NULL);
    #endif 
    
    /*Free memory*/
    free(g_trans_rev_buf);
    free(g_trans_send_buf);
    
    g_trans_rev_buf = NULL;
    g_trans_send_buf = NULL;
    
    g_trans_thread_flag = 0;

    
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
    
    FLOG_DEBUG("Exit \r\n");

    return TRANS_SUCCESS;
}


