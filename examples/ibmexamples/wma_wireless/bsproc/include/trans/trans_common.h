/* ----------------------------------------------------------------------------
 IBM Confidential

 IBM Wireless M2M Platform

 (C)Copyright IBM Corp. 2009, 2010, 2011

 All Rights Reserved.

 File Name: trans_common.h

 Change Activity:

 Date             Description of Change                            By
 -----------      ---------------------                            --------
 30-Nov.2011      Created                                          E Wulan

 ----------------------------------------------------------------------------
 PROLOG END TAG zYx                                                         */


#ifndef TRANS_COMMON_H_
#define TRANS_COMMON_H_


#include <sys/types.h>
#include <pthread.h>
//nclude <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <time.h>
#include <errno.h>
#include <string.h>


/*****************************************************************************+
*Macro
+*****************************************************************************/

/*********MAC ADDRESS************/

#define TRANS_DEFAULT_MAC           0xffffffffffff       
#define TRANS_AGENT_MAC               0x000000000000


#define TRANS_COMMON_SOCKFD_ERROR(sockfd, error_no,ret_code)                  \
    do    \
    {    \
        if (0 == error_no)    \
        {    \
                \
            close(sockfd);    \
            ret_code = 1;    \
        }    \
        else if (EINTR == error_no)    \
        {    \
                \
            ret_code = 2;    \
        }    \
        else if (EAGAIN == error_no)    \
        {    \
                \
            ret_code = 2;    \
        }    \
        else    \
        {    \
                \
            close(sockfd);    \
            ret_code = 1;    \
        }    \
            \
    }while(0);

#define TRANS_COMMON_SOCKFD_PRINT(sockfd, w_ret, error_no)                  \
    do    \
    {    \
        time_t   now;    \
        struct tm   *timenow;    \
            \
        time(&now);    \
        timenow = localtime(&now);    \
            \
        FLOG_ERROR("Send socket:%d recv() error! return:%d, errno:%d, errortext:'%s', time:%s",    \
                    sockfd, w_ret, error_no, strerror(error_no), asctime(timenow));    \
            \
    }while(0);


/*****************************************************************************+
*Enum
+*****************************************************************************/
enum trans_moudle_enum
{   
    TRANS_MOUDLE_LOCAL = 0x00, /*TRANS */
    TRANS_MOUDLE_AGENT,            /*AGENT */   
    TRANS_MOUDLE_RRH,                /*RRH */ 
    TRANS_MOUDLE_WIRELESS,      /*WIRELESS */
    TRANS_MOUDLE_BS,                   /*BS */
    TRANS_MOUDLE_MS,                  /*MS */
    TRANS_MOUDLE_MONITOR,      /*MONITOR */
    TRANS_MOUDLE_ACTION,         /*ACTION */
    TRANS_MOUDLE_UI,                  /*UI for testband*/
    TRANS_MOUDLE_BUF = 0xff     /**/

};

enum trans_queue_block_enum
{   
   TRANS_QUENE_NO_BLOCK = 0,       /*noblock */
   TRANS_QUENE_BLOCK = 1     /*block*/

};


/*Send Message Enum*/
enum trans_send_msg_enum 
{   
    TRANS_SEND_TO_RRH = 1,     /*Send Message to RRH for Query */
    TRANS_SEND_TO_AGENT,                      /*Send Message to Agent*/    
    TRANS_SEND_TO_MONITOR,                 /*Send Message to Monitor */
    TRANS_SEND_TO_BUF= 0xff                 /*BUF*/
};


enum trans_common_socket_type_enum
{   
    TRANS_COMMON_SOCKET_TYPE_AGENT,            /*AGENT */   
    TRANS_COMMON_SOCKET_TYPE_RRH,                /*RRH */ 
    TRANS_COMMON_SOCKET_TYPE_MONITOR,      /*MONITOR */
    TRANS_COMMON_SOCKET_TYPE_BUF = 0xff     /**/

};


/*****************************************************************************+
*Data structure
+*****************************************************************************/
struct trans_common_sockfd_info
{
    u_int8_t     uc_module_type; /*Module Type: WMA, WMB, Monitor, RRH, Agent*/
    u_int8_t     a_mac[TRANS_MAC_ADDR_LEN];       /*MAC Address*/
    int32_t       w_sockfd;       /*Socket fd*/

};

#define SIZEOF_TRANS_COMMON_SOCKFD_INFO   sizeof(struct trans_common_sockfd_info)

struct trans_common_sockfd
{
    struct trans_common_sockfd_info   st_sockfd[1024];   
    u_int32_t  uw_num;
    pthread_mutex_t   s_mutex;
};

#define SIZEOF_TRANS_COMMON_SOCKFD   sizeof(struct trans_common_sockfd)


/*=====================Buf Struct for Quene====================*/
struct trans_en_queue_msg
{
    u_int8_t     uc_result;     /*right ,wrong*/
    u_int32_t   uw_src_moudle;     /*Sender*/
    u_int32_t   uw_len;
};

#define SIZEOF_TRANS_EN_QUENE_MSG   sizeof(struct trans_en_queue_msg)

struct trans_queue_msg
{
    u_int8_t     uc_result;     /*right ,wrong*/
    u_int32_t   uw_src_moudle;     /*Sender*/
    u_int32_t   uw_len;
    u_int8_t*   p_msg;
};

#define SIZEOF_TRANS_QUENE_MSG   sizeof(struct trans_queue_msg)

/*=====================Buf Struct for Common Quene====================*/
struct trans_common_queue
{
    int32_t       w_sockfd;
    u_int8_t     uc_module_type; /*Device Type: WMA, WMB, Monitor, RRH, Agent*/
    u_int8_t     a_mac[TRANS_MAC_ADDR_LEN];       /*MAC Address*/
    u_int32_t   uw_len;
    void * p_msg;
};

#define SIZEOF_TRANS_COMMON_QUEUE   sizeof(struct trans_common_queue)

/*==================== Struct for Send Message====================*/

/*Send Message to RRH for Query*/
struct trans_send_query_to_rrh
{
    u_int32_t    uw_timeout;   /*time for timeout --- s*/
    fun_callback f_callback;     /*Call this function when timeout or message back*/
    u_int16_t    us_param_num;   /*Number of parameters*/
    void * p_param_type;    /*RRU Parameters type : 2Bytes for one parameters
                                          parameters 1 + parameters 2 + .....+ parameters n*/
    void * p_info;               /*Transaction*/

};

#define SIZEOF_TRANS_SEND_QUERY_TO_RRH     sizeof(struct trans_send_query_to_rrh)

/*Send Message to RRH for Config*/
struct trans_send_cfg_to_rrh
{
    u_int32_t    uw_timeout;   /*time for timeout --- s*/
    fun_callback f_callback;     /*Call this function when timeout or message back*/
    u_int16_t    us_param_num;   /*Number of parameters*/
    void * p_param_type;    /*RRU Parameters type : 2Bytes for one parameters
                                  parameters 1 + parameters 2 + .....+ parameters n*/
    void * p_param_value;    /*RRU Parameters value : 4Bytes for one parameters
                            parameters 1 + parameters 2 + .....+ parameters n*/
    void * p_info;               /*Transaction*/

};

#define SIZEOF_TRANS_SEND_CFG_TO_RRH     sizeof(struct trans_send_cfg_to_rrh)

/*Send Message to RRH */
struct trans_send_msg_to_rrh
{
    fun_callback f_callback;     /*Call this function when timeout or message back*/
    u_int8_t     uc_type;                /*Monitor Type :   enum rrh_monitor_type_enum*/
    u_int32_t    uw_payload_len;   /*length of Payload*/
    void *          p_payload;     /*Payload*/     
    void * p_info;               /*Transaction*/

};

#define SIZEOF_TRANS_SEND_MSG_TO_RRH     sizeof(struct trans_send_msg_to_rrh)


/*Send Message to Monitor*/
struct trans_send_msg_to_monitor
{
    //u_int8_t     uc_type;                /*Type : enum trans_monitor_type_enum */
    u_int8_t      uc_ack_flag;   /*result---0--OK or 1--error  or--enum trans_ack_flag_enum*/
    //u_int16_t    us_opration;   /*Opration*/     
    u_int32_t    uw_payload_len;   /*length of Payload*/    
    void *          p_payload;     /*Payload*/     
    void *          p_info;               /*Transaction*/
};

#define SIZEOF_TRANS_SEND_MSG_TO_MONITOR     sizeof(struct trans_send_msg_to_monitor)

/*Send Message to Monitor*/
struct trans_send_msg_to_agent
{
    //u_int32_t    uw_src_moudle;   /*Sender*/
    //u_int8_t      uc_ack_flag;      /*result---0--OK or 1--error  or--enum trans_ack_flag_enum*/
    //u_int8_t      uc_block_flag;   /*result---0 or 1--enum trans_queue_block_enum*/
    fun_callback f_callback;        /*Call this function when timeout or message back*/
    //u_int32_t   uw_resp_len;                /*Length of p_resp_msg*/    
    void *         p_resp_msg;     /*(1) RRH  (2)wireless array  (3)NULL*/
    void *         p_reqs_msg;     /*(1) struct trans_agent_metric_info  (2)NULL*/
    void *         p_info;               /*Transaction*/

};

#define SIZEOF_TRANS_SEND_MSG_TO_AGENT     sizeof(struct trans_send_msg_to_agent)


/*Send Periodic Sensing Message to Agent*/
struct trans_periodic_sensing_info
{
    float *         p_per_sensing;     /*1*1024 float*/
    int *            p_per_interref;     /*1* 21 int*/
};

#define SIZEOF_TRANS_PERIODIC_SENSING_INFO     sizeof(struct trans_periodic_sensing_info)

#if 0
/*Send Message to Monitor*/
struct trans_send_msg_to_wireless
{
    u_int8_t      uc_ack_flag;   /*result---0--OK or 1--error  or--enum trans_ack_flag_enum*/
    u_int8_t      uc_block_flag;      /*Block message or unblock message*/
    u_int32_t    uw_payload_len;   /*length of Payload*/    
    void *          p_payload;     /*Payload*/     

};

#define SIZEOF_TRANS_SEND_MSG_TO_WIRELESS     sizeof(struct trans_send_msg_to_wireless)

/*Send Message to Monitor*/
struct trans_send_msg_to_action
{
    u_int8_t      uc_ack_flag;   /*result---0--OK or 1--error  or--enum trans_ack_flag_enum*/
    void *          p_action_info;     /**/     
    u_int32_t    uw_action_len;   /*length of action msg*/    
    void *          p_action_msg;     /*Action message*/   
};

#define SIZEOF_TRANS_SEND_MSG_TO_ACTION     sizeof(struct trans_send_msg_to_action)
#endif


/*****************************************************************************+
*extern
+*****************************************************************************/

extern u_int32_t trans_send_action_msg(u_int8_t uc_send_type, void * p_send_info);
extern u_int32_t trans_send_wireless_msg(u_int8_t uc_send_type, void * p_send_info);
extern u_int32_t trans_send_rrh_msg(u_int8_t uc_send_type, void * p_send_info);
extern u_int32_t trans_send_monitor_msg(u_int8_t uc_send_type, void * p_send_info);
extern u_int32_t trans_send_agent_msg(u_int8_t uc_send_type, void * p_send_info);

extern u_int32_t trans_common_init(void);
extern u_int32_t trans_common_release(void);

extern u_int32_t trans_msg_en_quene(void *p_msg, struct trans_en_queue_msg *p_en_quene);
extern u_int32_t trans_msg_de_quene(u_int8_t *p_result);
extern u_int32_t trans_common_msg_en_queue(struct trans_common_queue *p_quene_info);
extern u_int32_t trans_common_msg_de_queue(struct trans_common_queue *p_quene_info);

#if 0
extern u_int32_t trans_common_socket_send(int32_t w_sockfd,
                                                                    u_int8_t *p_mac,
                                                                    void * p_send_buf,
                                                                    u_int32_t uw_send_len);

#endif
extern void trans_common_msg_receive(void);

extern u_int32_t trans_common_msg_func(void* p_info);

extern void trans_common_msg_process(void);

extern struct trans_device_info  g_trans_local_device_info;

extern struct trans_common_sockfd g_trans_common_sockfd;



#endif /* TRANS_COMMON_H_ */

