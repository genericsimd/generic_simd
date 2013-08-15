/* ----------------------------------------------------------------------------
 IBM Confidential

 IBM Wireless M2M Platform

 (C)Copyright IBM Corp. 2009, 2010, 2011

 All Rights Reserved.

 File Name: trans_timer.h

 Change Activity:

 Date             Description of Change                            By
 -----------      ---------------------                            --------
 01-Mar.2012      Created                                          E Wulan

 ----------------------------------------------------------------------------
 PROLOG END TAG zYx                                                         */


#ifndef TRANS_TIMER_H_
#define TRANS_TIMER_H_


#include <sys/types.h>
//#include <sys/time.h>
#include <trans_list.h>



/*****************************************************************************+
*Macro
+*****************************************************************************/

#define TRANS_TIMER_WINDOW 100LL
#define TRANS_TIMER_DEFULT_INTERVAL  500   /*500ms*/


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

enum trans_timer_state_enum
{
    TRANS_TIMER_STATE_IDLE = 0,  
    TRANS_TIMER_STATE_ALIVE,  
    TRANS_TIMER_STATE_STOP, 
    TRANS_TIMER_STATE_TIMEOUT  
};

enum trans_timer_type_enum
{
    TRANS_TIMER_TYPE_ONCE = 0,  
    TRANS_TIMER_TYPE_CIRCLE   
};


/*****************************************************************************+
*Data structure
+*****************************************************************************/
/*Funtion Callback*/
typedef int (*trans_timeout_action) (void *p_msg, size_t len, void *p_msg_info);

struct trans_timer_list
{
    struct trans_unordered_list * p_unordered_list;
    u_int32_t   uw_execute_timer;
    u_int32_t   uw_timer_id;
    pthread_mutex_t   t_mutex;

};

#define SIZEOF_TRANS_TIMER_LIST   sizeof(struct trans_timer_list)


struct trans_timer_info
{
    struct trans_timer_list * p_timer_list;
    u_int32_t uw_interval;  //time out interval    ---s
    u_int8_t uc_type;        //type  enum trans_timer_type_enum
    fun_callback f_callback;//function callbck
    void * p_data;              //data

};

#define SIZEOF_TRANS_TIMER_INFO   sizeof(struct trans_timer_info)

struct trans_timer
{
    u_int32_t uw_timer_id;
    u_int32_t uw_interval;  //interval fixed    ----ms
    u_int32_t uw_counter;  //interval decrease  -----ms
    u_int8_t uc_state;         //state  enum trans_timer_state_enum
    u_int8_t uc_type;          //type  enum trans_timer_type_enum
    fun_callback f_callback;
    void * p_data;              //data
};

#define SIZEOF_TRANS_TIMER   sizeof(struct trans_timer)

/*****************************************************************************+
*extern
+*****************************************************************************/
extern u_int32_t trans_timer_add(struct trans_timer_info * p_timer_info,
            void** pp_timer);

extern u_int32_t trans_timer_delete(struct trans_timer_list * p_timer_list,
                                        void* p_timer);

extern void trans_timer_handler(struct trans_timer_list * p_timer_list);

extern u_int32_t trans_timer_init(void);

extern u_int32_t trans_timer_release(void);


extern struct trans_timer_list g_trans_timer_list;


#endif /* TRANS_TIMER_H_ */


