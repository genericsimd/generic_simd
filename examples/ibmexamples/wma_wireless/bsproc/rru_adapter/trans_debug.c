/*****************************************************************************+
*
*  File Name: trans_debug.c
*
*  Function: debug
*
*  
*  Data:    2011-03-23
*  Modify:
*
+*****************************************************************************/


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
    
    if(1 != uc_module)
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

