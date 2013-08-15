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


#include <sys/types.h>
#include <malloc.h>
#include <syslog.h>
#include <flog.h>

u_int8_t g_trans_debug_com = 0;
u_int8_t g_trans_debug_rrh = 0;
u_int8_t g_trans_debug_agent = 0;
u_int8_t g_trans_debug_action = 0;
u_int8_t g_trans_debug_monitor = 0;
u_int8_t g_trans_debug_wireless = 0;
u_int8_t g_trans_debug_timer = 0;
u_int8_t g_trans_debug_list = 0;
u_int8_t g_trans_debug_device = 0;
u_int8_t g_trans_debug_transaction = 0;
u_int8_t g_trans_debug_msg = 0;


/*****************************************************************************+
 * Function: trans_debug_msg_print()*
 * Description: Message Print
 * Parameters:
 *           NONE
 * Return Values:
 *           NONE
 *
 *  
 *  Data:    2011-04-08
 *
 +******************************************************************************/
void trans_debug_msg_print(void *p_msg, u_int32_t uw_print_len, u_int8_t uc_module)
{

    u_int32_t uw_index = 0;
    u_int8_t * p_print_msg = NULL;

    if (NULL == p_msg)
    {
        return;
    }

    p_print_msg = (u_int8_t *)p_msg;
    
    if((1 != uc_module) || (1 != g_trans_debug_msg))
    {

        return;
    }

    FLOG_WARNING ("-------------------trans_debug_msg_print----------------------\r\n");

    for (uw_index = 0; uw_index < uw_print_len; uw_index++)
    {

        FLOG_WARNING ("0x%x", p_print_msg[uw_index]);
    }

    return;
}

