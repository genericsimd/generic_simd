/*****************************************************************************+
*
*  File Name: trans_agent.c
*
*  Function: Agent Message Process
*
*  
*  Data:    2011-04-23
*  Modify:
*
+*****************************************************************************/


#include <sys/types.h>
#include <syslog.h>
#include <flog.h>

#include <trans.h>
#include <trans_rrh.h>
#include <trans_agent.h>
#include <trans_wireless.h>
#include <trans_action.h>
#include <trans_timer.h>
#include <trans_list.h>
#include <trans_debug.h>
#ifdef TRANS_BS_COMPILE
#include <bs_cfg.h>
#endif
#ifdef TRANS_MS_COMPILE
#include <ms_cfg.h>
#endif

/*TCP*/
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <signal.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <pthread.h>
#include <errno.h>
#include <arpa/inet.h>

/*****************************************************************************+
 *Global Variables
 +*****************************************************************************/
pthread_mutex_t  g_trans_agent_metric_mutex;

struct trans_agent_socket  g_trans_agent_socket;


#ifdef TRANS_AGENT_COMPILE
struct trans_agent_config_info g_trans_agent_config_info;

extern u_int32_t trans_msg_en_quene(void *p_msg, struct trans_en_queue_msg *p_en_quene);
extern u_int32_t trans_msg_de_quene(u_int8_t *p_result);
#endif


#ifdef TRANS_AGENT_COMPILE

/*****************************************************************************+
 *Code 
+*****************************************************************************/

/*****************************************************************************+
* Function: trans_agent_get_time_stamp()
* Description: Get TimeStamp
* Parameters:
*           NONE
* Return Values:
*           NONE
*
*  
*  Data:    2011-05-11
* 
+*****************************************************************************/
u_int64_t trans_agent_get_time_stamp()
{
    struct timeval tp;
    struct timezone tzp;

    gettimeofday (&tp, &tzp);
//    return (((u_int64_t)tp.tv_sec) * 1000000000) + (((u_int64_t)tp.tv_usec) * 1000);
    return ((u_int64_t)tp.tv_sec);
}

/*****************************************************************************+
* Function: trans_agent_get_timestamp_string()
* Description: Get TimeStamp
* Parameters:
*           NONE
* Return Values:
*           TRANS_FAILD : 1
*           TRANS_SUCCESS  : 0
*
*  
*  Data:    2011-05-11
* 
+*****************************************************************************/
u_int32_t trans_agent_get_timestamp_string(u_int8_t *p_timestamp)
{
    #if (defined TRANS_BS_COMPILE) || (defined TRANS_RRH_COMPILE)
    sprintf((char *)p_timestamp, "%ld", trans_agent_get_time_stamp());
    #endif

    #ifdef TRANS_MS_COMPILE
    sprintf((char *)p_timestamp, "%lld", trans_agent_get_time_stamp());
    #endif
    
    return TRANS_SUCCESS;
}

/*****************************************************************************+
* Function: trans_agent_send_comnn_msg()
* Description: Send Connection message to Agent
* Parameters:
*           NONE
* Return Values:
*           TRANS_FAILD : 1
*           TRANS_SUCCESS  : 0
*
*  
*  Data:    2011-05-11
* 
+*****************************************************************************/
u_int32_t trans_agent_send_conn_msg(u_int8_t  *p_send_msg, int32_t w_agent_sockfd)
{
    int32_t w_ret = 0;
    struct trans_agent_msg *p_agent_msg = NULL;

    //u_int64_t  ul_timestamp = 0;
    u_int8_t  a_timestamp[20+1] = {0};
    u_int32_t uw_len = 0;
    char *p_msg = NULL;  
    
    if (NULL == p_send_msg)
    {
        FLOG_ERROR("NULL PTR! \r\n");
        return TRANS_FAILD;
    } 

    p_agent_msg = (struct trans_agent_msg *)p_send_msg;

    memset((u_int8_t*)p_agent_msg, 0, SIZEOF_TRANS_AGENT_MSG);
    
    json_t *msg = json_object();        // create an empty object;  

    json_t *source = json_string((char *)(g_trans_agent_config_info.a_device_id));// object for SOURCE;
    json_object_set_new(msg, "SOURCE", source);// set SOURCE;
    
    json_t *dest = json_string((char *)g_trans_agent_config_info.a_agent_id);// object for DEST
    json_object_set_new(msg, "DEST", dest);// set DEST
    
    json_t *body = json_object(); // create an empty object

    trans_agent_get_timestamp_string(a_timestamp);
    
    json_t *timestamp = json_string((char *)(a_timestamp));// object for timestamp
    json_object_set_new(body, "TIMESTAMP", timestamp);// set timestamp
    
    json_t *action = json_string("connect");// object for ACTION
    json_object_set_new(body, "ACTION", action);// set ACTION

    json_t *devid = json_string((char *)(g_trans_agent_config_info.a_device_id));// object for DEVID;
    json_object_set_new(body, "DEVID", devid);// set DEVID;
    
    json_object_set_new(msg, "BODY", body);  // set body elements in msg
    
    p_msg = json_dumps(msg, 0); //dump msg as a string
    
    //FLOG_DEBUG("json len : %ld \r\n", strlen((char *)p_msg));   
    //FLOG_DEBUG("json: %s \r\n", p_msg);
    
    strcpy((char *)p_agent_msg->a_magic, (char *)"CTRL");   /*Msg type：4Bytes*/

    uw_len = strlen((char *)p_msg);
    sprintf((char *)p_agent_msg->a_len, "%04d", uw_len);/*Msg Length：4Bytes*/
    
    strcpy((char *)(p_send_msg+SIZEOF_TRANS_AGENT_MSG), (char *)p_msg);
    
    FLOG_DEBUG("MSG: %s \r\n", p_send_msg);
    //FLOG_INFO("MSG len : %s \r\n", p_agent_msg->a_len);

    json_decref(msg);
    free(p_msg);

    w_ret = send(w_agent_sockfd, p_send_msg, uw_len+SIZEOF_TRANS_AGENT_MSG, 0);
    if(w_ret <= 0)
    {
        //close(sock);
        FLOG_ERROR("send error! w_ret = %d\r\n", w_ret);
        return TRANS_FAILD;
    }  

    #if 0
    //#ifdef TRANS_MS_COMPILE   
    u_int8_t uc_result = 0;
    
    FLOG_ERROR("Waite for message from agent\r\n");
    
    w_ret = trans_msg_de_quene(&uc_result);
    
    if ((TRANS_SUCCESS != w_ret) || (TRANS_SUCCESS != uc_result))
    {
        FLOG_ERROR("The response message from Agent error!result = %d, uw_ret. \r\n", uc_result, w_ret);
        return TRANS_FAILD; 
    }
    //#endif
    #endif
    
    return TRANS_SUCCESS;
}

/*****************************************************************************+
* Function: trans_agent_send_state_msg()
* Description: Send State Change message to Agent
* Parameters:
*           NONE
* Return Values:
*           TRANS_FAILD : 1
*           TRANS_SUCCESS  : 0
*
*  
*  Data:    2011-05-11
* 
+*****************************************************************************/
u_int32_t trans_agent_send_state_msg(u_int8_t  *p_send_msg, int32_t w_agent_sockfd)
{
    int32_t w_ret = 0;
    struct trans_agent_msg *p_agent_msg = NULL;

    //u_int64_t  ul_timestamp = 0;
    u_int8_t  a_timestamp[20+1] = {0};
    u_int32_t uw_len = 0;
    char *p_msg = NULL;  
    
    if (NULL == p_send_msg)
    {
        FLOG_ERROR("NULL PTR! \r\n");
        return TRANS_FAILD;
    } 
    
    p_agent_msg = (struct trans_agent_msg *)p_send_msg;

    memset((u_int8_t*)p_agent_msg, 0, SIZEOF_TRANS_AGENT_MSG);
    
    json_t *msg = json_object();        // create an empty object;  
    
    json_t *source = json_string((char *)(g_trans_agent_config_info.a_device_id));// object for SOURCE;
    json_object_set_new(msg, "SOURCE", source);// set SOURCE;
    
    json_t *dest = json_string((char *)g_trans_agent_config_info.a_agent_id);// object for DEST
    json_object_set_new(msg, "DEST", dest);// set DEST
    
    json_t *body = json_object(); // create an empty object

    trans_agent_get_timestamp_string(a_timestamp);
    
    json_t *timestamp = json_string((char *)(a_timestamp));// object for timestamp
    json_object_set_new(body, "TIMESTAMP", timestamp);// set timestamp
    
    json_t *action = json_string("stateChange");// object for ACTION
    json_object_set_new(body, "ACTION", action);// set ACTION
    
    //itoa(12, a_str, 10); 

    json_t *devid = json_string((char *)(g_trans_agent_config_info.a_device_id));// object for DEVID;
    json_object_set_new(body, "DEVID", devid);// set DEVID;

    json_t *state = json_string("1");// object for STATE;
    json_object_set_new(body, "STATE", state);// set STATE;
    
    json_object_set_new(msg, "BODY", body);  // set body elements in msg
    
    p_msg = json_dumps(msg, 0); //dump msg as a string
    
    //FLOG_DEBUG("json len : %ld \r\n", strlen((char *)p_msg));   
    //FLOG_DEBUG("json: %s \r\n", p_msg);
    
    strcpy((char *)p_agent_msg->a_magic, (char *)"MESS");   /*Msg type：4Bytes*/

    uw_len = strlen((char *)p_msg);
    sprintf((char *)p_agent_msg->a_len, "%04d", uw_len);/*Msg Length：4Bytes*/
    
    strcpy((char *)(p_send_msg+SIZEOF_TRANS_AGENT_MSG), (char *)p_msg);
    
    FLOG_DEBUG("MSG: %s \r\n", p_send_msg);
    //FLOG_INFO("MSG len : %s \r\n", p_agent_msg->a_len);

    json_decref(msg);
    free(p_msg);

    w_ret = send(w_agent_sockfd, p_send_msg, uw_len+SIZEOF_TRANS_AGENT_MSG, 0);
    if(w_ret <= 0)
    {
        //close(sock);
        FLOG_ERROR("send error! w_ret = %d\r\n", w_ret);
        return TRANS_FAILD;
    }  

    return TRANS_SUCCESS;
}


/*****************************************************************************+
* Function: trans_agent_send_hb_msg()
* Description: Send HeartBeat message to Agent
* Parameters:
*           NONE
* Return Values:
*           TRANS_FAILD : 1
*           TRANS_SUCCESS  : 0
*
*  
*  Data:    2011-06-30
* 
+*****************************************************************************/
u_int32_t trans_agent_send_hb_msg(u_int8_t  *p_send_msg, int32_t w_agent_sockfd)
{
    int32_t w_ret = 0;
    u_int32_t uw_ret = 0;
    struct trans_agent_msg *p_agent_msg = NULL;
    u_int8_t  a_timestamp[20+1] = {0};

    u_int32_t uw_len = 0;

    char *p_msg = NULL;  
    
    if (NULL == p_send_msg)
    {
        FLOG_ERROR("NULL PTR! \r\n");
        return TRANS_FAILD;
    } 

    p_agent_msg = (struct trans_agent_msg *)p_send_msg;

    memset((u_int8_t*)p_agent_msg, 0, SIZEOF_TRANS_AGENT_MSG);
    
    json_t *msg = json_object();        // create an empty object;  
    
    json_t *source = json_string((char *)(g_trans_agent_config_info.a_device_id));// object for DEVID;
    json_object_set_new(msg, "SOURCE", source);// set DEVID;

    //json_t *dest = json_string("51");// object for DEST
    json_t *dest = json_string(TRANS_AGENT_HEARTBEAT_MGMT);// object for DEST
    json_object_set_new(msg, "DEST", dest);// set DEST
    
    json_t *body = json_object(); // create an empty object
 
    json_t *devid = json_string((char *)(g_trans_agent_config_info.a_device_id));// object for DEVID;
    json_object_set_new(body, "module_id", devid);// set DEVID;

    json_t *action = json_string("heartbeat");// object for ACTION
    json_object_set_new(body, "action", action);// set ACTION

    trans_agent_get_timestamp_string(a_timestamp);
    
    json_t *timestamp = json_string((char *)(a_timestamp));// object for timestamp
    json_object_set_new(body, "timestamp", timestamp);// set timestamp
    
    json_object_set_new(msg, "BODY", body);  // set body elements in msg
    
    p_msg = json_dumps(msg, 0); //dump msg as a string
    
    //FLOG_DEBUG("json len : %ld \r\n", strlen((char *)p_msg));   
    //FLOG_DEBUG("json: %s \r\n", p_msg);
    
    strcpy((char *)p_agent_msg->a_magic, (char *)"MESS");   /*Msg type：4Bytes*/

    uw_len = strlen((char *)p_msg);
    sprintf((char *)p_agent_msg->a_len, "%04d", uw_len);/*Msg Length：4Bytes*/
    
    strcpy((char *)(p_send_msg+SIZEOF_TRANS_AGENT_MSG), (char *)p_msg);
   
    FLOG_DEBUG("MSG: %s \r\n", p_send_msg);
    
    json_decref(msg);
    free(p_msg);

    w_ret = send(w_agent_sockfd, p_send_msg, uw_len+SIZEOF_TRANS_AGENT_MSG, 0);
    if(w_ret <= 0)
    {
        FLOG_ERROR("send error! w_ret = %d\r\n", w_ret);
        return TRANS_FAILD;
    }  

    /*ADD TIMER*/
    struct timeval st_time_val;
    
    /*Send Heartbeat Msg Again  per 60 second*/
    gettimeofday(&st_time_val, NULL);  

    st_time_val.tv_sec = st_time_val.tv_sec + g_trans_agent_config_info.uw_agent_hb_time;   
    //st_time_val.tv_usec = st_time_val.tv_usec;

    void* p_timer_id = NULL;
    struct trans_timer_msg_info st_msg_info;
    
    st_msg_info.us_serial_number = 0;
    st_msg_info.uw_src_moudle = TRANS_MOUDLE_LOCAL;
    st_msg_info.uc_block_flag = TRANS_QUENE_NO_BLOCK;
    st_msg_info.f_callback = trans_agent_hb_timer_func;
    st_msg_info.p_user_info = NULL;
    
    uw_ret = trans_timer_add(&st_time_val, 
                            trans_agent_hb_timer_func, 
                            p_send_msg,
                            2048,
//                            uw_len + SIZEOF_TRANS_AGENT_MSG, 
                            &st_msg_info,
                            &p_timer_id);
    
    if (TRANS_SUCCESS != uw_ret) 
    {   
        FLOG_ERROR("Call trans_timer_add error! uw_ret = %d\r\n", uw_ret);
        return TRANS_FAILD;     
    }  
    
    return TRANS_SUCCESS;
}

/*****************************************************************************+
* Function: trans_agent_hb_timer_func()
* Description: Send HeartBeat message to Agent
* Parameters:
*           NONE
* Return Values:
*           TRANS_FAILD : 1
*           TRANS_SUCCESS  : 0
*
*  
*  Data:    2011-06-30
* 
+*****************************************************************************/
int trans_agent_hb_timer_func(void *p_msg, size_t len, void *p_msg_info)
{
    u_int32_t uw_ret = 0;
    p_msg_info = p_msg_info;
    len = len;
    
    uw_ret = trans_agent_send_hb_msg(p_msg, g_trans_agent_socket.w_agent_socket);
    if (TRANS_SUCCESS != uw_ret) 
    {   
        FLOG_ERROR("Call trans_agent_send_hb_msg error! uw_ret = %d\r\n", uw_ret);
        return TRANS_FAILD;     
    }  
    
    FLOG_INFO("Send Heartbeat to Agent.\r\n");

    return TRANS_SUCCESS;
}

/*****************************************************************************+
* Function: trans_agent_block_msg_timer_func()
* Description: 
* Parameters:
*           NONE
* Return Values:
*           TRANS_FAILD : 1
*           TRANS_SUCCESS  : 0
*
*  
*  Data:    2011-08-04
* 
+*****************************************************************************/
int trans_agent_block_msg_timer_func(void *p_msg, size_t len, void *p_msg_info)
{
    u_int32_t uw_ret = 0;    
    
    len = len;
    
    FLOG_ERROR("Time out : callback function\r\n"); 
    
    if ((NULL == p_msg) || (NULL == p_msg_info) )
    {
        FLOG_ERROR("NULL PTR! \r\n");
        return TRANS_FAILD;  
    }
    
    struct trans_en_queue_msg   st_en_quene;
    
    st_en_quene.uc_result = TRANS_FAILD;
    st_en_quene.uw_len = len;
    st_en_quene.uw_src_moudle = TRANS_MOUDLE_AGENT;
    
    uw_ret = trans_msg_en_quene(p_msg, &st_en_quene);
    
    if (TRANS_SUCCESS != uw_ret)
    {
        FLOG_ERROR("Call trans_msg_en_quene error! uw_ret = %d\r\n", uw_ret);
    
        return TRANS_FAILD;    
    }
    
    return TRANS_SUCCESS;    

}

/*****************************************************************************+
* Function: trans_agent_query_rrh_func()
* Description: 
* Parameters:
*           p_info : It could be NULL
*           len : Length
*           p_send_msg : struct trans_resp_msg_result
* Return Values:
*           TRANS_FAILD : 1
*           TRANS_SUCCESS  : 0
*
*  
*  Data:    2011-09-05
* 
+*****************************************************************************/
int trans_agent_query_rrh_func(void *p_info, 
                           size_t len,
                           void * p_send_msg)
{
    //u_int32_t uw_ret = 0;
    struct trans_resp_msg_header   *p_resp_result = NULL;

    FLOG_DEBUG("Enter \r\n");
    
    FLOG_ERROR("Agent Query RRH Timeout\r\n");
    
    //p_info = p_info;
    len = len;
    
    /*p_info could be NULL*/
    if (NULL == p_send_msg)
    {
        FLOG_ERROR("NULL PTR! \r\n");
        return TRANS_FAILD;
    }

    p_resp_result = (struct trans_resp_msg_header *)p_send_msg;

    switch (p_resp_result->uc_result)
    {
        /*sucessful*/ 
        case TRANS_ACK_FLAG_OK:
         /*Part error*/
        case TRANS_ACK_FLAG_P_ERR:
         /*Type error*/ 
        case TRANS_ACK_FLAG_TYPE_ERR:
        /*length error*/    
        case TRANS_ACK_FLAG_LEN_ERR:
        /*RRH CRC error*/ 
        case TRANS_ACK_FLAG_RRH_CRC_ERR:

            break;

        /*RRH timeout error*/ 
        case TRANS_ACK_FLAG_RRH_TIMEOUT:
        /*Other error*/    
        case TRANS_ACK_FLAG_OTHER_ERR:

            if (NULL != p_info)
            {
                FLOG_ERROR("Free user info %p\r\n", p_info);

                free(p_info);
            }

            break;
    
        /*Order no response back*/
        case TRANS_ACK_FLAG_ORDER_NO_RESP:
        /*Order need response back*/
        case TRANS_ACK_FLAG_ORDER_WITH_RESP:         
    
        /*Other*/
        default:
    
            FLOG_ERROR("Rev result type error! uc_result = %d\r\n", p_resp_result->uc_result);

            return TRANS_FAILD;
    
    }

    #if 0
    /*Send response message to agent*/
    uw_ret = trans_send_agent_msg(TRANS_SEND_TO_RRH_QUERY, &st_send_info);

    if (TRANS_SUCCESS != uw_ret)
    {
        FLOG_ERROR("Call trans_send_agent_msg error! uw_ret = %d\r\n", uw_ret);
    
        return TRANS_FAILD;
    }
    #endif
    
    FLOG_DEBUG("Exit \r\n");
    
    return TRANS_SUCCESS;
}


/*****************************************************************************+
* Function: trans_agent_cfg_rrh_func()
* Description: 
* Parameters:
*           p_info : It could be NULL
*           len : Length
*           p_send_msg : struct trans_resp_msg_result
* Return Values:
*           TRANS_FAILD : 1
*           TRANS_SUCCESS  : 0
*
*  
*  Data:    2011-09-05
* 
+*****************************************************************************/
int trans_agent_cfg_rrh_func(void *p_info, 
                           size_t len,
                           void * p_send_msg)
{
    //u_int32_t uw_ret = 0;
   struct trans_resp_msg_header   *p_resp_result = NULL;
   //struct trans_send_msg_to_monitor  st_send_info;

    FLOG_DEBUG("Enter \r\n");
    
    FLOG_INFO("Agent Config RRH Timeout\r\n");
    
    p_info = p_info;
    len = len;
    
    /*p_info could be NULL*/
    if (NULL == p_send_msg)
    {
        FLOG_ERROR("NULL PTR! \r\n");
        return TRANS_FAILD;
    }

    p_resp_result = (struct trans_resp_msg_header *)p_send_msg;

    switch (p_resp_result->uc_result)
    {
        /*sucessful*/ 
        case TRANS_ACK_FLAG_OK:
         /*Part error*/
        case TRANS_ACK_FLAG_P_ERR:
         /*Type error*/ 
        case TRANS_ACK_FLAG_TYPE_ERR:
        /*length error*/    
        case TRANS_ACK_FLAG_LEN_ERR:
        /*RRH CRC error*/ 
        case TRANS_ACK_FLAG_RRH_CRC_ERR:

            /*
            st_send_info.uc_ack_flag = p_resp_result->st_resp_com.uc_result;
            st_send_info.p_payload = ((u_int8_t *)p_resp_result) + SIZEOF_TRANS_RESP_MSG_HEADER;
            st_send_info.us_opration = 0;
            st_send_info.uw_payload_len = p_resp_result->st_resp_com.uw_len;
            */

            break;

        /*RRH timeout error*/ 
        case TRANS_ACK_FLAG_RRH_TIMEOUT:
        /*Other error*/    
        case TRANS_ACK_FLAG_OTHER_ERR:

            /*
            st_send_info.uc_ack_flag = p_resp_result->st_resp_com.uc_result;
            st_send_info.p_payload = NULL;
            st_send_info.us_opration = 0;
            st_send_info.uw_payload_len = 0;
            */

            break;
    
        /*Order no response back*/
        case TRANS_ACK_FLAG_ORDER_NO_RESP:
        /*Order need response back*/
        case TRANS_ACK_FLAG_ORDER_WITH_RESP:         
    
        /*Other*/
        default:
    
            FLOG_ERROR("Rev result type error! uc_result = %d\r\n", p_resp_result->uc_result);
            /*
            st_send_info.uc_ack_flag = TRANS_ACK_FLAG_OTHER_ERR;
            st_send_info.p_payload = NULL;
            st_send_info.us_opration = 0;
            st_send_info.uw_payload_len = 0;
            */

            return TRANS_FAILD;
    
    }

    #if 0
    /*Send response message to agent*/
    uw_ret = trans_send_monitor_msg(TRANS_SEND_TO_RRH_QUERY, &st_send_info);

    if (TRANS_SUCCESS != uw_ret)
    {
        FLOG_ERROR("Call trans_send_monitor_msg error! uw_ret = %d\r\n", uw_ret);
    
        return TRANS_FAILD;
    }
    #endif
    
    FLOG_DEBUG("Exit \r\n");
    
    return TRANS_SUCCESS;
}

#if 0

/*****************************************************************************+
* Function: trans_agent_send_alert_msg()
* Description: Send Alert message to Agent
* Parameters:
*           NONE
* Return Values:
*           TRANS_FAILD : 1
*           TRANS_SUCCESS  : 0
*
*  
*  Data:    2011-05-11
* 
+*****************************************************************************/
u_int32_t trans_agent_send_alert_msg(u_int16_t us_alarm_id, u_int8_t uc_alarm_value, 
                        u_int8_t  *p_send_msg, int32_t w_agent_sockfd)
{
    int32_t w_ret = 0;
    struct trans_agent_msg *p_agent_msg = NULL;

    //u_int64_t  ul_timestamp = 0;
    u_int8_t  a_timestamp[20+1] = {0};
    //u_int8_t  a_str[20+1] = {0};

    u_int32_t uw_len = 0;

    char *p_msg = NULL;  
    
    if (NULL == p_send_msg)
    {
        FLOG_ERROR("NULL PTR! \r\n");
        return TRANS_FAILD;
    } 

    p_agent_msg = (struct trans_agent_msg *)p_send_msg;

    memset((u_int8_t*)p_agent_msg, 0, SIZEOF_TRANS_AGENT_MSG);
    
    json_t *msg = json_object();        // create an empty object;  
    
    json_t *source = json_string((char *)(g_trans_agent_config_info.a_device_id));// object for SOURCE;
    json_object_set_new(msg, "SOURCE", source);// set SOURCE;
    
    json_t *dest = json_string((char *)g_trans_agent_config_info.a_agent_id);// object for DEST
    json_object_set_new(msg, "DEST", dest);// set DEST
    
    json_t *body = json_object(); // create an empty object

    trans_agent_get_timestamp_string(a_timestamp);
    
    json_t *timestamp = json_string((char *)(a_timestamp));// object for timestamp
    json_object_set_new(body, "TIMESTAMP", timestamp);// set timestamp
    
    json_t *action = json_string("alert");// object for ACTION
    json_object_set_new(body, "ACTION", action);// set ACTION

    json_t *devid = json_string((char *)(g_trans_agent_config_info.a_device_id));// object for DEVID;
    json_object_set_new(body, "DEVID", devid);// set DEVID;

    json_t *alertid = json_integer(us_alarm_id);// object for ALERTID;
    json_object_set_new(body, "ALERTID", alertid);// set ALERTID;

    json_t *alerval = json_integer(uc_alarm_value);// object for VALUE;
    json_object_set_new(body, "VALUE", alerval);// set VALUE;

    json_object_set_new(msg, "BODY", body);  // set body elements in msg
    
    p_msg = json_dumps(msg, 0); //dump msg as a string
    
    //FLOG_DEBUG("json len : %ld \r\n", strlen((char *)p_msg));   
    //FLOG_DEBUG("json: %s \r\n", p_msg);
           
    strcpy((char *)p_agent_msg->a_magic, (char *)"MESS");   /*Msg type：4Bytes*/

    uw_len = strlen((char *)p_msg);
    sprintf((char *)p_agent_msg->a_len, "%04d", uw_len);/*Msg Length：4Bytes*/
    
    strcpy((char *)(p_send_msg+SIZEOF_TRANS_AGENT_MSG), (char *)p_msg);
   
    json_decref(msg);
    free(p_msg);
    
    FLOG_DEBUG("MSG: %s \r\n", p_send_msg);
    //FLOG_INFO("MSG len : %s \r\n", p_agent_msg->a_len);

    w_ret = send(w_agent_sockfd, p_send_msg, uw_len+SIZEOF_TRANS_AGENT_MSG, 0);
    if(w_ret <= 0)
    {
        //close(sock);
        FLOG_ERROR("send error! w_ret = %d\r\n", w_ret);
        return TRANS_FAILD;
    }  
    
    return TRANS_SUCCESS;
}

/*****************************************************************************+
* Function: trans_agent_send_spectrum_msg()
* Description: Send Spectrum message to Agent
* Parameters:
*           NONE
* Return Values:
*           TRANS_FAILD : 1
*           TRANS_SUCCESS  : 0
*
*  
*  Data:    2011-07-27
* 
+*****************************************************************************/
u_int32_t trans_agent_send_spectrum_msg(struct trans_agent_spectrum_info *p_spectrum_info, 
                        u_int8_t  *p_send_msg, int32_t w_agent_sockfd)
{
    int32_t w_ret = 0;
    u_int32_t uw_ret = 0;
    struct trans_agent_msg *p_agent_msg = NULL;
    u_int8_t  a_str[20+1] = {0};

    u_int32_t uw_len = 0;
    u_int32_t uw_index = 0;

    char *p_msg = NULL; 
    double  d_sensing = 0.0;
    
    if (NULL == p_send_msg)
    {
        FLOG_ERROR("NULL PTR! \r\n");
        return TRANS_FAILD;
    } 

    p_agent_msg = (struct trans_agent_msg *)p_send_msg;

    memset((u_int8_t*)p_agent_msg, 0, SIZEOF_TRANS_AGENT_MSG);
    
    json_t *msg = json_object();        // create an empty object;  
    
    json_t *source = json_string((char *)(g_trans_agent_config_info.a_device_id));// object for SOURCE;
    json_object_set_new(msg, "SOURCE", source);// set SOURCE;
    
    sprintf((char *)a_str, "%d", 53);/*Msg Length：4Bytes*/
    json_t *dest = json_string((char *)a_str);// object for DEST
    json_object_set_new(msg, "DEST", dest);// set DEST
    
    json_t *body = json_object(); // create an empty object

    trans_agent_get_timestamp_string(a_str);
    
    json_t *timestamp = json_string((char *)(a_str));// object for timestamp
    json_object_set_new(body, "TIMESTAMP", timestamp);// set timestamp
    
    json_t *action = json_string("getSpectrumResult");// object for ACTION
    json_object_set_new(body, "ACTION", action);// set ACTION

    json_t *devid = json_string((char *)(g_trans_agent_config_info.a_device_id));// object for DEVID;
    json_object_set_new(body, "DEVID", devid);// set DEVID;

    json_t *gps = json_object(); // create an empty object

    json_t *latitude = json_real(p_spectrum_info->f_latitude);// object for ALTITUDE;
    json_object_set_new(gps, "LATITUDE", latitude);// set ALTITUDE;

    json_t *longitude = json_real(p_spectrum_info->f_longitude);// object for LONGITUDE;
    json_object_set_new(gps, "LONGITUDE", longitude);// set LONGITUDE;

    json_object_set_new(body, "GPS", gps);  // set body elements in msg

    json_t *array = json_array();

    for (uw_index = 0; uw_index< TRANS_AGENT_SENSING_NUM; uw_index++)
    {
        d_sensing = *(((float *)p_spectrum_info->p_sensing) + uw_index);
        json_t *value = json_real(d_sensing);
        json_array_append_new(array, value);
    }
  
    json_object_set_new(body, "SENSING", array);  // set body elements in msg  
    
    json_object_set_new(msg, "BODY", body);  // set body elements in msg
    
    p_msg = json_dumps(msg, 0); //dump msg as a string
    
    //FLOG_DEBUG("json len : %ld \r\n", strlen((char *)p_msg));   
    //FLOG_DEBUG("json: %s \r\n", p_msg);
           
    strcpy((char *)p_agent_msg->a_magic, (char *)"MESS");   /*Msg type：4Bytes*/

    uw_len = strlen((char *)p_msg);
    sprintf((char *)p_agent_msg->a_len, "%04d", uw_len);/*Msg Length：4Bytes*/
    
    strcpy((char *)(p_send_msg+SIZEOF_TRANS_AGENT_MSG), (char *)p_msg);
   
    json_decref(msg);
    free(p_msg);
    
    FLOG_DEBUG("MSG: %s \r\n", p_send_msg);
    //FLOG_INFO("MSG len : %s \r\n", p_agent_msg->a_len);

    w_ret = send(w_agent_sockfd, p_send_msg, uw_len+SIZEOF_TRANS_AGENT_MSG, 0);
    if(w_ret <= 0)
    {
        //close(sock);
        FLOG_ERROR("send error! w_ret = %d\r\n", w_ret);
        return TRANS_FAILD;
    }  

    /*ADD TIMER*/
    struct timeval st_time_val;
    gettimeofday(&st_time_val, NULL);  
    
    /*Timeout -- 10s*/
    st_time_val.tv_sec = st_time_val.tv_sec + 20;   
    //st_time_val.tv_usec = st_time_val.tv_usec;
    
    void* p_timer_id = NULL;
    struct trans_timer_msg_info st_msg_info;
    
    st_msg_info.us_serial_number = 0;
    st_msg_info.uw_src_moudle = TRANS_MOUDLE_AGENT;
    st_msg_info.uc_block_flag = TRANS_QUENE_BLOCK;
    st_msg_info.f_callback = trans_agent_block_msg_timer_func;
    st_msg_info.p_user_info = NULL;
    
    uw_ret = trans_timer_add(&st_time_val, 
                            trans_agent_block_msg_timer_func, 
                            p_send_msg, 
                            uw_len + SIZEOF_TRANS_AGENT_MSG, 
                            &st_msg_info,
                            &p_timer_id);
    
    if (TRANS_SUCCESS != uw_ret) 
    {   
        FLOG_ERROR("Call trans_timer_add error! uw_ret = %d\r\n", uw_ret);
        return TRANS_FAILD;     
    } 
    
    return TRANS_SUCCESS;
}

/*****************************************************************************+
* Function: trans_agent_forward_spectrum_msg()
* Description: 
* Parameters:
*           NONE
* Return Values:
*           TRANS_FAILD : 1
*           TRANS_SUCCESS  : 0
*
*  
*  Data:    2011-08-04
* 
+*****************************************************************************/
u_int32_t trans_agent_forward_spectrum_msg(u_int8_t  *p_send_msg, void *p_value)
{
    u_int32_t uw_ret = 0;
    struct trans_agent_spectrum_info st_spectrum_info;
    int32_t w_agent_sockfd = 0;

    w_agent_sockfd = g_trans_agent_socket.w_agent_socket;

    st_spectrum_info.f_latitude = 39.9;    //get
    st_spectrum_info.f_longitude = 116.4;   //get

//    st_spectrum_info.f_latitude = 0;    //get
//    st_spectrum_info.f_longitude = 0;   //get

    st_spectrum_info.p_sensing = p_value;

    uw_ret = trans_agent_send_spectrum_msg(&st_spectrum_info, p_send_msg, w_agent_sockfd);
    /*Error*/
    if (TRANS_SUCCESS != uw_ret)
    {
        FLOG_ERROR("Call trans_agent_send_spectrum_msg error! uw_ret = %d\r\n", uw_ret);        
        return TRANS_FAILD;

    }
    
    u_int8_t uc_result = 0;
    uw_ret = trans_msg_de_quene(&uc_result);
    
    if ((TRANS_SUCCESS != uw_ret) || (TRANS_SUCCESS != uc_result))
    {
        FLOG_ERROR("The response message from Agent error!result = %d, uw_ret. \r\n", uc_result, uw_ret);
        return TRANS_FAILD; 
    }
    
    return TRANS_SUCCESS;
}


/*****************************************************************************+
* Function: trans_agent_json_metric_process()
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
u_int32_t trans_agent_json_metric_process(u_int8_t  *p_rev_msg, u_int8_t  *p_send_msg, int32_t w_agent_sockfd)

{
    int32_t w_ret = 0;
   
    struct trans_agent_msg *p_agent_rev_msg = NULL;
    struct trans_agent_msg *p_agent_send_msg = NULL;
    //u_int8_t a_action[20] = {0};

    const char * p_action = NULL;
    json_error_t error;

    u_int32_t  uw_json_len = 0;

    //u_int8_t  a_str[20+1] = {0};
    const char *p_str = NULL;
    u_int8_t  a_timestamp[20+1] = {0};

    char *p_msg = NULL;  
    u_int32_t uw_len = 0;

    if ((NULL == p_rev_msg) ||(NULL == p_send_msg))
    {
        FLOG_ERROR("NULL PTR! \r\n");
        return TRANS_FAILD;
    }     
    
    FLOG_INFO("Enter \r\n");  
    
    p_agent_rev_msg = (struct trans_agent_msg *)p_rev_msg;

    FLOG_DEBUG("json: %s \r\n", p_rev_msg);

    uw_json_len = atoi((char *)(p_agent_rev_msg->a_len)); 
    FLOG_DEBUG("json len : %d \r\n", uw_json_len);

    
    json_t *msg = json_loads((char *)(p_rev_msg+SIZEOF_TRANS_AGENT_MSG), &error); //decodes the json string and returns the object it contains

    json_t *dest = json_object_get(msg, "DEST"); // get DEST

    p_str = json_string_value(dest);

    int w_dest = atoi((char *)(p_str)); 
    FLOG_DEBUG("%s, %d \r\n", p_str, w_dest);

    json_t *body = json_object_get(msg, "BODY"); //get BODY elements

    //json_t *timestamp = json_object_get(body, "TIMESTAMP"); // get timestamp

    json_t *action = json_object_get(body, "action"); // get ACTION

    p_action = json_string_value(action);

    //json_t *devid = json_object_get(body, "DEVID"); // get DEVID

    //int w_devid= json_string_value(devid);
    
    FLOG_DEBUG("action: %s \r\n", p_action);
    
    FLOG_INFO("********send******** \r\n");

    p_agent_send_msg = (struct trans_agent_msg *)p_send_msg;

    memcpy(p_agent_send_msg->a_magic, p_agent_rev_msg->a_magic, 4);

    json_t *send_msg = json_object();        // create an empty object;  

    json_object_set_new(send_msg, "DEST", json_object_get(msg, "DEST"));// set DEST

    json_t *send_body = json_object(); // create an empty object

    trans_agent_get_timestamp_string(a_timestamp);
    
    json_t *timestamp = json_string((char *)(a_timestamp));// object for timestamp
    json_object_set_new(send_body, "timestamp", timestamp);// set timestamp    

    json_t *send_action = json_string("sendMetric");// object for ACTION
    json_object_set_new(send_body, "action", send_action);// set ACTION

    json_t *devid = json_string((char *)(g_trans_agent_config_info.a_device_id));// object for DEVID;
    json_object_set_new(send_body, "device_id", devid);// set DEVID;

    json_object_set_new(send_body, "metric_id", json_object_get(body, "metric_id"));// set METRICID;

    json_t *metricval = json_real(100);// object for VALUE;
    json_object_set_new(send_body, "value", metricval);// set VALUE;

    json_object_set_new(send_msg, "BODY", send_body);  // set body elements in msg

    p_msg = json_dumps(send_msg, 0); //dump msg as a string  

    FLOG_DEBUG("sendMetric: json len : %ld \r\n", strlen((char *)p_msg));   
    FLOG_DEBUG("sendMetric :json: %s \r\n", p_msg);

    uw_len = strlen((char *)p_msg);
    sprintf((char *)p_agent_send_msg->a_len, "%04d", uw_len);/*Msg Length：4Bytes*/
    
    strcpy((char *)(p_send_msg+SIZEOF_TRANS_AGENT_MSG), (char *)p_msg);
    
    FLOG_DEBUG("sendMetric MSG: %s \r\n", p_send_msg);
    //FLOG_INFO("sendMetric MSG len : %s \r\n", p_agent_send_msg->a_len);

    json_decref(send_msg);
    free(p_msg);

    w_ret = send(w_agent_sockfd, p_send_msg, uw_len+SIZEOF_TRANS_AGENT_MSG, 0);
    if(w_ret <= 0)
    {
        //close(sock);
        FLOG_ERROR("send error! w_ret = %d\r\n", w_ret);
        return TRANS_FAILD;
    }  

    FLOG_INFO("Exit \r\n");

    return TRANS_SUCCESS;
}

//#endif


/*****************************************************************************+
* Function: trans_agent_build_metric_msg()
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
u_int32_t trans_agent_build_metric_msg(
                            struct trans_send_msg_to_agent *p_msg_info,
                            u_int32_t * p_len,
                            u_int8_t * p_send_msg)
{
    struct trans_agent_metric_info *p_agent_metric = NULL;

    struct trans_agent_msg *p_agent_msg = NULL;
    u_int8_t  a_srt[20+1] = {0};
    char *p_msg = NULL;  
    u_int32_t uw_len = 0;

    u_int8_t uc_index = 0;
    u_int32_t uw_num = 0;
    int32_t w_value = 0;
    u_int32_t  uw_value = 0;
    float     f_value = 0.0;
    u_int8_t * p_temp = NULL;

    json_t *value1 = NULL;
    json_t *value2 = NULL;

    FLOG_DEBUG("Enter! \r\n");

    if ((NULL == p_msg_info) || (NULL == p_send_msg))
    {
        FLOG_ERROR("NULL PTR! \r\n");
        return TRANS_FAILD;
    } 

    p_agent_metric = (struct trans_agent_metric_info *)p_msg_info->p_reqs_msg;

    if (NULL == p_agent_metric)
    {
        FLOG_ERROR("NULL PTR p_agent_metric! \r\n");
        return TRANS_FAILD;
    }

    uw_num = p_agent_metric->uc_metric_num;

    p_agent_msg = (struct trans_agent_msg *)p_send_msg;
    
    memset((u_int8_t*)p_agent_msg, 0, SIZEOF_TRANS_AGENT_MSG);
    
    json_t *msg = json_object();        // create an empty object;  
    
    json_t *source = json_string((char *)(g_trans_agent_config_info.a_device_id));// object for SOURCE;
    json_object_set_new(msg, "SOURCE", source);// set SOURCE;
    
    sprintf((char *)a_srt, "%d", p_agent_metric->w_source_id);
    json_t *dest = json_string((char *)a_srt);// object for DEST
    json_object_set_new(msg, "DEST", dest);// set DEST
    
    json_t *body = json_object(); // create an empty object
    
    json_t *id = json_integer(p_agent_metric->w_msg_id);// object for id;
    json_object_set_new(body, "id", id);// set id;
    
    json_t *ref = json_integer(p_agent_metric->w_msg_id);// object for ref;
    json_object_set_new(body, "ref", ref);// set ref;
    
    trans_agent_get_timestamp_string(a_srt);
    
    json_t *timestamp = json_string((char *)(a_srt));// object for timestamp
    json_object_set_new(body, "timestamp", timestamp);// set timestamp    
    
    json_t *sendaction = json_string("getMetricValues_res");// object for action
    json_object_set_new(body, "action", sendaction);// set action
    
    json_t *moduleid = json_string((char *)(g_trans_agent_config_info.a_device_id));// object for module_id;
    json_object_set_new(body, "module_id", moduleid);// set module_id;
    
    json_t *content = json_object(); // create an empty object
    
    json_t *elementid = json_string((char *)(p_agent_metric->a_element_id));// object for element_id;
    json_object_set_new(content, "element_id", elementid);// set element_id;
    
    json_t *array1 = json_array();
    json_t *array2 = json_array();
    

    if (0 == strcmp ("rrh", (char *)p_agent_metric->a_element_id))
    {
        for (uc_index = 0; uc_index < uw_num; uc_index++)
        {
            /*Notice: just can change value of int8_t , if want to change int32_t, modify the code*/
            p_temp = ((u_int8_t *)(p_msg_info->p_resp_msg)) + ((uc_index + 1) * 4);
                
            switch (p_agent_metric->a_metric_id[uc_index])
            {
                /*Power Amplifier Temperature*/ 
                case TRANS_AGENT_RRH_METRIC_ID_PAT:                     
                /*Downlink Output Power for channel 1#*/
                case TRANS_AGENT_RRH_METRIC_ID_CH1_PWR:
                /*Downlink Output Power for channel 2#*/
                case TRANS_AGENT_RRH_METRIC_ID_CH2_PWR:
                    w_value = *((int8_t *)(p_temp - 1));

                    value1 = json_string((char *)(p_agent_metric->a_metrics[uc_index]));// object for element_id;
                    json_array_append_new(array1, value1);
                    
                    value2 = json_real(w_value);// object for element_id;
                    json_array_append_new(array2, value2);

           
                    break;
            
                /*Downlink Voltage Standing Wave Radio (VSWR) for channel 1#*/    
                case TRANS_AGENT_RRH_METRIC_ID_CH1_VSWR: 
                /*Downlink Voltage Standing Wave Radio (VSWR) for channel 2#*/    
                case TRANS_AGENT_RRH_METRIC_ID_CH2_VSWR: 
            
                    w_value = *((u_int8_t *)(p_temp - 1));      
                    
                    value1 = json_string((char *)(p_agent_metric->a_metrics[uc_index]));// object for element_id;
                    json_array_append_new(array1, value1);
                    
                    value2 = json_real(w_value);// object for element_id;
                    json_array_append_new(array2, value2);

                    break;    
                                   
                default:
                    
                    FLOG_ERROR("Unknow RRH message! metric_id = 0x%x\r\n", p_agent_metric->a_metric_id);
                    return TRANS_FAILD;
            
            }  
        
        }

//        if ( (0 == TRANS_SEND_METRIC_TO_RRH_FAKE) )
//        {
            /*Its memory was malloced in function ---trans_agent_rev_query_rrh()*/
//            if (p_agent_metric != NULL)
//            {
        free(p_agent_metric);
//            }
//        }
//        else
//        {

            /*********Test**********/
            //free(p_agent_metric);
            /*********Test**********/
//        }        
    }
    else if (0 == strcmp ("bs", (char *)p_agent_metric->a_element_id))
    {
        //memcpy(st_agent_metric.a_metric_val, p_msg_info->p_resp_msg, p_msg_info->uw_resp_len);
        for (uc_index = 0; uc_index < uw_num; uc_index++)
        {
            /*Notice: just can change value of float or int32_t , if want to change int8_t or int16_t, modify the code*/
            p_temp = ((u_int8_t *)(p_msg_info->p_resp_msg)) + (uc_index * 4);
                
            switch (p_agent_metric->a_metric_id[uc_index])
            {
                /*Per WMB DL throughput(avg)*/ 
                case TRANS_AGENT_BS_METRIC_ID_WMB_DL:                     
                /*Per WMB UL throughput(avg)*/
                case TRANS_AGENT_BS_METRIC_ID_WMB_UL:
                /*Per conn. DL throughput(avg)*/
                case TRANS_AGENT_BS_METRIC_ID_CONN_DL:
                /*Per conn. UL throughput(avg)*/    
                case TRANS_AGENT_BS_METRIC_ID_CONN_UL:
                /*UL management throughput(avg)*/    
                case TRANS_AGENT_BS_METRIC_ID_UL_MANG:
                /*DL management throughput(avg)*/    
                case TRANS_AGENT_BS_METRIC_ID_DL_MANG:

                    f_value = *((float *)(p_temp));
        
                    value1 = json_string((char *)(p_agent_metric->a_metrics[uc_index]));// object for element_id;
                    json_array_append_new(array1, value1);
                    
                    value2 = json_real(f_value);// object for element_id;
                    json_array_append_new(array2, value2);        
           
                    break;
            
                /*Ranging Success conut*/    
                case TRANS_AGENT_BS_METRIC_ID_RANG: 
                /*connection conut*/    
                case TRANS_AGENT_BS_METRIC_ID_CONN: 
            
                    uw_value = *((u_int32_t *)(p_temp));      
                    
                    value1 = json_string((char *)(p_agent_metric->a_metrics[uc_index]));// object for element_id;
                    json_array_append_new(array1, value1);
                    
                    value2 = json_real(uw_value);// object for element_id;
                    json_array_append_new(array2, value2);
        
                    break;    
                                   
                default:
                    
                    FLOG_ERROR("Unknow BS message! metric_id = 0x%x\r\n", p_agent_metric->a_metric_id);
                    return TRANS_FAILD;
            
            }          
        }

        /*p_agent_metric will be free when Action is end*/
        
    }
    else if (0 == strcmp ("cpe", (char *)p_agent_metric->a_element_id))
    {
        //memcpy(st_agent_metric.a_metric_val, p_msg_info->p_resp_msg, p_msg_info->uw_resp_len);
        //memcpy(st_agent_metric.a_metric_val, p_msg_info->p_resp_msg, p_msg_info->uw_resp_len);
        for (uc_index = 0; uc_index < uw_num; uc_index++)
        {
            /*Notice: just can change value of float or int32_t , if want to change int8_t or int16_t, modify the code*/
            p_temp = ((u_int8_t *)(p_msg_info->p_resp_msg)) + (uc_index * 4);
                
            switch (p_agent_metric->a_metric_id[uc_index])
            {
                /*RSSI*/ 
                case TRANS_AGENT_MS_METRIC_ID_RSSI:                     
                /*SINR*/
                case TRANS_AGENT_MS_METRIC_ID_SINR:
                /*Tx Power*/
                case TRANS_AGENT_MS_METRIC_ID_TX_PWR:
                /*Temperature*/  
                case TRANS_AGENT_MS_METRIC_ID_T:

                    w_value = *((int32_t *)(p_temp));
        
                    value1 = json_string((char *)(p_agent_metric->a_metrics[uc_index]));// object for element_id;
                    json_array_append_new(array1, value1);
                    
                    value2 = json_real(w_value);// object for element_id;
                    json_array_append_new(array2, value2);        
           
                    break;
                                  
                default:
                    
                    FLOG_ERROR("Unknow MS message! metric_id = 0x%x\r\n", p_agent_metric->a_metric_id);
                    return TRANS_FAILD;
            
            }          
        }

        /*p_agent_metric will be free when Action is end*/
        
    }
    else
    {
        FLOG_ERROR("Source module error! src_moudle = %s\r\n", p_agent_metric->a_element_id);
    }
   
    json_object_set_new(content, "metrics", array1);  // set body elements in msg  
    json_object_set_new(content, "values", array2);  // set body elements in msg  
    
    json_object_set_new(body, "content", content);  // set body elements in msg
    json_object_set_new(msg, "BODY", body);  // set body elements in msg
    
    p_msg = json_dumps(msg, 0); //dump msg as a string  
    
    //FLOG_DEBUG("sendMetric: json len : %ld \r\n", strlen((char *)p_msg));   
    //FLOG_DEBUG("sendMetric :json: %s \r\n", p_msg);
    
    strcpy((char *)p_agent_msg->a_magic, (char *)"MESS");   /*Msg type：4Bytes*/
    
    uw_len = strlen((char *)p_msg);
    
    sprintf((char *)p_agent_msg->a_len, "%04d", uw_len);/*Msg Length：4Bytes*/
    
    strcpy((char *)(p_send_msg + SIZEOF_TRANS_AGENT_MSG), (char *)p_msg);
    
    json_decref(msg);
    free(p_msg);
    
    FLOG_DEBUG("sendMetric MSG: %s \r\n", p_send_msg);
    //FLOG_INFO("sendMetric MSG len : %s \r\n", p_agent_send_msg->a_len);

    *p_len = uw_len + SIZEOF_TRANS_AGENT_MSG;

    FLOG_DEBUG("Exit! \r\n");
    
    return TRANS_SUCCESS;

}

/*****************************************************************************+
* Function: trans_agent_build_alert_msg()
* Description: Build Alert message to Agent
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
u_int32_t trans_agent_build_alert_msg(
                            struct trans_send_msg_to_agent *p_msg_info,
                            u_int32_t * p_len,
                            u_int8_t * p_send_msg)
{
    struct trans_agent_msg *p_agent_msg = NULL;

    u_int8_t  a_timestamp[20+1] = {0};

    u_int32_t uw_len = 0;
    char *p_msg = NULL;  

    struct trans_agent_alert_info *p_alert_info = NULL;
    
    if ((NULL == p_msg_info) || (NULL == p_send_msg))
    {
        FLOG_ERROR("NULL PTR! \r\n");
        return TRANS_FAILD;
    } 

    p_alert_info = (struct trans_agent_alert_info *)p_msg_info->p_resp_msg;

    p_agent_msg = (struct trans_agent_msg *)p_send_msg;

    memset((u_int8_t*)p_agent_msg, 0, SIZEOF_TRANS_AGENT_MSG);
    
    json_t *msg = json_object();        // create an empty object;  
    
    json_t *source = json_string((char *)(g_trans_agent_config_info.a_device_id));// object for SOURCE;
    json_object_set_new(msg, "SOURCE", source);// set SOURCE;
    
    json_t *dest = json_string((char *)g_trans_agent_config_info.a_agent_id);// object for DEST
    json_object_set_new(msg, "DEST", dest);// set DEST
    
    json_t *body = json_object(); // create an empty object

    trans_agent_get_timestamp_string(a_timestamp);
    
    json_t *timestamp = json_string((char *)(a_timestamp));// object for timestamp
    json_object_set_new(body, "TIMESTAMP", timestamp);// set timestamp
    
    json_t *action = json_string("alert");// object for ACTION
    json_object_set_new(body, "ACTION", action);// set ACTION

    json_t *devid = json_string((char *)(g_trans_agent_config_info.a_device_id));// object for DEVID;
    json_object_set_new(body, "DEVID", devid);// set DEVID;

    json_t *alertid = json_integer(p_alert_info->us_alarm_id);// object for ALERTID;
    json_object_set_new(body, "ALERTID", alertid);// set ALERTID;

    json_t *alertval = json_real(p_alert_info->w_alarm_value);// object for VALUE;
    json_object_set_new(body, "VALUE", alertval);// set VALUE;

    json_object_set_new(msg, "BODY", body);  // set body elements in msg
    
    p_msg = json_dumps(msg, 0); //dump msg as a string
    
    //FLOG_DEBUG("json len : %ld \r\n", strlen((char *)p_msg));   
    //FLOG_DEBUG("json: %s \r\n", p_msg);
           
    strcpy((char *)p_agent_msg->a_magic, (char *)"MESS");   /*Msg type：4Bytes*/

    uw_len = strlen((char *)p_msg);
    sprintf((char *)p_agent_msg->a_len, "%04d", uw_len);/*Msg Length：4Bytes*/
    
    strcpy((char *)(p_send_msg+SIZEOF_TRANS_AGENT_MSG), (char *)p_msg);
   
    json_decref(msg);
    free(p_msg);
    
    FLOG_DEBUG("MSG: %s \r\n", p_send_msg);
    //FLOG_INFO("MSG len : %s \r\n", p_agent_msg->a_len);

    *p_len = uw_len + SIZEOF_TRANS_AGENT_MSG;

    return TRANS_SUCCESS;
}

/*****************************************************************************+
* Function: trans_agent_build_spectrum_msg()
* Description: Build Spectrum message to Agent
* Parameters:
*           NONE
* Return Values:
*           TRANS_FAILD : 1
*           TRANS_SUCCESS  : 0
*
*  
*  Data:    2011-07-27
* 
+*****************************************************************************/
u_int32_t trans_agent_build_spectrum_msg(
                        struct trans_send_msg_to_agent *p_msg_info,                        
                        u_int32_t * p_len,
                        u_int8_t * p_send_msg)
{
    int32_t w_ret = 0;
    struct trans_agent_msg *p_agent_msg = NULL;
    u_int8_t  a_str[20+1] = {0};
    
    u_int32_t uw_len = 0;
    u_int32_t uw_index = 0;
    
    char *p_msg = NULL; 
    double  d_sensing = 0.0;
    double f_latitude = 0.0;
    double f_longitude = 0.0;
    float     f_sensing_thd = 0.0;
    int        w_policy_code = 0;
    
    //struct trans_agent_spectrum_info *p_spectrum_info, 
    FLOG_DEBUG("Enter \r\n");
    
    if ((NULL == p_msg_info) || (NULL == p_send_msg) || (NULL == p_msg_info->p_resp_msg))
    {
        FLOG_ERROR("NULL PTR! \r\n");
        return TRANS_FAILD;
    } 

    f_latitude = 39.9;
    f_longitude = 116.4;

//    f_latitude = 0;
//    f_longitude = 0;

    w_ret = get_global_param ("INITIAL_SENSING_POLICY", & (w_policy_code));    
    if (TRANS_SUCCESS != w_ret)
    {
        FLOG_ERROR ("get parameters INITIAL_SENSING_POLICY error\n");
        return TRANS_FAILD;
    }
    
    w_ret = get_global_param ("INITIAL_SENSING_THD", & (f_sensing_thd));    
    if (TRANS_SUCCESS != w_ret)
    {
        FLOG_ERROR ("get parameters INITIAL_SENSING_THD error\n");
        return TRANS_FAILD;
    }

    p_agent_msg = (struct trans_agent_msg *)p_send_msg;
    
    memset((u_int8_t*)p_agent_msg, 0, SIZEOF_TRANS_AGENT_MSG);
    
    json_t *msg = json_object();        // create an empty object;  
    
    json_t *source = json_string((char *)(g_trans_agent_config_info.a_device_id));// object for SOURCE;
    json_object_set_new(msg, "SOURCE", source);// set SOURCE;
    
    //sprintf((char *)a_str, "%d", 53);/*Msg Length：4Bytes*/
    json_t *dest = json_string((char *)TRANS_AGENT_SPECTRUM_MGMT);// object for DEST
    json_object_set_new(msg, "DEST", dest);// set DEST
    
    json_t *body = json_object(); // create an empty object
    
    trans_agent_get_timestamp_string(a_str);
    
    json_t *timestamp = json_string((char *)(a_str));// object for timestamp
    json_object_set_new(body, "TIMESTAMP", timestamp);// set timestamp
    
    json_t *action = json_string("getSpectrumResult");// object for ACTION
    json_object_set_new(body, "ACTION", action);// set ACTION
    
    json_t *devid = json_string((char *)(g_trans_agent_config_info.a_device_id));// object for DEVID;
    json_object_set_new(body, "DEVID", devid);// set DEVID;

    json_t *sensingthd = json_real(f_sensing_thd);// object for SENSING_THD;
    json_object_set_new(body, "SENSING_THD", sensingthd);// set SENSING_THD;
    
    json_t *policycode = json_integer(w_policy_code);// object for POLICYCODE;
    json_object_set_new(body, "POLICYCODE", policycode);// set POLICYCODE;

    json_t *gps = json_object(); // create an empty object
    
    json_t *latitude = json_real(f_latitude);// object for ALTITUDE;
    json_object_set_new(gps, "LATITUDE", latitude);// set ALTITUDE;
    
    json_t *longitude = json_real(f_longitude);// object for LONGITUDE;
    json_object_set_new(gps, "LONGITUDE", longitude);// set LONGITUDE;
    
    json_object_set_new(body, "GPS", gps);  // set body elements in msg
    
    json_t *array = json_array();
    
    for (uw_index = 0; uw_index< TRANS_AGENT_SENSING_NUM; uw_index++)
    {
        d_sensing = *(((float *)p_msg_info->p_resp_msg) + uw_index);
        json_t *value = json_real(d_sensing);
        json_array_append_new(array, value);
    }
    
    json_object_set_new(body, "SENSING", array);  // set body elements in msg  
    
    json_object_set_new(msg, "BODY", body);  // set body elements in msg
    
    p_msg = json_dumps(msg, 0); //dump msg as a string
    
    //FLOG_DEBUG("json len : %ld \r\n", strlen((char *)p_msg));   
    //FLOG_DEBUG("json: %s \r\n", p_msg);
           
    strcpy((char *)p_agent_msg->a_magic, (char *)"MESS");   /*Msg type：4Bytes*/
    
    uw_len = strlen((char *)p_msg);
    sprintf((char *)p_agent_msg->a_len, "%04d", uw_len);/*Msg Length：4Bytes*/
    
    strcpy((char *)(p_send_msg+SIZEOF_TRANS_AGENT_MSG), (char *)p_msg);
    
    json_decref(msg);
    free(p_msg);
    
    FLOG_DEBUG("MSG: %s \r\n", p_send_msg);
    //FLOG_INFO("MSG len : %s \r\n", p_agent_msg->a_len);

    *p_len = uw_len + SIZEOF_TRANS_AGENT_MSG;

    FLOG_DEBUG("Exit \r\n");
    
    return TRANS_SUCCESS;

}

/*****************************************************************************+
* Function: trans_agent_build_periodic_msg()
* Description: Build Spectrum message to Agent
* Parameters:
*           NONE
* Return Values:
*           TRANS_FAILD : 1
*           TRANS_SUCCESS  : 0
*
*  
*  Data:    2011-09-29
* 
+*****************************************************************************/
u_int32_t trans_agent_build_periodic_msg(
                        struct trans_send_msg_to_agent *p_msg_info,                        
                        u_int32_t * p_len,
                        u_int8_t * p_send_msg)
{
    int32_t w_ret = 0;
    struct trans_agent_msg *p_agent_msg = NULL;
    u_int8_t  a_str[20+1] = {0};
    
    u_int32_t uw_len = 0;
    u_int32_t uw_index = 0;
    
    char *p_msg = NULL; 
    double  d_sensing = 0.0;
    int        w_interref = 0;
    double f_latitude = 0.0;
    double f_longitude = 0.0;
    float     f_sensing_thd = 0.0;
    int        w_working_freq = 0;

    struct trans_periodic_sensing_info * p_periodical = NULL;
    
    FLOG_DEBUG("Enter \r\n");
    
    if ((NULL == p_msg_info) || (NULL == p_send_msg) || (NULL == p_msg_info->p_resp_msg))
    {
        FLOG_ERROR("1 NULL PTR! \r\n");
        return TRANS_FAILD;
    } 

    p_periodical = (struct trans_periodic_sensing_info *)p_msg_info->p_resp_msg;
    if ((NULL == p_periodical) 
        || (NULL == p_periodical->p_per_interref) 
        || (NULL == p_periodical->p_per_sensing))
    {
        FLOG_ERROR("2 NULL PTR! \r\n");
        return TRANS_FAILD;
    } 
    
    f_latitude = 39.9;
    f_longitude = 116.4;

    w_ret = get_global_param ("DEFAULT_WORKING_FREQ", (void *)&(w_working_freq));
    if (w_ret != 0)
    {
        FLOG_ERROR ("get parameters DEFAULT_WORKING_FREQ error\n");
        return TRANS_FAILD;
    }
    
    w_ret = get_global_param ("PERIODIC_SENSING_THD", & (f_sensing_thd));    
    if (TRANS_SUCCESS != w_ret)
    {
        FLOG_ERROR ("get parameters INITIAL_SENSING_THD error\n");
        return TRANS_FAILD;
    }

    p_agent_msg = (struct trans_agent_msg *)p_send_msg;
    
    memset((u_int8_t*)p_agent_msg, 0, SIZEOF_TRANS_AGENT_MSG);
    
    json_t *msg = json_object();        // create an empty object;  
    
    json_t *source = json_string((char *)(g_trans_agent_config_info.a_device_id));// object for SOURCE;
    json_object_set_new(msg, "SOURCE", source);// set SOURCE;
    
    //sprintf((char *)a_str, "%d", 53);/*Msg Length：4Bytes*/
    json_t *dest = json_string((char *)TRANS_AGENT_SPECTRUM_MGMT);// object for DEST
    json_object_set_new(msg, "DEST", dest);// set DEST
    
    json_t *body = json_object(); // create an empty object
    
    trans_agent_get_timestamp_string(a_str);
    
    json_t *timestamp = json_string((char *)(a_str));// object for timestamp
    json_object_set_new(body, "TIMESTAMP", timestamp);// set timestamp
    
    json_t *action = json_string("sendPeriodicResult");// object for ACTION
    json_object_set_new(body, "ACTION", action);// set ACTION
    
    json_t *devid = json_string((char *)(g_trans_agent_config_info.a_device_id));// object for DEVID;
    json_object_set_new(body, "DEVID", devid);// set DEVID;

    json_t *sensingthd = json_real(f_sensing_thd);// object for SENSING_THD;
    json_object_set_new(body, "SENSING_THD", sensingthd);// set SENSING_THD;
    
    json_t *freq = json_real(w_working_freq);// object for POLICYCODE;
    json_object_set_new(body, "FREQ", freq);// set POLICYCODE;

    json_t *gps = json_object(); // create an empty object
    
    json_t *latitude = json_real(f_latitude);// object for ALTITUDE;
    //json_object_set_new(gps, "LATITUDE", latitude);// set LATITUDE;
    json_object_set_new(gps, "ALTITUDE", latitude);// set ALTITUDE;
    
    json_t *longitude = json_real(f_longitude);// object for LONGITUDE;
    json_object_set_new(gps, "LONGITUDE", longitude);// set LONGITUDE;
    
    json_object_set_new(body, "GPS", gps);  // set body elements in msg
    
    json_t *array1 = json_array();
    
    for (uw_index = 0; uw_index< TRANS_AGENT_PERSENSING_NUM; uw_index++)
    {
        d_sensing = *(((float *)p_periodical->p_per_sensing) + uw_index);
        json_t *value1 = json_real(d_sensing);
        json_array_append_new(array1, value1);
    }

    json_t *array2 = json_array();
    
    for (uw_index = 0; uw_index< TRANS_AGENT_PERINTERREF_NUM; uw_index++)
    {
        w_interref = *(((int *)p_periodical->p_per_interref) + uw_index);
        json_t *value2 = json_real(w_interref);
        json_array_append_new(array2, value2);
    }
    
    json_object_set_new(body, "PERSENSING", array1);  // set body elements in msg  
    
    json_object_set_new(body, "PERINTERREF", array2);  // set body elements in msg  
    
    json_object_set_new(msg, "BODY", body);  // set body elements in msg
    
    p_msg = json_dumps(msg, 0); //dump msg as a string
    
    //FLOG_DEBUG("json len : %ld \r\n", strlen((char *)p_msg));   
    //FLOG_DEBUG("json: %s \r\n", p_msg);
           
    strcpy((char *)p_agent_msg->a_magic, (char *)"MESS");   /*Msg type：4Bytes*/
    
    uw_len = strlen((char *)p_msg);
    sprintf((char *)p_agent_msg->a_len, "%04d", uw_len);/*Msg Length：4Bytes*/
    
    strcpy((char *)(p_send_msg + SIZEOF_TRANS_AGENT_MSG), (char *)p_msg);
    
    json_decref(msg);
    free(p_msg);
    
    FLOG_DEBUG("MSG: %s \r\n", p_send_msg);
    //FLOG_INFO("MSG len : %s \r\n", p_agent_msg->a_len);

    *p_len = uw_len + SIZEOF_TRANS_AGENT_MSG;

    FLOG_DEBUG("Exit \r\n");
    
    return TRANS_SUCCESS;

}

#endif

/*****************************************************************************+
* Function: trans_agent_send_t_update_msg()
* Description: Send TopologyUpdate message to Agent
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
static u_int32_t trans_agent_send_t_update_msg(
                            struct trans_send_msg_to_agent *p_msg_info,
                            u_int32_t   uw_buf_len,
                            u_int8_t * p_send_buf)

{
    struct trans_agent_msg *p_agent_msg = NULL;

    u_int8_t  a_timestamp[20+1] = {0};
    u_int32_t uw_len = 0;
    char *p_msg = NULL;  

    u_int8_t * p_send_msg = NULL;
    int w_ret = 0;
    u_int32_t uw_msg_len = 0;

    (void) p_send_buf;
    (void) uw_buf_len;
    
    if (NULL == p_msg_info)
    {
        FLOG_ERROR("NULL PTR! \r\n");
        return TRANS_FAILD;
    } 

    //p_alert_info = (struct trans_agent_alert_info *)p_msg_info->p_resp_msg;
    
    json_t *msg = json_object();        // create an empty object;  
    
    json_t *source = json_string((char *)(g_trans_agent_config_info.a_device_id));// object for SOURCE;
    json_object_set_new(msg, "SOURCE", source);// set SOURCE;
    
    json_t *dest = json_string((char *)TRANS_AGENT_SPECTRUM_MGMT);// object for DEST
    json_object_set_new(msg, "DEST", dest);// set DEST
    
    json_t *body = json_object(); // create an empty object

    trans_agent_get_timestamp_string(a_timestamp);
    
    json_t *timestamp = json_string((char *)(a_timestamp));// object for timestamp
    json_object_set_new(body, "TIMESTAMP", timestamp);// set timestamp
    
    json_t *action = json_string("topologyUpdate");// object for ACTION
    json_object_set_new(body, "ACTION", action);// set ACTION

    json_t *devid = json_string((char *)(g_trans_agent_config_info.a_device_id));// object for DEVID;
    json_object_set_new(body, "DEVID", devid);// set DEVID;

    json_t *array = json_array();
    
    json_t *value1 = json_string((char *)(p_msg_info->p_resp_msg));// object for element_id;
    json_array_append_new(array, value1);

    json_t *value2 = json_string((char *)(g_trans_agent_config_info.a_device_id));// object for element_id;
    json_array_append_new(array, value2);

    json_object_set_new(body, "TOPOLOGY", array);  // set body elements in msg  

    json_object_set_new(msg, "BODY", body);  // set body elements in msg
    
    p_msg = json_dumps(msg, 0); //dump msg as a string
    
    uw_len = strlen((char *)p_msg);
    
    uw_msg_len = uw_len + 1 + SIZEOF_TRANS_AGENT_MSG;
    
    p_send_msg = (u_int8_t *)malloc(uw_msg_len);
    if (NULL == p_send_msg)
    {
        FLOG_ERROR("malloc p_send_msg error! \r\n");

        json_decref(msg);
        free(p_msg);

        return TRANS_FAILD;   
    } 
    /*memset*/
    memset((u_int8_t*)p_send_msg, 0, uw_msg_len);
    
    p_agent_msg = (struct trans_agent_msg *)p_send_msg;
    
    strcpy((char *)p_agent_msg->a_magic, (char *)"MESS");   /*Msg type：4Bytes*/
    
    sprintf((char *)p_agent_msg->a_len, "%04d", uw_len);/*Msg Length：4Bytes*/
    
    strcpy((char *)(p_send_msg + SIZEOF_TRANS_AGENT_MSG), (char *)p_msg);
    
    json_decref(msg);
    free(p_msg);
        
    FLOG_DEBUG("send TopologyUpdate MSG: %s \r\n", p_send_msg);    
    //FLOG_INFO("send TopologyUpdate MSG len : %s \r\n", p_agent_send_msg->a_len);
    
    w_ret = send(g_trans_agent_socket.w_agent_socket, p_send_msg, uw_msg_len - 1, 0);
    if(w_ret <= 0)
    {
        //close(sock);
        FLOG_ERROR("send error! w_ret = %d\r\n", w_ret);

        free(p_send_msg);
        return TRANS_FAILD;
    }  
    
    free(p_send_msg);
    
    FLOG_INFO("send TopologyUpdate message to Agent OK! \r\n");

    return TRANS_SUCCESS;
}

/*****************************************************************************+
* Function: trans_agent_send_metric_msg()
* Description: 
* Parameters:
*           NONE
* Return Values:
*           TRANS_FAILD : 1
*           TRANS_SUCCESS  : 0
*
*  
*  Data:    2011-10-27
* 
+*****************************************************************************/
static u_int32_t trans_agent_send_metric_msg(
                            struct trans_send_msg_to_agent *p_msg_info,
                            u_int32_t   uw_buf_len,
                            u_int8_t * p_send_buf)
{
    struct trans_agent_metric_info *p_agent_metric = NULL;

    struct trans_agent_msg *p_agent_msg = NULL;
    u_int8_t  a_srt[20+1] = {0};
    char *p_msg = NULL;  
    u_int32_t uw_len = 0;

    u_int8_t uc_index = 0;
    u_int32_t uw_num = 0;
    int32_t w_value = 0;
    u_int32_t  uw_value = 0;
    float     f_value = 0.0;
    u_int8_t * p_temp = NULL;

    json_t *value1 = NULL;
    json_t *value2 = NULL;

    u_int8_t * p_send_msg = NULL;
    int w_ret = 0;
    u_int32_t uw_msg_len = 0;


    FLOG_DEBUG("Enter! \r\n");

    (void) p_send_buf;
    (void) uw_buf_len;

    if (NULL == p_msg_info)
    {
        FLOG_ERROR("NULL PTR! \r\n");
        return TRANS_FAILD;
    } 

    p_agent_metric = (struct trans_agent_metric_info *)p_msg_info->p_reqs_msg;

    if (NULL == p_agent_metric)
    {
        FLOG_ERROR("NULL PTR p_agent_metric! \r\n");
        return TRANS_FAILD;
    }

    uw_num = p_agent_metric->uc_metric_num;
   
    json_t *msg = json_object();        // create an empty object;  
    
    json_t *source = json_string((char *)(g_trans_agent_config_info.a_device_id));// object for SOURCE;
    json_object_set_new(msg, "SOURCE", source);// set SOURCE;
    
    sprintf((char *)a_srt, "%d", p_agent_metric->w_source_id);
    json_t *dest = json_string((char *)a_srt);// object for DEST
    json_object_set_new(msg, "DEST", dest);// set DEST
    
    json_t *body = json_object(); // create an empty object
    
    json_t *id = json_integer(p_agent_metric->w_msg_id);// object for id;
    json_object_set_new(body, "id", id);// set id;
    
    json_t *ref = json_integer(p_agent_metric->w_msg_id);// object for ref;
    json_object_set_new(body, "ref", ref);// set ref;
    
    trans_agent_get_timestamp_string(a_srt);
    
    json_t *timestamp = json_string((char *)(a_srt));// object for timestamp
    json_object_set_new(body, "timestamp", timestamp);// set timestamp    
    
    json_t *sendaction = json_string("getMetricValues_res");// object for action
    json_object_set_new(body, "action", sendaction);// set action
    
    json_t *moduleid = json_string((char *)(g_trans_agent_config_info.a_device_id));// object for module_id;
    json_object_set_new(body, "module_id", moduleid);// set module_id;
    
    json_t *content = json_object(); // create an empty object
    
    json_t *elementid = json_string((char *)(p_agent_metric->a_element_id));// object for element_id;
    json_object_set_new(content, "element_id", elementid);// set element_id;
    
    json_t *array1 = json_array();
    json_t *array2 = json_array();
    

    if (0 == strcmp ("rrh", (char *)p_agent_metric->a_element_id))
    {
        for (uc_index = 0; uc_index < uw_num; uc_index++)
        {
            /*Notice: just can change value of int8_t , if want to change int32_t, modify the code*/
            p_temp = ((u_int8_t *)(p_msg_info->p_resp_msg)) + ((uc_index + 1) * 4);
                
            switch (p_agent_metric->a_metric_id[uc_index])
            {
                /*Power Amplifier Temperature*/ 
                case TRANS_AGENT_RRH_METRIC_ID_PAT:                     
                /*Downlink Output Power for channel 1#*/
                case TRANS_AGENT_RRH_METRIC_ID_CH1_PWR:
                /*Downlink Output Power for channel 2#*/
                case TRANS_AGENT_RRH_METRIC_ID_CH2_PWR:
                    w_value = *((int8_t *)(p_temp - 1));

                    value1 = json_string((char *)(p_agent_metric->a_metrics[uc_index]));// object for element_id;
                    json_array_append_new(array1, value1);
                    
                    value2 = json_real(w_value);// object for element_id;
                    json_array_append_new(array2, value2);

           
                    break;
            
                /*Downlink Voltage Standing Wave Radio (VSWR) for channel 1#*/    
                case TRANS_AGENT_RRH_METRIC_ID_CH1_VSWR: 
                /*Downlink Voltage Standing Wave Radio (VSWR) for channel 2#*/    
                case TRANS_AGENT_RRH_METRIC_ID_CH2_VSWR: 
            
                    w_value = *((u_int8_t *)(p_temp - 1));      
                    
                    value1 = json_string((char *)(p_agent_metric->a_metrics[uc_index]));// object for element_id;
                    json_array_append_new(array1, value1);
                    
                    value2 = json_real(w_value);// object for element_id;
                    json_array_append_new(array2, value2);

                    break;    
                                   
                default:
                    
                    FLOG_ERROR("Unknow RRH message! metric_id = 0x%x\r\n", p_agent_metric->a_metric_id);
                    return TRANS_FAILD;
            
            }  
        
        }

//        if ( (0 == TRANS_SEND_METRIC_TO_RRH_FAKE) )
//        {
            /*Its memory was malloced in function ---trans_agent_rev_query_rrh()*/
//            if (p_agent_metric != NULL)
//            {
        free(p_agent_metric);
//            }
//        }
//        else
//        {

            /*********Test**********/
            //free(p_agent_metric);
            /*********Test**********/
//        }        
    }
    else if (0 == strcmp ("bs", (char *)p_agent_metric->a_element_id))
    {
        //memcpy(st_agent_metric.a_metric_val, p_msg_info->p_resp_msg, p_msg_info->uw_resp_len);
        for (uc_index = 0; uc_index < uw_num; uc_index++)
        {
            /*Notice: just can change value of float or int32_t , if want to change int8_t or int16_t, modify the code*/
            p_temp = ((u_int8_t *)(p_msg_info->p_resp_msg)) + (uc_index * 4);
                
            switch (p_agent_metric->a_metric_id[uc_index])
            {
                /*Per WMB DL throughput(avg)*/ 
                case TRANS_AGENT_BS_METRIC_ID_WMB_DL:                     
                /*Per WMB UL throughput(avg)*/
                case TRANS_AGENT_BS_METRIC_ID_WMB_UL:
                /*Per conn. DL throughput(avg)*/
                case TRANS_AGENT_BS_METRIC_ID_CONN_DL:
                /*Per conn. UL throughput(avg)*/    
                case TRANS_AGENT_BS_METRIC_ID_CONN_UL:
                /*UL management throughput(avg)*/    
                case TRANS_AGENT_BS_METRIC_ID_UL_MANG:
                /*DL management throughput(avg)*/    
                case TRANS_AGENT_BS_METRIC_ID_DL_MANG:

                    f_value = *((float *)(p_temp));
        
                    value1 = json_string((char *)(p_agent_metric->a_metrics[uc_index]));// object for element_id;
                    json_array_append_new(array1, value1);
                    
                    value2 = json_real(f_value);// object for element_id;
                    json_array_append_new(array2, value2);        
           
                    break;
            
                /*Ranging Success conut*/    
                case TRANS_AGENT_BS_METRIC_ID_RANG: 
                /*connection conut*/    
                case TRANS_AGENT_BS_METRIC_ID_CONN: 
                /*CRCErrorCount*/         
                case TRANS_AGENT_BS_METRIC_ID_CRC_ERR:
            
                    uw_value = *((u_int32_t *)(p_temp));      
                    
                    value1 = json_string((char *)(p_agent_metric->a_metrics[uc_index]));// object for element_id;
                    json_array_append_new(array1, value1);
                    
                    value2 = json_real(uw_value);// object for element_id;
                    json_array_append_new(array2, value2);
        
                    break;    
                                   
                default:
                    
                    FLOG_ERROR("Unknow BS message! metric_id = 0x%x\r\n", p_agent_metric->a_metric_id);
                    return TRANS_FAILD;
            
            }          
        }

        /*p_agent_metric will be free when Action is end*/
        
    }
    else if (0 == strcmp ("cpe", (char *)p_agent_metric->a_element_id))
    {
        //memcpy(st_agent_metric.a_metric_val, p_msg_info->p_resp_msg, p_msg_info->uw_resp_len);
        //memcpy(st_agent_metric.a_metric_val, p_msg_info->p_resp_msg, p_msg_info->uw_resp_len);
        for (uc_index = 0; uc_index < uw_num; uc_index++)
        {
            /*Notice: just can change value of float or int32_t , if want to change int8_t or int16_t, modify the code*/
            p_temp = ((u_int8_t *)(p_msg_info->p_resp_msg)) + (uc_index * 4);
                
            switch (p_agent_metric->a_metric_id[uc_index])
            {
                /*RSSI*/ 
                case TRANS_AGENT_MS_METRIC_ID_RSSI:                     
                /*SINR*/
                case TRANS_AGENT_MS_METRIC_ID_SINR:
                /*Tx Power*/
                case TRANS_AGENT_MS_METRIC_ID_TX_PWR:
                /*Temperature*/  
                case TRANS_AGENT_MS_METRIC_ID_T:

                    w_value = *((int32_t *)(p_temp));
        
                    value1 = json_string((char *)(p_agent_metric->a_metrics[uc_index]));// object for element_id;
                    json_array_append_new(array1, value1);
                    
                    value2 = json_real(w_value);// object for element_id;
                    json_array_append_new(array2, value2);        
           
                    break;
                                  
                default:
                    
                    FLOG_ERROR("Unknow MS message! metric_id = 0x%x\r\n", p_agent_metric->a_metric_id);
                    return TRANS_FAILD;
            
            }          
        }

        /*p_agent_metric will be free when Action is end*/
        
    }
    else
    {
        FLOG_ERROR("Source module error! src_moudle = %s\r\n", p_agent_metric->a_element_id);
    }
   
    json_object_set_new(content, "metrics", array1);  // set body elements in msg  
    json_object_set_new(content, "values", array2);  // set body elements in msg  
    
    json_object_set_new(body, "content", content);  // set body elements in msg
    json_object_set_new(msg, "BODY", body);  // set body elements in msg
    
    p_msg = json_dumps(msg, 0); //dump msg as a string  
    uw_len = strlen((char *)p_msg);
    
    uw_msg_len = uw_len + 1 + SIZEOF_TRANS_AGENT_MSG;

    p_send_msg = (u_int8_t *)malloc(uw_msg_len);
    if (NULL == p_send_msg)
    {
        FLOG_ERROR("malloc p_send_msg error! \r\n");

        json_decref(msg);
        free(p_msg);
        
        return TRANS_FAILD;   
    } 
    /*memset*/
    memset((u_int8_t*)p_send_msg, 0, uw_msg_len);

    p_agent_msg = (struct trans_agent_msg *)p_send_msg;
    
    strcpy((char *)p_agent_msg->a_magic, (char *)"MESS");   /*Msg type：4Bytes*/
    
    sprintf((char *)p_agent_msg->a_len, "%04d", uw_len);/*Msg Length：4Bytes*/
    
    strcpy((char *)(p_send_msg + SIZEOF_TRANS_AGENT_MSG), (char *)p_msg);
    
    json_decref(msg);
    free(p_msg);
        
    FLOG_DEBUG("sendMetric MSG: %s \r\n", p_send_msg);    
    //FLOG_INFO("sendMetric MSG len : %s \r\n", p_agent_send_msg->a_len);

    w_ret = send(g_trans_agent_socket.w_agent_socket, p_send_msg, uw_msg_len - 1, 0);
    if(w_ret <= 0)
    {
        //close(sock);
        FLOG_ERROR("send error! w_ret = %d\r\n", w_ret);

        free(p_send_msg);
        return TRANS_FAILD;
    }  

    free(p_send_msg);

    FLOG_INFO("send getMetricValues_res message to Agent OK! \r\n");

    FLOG_DEBUG("Exit! \r\n");
    
    return TRANS_SUCCESS;

}

/*****************************************************************************+
* Function: trans_agent_send_alert_msg()
* Description: Build Alert message to Agent
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
static u_int32_t trans_agent_send_alert_msg(
                            struct trans_send_msg_to_agent *p_msg_info,
                            u_int32_t   uw_buf_len,
                            u_int8_t * p_send_buf)

{
    struct trans_agent_msg *p_agent_msg = NULL;

    u_int8_t  a_timestamp[20+1] = {0};
    struct trans_agent_alert_info *p_alert_info = NULL;
    u_int32_t uw_len = 0;
    char *p_msg = NULL;  

    u_int8_t * p_send_msg = NULL;
    int w_ret = 0;
    u_int32_t uw_msg_len = 0;

    (void) p_send_buf;
    (void) uw_buf_len;
    
    if (NULL == p_msg_info)
    {
        FLOG_ERROR("NULL PTR! \r\n");
        return TRANS_FAILD;
    } 

    p_alert_info = (struct trans_agent_alert_info *)p_msg_info->p_resp_msg;
    
    json_t *msg = json_object();        // create an empty object;  
    
    json_t *source = json_string((char *)(g_trans_agent_config_info.a_device_id));// object for SOURCE;
    json_object_set_new(msg, "SOURCE", source);// set SOURCE;
    
    json_t *dest = json_string((char *)g_trans_agent_config_info.a_agent_id);// object for DEST
    json_object_set_new(msg, "DEST", dest);// set DEST
    
    json_t *body = json_object(); // create an empty object

    trans_agent_get_timestamp_string(a_timestamp);
    
    json_t *timestamp = json_string((char *)(a_timestamp));// object for timestamp
    json_object_set_new(body, "TIMESTAMP", timestamp);// set timestamp
    
    json_t *action = json_string("alert");// object for ACTION
    json_object_set_new(body, "ACTION", action);// set ACTION

    json_t *devid = json_string((char *)(g_trans_agent_config_info.a_device_id));// object for DEVID;
    json_object_set_new(body, "DEVID", devid);// set DEVID;

    json_t *alertid = json_integer(p_alert_info->us_alarm_id);// object for ALERTID;
    json_object_set_new(body, "ALERTID", alertid);// set ALERTID;

    json_t *alertval = json_real(p_alert_info->w_alarm_value);// object for VALUE;
    json_object_set_new(body, "VALUE", alertval);// set VALUE;

    json_object_set_new(msg, "BODY", body);  // set body elements in msg
    
    p_msg = json_dumps(msg, 0); //dump msg as a string
    
    uw_len = strlen((char *)p_msg);
    
    uw_msg_len = uw_len + 1 + SIZEOF_TRANS_AGENT_MSG;
    
    p_send_msg = (u_int8_t *)malloc(uw_msg_len);
    if (NULL == p_send_msg)
    {
        FLOG_ERROR("malloc p_send_msg error! \r\n");

        json_decref(msg);
        free(p_msg);

        return TRANS_FAILD;   
    } 
    /*memset*/
    memset((u_int8_t*)p_send_msg, 0, uw_msg_len);
    
    p_agent_msg = (struct trans_agent_msg *)p_send_msg;
    
    strcpy((char *)p_agent_msg->a_magic, (char *)"MESS");   /*Msg type：4Bytes*/
    
    sprintf((char *)p_agent_msg->a_len, "%04d", uw_len);/*Msg Length：4Bytes*/
    
    strcpy((char *)(p_send_msg + SIZEOF_TRANS_AGENT_MSG), (char *)p_msg);
    
    json_decref(msg);
    free(p_msg);
        
    FLOG_DEBUG("send Alert MSG: %s \r\n", p_send_msg);    
    //FLOG_INFO("send Alert MSG len : %s \r\n", p_agent_send_msg->a_len);
    
    w_ret = send(g_trans_agent_socket.w_agent_socket, p_send_msg, uw_msg_len - 1, 0);
    if(w_ret <= 0)
    {
        //close(sock);
        FLOG_ERROR("send error! w_ret = %d\r\n", w_ret);

        free(p_send_msg);
        return TRANS_FAILD;
    }  
    
    free(p_send_msg);
    
    FLOG_INFO("send alert message to Agent OK! \r\n");

    return TRANS_SUCCESS;
}

/*****************************************************************************+
* Function: trans_agent_send_spectrum_msg()
* Description: Build Spectrum message to Agent
* Parameters:
*           NONE
* Return Values:
*           TRANS_FAILD : 1
*           TRANS_SUCCESS  : 0
*
*  
*  Data:    2011-07-27
* 
+*****************************************************************************/
static u_int32_t trans_agent_send_spectrum_msg(
                        struct trans_send_msg_to_agent *p_msg_info,                        
                        u_int32_t   uw_buf_len,
                        u_int8_t * p_send_buf)

{
    int32_t w_ret = 0;
    struct trans_agent_msg *p_agent_msg = NULL;
    u_int8_t  a_str[20+1] = {0};
    
    u_int32_t uw_len = 0;
    u_int32_t uw_index = 0;
    
    char *p_msg = NULL; 
    double  d_sensing = 0.0;
    double f_latitude = 0.0;
    double f_longitude = 0.0;
    float     f_sensing_thd = 0.0;
    int        w_policy_code = 0;

    u_int8_t * p_send_msg = NULL;
    u_int32_t uw_msg_len = 0;
    
    //struct trans_agent_spectrum_info *p_spectrum_info, 
    FLOG_DEBUG("Enter \r\n");
    
    (void) p_send_buf;
    (void) uw_buf_len;
    
    if ((NULL == p_msg_info) || (NULL == p_msg_info->p_resp_msg))
    {
        FLOG_ERROR("NULL PTR! \r\n");
        return TRANS_FAILD;
    } 

    w_ret = get_global_param("RRU_LONGITUDE", (void *)(a_str));
    if (0 != w_ret)
    {
        FLOG_ERROR("get parameters RRU_LONGITUDE error");
        return TRANS_FAILD;
    }
    
    f_longitude = atof((char *)a_str); 
    
    w_ret = get_global_param("RRU_LATITUDE", (void *)(a_str));
    if (0 != w_ret)
    {
        FLOG_ERROR("get parameters RRU_LATITUDE error");
        return TRANS_FAILD;
    }
    
    f_latitude = atof((char *)a_str); 

    w_ret = get_global_param ("INITIAL_SENSING_POLICY", & (w_policy_code));    
    if (TRANS_SUCCESS != w_ret)
    {
        FLOG_ERROR ("get parameters INITIAL_SENSING_POLICY error\n");
        return TRANS_FAILD;
    }
    
    w_ret = get_global_param ("INITIAL_SENSING_THD", & (f_sensing_thd));    
    if (TRANS_SUCCESS != w_ret)
    {
        FLOG_ERROR ("get parameters INITIAL_SENSING_THD error\n");
        return TRANS_FAILD;
    }
     
    json_t *msg = json_object();        // create an empty object;  
    
    json_t *source = json_string((char *)(g_trans_agent_config_info.a_device_id));// object for SOURCE;
    json_object_set_new(msg, "SOURCE", source);// set SOURCE;
    
    //sprintf((char *)a_str, "%d", 53);/*Msg Length：4Bytes*/
    json_t *dest = json_string((char *)TRANS_AGENT_SPECTRUM_MGMT);// object for DEST
    json_object_set_new(msg, "DEST", dest);// set DEST
    
    json_t *body = json_object(); // create an empty object
    
    trans_agent_get_timestamp_string(a_str);
    
    json_t *timestamp = json_string((char *)(a_str));// object for timestamp
    json_object_set_new(body, "TIMESTAMP", timestamp);// set timestamp
    
    json_t *action = json_string("getSpectrumResult");// object for ACTION
    json_object_set_new(body, "ACTION", action);// set ACTION
    
    json_t *devid = json_string((char *)(g_trans_agent_config_info.a_device_id));// object for DEVID;
    json_object_set_new(body, "DEVID", devid);// set DEVID;

    json_t *sensingthd = json_real(f_sensing_thd);// object for SENSING_THD;
    json_object_set_new(body, "SENSING_THD", sensingthd);// set SENSING_THD;
    
    json_t *policycode = json_integer(w_policy_code);// object for POLICYCODE;
    json_object_set_new(body, "POLICYCODE", policycode);// set POLICYCODE;

    json_t *gps = json_object(); // create an empty object
    
    json_t *latitude = json_real(f_latitude);// object for ALTITUDE;
    json_object_set_new(gps, "LATITUDE", latitude);// set ALTITUDE;
    
    json_t *longitude = json_real(f_longitude);// object for LONGITUDE;
    json_object_set_new(gps, "LONGITUDE", longitude);// set LONGITUDE;
    
    json_object_set_new(body, "GPS", gps);  // set body elements in msg
    
    json_t *array = json_array();
    
    for (uw_index = 0; uw_index< TRANS_AGENT_SENSING_NUM; uw_index++)
    {
        d_sensing = *(((float *)p_msg_info->p_resp_msg) + uw_index);
        json_t *value = json_real(d_sensing);
        json_array_append_new(array, value);
    }
    
    json_object_set_new(body, "SENSING", array);  // set body elements in msg  
    
    json_object_set_new(msg, "BODY", body);  // set body elements in msg
    
    p_msg = json_dumps(msg, 0); //dump msg as a string
    
    uw_len = strlen((char *)p_msg);
    
    uw_msg_len = uw_len + 1 + SIZEOF_TRANS_AGENT_MSG;
    
    p_send_msg = (u_int8_t *)malloc(uw_msg_len);
    if (NULL == p_send_msg)
    {
        FLOG_ERROR("malloc p_send_msg error! \r\n");
        
        json_decref(msg);
        free(p_msg);
        
        return TRANS_FAILD;   
    } 
    /*memset*/
    memset((u_int8_t*)p_send_msg, 0, uw_msg_len);
    
    p_agent_msg = (struct trans_agent_msg *)p_send_msg;
    
    strcpy((char *)p_agent_msg->a_magic, (char *)"MESS");   /*Msg type：4Bytes*/
    
    sprintf((char *)p_agent_msg->a_len, "%04d", uw_len);/*Msg Length：4Bytes*/
    
    strcpy((char *)(p_send_msg + SIZEOF_TRANS_AGENT_MSG), (char *)p_msg);
    
    json_decref(msg);
    free(p_msg);
        
    FLOG_DEBUG("send getSpectrumResult MSG: %s \r\n", p_send_msg);    
    //FLOG_INFO("send getSpectrumResult MSG len : %s \r\n", p_agent_send_msg->a_len);
    
    w_ret = send(g_trans_agent_socket.w_agent_socket, p_send_msg, uw_msg_len - 1, 0);
    if(w_ret <= 0)
    {
        //close(sock);
        FLOG_ERROR("send error! w_ret = %d\r\n", w_ret);

        free(p_send_msg);
        return TRANS_FAILD;
    }  
    
    FLOG_INFO("send getSpectrumResult message to Agent OK! \r\n");

    if (TRANS_QUENE_BLOCK == p_msg_info->uc_block_flag)
    {
        /*ADD TIMER*/
        struct timeval st_time_val;
        gettimeofday(&st_time_val, NULL);  
        
        /*Timeout -- 30s*/
        st_time_val.tv_sec = st_time_val.tv_sec + 30;   
        //st_time_val.tv_usec = st_time_val.tv_usec;
        
        void* p_timer_id = NULL;
        struct trans_timer_msg_info st_msg_info;
        
        st_msg_info.us_serial_number = 0xffff;
        st_msg_info.uw_src_moudle = TRANS_MOUDLE_BS;
        st_msg_info.uc_block_flag = TRANS_QUENE_BLOCK;
        st_msg_info.f_callback = trans_agent_block_msg_timer_func;
        st_msg_info.p_user_info = NULL;
        
        w_ret = trans_timer_add(&st_time_val, 
                                trans_agent_block_msg_timer_func, 
                                p_send_msg, 
                                uw_msg_len, 
                                &st_msg_info,
                                &p_timer_id);
        
        if (TRANS_SUCCESS != w_ret) 
        {   
            FLOG_ERROR("Call trans_timer_add error! uw_ret = %d\r\n", w_ret);

            free(p_send_msg);
            return TRANS_FAILD;     
        } 
    
        u_int8_t uc_result = 0;
        w_ret = trans_msg_de_quene(&uc_result);
        
        if ((TRANS_SUCCESS != w_ret) || (TRANS_SUCCESS != uc_result))
        {
            FLOG_ERROR("The response message from Agent error!result = %d, uw_ret. \r\n", uc_result, w_ret);

            free(p_send_msg);
            return TRANS_FAILD; 
        }    
    }

    free(p_send_msg);
  
    FLOG_DEBUG("Exit \r\n");
    
    return TRANS_SUCCESS;

}

/*****************************************************************************+
* Function: trans_agent_send_periodic_msg()
* Description: Build Spectrum message to Agent
* Parameters:
*           NONE
* Return Values:
*           TRANS_FAILD : 1
*           TRANS_SUCCESS  : 0
*
*  
*  Data:    2011-09-29
* 
+*****************************************************************************/
static u_int32_t trans_agent_send_periodic_msg(
                        struct trans_send_msg_to_agent *p_msg_info,                        
                        u_int32_t   uw_buf_len,
                        u_int8_t * p_send_buf)

{
    int32_t w_ret = 0;
    struct trans_agent_msg *p_agent_head = NULL;
    u_int8_t  a_str[20+1] = {0};
    
    u_int32_t uw_periodic_len = 0;
    u_int32_t uw_agent_len = 0;
    u_int32_t uw_index = 0;
    
    char *p_periodic_msg = NULL; 
    char *p_agent_msg = NULL; 
    double  d_sensing = 0.0;
    int        w_interref = 0;
    double f_latitude = 0.0;
    double f_longitude = 0.0;
    float     f_sensing_thd = 0.0;
    int        w_working_freq = 0;

    struct trans_periodic_sensing_info * p_periodical = NULL;

    u_int8_t * p_send_msg = NULL;
    u_int32_t uw_msg_len = 0;
   
    FLOG_DEBUG("Enter \r\n");

    (void) p_send_buf;
    (void) uw_buf_len;
    
    if ((NULL == p_msg_info) || (NULL == p_msg_info->p_resp_msg))
    {
        FLOG_ERROR("1 NULL PTR! \r\n");
        return TRANS_FAILD;
    } 

    p_periodical = (struct trans_periodic_sensing_info *)p_msg_info->p_resp_msg;
    if ((NULL == p_periodical) 
        || (NULL == p_periodical->p_per_interref) 
        || (NULL == p_periodical->p_per_sensing))
    {
        FLOG_ERROR("2 NULL PTR! \r\n");
        return TRANS_FAILD;
    } 
    
    w_ret = get_global_param("RRU_LONGITUDE", (void *)(a_str));
    if (0 != w_ret)
    {
        FLOG_ERROR("get parameters RRU_LONGITUDE error");
        return TRANS_FAILD;
    }
    
    f_longitude = atof((char *)a_str); 
    
    w_ret = get_global_param("RRU_LATITUDE", (void *)(a_str));
    if (0 != w_ret)
    {
        FLOG_ERROR("get parameters RRU_LATITUDE error");
        return TRANS_FAILD;
    }
    
    f_latitude = atof((char *)a_str); 

    w_ret = get_global_param ("DEFAULT_WORKING_FREQ", (void *)&(w_working_freq));
    if (w_ret != 0)
    {
        FLOG_ERROR ("get parameters DEFAULT_WORKING_FREQ error\n");
        return TRANS_FAILD;
    }
    
    w_ret = get_global_param ("PERIODIC_SENSING_THD", & (f_sensing_thd));    
    if (TRANS_SUCCESS != w_ret)
    {
        FLOG_ERROR ("get parameters INITIAL_SENSING_THD error\n");
        return TRANS_FAILD;
    }

    trans_agent_get_timestamp_string(a_str);
    json_t *timestamp = json_string((char *)(a_str));// object for timestamp
    
    json_t *action = json_string("sendPeriodicResult");// object for ACTION
    
    json_t *devid = json_string((char *)(g_trans_agent_config_info.a_device_id));// object for DEVID;

    /*Creat JSON message for periodic sensing result stream */    
    json_t *periodic_msg = json_object();        // create an empty object;  

    //json_t *timestamp = json_string((char *)(a_str));// object for timestamp
    json_object_set_new(periodic_msg, "TIMESTAMP", timestamp);// set timestamp
    
    //json_t *action = json_string("sendPeriodicResult");// object for ACTION
    json_object_set_new(periodic_msg, "ACTION", action);// set ACTION
    
    //json_t *devid = json_string((char *)(g_trans_agent_config_info.a_device_id));// object for DEVID;
    json_object_set_new(periodic_msg, "DEVID", devid);// set DEVID;
    
    json_t *sensingthd = json_real(f_sensing_thd);// object for SENSING_THD;
    json_object_set_new(periodic_msg, "SENSING_THD", sensingthd);// set SENSING_THD;
    
    json_t *freq = json_real(w_working_freq);// object for POLICYCODE;
    json_object_set_new(periodic_msg, "FREQ", freq);// set POLICYCODE;
    
    json_t *gps = json_object(); // create an empty object
    
    json_t *latitude = json_real(f_latitude);// object for ALTITUDE;
    //json_object_set_new(gps, "LATITUDE", latitude);// set LATITUDE;
    json_object_set_new(gps, "ALTITUDE", latitude);// set ALTITUDE;
    
    json_t *longitude = json_real(f_longitude);// object for LONGITUDE;
    json_object_set_new(gps, "LONGITUDE", longitude);// set LONGITUDE;
    
    json_object_set_new(periodic_msg, "GPS", gps);  // set body elements in msg
    
    json_t *array1 = json_array();
    
    for (uw_index = 0; uw_index< TRANS_AGENT_PERSENSING_NUM; uw_index++)
    {
        d_sensing = *(((float *)p_periodical->p_per_sensing) + uw_index);
        json_t *value1 = json_real(d_sensing);
        json_array_append_new(array1, value1);
    }
    
    json_t *array2 = json_array();
    
    for (uw_index = 0; uw_index< TRANS_AGENT_PERINTERREF_NUM; uw_index++)
    {
        w_interref = *(((int *)p_periodical->p_per_interref) + uw_index);
        json_t *value2 = json_real(w_interref);
        json_array_append_new(array2, value2);
    }
    
    json_object_set_new(periodic_msg, "PERSENSING", array1);  // set body elements in msg  
    
    json_object_set_new(periodic_msg, "PERINTERREF", array2);  // set body elements in msg  
    
    p_periodic_msg = json_dumps(periodic_msg, 0); //dump msg as a string    
    uw_periodic_len = strlen((char *)p_periodic_msg);


    /*Creat JSON message for agent */    
    json_t *agent_msg = json_object();        // create an empty object;  
    
    json_t *source = json_string((char *)(g_trans_agent_config_info.a_device_id));// object for SOURCE;
    json_object_set_new(agent_msg, "SOURCE", source);// set SOURCE;
    
    json_t *dest = json_string((char *)TRANS_AGENT_SPECTRUM_MGMT);// object for DEST
    json_object_set_new(agent_msg, "DEST", dest);// set DEST
    
    json_t *agent_body = json_object(); // create an empty object    
    
    //json_t *timestamp = json_string((char *)(a_str));// object for timestamp
    json_object_set_new(agent_body, "TIMESTAMP", timestamp);// set timestamp

    sprintf((char *)a_str, "%d", uw_periodic_len);/*Msg Length：4Bytes*/
    json_t *len= json_string((char *)(a_str));// object for DEST
    json_object_set_new(agent_body, "LENGTH", len);// set DEST
    
    //json_t *action = json_string("sendPeriodicResult");// object for ACTION
    json_object_set_new(agent_body, "ACTION", action);// set ACTION
    
    //json_t *devid = json_string((char *)(g_trans_agent_config_info.a_device_id));// object for DEVID;
    json_object_set_new(agent_body, "DEVID", devid);// set DEVID;

    json_object_set_new(agent_msg, "BODY", agent_body);  // set body elements in msg
    
    p_agent_msg = json_dumps(agent_msg, 0); //dump msg as a string    
    uw_agent_len = strlen((char *)p_agent_msg);
    
    /*Creat the whole message*/
    uw_msg_len = 2 + uw_agent_len + uw_periodic_len + SIZEOF_TRANS_AGENT_MSG;
    
    p_send_msg = (u_int8_t *)malloc(uw_msg_len);
    if (NULL == p_send_msg)
    {
        FLOG_ERROR("malloc p_send_msg error! \r\n");
        
        json_decref(agent_msg);
        json_decref(periodic_msg);
        free(p_agent_msg);
        free(p_periodic_msg);
        
        return TRANS_FAILD;   
    } 
    /*memset*/
    memset((u_int8_t*)p_send_msg, 0, uw_msg_len);
    
    p_agent_head = (struct trans_agent_msg *)p_send_msg;
    
    strcpy((char *)p_agent_head->a_magic, (char *)"FILE");   /*Msg type：4Bytes*/
    
    sprintf((char *)p_agent_head->a_len, "%04d", uw_agent_len);/*Msg Length：4Bytes*/
    
    memcpy(p_send_msg + SIZEOF_TRANS_AGENT_MSG, p_agent_msg, uw_agent_len);   

    memcpy(p_send_msg + SIZEOF_TRANS_AGENT_MSG + uw_agent_len, p_periodic_msg, uw_periodic_len);   
    //strcpy((char *)(p_send_msg + SIZEOF_TRANS_AGENT_MSG), (char *)p_agent_msg);
    //strcpy((char *)(p_send_msg + SIZEOF_TRANS_AGENT_MSG + uw_agent_len), (char *)p_periodic_msg);
    
    json_decref(agent_msg);
    json_decref(periodic_msg);
    free(p_agent_msg);
    free(p_periodic_msg);
        
    FLOG_DEBUG("send sendPeriodicResult MSG: %s \r\n", p_send_msg);     
    FLOG_DEBUG("uw_agent_len = %d, uw_periodic_len = %d, uw_msg_len = %d. \r\n", uw_agent_len, uw_periodic_len, uw_msg_len); 
    //FLOG_INFO("send sendPeriodicResult MSG len : %s \r\n", p_agent_send_msg->a_len);
    
    w_ret = send(g_trans_agent_socket.w_agent_socket, p_send_msg, uw_msg_len - 2, 0);
    if(w_ret <= 0)
    {
        //close(sock);
        FLOG_ERROR("send error! w_ret = %d\r\n", w_ret);

        free(p_send_msg);
        return TRANS_FAILD;
    }  

    free(p_send_msg);
    
    FLOG_INFO("send sendPeriodicResult message to Agent OK! \r\n");

    FLOG_DEBUG("Exit \r\n");
    
    return TRANS_SUCCESS;

}

/*****************************************************************************+
* Function: trans_agent_send_msg()
* Description: 
* Parameters:
*           NONE
* Return Values:
*           TRANS_FAILD : 1
*           TRANS_SUCCESS  : 0
*
*  
*  Data:    2011-09-14
* 
+*****************************************************************************/
u_int32_t trans_agent_send_msg(
                            struct trans_send_msg_to_agent *p_msg_info,
                            size_t len,
                            u_int8_t * p_send_msg)
{

    int32_t w_ret = 0;
    u_int8_t  a_action[TRANS_AGENT_ACTION_SIZE+1] = {0};
    //int32_t w_agent_sockfd = 0;
    //u_int32_t uw_len = 0;

    //u_int8_t      uc_ack_flag;   
    if ((NULL == p_msg_info) || (NULL == p_send_msg))
    {
        FLOG_ERROR("NULL PTR! \r\n");
        return TRANS_FAILD;
    } 
    
    //uw_len = len;
        
    strcpy((char *)(a_action), (char *)(p_msg_info->p_reqs_msg));

    if (0 == strcmp ("getMetricValues", (char *)a_action))
    {
        w_ret = trans_agent_send_metric_msg(p_msg_info, 
                            len, 
                            p_send_msg);

        if (TRANS_SUCCESS != w_ret)
        {
            FLOG_ERROR("Call trans_agent_send_metric_msg error! uw_ret = %d\r\n", w_ret);
            return TRANS_FAILD;
        }             
    }
    else if (0 == strcmp ("alert", (char *)a_action))
    {
        w_ret = trans_agent_send_alert_msg(p_msg_info, 
                            len, 
                            p_send_msg);
    
        if (TRANS_SUCCESS != w_ret)
        {
            FLOG_ERROR("Call trans_agent_send_alert_msg error! uw_ret = %d\r\n", w_ret);
            return TRANS_FAILD;
        }
    }
    else if (0 == strcmp ("spectrum", (char *)a_action))
    {
        w_ret = trans_agent_send_spectrum_msg(p_msg_info, 
                            len, 
                            p_send_msg);
    
        if (TRANS_SUCCESS != w_ret)
        {
            FLOG_ERROR("Call trans_agent_send_spectrum_msg error! uw_ret = %d\r\n", w_ret);
            return TRANS_FAILD;
        }
    }
    else if (0 == strcmp ("periodic", (char *)a_action))
    {
        w_ret = trans_agent_send_periodic_msg(p_msg_info, 
                            len, 
                            p_send_msg);
    
        if (TRANS_SUCCESS != w_ret)
        {
            FLOG_ERROR("Call trans_agent_send_periodic_msg error! uw_ret = %d\r\n", w_ret);
            return TRANS_FAILD;
        }
    }    
    else if (0 == strcmp ("topologyUpdate", (char *)a_action))
    {
        w_ret = trans_agent_send_t_update_msg(p_msg_info, 
                            len, 
                            p_send_msg);
    
        if (TRANS_SUCCESS != w_ret)
        {
            FLOG_ERROR("Call trans_agent_send_t_update_msg error! uw_ret = %d\r\n", w_ret);
            return TRANS_FAILD;
        }
    }    
    else
    {
        FLOG_ERROR("action/ACTION error! action = %s .\r\n", a_action);
        
        return TRANS_FAILD;
    }
    
    #if 0
    //trans_debug_msg_print(p_send_msg, 40, g_trans_debug_agent);

    w_agent_sockfd = g_trans_agent_socket.w_agent_socket;
    
    w_ret = send(w_agent_sockfd, p_send_msg, uw_len, 0);
    if(w_ret <= 0)
    {
        //close(sock);
        FLOG_ERROR("send error! w_ret = %d\r\n", w_ret);
        return TRANS_FAILD;
    }  

    FLOG_INFO("send Agent message OK! \r\n");

    if (TRANS_QUENE_BLOCK == p_msg_info->uc_block_flag)
    {
        /*ADD TIMER*/
        struct timeval st_time_val;
        gettimeofday(&st_time_val, NULL);  
        
        /*Timeout -- 20s*/
        st_time_val.tv_sec = st_time_val.tv_sec + 20;   
        //st_time_val.tv_usec = st_time_val.tv_usec;
        
        void* p_timer_id = NULL;
        struct trans_timer_msg_info st_msg_info;
        
        st_msg_info.us_serial_number = 0xffff;
        st_msg_info.uw_src_moudle = TRANS_MOUDLE_BS;
        st_msg_info.uc_block_flag = TRANS_QUENE_BLOCK;
        st_msg_info.f_callback = trans_agent_block_msg_timer_func;
        st_msg_info.p_user_info = NULL;
        
        w_ret = trans_timer_add(&st_time_val, 
                                trans_agent_block_msg_timer_func, 
                                p_send_msg, 
                                uw_len + SIZEOF_TRANS_AGENT_MSG, 
                                &st_msg_info,
                                &p_timer_id);
        
        if (TRANS_SUCCESS != w_ret) 
        {   
            FLOG_ERROR("Call trans_timer_add error! uw_ret = %d\r\n", w_ret);
            return TRANS_FAILD;     
        } 

        u_int8_t uc_result = 0;
        w_ret = trans_msg_de_quene(&uc_result);
        
        if ((TRANS_SUCCESS != w_ret) || (TRANS_SUCCESS != uc_result))
        {
            FLOG_ERROR("The response message from Agent error!result = %d, uw_ret. \r\n", uc_result, w_ret);
            return TRANS_FAILD; 
        }

    }
    #endif
    
    return TRANS_SUCCESS;
    
}


/*****************************************************************************+
* Function: trans_agent_get_metric_id()
* Description: 
* Parameters:
*           NONE
* Return Values:
*           TRANS_FAILD : 1
*           TRANS_SUCCESS  : 0
*
*  
*  Data:    2011-08-01
* 
+*****************************************************************************/
u_int32_t trans_agent_get_metric_id(const char * p_metric, int32_t *p_metric_id)
{
    if ((NULL == p_metric) ||(NULL == p_metric_id))
    {
        FLOG_ERROR("NULL PTR! \r\n");
        return TRANS_FAILD;
    } 

    /*BS metric*/
    if (0 == strcmp ("RangingSuccessCount", (char *)p_metric))
    {
        *p_metric_id = TRANS_AGENT_BS_METRIC_ID_RANG;
    }
    else if (0 == strcmp ("PerWmbDLThroughput", (char *)p_metric))
    {
        *p_metric_id = TRANS_AGENT_BS_METRIC_ID_WMB_DL;
    }
    else if (0 == strcmp ("PerWmbULThroughput", (char *)p_metric))
    {
        *p_metric_id = TRANS_AGENT_BS_METRIC_ID_WMB_UL;
    }
    else if (0 == strcmp ("PerConnDLThroughput", (char *)p_metric))
    {
        *p_metric_id = TRANS_AGENT_BS_METRIC_ID_CONN_DL;
    }
    else if (0 == strcmp ("PerConnULThroughput", (char *)p_metric))
    {
        *p_metric_id = TRANS_AGENT_BS_METRIC_ID_CONN_UL;
    }
    else if (0 == strcmp ("ULMgmtThroughput", (char *)p_metric))
    {
        *p_metric_id = TRANS_AGENT_BS_METRIC_ID_UL_MANG;
    }    
    else if (0 == strcmp ("DLMgmtThroughput", (char *)p_metric))
    {
        *p_metric_id = TRANS_AGENT_BS_METRIC_ID_DL_MANG;
    }       
    else if (0 == strcmp ("ConnectionCount", (char *)p_metric))
    {
        *p_metric_id = TRANS_AGENT_BS_METRIC_ID_CONN;
    }   
    else if (0 == strcmp ("CRCErrorCount", (char *)p_metric))
    {
        *p_metric_id = TRANS_AGENT_BS_METRIC_ID_CRC_ERR;
    }  

    /*RRH metric*/
    else if (0 == strcmp ("PwrAmplTemp", (char *)p_metric))
    {
        *p_metric_id = TRANS_AGENT_RRH_METRIC_ID_PAT;
    }
    else if (0 == strcmp ("DownOutPwr1", (char *)p_metric))
    {
        *p_metric_id = TRANS_AGENT_RRH_METRIC_ID_CH1_PWR;    
    }
    else if (0 == strcmp ("DownOutPwr2", (char *)p_metric))
    {
        *p_metric_id = TRANS_AGENT_RRH_METRIC_ID_CH2_PWR;    
    }
    else if (0 == strcmp ("DownVSWR1", (char *)p_metric))
    {
        *p_metric_id = TRANS_AGENT_RRH_METRIC_ID_CH1_VSWR;    
    }    
    else if (0 == strcmp ("DownVSWR2", (char *)p_metric))
    {
        *p_metric_id = TRANS_AGENT_RRH_METRIC_ID_CH2_VSWR;    
    }    
    /*MS metric*/
    else if (0 == strcmp ("RSSI", (char *)p_metric))
    {
        *p_metric_id = TRANS_AGENT_MS_METRIC_ID_RSSI;
    }   
    else if (0 == strcmp ("SINR", (char *)p_metric))
    {
        *p_metric_id = TRANS_AGENT_MS_METRIC_ID_SINR;    
    }   
    else if (0 == strcmp ("TxPower", (char *)p_metric))
    {
        *p_metric_id = TRANS_AGENT_MS_METRIC_ID_TX_PWR;    
    }   
    else if (0 == strcmp ("Temperature", (char *)p_metric))
    {
        *p_metric_id = TRANS_AGENT_MS_METRIC_ID_T;    
    }   
    else
    {
        FLOG_ERROR("p_metric error! p_metric = %s .\r\n", p_metric);
        
        return TRANS_FAILD;
    }

    FLOG_DEBUG("p_metric: %s, *p_metric_id; %d \r\n", p_metric, *p_metric_id);

    return TRANS_SUCCESS;
}

/*****************************************************************************+
* Function: trans_agent_get_rrh_id()
* Description: 
* Parameters:
*           NONE
* Return Values:
*           TRANS_FAILD : 1
*           TRANS_SUCCESS  : 0
*
*  
*  Data:    2011-08-01
* 
+*****************************************************************************/
u_int32_t trans_agent_get_rrh_id(const char * p_metric, u_int16_t *p_rrh_id)
{
    if ((NULL == p_metric) ||(NULL == p_rrh_id))
    {
        FLOG_ERROR("NULL PTR! \r\n");
        return TRANS_FAILD;
    } 

    /*RRH metric*/
    if (0 == strcmp ("PwrAmplTemp", (char *)p_metric))
    {
        *p_rrh_id = htons(RRH_MSG_PA_TEMP_VALUE);

    }
    else if (0 == strcmp ("DownOutPwr1", (char *)p_metric))
    {
        *p_rrh_id = htons(RRH_MSG_DL_INPUT1_LEVEL);
    
    }
    else if (0 == strcmp ("DownOutPwr2", (char *)p_metric))
    {
        *p_rrh_id = htons(RRH_MSG_DL_INPUT2_LEVEL);
    
    }
    else if (0 == strcmp ("DownVSWR1", (char *)p_metric))
    {
        *p_rrh_id = htons(RRH_MSG_DL_VSWR1_VALUE);
    
    }
    else if (0 == strcmp ("DownVSWR2", (char *)p_metric))
    {
        *p_rrh_id = htons(RRH_MSG_DL_VSWR2_VALUE);
    
    }
    else
    {
        FLOG_ERROR("p_metric error! p_metric = %s .\r\n", p_metric);
        
        return TRANS_FAILD;
    }

    FLOG_DEBUG("p_metric: %s, *p_rrh_id; 0x%x \r\n", p_metric, *p_rrh_id);

    return TRANS_SUCCESS;
}

/*****************************************************************************+
* Function: trans_agent_get_metric_info()
* Description: 
* Parameters:
*           NONE
* Return Values:
*           TRANS_FAILD : 1
*           TRANS_SUCCESS  : 0
*
*  
*  Data:    2011-08-01
* 
+*****************************************************************************/
u_int32_t trans_agent_get_metric_info(json_t *msg,  
                                    struct trans_agent_metric_info *p_metric_info)
{
    const char * p_tr = NULL;
    u_int32_t     uw_num = 0;
    u_int8_t  uc_index = 0;
    u_int32_t uw_ret = 0;
    
    if ((NULL == msg) ||(NULL == p_metric_info))
    {
        FLOG_ERROR("NULL PTR! \r\n");
        return TRANS_FAILD;
    }  

    /*Get SOURCE*/
    json_t *source = json_object_get(msg, "SOURCE"); // get SOURCE
    if (NULL == source)
    {
        FLOG_ERROR("Call json_object_get for SOURCE  error! NULL PTR  .\r\n");
        
        return TRANS_FAILD;
    }
    
    p_tr = json_string_value(source);
    if (NULL == p_tr)
    {
        FLOG_ERROR("Call json_string_value for SOURCE  error! NULL PTR  .\r\n");
        
        return TRANS_FAILD;
    }
    
    p_metric_info->w_source_id = atoi((char *)(p_tr)); 
    
    //FLOG_DEBUG("w_source_id =0x%x, source: %s. \r\n", p_metric_info->w_source_id, p_tr);    
    
    json_t *body = json_object_get(msg, "BODY"); //get BODY elements
    if (NULL == body)
    {
        FLOG_ERROR("Call json_object_get for BODY  error! NULL PTR  .\r\n");
        
        return TRANS_FAILD;
    }
    
    json_t *id= json_object_get(body, "id");// get metric_id
    if (NULL == id)
    {
        FLOG_ERROR("Call json_object_get for id  error! NULL PTR  .\r\n");
        
        return TRANS_FAILD;
    }

    p_metric_info->w_msg_id = json_integer_value(id);
    if (0 == p_metric_info->w_msg_id)
    {
        FLOG_ERROR("Call json_integer_value for w_msg_id error! .\r\n");
        
        return TRANS_FAILD;
    }
    
    //FLOG_DEBUG("w_msg_id = %d. \r\n", p_metric_info->w_msg_id);

    json_t *content = json_object_get(body, "content"); //get BODY elements
    if (NULL == body)
    {
        FLOG_ERROR("Call json_object_get for content error! NULL PTR  .\r\n");
        
        return TRANS_FAILD;
    }

    json_t *elementid = json_object_get(content, "element_id");// get metric_id
    if (NULL == elementid)
    {
        FLOG_ERROR("Call json_object_get for element_id  error! NULL PTR  .\r\n");
        
        return TRANS_FAILD;
    }

    p_tr = json_string_value(elementid);
    if (NULL == p_tr)
    {
        FLOG_ERROR("Call json_string_value for element_id  error! NULL PTR  .\r\n");
        
        return TRANS_FAILD;
    }

    strcpy((char *)(p_metric_info->a_element_id), p_tr);

    //FLOG_DEBUG("a_element_id =%s. \r\n", p_metric_info->a_element_id);

    json_t *array= json_object_get(content, "metrics");// get metric_id
    if (NULL == array)
    {
        FLOG_ERROR("Call json_object_get for array error! NULL PTR  .\r\n");
        
        return TRANS_FAILD;
    }

    uw_num = json_array_size(array);
    #if 0
    if (1 != uw_num)
    {
        FLOG_ERROR("Call json_array_size for array error! uw_num = %d.\r\n", uw_num);
        
        return TRANS_FAILD;
    }
    #endif

    p_metric_info->uc_metric_num = uw_num;

    for (uc_index = 0; uc_index < uw_num; uc_index++)
    {
        json_t *metrics = json_array_get(array, uc_index);
        if (NULL == metrics)
        {
            FLOG_ERROR("Call json_array_get for metrics error! NULL PTR  .\r\n");
            
            return TRANS_FAILD;
        }
        
        p_tr = json_string_value(metrics);
        if (NULL == p_tr)
        {
            FLOG_ERROR("Call json_string_value for metrics  error! NULL PTR  .\r\n");
            
            return TRANS_FAILD;
        }
        
        strcpy((char *)(p_metric_info->a_metrics[uc_index]), p_tr);

        FLOG_DEBUG("a_metrics[%d] =%s. \r\n", uc_index, p_metric_info->a_metrics[uc_index]);   
        
        uw_ret = trans_agent_get_metric_id((char *)p_metric_info->a_metrics[uc_index], 
                        &(p_metric_info->a_metric_id[uc_index]));
        
        if (TRANS_SUCCESS != uw_ret)
        {
            FLOG_ERROR("Call trans_agent_get_metric_id error! uw_ret = %d.\r\n", uw_ret);
            
            return TRANS_FAILD;
        }

    }

    return TRANS_SUCCESS;
}

/*****************************************************************************+
* Function: trans_agent_forward2_action()
* Description: 
* Parameters:
*           NONE
* Return Values:
*           TRANS_FAILD : 1
*           TRANS_SUCCESS  : 0
*
*  
*  Data:    2011-08-01
* 
+*****************************************************************************/
u_int32_t trans_agent_forward2_action(fun_callback f_ptr,
                           size_t len,
                           void    *p_action_msg)
{
    u_int32_t   uw_ret = 0;
    struct trans_action_info st_action_info;
    //struct trans_agent_metric_info *p_metric_info = NULL;
    
    gettimeofday(&(st_action_info.st_tv), NULL);

    st_action_info.f_callback = f_ptr;
    st_action_info.p_user_info = NULL;
    st_action_info.uw_src_moudle = TRANS_MOUDLE_AGENT;
    
    uw_ret = trans_action_add(&st_action_info, len, p_action_msg);
    
    if (TRANS_SUCCESS != uw_ret)
    {
        FLOG_ERROR("Call trans_action_add error! uw_ret = %d\r\n", uw_ret);
    
        return TRANS_FAILD;    
    }    

    return TRANS_SUCCESS;
}


/*****************************************************************************+
* Function: trans_agent_forward2_query_rrh()
* Description: 
* Parameters:
*           NONE
* Return Values:
*           TRANS_FAILD : 1
*           TRANS_SUCCESS  : 0
*
*  
*  Data:    2011-08-01
* 
+*****************************************************************************/
u_int32_t trans_agent_forward2_query_rrh(void *p_msg_info,
                           u_int8_t  uc_num,
                           void    *p_type)
{
    u_int32_t uw_ret = 0;
    struct trans_send_query_to_rrh st_query_rrh;
    
    if ((NULL == p_msg_info) ||(NULL == p_type))
    {
        FLOG_ERROR("NULL PTR! \r\n");
        return TRANS_FAILD;
    }  
    
    st_query_rrh.f_callback = trans_agent_query_rrh_func;

    /*RRU Parameters type : 2Bytes for one parameters*/
    st_query_rrh.us_param_num = uc_num;
    st_query_rrh.uw_timeout = 10;
    st_query_rrh.p_param_type = p_type;
    st_query_rrh.p_info = p_msg_info;

    uw_ret = trans_send_agent_msg(TRANS_SEND_TO_RRH_QUERY, &st_query_rrh);
    if (TRANS_SUCCESS != uw_ret)
    {
        FLOG_ERROR("Call trans_send_agent_msg error! uw_ret = %d\r\n", uw_ret);
    
        return TRANS_FAILD;    
    } 

    return TRANS_SUCCESS;    
}


/*****************************************************************************+
* Function: trans_agent_forward2_cfg_rrh()
* Description: 
* Parameters:
*           NONE
* Return Values:
*           TRANS_FAILD : 1
*           TRANS_SUCCESS  : 0
*
*  
*  Data:    2011-08-01
* 
+*****************************************************************************/
u_int32_t trans_agent_forward2_cfg_rrh(void *p_msg_info,
                           u_int8_t  uc_num,
                           void    *p_type,
                           void    *p_value)
{
    u_int32_t uw_ret = 0;
    struct trans_send_cfg_to_rrh st_cfg_rrh;
    
    if ((NULL == p_msg_info) ||(NULL == p_type) ||(NULL == p_value))
    {
        FLOG_ERROR("NULL PTR! \r\n");
        return TRANS_FAILD;
    }  
    
    st_cfg_rrh.f_callback = trans_agent_cfg_rrh_func;

    /*RRU Parameters type : 2Bytes for one parameters*/
    st_cfg_rrh.us_param_num = uc_num;
    st_cfg_rrh.uw_timeout = 10;
    st_cfg_rrh.p_param_type = p_type;
    st_cfg_rrh.p_param_value = p_value;
    st_cfg_rrh.p_info = p_msg_info;

    uw_ret = trans_send_agent_msg(TRANS_SEND_TO_RRH_CFG, &st_cfg_rrh);
    if (TRANS_SUCCESS != uw_ret)
    {
        FLOG_ERROR("Call trans_send_agent_msg error! uw_ret = %d\r\n", uw_ret);
    
        return TRANS_FAILD;    
    } 

    return TRANS_SUCCESS;    
}

/*****************************************************************************+
* Function: trans_agent_rev_query_bs()
* Description: 
* Parameters:
*           NONE
* Return Values:
*           TRANS_FAILD : 1
*           TRANS_SUCCESS  : 0
*
*  
*  Data:    2011-08-01
* 
+*****************************************************************************/
u_int32_t trans_agent_rev_query_bs(struct trans_agent_metric_info *p_metric_info,
                           size_t len,                           
                           u_int8_t * p_send_msg)
{
    #ifdef TRANS_BS_COMPILE
    u_int32_t uw_ret = 0;
    fun_callback f_ptr = NULL;

    f_ptr = trans_wireless_action_bs_metric;
    
    uw_ret = trans_agent_forward2_action(f_ptr, SIZEOF_TRANS_AGENT_METRIC_INFO, p_metric_info);
    if (TRANS_SUCCESS != uw_ret)
    {
        FLOG_ERROR("Call trans_agent_forward2_action error! uw_ret = %d\r\n", uw_ret);
    
        return TRANS_FAILD;    
    } 
    #else
    
    p_metric_info= p_metric_info;
    
    #endif
    
    len = len;
    p_send_msg = p_send_msg;    
    
    return TRANS_SUCCESS;   
}

/*****************************************************************************+
* Function: trans_agent_rev_query_ms()
* Description: 
* Parameters:
*           NONE
* Return Values:
*           TRANS_FAILD : 1
*           TRANS_SUCCESS  : 0
*
*  
*  Data:    2011-08-01
* 
+*****************************************************************************/
u_int32_t trans_agent_rev_query_ms(struct trans_agent_metric_info *p_metric_info,
                           size_t len,                           
                           u_int8_t * p_send_msg)
{
    #ifdef TRANS_MS_COMPILE
    u_int32_t uw_ret = 0;
    fun_callback f_ptr = NULL;

    f_ptr = trans_wireless_action_ms_metric;
    
    uw_ret = trans_agent_forward2_action(f_ptr, SIZEOF_TRANS_AGENT_METRIC_INFO, p_metric_info);
    if (TRANS_SUCCESS != uw_ret)
    {
        FLOG_ERROR("Call trans_agent_forward2_action error! uw_ret = %d\r\n", uw_ret);
    
        return TRANS_FAILD;    
    } 
    #else
    
    p_metric_info = p_metric_info;
    
    #endif

    len = len;
    p_send_msg = p_send_msg;    
    
    return TRANS_SUCCESS;   
}

/*****************************************************************************+
* Function: trans_agent_rev_query_rrh()
* Description: 
* Parameters:
*           NONE
* Return Values:
*           TRANS_FAILD : 1
*           TRANS_SUCCESS  : 0
*
*  
*  Data:    2011-08-01
* 
+*****************************************************************************/
u_int32_t trans_agent_rev_query_rrh(struct trans_agent_metric_info *p_metric_info,
                           size_t len,
                           u_int8_t * p_send_msg)
{
    u_int32_t uw_ret = 0;
    u_int8_t uc_num = 0;
    u_int8_t  uc_index = 0;
    u_int8_t * p_metric_name = NULL;
    u_int16_t  a_rrh_type[TRANS_AGENT_METRIC_MAX_NUM] = {0};

    struct trans_agent_metric_info *p_msg_info = NULL;
    
    len =  len;
        
    if ((NULL == p_metric_info) ||(NULL == p_send_msg))
    {
        FLOG_ERROR("NULL PTR! \r\n");
        return TRANS_FAILD;
    }  
    
    uc_num = p_metric_info->uc_metric_num;

    for (uc_index = 0; uc_index < uc_num; uc_index++)
    {
        p_metric_name = p_metric_info->a_metrics[uc_index];
            
        uw_ret = trans_agent_get_rrh_id((char *)p_metric_name, &(a_rrh_type[uc_index]));
        if (TRANS_SUCCESS != uw_ret)
        {
            FLOG_ERROR("Call trans_agent_get_rrh_id error! uw_ret = %d\r\n", uw_ret);
        
            return TRANS_FAILD;    
        } 
    }

    /*When rrh message back or rrh message timeout---It could be free, not free now*/
    /*free it in trans_agent_build_metric_msg()*/
    p_msg_info = (struct trans_agent_metric_info *)malloc(SIZEOF_TRANS_AGENT_METRIC_INFO);
    if (NULL == p_msg_info)
    {
        FLOG_ERROR("malloc p_msg_info error! \r\n");
        return TRANS_FAILD;   
    }  
    
    memcpy(p_msg_info, p_metric_info, SIZEOF_TRANS_AGENT_METRIC_INFO);
        
    uw_ret = trans_agent_forward2_query_rrh(p_msg_info, uc_num, a_rrh_type);
    if (TRANS_SUCCESS != uw_ret)
    {
        FLOG_ERROR("Call trans_send_agent_msg error! uw_ret = %d\r\n", uw_ret);
    
        free(p_msg_info);
        return TRANS_FAILD;    
    } 
    
    len = len;
    p_send_msg = p_send_msg;
    
    return TRANS_SUCCESS;  
}

/*****************************************************************************+
* Function: trans_agent_rev_metric_msg()
* Description: 
* Parameters:
*           NONE
* Return Values:
*           TRANS_FAILD : 1
*           TRANS_SUCCESS  : 0
*
*  
*  Data:    2011-08-01
* 
+*****************************************************************************/
u_int32_t trans_agent_rev_metric_msg(json_t *msg, u_int8_t  *p_send_msg, 
                                    struct trans_thread_info *p_thread_info)
{
    u_int32_t uw_ret = 0;
    struct trans_agent_metric_info st_metric_info;
        
    //FLOG_DEBUG("Enter \r\n");

    if ((NULL == msg) ||(NULL == p_send_msg) || (NULL == p_thread_info))
    {
        FLOG_ERROR("NULL PTR! \r\n");
        return TRANS_FAILD;
    }  
    
    uw_ret = trans_agent_get_metric_info(msg, &st_metric_info);
    if (TRANS_SUCCESS != uw_ret)
    {
        FLOG_ERROR("Call trans_agent_get_metric_info error! uw_ret = %d\r\n", uw_ret);
    
        return TRANS_FAILD;    
    }  

    strcpy((char *)(st_metric_info.a_action), "getMetricValues");

    #ifdef TRANS_RRH_COMPILE
    if (0 == strcmp ("rrh", (char *)st_metric_info.a_element_id))
    {
        if ( (0 == TRANS_SEND_METRIC_TO_RRH_FAKE) && (g_enable_metric == 1) )
        {
            uw_ret = trans_agent_rev_query_rrh(&st_metric_info, 4096, p_send_msg);
            
            if (TRANS_SUCCESS != uw_ret)
            {
                FLOG_ERROR("Call trans_agent_rev_query_rrh error! uw_ret = %d\r\n", uw_ret);
            
                return TRANS_FAILD;
            }
        }
        else
        {
            /*********Test**********/
            struct trans_send_msg_to_agent st_msg_info;
            u_int8_t  *p_param = (u_int8_t *)malloc((st_metric_info.uc_metric_num * 4));

            memset((u_int8_t*)p_param, 0, st_metric_info.uc_metric_num * 4); 
            
            st_msg_info.f_callback = NULL;
            st_msg_info.uc_ack_flag = 0;
            st_msg_info.uc_block_flag = 0;
            st_msg_info.p_reqs_msg = (struct trans_agent_metric_info *)malloc (sizeof (struct trans_agent_metric_info));
            memcpy(st_msg_info.p_reqs_msg, &st_metric_info, (sizeof (struct trans_agent_metric_info)));
 
            st_msg_info.uw_resp_len = 4;
            st_msg_info.p_resp_msg = p_param;
            
            uw_ret = trans_agent_send_msg(&st_msg_info, 4096, p_send_msg);
            if (TRANS_SUCCESS != uw_ret)
            {
                FLOG_ERROR("Call trans_agent_send_msg error! uw_ret = %d\r\n", uw_ret);
            
                return TRANS_FAILD;
            }
            
            /*********Test**********/

        }    
        
        return TRANS_SUCCESS;
    }
    #endif
    #ifdef TRANS_BS_COMPILE
    if (0 == strcmp ("bs", (char *)st_metric_info.a_element_id))
    {
        uw_ret = trans_agent_rev_query_bs(&st_metric_info, 4096, p_send_msg);
        
        if (TRANS_SUCCESS != uw_ret)
        {
            FLOG_ERROR("Call trans_agent_rev_query_bs error! uw_ret = %d\r\n", uw_ret);

            return TRANS_FAILD;
        }

        return TRANS_SUCCESS;
    }
    #endif
    #ifdef TRANS_MS_COMPILE
    if (0 == strcmp ("cpe", (char *)st_metric_info.a_element_id))
    {
        uw_ret = trans_agent_rev_query_ms(&st_metric_info, 4096, p_send_msg);
        
        if (TRANS_SUCCESS != uw_ret)
        {
            FLOG_ERROR("Call trans_agent_rev_query_ms error! uw_ret = %d\r\n", uw_ret);

            return TRANS_FAILD;
        }

        return TRANS_SUCCESS;
    }
    #endif

    FLOG_ERROR("Source module %s error \r\n", st_metric_info.a_element_id);

    #if 0
    switch (st_metric_info.w_metric_id&0xff00)
    {
        #ifdef TRANS_RRH_COMPILE
        case TRANS_AGENT_RRH_METRIC_ID: /* RRH METRIC */
            
            FLOG_DEBUG("REV RRH METRIC. \r\n");
            
            uw_ret = trans_agent_cfg_forward2_rrh(&st_metric_info, 
                                    4096,
                                    p_send_msg);
            //uw_ret = trans_rrh_forward_agent_metric(p_send_msg, &st_metric_info);
            if (TRANS_SUCCESS != uw_ret)
            {
                FLOG_ERROR("Call trans_rrh_forward_agent_metric error! uw_ret = %d\r\n", uw_ret);
            
                return TRANS_FAILD;    
            }   
            
            break; 
        #endif
        
        #ifdef TRANS_BS_COMPILE
        case TRANS_AGENT_BS_METRIC_ID: /* BS METRIC */
        
            FLOG_DEBUG("REV BS METRIC. \r\n");
            
            gettimeofday(&(st_action_info.st_tv), NULL);
            st_action_info.uc_action = TRANS_WIRELESS_ACTION_GET_BS_METRIC;
            st_action_info.uw_src_moudle = TRANS_MOUDLE_AGENT;
            memcpy(&(st_action_info.u_action_info.st_agent_metric), &(st_metric_info), 
                                        SIZEOF_TRANS_AGENT_METRIC_INFO);

            uw_ret = trans_action_add(&st_action_info);
            
            if (TRANS_SUCCESS != uw_ret)
            {
                FLOG_ERROR("Call trans_action_add for BS error! uw_ret = %d\r\n", uw_ret);
            
                return TRANS_FAILD;    
            }  

            break;  
        #endif

        #ifdef TRANS_MS_COMPILE
        case TRANS_AGENT_MS_METRIC_ID: /* MS METRIC   */
          
            FLOG_DEBUG("REV MS METRIC. \r\n");

            gettimeofday(&(st_action_info.st_tv), NULL);
            st_action_info.uc_action = TRANS_WIRELESS_ACTION_GET_MS_METRIC;
            st_action_info.uw_src_moudle = TRANS_MOUDLE_AGENT;
            memcpy(&(st_action_info.u_action_info.st_agent_metric), &(st_metric_info), 
                                        SIZEOF_TRANS_AGENT_METRIC_INFO);
            
            uw_ret = trans_action_add(&st_action_info);

            if (TRANS_SUCCESS != uw_ret)
            {
                FLOG_ERROR("Call trans_action_add for MS error! uw_ret = %d\r\n", uw_ret);
            
                return TRANS_FAILD;    
            }   

            break;      
        #endif

        default:

            FLOG_ERROR("Rev unknow metric_id! w_metric_id = %d\r\n", st_metric_info.w_metric_id);

            return TRANS_FAILD;

    }
    #endif
   
    //FLOG_DEBUG("Exit \r\n");
    
    return TRANS_FAILD;
}

/*****************************************************************************+
* Function: trans_agent_get_spectrum_info()
* Description: 
* Parameters:
*           NONE
* Return Values:
*           TRANS_FAILD : 1
*           TRANS_SUCCESS  : 0
*
*  
*  Data:    2011-09-21
* 
+*****************************************************************************/
u_int32_t trans_agent_get_spectrum_info(json_t *msg,  
                    struct trans_agent_spectrum_resp_info *p_sectrum_resp_info)
{
    //int32_t w_ret = 0;
    //const char * p_tr = NULL;
    u_int32_t uw_index = 0;
    //u_int32_t uw_ret = 0;
    
    //int32_t w_source_id =0;
    float f_freq = 0;
    int   a_interref[TRANS_AGENT_INTERREF_NUM] = {0};     /*1* 21 int*/
    //char a_interref_str[TRANS_AGENT_INTERREF_NUM + 1] = {0};  /*1* 22 int  -> 22 char*/
    int w_error_code = 0;
  
    FLOG_DEBUG("Enter \r\n");
    
    if ((NULL == msg) ||(NULL == p_sectrum_resp_info))
    {
        FLOG_ERROR("NULL PTR! \r\n");
        return TRANS_FAILD;
    }  
    
    #if 0
    /*Get SOURCE*/
    json_t *source = json_object_get(msg, "SOURCE"); // get SOURCE
    if (NULL == source)
    {
        FLOG_ERROR("Call json_object_get for SOURCE  error! NULL PTR  .\r\n");
        
        return TRANS_FAILD;
    }
    
    p_tr = json_string_value(source);
    if (NULL == p_tr)
    {
        FLOG_ERROR("Call json_string_value for SOURCE  error! NULL PTR  .\r\n");

        return TRANS_FAILD;
    }
    
    w_source_id = atoi((char *)(p_tr)); 
    #endif
    
    //FLOG_DEBUG("w_source_id =0x%x, source: %s. \r\n", w_source_id, p_tr);    
    
    json_t *body = json_object_get(msg, "BODY"); //get BODY elements
    if (NULL == body)
    {
        FLOG_ERROR("Call json_object_get for BODY  error! NULL PTR  .\r\n");

        return TRANS_FAILD;
    }
    
    /*ERRORCODE*/
    json_t *errorcode= json_object_get(body, "ERRORCODE");// get metric_id
    if (NULL == errorcode)
    {
        FLOG_ERROR("Call json_object_get for errorcode  error! NULL PTR  .\r\n");
        
        return TRANS_FAILD;
    }
    
    w_error_code = json_integer_value(errorcode);

    p_sectrum_resp_info->w_error_code = w_error_code;
    
    /*The result is faild*/
    if (0 != w_error_code)
    {
        FLOG_ERROR("Spectrum errorcode = %d error!.\r\n", w_error_code);
        
        return TRANS_SUCCESS;
    }

    json_t *freq= json_object_get(body, "FREQ");// get metric_id
    if (NULL == freq)
    {
        FLOG_ERROR("Call json_object_get for freq  error! NULL PTR  .\r\n");
        
        return TRANS_FAILD;
    }
    
    f_freq = (float)json_number_value(freq);
    if (0.0 == f_freq)
    {
        FLOG_ERROR("Call json_number_value for freq error! NULL PTR  .\r\n");
        
        return TRANS_FAILD;
    }
    
    p_sectrum_resp_info->w_f_freq = (int)(f_freq*1000);
    
    FLOG_INFO("Rev DEFAULT_WORKING_FREQ %d\n", p_sectrum_resp_info->w_f_freq);
    
    json_t *array= json_object_get(body, "SENSING");// get metric_id
    if (NULL == array)
    {
        FLOG_ERROR("Call json_object_get for array  error! NULL PTR  .\r\n");
        
        return TRANS_FAILD;
    }
    
    for (uw_index = 0; uw_index < TRANS_AGENT_INTERREF_NUM; uw_index++)
    {
        json_t *value = json_array_get(array, uw_index);
        if (NULL == value)
        {
            FLOG_ERROR("Call json_array_get for value error! NULL PTR  .\r\n");
            
            return TRANS_FAILD;
        }        
    
        /*It could be 0*/
        a_interref[uw_index] = json_integer_value(value);
    
        //FLOG_DEBUG("a_interref[%d] = %d. \r\n", uw_index, a_interref[uw_index]);
    
        sprintf((char *)p_sectrum_resp_info->a_interref_str, "%s%d", 
                    p_sectrum_resp_info->a_interref_str, a_interref[uw_index]);
    }

    FLOG_INFO("Rev. INTERFERENCE_ACTIVE %s\n", p_sectrum_resp_info->a_interref_str);
  
    return TRANS_SUCCESS;
}

/*****************************************************************************+
* Function: trans_agent_rev_spectrum_msg()
* Description: Rev Spectrum message 
* Parameters:
*           NONE
* Return Values:
*           TRANS_FAILD : 1
*           TRANS_SUCCESS  : 0
*
*  
*  Data:    2011-07-27
* 
+*****************************************************************************/
u_int32_t trans_agent_rev_spectrum_msg(json_t *msg, u_int8_t  *p_send_msg, 
                                    struct trans_thread_info *p_thread_info)
{
    //int32_t w_ret = 0;
    //const char * p_tr = NULL;
    u_int32_t uw_ret = 0;
    struct trans_agent_spectrum_resp_info st_sectrum_resp;
        
    struct trans_en_queue_msg   st_en_quene;
    u_int8_t  uc_flag = TRANS_SET_SENSING_RESULT_FAKE;
    
    memset((u_int8_t*)&st_en_quene, 0, SIZEOF_TRANS_EN_QUENE_MSG);

    memset((u_int8_t*)&st_sectrum_resp, 0, SIZEOF_TRANS_AGENT_SPECTRUM_RESP_INFO);
    
    FLOG_DEBUG("Enter \r\n");
    
    if ((NULL == msg) ||(NULL == p_send_msg) || (NULL == p_thread_info))
    {
        FLOG_ERROR("NULL PTR! \r\n");
        return TRANS_FAILD;
    }  

    /*Delete Timer*/
    //struct trans_timer_msg_info st_timer_info;        
    u_int8_t uc_find_flag = 0;
    u_int16_t  us_no = 0xffff;
    
    uw_ret = trans_timer_find_by_serial_num(us_no, 
                p_send_msg, 
                &uc_find_flag);
    
    if (TRANS_SUCCESS != uw_ret)
    {
        FLOG_ERROR("Call trans_timer_find_by_serial_num error! uw_ret = %d\r\n", uw_ret);    
        return TRANS_FAILD;    
    }

    /*if timeout ,discard message:  do nothing*/
    if (1 != uc_find_flag)
    {
        FLOG_ERROR("Time out error! uc_find_flag = %d\r\n", uc_find_flag);
    
        /*enum trans_ack_flag_enum*/
        return TRANS_FAILD;    
    }

    uw_ret = trans_agent_get_spectrum_info(msg, &st_sectrum_resp);
    /*Error*/
    if (TRANS_SUCCESS != uw_ret)
    {
        FLOG_ERROR("Call trans_agent_get_spectrum_info error! uw_ret = %d\r\n", uw_ret);
    
        st_en_quene.uc_result = TRANS_FAILD;
    }
    
    if (0 != st_sectrum_resp.w_error_code)
    {
        FLOG_ERROR("Send spectrum alert to Agent.\r\n");
        
        st_en_quene.uc_result = TRANS_FAILD;

        /*Send Alert to Agent*/
        struct trans_agent_alert_info        st_alert_info;
        
        st_alert_info.us_alarm_id = (u_int16_t)(0x0300 + st_sectrum_resp.w_error_code);
        st_alert_info.w_alarm_value = 0;
        
        struct trans_send_msg_to_agent st_msg_info;
        st_msg_info.f_callback = NULL;
        st_msg_info.uc_block_flag = TRANS_QUENE_NO_BLOCK;
        st_msg_info.uc_ack_flag = TRANS_ACK_FLAG_OK;
        st_msg_info.uw_resp_len = SIZEOF_TRANS_AGENT_ALERT_INFO;
        st_msg_info.p_resp_msg = &st_alert_info;
        st_msg_info.p_reqs_msg = "alert";
        
        uw_ret = trans_send_wireless_msg(TRANS_SEND_TO_AGENT, &st_msg_info);
        /*Error*/
        if (TRANS_SUCCESS != uw_ret)
        {
            FLOG_ERROR("Call trans_send_wireless_msg error! uw_ret = %d\r\n", uw_ret);
        
            //st_en_quene.uc_result = TRANS_SUCCESS;
        }
    }
    /*The result is sucessful*/
    else
    {
        //#if 0
        if (0 == uc_flag)
        {
            uw_ret = set_global_param("DEFAULT_WORKING_FREQ", (void *)&(st_sectrum_resp.w_f_freq));
            if (0 != uw_ret)
            {
                FLOG_ERROR("Call set_global_param for FREQ error");
                
                st_en_quene.uc_result = TRANS_FAILD;
            
            }
            
            //FLOG_DEBUG("a_interref_str = %s. \r\n", a_interref_str);
            uw_ret = set_global_param("INTERFERENCE_ACTIVE", (void *)st_sectrum_resp.a_interref_str);
            
            FLOG_INFO("Rev. INTERFERENCE_ACTIVE %s\n", st_sectrum_resp.a_interref_str);
            
            if (0 != uw_ret)
            {
                FLOG_ERROR("Call set_global_param for INTERFERENCE error");
                
                st_en_quene.uc_result = TRANS_FAILD;
            }
        }

        //#endif
    }    

    //struct trans_en_queue_msg   st_en_quene;
    /*2 --CRC*/
    st_en_quene.uw_src_moudle = TRANS_MOUDLE_AGENT;
    //st_en_quene.uc_result = TRANS_SUCCESS;
    st_en_quene.uw_len = 0;
    
    uw_ret = trans_msg_en_quene((void *)msg, &st_en_quene);
    
    if (TRANS_SUCCESS != uw_ret)
    {
        FLOG_ERROR("Call trans_msg_en_quene error! uw_ret = %d\r\n", uw_ret);
        return TRANS_FAILD;
    }

    //uw_ret = st_en_quene.uc_result;

    FLOG_DEBUG("Exit \r\n");

    return TRANS_SUCCESS;
}

//#ifdef TRANS_MS_COMPILE

/*****************************************************************************+
* Function: trans_agent_rev_idupdate_msg()
* Description: Rev Spectrum message 
* Parameters:
*           NONE
* Return Values:
*           TRANS_FAILD : 1
*           TRANS_SUCCESS  : 0
*
*  
*  Data:    2011-07-27
* 
+*****************************************************************************/
u_int32_t trans_agent_rev_idupdate_msg(json_t *msg, u_int8_t  *p_send_msg, 
                                    struct trans_thread_info *p_thread_info)
{
    const char * p_tr = NULL;
    u_int32_t uw_ret = 0;

    if ((NULL == msg) ||(NULL == p_send_msg) || (NULL == p_thread_info))
    {
        FLOG_ERROR("NULL PTR! \r\n");
        return TRANS_FAILD;
    }  

    FLOG_DEBUG("Enter \r\n");

    json_t *body = json_object_get(msg, "BODY"); //get BODY elements
    if (NULL == body)
    {
        FLOG_ERROR("Call json_object_get for BODY  error! NULL PTR  .\r\n");
    
        return TRANS_FAILD;
    }
    
    json_t *dev_id = json_object_get(body, "DEVID");// get DEVID
    if (NULL == dev_id)
    {
        FLOG_ERROR("Call json_object_get for dev_id  error! NULL PTR  .\r\n");
        
        return TRANS_FAILD;
    }
    
    p_tr = json_string_value(dev_id);
    if (NULL == p_tr)
    {
        FLOG_ERROR("Call json_string_value for dev_id  error! NULL PTR  .\r\n");
        
        return TRANS_FAILD;
    }

    if (TRANS_AGENT_MODULE_ID_SIZE < strlen(p_tr))
    {
        FLOG_ERROR("dev_id  length %d error!\r\n", strlen(p_tr));
        
        return TRANS_FAILD;
    }
    else
    {
        strcpy((char *)(g_trans_agent_config_info.a_device_id), p_tr);
    } 

    FLOG_WARNING("Update Devid %s\r\n", g_trans_agent_config_info.a_device_id);

    /*Send State Change message to Agent*/
    uw_ret = trans_agent_send_state_msg(p_send_msg, g_trans_agent_socket.w_agent_socket);   
    /*Error*/
    if (TRANS_SUCCESS != uw_ret)
    {

        FLOG_ERROR("Call trans_agent_send_state_msg error! uw_ret = %d\r\n", uw_ret);   
        return uw_ret;
    } 

    #if 0
    /*enqueue*/
    struct trans_en_queue_msg   st_en_quene;

    memset((u_int8_t*)&st_en_quene, 0, SIZEOF_TRANS_EN_QUENE_MSG);
    
    st_en_quene.uc_result = TRANS_SUCCESS;
    st_en_quene.uw_len = 0;
    st_en_quene.uw_src_moudle = TRANS_MOUDLE_AGENT;
    
    uw_ret = trans_msg_en_quene((void *)msg, &st_en_quene);
    
    if (TRANS_SUCCESS != uw_ret)
    {
        FLOG_ERROR("Call trans_msg_en_quene error! uw_ret = %d\r\n", uw_ret);
    
        return TRANS_FAILD;    
    }
    #endif
    
    FLOG_DEBUG("Exit \r\n");

    return TRANS_SUCCESS;
}

//#endif

/*****************************************************************************+
* Function: trans_agent_rev_msg_process()
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
u_int32_t trans_agent_rev_msg_process(u_int8_t *p_rev_msg, u_int8_t *p_send_msg, 
                           struct trans_thread_info *p_thread_info)
{
    struct trans_agent_msg *p_agent_rev_msg = NULL;
    const char * p_action = NULL;
    const char * p_dest = NULL;
    json_error_t error;    
    //u_int32_t  uw_json_len = 0;
    u_int32_t uw_ret = 0;

    FLOG_DEBUG("Enter \r\n");

    if ((NULL == p_rev_msg) ||(NULL == p_send_msg) || (NULL == p_thread_info))
    {
        FLOG_ERROR("NULL PTR! \r\n");
        return TRANS_FAILD;
    }  
     
    p_agent_rev_msg = (struct trans_agent_msg *)p_rev_msg;

    if (0 != strncmp ("MESS", (char *)p_agent_rev_msg->a_magic, (int)4))
    {
        FLOG_ERROR("Magic error! a_magic = %s .\r\n", p_agent_rev_msg->a_magic);
        
        return TRANS_FAILD;
    }

    /*Length check ????*/
    
    FLOG_DEBUG("json: %s \r\n", p_rev_msg);
    
    //uw_json_len = atoi((char *)(p_agent_rev_msg->a_len)); 
    //FLOG_DEBUG("json len : %d \r\n", uw_json_len);
   
    json_t *msg = json_loads((char *)(p_rev_msg+SIZEOF_TRANS_AGENT_MSG), &error); //decodes the json string and returns the object it contains
    if (NULL == msg)
    {
        FLOG_ERROR("Call json_loads error! NULL PTR  .\r\n");
        
        return TRANS_FAILD;
    }
    
    /*DEST  check*/
    json_t *dest = json_object_get(msg, "DEST"); // get DEST
    if (NULL == dest)
    {
        FLOG_ERROR("Call json_object_get for DEST  error! NULL PTR  .\r\n");
        
        return TRANS_FAILD;
    }
    
    p_dest = json_string_value(dest);
    if (0 != strcmp ((char *)g_trans_agent_config_info.a_device_id, (char *)p_dest))
    {
        FLOG_ERROR("Device_id error! device_id = %s .\r\n", p_dest);
        
        return TRANS_FAILD;
    }
    
    json_t *body = json_object_get(msg, "BODY"); //get BODY elements
    if (NULL == body)
    {
        FLOG_ERROR("Call json_object_get for BODY  error! NULL PTR  .\r\n");
        
        return TRANS_FAILD;
    }
    
    json_t *action = json_object_get(body, "ACTION"); // get action    
    if (NULL == action)
    {
        FLOG_DEBUG("ACTION action\r\n");  
        
        //json_t *action = json_object_get(body, "action"); // get ACTION  
        action = json_object_get(body, "action"); // get ACTION  
        if (NULL == action)
        {
            FLOG_DEBUG("Call json_object_get for action/ACTION  error! NULL PTR  .\r\n");
            
            return TRANS_FAILD;
        }
    }

    p_action = json_string_value(action);
    if (NULL == p_action)
    {
        FLOG_DEBUG("Call json_string_value for action/ACTION  error! NULL PTR  .\r\n");
        
        return TRANS_FAILD;
    }

    FLOG_DEBUG("action: %s \r\n", p_action);    
    
    if (0 == strcmp ("getMetricValues", (char *)p_action))
    {
        uw_ret = trans_agent_rev_metric_msg(msg, p_send_msg, p_thread_info);

        if (TRANS_SUCCESS != uw_ret)
        {
            FLOG_ERROR("Call trans_agent_rev_metric_msg error! uw_ret = %d\r\n", uw_ret);
        }
    }
    else if (0 == strcmp ("sendSpectrumResult", (char *)p_action))
    {
        uw_ret = trans_agent_rev_spectrum_msg(msg, p_send_msg, p_thread_info);
        
        if (TRANS_SUCCESS != uw_ret)
        {
            FLOG_ERROR("Call trans_agent_rev_spectrum_msg error! uw_ret = %d\r\n", uw_ret);
        }
    }
    //#ifdef TRANS_MS_COMPILE
    else if (0 == strcmp ("idUpdate", (char *)p_action))
    {
        uw_ret = trans_agent_rev_idupdate_msg(msg, p_send_msg, p_thread_info);
        
        if (TRANS_SUCCESS != uw_ret)
        {
            FLOG_ERROR("Call trans_agent_rev_idupdate_msg error! uw_ret = %d\r\n", uw_ret);
        }
    }
    //#endif
    else
    {
        FLOG_ERROR("action/ACTION error! action = %s .\r\n", p_action);
        
        uw_ret = TRANS_FAILD;
    }

    json_decref(msg); 

    FLOG_DEBUG("Exit uw_ret = %d,\r\n", uw_ret);
    
    return uw_ret;

}


/*****************************************************************************+
* Function: trans_agent_rev_msg()
* Description: Revice Message From Socket
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
u_int32_t trans_agent_rev_msg(u_int8_t *p_rev_msg, int32_t w_agent_socket)
{
    int32_t w_json_len = 0;
    int32_t w_rev_len1 = 0, w_rev_len2 = 0;
    int32_t w_rev_len = 0;    
    int w_len_tmp = 0;

    /*First rev the head including type(4) and len(4) : 8 Bytes*/
    w_len_tmp = SIZEOF_TRANS_AGENT_MSG;
    
    /*Rev JSON msg*/
    while (w_len_tmp)
    {
        /*Then rev a totle message except the len*/
        w_rev_len1 = recv(w_agent_socket, 
                                p_rev_msg + (SIZEOF_TRANS_AGENT_MSG - w_len_tmp),  
                                w_len_tmp, 
                                0);
        /*Error*/
        if (w_rev_len1 <= 0)
        {
            FLOG_ERROR("Rev complete w_rev_len1 =%d. \r\n", w_rev_len1);
            return TRANS_FAILD;
        }
    
        w_len_tmp = w_len_tmp - w_rev_len1;    
        w_rev_len = w_rev_len + w_rev_len1;
    }
    
    if (SIZEOF_TRANS_AGENT_MSG != w_rev_len)
    {
        FLOG_ERROR("Receive Agent Message Header Length error! header_len  = %d, rev_len  = %d\r\n", SIZEOF_TRANS_AGENT_MSG, w_rev_len);
        return TRANS_FAILD;
    }

    #if 0
    w_rev_len1 = recv(w_agent_socket, 
                            p_rev_msg, 
                            SIZEOF_TRANS_AGENT_MSG, 
                            0);
    
    if (w_rev_len1 <= 0)
    {
        FLOG_ERROR("Rev complete w_rev_len1 =%d. \r\n", w_rev_len1);
        return TRANS_FAILD;
    }
    #endif

    /*Get the len--offset the type 4Bytes ----  u_int8_t   a_magic[4]*/
    w_json_len = atoi((char *)(p_rev_msg + 4));
    FLOG_DEBUG("Rev json len =%d . \r\n", w_json_len);
    
    /*?????len check  ----Not sure max len?????*/
    if ((w_json_len + SIZEOF_TRANS_AGENT_MSG> TRANS_REV_MSG_MAX_LEN)
        || (0 == w_json_len))
    {
        FLOG_ERROR("recv agent msg json length error! w_json_len = %d\r\n", w_json_len);
        return TRANS_FAILD;
    }
    
    w_rev_len = 0;
    w_len_tmp = 0;
    
    /*Rev JSON msg*/
    w_len_tmp = w_json_len;
    
    /*Rev JSON msg*/
    while (w_len_tmp)
    {
        /*Then rev a totle message except the len*/
        w_rev_len2 = recv(w_agent_socket, 
                                p_rev_msg + (w_json_len - w_len_tmp) + SIZEOF_TRANS_AGENT_MSG,  
                                w_len_tmp, 
                                0);
        /*Error*/
        if (w_rev_len2 <= 0)
        {
            FLOG_ERROR("Receivev Agent message error! w_rev_len2 = %d\r\n", w_rev_len2);
            return TRANS_FAILD;
        }
    
        w_len_tmp = w_len_tmp - w_rev_len2;    
        w_rev_len = w_rev_len + w_rev_len2;
    }

    if (w_json_len != w_rev_len)
    {
        FLOG_ERROR("Receive Agent Message Length error! json_len  = %d, rev_len  = %d\r\n", w_json_len, w_rev_len);
        return TRANS_FAILD;
    }

    #if 0
    w_rev_len2 = recv(w_agent_socket, 
                                p_rev_msg + SIZEOF_TRANS_AGENT_MSG,   
                                w_json_len, 
                                0);
    /*Error*/
    if (w_rev_len2 <= 0)
    {
    
        #if 0
        /*如果异常，该进入什么处理流程啊*/
        //close(sock);        
    
        /*重复关闭有没有问题*/
        close (g_rrh_client_socket.w_sockFd);
        close (g_rrh_server_socket.w_sockFd);
        #endif
        FLOG_ERROR("recv agent msg error! w_rev_len2 = %d\r\n", w_rev_len2);
        return TRANS_FAILD;
    }
    #endif

    FLOG_DEBUG("Rev Agent Msg OK len = %d. \r\n", w_rev_len+w_rev_len1);

    trans_debug_msg_print(p_rev_msg, 40, g_trans_debug_agent);
    
    return TRANS_SUCCESS;
}

/*****************************************************************************+
* Function: trans_agent_tcp_socket()
* Description: 
* Parameters:
*           NONE
* Return Values:
*           ADAPT_FAILD : 1
*           ADAPT_SUCCESS  : 0

*  
*  Data:    2011-04-23
* 
+*****************************************************************************/
u_int32_t trans_agent_tcp_socket()
{
    int32_t w_agent_socket = 1;

    int32_t w_ret = 0;
    struct sockaddr_in st_peer_addr; 
    //struct sockaddr_in st_client_addr; 
    socklen_t sin_size = sizeof(struct sockaddr_in);    

    int32_t w_sendbuflen = 0, w_recvbuflen = 0, w_reuseORnot = 0;

    FLOG_DEBUG("Enter \r\n");
   
    /* Create an IPv4 Internet Socket */
    w_ret = w_agent_socket = socket (AF_INET, SOCK_STREAM, 0);
    
    if (w_ret < 0)
    {    
        FLOG_ERROR("Creat socket error! w_ret = %d\r\n", w_ret);
        return TRANS_FAILD;    
    }
    
    /*Set if reuse the adderss*/
    w_reuseORnot = 1;
    w_ret = setsockopt (w_agent_socket, SOL_SOCKET, SO_REUSEADDR, &w_reuseORnot,
            sizeof(int32_t));
    
    if (w_ret < 0)
    {
        FLOG_ERROR("Call setsockopt SO_REUSEADDR error! w_ret = %d\r\n", w_ret);

        close (w_agent_socket);
        return TRANS_FAILD;
     }
    
    
    /*Set the length of the REV buffer*/
    w_recvbuflen = TRANS_AGENT_REV_BUF_MAX_LEN;
    w_ret = setsockopt (w_agent_socket, SOL_SOCKET, SO_RCVBUF, &w_recvbuflen, sizeof(int32_t));
    
    if (w_ret < 0)
    {
        FLOG_ERROR("Call setsockopt SO_RCVBUF error! w_ret = %d\r\n", w_ret);

        close (w_agent_socket);
        return TRANS_FAILD;
    }
    
    /*Set the length of the revice buffer*/
    w_sendbuflen = TRANS_AGENT_SEND_BUF_MAX_LEN;
    w_ret = setsockopt (w_agent_socket, SOL_SOCKET, SO_SNDBUF, &w_sendbuflen, sizeof(int32_t));
    
    if (w_ret < 0)
    {
        FLOG_ERROR("Call setsockopt SO_SNDBUF error! w_ret = %d\r\n", w_ret);

        close (w_agent_socket);
        return TRANS_FAILD;    
    }
    
    /* Zero out structure */
    memset(&st_peer_addr, 0, sizeof(st_peer_addr));    
    /* Create an AF_INET address */
    st_peer_addr.sin_family = AF_INET;         
    st_peer_addr.sin_port = htons(g_trans_agent_config_info.us_agent_tcp_port);     
    //st_client_addr.sin_addr.s_addr = g_ad_agent_server_socket.uw_ipAddr; 
    //st_peer_addr.sin_addr.s_addr = inet_addr("127.0.0.1"); 
    st_peer_addr.sin_addr.s_addr = g_trans_agent_config_info.uw_agent_ip_addr; 

    /*Connect  persistent */
    w_ret = connect(w_agent_socket, (struct sockaddr * )&st_peer_addr, sin_size); 
    
    //if ((w_ret < 0) && (0 == g_trans_agent_config_info.uc_connect_num))
    if (w_ret < 0)
    {
        FLOG_ERROR("Call connect error! w_ret = %d, w_agent_socket = %d.\r\n", w_ret, w_agent_socket);

        close(w_agent_socket);
        return TRANS_FAILD;
    }

    g_trans_moudle_socket_fd[TRANS_MOUDLE_AGENT] = w_agent_socket;
    
    pthread_mutex_lock (&(g_trans_agent_socket.m_mutex));                   
    g_trans_agent_socket.w_agent_socket = w_agent_socket;                     
    pthread_mutex_unlock(&(g_trans_agent_socket.m_mutex)); 

    FLOG_DEBUG("Exist agent_socket = %d\r\n", w_agent_socket); 
    
    return w_ret;
}


/*****************************************************************************+
* Function: trans_agent_init()
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

u_int32_t trans_agent_init(struct trans_init_info *p_init_info)
{
    /*Init the Global variables */
    
    if (NULL == p_init_info)
    {
        FLOG_ERROR("NULL PTR! \r\n");
        return TRANS_FAILD;
    }  
    
    memset((u_int8_t*)&g_trans_agent_config_info, 0, sizeof(struct trans_agent_config_info));

    //g_trans_agent_config_info.uc_connect_num = TRANS_AGENT_TCP_CONNECT_NUM;
    
    g_trans_agent_config_info.us_agent_tcp_port = p_init_info->us_agent_tcp_port;
    /*HEART BEAT TIME INTERVAL  (second)*/
    g_trans_agent_config_info.uw_agent_hb_time = p_init_info->uw_agent_hb_time;
    
    #ifdef TRANS_BS_COMPILE
    g_trans_agent_config_info.uw_agent_ip_addr = p_init_info->uw_agent_ip_addr;
    
    strcpy((char *)g_trans_agent_config_info.a_device_id, (char *)p_init_info->a_agent_device_id);

    strcpy((char *)g_trans_agent_config_info.a_agent_id, (char *)p_init_info->a_agent_id);
    #endif
    
    #ifdef TRANS_MS_COMPILE   
    g_trans_agent_config_info.uw_agent_ip_addr = p_init_info->uw_agent_ip_addr;
    
    strcpy((char *)g_trans_agent_config_info.a_device_id, (char *)TRANS_AGENT_SELF_DEVICE_ID);
    
    strcpy((char *)g_trans_agent_config_info.a_agent_id, (char *)TRANS_AGENT_AGENT_DEVICE_ID);

    g_trans_agent_config_info.us_agent_tcp_port = p_init_info->us_agent_tcp_port;

    g_trans_agent_config_info.uw_agent_hb_time = TRANS_AGENT_HEARTBEAT_TIMEOUT_NUM;

    #endif

    g_trans_agent_socket.w_agent_socket = -1;    

    pthread_mutex_init (&g_trans_agent_socket.m_mutex, NULL);
    
    pthread_mutex_init (&g_trans_agent_metric_mutex, NULL);
 
    return TRANS_SUCCESS;
}

/*****************************************************************************+
* Function: trans_agent_release()
* Description: init 
* Parameters:
*           NONE
* Return Values:
*           TRANS_FAILD : 1
*           TRANS_SUCCESS  : 0

*  
*  Data:    2011-03-23
* 
+*****************************************************************************/
u_int32_t trans_agent_release()
{
    memset((u_int8_t*)&g_trans_agent_config_info, 0, sizeof(struct trans_agent_config_info));
    
    /*Close socket*/
    close(g_trans_agent_socket.w_agent_socket);

    g_trans_agent_socket.w_agent_socket = -1;    

    return TRANS_SUCCESS;
}

#endif

