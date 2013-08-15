/* ----------------------------------------------------------------------------
 IBM Confidential

 IBM Wireless M2M Platform

 (C)Copyright IBM Corp. 2009, 2010, 2011

 All Rights Reserved.

 File Name: trans_wireless.c

 Change Activity:

 Date             Description of Change                            By
 -----------      ---------------------                            --------
 14-Apr.2011      Created                                          E Wulan

 ----------------------------------------------------------------------------
 PROLOG END TAG zYx                                                         */


#ifndef TRANS_WIRELESS_H_
#define TRANS_WIRELESS_H_


#include <sys/types.h>
#include <sys/time.h>

/*****************************************************************************+
*Macro
+*****************************************************************************/

//#define TRANS_TIMER_WINDOW 100LL

#define TRANS_WIRELESS_METRIC_MSG_MAX_LEN               10240

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

struct trans_wireless_config_rrh
{
    u_int16_t   us_tag;        /*Tag*/                                                
    u_int32_t   uw_value;  /*Value Length*/
}__attribute__ ((packed));

#define SIZEOF_TRANS_WIRELESS_CONFIG_RRH     sizeof(struct trans_wireless_config_rrh)

/*****************************************************************************+
*extern
+*****************************************************************************/

extern u_int32_t trans_wireless_send2_rrh_ch_mode(void *p_info, 
                           size_t len,
                           void * p_value);

extern u_int32_t trans_wireless_send2_rrh_ch_flag(void *p_info, 
                           size_t len,
                           void * p_value);

extern u_int32_t trans_wireless_send2_rrh_ch_freq(void *p_info, 
                           size_t len,
                           void * p_value);


extern u_int32_t trans_wireless_send2_rrh_dl_pre_time(void *p_info, 
                           size_t len,
                           void * p_value);

extern u_int32_t trans_wireless_send2_rrh_tx_len(void *p_info, 
                           size_t len,
                           void * p_value);

extern u_int32_t trans_wireless_send2_rrh_rx_len(void *p_info, 
                           size_t len,
                           void * p_value);


extern u_int32_t trans_wireless_send2_rrh_pa_switch(void *p_info, 
                           size_t len,
                           void * p_value);

extern u_int32_t trans_wireless_send2_rrh_gps_enable(void *p_info, 
                           size_t len,
                           void * p_value);

extern u_int32_t trans_wireless_send2_rrh_output_pow(void *p_info, 
                           size_t len,
                           void * p_value);

extern u_int32_t trans_wireless_send2_rrh_agc_enable(void *p_info, 
                           size_t len,
                           void * p_value);

extern u_int32_t trans_wireless_send2_rrh_ch_rx_pgc(void *p_info, 
                           size_t len,
                           void * p_value);

extern u_int32_t trans_wireless_send2_rrh_carrier_info(void *p_info, 
                           size_t len,
                           void * p_value);

extern u_int32_t trans_wireless_send2_rrh_byte_order(void *p_info, 
                           size_t len,
                           void * p_value);

extern u_int32_t trans_wireless_send2_rrh_ttg_rtg(void *p_info, 
                           size_t len,
                           void * p_value);

extern u_int32_t trans_wireless_send2_rrh_data_port(void *p_info, 
                           size_t len,
                           void * p_value);

extern u_int32_t trans_wireless_send2_rrh_q_gps(void *p_info, 
                           size_t len,
                           void * p_value);

extern u_int32_t trans_wireless_send2_rrh_q_power(void *p_info, 
                           size_t len,
                           void * p_value);

extern u_int32_t trans_wireless_send2_rrh_q_rru_id(void *p_info, 
                           size_t len,
                           void * p_value);

extern u_int32_t trans_wireless_send2_rrh_q_gps_lock(void *p_info, 
                           size_t len,
                           void * p_value);

extern u_int32_t trans_wireless_send2_agent_spectrum(void *p_info, 
                           size_t len,
                           void * p_value);

extern u_int32_t trans_wireless_send2_agent_periodic(void *p_info, 
                           size_t len,
                           void * p_value);

extern u_int32_t trans_wireless_send2_agent_topology(void *p_info, 
                           size_t len,
                           void * p_value);


extern int trans_wireless_action_bs_metric(void *p_info, 
                           size_t len,
                           void * p_rev_buf);

extern int trans_wireless_action_ms_metric(void *p_info, 
                           size_t len,
                           void * p_rev_buf);

extern u_int32_t trans_wireless_send2_monitor_hook_resp(u_int8_t uc_ack_flag,
                                            u_int32_t uw_transaction, 
                                            size_t len,
                                            void * p_value);


extern u_int32_t trans_wireless_tcp_socket(u_int32_t uw_ip_addr, u_int16_t us_port);



#if 0
extern u_int32_t trans_wireless_action_exe(u_int8_t uc_action, u_int32_t uw_src_moudle, 
                                u_int32_t uw_len, u_int8_t *p_msg);
#endif
extern u_int32_t trans_wireless_init(void);

extern u_int32_t trans_wireless_release(void);


#endif /* TRANS_WIRELESS_H_ */

