/* ----------------------------------------------------------------------------
 IBM Confidential

 IBM Wireless M2M Platform

 (C)Copyright IBM Corp. 2009, 2010, 2011

 All Rights Reserved.

 File Name: trans_action.h

 Change Activity:

 Date             Description of Change                            By
 -----------      ---------------------                            --------
 05-July.2011      Created                                          E Wulan

 ----------------------------------------------------------------------------
 PROLOG END TAG zYx                                                         */

#ifndef TRANS_ACTION_H_
#define TRANS_ACTION_H_


#include <sys/types.h>
//#include <sys/time.h>

/*****************************************************************************+
*Macro
+*****************************************************************************/

//#define TRANS_TIMER_WINDOW 100LL

#define TRANS_ACTION_DEFULT_INTERVAL  1   /*1s*/


/*****************************************************************************+
*Enum
+*****************************************************************************/

/*****************************************************************************+
*Data structure
+*****************************************************************************/
struct trans_action_list
{
    struct trans_unordered_list * p_unordered_list;
    u_int32_t   uw_execute_action;
    u_int32_t   uw_action_id;
    pthread_mutex_t   a_mutex;

};

#define SIZEOF_TRANS_ACTION_LIST   sizeof(struct trans_action_list)

/*Action Info for Fill Action List*/
struct trans_action_info
{
    struct trans_action_list * p_action_list;
    u_int32_t   uw_src_moudle;    
    fun_callback f_callback;
    void * p_info;    /**/
};
    
#define SIZEOF_TRANS_ACTION_INFO   sizeof(struct trans_action_info)


/*Action node*/
struct trans_action
{
    u_int32_t   uw_action_id;
    u_int32_t   uw_src_moudle;
    fun_callback f_callback;
    u_int32_t   uw_msg_len;
    void*          p_msg;
    void*         p_info;/*Don't care its memory, it is controled by user in f_callback*/

};

#define SIZEOF_TRANS_ACTION   sizeof(struct trans_action)

/*****************************************************************************+
*extern
+*****************************************************************************/
extern u_int32_t trans_action_fun_exe(struct trans_action * p_action);

extern u_int32_t trans_action_add(struct trans_action_info *p_action_info,
                            size_t len,
                            void * p_action_msg);

extern void trans_action_handler(struct trans_action_list * p_action_list);

//extern u_int32_t trans_action_delete_by_src(u_int32_t uw_src_moudle);

extern u_int32_t trans_action_init(void);

extern u_int32_t trans_action_release(void);


extern struct trans_action_list g_trans_action_list;


#endif /* TRANS_ACTION_H_ */


