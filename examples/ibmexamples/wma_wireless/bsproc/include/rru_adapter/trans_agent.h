/*****************************************************************************+
*
*  File Name: trans_agent.h
*
*  Function: 
*
*  
*  Data:    2011-04-14
*  Modify:
*
+*****************************************************************************/

#ifndef TRANS_AGENT_H_
#define TRANS_AGENT_H_


#include <sys/types.h>
/*Jansson lib*/
#include <jansson.h>

/*****************************************************************************+
*Macro
+*****************************************************************************/

#define TRANS_AGENT_HEARTBEAT_TIMEOUT_NUM      60   /* IP Port FOR TCP Connect */

//#define TRANS_AGENT_CLIENT_TCP_PORT     55555   /* Client Port (Local as client) :55555*/
//#define TRANS_AGENT_SERVER_TCP_PORT     55556   /* Server Port (Peer agent as server) :55556*/
#define TRANS_AGENT_TCP_CONNECT_NUM          10   /*TCP connect number*/


#define TRANS_AGENT_SEND_BUF_MAX_LEN        (1024*10)
#define TRANS_AGENT_REV_BUF_MAX_LEN          (1024*10)

#define TRANS_AGENT_TIMESTAMP_SIZE 20
#define TRANS_AGENT_ACTION_SIZE 80 
//#define TRANS_AGENT_DEVICE_ID_SIZE  TRANS_AGENT_DEVICE_ID_SIZE
#define TRANS_AGENT_MODULE_ID_SIZE               TRANS_AGENT_DEVICE_ID_SIZE

#define TRANS_AGENT_BS_METRIC_ID      0x0100   /* BS METRIC */
#define TRANS_AGENT_RRH_METRIC_ID    0x0200  /* RRH METRIC */
#define TRANS_AGENT_MS_METRIC_ID      0x0300  /* MS METRIC */

#ifdef TRANS_MS_COMPILE   
/*In WMB This value is fixed*/
#define TRANS_AGENT_IP_PORT      59999               /* IP Port FOR TCP Connect */
#define TRANS_AGENT_IP_ADDR      "127.0.0.1"      /* IP ADDR FOR TCP Connect */
#define TRANS_AGENT_SELF_DEVICE_ID    "61"   /* Local  device Id*/
#define TRANS_AGENT_AGENT_DEVICE_ID      "01"  /* Peer Agent  device Id */
#endif

#define TRANS_AGENT_MSG_SENSING_NUM      TRANS_AGENT_SENSING_NUM  /*  */
#define TRANS_AGENT_INTERREF_NUM      (21)    /*  */

#define TRANS_AGENT_METRIC_MAX_NUM      (10)    /*  */

/*Revice Module*/
#define TRANS_AGENT_SPECTRUM_MGMT      ("53")    /* spectrum manager */
#define TRANS_AGENT_HEARTBEAT_MGMT      ("51")    /* Heartbeat manager */



#define TRANS_AGENT_PERSENSING_NUM      (1*1024)  /* 1*1024 float */
#define TRANS_AGENT_PERINTERREF_NUM      (1*21)  /* 1* 21 int */


/*****************************************************************************+
*Enum
+*****************************************************************************/
/*Metric ID */
enum trans_agent_bs_metric_id_enum
{
    TRANS_AGENT_BS_METRIC_ID_WMB_DL = 0x0101,   /*Per WMB DL throughput(avg)*/ 
    TRANS_AGENT_BS_METRIC_ID_WMB_UL = 0x0102,   /*Per WMB UL throughput(avg)*/
    TRANS_AGENT_BS_METRIC_ID_CONN_DL = 0x0103,  /*Per conn. DL throughput(avg)*/
    TRANS_AGENT_BS_METRIC_ID_CONN_UL = 0x0104,  /*Per conn. UL throughput(avg)*/    
    TRANS_AGENT_BS_METRIC_ID_UL_MANG = 0x0105,  /*UL management throughput(avg)*/    
    TRANS_AGENT_BS_METRIC_ID_DL_MANG = 0x0106,  /*DL management throughput(avg)*/    
    TRANS_AGENT_BS_METRIC_ID_RANG = 0x0107,         /*Ranging Success conut*/    
    TRANS_AGENT_BS_METRIC_ID_CONN = 0x0108,         /*connection conut*/    
    TRANS_AGENT_BS_METRIC_ID_CRC_ERR = 0x0109,         /*CRCErrorCount*/    
    TRANS_AGENT_BS_METRIC_ID_BUF = 0x01ff             /*BUF*/    
};

/*Metric ID */
enum trans_agent_rrh_metric_id_enum
{
    TRANS_AGENT_RRH_METRIC_ID_PAT  = 0x0201,                /*Power Amplifier Temperature*/ 
    TRANS_AGENT_RRH_METRIC_ID_CH1_PWR  = 0x0202,       /*Downlink Output Power for channel 1#*/
    TRANS_AGENT_RRH_METRIC_ID_CH2_PWR  = 0x0203,       /*Downlink Output Power for channel 2#*/
    TRANS_AGENT_RRH_METRIC_ID_CH1_VSWR  = 0x0204,     /*Downlink Voltage Standing Wave Radio (VSWR) for channel 1#*/    
    TRANS_AGENT_RRH_METRIC_ID_CH2_VSWR = 0x0205,      /*Downlink Voltage Standing Wave Radio (VSWR) for channel 2#*/    
    TRANS_AGENT_RRH_METRIC_ID_BUF = 0x02ff            /*BUF*/    
};

/*Metric ID */
enum trans_agent_ms_metric_id_enum
{
    TRANS_AGENT_MS_METRIC_ID_RSSI = 0x0301,          /*RSSI*/ 
    TRANS_AGENT_MS_METRIC_ID_SINR= 0x0302,   /*SINR*/
    TRANS_AGENT_MS_METRIC_ID_TX_PWR = 0x0303,  /*Tx Power*/
    TRANS_AGENT_MS_METRIC_ID_T = 0x0304,     /*Temperature*/    
    TRANS_AGENT_MS_METRIC_ID_BUF = 0x03ff            /*BUF*/       
};



/*****************************************************************************+
*Data structure
+*****************************************************************************/
/* Agent Config Info  */  
struct trans_agent_config_info
{   
    //u_int8_t     uc_connect_num;    /*Number of TCP connect*/
    u_int16_t   us_agent_tcp_port; /*AGENT  TCP Port*/
    u_int32_t   uw_agent_ip_addr;/*AGENT IP */
    u_int32_t   uw_agent_hb_time;/*HEART BEAT TIME INTERVAL  (second)*/
    u_int8_t    a_device_id[TRANS_AGENT_MODULE_ID_SIZE + 1];
    u_int8_t    a_agent_id[TRANS_AGENT_MODULE_ID_SIZE + 1];

};  

/* Monitor Socket Info  */  
struct trans_agent_socket
{   
    //u_int8_t   uc_lock_flag; /*Lock Flag:once error ,lock the monitor socket*/
    //u_int16_t uw_connect_num;/*Connect Number total */
    int32_t     w_agent_socket; /*Socketfd*/
    pthread_mutex_t   m_mutex;
}; 

/*Agent Message Format*/
struct trans_agent_msg
{
    u_int8_t   a_magic[4];          /*Msg type£º4Bytes   String*/
    u_int8_t   a_len[4];              /*Msg Length£º4Bytes   String   Decimal Number*/
};

#define SIZEOF_TRANS_AGENT_MSG     sizeof(struct trans_agent_msg)

struct trans_agent_metric_info
{
    u_int8_t a_action[TRANS_AGENT_ACTION_SIZE + 1];
    int32_t w_source_id;
    int32_t a_metric_id[TRANS_AGENT_METRIC_MAX_NUM];
    int32_t w_msg_id;   
    u_int8_t uc_metric_num;
    u_int8_t a_element_id[TRANS_AGENT_MODULE_ID_SIZE + 1];
    u_int8_t a_metrics[TRANS_AGENT_METRIC_MAX_NUM][TRANS_AGENT_MODULE_ID_SIZE + 1];    
};

#define SIZEOF_TRANS_AGENT_METRIC_INFO     sizeof(struct trans_agent_metric_info)

struct trans_agent_alert_info
{
    u_int16_t us_alarm_id;
    int32_t     w_alarm_value; 
};

#define SIZEOF_TRANS_AGENT_ALERT_INFO     sizeof(struct trans_agent_alert_info)

struct trans_agent_spectrum_resp_info
{
    int w_f_freq;
    int w_error_code;
    char a_interref_str[TRANS_AGENT_INTERREF_NUM + 1];  /*1* 21 int  -> 21 char*/
};

#define SIZEOF_TRANS_AGENT_SPECTRUM_RESP_INFO     sizeof(struct trans_agent_spectrum_resp_info)


#if 0
struct trans_agent_metric_msg
{
    struct trans_agent_metric_info *p_agent_metric;
    int32_t a_metric_val[TRANS_AGENT_METRIC_MAX_NUM];
};


#define SIZEOF_TRANS_AGENT_METRIC_MSG     sizeof(struct trans_agent_metric_msg)
#endif

struct trans_agent_spectrum_info
{
    double f_latitude;
    double f_longitude;
    float    *p_sensing;
};

#define SIZEOF_TRANS_AGENT_SPECTRUM_INFO     sizeof(struct trans_agent_spectrum_info)

struct trans_agent_spectrum_value
{
    float f_freq;
    int   a_interref[21];
};

#define SIZEOF_TRANS_AGENT_APECTRUM_VALYE     sizeof(struct trans_agent_spectrum_value)


/*****************************************************************************+
*extern
+*****************************************************************************/
extern u_int32_t trans_agent_init(struct trans_init_info *p_init_info);
extern u_int32_t trans_agent_tcp_socket();

extern u_int32_t trans_agent_rev_msg(u_int8_t *p_rev_msg, int32_t w_agent_socket);

extern u_int32_t trans_agent_rev_msg_process(u_int8_t *p_rev_msg, u_int8_t *p_send_msg, 
                           struct trans_thread_info *p_thread_info);

extern u_int32_t trans_agent_get_metric_id(const char * p_metric, int32_t *p_metric_id);

extern u_int32_t trans_agent_get_metric_info(json_t *msg,  
                                    struct trans_agent_metric_info *p_metric_info);

extern u_int32_t trans_agent_rev_metric_msg(json_t *msg, u_int8_t  *p_send_msg, 
                                    struct trans_thread_info *p_thread_info);

extern u_int32_t trans_agent_rev_spectrum_msg(json_t *msg, u_int8_t  *p_send_msg, 
                                    struct trans_thread_info *p_thread_info);

//extern u_int32_t trans_agent_json_metric_process(u_int8_t  *p_rev_msg, u_int8_t  *p_send_msg, int32_t w_agent_sockfd);

#if 0
extern u_int32_t trans_agent_send_metric_msg(struct trans_agent_metric_msg *p_metric_msg, 
                                                       u_int8_t  *p_send_msg, int32_t w_agent_sockfd);

extern u_int32_t trans_agent_send_spectrum_msg(struct trans_agent_spectrum_info *p_spectrum_info, 
                               u_int8_t  *p_send_msg, int32_t w_agent_sockfd);

extern u_int32_t trans_agent_forward_spectrum_msg(u_int8_t  *p_send_msg, void *p_value);

extern u_int32_t trans_agent_send_alert_msg(u_int16_t us_alarm_id, u_int8_t uc_alarm_value, 
                        u_int8_t  *p_send_msg, int32_t w_agent_sockfd);
#endif

extern u_int32_t trans_agent_send_msg(
                            struct trans_send_msg_to_agent *p_msg_info,
                            size_t len,
                            u_int8_t * p_send_msg);

extern u_int32_t trans_agent_send_state_msg(u_int8_t  *p_send_msg, int32_t w_agent_sockfd);

extern u_int32_t trans_agent_send_conn_msg(u_int8_t  *p_send_msg, int32_t w_agent_sockfd);

extern int trans_agent_hb_timer_func(void *p_msg, size_t len, void *p_msg_info);

extern u_int32_t trans_agent_send_hb_msg(u_int8_t  *p_send_msg, int32_t w_agent_sockfd);

extern u_int32_t trans_agent_get_timestamp_string(u_int8_t *p_timestamp);

extern u_int64_t trans_agent_get_time_stamp();

extern u_int32_t trans_agent_release();                           

extern struct trans_agent_config_info g_trans_agent_config_info;

extern struct trans_agent_socket  g_trans_agent_socket;

extern pthread_mutex_t  g_trans_agent_metric_mutex;


#endif /* TRANS_AGENT_H_ */

