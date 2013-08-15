/*****************************************************************************+
*
*  File Name: trans_timer.h
*
*  Function: Timer
*
*  
*  Data:    2011-4-14
*  Modify:
*
+*****************************************************************************/

#ifndef TRANS_TIMER_H_
#define TRANS_TIMER_H_


#include <sys/types.h>
#include <sys/time.h>
#include <trans_agent.h>

/*****************************************************************************+
*Macro
+*****************************************************************************/

#define TRANS_TIMER_WINDOW 100LL


/*****************************************************************************+
*Enum
+*****************************************************************************/
enum trans_timer_compare_enum 
{   
   TRANS_TIMER_COMPARE_TV1 = 0,       /*TV1>TV2 */
   TRANS_TIMER_COMPARE_TV2 = 1,       /*TV1<TV2 */
   TRANS_TIMER_COMPARE_EQUAL = 2       /*TV1=TV2 */

};

enum trans_timer_window_enum 
{   
   TRANS_TIMER_OUT_WINDOW = 0 ,
   TRANS_TIMER_IN_WINDOW = 1, 
   TRANS_TIMER_WINDOW_ERROR = 2 

};


enum trans_timer_delete_enum 
{   
   TRANS_TIMER_NO_DELETE = 0,       
   TRANS_TIMER_DELETE = 1     

};


/*****************************************************************************+
*Data structure
+*****************************************************************************/
/*Funtion Callback*/
typedef int (*trans_timeout_action) (void *p_msg, size_t len, void *p_msg_info);

struct trans_timer_msg_info
{
    u_int32_t   uw_src_moudle;     /*Sender*/
    u_int16_t   us_serial_number;  /*serial_number for the sending message*/
    u_int8_t     uc_block_flag;
    fun_callback f_callback;
    /*
    union 
    {
        struct trans_agent_metric_info st_agent_metric;
        void * p_info;
    
    } u_extra_info;
    */
    void  *        p_user_info;/*Don't care its memory, it is controled by user in f_callback*/
};

#define SIZEOF_TRANS_TIMER_MSG_INFO   sizeof(struct trans_timer_msg_info)

#if 0
struct trans_timer{
    long long int   ll_timer_id;
    struct timeval st_expired;/*time out */
    u_int8_t uc_deleted;         /*Flag for Timer out*/
    //u_int32_t uw_flags;
    u_int32_t   uw_src_moudle;     /*Sender*/
    u_int16_t   us_serial_number;  /*serial_number for the sending message*/
    trans_timeout_action f_action;    /*Funtion Callback*/
    size_t len;
    u_int8_t* p_msg;
    //void* timer_specific_data;
};
#endif

struct trans_timer
{
    #if (defined TRANS_BS_COMPILE) || (defined TRANS_RRH_COMPILE)
    long long int   ll_timer_id;
    #endif
    #ifdef TRANS_MS_COMPILE
    int   ll_timer_id;
    #endif
    u_int8_t uc_deleted;         /*Flag for Timer out*/
    struct timeval st_expired;         /*time out */
    //trans_timeout_action f_action;    /*Funtion Callback*/
    struct trans_timer_msg_info st_msg_info;
    size_t len;
    u_int8_t* p_msg;
};


#define SIZEOF_TRANS_TIMER   sizeof(struct trans_timer)

/*****************************************************************************+
*extern
+*****************************************************************************/
extern u_int32_t trans_timer_compare(const struct timeval *tv1, const struct timeval *tv2);

extern void trans_timer_subtracte(const struct timeval *tv1, const struct timeval *tv2, 
                struct timeval *tvres);

extern u_int32_t trans_timer_window(const struct timeval *tv1, const struct timeval *tv2, 
                u_int32_t uw_window_len);

extern u_int32_t trans_timer_print_timeval(const char *str, const struct timeval *tv);

extern u_int32_t trans_timer_find_by_serial_num(u_int16_t us_s_num, 
                                            u_int8_t *p_msg_info, u_int8_t *p_find_flag);

extern u_int32_t trans_timer_add(const struct timeval *tv,
            trans_timeout_action f_ptr,
            void* p_data,
            size_t len,
            struct trans_timer_msg_info *p_msg_info,
            void** pp_timer_id);

extern u_int32_t trans_timer_delete_by_src(u_int32_t uw_src_moudle);

extern u_int32_t trans_timer_delete(void* p_timer_id);

extern void trans_timer_handler(void* p_timer_list);

extern u_int32_t trans_timer_init(void);

extern u_int32_t trans_timer_release(void);


extern struct trans_ordered_list *g_p_timer_list;



#endif /* TRANS_TIMER_H_ */


