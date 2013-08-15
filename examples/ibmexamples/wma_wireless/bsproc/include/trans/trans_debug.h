/* ----------------------------------------------------------------------------
 IBM Confidential

 IBM Wireless M2M Platform

 (C)Copyright IBM Corp. 2009, 2010, 2011

 All Rights Reserved.

 File Name: trans_debug.c

 Change Activity:

 Date             Description of Change                            By
 -----------      ---------------------                            --------
 23-Mar.2011      Created                                          E Wulan

 ----------------------------------------------------------------------------
 PROLOG END TAG zYx                                                         */



#ifndef TRANS_DEBUG_H_
#define TRANS_DEBUG_H_


#include <sys/types.h>
//#include "flog.h"


//extern void trans_msg_print(u_int8_t *p_msg, u_int16_t us_print_len, u_int16_t us_msg_len);
extern void trans_debug_msg_print(void *p_msg, u_int32_t uw_print_len, u_int8_t   uc_module);

extern u_int8_t g_trans_debug_com;
extern u_int8_t g_trans_debug_rrh;
extern u_int8_t g_trans_debug_agent;
extern u_int8_t g_trans_debug_action;
extern u_int8_t g_trans_debug_monitor;
extern u_int8_t g_trans_debug_wireless;
extern u_int8_t g_trans_debug_timer;
extern u_int8_t g_trans_debug_list;
extern u_int8_t g_trans_debug_device;
extern u_int8_t g_trans_debug_transaction;



#if LOG_LEVEL >= LOG_LEVEL_FATAL
#define FLOG_FATAL_TRANS(flag, fmt, args...)   \
                if (1 == flag) \
                    flog_err("FATAL", LOG_CRIT, __FILE__, __FUNCTION__, fmt, ##args)
#else
#define FLOG_FATAL_TRANS(flag, fmt, args...)
#endif
                
#if LOG_LEVEL >= LOG_LEVEL_ERROR
#define FLOG_ERROR_TRANS(flag, fmt, args...)   \
                if (1 == flag) \
                    flog_err("ERROR", LOG_ERR, __FILE__, __FUNCTION__, fmt, ##args)
#else
#define FLOG_ERROR_TRANS(flag, fmt, args...)
#endif
                
#if LOG_LEVEL >= LOG_LEVEL_WARN
#define FLOG_WARNING_TRANS(flag, fmt, args...)   \
                if (1 == flag) \
                    flog_info("WARNING", LOG_WARNING, __FILE__, __FUNCTION__, fmt, ##args)
#else
#define FLOG_WARNING_TRANS(flag, fmt, args...)
#endif
                
#if LOG_LEVEL >= LOG_LEVEL_INFO
#define FLOG_INFO_TRANS(flag, fmt, args...)   \
                if (1 == flag) \
                    flog_info("INFO", LOG_INFO, __FILE__, __FUNCTION__, fmt, ##args)
#else
#define FLOG_INFO_TRANS(flag, fmt, args...)
#endif
                
#if LOG_LEVEL >= LOG_LEVEL_DEBUG
#define FLOG_DEBUG_TRANS(flag, fmt, args...)   \
                if (1 == flag) \
                    flog_debug("DEBUG", LOG_DEBUG, __FILE__, __LINE__, __FUNCTION__, fmt, ##args)
#else
#define FLOG_DEBUG_TRANS(flag, fmt, args...)
#endif


            


#endif
