/*****************************************************************************+
*
*  File Name: trans_wireless.c
*
*  Function: TRANS Wireless, BS or MS
*
*  
*  Data:    2011-04-14
*  Modify:
*
+*****************************************************************************/

#include <sys/types.h>
#include <pthread.h>
#include <sys/time.h>
#include <malloc.h>
#include <syslog.h>
#include <flog.h>

#include <trans.h>
#include <trans_rrh.h>
#include <trans_agent.h>
#include <trans_wireless.h>
#include <trans_timer.h>
#include <trans_list.h>
#include <trans_debug.h>

#ifdef TRANS_BS_COMPILE
#include "bs_cfg.h"
#endif

#ifdef TRANS_MS_COMPILE
#include "macphy_init_process.h"
#endif

/*****************************************************************************+
 *Global Variables 
+*****************************************************************************/
#if (defined TRANS_BS_COMPILE) || (defined TRANS_MS_COMPILE)


#endif

/*****************************************************************************+
 *Code 
+*****************************************************************************/
#ifdef TRANS_BS_COMPILE

/*****************************************************************************+
* Function: trans_wireless_action_bs_metric()
* Description: 
* Parameters:
*           NONE
* Return Values:
*           TRANS_FAILD : 1
*           TRANS_SUCCESS  : 0
*
*  
*  Data:    2011-07-14
* 
+*****************************************************************************/
int trans_wireless_action_bs_metric(void *p_user_info, 
                                size_t len, 
                                void *p_action_msg)
{
    u_int32_t uw_ret = 0;
    u_int32_t uw_len = 0;
    //char tmp[128];
    u_int8_t  *p_send_msg = NULL;

    u_int8_t uc_num = 0;
    u_int8_t  uc_index = 0;
    u_int8_t * p_metric_name = NULL;

    struct trans_agent_metric_info  *p_metric_info = NULL;
    struct trans_send_msg_to_agent st_agent_msg_info;

    p_user_info = p_user_info;
        
    /*p_user_info is NULL It is different with RRH*/
    if (NULL == p_action_msg)
    {
        FLOG_ERROR("NULL PTR! \r\n");
        return TRANS_FAILD;
    }

    if (0 == len)
    {
        FLOG_ERROR("Length error! \r\n");
        return TRANS_FAILD;
    }

    p_send_msg = (u_int8_t *)malloc(TRANS_WIRELESS_METRIC_MSG_MAX_LEN);
    if (NULL == p_send_msg)
    {
        FLOG_ERROR("malloc p_send_msg error! \r\n");
        return TRANS_FAILD;   
    }
    
    memset((u_int8_t*)p_send_msg, 0, TRANS_WIRELESS_METRIC_MSG_MAX_LEN);

    /*If the Metric value is string,-----Must modify the code, it just can process float and int32*/
    p_metric_info = (struct trans_agent_metric_info *)(p_action_msg);
   
    uc_num = p_metric_info->uc_metric_num;
    
    /*Get Metric*/
    for (uc_index = 0; uc_index < uc_num; uc_index++)
    {
        p_metric_name = p_metric_info->a_metrics[uc_index];
            
        /*4:  float or int32*/
        uw_ret = get_global_param ((char *)p_metric_name, (p_send_msg + uw_len));
       
        if (uw_ret != 0)
        {
            FLOG_ERROR ("get parameters %s error\n", p_metric_name);

            free(p_send_msg);
            return TRANS_FAILD;     
        }

        uw_len = (uc_index*4);
    }
        
    #if 0
    sprintf(tmp, "%04x", p_metric_info->a_metrics[0]);

    uw_ret = get_global_param (tmp, & ( st_metric_msg.w_metric_val ));

    if (uw_ret != 0)
    {
        FLOG_ERROR ("get parameters %s error\n", tmp);
    }
    #endif

    st_agent_msg_info.f_callback = NULL;
    st_agent_msg_info.uc_block_flag = TRANS_QUENE_NO_BLOCK;
    st_agent_msg_info.uc_ack_flag = TRANS_ACK_FLAG_OK;
    
    st_agent_msg_info.uw_resp_len = uw_len;
    st_agent_msg_info.p_reqs_msg = p_action_msg;
    st_agent_msg_info.p_resp_msg = p_send_msg;

    uw_ret = trans_send_wireless_msg(TRANS_SEND_TO_AGENT, &st_agent_msg_info);
    if (TRANS_SUCCESS != uw_ret) 
    {   
        FLOG_ERROR("Call trans_send_rrh_msg error! uw_ret = %d\r\n", uw_ret);
        //return TRANS_FAILD;     
    }  

    free(p_send_msg);
    

    #if 0
    u_int8_t  *p_send_msg = NULL;
    int32_t w_sockfd = 0;
    
    /* Allocate a memory.  */
    p_send_msg = (u_int8_t *)malloc(TRANS_WIRELESS_METRIC_MSG_MAX_LEN);
    if (NULL == p_send_msg)
    {
        FLOG_ERROR("1 malloc p_send_msg error! \r\n");
        return TRANS_FAILD;   
    }

    /*Send Metric*/
    w_sockfd = g_trans_moudle_socket_fd[uw_src_moudle];

    FLOG_DEBUG("uw_src_moudle = %d, w_sockfd =%d, uw_metric_id = %d. \r\n", 
                    uw_src_moudle, w_sockfd, st_metric_msg.p_agent_metric->w_metric_id);    
    
    pthread_mutex_lock(&(g_trans_agent_metric_mutex));  
    
    uw_ret = trans_agent_send_metric_msg(&st_metric_msg, p_send_msg, w_sockfd);
    if (TRANS_SUCCESS != uw_ret)
    {
        FLOG_ERROR("Call trans_agent_send_metric_msg error! uw_ret = %d\r\n", uw_ret);

    } 

    pthread_mutex_unlock(&(g_trans_agent_metric_mutex));

    free(p_send_msg);
  
    #endif

    return uw_ret;
}

#endif

#ifdef TRANS_MS_COMPILE

/*****************************************************************************+
* Function: trans_wireless_action_ms_metric()
* Description: 
* Parameters:
*           NONE
* Return Values:
*           TRANS_FAILD : 1
*           TRANS_SUCCESS  : 0
*
*  
*  Data:    2011-07-14
* 
+*****************************************************************************/
int trans_wireless_action_ms_metric(void *p_user_info, 
                                size_t len, 
                                void *p_action_msg)
{
    u_int32_t uw_ret = 0;
    u_int32_t uw_len = 0;
    //char tmp[128];
    u_int8_t  *p_send_msg = NULL;
    
    u_int8_t uc_num = 0;
    u_int8_t  uc_index = 0;
    u_int8_t * p_metric_name = NULL;
    
    struct trans_agent_metric_info  *p_metric_info = NULL;
    struct trans_send_msg_to_agent st_agent_msg_info;
    
    struct ss_perf_metrics st_result;
    int w_value = 0;    
    
    p_user_info = p_user_info;

    /*p_user_info is NULL It is different with RRH*/
    if (NULL == p_action_msg)
    {
        FLOG_ERROR("NULL PTR! \r\n");
        return TRANS_FAILD;
    }
    
    if (0 == len)
    {
        FLOG_ERROR("Length error! \r\n");
        return TRANS_FAILD;
    }
    
    p_send_msg = (u_int8_t *)malloc(TRANS_WIRELESS_METRIC_MSG_MAX_LEN);
    if (NULL == p_send_msg)
    {
        FLOG_ERROR("malloc p_send_msg error! \r\n");
        return TRANS_FAILD;   
    }

    memset((u_int8_t*)p_send_msg, 0, TRANS_WIRELESS_METRIC_MSG_MAX_LEN);
    
    /*If the Metric value is string,-----Must modify the code, it just can process float and int32*/
    p_metric_info = (struct trans_agent_metric_info *)(p_action_msg);
    
    uc_num = p_metric_info->uc_metric_num;

    uw_ret = init_report_reg(&st_result);
    if (TRANS_SUCCESS != uw_ret) 
    {   
        FLOG_ERROR("Call init_report_reg error! uw_ret = %d\r\n", uw_ret);
        free(p_send_msg);
        return TRANS_FAILD;     
    }  
    
    /*Get Metric*/
    for (uc_index = 0; uc_index < uc_num; uc_index++)
    {
        p_metric_name = p_metric_info->a_metrics[uc_index];
            
        /*4:  float or int32*/
        if (0 == strcmp ("RSSI", (char *)p_metric_name))
        {
            w_value = st_result.rssi;            
        }
        else if (0 == strcmp ("SINR", (char *)p_metric_name))
        {
            w_value = st_result.sinr;          
        }
        else if (0 == strcmp ("TxPower", (char *)p_metric_name))
        {
            w_value = st_result.tx_power;          
        }
        else if (0 == strcmp ("Temperature", (char *)p_metric_name))
        {
            w_value = st_result.temperature;          
        }
        else
        {
            FLOG_ERROR("Metric name error! metric_name = %s .\r\n", p_metric_name);
            
            free(p_send_msg);
            return TRANS_FAILD;   
        }
        
        /*4----sizeof(w_value)*/
        memcpy(p_send_msg + uw_len, &w_value, 4);
        
        uw_len = (uc_index*4);
    }
   
    st_agent_msg_info.f_callback = NULL;
    st_agent_msg_info.uc_block_flag = TRANS_QUENE_NO_BLOCK;
    st_agent_msg_info.uc_ack_flag = TRANS_ACK_FLAG_OK;
    
    st_agent_msg_info.uw_resp_len = uw_len;
    st_agent_msg_info.p_reqs_msg = p_action_msg;
    st_agent_msg_info.p_resp_msg = p_send_msg;
    
    uw_ret = trans_send_wireless_msg(TRANS_SEND_TO_AGENT, &st_agent_msg_info);
    if (TRANS_SUCCESS != uw_ret) 
    {   
        FLOG_ERROR("Call trans_send_rrh_msg error! uw_ret = %d\r\n", uw_ret);
        //return TRANS_FAILD;     
    }  
    
    free(p_send_msg);   

    
    #if 0
    st_metric_msg.p_agent_metric = (struct trans_agent_metric_info *)(p_msg);

    /*Check Metric ID*/
    if ((TRANS_AGENT_MS_METRIC_ID_RSSI > st_metric_msg.p_agent_metric->w_metric_id)
        || (TRANS_AGENT_MS_METRIC_ID_T < st_metric_msg.p_agent_metric->w_metric_id))
    {
        FLOG_ERROR("Metric Id error uw_metric_id = %d! \r\n", st_metric_msg.p_agent_metric->w_metric_id);
        return TRANS_FAILD;
    }
    
    /*Get Metric*/

    init_report_reg(&s_result);

    st_metric_msg.w_metric_val = s_result.ant0_rssi_reuse; //GET
   
    #ifdef TRANS_AGENT_COMPILE
    u_int8_t  *p_send_msg = NULL;
    int32_t w_sockfd = 0;
    
    /* Allocate a memory.  */
    p_send_msg = (u_int8_t *)malloc(TRANS_WIRELESS_METRIC_MSG_MAX_LEN);
    if (NULL == p_send_msg)
    {
        FLOG_ERROR("1 malloc p_send_msg error! \r\n");
        return TRANS_FAILD;   
    }

    /*Send Metric*/
    w_sockfd = g_trans_moudle_socket_fd[uw_src_moudle];

    FLOG_DEBUG("uw_src_moudle = %d, w_sockfd =%d, uw_metric_id = %d. \r\n", 
                    uw_src_moudle, w_sockfd, st_metric_msg.p_agent_metric->w_metric_id);    
    
    pthread_mutex_lock(&(g_trans_agent_metric_mutex));  
    
    uw_ret = trans_agent_send_metric_msg(&st_metric_msg, p_send_msg, w_sockfd);
    if (TRANS_SUCCESS != uw_ret)
    {
        FLOG_ERROR("Call trans_agent_send_metric_msg error! uw_ret = %d\r\n", uw_ret);

    } 

    pthread_mutex_unlock(&(g_trans_agent_metric_mutex));

    free(p_send_msg);
    
    #else
    
    uw_src_moudle = uw_src_moudle;    
    
    #endif
    #endif

    return uw_ret;
}

#endif

#if (defined TRANS_BS_COMPILE) || (defined TRANS_MS_COMPILE)
#if 0
/*****************************************************************************+
* Function: trans_wireless_action_exe()
* Description: 
* Parameters:
*           NONE
* Return Values:
*           TRANS_FAILD : 1
*           TRANS_SUCCESS  : 0
*
*  
*  Data:    2011-07-14
* 
+*****************************************************************************/
u_int32_t trans_wireless_action_exe(u_int8_t uc_action, u_int32_t uw_src_moudle, 
                                u_int32_t uw_len, u_int8_t *p_msg)
{
    u_int32_t uw_ret = 0;

    FLOG_DEBUG("Enter \r\n");    
     
    if (NULL == p_msg)
    {
        FLOG_ERROR("NULL PTR! \r\n");
        return TRANS_FAILD;
    } 

    switch (uc_action)
    {
        #ifdef TRANS_BS_COMPILE
        /* BS METRIC */
        case TRANS_WIRELESS_ACTION_GET_BS_METRIC: 
            
            uw_ret = trans_wireless_action_get_bs_metric(uw_src_moudle, uw_len, p_msg);
            if (TRANS_SUCCESS != uw_ret)
            {
                FLOG_ERROR("Call trans_bs_action_get_bs_metric error! uw_ret = %d\r\n", uw_ret);
            
                return TRANS_FAILD;    
            }   
            
            break; 

        #endif

        #ifdef TRANS_MS_COMPILE
        /* MS METRIC */
        case TRANS_WIRELESS_ACTION_GET_MS_METRIC: 
            
            uw_ret = trans_wireless_action_get_ms_metric(uw_src_moudle, uw_len, p_msg);
            if (TRANS_SUCCESS != uw_ret)
            {
                FLOG_ERROR("Call trans_bs_action_get_ms_metric error! uw_ret = %d\r\n", uw_ret);
            
                return TRANS_FAILD;    
            }   
            
            break; 

        #endif
    
        default:
    
            FLOG_ERROR("Action error uc_action = %d! \r\n", uc_action);    
            return TRANS_FAILD;
    
        break;
    
    } 

    FLOG_DEBUG("Exit \r\n");  
    
    return TRANS_SUCCESS;
}
#endif
/*****************************************************************************+
* Function: trans_wireless_init()
* Description: 
* Parameters:
*           NONE
* Return Values:
*           TRANS_FAILD : 1
*           TRANS_SUCCESS  : 0
*
*  
*  Data:    2011-07-14
* 
+*****************************************************************************/
u_int32_t trans_wireless_init(void)
{
   
    return TRANS_SUCCESS;
}

/*****************************************************************************+
* Function: trans_wireless_release()
* Description: 
* Parameters:
*           NONE
* Return Values:
*           TRANS_FAILD : 1
*           TRANS_SUCCESS  : 0
*
*  
*  Data:    2011-07-14
* 
+*****************************************************************************/
u_int32_t trans_wireless_release(void)
{

 
    return TRANS_SUCCESS;
}


#endif

