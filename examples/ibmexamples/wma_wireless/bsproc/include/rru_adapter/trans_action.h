/*****************************************************************************+
*
*  File Name: trans_action.h
*
*  Function: Action
*
*  
*  Data:    2011-07-14
*  Modify:
*
+*****************************************************************************/

#ifndef TRANS_ACTION_H_
#define TRANS_ACTION_H_


#include <sys/types.h>
#include <sys/time.h>

/*****************************************************************************+
*Macro
+*****************************************************************************/

//#define TRANS_TIMER_WINDOW 100LL

//#define TRANS_         256

/*****************************************************************************+
*Enum
+*****************************************************************************/

/*****************************************************************************+
*Data structure
+*****************************************************************************/
/*Action Info for Fill Action List*/
struct trans_action_info
{
    struct timeval  st_tv;
    u_int32_t   uw_src_moudle;    
    //u_int8_t   uc_action;
    fun_callback f_callback;
    void * p_user_info; /*Don't care its memory, it is controled by user in f_callback*/
};
    
#define SIZEOF_TRANS_ACTION_INFO   sizeof(struct trans_action_info)


/*Action node*/
struct trans_action
{
    u_int32_t   uw_action_id;
    u_int32_t   uw_src_moudle;
    //u_int8_t     uc_action;
    fun_callback f_callback;
    u_int32_t   uw_msg_len;
    u_int8_t*   p_msg;
    void  *        p_user_info;/*Don't care its memory, it is controled by user in f_callback*/

};

#define SIZEOF_TRANS_ACTION   sizeof(struct trans_action)

/*****************************************************************************+
*extern
+*****************************************************************************/
extern u_int32_t  trans_action_get_action_id(void);

extern u_int32_t trans_action_fun_exe(struct trans_action * p_action);

extern u_int32_t trans_action_add(struct trans_action_info *p_action_info,
                            size_t len,
                            void * p_action_msg);

extern void trans_action_handler(void* p_action_list);

extern u_int32_t trans_action_delete_by_src(u_int32_t uw_src_moudle);

extern u_int32_t trans_action_init(void);

extern u_int32_t trans_action_release(void);


extern struct trans_ordered_list *g_trans_action_list;

#endif /* TRANS_ACTION_H_ */


