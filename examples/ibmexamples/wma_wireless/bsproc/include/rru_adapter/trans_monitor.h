/*****************************************************************************+
*
*  File Name: trans_monitor.h
*
*  Function: TRANS MONITOR MODULE 
*
*  
*  Data:    2011-09-13
*  Modify:
*
+*****************************************************************************/

#ifndef TRANS_MONITOR_H_
#define TRANS_MONITOR_H_


//#include <sys/types.h>
//#include <sys/time.h>

/*****************************************************************************+
*Macro
+*****************************************************************************/

#define TRANS_MONITOR_MSG_MAX_LEN        8192

#define TRANS_MONITOR_TEST_COMPILE    

//#define TRANS_TIMER_WINDOW 100LL
#define TRANS_MONITOR_SEND_BUF_MAX_LEN        (1024*10)
#define TRANS_MONITOR_REV_BUF_MAX_LEN          (1024*10)

#define TRANS_MONITOR_LOCAL_IP_PORT      69999               /* IP Port FOR TCP Connect */
//#define TRANS_MONITOR_LOCAL_IP_ADDR      INADDR_ANY               /* IP Address FOR TCP Connect */

/*****************************************************************************+
*Enum
+*****************************************************************************/

/*Monitor Type*/
enum trans_monitor_type_enum 
{   
    TRANS_MONITOR_TYPE_RRH_Q  = 0x01,       /*0x01 : Query RRH*/
    TRANS_MONITOR_TYPE_RRH_Q_RESP  = 0x02,       /*0x02 : Query RRH Response*/
    TRANS_MONITOR_TYPE_WIRELESS_Q  = 0x03,       /*0x03 : Query Wireless*/
    TRANS_MONITOR_TYPE_WIRELESS_Q_RESP  = 0x04,       /*0x04 : Query Wireless Response*/
    TRANS_MONITOR_TYPE_RRH_C  = 0x05,       /*0x05 : Config RRH*/
    TRANS_MONITOR_TYPE_OPERATION  = 0x10,       /*0x10 : Operation*/
    TRANS_MONITOR_TYPE_TRACE  = 0x20,       /*0x20 : Trace*/
    TRANS_MONITOR_TYPE_BUF = 0xff       /*BUF */

};


/*****************************************************************************+
*Data structure
+*****************************************************************************/
/* Monitor Config Info  */  
struct trans_monitor_config_info
{   
    u_int16_t   us_monitor_tcp_port; /*Monitor  TCP Port*/
    u_int32_t   uw_monitor_ip_addr;/*Monitor IP */

};  

/* Monitor Socket Info  */  
struct trans_monitor_socket
{   
    u_int8_t   uc_lock_flag; /*Lock Flag:once error ,lock the monitor socket*/
    u_int16_t uw_connect_num;/*Connect Number total */
    int32_t     w_monitor_socket; /*Socketfd*/
    pthread_mutex_t   m_mutex;

}; 

/*Build Info For Monitoring Protocol header*/
struct trans_monitor_build_msg_info
{
    u_int8_t     uc_type;                /*Type : enum trans_monitor_type_enum */
    u_int8_t     uc_ack_flag;          /*ACK Flag : enum trans_ack_flag_enum*/
    u_int16_t   us_operation;        /*Operation*/
    u_int32_t   uw_payload_len;    /*Payload Length*/
    void *         p_payload;             /*Payload*/

};

#define SIZEOF_TRANS_MONITOR_BUILD_MSG_INFO     sizeof(struct trans_monitor_build_msg_info)


/*=================Monitor Protocol========((packed))=============*/
/*
   |Message Length(4Bytes)  |  Message Header  (16Bytes)  |Message Payload (.....) |
*/

/*Monitoring Protocol header*/
struct trans_monitor_header
{
    u_int32_t   uw_msg_len;        /*1.Message length : 4 Bytes, Header Length + Payload Length.*/
    u_int8_t     uc_type;                /*Type : enum trans_monitor_type_enum */
    u_int32_t   uw_serial_no;       /*Serial NO.*/
    u_int32_t   uw_payload_len;  /*Payload Length*/
    u_int8_t     uc_ack_flag;          /*ACK Flag : enum trans_ack_flag_enum*/
    u_int16_t   us_operation;        /*Operation ID*/ 
    u_int32_t   uw_transaction;    /*Transaction ID*/
    u_int32_t   uw_server_id;     /*Server BBU ID*/
    u_int8_t     a_reserve[4];         /*Extend*/

}__attribute__ ((packed));

#define SIZEOF_TRANS_MONITOR_HEADER     sizeof(struct trans_monitor_header)

/*Monitoring Protocol Message Payload : TYPE For Query RRH*/
/* [T 1] + [T 2] + ¡­... + [T n] , Tag refer to ¡°IBM RRU C&M Message¡±*/
struct trans_monitor_payload_rrh_q
{
    u_int16_t   us_tag;        /*Tag: */                                                

}__attribute__ ((packed));

#define SIZEOF_TRANS_MONITOR_PAYLOAD_RRH_Q     sizeof(struct trans_monitor_payload_rrh_q)

/*Monitoring Protocol Message Payload : TYPE For Query RRH Response*/
/*[TLV 1] + [ TLV 2] + ¡­... + [ TLV n]*/
/*Tag,  Value Length and Value refer to ¡°IBM RRU C&M Message¡±.*/
struct trans_monitor_payload_rrh_q_resp
{
    u_int16_t   us_tag;        /*Tag*/                                                
    u_int8_t     uc_val_len;  /*Value Length*/
    void *         p_value;      /*Value   :  Length is Value Length*/
}__attribute__ ((packed));

#define SIZEOF_TRANS_MONITOR_PAYLOAD_RRH_Q_RESP     sizeof(struct trans_monitor_payload_rrh_q_resp)

/*Monitoring Protocol Message Payload : TYPE For Query Wireless*/
/*[LT 1] + [LT 2] + ¡­... + [LT n]*/
/*Tag,  Value Length and Value refer to ¡°IBM RRU C&M Message¡±.*/
struct trans_monitor_payload_wireless_q
{
    u_int8_t     uc_tag_len;  /*Tag Length: Tag Length : must include '\0'*/
    char *         p_tag;       /*Tag   :  Tag : a string with the end of '\0',  Length is Tag Length*/
}__attribute__ ((packed));

#define SIZEOF_TRANS_MONITOR_PAYLOAD_WIRELESS_Q     sizeof(struct trans_monitor_payload_wireless_q)


/*Monitoring Protocol Message Payload : TYPE For Query Wireless Response*/
/*[LT 1] + [LT 2] + ¡­... + [LT n]*/
/*Tag,  Value Length and Value refer to ¡°IBM RRU C&M Message¡±.*/
struct trans_monitor_payload_wireless_q_resp
{
    u_int8_t     uc_tag_len;  /*Tag Length: Tag Length : must include '\0'*/
    char *         p_tag;       /*Tag   :  Tag : a string with the end of '\0',  Length is Tag Length*/
    u_int16_t   us_val_len;  /*Value Length: If value is a string, it must include '\0''*/
    void *         p_value;       /*Value   :  Length is Value Length*/

}__attribute__ ((packed));

#define SIZEOF_TRANS_MONITOR_PAYLOAD_WIRELESS_Q_RESP     sizeof(struct trans_monitor_payload_wireless_q_resp)


/*****************************************************************************+
*extern
+*****************************************************************************/
extern u_int32_t trans_monitor_release();

extern u_int32_t trans_monitor_init(struct trans_init_info *p_init_info);

extern u_int32_t trans_monitor_tcp_socket();

extern void trans_monitor_tcp_connect();

extern u_int32_t trans_monitor_rev_msg(u_int8_t *p_rev_msg, int32_t w_monitor_socket);

extern u_int32_t trans_monitor_rev_msg_process(struct trans_msg_info *p_msg_info, 
                           size_t len,
                           void * p_send_msg);

extern u_int32_t trans_monitor_send_monitor_msg(
                            struct trans_send_msg_to_monitor *p_msg_info,
                            size_t len,
                            u_int8_t * p_send_msg);

extern u_int32_t trans_monitor_send_wireless_msg(
                            struct trans_send_msg_to_monitor *p_msg_info,
                            size_t len,
                            u_int8_t * p_send_msg);

extern u_int32_t trans_monitor_send_agent_msg(
                            struct trans_send_msg_to_monitor *p_msg_info,
                            size_t len,
                            u_int8_t * p_send_msg);

extern u_int32_t trans_monitor_send_rrh_msg(
                            struct trans_send_msg_to_monitor *p_msg_info,
                            size_t len,
                            u_int8_t * p_send_msg);

extern u_int32_t trans_monitor_send_action_msg(
                            struct trans_send_msg_to_monitor *p_msg_info,
                            size_t len,
                            u_int8_t * p_send_msg);

//extern void trans_monitor_tcp_connect_failed();

extern void trans_monitor_conn_failed();

extern struct trans_monitor_socket  g_trans_monitor_socket;


#endif /* TRANS_MONITOR_H_ */

