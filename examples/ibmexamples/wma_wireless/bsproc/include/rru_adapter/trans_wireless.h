/*****************************************************************************+
*
*  File Name: trans_wireless.h
*
*  Function: TRANS WIRELESS MODULE BS / MS
*
*  
*  Data:    2011-07-14
*  Modify:
*
+*****************************************************************************/

#ifndef TRANS_WIRELESS_H_
#define TRANS_WIRELESS_H_


#include <sys/types.h>
#include <sys/time.h>

/*****************************************************************************+
*Macro
+*****************************************************************************/

//#define TRANS_TIMER_WINDOW 100LL

#define TRANS_WIRELESS_METRIC_MSG_MAX_LEN               10*4

/*****************************************************************************+
*Enum
+*****************************************************************************/
/*RRH State*/
enum trans_bs_action_enum 
{   
    TRANS_WIRELESS_ACTION_GET_BS_METRIC  = 0x00,       /*get metric from BS*/
    TRANS_WIRELESS_ACTION_GET_MS_METRIC  = 0x01,       /*get metric from MS*/
    TRANS_WIRELESS_ACTION_BUF = 0xff       /*BUF */

};

/*****************************************************************************+
*Data structure
+*****************************************************************************/
#if 0
/*Action for Get metric*/
struct trans_bs_action_get_metric
{
    int32_t w_metric_id;
    int32_t w_source_id;
};

#define SIZEOF_TRANS_BS_ACTION_GET_METRIC     sizeof(struct trans_bs_action_get_metric)
#endif


/*****************************************************************************+
*extern
+*****************************************************************************/

extern int trans_wireless_action_bs_metric(void *p_user_info, 
                                size_t len, 
                                void *p_action_msg);

extern int trans_wireless_action_ms_metric(void *p_user_info, 
                                size_t len, 
                                void *p_action_msg);

#if 0
extern u_int32_t trans_wireless_action_exe(u_int8_t uc_action, u_int32_t uw_src_moudle, 
                                u_int32_t uw_len, u_int8_t *p_msg);
#endif
extern u_int32_t trans_wireless_init(void);

extern u_int32_t trans_wireless_release(void);


#endif /* TRANS_WIRELESS_H_ */

