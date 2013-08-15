/* ----------------------------------------------------------------------------
 IBM Confidential

 IBM Wireless M2M Platform

 (C)Copyright IBM Corp. 2009, 2010, 2011

 All Rights Reserved.

 File Name: trans_common.c

 Change Activity:

 Date             Description of Change                            By
 -----------      ---------------------                            --------
 30-Nov.2011      Created                                          E Wulan

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

struct trans_device_info  g_trans_local_device_info;

extern int32_t   g_trans_monitor_socket_now;

struct trans_common_sockfd g_trans_common_sockfd;

extern int connect_agent (void);


#if 0
/*****************************************************************************+
* Function: trans_common_socket_send()
* Description: 
* Parameters:
*           NONE
* Return Values:
*           TRANS_FAILD : 1
*           TRANS_SUCCESS  : 0
*
*  Data:    2012-05-04
* 
+*****************************************************************************/
u_int32_t trans_common_socket_send(int32_t w_sockfd,
                                u_int8_t *p_mac,
                                void * p_send_buf,
                                u_int32_t uw_send_len)
{
    int32_t w_ret = 0;
    int32_t     w_send_len_temp = 0;
    int32_t w_total_send_len = 0;

    u_int32_t uw_congestion_num = 0;
    
    time_t   now;
    struct tm   *timenow;
    
    /*Send New Message to Monitor*/
    while (w_total_send_len < ((int)uw_send_len))
    {
        if (uw_send_len - w_total_send_len > 2*1024)
        {
            w_send_len_temp = 2*1024 ;
        }
        else
        {
            w_send_len_temp = uw_send_len - w_total_send_len;
        }

        uw_congestion_num = trans_device_get_congestion_num(p_mac, 0); 
        
        /*Not Congestion*/
        if (0 == uw_congestion_num)
        {
            /*Send New Message to Monitor*/
            w_ret = send(w_sockfd, (p_send_buf + w_total_send_len), w_send_len_temp, 0);
            
            if(w_ret <= 0)
            {
                //close(w_sockfd);
                if (EAGAIN == errno)
                {
                    FLOG_ERROR("Congestion start! \r\n");
                
                    uw_congestion_num = trans_device_get_congestion_num(p_mac, 1); 
                }  

                time(&now);
                timenow = localtime(&now);
                
                FLOG_ERROR("Send socket:%d recv() error! return:%d, errno:%d, errortext:'%s', time:%s", 
                    w_sockfd, w_ret, errno, strerror(errno), asctime(timenow));

                return TRANS_FAILD;
            }  
    
            if (w_ret != w_send_len_temp)
            {
                time(&now);
                timenow = localtime(&now);
                
                FLOG_ERROR("send length error! %d, %d. Time : %s.\r\n", w_ret, w_send_len_temp, asctime(timenow));
                
                w_send_len_temp = w_ret;
            }
    
            w_total_send_len = w_total_send_len + w_send_len_temp;
            
        }
        /*Congestion*/
        else
        {
            FLOG_ERROR("Congestion : discard the message! \r\n");
            
            return TRANS_FAILD;
        }

    }
    
    return TRANS_SUCCESS;
}
#endif


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
    #ifndef TRANS_MONITOR_TEST_COMPILE      
    
    struct queue_msg st_quene_msg;
    struct trans_queue_msg *p_quene_msg =NULL;
    u_int32_t   uw_len = 0;
    
    FLOG_DEBUG_TRANS(g_trans_debug_com, "Enter \r\n");  
      
    if (NULL == p_en_quene)
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

    if ((0 != uw_len) && (NULL != p_msg))
    {
        /* Allocate a memory.  */
        p_quene_msg->p_msg = (u_int8_t *)malloc(uw_len);
        if (NULL == p_quene_msg->p_msg)
        {
            FLOG_ERROR("2 malloc p_quene_msg->p_msg error! \r\n");
            return TRANS_FAILD;   
        }

        p_quene_msg->uw_len= uw_len;
        memcpy(p_quene_msg->p_msg, (u_int8_t *)p_msg, uw_len); 
    }
    else
    {
        p_quene_msg->uw_len= 0;
        p_quene_msg->p_msg = NULL;
    }   
   
    p_quene_msg->uc_result = p_en_quene->uc_result;
    p_quene_msg->uw_src_moudle = p_en_quene->uw_src_moudle;

    FLOG_DEBUG_TRANS(g_trans_debug_com, "%d, %d, %d\r\n", p_quene_msg->uc_result, p_quene_msg->uw_src_moudle, p_quene_msg->uw_len); 
        
    st_quene_msg.my_type = bsagent_en_id;
    st_quene_msg.p_buf = p_quene_msg;
    
    //trans_debug_msg_print(p_quene_msg->p_msg, 10, g_trans_debug_com);

    if (wmrt_enqueue (bsagent_en_id, &st_quene_msg, sizeof(struct queue_msg))
            == -1)
    {
        FLOG_ERROR ("ENQUEUE ERROR\n");
    }
    
    FLOG_DEBUG_TRANS(g_trans_debug_com, "Exit \r\n"); 

    #else
    (void)p_msg;
    (void)p_en_quene;

    #endif

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
    #ifndef TRANS_MONITOR_TEST_COMPILE      
    
    struct queue_msg st_quene_msg;
    struct trans_queue_msg *p_quene_msg =NULL;
    
    FLOG_DEBUG_TRANS(g_trans_debug_com, "Enter \r\n");   
     
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

    FLOG_DEBUG_TRANS(g_trans_debug_com, "%d, %d, %d\r\n", p_quene_msg->uc_result, p_quene_msg->uw_src_moudle, p_quene_msg->uw_len);

    //trans_debug_msg_print(p_quene_msg->p_msg, 10, g_trans_debug_com);
    
    if (NULL != p_quene_msg->p_msg)
    {
        free(p_quene_msg->p_msg);
    }
    
    free(p_quene_msg);

    FLOG_DEBUG_TRANS(g_trans_debug_com, "Exit \r\n"); 

    #else
    (void)p_result;

    #endif

    return TRANS_SUCCESS;
}

/*****************************************************************************+
* Function: trans_common_msg_en_queue()
* Description: 
* Parameters:
*           NONE
* Return Values:
*           TRANS_FAILD : 1
*           TRANS_SUCCESS  : 0
*
*  
*  Data:    2011-12-05
* 
+*****************************************************************************/
u_int32_t trans_common_msg_en_queue(struct trans_common_queue *p_quene_info)
{
    #ifndef TRANS_MONITOR_TEST_COMPILE      
    
    struct queue_msg st_quene_msg;
    struct trans_common_queue *p_quene = NULL;
    //u_int32_t   uw_len = 0;
    
    FLOG_DEBUG_TRANS(g_trans_debug_com, "Enter \r\n");  
    
    /* Allocate a memory.  */
    p_quene = (struct trans_common_queue *)malloc(SIZEOF_TRANS_COMMON_QUEUE);
    
    if (NULL == p_quene)
    {
        FLOG_ERROR("malloc p_quene error! \r\n");
        return TRANS_FAILD;   
    }

    memset((u_int8_t*)p_quene, 0, SIZEOF_TRANS_COMMON_QUEUE); 

    memcpy(p_quene, p_quene_info, SIZEOF_TRANS_COMMON_QUEUE);

    FLOG_DEBUG_TRANS(g_trans_debug_com, "sockfd = %d, uc_module_type = %d, uw_len = %d\r\n", p_quene->w_sockfd, p_quene->uc_module_type, p_quene->uw_len); 
        
    st_quene_msg.my_type = trans_msg_en_id;
    st_quene_msg.p_buf = p_quene;
    
    //trans_debug_msg_print(p_quene_msg->p_msg, 10, g_trans_debug_com);

    if (wmrt_enqueue (trans_msg_en_id, &st_quene_msg, sizeof(struct queue_msg))
            == -1)
    {
        FLOG_ERROR ("ENQUEUE ERROR\n");
    }
    
    trans_debug_msg_print(p_quene_info->p_msg, p_quene->uw_len, g_trans_debug_com);  
    //FLOG_DEBUG_TRANS(g_trans_debug_com, "Len = %d \r\n", p_quene_info->uw_len); 

    FLOG_DEBUG_TRANS(g_trans_debug_com, "Exit \r\n"); 

    #else
    (void)p_quene_info;

    #endif

    return TRANS_SUCCESS;
}

/*****************************************************************************+
* Function: trans_common_msg_de_queue()
* Description: 
* Parameters:
*           NONE
* Return Values:
*           TRANS_FAILD : 1
*           TRANS_SUCCESS  : 0
*
*  
*  Data:    2011-12-05
* 
+*****************************************************************************/
u_int32_t trans_common_msg_de_queue(struct trans_common_queue *p_quene_info)
{
    #ifndef TRANS_MONITOR_TEST_COMPILE      
    
    struct queue_msg st_quene_msg;
    
    FLOG_DEBUG_TRANS(g_trans_debug_com, "Enter \r\n");   
     
    if (NULL == p_quene_info )
    {
        FLOG_ERROR("1 NULL PTR! \r\n");
        return TRANS_FAILD;  
    }

    st_quene_msg.my_type = trans_msg_de_id;
    
    if (wmrt_dequeue (trans_msg_de_id, &st_quene_msg, sizeof(struct queue_msg))
            == -1)
    {
        FLOG_ERROR ("DEQUEUE ERROR\n");
    }

    if (NULL == st_quene_msg.p_buf)
    {
        FLOG_ERROR("2 NULL PTR! \r\n");
        return TRANS_FAILD;      
    } 
    
    memcpy(p_quene_info, st_quene_msg.p_buf, SIZEOF_TRANS_COMMON_QUEUE);

    FLOG_DEBUG_TRANS(g_trans_debug_com, "sockfd = %d, uc_module_type = %d, uw_len = %d\r\n", p_quene_info->w_sockfd, p_quene_info->uc_module_type, p_quene_info->uw_len); 
    
    free(st_quene_msg.p_buf);

    trans_debug_msg_print(p_quene_info->p_msg, p_quene_info->uw_len, g_trans_debug_com);  
    //FLOG_DEBUG_TRANS(g_trans_debug_com, "Len = %d \r\n", p_quene_info->uw_len); 

    FLOG_DEBUG_TRANS(g_trans_debug_com, "Exit \r\n"); 

    #else
    (void)p_quene_info;

    #endif
   
    return TRANS_SUCCESS;
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
    u_int32_t uw_len = 0;
    
    #if 0
    p_send_msg = (u_int8_t *)malloc(uw_len);
    if (NULL == p_send_msg)
    {
        FLOG_ERROR("malloc p_send_msg error! \r\n");
        return TRANS_FAILD;   
    }  
    
    /*memset*/
    memset((u_int8_t*)p_send_msg, 0, uw_len);
    #endif
    
    switch (uc_send_type)
    {
        #ifdef TRANS_RRH_COMPILE
        /*Send Message to RRH for Query */
        case TRANS_SEND_TO_RRH:
    
            uw_ret = trans_rrh_send_action(p_send_info, uw_len, p_send_msg);
            if (TRANS_SUCCESS != uw_ret)
            {
                FLOG_ERROR("Call trans_rrh_send_action error! uw_ret = %d\r\n", uw_ret);
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
    
    //free(p_send_msg);
    
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
    u_int32_t uw_len = 0;
    
    #if 0
    p_send_msg = (u_int8_t *)malloc(uw_len);
    if (NULL == p_send_msg)
    {
        FLOG_ERROR("malloc p_send_msg error! \r\n");
        return TRANS_FAILD;   
    }  
    
    /*memset*/
    memset((u_int8_t*)p_send_msg, 0, uw_len);
   #endif
   
    switch (uc_send_type)
    {
        #ifdef TRANS_RRH_COMPILE
        /*Send Message to RRH for Query */
        case TRANS_SEND_TO_RRH:
    
            uw_ret = trans_rrh_send_bs(p_send_info, uw_len, p_send_msg);
            if (TRANS_SUCCESS != uw_ret)
            {
                FLOG_ERROR("Call trans_rrh_send_bs error! uw_ret = %d\r\n", uw_ret);
            }
            
            break;
            
        #endif

        #ifdef TRANS_MONITOR_COMPILE
        /*Send Message to Monitor*/
        case TRANS_SEND_TO_MONITOR:
    
            uw_ret = trans_monitor_send_wireless_msg(p_send_info, uw_len, p_send_msg);
            if (TRANS_SUCCESS != uw_ret)
            {
                FLOG_ERROR("Call trans_monitor_send_wireless_msg error! uw_ret = %d\r\n", uw_ret);
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
                    
        default:
    
            FLOG_ERROR("Send type error! send_type = %d\r\n", uc_send_type);
            uw_ret = TRANS_FAILD;
    
    }
    
    //free(p_send_msg);
    
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

    #if (defined TRANS_AGENT_COMPILE) || (defined TRANS_MONITOR_COMPILE)
    u_int8_t *p_send_msg = NULL; 
    u_int32_t uw_len = 0;
    #endif
    
    #if 0
    p_send_msg = (u_int8_t *)malloc(uw_len);
    if (NULL == p_send_msg)
    {
        FLOG_ERROR("malloc p_send_msg error! \r\n");
        return TRANS_FAILD;   
    }  
    
    /*memset*/
    memset((u_int8_t*)p_send_msg, 0, uw_len);
    #endif
    
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
        /*Send Message to RRH  */
        case TRANS_SEND_TO_RRH:

                    
        default:
    
            p_send_info = p_send_info;
            FLOG_ERROR("Send type error! send_type = %d\r\n", uc_send_type);
            uw_ret = TRANS_FAILD;
    
    }
    
    //free(p_send_msg);
  
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
    u_int32_t uw_len = 0;
    
    #if 0
    p_send_msg = (u_int8_t *)malloc(uw_len);
    if (NULL == p_send_msg)
    {
        FLOG_ERROR("malloc p_send_msg error! \r\n");
        return TRANS_FAILD;   
    }  

    /*memset*/
    memset((u_int8_t*)p_send_msg, 0, uw_len);
    #endif
    
    switch (uc_send_type)
    {
        #ifdef TRANS_RRH_COMPILE
        /*Send Message to RRH  */
        case TRANS_SEND_TO_RRH:

            uw_ret = trans_rrh_send_monitor(p_send_info, uw_len, p_send_msg);
            if (TRANS_SUCCESS != uw_ret)
            {
                FLOG_ERROR("Call trans_rrh_send_monitor error! uw_ret = %d\r\n", uw_ret);

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

    //free(p_send_msg);

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
    u_int32_t uw_len = 0;

    #if 0
    p_send_msg = (u_int8_t *)malloc(uw_len);
    if (NULL == p_send_msg)
    {
        FLOG_ERROR("malloc p_send_msg error! \r\n");
        return TRANS_FAILD;   
    }  
    
    /*memset*/
    memset((u_int8_t*)p_send_msg, 0, uw_len);
    #endif

    switch (uc_send_type)
    {
        #ifdef TRANS_RRH_COMPILE
        /*Send Message to RRH  */
        case TRANS_SEND_TO_RRH:
    
            uw_ret = trans_rrh_send_agent(p_send_info, uw_len , p_send_msg);
            if (TRANS_SUCCESS != uw_ret)
            {
                FLOG_ERROR("Call trans_rrh_send_agent error! uw_ret = %d\r\n", uw_ret);
            }
            
            break;
        #else
        (void)p_send_info;
        (void)uw_len;
        (void)p_send_msg;
        #endif
            
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
    
    //free(p_send_msg);
    
    return uw_ret;
}

/*****************************************************************************+
* Function: trans_common_delete()
* Description: 
* Parameters:
*           p_mac : mac address
* Return Values:
*           TRANS_FAILD : 1
*           TRANS_SUCCESS  : 0
*
*  
*  Data:    2012-03-20
* 
+*****************************************************************************/
int trans_common_delete(void *p_info, 
                           size_t len,
                           void * p_msg)
{
    u_int32_t uw_ret = 0;
    u_int8_t  *p_mac = NULL;
    u_int32_t uw_index = 0;
    
    fun_callback f_callback = NULL;    
    
    (void)p_info;
    (void)len;   

    if (NULL == p_msg)
    {
        FLOG_ERROR("NULL PTR! p_msg. \r\n");
        return TRANS_FAILD;       
    }

    p_mac = (u_int8_t *)p_msg;

    FLOG_DEBUG_TRANS(g_trans_debug_com, "Enter, mac = 0x%x:0x%x:0x%x:0x%x:0x%x:0x%x.\n", 
            p_mac[0], 
            p_mac[1], 
            p_mac[2], 
            p_mac[3], 
            p_mac[4], 
            p_mac[5]);

    /*Delete transaction List and timer list*/
    FLOG_DEBUG_TRANS(g_trans_debug_com, "Delete transaction info\r\n");

    uw_ret = trans_transaction_clear(p_mac);
    if (TRANS_SUCCESS != uw_ret)
    {
        FLOG_ERROR("Call trans_transaction_clear error! uw_ret = %d\r\n", uw_ret);
        //return;
    } 
    
    /*In action handle, do not op action list*/

    /*Delele function callback*/
    for (uw_index = TRANS_REGISTER_FUN_MONITOR_OP; uw_index < TRANS_REGISTER_FUN_BUF; uw_index ++)
    {
        if (1 == g_trans_register_delete_func[uw_index].uc_use_flag)
        {
            f_callback = g_trans_register_delete_func[uw_index].f_callback;

            if (NULL == f_callback)
            {
                FLOG_ERROR("NULL PTR! f_callback\r\n");
            }
            else
            {
                (*(f_callback))(NULL, 0, p_msg);
            }
        }
    }

    /*Delete Device List*/
    FLOG_DEBUG_TRANS(g_trans_debug_com, "Delete device info! \r\n");
       
    uw_ret = trans_device_delete(p_mac);
    if (TRANS_SUCCESS != uw_ret)
    {
        FLOG_ERROR("Call trans_device_delete error! uw_ret = %d\r\n", uw_ret);
        //return;
    } 
    
    FLOG_DEBUG_TRANS(g_trans_debug_com, "Exit \r\n");    
    
    return TRANS_SUCCESS;
}

/*****************************************************************************+
* Function: trans_common_action_delete()
* Description: 
* Parameters:
*           p_mac : mac address
* Return Values:
*           TRANS_FAILD : 1
*           TRANS_SUCCESS  : 0
*
*  
*  Data:    2012-03-20
* 
+*****************************************************************************/
u_int32_t trans_common_action_delete(u_int8_t *p_mac)
{
    u_int32_t uw_ret = 0;
    
    struct trans_action_info st_action_info;
    
    FLOG_DEBUG_TRANS(g_trans_debug_com, "Enter \r\n");
    
    st_action_info.f_callback = trans_common_delete;
    st_action_info.p_info = NULL;
    st_action_info.uw_src_moudle = TRANS_MOUDLE_LOCAL;
    st_action_info.p_action_list = &g_trans_action_list;

    FLOG_DEBUG_TRANS(g_trans_debug_com, "mac = 0x%x:0x%x:0x%x:0x%x:0x%x:0x%x.\n", 
            p_mac[0], 
            p_mac[1], 
            p_mac[2], 
            p_mac[3], 
            p_mac[4], 
            p_mac[5]);
    
    uw_ret = trans_action_add(&st_action_info, TRANS_MAC_ADDR_LEN, p_mac);
    
    if (TRANS_SUCCESS != uw_ret)
    {
        FLOG_ERROR("Call trans_action_add error! uw_ret = %d\r\n", uw_ret);
    
        return TRANS_FAILD;    
    }  
    
    FLOG_DEBUG_TRANS(g_trans_debug_com, "Exit \r\n");      
    
    return TRANS_SUCCESS;    
    
}

/*****************************************************************************+
* Function: trans_common_msg_receive()
* Description: 
* Parameters:
*           NONE
* Return Values:
*           TRANS_FAILD : 1
*           TRANS_SUCCESS  : 0
*
*  
*  Data:    2011-11-02
* 
+*****************************************************************************/
void trans_common_msg_receive(void)
{
    int32_t w_ret = 0;
    u_int32_t uw_ret = 0;
    u_int32_t uw_index = 0;
    
    u_int32_t uw_num_elems = 0;

    int32_t w_max_sockfd = 0;
   
    fd_set readfds;    
    struct timeval st_time_val;
    
    u_int8_t  *p_rev_buf = NULL;
    int32_t  w_len = 0;    
    
    //struct trans_device_info *p_device_info = NULL;

    //struct trans_device_info *p_temp_info = NULL;
    int32_t w_temp_sockfd = 0;
    u_int8_t uc_temp_type = 0;

    //struct trans_common_sockfd_info  *p_sockfd_info = NULL;
    
    FLOG_DEBUG_TRANS(g_trans_debug_com, "Enter.\r\n"); 
        
    w_ret = pthread_setcanceltype (PTHREAD_CANCEL_ASYNCHRONOUS, NULL);
    if (0 != w_ret)
    {
        FLOG_ERROR("Call pthread_setcanceltype error! \r\n");    
        return ;
    }
    
    while (1)
    {           
        uw_ret = trans_monitor_rev_registration();

        if ((1 > uw_num_elems) && (2 == uw_ret))
        {
            sleep(1);
        }
        
        FD_ZERO(&readfds);   
        
        uw_num_elems = g_trans_common_sockfd.uw_num;
        w_max_sockfd = 0;
        
        FLOG_DEBUG_TRANS(g_trans_debug_com, "uw_num_elems:%d.\r\n", uw_num_elems); 
        //FLOG_ERROR("uw_num_elems:%d.\r\n", uw_num_elems); 

        for (uw_index = 0; uw_index < uw_num_elems; uw_index ++)
        {
            w_temp_sockfd = g_trans_common_sockfd.st_sockfd[uw_index].w_sockfd;
        
            if (0 < w_temp_sockfd)
            {
                FD_SET(w_temp_sockfd, &readfds);
                
                w_max_sockfd = TRANS_MAX(w_max_sockfd, w_temp_sockfd);
                
                FLOG_DEBUG_TRANS(g_trans_debug_com, "sockfd:%d.\r\n", w_temp_sockfd); 
            }        
        }
        
        #if 0
        gettimeofday(&st_time_val, NULL);        
        st_time_val.tv_sec += 1;
        #endif
        st_time_val.tv_sec = 1;
        
        w_ret = select(w_max_sockfd + 1, &readfds, NULL, NULL, &st_time_val);
        //w_ret = select(w_max_socket + 1, &readfds, NULL, NULL, NULL);
        if(w_ret < 0)
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

        for (uw_index = 0; uw_index < uw_num_elems; uw_index ++)
        {
            w_temp_sockfd = g_trans_common_sockfd.st_sockfd[uw_index].w_sockfd;
            uc_temp_type = g_trans_common_sockfd.st_sockfd[uw_index].uc_module_type;

            if (1 >= w_temp_sockfd)
            {
                continue;
            }

            if((FD_ISSET(w_temp_sockfd, &readfds)))
            {
                switch (uc_temp_type)
                {
                    #ifdef TRANS_RRH_COMPILE
                    case TRANS_MOUDLE_RRH:
                        uw_ret = trans_rrh_rev_msg(&p_rev_buf, w_temp_sockfd, &w_len);
                        /*Error*/
                        if (TRANS_SUCCESS != uw_ret)
                        {
                            /*???????If  error , Do what?????*/                    
                            FLOG_ERROR("Call trans_rrh_rev_msg error! uw_ret = %d\r\n", uw_ret);
                            FLOG_ERROR("Connection RRH failed\r\n");
                            
                            close(g_trans_rrh_socket);
                            g_trans_rrh_socket = -1;
                            
                            /*Delete heartbeat timer*/
                            trans_timer_delete(&g_trans_timer_list, g_trans_rrh_hb_timer);

                            //return ;
                        }  
                        else
                        {
                            /*Enquene*/
                        }
                        
                        break;

                    #endif

                    #ifdef TRANS_AGENT_COMPILE
                    case TRANS_MOUDLE_AGENT:
                        uw_ret = trans_agent_rev_msg(&p_rev_buf, w_temp_sockfd, &w_len);
                        /*Error*/
                        if (TRANS_SUCCESS != uw_ret)
                        {
                            /*Delete Socket Info from Device List*/        
                            FLOG_ERROR("Call trans_agent_rev_msg error! uw_ret = %d\r\n", uw_ret);
                            FLOG_WARNING("Lost the connection with Agent. \r\n");

                            g_trans_agent_socket.w_agent_socket = -1;   

                            /*Delete heartbeat timer*/
                            trans_timer_delete(&g_trans_timer_list, g_trans_agent_hb_timer);
                            //return ;
                        }  
                        else
                        {
                            /*Enquene*/
                        }                        
                        break;                    
                    #endif

                    #ifdef TRANS_MONITOR_COMPILE
                    case TRANS_MOUDLE_MONITOR:
                    case TRANS_MOUDLE_BS:
                    case TRANS_MOUDLE_MS:
                        uw_ret = trans_monitor_rev_msg(&p_rev_buf, w_temp_sockfd, &w_len);
                        /*Error*/
                        if (TRANS_SUCCESS != uw_ret)
                        {
                            /*???????If  error , Do what?????*/                    
                            FLOG_ERROR("Call trans_monitor_rev_msg error! uw_ret = %d\r\n", uw_ret);
                            FLOG_WARNING("Lost the connection with Monitor. \r\n");
        
                            //return ;
                        }   
                        else
                        {
                            /*Enqueue*/
                        }                        
                        break;                    
                    #endif

                    default:
                    
                        FLOG_ERROR("Rev Socket type error! type = %d\r\n", uc_temp_type);
                        break;
                }

                /*Enqueue*/
                if (TRANS_SUCCESS == uw_ret)
                {
                    struct trans_common_queue st_queue_info;

                    st_queue_info.p_msg = p_rev_buf;
                    st_queue_info.uc_module_type = uc_temp_type;
                    st_queue_info.uw_len = w_len;
                    st_queue_info.w_sockfd = w_temp_sockfd;
                    
                    memcpy(st_queue_info.a_mac, g_trans_common_sockfd.st_sockfd[uw_index].a_mac, TRANS_MAC_ADDR_LEN);
                    
                    uw_ret = trans_common_msg_en_queue(&st_queue_info);
                    if (TRANS_SUCCESS != uw_ret)
                    {
                        FLOG_ERROR("Call trans_common_msg_en_queue error! uw_ret = %d\r\n", uw_ret);
                        //return;
                    } 

                }
                /*Revice message failed, socket connection failed----delete device /transaction /timer info from list*/
                else if (TRANS_FAILD == uw_ret)
                {
    
                    //uw_ret = trans_common_action_delete(g_trans_common_sockfd.st_sockfd[uw_index].a_mac);
                    uw_ret = trans_common_delete(NULL, TRANS_MAC_ADDR_LEN,
                                        g_trans_common_sockfd.st_sockfd[uw_index].a_mac);
                    if (TRANS_SUCCESS != uw_ret)
                    {
                        FLOG_ERROR("Call trans_common_action_delete error! uw_ret = %d\r\n", uw_ret);
                        //return;
                    } 

                    #ifdef TRANS_AGENT_COMPILE
                    if (TRANS_MOUDLE_AGENT == uc_temp_type)
                    {
                        uw_ret = connect_agent();                        
                        if (0 != uw_ret)
                        {
                            FLOG_ERROR("Call connect_agent error! uw_ret = %d\r\n", uw_ret);                        
                            //return;
                        }
                    }   
                    #endif

                    free(p_rev_buf);
                }
                /*Discard this message*/
                else
                {
                    FLOG_ERROR("Receive error : discard the message.\r\n", uw_ret);
                    free(p_rev_buf);
                }
            }

            p_rev_buf = NULL;
        }
    }

    return;
}

#if 0

/*****************************************************************************+
* Function: trans_common_msg_receive()
* Description: 
* Parameters:
*           NONE
* Return Values:
*           TRANS_FAILD : 1
*           TRANS_SUCCESS  : 0
*
*  
*  Data:    2011-11-02
* 
+*****************************************************************************/
void trans_common_msg_receive(void)
{
    int32_t w_ret = 0;
    u_int32_t uw_ret = 0;
    u_int32_t uw_index = 0;
    
    u_int32_t uw_num_elems = 0;

    int32_t w_max_sockfd = 0;
   
    fd_set readfds;    
    struct timeval st_time_val;
    
    u_int8_t  *p_rev_buf = NULL;
    int32_t  w_len = 0;    
    
    struct trans_device_info *p_device_info = NULL;

    struct trans_device_info *p_temp_info = NULL;
    int32_t w_temp_sockfd = 0;
    u_int8_t uc_temp_type = 0;

    
    FLOG_DEBUG_TRANS(g_trans_debug_com, "Enter.\r\n"); 
        
    w_ret = pthread_setcanceltype (PTHREAD_CANCEL_ASYNCHRONOUS, NULL);
    if (0 != w_ret)
    {
        FLOG_ERROR("Call pthread_setcanceltype error! \r\n");    
        return ;
    }
    
    while (1)
    {           
        uw_ret = trans_device_find_all(&uw_num_elems, (void *)&p_device_info);
        /*Error or no connection*/
        if (TRANS_SUCCESS != uw_ret)
        {
            FLOG_ERROR("Call trans_device_find_all error! uw_num_elems = %d\r\n", uw_num_elems);
            //return ;

            //sleep(1);
        }  

        uw_ret = trans_monitor_rev_registration();

        if ((1 > uw_num_elems) && (2 == uw_ret))
        {
            sleep(1);
        }

        
        FD_ZERO(&readfds);   

        
        FLOG_DEBUG_TRANS(g_trans_debug_com, "uw_num_elems:%d.\r\n", uw_num_elems); 

        for (uw_index = 0; uw_index < uw_num_elems; uw_index ++)
        {
            p_temp_info = ((struct trans_device_info *)p_device_info) + uw_index;
            w_temp_sockfd = p_temp_info->w_sockfd;

            FD_SET(w_temp_sockfd, &readfds);
            
            w_max_sockfd = TRANS_MAX(w_max_sockfd, w_temp_sockfd);
            
            FLOG_DEBUG_TRANS(g_trans_debug_com, "sockfd:%d.\r\n", w_temp_sockfd); 

        }
/*
        gettimeofday(&st_time_val, NULL);
        
        st_time_val.tv_sec += 1;
*/
        st_time_val.tv_sec = 1;
      
        w_ret = select(w_max_sockfd + 1, &readfds, NULL, NULL, &st_time_val);
        //w_ret = select(w_max_socket + 1, &readfds, NULL, NULL, NULL);
        if(w_ret < 0)
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

        for (uw_index = 0; uw_index < uw_num_elems; uw_index ++)
        {
            p_temp_info = ((struct trans_device_info *)p_device_info) + uw_index;
            
            w_temp_sockfd = p_temp_info->w_sockfd;
            uc_temp_type = p_temp_info->uc_module_type;

            if((FD_ISSET(w_temp_sockfd, &readfds)))
            {
                switch (uc_temp_type)
                {
                    #ifdef TRANS_RRH_COMPILE
                    case TRANS_MOUDLE_RRH:
                        uw_ret = trans_rrh_rev_msg(&p_rev_buf, w_temp_sockfd, &w_len);
                        /*Error*/
                        if (TRANS_SUCCESS != uw_ret)
                        {
                            /*???????If  error , Do what?????*/                    
                            FLOG_ERROR("Call trans_rrh_rev_msg error! uw_ret = %d\r\n", uw_ret);
                            FLOG_ERROR("Connection RRH failed\r\n");
                            //return ;
                        }  
                        else
                        {
                            /*Enquene*/
                        }
                        
                        break;

                    #endif

                    #ifdef TRANS_AGENT_COMPILE
                    case TRANS_MOUDLE_AGENT:
                        uw_ret = trans_agent_rev_msg(&p_rev_buf, w_temp_sockfd, &w_len);
                        /*Error*/
                        if (TRANS_SUCCESS != uw_ret)
                        {
                            /*Delete Socket Info from Device List*/        
                            //#if 0
                            pthread_mutex_lock (&(g_trans_agent_socket.m_mutex));   
                            close(g_trans_agent_socket.w_agent_socket);
                            g_trans_agent_socket.w_agent_socket = -1;                     
                            pthread_mutex_unlock(&(g_trans_agent_socket.m_mutex)); 
                            //#endif
                        
                            FLOG_ERROR("Call trans_agent_rev_msg error! uw_ret = %d\r\n", uw_ret);
                        
                            FLOG_WARNING("Lost the connection with Agent. \r\n");
                            //return ;
                        }  
                        else
                        {
                            /*Enquene*/
                        }                        
                        break;                    
                    #endif

                    #ifdef TRANS_MONITOR_COMPILE
                    case TRANS_MOUDLE_MONITOR:
                    case TRANS_MOUDLE_BS:
                    case TRANS_MOUDLE_MS:
                        uw_ret = trans_monitor_rev_msg(&p_rev_buf, w_temp_sockfd, &w_len);
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
                            /*Enqueue*/
                        }                        
                        break;                    
                    #endif

                    default:
                    
                        FLOG_ERROR("Rev Socket type error! type = %d\r\n", uc_temp_type);
                        break;
                }

                /*Enqueue*/
                if (TRANS_SUCCESS == uw_ret)
                {
                    struct trans_common_queue st_queue_info;

                    st_queue_info.p_msg = p_rev_buf;
                    st_queue_info.uc_module_type = uc_temp_type;
                    st_queue_info.uw_len = w_len;
                    st_queue_info.w_sockfd = w_temp_sockfd;
                    
                    memcpy(st_queue_info.a_mac, p_temp_info->a_mac, TRANS_MAC_ADDR_LEN);
                    
                    uw_ret = trans_common_msg_en_queue(&st_queue_info);
                    if (TRANS_SUCCESS != uw_ret)
                    {
                        FLOG_ERROR("Call trans_common_msg_en_queue error! uw_ret = %d\r\n", uw_ret);

                        return;
                    } 

                }
                /*Revice message failed, socket connection failed----delete device info from list*/
                else
                {
                    /*Delete Device List*/
                    FLOG_DEBUG_TRANS(g_trans_debug_com, "Delete device info! sockfd = %d\r\n", w_temp_sockfd);
                    
                    uw_ret = trans_device_delete(p_temp_info->a_mac);
                    if (TRANS_SUCCESS != uw_ret)
                    {
                        FLOG_ERROR("Call trans_device_delete error! uw_ret = %d\r\n", uw_ret);

                        return;
                    } 
                }
            }
        }

        /**/
        if (NULL != p_device_info)
        {
            free(p_device_info);      

            p_device_info = NULL;
        }       
    }

    return;
}


/*****************************************************************************+
* Function: trans_common_msg_receive()
* Description: 
* Parameters:
*           NONE
* Return Values:
*           TRANS_FAILD : 1
*           TRANS_SUCCESS  : 0
*
*  
*  Data:    2011-11-02
* 
+*****************************************************************************/
void trans_common_msg_receive(void)
{
    struct trans_ordered_list *p_ordered_list = NULL;
    int32_t w_ret = 0;
    u_int32_t uw_ret = 0;
    u_int32_t uw_index = 0;
    
    u_int32_t uw_num_elems = 0;
    struct trans_device * p_device = NULL;
    
    struct trans_common_sockfd_info * p_sockfd = NULL;
    int32_t w_max_sockfd = 0;


    
    fd_set readfds;    
    struct timeval st_time_val;
    
    u_int8_t  *p_rev_buf = NULL;
    int32_t  w_len = 0;    
    
    w_ret = pthread_setcanceltype (PTHREAD_CANCEL_ASYNCHRONOUS, NULL);
    if (0 != w_ret)
    {
        FLOG_ERROR("Call pthread_setcanceltype error! \r\n");    
        return ;
    }
    
    p_ordered_list = (struct trans_ordered_list *)g_trans_device_list;

    if (NULL == p_ordered_list)
    {
        FLOG_ERROR("g_trans_device_list is not initial! \r\n");    
        return ;
    }

    while (1)
    {
        pthread_mutex_lock(&(p_ordered_list->qmutex));
        
        uw_num_elems = p_ordered_list->uw_node_num;
        
        if (0 > uw_num_elems)
        {
            FLOG_ERROR("List Number = %d error \r\n", uw_num_elems);
            
            return;   
        }
        
        if (0 == uw_num_elems)
        {
            FLOG_DEBUG("Device List is empty \r\n");
            
            continue;   
        }
        
        if (NULL == p_ordered_list->p_head)
        {
            FLOG_ERROR("1 NULL PTR! \r\n");
        
            return;   
        }   
        
        p_crru = p_ordered_list->p_head;    
        
        p_sockfd = (struct trans_common_sockfd_info *)malloc(SIZEOF_TRANS_COMMON_SOCKFD_INFO * uw_num_elems);
        if (NULL = p_sockfd)
        {
            FLOG_DEBUG("Malloc p_sockfd error.\r\n");
            
            return;   
        }
        
        FD_ZERO(&readfds);   

        for (uw_index = 0; uw_index < uw_num_elems; uw_index ++)
        {
            if (NULL == p_crru)
            {
                FLOG_ERROR("2 NULL PTR! \r\n");
            
                return;   
            } 

            if (NULL == p_crru->p_data)
            {
                FLOG_ERROR("3 NULL PTR! \r\n");
            
                return;   
            } 
          
            p_device = p_ordered_list->p_head->p_data;

            pthread_mutex_lock(&(p_device->dev_mutex));
            
            if (0 > p_device->w_sockfd)
            {
                FLOG_ERROR("w_sockfd = %d error! \r\n", p_device->w_sockfd);                
                return TRANS_FAILD;   
            }
            
            p_sockfd[uw_index]->uc_module_type = p_device->uc_module_type;
            p_sockfd[uw_index]->w_sockfd = p_device->w_sockfd;

            memcpy(p_sockfd[uw_index]->a_mac, p_device->a_mac, TRANS_MAC_ADDR_LEN);
            
            FD_SET(p_sockfd[uw_index]->w_sockfd, &readfds);

            w_max_sockfd = TRANS_MAX(w_max_sockfd, p_sockfd[uw_index]->w_sockfd);

            pthread_mutex_unlock(&(p_device->dev_mutex));         

            p_crru = p_crru->p_next;      
           
        }

        pthread_mutex_unlock(&(p_ordered_list->qmutex));   
/*
        gettimeofday(&st_time_val, NULL);
        
        st_time_val.tv_sec += 1;
*/
        st_time_val.tv_sec = 1;
      
        w_ret = select(w_max_sockfd + 1, &readfds, NULL, NULL, &st_time_val);
        //w_ret = select(w_max_socket + 1, &readfds, NULL, NULL, NULL);
        if(w_ret < 0)
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

        for (uw_index = 0; uw_index < uw_num_elems; uw_index ++)
        {
            if((FD_ISSET(p_sockfd[uw_index]->w_sockfd, &readfds)))
            {
                switch (p_sockfd[uw_index]->uc_module_type)
                {
                    #ifdef TRANS_RRH_COMPILE
                    case TRANS_DEVICE_RRH:
                        uw_ret = trans_rrh_rev_msg(&p_rev_buf, p_sockfd[uw_index]->w_sockfd, &w_len);
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
                            /*Enquene*/
                        }
                        
                        break;

                    #endif

                    #ifdef TRANS_AGENT_COMPILE
                    case TRANS_DEVICE_AGENT:
                        uw_ret = trans_agent_rev_msg(&p_rev_buf, p_sockfd[uw_index]->w_sockfd, &w_len);
                        /*Error*/
                        if (TRANS_SUCCESS != uw_ret)
                        {
                            /*Delete Socket Info from Device List*/        
                            #if 0
                            pthread_mutex_lock (&(g_trans_agent_socket.m_mutex));   
                            close(g_trans_agent_socket.w_agent_socket);
                            g_trans_agent_socket.w_agent_socket = -1;                     
                            pthread_mutex_unlock(&(g_trans_agent_socket.m_mutex)); 
                            #endif
                        
                            FLOG_ERROR("Call trans_agent_rev_msg error! uw_ret = %d\r\n", uw_ret);
                        
                            FLOG_WARNING("Lost the connection with Agent. \r\n");
                            //return ;
                        }  
                        else
                        {
                            /*Enquene*/
                        }                        
                        break;                    
                    #endif

                    #ifdef TRANS_MONITOR_COMPILE
                    case TRANS_DEVICE_MONITOR:
                    case TRANS_DEVICE_WMA:
                    case TRANS_DEVICE_WMB:
                        uw_ret = trans_monitor_rev_msg(&p_rev_buf, p_sockfd[uw_index]->w_sockfd, &w_len);
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
                            /*Enquene*/
                        }                        
                        break;                    
                    #endif

                    default:
                    
                        FLOG_ERROR("Rev Socket type error! type = %d\r\n", p_sockfd[uw_index]->uc_module_type);
                        break;
                }

                if (TRANS_SUCCESS == uw_ret)
                {
                    struct trans_common_queue st_quene_info;

                    st_quene_info.p_msg = p_rev_buf;
                    st_quene_info.uc_module_type = p_sockfd[uw_index]->uc_module_type;
                    st_quene_info.uw_len = w_len;
                    st_quene_info.w_sockfd = p_sockfd[uw_index]->w_sockfd;
                    
                    memcpy(st_quene_info.a_mac, p_sockfd[uw_index]->a_mac, TRANS_MAC_ADDR_LEN);
                    
                    uw_ret = trans_common_msg_en_queue(&st_quene_info);
                    if (TRANS_SUCCESS != uw_ret)
                    {
                        FLOG_ERROR("Call trans_common_msg_en_queue error! uw_ret = %d\r\n", uw_ret);

                        return;
                    } 

                }
                else
                {
                    /*Delete Device List*/
                }
                
            }
        }
    }

    return;
}
#endif

/*****************************************************************************+
* Function: trans_common_msg_parse()
* Description: 
* Parameters:
*           NONE
* Return Values:
*           TRANS_FAILD : 1
*           TRANS_SUCCESS  : 0
*
*  
*  Data:    2011-11-02
* 
+*****************************************************************************/
static u_int32_t trans_common_msg_parse(void* p_info)
{
    u_int32_t uw_ret = 0;
    struct trans_common_queue st_queue_info;

    u_int32_t  uw_len = 0;
    void * p_rev_msg = NULL;

    void ** p_info_tmp = NULL;
    
    FLOG_DEBUG_TRANS(g_trans_debug_com, "Enter \r\n");

    memset(&st_queue_info, 0, SIZEOF_TRANS_COMMON_QUEUE);

    /*Dequene*/
    uw_ret = trans_common_msg_de_queue(&st_queue_info);
    if (TRANS_SUCCESS != uw_ret)
    {
        /*???????If  error , Do what?????*/                    
        FLOG_ERROR("Call trans_monitor_rev_msg error! uw_ret = %d\r\n", uw_ret);
        return TRANS_FAILD;    
    }
    
    uw_len = st_queue_info.uw_len;
    p_rev_msg = st_queue_info.p_msg;

    
    trans_debug_msg_print(p_rev_msg, 40, g_trans_debug_com);  
    FLOG_DEBUG_TRANS(g_trans_debug_com, "Len = %d \r\n", uw_len); 
        
    switch (st_queue_info.uc_module_type)
    {
        #ifdef TRANS_RRH_COMPILE
        case TRANS_MOUDLE_RRH:
            uw_ret = trans_rrh_parse_msg(p_info, uw_len, p_rev_msg);
            /*Error*/
            if (TRANS_SUCCESS != uw_ret)
            {
                FLOG_ERROR("Call trans_rrh_rev_msg_parse error! uw_ret = %d\r\n", uw_ret);
                return TRANS_FAILD;                
            }  
            
            
            p_info_tmp = p_info;
            /*Fill in transaction*/
            uw_ret = trans_transaction_set_rrh(*p_info_tmp, NULL, uw_len, p_rev_msg);
            if (TRANS_SUCCESS != uw_ret)
            {
                FLOG_ERROR("Call trans_transaction_set_rrh error! uw_ret = %d\r\n", uw_ret);
                return TRANS_FAILD;
            }

            break;
    
        #endif
    
        #ifdef TRANS_AGENT_COMPILE
        case TRANS_MOUDLE_AGENT:
            uw_ret = trans_agent_parse_msg(p_info, st_queue_info.uw_len, st_queue_info.p_msg);
            /*Error*/
            if (TRANS_SUCCESS != uw_ret)
            {
                FLOG_ERROR("Call trans_agent_rev_msg_parse error! uw_ret = %d\r\n", uw_ret);
                return TRANS_FAILD;               
            }     
            
            p_info_tmp = p_info;
            /*Fill in transaction*/
            uw_ret = trans_transaction_set_agent(*p_info_tmp, NULL, uw_len, p_rev_msg);
            if (TRANS_SUCCESS != uw_ret)
            {
                FLOG_ERROR("Call trans_transaction_set_agent error! uw_ret = %d\r\n", uw_ret);
                return TRANS_FAILD;
            }

            break;  
            
        #endif
    
        #ifdef TRANS_MONITOR_COMPILE
        case TRANS_MOUDLE_MONITOR:
            
            /*Just using for these module*/
            g_trans_monitor_socket_now = st_queue_info.w_sockfd;
                
            uw_ret = trans_monitor_parse_msg(p_info, st_queue_info.uw_len, st_queue_info.p_msg);
            /*Error*/
            if (TRANS_SUCCESS != uw_ret)
            {
                FLOG_ERROR("Call trans_monitor_rev_msg_parse error! uw_ret = %d\r\n", uw_ret);
                return TRANS_FAILD;               
            }        

            p_info_tmp = p_info;
            /*Fill in transaction*/
            uw_ret = trans_transaction_set_monitor(*p_info_tmp, uw_len, p_rev_msg);
            if (TRANS_SUCCESS != uw_ret)
            {
                FLOG_ERROR("Call trans_transaction_set_monitor error! uw_ret = %d\r\n", uw_ret);
                return TRANS_FAILD;
            }
            
            break;   

        case TRANS_MOUDLE_MS:
            
            /*Just using for these module*/
            g_trans_monitor_socket_now = st_queue_info.w_sockfd;
                
            uw_ret = trans_monitor_parse_msg(p_info, st_queue_info.uw_len, st_queue_info.p_msg);
            /*Error*/
            if (TRANS_SUCCESS != uw_ret)
            {
                FLOG_ERROR("Call trans_monitor_rev_msg_parse error! uw_ret = %d\r\n", uw_ret);
                return TRANS_FAILD;               
            }        
            
            p_info_tmp = p_info;
            /*Fill in transaction*/
            uw_ret = trans_transaction_set_ms(*p_info_tmp, uw_len, p_rev_msg);
            if (TRANS_SUCCESS != uw_ret)
            {
                FLOG_ERROR("Call trans_transaction_set_ms error! uw_ret = %d\r\n", uw_ret);
                return TRANS_FAILD;
            }
            
            break;   
        case TRANS_MOUDLE_BS:
        case TRANS_MOUDLE_UI:

            /*Just using for these module*/
            g_trans_monitor_socket_now = st_queue_info.w_sockfd;
                
            uw_ret = trans_monitor_parse_msg(p_info, st_queue_info.uw_len, st_queue_info.p_msg);
            /*Error*/
            if (TRANS_SUCCESS != uw_ret)
            {
                FLOG_ERROR("Call trans_monitor_rev_msg_parse error! uw_ret = %d\r\n", uw_ret);
                return TRANS_FAILD;               
            }        

            p_info_tmp = p_info;
            /*Fill in transaction*/
            uw_ret = trans_transaction_set_bs(*p_info_tmp, uw_len, p_rev_msg);
            if (TRANS_SUCCESS != uw_ret)
            {
                FLOG_ERROR("Call trans_transaction_set_bs error! uw_ret = %d\r\n", uw_ret);
                return TRANS_FAILD;
            }

            break;     
            
        #endif
    
        default:
        
            FLOG_ERROR("Rev Socket type error! type = %d\r\n", st_queue_info.uc_module_type);
            return TRANS_FAILD;
            
    }

    uw_ret = trans_transaction_set_comn(*p_info_tmp, 
                            TRANS_TRANSACTION_FLAG_NO_DELETE,
                            st_queue_info.uc_module_type);
    if (TRANS_SUCCESS != uw_ret)
    {
        FLOG_ERROR("Call trans_transaction_set_comn error! uw_ret = %d\r\n", uw_ret);
        return TRANS_FAILD;
    }
    
    FLOG_DEBUG_TRANS(g_trans_debug_com, "Exit \r\n");

    return TRANS_SUCCESS;
}

/*****************************************************************************+
* Function: trans_common_msg_func()
* Description: 
* Parameters:
*           NONE
* Return Values:
*           TRANS_FAILD : 1
*           TRANS_SUCCESS  : 0
*
*  
*  Data:    2011-11-02
* 
+*****************************************************************************/
u_int32_t trans_common_msg_func(void* p_info)
{
    u_int32_t uw_ret = 0;
    
    fun_callback f_callback = NULL;
    void * p_ptr = NULL;
    struct trans_transaction_func  *p_func = NULL;

    struct trans_action_info st_action_info;

    FLOG_DEBUG_TRANS(g_trans_debug_com, "Enter \r\n");
 
    p_ptr = trans_transaction_get_func(p_info);

    if (NULL == p_ptr)
    {
        FLOG_ERROR("NULL PTR! p_ptr. \r\n");
        return TRANS_FAILD;       
    }
    
    p_func = (struct trans_transaction_func *)p_ptr;

    if (TRANS_REGISTER_FUN_BUF <= p_func->uw_func_id)
    {
        FLOG_ERROR("uw_func_id = %d error. \r\n", p_func->uw_func_id);
        return TRANS_FAILD;   
    }

    if (0 == p_func->uw_func_id)
    {
        return TRANS_SUCCESS;
    }

    f_callback = g_trans_register_exe_func[p_func->uw_func_id].f_callback;

    if (NULL == f_callback)
    {
        FLOG_ERROR("NULL PTR! f_callback. uw_func_id = %d.\r\n", p_func->uw_func_id);
        return TRANS_FAILD;   

    }

    if (TRANS_TRANSACTION_FLAG_EXE_NOW == p_func->uc_exe_flag)
    {
        (*(f_callback))(p_info, p_func->w_msg_len, p_func->p_msg);
        
    }
    else
    {
         /*Add action list*/
         if (NULL == p_func->p_msg)
        {
             FLOG_ERROR("NULL PTR! fp_func->p_msg.\r\n");
         }
         
         st_action_info.f_callback = f_callback;
         st_action_info.p_info = p_info;
         st_action_info.uw_src_moudle = trans_transaction_get_comn_src(p_info);
         st_action_info.p_action_list = &g_trans_action_list;
         
         uw_ret = trans_action_add(&st_action_info, p_func->w_msg_len, p_func->p_msg);
         
         if (TRANS_SUCCESS != uw_ret)
         {
             FLOG_ERROR("Call trans_action_add error! uw_ret = %d\r\n", uw_ret);
         
             return TRANS_FAILD;    
         }  
    }
    
    FLOG_DEBUG_TRANS(g_trans_debug_com, "Exit\r\n");
    
    return TRANS_SUCCESS;
}

/*****************************************************************************+
* Function: trans_common_msg_process()
* Description: 
* Parameters:
*           NONE
* Return Values:
*           TRANS_FAILD : 1
*           TRANS_SUCCESS  : 0
*
*  
*  Data:    2011-11-02
* 
+*****************************************************************************/
void trans_common_msg_process(void)
{
    int32_t w_ret = 0;
    u_int32_t uw_ret = 0;
    
    void* p_trans = NULL;    
    void * p_timer = NULL;
    
    w_ret = pthread_setcanceltype (PTHREAD_CANCEL_ASYNCHRONOUS, NULL);
    if (0 != w_ret)
    {
        FLOG_ERROR("Call pthread_setcanceltype error! \r\n");    
        return ;
    }
    
    while (1)
    {
        p_trans = NULL;
        
        /*Parsing message*/
        uw_ret = trans_common_msg_parse(&p_trans);
        if (TRANS_SUCCESS != uw_ret)
        {
            FLOG_ERROR("Call trans_common_msg_parse error! uw_ret = %d\r\n", uw_ret);
        
            //return;
        }
        else
        {
            /*Message Function Callback*/
            uw_ret = trans_common_msg_func(p_trans);
            if (TRANS_SUCCESS != uw_ret)
            {
                FLOG_ERROR("Call trans_common_msg_func error! uw_ret = %d\r\n", uw_ret);
            
                //return;     
            }
        }

        if (TRANS_SUCCESS != uw_ret)
        {
            uw_ret = trans_transaction_set_comn(p_trans, 
                                        TRANS_TRANSACTION_FLAG_DELETE,
                                        TRANS_MOUDLE_BUF);
            if (TRANS_SUCCESS != uw_ret)
            {
                FLOG_ERROR("Call trans_transaction_set_comn error! uw_ret = %d\r\n", uw_ret);
            
                continue;
            }

            /*Check if add timer*/
            p_timer = trans_transaction_get_timer(p_trans);
            
            if (NULL != p_timer)
            {
                uw_ret = trans_timer_delete(&g_trans_timer_list, p_timer);
                if(TRANS_SUCCESS != uw_ret) 
                {
                    FLOG_ERROR("Call trans_timer_delete error!uw_ret = %d \r\n", uw_ret);
                    
                    /*Can not return TRANS_FAILD*/
                    //return TRANS_FAILD;    
                }
            }
        }
        
        /*Delete p_trans*/
        uw_ret = trans_transaction_delete(p_trans);
        if (TRANS_SUCCESS != uw_ret)
        {
            FLOG_ERROR("Call trans_transaction_delete error! uw_ret = %d\r\n", uw_ret);
        
            return;
        }
    
    }

    return;
}


/*****************************************************************************+
* Function: trans_common_init()
* Description: init
* Parameters:
* Return Values:
*           TRANS_FAILD : 1
*           TRANS_SUCCESS  : 0
*
*  
*  Data:    2012-01-06
* 
+*****************************************************************************/
u_int32_t trans_common_init(void)
{
    u_int32_t uw_ret = 0;
    //u_int32_t uw_index = 0;
    
    /*struct trans_common_sockfd g_trans_common_sockfd;*/
    
    g_trans_common_sockfd.uw_num = 0;
   
    memset(g_trans_common_sockfd.st_sockfd, 0, 1024*SIZEOF_TRANS_COMMON_SOCKFD_INFO);

    if(pthread_mutex_init(&(g_trans_common_sockfd.s_mutex), NULL)) 
    {
        FLOG_ERROR("Initializing g_trans_common_sockfd.s_mutex mutex error! \r\n");

        return TRANS_FAILD;
    }
    
    /*Add device info ---Local<-->Local, socket : 0*/
    struct trans_device_info st_device_info;
    void * p_device = NULL;
    
    st_device_info.uc_module_type = g_trans_local_device_info.uc_module_type;
    st_device_info.w_sockfd = g_trans_local_device_info.w_sockfd;
    st_device_info.uc_states = TRANS_DEVICE_ACTIVE;
    
    memcpy(st_device_info.a_mac, g_trans_local_device_info.a_mac, TRANS_MAC_ADDR_LEN);
    
    uw_ret = trans_device_add(&st_device_info, &p_device);
    if(TRANS_SUCCESS != uw_ret) 
    {
        FLOG_ERROR("Call trans_device_add error!w_ret = %d. \r\n", uw_ret);
        return uw_ret;
    }
    
    return TRANS_SUCCESS;
}

/*****************************************************************************+
* Function: trans_common_release()
* Description: init
* Parameters:
* Return Values:
*           TRANS_FAILD : 1
*           TRANS_SUCCESS  : 0
*
*  
*  Data:    2012-01-06
* 
+*****************************************************************************/
u_int32_t trans_common_release(void)
{
//    u_int32_t uw_ret = 0;
    
    /*struct trans_common_sockfd g_trans_common_sockfd;*/
    
    g_trans_common_sockfd.uw_num = 0;
    
    memset(g_trans_common_sockfd.st_sockfd, 0, 1024*SIZEOF_TRANS_COMMON_SOCKFD_INFO);

    /*Delete Device List*/
/*
    uw_ret = trans_device_delete(g_trans_local_device_info.a_mac);
    if (TRANS_SUCCESS != uw_ret)
    {
        FLOG_ERROR("Call trans_device_delete error! uw_ret = %d\r\n", uw_ret);
    
        return TRANS_FAILD;
    } 
*/
    return TRANS_SUCCESS;
}



