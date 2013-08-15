/* ----------------------------------------------------------------------------
 IBM Confidential

 IBM Wireless M2M Platform

 (C)Copyright IBM Corp. 2009, 2010, 2011

 All Rights Reserved.

 File Name: trans_monitor.h

 Change Activity:

 Date             Description of Change                            By
 -----------      ---------------------                            --------
 01-Sep.2011      Created                                          E Wulan

 ----------------------------------------------------------------------------
 PROLOG END TAG zYx                                                         */


#ifndef TRANS_MONITOR_H_
#define TRANS_MONITOR_H_


//#include <sys/types.h>
//#include <sys/time.h>
#include <trans_common.h>

/*****************************************************************************+
*Macro
+*****************************************************************************/

#define TRANS_MONITOR_MSG_MAX_LEN        81920

//#define TRANS_MONITOR_TEST_COMPILE    

//#define TRANS_TIMER_WINDOW 100LL
#define TRANS_MONITOR_SEND_BUF_MAX_LEN        (650*1024)
#define TRANS_MONITOR_REV_BUF_MAX_LEN          (650*1024)

#define TRANS_MONITOR_LOCAL_IP_PORT      49999               /* IP Port FOR TCP Connect */
//#define TRANS_MONITOR_LOCAL_IP_ADDR      INADDR_ANY               /* IP Address FOR TCP Connect */

#define TRANS_MONITOR_CONNECT_MAX_NUM        10
#define TRANS_MONITOR_MAX_SOCKFD        1024


#define TRANS_MONITOR_REGISTER_MAX_LEN        37

/*****************************************************************************+
*Enum
+*****************************************************************************/

/*Monitor Type*/
enum trans_monitor_type_enum 
{   
    TRANS_MONITOR_TYPE_REGISTER  = 0x01,                  /*0x01 : Register*/
    TRANS_MONITOR_TYPE_REGISTER_RESP  = 0x02,       /*0x02 : Register Response*/
    TRANS_MONITOR_TYPE_QUERY  = 0x03,                       /*0x03 : Query*/
    TRANS_MONITOR_TYPE_QUERY_RESP  = 0x04,            /*0x04 : Query Response */
    TRANS_MONITOR_TYPE_CONFIG  = 0x05,                       /*0x05 : Config*/
    TRANS_MONITOR_TYPE_CONFIG_RESP  = 0x06,            /*0x06 : Config Response */
    TRANS_MONITOR_TYPE_HOOK  = 0x07,                         /*0x07 : HOOK*/
    TRANS_MONITOR_TYPE_HOOK_RESP  = 0x08,              /*0x08 : HOOK Response */
    TRANS_MONITOR_TYPE_OPRATION  = 0x09,                  /*0x09 : Operation*/
    TRANS_MONITOR_TYPE_OPRATION_RESP  = 0x10,       /*0x10 : Operation Response*/
    TRANS_MONITOR_TYPE_BUF = 0xff       /*BUF */

};

/*Monitor device Type*/
enum trans_monitor_device_type_enum 
{   
    TRANS_MONITOR_DEVICE_TYPE_MONITOR  = 0x01,             /*0x01 : Monitor*/
    TRANS_MONITOR_DEVICE_TYPE_WMA  = 0x02,                     /*0x02 : WMA*/
    TRANS_MONITOR_DEVICE_TYPE_WMB  = 0x03,                     /*0x03 : WMB*/
    TRANS_MONITOR_DEVICE_TYPE_UI  = 0x04,                         /*0x04 : UI for Testband*/
    TRANS_MONITOR_DEVICE_TYPE_BUF = 0xff       /*BUF */

};

/*Query trans device Type*/
enum trans_monitor_query_devide_enum 
{   
    TRANS_MONITOR_QUERY_DEVICE_ROOT  = 0x01,             /*0x01 : root node*/
    TRANS_MONITOR_QUERY_DEVICE_LEAF  = 0x02,              /*0x02 : leaf node*/
    TRANS_MONITOR_QUERY_DEVICE_BUF= 0xff       /*BUF */

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

/* Monitor register sockfd Info  */  
struct trans_monitor_register
{   
    int32_t   w_sockfd;/* */
    void *p_timer; /*Timer PTR*/

};  

/* Monitor Socket Info  */  
struct trans_monitor_socket
{   
    u_int8_t   uc_lock_flag; /*Lock Flag:once error ,lock the monitor socket*/
    u_int16_t uw_connect_num;/*Connect Number total */
    int32_t     w_monitor_socket; /*Socketfd*/
    pthread_mutex_t   m_mutex;

}; 

struct trans_monitor_info
{
    u_int8_t     uc_type;                /*Type : enum trans_monitor_type_enum */
    u_int16_t   us_operation;        /*Operation*/
    u_int32_t   uw_transaction;    /*Transaction ID*/
    u_int8_t     a_src_mac[TRANS_MAC_ADDR_LEN];          /*Source MAC*/
    u_int8_t     a_dst_mac[TRANS_MAC_ADDR_LEN];          /*Dest MAC*/
};

#define SIZEOF_TRANS_MONITOR_INFO     sizeof(struct trans_monitor_info)

/*Build Info For Monitoring Protocol header*/
struct trans_monitor_build_msg_info
{
    u_int8_t     uc_type;                /*Type : enum trans_monitor_type_enum */
    u_int8_t     uc_ack_flag;          /*ACK Flag : enum trans_ack_flag_enum*/
    u_int16_t   us_operation;        /*Operation*/
    u_int32_t   uw_transaction;    /*Transaction ID*/
    u_int8_t     a_src_mac[TRANS_MAC_ADDR_LEN];          /*Source MAC*/
    u_int8_t     a_dst_mac[TRANS_MAC_ADDR_LEN];          /*Dest MAC*/
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
    u_int8_t     a_src_mac[TRANS_MAC_ADDR_LEN];          /*Source MAC*/
    u_int8_t     a_dst_mac[TRANS_MAC_ADDR_LEN];          /*Dest MAC*/
    u_int8_t     a_reserve[4];         /*Extend*/

}__attribute__ ((packed));

#define SIZEOF_TRANS_MONITOR_HEADER     sizeof(struct trans_monitor_header)

/*Monitoring Protocol Message Payload : TYPE For Query RRH*/
/* [T 1] + [T 2] + ¡­... + [T n] , Tag refer to ¡°IBM RRU C&M Message¡±*/
struct trans_monitor_payload_q_rrh
{
    u_int16_t   us_tag;        /*Tag: */                                                

}__attribute__ ((packed));

#define SIZEOF_TRANS_MONITOR_PAYLOAD_Q_RRH     sizeof(struct trans_monitor_payload_q_rrh)

/*Monitoring Protocol Message Payload : TYPE For Query RRH Response*/
/*[TLV 1] + [ TLV 2] + ¡­... + [ TLV n]*/
/*Tag,  Value Length and Value refer to ¡°IBM RRU C&M Message¡±.*/
struct trans_monitor_payload_q_rrh_resp
{
    u_int16_t   us_tag;        /*Tag*/                                                
    u_int8_t     uc_val_len;  /*Value Length*/
    void *         p_value;      /*Value   :  Length is Value Length*/
}__attribute__ ((packed));

#define SIZEOF_TRANS_MONITOR_PAYLOAD_Q_RRH_RESP     sizeof(struct trans_monitor_payload_q_rrh_resp)

/*Monitoring Protocol Message Payload : TYPE For Query Wireless*/
/*[LT 1] + [LT 2] + ¡­... + [LT n]*/
/*Tag,  Value Length and Value refer to ¡°IBM RRU C&M Message¡±.*/
struct trans_monitor_payload_q_wireless
{
    u_int8_t     uc_tag_len;  /*Tag Length: Tag Length : must include '\0'*/
    char *         p_tag;       /*Tag   :  Tag : a string with the end of '\0',  Length is Tag Length*/
}__attribute__ ((packed));

#define SIZEOF_TRANS_MONITOR_PAYLOAD_Q _WIRELESS    sizeof(struct trans_monitor_payload_q_wireless)


/*Monitoring Protocol Message Payload : TYPE For Query Wireless Response*/
/*[LT 1] + [LT 2] + ¡­... + [LT n]*/
/*Tag,  Value Length and Value refer to ¡°IBM RRU C&M Message¡±.*/
struct trans_monitor_payload_q_wireless_resp
{
    u_int8_t     uc_tag_len;  /*Tag Length: Tag Length : must include '\0'*/
    char *         p_tag;       /*Tag   :  Tag : a string with the end of '\0',  Length is Tag Length*/
    u_int16_t   us_val_len;  /*Value Length: If value is a string, it must include '\0''*/
    void *         p_value;       /*Value   :  Length is Value Length*/

}__attribute__ ((packed));

#define SIZEOF_TRANS_MONITOR_PAYLOAD_Q_WIRELESS_RESP     sizeof(trans_monitor_payload_q_wireless_resp)

/*Monitoring Protocol Message Payload : TYPE For Config RRH*/
/*[TLV 1] + [ TLV 2] + ¡­... + [ TLV n]*/
/*Tag,  Value Length and Value refer to ¡°IBM RRU C&M Message¡±.*/
struct trans_monitor_payload_c_rrh
{
    u_int16_t   us_tag;        /*Tag*/                                                
    //u_int8_t     uc_val_len;  /*Value Length*/
    u_int32_t   uw_value;  /*Value Length*/
    //void *         p_value;      /*Value   :  Length is Value Length*/
}__attribute__ ((packed));

#define SIZEOF_TRANS_MONITOR_PAYLOAD_C_RRH     sizeof(struct trans_monitor_payload_c_rrh)

/*Monitoring Protocol Message Payload : TYPE For Query or Config Wireless*/
struct trans_monitor_payload_wireless
{
    u_int8_t   a_name[64];        /*name*/                                                
    u_int32_t   uw_type;          /*Value Length*/
    u_int8_t    a_value[128];      /*Value  */
}__attribute__ ((packed));

#define SIZEOF_TRANS_MONITOR_PAYLOAD_WIRELESS     sizeof(struct trans_monitor_payload_wireless)

/*Monitoring Protocol Message Payload : TYPE For Operation dump*/
struct trans_monitor_payload_dump
{
    u_int8_t  a_cfg_name[64];
    u_int32_t     uw_on_off;

}__attribute__ ((packed));

#define SIZEOF_TRANS_MONITOR_PAYLOAD_DUMP     sizeof(struct trans_monitor_payload_dump)

/*Monitoring Protocol Message Payload : TYPE For Query trans*/
struct trans_monitor_payload_trans
{
    u_int8_t  a_cfg_name[64];
    u_int32_t     uw_type;

}__attribute__ ((packed));

#define SIZEOF_TRANS_MONITOR_PAYLOAD_TRANS     sizeof(struct trans_monitor_payload_trans)

/*Monitoring Protocol Message Payload : TYPE For Query trans device info*/
struct trans_monitor_payload_trans_device
{
    u_int8_t  a_mac[6];
    u_int8_t  uc_type;

}__attribute__ ((packed));

#define SIZEOF_TRANS_MONITOR_PAYLOAD_TRANS_DEVICE     sizeof(struct trans_monitor_payload_trans_device)


/*****************************************************************************+
*extern
+*****************************************************************************/
extern u_int32_t trans_monitor_release();

extern u_int32_t trans_monitor_init(struct trans_init_info *p_init_info);

extern u_int32_t trans_monitor_tcp_socket();

extern void trans_monitor_tcp_connect();

extern u_int32_t trans_monitor_rev_msg(u_int8_t **pp_rev_msg, int32_t w_monitor_socket, int32_t *p_len);

extern u_int32_t trans_monitor_parse_msg(void *p_info, 
                           size_t len,
                           void * p_rev_buf);

extern u_int32_t trans_monitor_build_msg(struct trans_monitor_build_msg_info *p_build_info, 
                                    u_int8_t *p_send_msg,
                                    u_int32_t *p_send_len);


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

extern u_int32_t trans_monitor_send_registration(int32_t w_sockfd);
extern u_int32_t trans_monitor_rev_registration();


#endif /* TRANS_MONITOR_H_ */

