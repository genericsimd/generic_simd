
/*****************************************************************************+
*
*  File Name: trans.h
*
*  Function: TRANS
*
*  
*  Data:    2011-03-14
*  Modify:
*
+*****************************************************************************/

#ifndef TRANS_H_
#define TRANS_H_


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


//#define TRANS_RRH_COMPILE    
//#define TRANS_AGENT_COMPILE    
//#define TRANS_BS_COMPILE    
//#define TRANS_MS_COMPILE   
//#define TRANS_MONITOR_COMPILE 
//#define TRANS_ACTION_COMPILE 

//#define TRANS_RRH_RAW_SOCKET 

#define TRANS_SUCCESS   0     /* successful */
#define TRANS_FAILD    1         /* faild */

#undef TRANS_MAX
#define TRANS_MAX(x,y)                   ((x) > (y) ? (x) : (y))

#define TRANS_SEND_METRIC_TO_RRH_FAKE         1

#define TRANS_SET_SENSING_RESULT_FAKE         0


#define TRANS_REV_MSG_MAX_LEN        4096

#define TRANS_SEND_MSG_MAX_LEN        4096


#define TRANS_RRH_DEVICE_ID_SIZE        20

#define TRANS_AGENT_DEVICE_ID_SIZE        20
#define TRANS_AGENT_ID_SIZE                  20

#define TRANS_AGENT_SENSING_NUM      (11*25)  /* For getSpectrumResult message */

#ifdef TRANS_BS_COMPILE
#define TRANS_AGENT_CONNECT_NUM                  20  /*connect 20 times, sleep(3)/times*/
#endif
#ifdef TRANS_MS_COMPILE
#define TRANS_AGENT_CONNECT_NUM                  0   /*connect forever*/
#endif

#define TRANS_MONITOR_SEND_MSG_MAX_LEN        8192
#define TRANS_ACTION_SEND_MSG_MAX_LEN        8192
#define TRANS_AGENT_SEND_MSG_MAX_LEN        256
#define TRANS_WIRELESS_SEND_MSG_MAX_LEN        8192
#define TRANS_RRH_SEND_MSG_MAX_LEN        1024


/*********TYPE For Send message To RRH************/
#define TRANS_SEND_INIT_CFG                             0x00  /*Init Config*/
#define TRANS_SEND_CFG_CHANNEL_MODE        0x01  /**/
#define TRANS_SEND_CFG_CHANNEL_FREQ         0x02  /*Freq*/
#define TRANS_SEND_CFG_CHANNEL_FLAG          0x03  /*Open ,  Close*/
#define TRANS_SEND_CFG_DL_PRESEND_TIME    0x04  /**/
#define TRANS_SEND_CFG_TX_LEN                        0x05  /**/
#define TRANS_SEND_CFG_RX_LEN                        0x06  /**/
#define TRANS_SEND_CFG_GPS_ENABLE               0x07  /**/
#define TRANS_SEND_CFG_OUTPUT_POWER        0x08  /**/
#define TRANS_SEND_CFG_PA_SWITCH                 0x09  /**/
#define TRANS_SEND_CFG_AGC                              0x10  /**/
#define TRANS_SEND_CFG_CH_RX_PGC                0x11  /**/



#define TRANS_SEND_INIT_QUERY               0x80  /*Init Query*/
#define TRANS_SEND_QUERY_POWER          0x81  /*Query Power*/
#define TRANS_SEND_QUERY_GPS_LOCK    0x82  /*Query GPS LOCK*/



/*********TYPE For Send message To Agent************/
#define TRANS_SEND_SPECTRUM_TO_AGENT          0x01  /*spectrum message including: GPS info*/

#define TRANS_HEARTBEAT_TIMEOUT 3

#define TRANS_RRH_LISTEN_TIMEOUT 20


extern u_int32_t g_enable_metric;

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
    TRANS_MOUDLE_BUF = 0xff     /**/

};

enum trans_queue_block_enum
{   
   TRANS_QUENE_NO_BLOCK = 0,       /*noblock */
   TRANS_QUENE_BLOCK = 1     /*block*/

};

/*Callback Function Enum*/
enum trans_register_fun_enum 
{   
    TRANS_REGISTER_FUN_MONITOR_OP  = 1024,   /*Callback function for monitor operation */
    TRANS_REGISTER_FUN_BUF  = 2048        /**/

};

/*Trans ACK Flag*/
enum trans_ack_flag_enum
{   
    TRANS_ACK_FLAG_OK = 0x00,              /*sucessful*/ 
    TRANS_ACK_FLAG_P_ERR = 0x01,        /*Part error*/
    TRANS_ACK_FLAG_TYPE_ERR = 0x02,  /*Type error*/ 
    TRANS_ACK_FLAG_LEN_ERR = 0x03,    /*length error*/
    TRANS_ACK_FLAG_RRH_CRC_ERR = 0x04,   /*RRH CRC error*/ 
    TRANS_ACK_FLAG_RRH_TIMEOUT = 0xC0,    /*RRH timeout error*/ 
    TRANS_ACK_FLAG_OTHER_ERR = 0xf0,    /*Other error*/    
    TRANS_ACK_FLAG_CLEAN_OPERATION = 0xfd,               /*Order ---Clean operation*/    
    TRANS_ACK_FLAG_ORDER_NO_RESP = 0xfe,     /*Order ---no response back*/
    TRANS_ACK_FLAG_ORDER_WITH_RESP = 0xff  /*Order ---need response back*/
};

/*Trans ACK Type*/
enum trans_resp_msg_type_enum
{   
    TRANS_RESP_MSG_QUERY_RRH = 0x00,        /**/ 
    TRANS_RESP_MSG_CONFIG_RRH = 0x01        /**/

};

/*Send Message Enum*/
enum trans_send_msg_enum 
{   
    TRANS_SEND_TO_RRH_QUERY  = 1,     /*Send Message to RRH for Query */
    TRANS_SEND_TO_RRH_CFG,                 /*Send Message to RRH for Config */
    TRANS_SEND_TO_AGENT,                      /*Send Message to Agent*/    
    TRANS_SEND_TO_MONITOR,                 /*Send Message to Monitor */
    TRANS_SEND_TO_BUF= 0xff                 /*BUF*/
};

#if 0
/*Monitor Type*/
enum trans_monitor_type_enum 
{   
    TRANS_MONITOR_TYPE_RRH_Q  = 0x01,       /*0x01 : Query RRH*/
    TRANS_MONITOR_TYPE_RRH_Q_RESP  = 0x02,       /*0x02 : Query RRH Response*/
    TRANS_MONITOR_TYPE_WIRELESS_Q  = 0x03,       /*0x03 : Query Wireless*/
    TRANS_MONITOR_TYPE_WIRELESS_Q_RESP  = 0x04,       /*0x04 : Query Wireless Response*/
    TRANS_MONITOR_TYPE_OPERATION  = 0x10,       /*0x10 : Operation*/
    TRANS_MONITOR_TYPE_TRACE  = 0x20,       /*0x20 : Trace*/

    TRANS_MONITOR_TYPE_TRACE_BS_FCH  = 0x21,       /*0x21 : Trace : BS PHY Tx:  FCH+dl_unused_subch*/

    TRANS_MONITOR_TYPE_TRACE_BS_CH  = 0x41,       /*0x41 : Trace : BS PHY Rx: Channel Quality*/
    TRANS_MONITOR_TYPE_TRACE_BS_CONS  = 0x42,       /*0x42 : Trace : BS PHY Rx: Rx Constellation*/
    TRANS_MONITOR_TYPE_TRACE_BS_B  = 0x43,       /*0x43 : Trace : BS PHY Rx: BER/BLER*/
    TRANS_MONITOR_TYPE_TRACE_BS_PS  = 0x44,       /*0x44 : Trace : BS PHY Rx: PS related*/

    TRANS_MONITOR_TYPE_TRACE_BS_DISP  = 0xA1,       /*0xA1 : Trace : MS: Display*/
    TRANS_MONITOR_TYPE_TRACE_BS_DEG  = 0xA2,       /*0xA1 : Trace : MS: Debug*/
    
    TRANS_MONITOR_TYPE_BUF = 0xff       /*BUF */

};
#endif

/*****************************************************************************+
*Data structure
+*****************************************************************************/
/*  RRH Init Info  */  
struct trans_init_info
{   
    u_int8_t     uc_server_id;   /*SERVER ID*/
    u_int8_t     a_mac[6];       /*Local MAC Address*/
    u_int32_t   uw_rru_id;    /*RRU ID*/
    u_int32_t   uw_rrh_ip_addr;/*RRU IP */
    u_int32_t   uw_rrh_mask_addr;/*RRU  mask*/
    u_int32_t   uw_ser_ip_addr;/*SERVER IP */
    u_int32_t   uw_ser_broc_addr;/*SERVER BROADCAST IP */
    u_int16_t   us_ser_tcp_port; /*SERVER  TCP Port*/
    u_int16_t   us_ser_data_port; /*SERVER  I/Q  Data Port*/ 
    u_int8_t     a_rrh_nic_if[TRANS_RRH_DEVICE_ID_SIZE];   /*Device ID : ETH1, Eth2....*/
    u_int16_t   us_agent_tcp_port; /*AGENT  TCP Port*/
    u_int32_t   uw_agent_ip_addr;/*AGENT IP */
    u_int32_t   uw_agent_hb_time;/*HEART BEAT TIME INTERVAL  (second)*/
    u_int8_t    a_agent_device_id[TRANS_AGENT_DEVICE_ID_SIZE + 1];
    u_int8_t    a_agent_id[TRANS_AGENT_ID_SIZE + 1];
    u_int16_t  us_monitor_port;  /*Monitor server port*/
};


/*thread info*/
struct trans_thread_info
{
    int32_t w_rrh_sockfd;
    int32_t w_agent_sockfd;    
    int32_t w_monitor_sockfd;

};

/*Thread ID*/
struct trans_thread_id
{
    pthread_t timer_thread;
    pthread_t msg_thread;
    pthread_t action_thread;
    pthread_t monitor_thread;

};

struct trans_msg_info
{
    struct trans_thread_info *p_thread_info;
    u_int8_t *p_rev_msg;
};

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
    //u_int16_t   us_serial_number;  /*serial_number for the sending message*/
    u_int32_t   uw_src_moudle;     /*Sender*/
    u_int32_t   uw_len;
    u_int8_t*   p_msg;
};

#define SIZEOF_TRANS_QUENE_MSG   sizeof(struct trans_queue_msg)

/*==================== Struct for Function register====================*/

/**********************************************************************+
* Function: fun_callback()
* Description: Callback Function
* Parameters:
*           p_info : Info defined by user , must malloc and free by user--It could be NULL or other
*           len : The length of p_result*---It could be 0
*           p_result : (1)struct trans_resp_msg_result 
*                           (2)It could be NULL or other
*                            Don't care its memory.
* Return Values:
*           NONE
*
*  
*  Data:    2011-09-05
+*********************************************************************/
typedef int (*fun_callback) (void *p_info, size_t len, void *p_result);


struct trans_register_func
{
    u_int8_t  uc_use_flag;  
    fun_callback f_callback;
};

/*==================== Struct for Response Message====================*/

/*
   |  Message Header  (struct trans_resp_msg_com)  |Message Payload (.....) |
*/

/*Result for RRH Message Back*/
struct trans_resp_msg_header
{
    //u_int8_t     uc_tpye;    /*enum trans_resp_msg_type_enum */
    u_int8_t     uc_result;  /*enum trans_ack_flag_enum*/
    u_int32_t   uw_len;     /*Length of p_result_msg*/
    void *         p_buf;       /*Message Payload*/
}__attribute__ ((packed));

#define SIZEOF_TRANS_RESP_MSG_HEADER    sizeof(struct trans_resp_msg_header)

#if 0
/*Result for RRH Message Back*/
struct trans_resp_msg
{
    struct trans_resp_msg_header  st_resp_com;
    
    /*Message Payload*/  
    /*
    (1) TLV----refer to RRH protocol  
    (2) Monitor payload 
    (3)others
    */
};

//#define SIZEOF_TRANS_RESP_MSG   sizeof(struct trans_resp_msg)
#endif

/*==================== Struct for Send Message====================*/

/*Send Message to RRH for Query*/
struct trans_send_query_to_rrh
{
    u_int32_t    uw_timeout;   /*time for timeout --- s*/
    fun_callback f_callback;     /*Call this function when timeout or message back*/
    u_int16_t    us_param_num;   /*Number of parameters*/
    void * p_param_type;    /*RRU Parameters type : 2Bytes for one parameters
                                          parameters 1 + parameters 2 + .....+ parameters n*/
    void *  p_info;               /*Input for f_callback*/
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
    void *  p_info;               /*Input for f_callback*/

};

#define SIZEOF_TRANS_SEND_CFG_TO_RRH     sizeof(struct trans_send_cfg_to_rrh)

/*Send Message to Monitor*/
struct trans_send_msg_to_monitor
{
    //u_int8_t     uc_type;                /*Type : enum trans_monitor_type_enum */
    u_int8_t      uc_ack_flag;   /*result---0--OK or 1--error  or--enum trans_ack_flag_enum*/
    u_int16_t    us_opration;   /*Opration*/     
    u_int32_t    uw_payload_len;   /*length of Payload*/    
    void *          p_payload;     /*Payload*/     

};

#define SIZEOF_TRANS_SEND_MSG_TO_MONITOR     sizeof(struct trans_send_msg_to_monitor)

/*Send Message to Monitor*/
struct trans_send_msg_to_agent
{
    //u_int32_t    uw_src_moudle;   /*Sender*/
    u_int8_t      uc_ack_flag;      /*result---0--OK or 1--error  or--enum trans_ack_flag_enum*/
    u_int8_t      uc_block_flag;   /*result---0 or 1--enum trans_queue_block_enum*/
    fun_callback f_callback;        /*Call this function when timeout or message back*/
    u_int32_t   uw_resp_len;                /*Length of p_resp_msg*/    
    void *         p_resp_msg;     /*(1) RRH  (2)wireless array  (3)NULL*/
    void *         p_reqs_msg;     /*(1) struct trans_agent_metric_info  (2)NULL*/
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

extern u_int32_t trans_init(struct trans_init_info *p_init_info);
extern u_int32_t trans_connect_rrh();
extern u_int32_t trans_connect_agent();

//extern void trans_msg_process();
//extern u_int32_t trans_monitor_thread();
//extern u_int32_t trans_msg_thread();
//extern u_int32_t trans_timer_thread();
//extern u_int32_t trans_action_thread();
extern u_int32_t trans_init_finish_inform_agent();

extern u_int32_t trans_send_msg_to_rrh(u_int8_t uc_type, u_int8_t uc_num, void *p_value);
//extern u_int32_t trans_send_msg_to_agent(u_int8_t uc_type, void *p_value);

extern u_int32_t trans_send_action_msg(u_int8_t uc_send_type, void * p_send_info);
extern u_int32_t trans_send_wireless_msg(u_int8_t uc_send_type, void * p_send_info);
extern u_int32_t trans_send_rrh_msg(u_int8_t uc_send_type, void * p_send_info);
extern u_int32_t trans_send_monitor_msg(u_int8_t uc_send_type, void * p_send_info);
extern u_int32_t trans_send_agent_msg(u_int8_t uc_send_type, void * p_send_info);

extern u_int32_t trans_thread_create();
extern u_int32_t trans_release();

extern u_int32_t trans_register_func_callback(u_int16_t us_register_type,
                                    void * p_register_name,  
                                    fun_callback f_exe_func,
                                    fun_callback f_delete_func);


extern int32_t g_trans_moudle_socket_fd[TRANS_MOUDLE_BUF];

extern struct trans_thread_id  g_trans_thread_id;

extern struct trans_register_func  g_trans_register_exe_func[TRANS_REGISTER_FUN_BUF];
extern struct trans_register_func  g_trans_register_delete_func[TRANS_REGISTER_FUN_BUF];

u_int32_t turn_metric (u_int32_t on_off);

#endif /* TRANS_H_ */

