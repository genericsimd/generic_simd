/* ----------------------------------------------------------------------------
 IBM Confidential

 IBM Wireless M2M Platform

 (C)Copyright IBM Corp. 2009, 2010, 2011

 All Rights Reserved.

 File Name: trans_transaction.h

 Change Activity:

 Date             Description of Change                            By
 -----------      ---------------------                            --------
 30-Nov.2012      Created                                          E Wulan

 ----------------------------------------------------------------------------
 PROLOG END TAG zYx                                                         */


#ifndef TRANS_TRNSACTION_H_
#define TRANS_TRNSACTION_H_


#include <sys/types.h>


/*****************************************************************************+
*Macro
+*****************************************************************************/
#define TRANS_TRANSACTION_INVALID_NUM         0xffffffff


/*****************************************************************************+
*Enum
+*****************************************************************************/
enum trans_transaction_use_flag
{   
   TRANS_TRANSACTION_FLAG_NO_USE = 0,       /*no use */
   TRANS_TRANSACTION_FLAG_USE = 1     /*use*/

};

enum trans_transaction_delete_flag
{   
   TRANS_TRANSACTION_FLAG_NO_DELETE = 0,       /*no delete */
   TRANS_TRANSACTION_FLAG_DELETE = 1     /*delete*/

};

enum trans_transaction_exe_flag
{   
   TRANS_TRANSACTION_FLAG_EXE_NOW = 0,       /*no delete */
   TRANS_TRANSACTION_FLAG_EXE_ACTION = 1     /*delete*/

};

/*****************************************************************************+
*Data structure
+*****************************************************************************/
struct trans_transaction_list
{
    struct trans_unordered_list * p_unordered_list;
    u_int32_t   uw_execute_transaction;
    u_int32_t   uw_transaction_id;
    pthread_mutex_t   ta_mutex;

};

#define SIZEOF_TRANS_TRANSACTION_LIST   sizeof(struct trans_transaction_list)

struct trans_transaction_cmp_info
{
    u_int8_t     a_mac[TRANS_MAC_ADDR_LEN];       /*MAC Address*/
    u_int32_t   uw_num;
};

#define SIZEOF_TRANS_TRANSACTION_CMP_INFO   sizeof(struct trans_transaction_cmp_info)

/*==================== Struct for Transaction====================*/
/*This struct run through all the TRANS flow----It is created when receive a message*/

/*struct trans_transaction_module  st_common;*/
struct trans_transaction_common
{
    u_int32_t uw_src_module;
    u_int8_t   uc_deleted;
};

#define SIZEOF_TRANS_TRANSACTION_COMMON     sizeof(struct trans_transaction_common)

struct trans_transaction_socket
{
    int32_t       w_sockfd;
    u_int8_t     a_mac[TRANS_MAC_ADDR_LEN];       /*MAC Address*/
    u_int32_t   uw_num;
};

#define SIZEOF_TRANS_TRANSACTION_SOCKET   sizeof(struct trans_transaction_socket)


/*struct trans_transaction_module  st_func;*/
struct trans_transaction_func
{
    u_int8_t     uc_exe_flag;    /**/
    u_int32_t   uw_func_id;     /*Funtion Callback ID*/
    int32_t      w_msg_len;
    void  *       p_msg;
};

#define SIZEOF_TRANS_TRANSACTION_FUNC     sizeof(struct trans_transaction_func)

/*struct trans_transaction_module  st_user;*/
struct trans_transaction_user
{
    int32_t      w_msg_len;
    void  *       p_msg;
};

#define SIZEOF_TRANS_TRANSACTION_USER     sizeof(struct trans_transaction_user)


/*struct trans_transaction_module  st_result;*/
struct trans_transaction_result
{
    //u_int8_t     uc_tpye;    /*enum trans_resp_msg_type_enum */
    u_int32_t     uw_result;  /*enum trans_ack_flag_enum*/
    //u_int32_t   uw_len;     /*Length of p_result_msg*/
};

#define SIZEOF_TRANS_TRANSACTION_RESULT     sizeof(struct trans_transaction_result)

struct trans_transaction_timer
{
    void  *       p_timer;  /*Timer PTR*/
};

#define SIZEOF_TRANS_TRANSACTION_TIMER     sizeof(struct trans_transaction_timer)

/*struct trans_transaction_module  st_agent;*/
struct trans_transaction_agent
{
    //u_int8_t   uc_lock_flag; /*Lock Flag:once error ,lock the monitor socket*/
    //u_int16_t uw_connect_num;/*Connect Number total */
    int32_t     w_agent_socket; /*Socketfd*/
    fun_callback f_callback;          /*Funtion Callback*/
    int32_t     w_msg_len;
    void  *      p_msg;

};

#define SIZEOF_TRANS_TRANSACTION_AGENT     sizeof(struct trans_transaction_agent)

/*struct trans_transaction_module  st_agent;*/
struct trans_transaction_rrh
{
    int32_t       w_rrh_socket; /*Socketfd*/
    fun_callback f_callback;          /*Funtion Callback*/
    int32_t      w_msg_len;
    void  *       p_msg;

};

#define SIZEOF_TRANS_TRANSACTION_RRH     sizeof(struct trans_transaction_rrh)

/*struct trans_transaction_module  st_monitor;*/
struct trans_transaction_monitor
{
    //int32_t       w_monitor_socket; /*Socketfd*/
    //u_int16_t uw_connect_num;/*Connect Number total */
    //fun_callback f_callback;          /*Funtion Callback*/
    int32_t      w_msg_len;
    void  *       p_msg;

};

#define SIZEOF_TRANS_TRANSACTION_MONITOR     sizeof(struct trans_transaction_monitor)

/*struct trans_transaction_module  st_monitor;*/
struct trans_transaction_bs
{
    //int32_t       w_bs_socket; /*Socketfd*/
    //u_int16_t uw_connect_num;/*Connect Number total */
    //fun_callback f_callback;          /*Funtion Callback*/
    int32_t      w_msg_len;
    void  *       p_msg;

};

#define SIZEOF_TRANS_TRANSACTION_BS     sizeof(struct trans_transaction_bs)

/*struct trans_transaction_module  st_monitor;*/
struct trans_transaction_ms
{
    //int32_t       w_ms_socket; /*Socketfd*/
    //u_int16_t uw_connect_num;/*Connect Number total */
    //fun_callback f_callback;          /*Funtion Callback*/
    int32_t      w_msg_len;
    void  *       p_msg;

};

#define SIZEOF_TRANS_TRANSACTION_MS     sizeof(struct trans_transaction_ms)


struct trans_transaction_module
{
    u_int8_t      uc_used_flag;   /*0--use or 1--no use*/
    void *          p_module;    
};

#define SIZEOF_TRANS_TRANSACTION_MODULE     sizeof(struct trans_transaction_module)

/*Transaction*/
struct trans_transaction
{
    struct trans_transaction_module  st_common; /*struct trans_transaction_common*/
    struct trans_transaction_module  st_src_socket;       /*struct trans_transaction_socket*/
    struct trans_transaction_module  st_dst_socket;       /*struct trans_transaction_socket*/
    struct trans_transaction_module  st_func;       /*struct trans_transaction_func*/
    struct trans_transaction_module  st_timer;      /*struct trans_transaction_timer*/
    //struct trans_transaction_module  st_action;     /*struct trans_transaction_action*/
    struct trans_transaction_module  st_agent;      /*struct trans_transaction_agent*/
    struct trans_transaction_module  st_rrh;          /*struct trans_transaction_rrh*/
    struct trans_transaction_module  st_monitor;  /*struct trans_transaction_monitor*/
    struct trans_transaction_module  st_bs;     /*struct trans_transaction_bs*/
    struct trans_transaction_module  st_ms;     /*struct trans_transaction_ms*/
    //struct trans_transaction_module  st_wireless; /*struct trans_transaction_wireless*/
    struct trans_transaction_module  st_user;        /*struct trans_transaction_user*/
    struct trans_transaction_module  st_result;      /*struct trans_transaction_result*/

};

#define SIZEOF_TRANS_TRANSACTION     sizeof(struct trans_transaction)


/*****************************************************************************+
*extern
+*****************************************************************************/
extern u_int32_t trans_transaction_creat(void** pp_trans);
extern u_int32_t trans_transaction_delete(void *p_info);

extern u_int32_t trans_transaction_set_comn(void *p_info, 
                            u_int8_t uc_deleted,
                            u_int32_t  uw_src_module);
extern u_int32_t trans_transaction_get_comn_src(void *p_info);

extern u_int32_t trans_transaction_set_func(void *p_info, 
                            u_int8_t uc_exe_flag,
                            u_int32_t uw_func_id,
                            int32_t w_msg_len,
                            void *p_msg);
extern void * trans_transaction_get_func(void *p_info);

extern u_int32_t trans_transaction_set_dst(void *p_info, 
                            u_int32_t uw_num,
                            int32_t  w_sockfd,
                            u_int8_t *p_mac);

extern int32_t trans_transaction_get_dst_sock(void *p_info);

extern int32_t trans_transaction_get_dst_num(void *p_info);

extern void * trans_transaction_get_dst_mac(void *p_info);

extern u_int32_t trans_transaction_set_user(void *p_info, 
                           size_t len,
                           void * p_buf);
extern void * trans_transaction_get_user(void *p_info, int32_t *p_msg_len);

extern u_int32_t trans_transaction_set_timer(void *p_info, 
                           void * p_buf);

extern void * trans_transaction_get_timer(void *p_info);

extern int32_t trans_transaction_get_tra_id(void *p_info);

extern void * trans_transaction_get_src_mac(void *p_info);

extern u_int32_t trans_transaction_set_result(void *p_info, 
                           u_int32_t  uw_result);

extern int32_t trans_transaction_get_result(void *p_info);

extern u_int32_t trans_transaction_set_rrh(void *p_info,
                                        fun_callback f_callback,
                                        size_t len,
                                        void * p_rev_buf);
                                        
extern fun_callback trans_transaction_get_rrh(void *p_info);

extern u_int32_t trans_transaction_set_monitor(void *p_info,
                                        //fun_callback f_callback,
                                        size_t len,
                                        void * p_rev_buf);

extern u_int32_t trans_transaction_set_bs(void *p_info,
                                        size_t len,
                                        void * p_rev_buf);

extern u_int32_t trans_transaction_set_ms(void *p_info,
                                        //fun_callback f_callback,
                                        size_t len,
                                        void * p_rev_buf);

extern u_int32_t trans_transaction_set_agent(void *p_info, 
                           fun_callback f_callback,
                           size_t len,
                           void * p_rev_buf);

extern fun_callback trans_transaction_get_agent(void *p_info);

extern u_int32_t trans_transaction_get_in(u_int32_t uw_num,
                                                 u_int8_t  *p_mac,
                                                 void ** pp_info); 

extern u_int32_t trans_transaction_get_out(u_int32_t uw_num,
                                                 u_int8_t  *p_mac,
                                                 void ** pp_info); 

extern u_int32_t trans_transaction_get_out_by_mac(u_int32_t uw_num,
                                                 u_int8_t  *p_mac,
                                                 void ** pp_info); 

extern u_int32_t trans_transaction_get_out_by_ptr(void *p_info,
                                                 void ** pp_info);

extern u_int32_t trans_transaction_time_out(void *p_trans);

extern u_int32_t trans_transaction_clear(u_int8_t  *p_mac);

extern u_int32_t trans_transaction_init(void);

extern u_int32_t trans_transaction_release(void);





#endif /* TRANS_TRNSACTION_H_ */

