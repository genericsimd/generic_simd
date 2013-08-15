/* ----------------------------------------------------------------------------
 IBM Confidential

 IBM Wireless M2M Platform

 (C)Copyright IBM Corp. 2009, 2010, 2011

 All Rights Reserved.

 File Name: trans_device.c

 Change Activity:

 Date             Description of Change                            By
 -----------      ---------------------                            --------
 01-Dec.2011      Created                                          E Wulan

 ----------------------------------------------------------------------------
 PROLOG END TAG zYx                                                         */


#include <sys/types.h>
#include <syslog.h>
#include <flog.h>

#include <trans.h>
#include <trans_common.h>
#include <trans_list.h>
#include <trans_device.h>
//#include <trans_rrh.h>
//#include <trans_agent.h>
//#include <trans_wireless.h>
//#include <trans_action.h>
//#include <trans_timer.h>
//#include <trans_list.h>
#include <trans_debug.h>
#include <trans_monitor.h>


/*****************************************************************************+
 *Global Variables 
+*****************************************************************************/
struct trans_unordered_list * g_trans_device_list = NULL;

u_int32_t g_execute_device = 1;

u_int32_t g_trans_device_id = 1;
pthread_mutex_t  g_trans_device_id_mutex;

extern struct trans_common_sockfd g_trans_common_sockfd;

/*****************************************************************************+
 *Code 
+*****************************************************************************/

/*****************************************************************************+
* Function: trans_device_cmp()
* Description: 
* Parameters:
*           NONE
* Return Values:
*           TRANS_FAILD : 1
*           TRANS_SUCCESS  : 0
*
*  
*  Data:    2012-03-06
* 
+*****************************************************************************/
int trans_device_cmp(void *p_data, void *p_key, void *p_result)
{
    struct trans_device * p_device = NULL;   
    u_int8_t  *p_mac = NULL;
    u_int8_t  *p_cmp = NULL;

    #if 0
    if ((NULL == p_data) || (NULL == p_key) || (NULL == p_result))
    {
        FLOG_ERROR("NULL PTR. \r\n");        
        return TRANS_FAILD;     
    }
    #endif
    
    p_device = (struct trans_device *)p_data;
    p_mac = (u_int8_t  *)p_key;
    p_cmp = (u_int8_t  *)p_result;

    if (trans_mac_addr_cmp(p_device->a_mac, p_mac))
    {   
        *p_cmp = 1;

    } 
    else
    {
        *p_cmp = 0;
    }
    
    FLOG_DEBUG_TRANS(g_trans_debug_device, "Exit %d\r\n", *p_cmp); 
    
    return TRANS_SUCCESS;

}

/*****************************************************************************+
* Function: trans_device_get_in()
* Description: Initializing
* Parameters:
*           NONE
* Return Values:
*           TRANS_FAILD : 1
*           TRANS_SUCCESS  : 0
*
*  
*  Data:    2012-03-06
* 
+*****************************************************************************/
u_int32_t trans_device_get_in(struct trans_unordered_list * p_unordered_list,
                                                 u_int8_t  *p_mac,
                                                 void ** pp_data) 
{
    u_int32_t uw_ret = 0;
    
    FLOG_DEBUG_TRANS(g_trans_debug_device, "Enter\r\n"); 
    
    *pp_data = NULL;
    
    uw_ret = trans_unordered_list_get_in(p_unordered_list, 
                            p_mac,
                            trans_device_cmp,
                            pp_data);
    
    if(TRANS_SUCCESS != uw_ret) 
    {
        FLOG_ERROR("Call trans_unordered_list_get_in error!uw_ret = %d \r\n", uw_ret);
        return TRANS_FAILD;    
    }

    return TRANS_SUCCESS;
}

/*****************************************************************************+
* Function: trans_device_get_out()
* Description: Initializing
* Parameters:
*           NONE
* Return Values:
*           TRANS_FAILD : 1
*           TRANS_SUCCESS  : 0
*
*  
*  Data:    2012-03-06
* 
+*****************************************************************************/
u_int32_t trans_device_get_out(struct trans_unordered_list * p_unordered_list,
                                                 u_int8_t  *p_mac,
                                                 void ** pp_data) 
{
    u_int32_t uw_ret = 0;

    * pp_data = NULL;
    
    uw_ret = trans_unordered_list_get_out(p_unordered_list, 
                            p_mac,
                            trans_device_cmp,
                            pp_data);
    
    if(TRANS_SUCCESS != uw_ret) 
    {
        FLOG_ERROR("Call trans_unordered_list_get_out error!uw_ret = %d \r\n", uw_ret);
        return TRANS_FAILD;    
    }

    return TRANS_SUCCESS;
}


/*****************************************************************************+
* Function: trans_device_get_plus()
* Description: 
* Parameters:
*           NONE
* Return Values:
*           TRANS_FAILD : 1
*           TRANS_SUCCESS  : 0
*
*  
*  Data:    2011-12-01
* 
+*****************************************************************************/
void trans_device_get_plus(u_int8_t *p_mac, void **pp_info)
{
    struct trans_unordered_list *p_unordered_list = g_trans_device_list;

    struct trans_device * p_device = NULL;
    u_int32_t uw_ret = 0;

    *pp_info = NULL;

    if (NULL == p_unordered_list)
    {
        FLOG_ERROR("Device list is not initialized. \r\n");
        
        return ;     
    }

    if (NULL == p_mac)
    {
        FLOG_ERROR("NULL PTR, p_mac. \r\n");
        
        return ;     
    }

    FLOG_DEBUG_TRANS(g_trans_debug_device, "MAC :0x%x:0x%x:0x%x:0x%x:0x%x:0x%x..\r\n", 
                        p_mac[0], p_mac[1], p_mac[2], p_mac[3], p_mac[4], p_mac[5]); 

    uw_ret =  trans_device_get_in(p_unordered_list, p_mac, pp_info); 
    if(TRANS_SUCCESS != uw_ret) 
    {
        FLOG_ERROR("Call trans_device_get_in error!uw_ret = %d \r\n", uw_ret);
        return ;    
    }
    
    if (NULL == *pp_info)
    {
        FLOG_DEBUG_TRANS(g_trans_debug_device, "Do not find the device info!\r\n");
        return ;    
    }
    
    p_device = (struct trans_device *)*pp_info;

    p_device->uw_used_num++;
   
    FLOG_DEBUG_TRANS(g_trans_debug_device, "Exit find :w_sockfd = %d, type = %d.\r\n", 
                        p_device->w_sockfd, p_device->uc_module_type); 

    return ;
}

/*****************************************************************************+
* Function: trans_device_get_sub()
* Description: 
* Parameters:
*           NONE
* Return Values:
*           TRANS_FAILD : 1
*           TRANS_SUCCESS  : 0
*
*  
*  Data:    2011-12-01
* 
+*****************************************************************************/
void trans_device_get_sub(void *p_info)
{
    struct trans_unordered_list *p_unordered_list = g_trans_device_list;

    struct trans_device * p_device = NULL;
    u_int32_t uw_ret = 0;
    
    void* p_device_ptr = NULL;
    
    if (NULL == p_unordered_list)
    {
        FLOG_ERROR("Device list is not initialized. \r\n");        
        return ;     
    }

    if (NULL == p_info)
    {
        FLOG_ERROR("NULL PTR, p_info. \r\n");        
        return ;     
    }

    p_device = (struct trans_device * )p_info; 

    FLOG_DEBUG_TRANS(g_trans_debug_device, "MAC :0x%x:0x%x:0x%x:0x%x:0x%x:0x%x.\r\n", 
                        p_device->a_mac[0], p_device->a_mac[1], p_device->a_mac[2], p_device->a_mac[3], p_device->a_mac[4], p_device->a_mac[5]); 

    p_device->uw_used_num--;
    
    /*Delete Node from list*/
    if ((0 == p_device->uw_used_num) && (1 == p_device->uc_delete_flag))
    {
        uw_ret =  trans_device_get_out(p_unordered_list, p_device->a_mac, &p_device_ptr); 
        if(TRANS_SUCCESS != uw_ret) 
        {
            FLOG_ERROR("Call trans_device_get_out error!uw_ret = %d \r\n", uw_ret);
            return ;    
        }
    
        if (NULL == p_device_ptr)
        {
            FLOG_ERROR("Dn not find the device info!\r\n");
            return ;    
        }
        
        free (p_device);
    }

    return ;
}


/*****************************************************************************+
* Function: trans_device_get_index()
* Description: calculate and return the device index
* Parameters:
*           NONE
* Return Values:
*           uw_action_id
*
*  
*  Data:    2011-12-01
* 
+*****************************************************************************/
u_int32_t  trans_device_get_index(void) 
{
    u_int32_t uw_device_index = 0;
    
    pthread_mutex_lock(&(g_trans_device_id_mutex));  
    
    uw_device_index = g_trans_device_id;

    g_trans_device_id++;

    /*Can't set the g_trans_rrh_serial_number = 0, 0 means nothing*/
    if (0xffffffff == g_trans_device_id)
    {
        g_trans_device_id = 1;

    }
    
    pthread_mutex_unlock(&(g_trans_device_id_mutex));
    
    FLOG_DEBUG_TRANS(g_trans_debug_device, "Exit uw_device_index = %d\r\n", uw_device_index); 
    
    return(uw_device_index);
}

/*****************************************************************************+
* Function: trans_device_get_serial_num()
* Description: calculate and return the serial number
* Parameters:
*           NONE
* Return Values:
*           uw_serial_num
*
*  
*  Data:    2011-09-14
* 
+*****************************************************************************/
u_int32_t  trans_device_get_serial_num(u_int8_t *p_mac) 
{
    //struct trans_device_ordered_list *p_ordered_list = g_trans_device_list;
    u_int32_t uw_serial_num = 0;
    
    //u_int32_t uw_ret = 0;
    void * p_info = NULL;
    
    struct trans_device * p_device = NULL;   

    trans_device_get_plus(p_mac, &p_info);
   
    if (NULL == p_info)
    {
        FLOG_ERROR("Do not find the device info!\r\n");
        return 0;    
    }
    
    p_device = (struct trans_device *)p_info;

    if (TRANS_DEVICE_DELETE == p_device->uc_delete_flag)
    {
        FLOG_DEBUG_TRANS(g_trans_debug_device, "The device is delete!\r\n");
        return 0;    
    }
    
    pthread_mutex_lock(&(p_device->dev_mutex));  
    
    uw_serial_num = p_device->uw_serial_num;
    
    (p_device->uw_serial_num)++;
    
    /*Can't set the g_trans_rrh_serial_number = 0, 0 means nothing*/
    if (0xffffffff == p_device->uw_serial_num)
    {
        p_device->uw_serial_num = 1;
    
    }
    
    pthread_mutex_unlock(&(p_device->dev_mutex));

    trans_device_get_sub(p_info);
    
    FLOG_DEBUG_TRANS(g_trans_debug_device, "Exit uw_serial_num = %d\r\n", uw_serial_num); 
    
    return(uw_serial_num);

}

/*****************************************************************************+
* Function: trans_device_get_congestion_num()
* Description: calculate and return the serial number
* Parameters:
*           NONE
* Return Values:
*           uw_serial_num
*
*  Data:    2012-05-07
* 
+*****************************************************************************/
u_int32_t  trans_device_get_congestion_num(u_int8_t *p_mac, u_int8_t uc_flag) 
{
    //struct trans_device_ordered_list *p_ordered_list = g_trans_device_list;
    u_int32_t uw_congestion_num = 0;    
    //u_int32_t uw_ret = 0;
    void * p_info = NULL;
    
    struct trans_device * p_device = NULL;   

    trans_device_get_plus(p_mac, &p_info);
   
    if (NULL == p_info)
    {
        FLOG_ERROR("Do not find the device info!\r\n");
        return 0;    
    }
    
    p_device = (struct trans_device *)p_info;

    if (TRANS_DEVICE_DELETE == p_device->uc_delete_flag)
    {
        FLOG_DEBUG_TRANS(g_trans_debug_device, "The device is delete!\r\n");
        return 0;    
    }
    
    pthread_mutex_lock(&(p_device->dev_mutex));  
    
    uw_congestion_num = p_device->uw_congestion_num;

    if (1 <= p_device->uw_congestion_num)
    {
        (p_device->uw_congestion_num)++;

        if (10 == p_device->uw_congestion_num)
        {
            p_device->uw_congestion_num = 0;
        }            
    }

    if (1 == uc_flag)
    {
        (p_device->uw_congestion_num) = 1;
    }
    else if (2 == uc_flag)
    {
        p_device->uw_congestion_num = 0;
    }
    else if (0 == uc_flag)
    {

    }
    else
    {
        FLOG_ERROR("uc_flag = %d error!\r\n", uc_flag);
    }
    
    pthread_mutex_unlock(&(p_device->dev_mutex));

    trans_device_get_sub(p_info);
    
    FLOG_DEBUG_TRANS(g_trans_debug_device, "Exit uw_congestion_num = %d\r\n", uw_congestion_num); 
    
    return(uw_congestion_num);

}


/*****************************************************************************+
* Function: trans_device_get_transaction_id()
* Description: calculate and return the transaction ID
* Parameters:
*           NONE
* Return Values:
*           uw_serial_num
*
*  
*  Data:    2011-09-14
* 
+*****************************************************************************/
u_int32_t  trans_device_get_transaction_id(u_int8_t *p_mac) 
{
    //struct trans_device_ordered_list *p_ordered_list = g_trans_device_list;
    u_int32_t uw_transaction = 0;
    //u_int32_t uw_ret = 0;
    void * p_info = NULL;
    
    FLOG_DEBUG_TRANS(g_trans_debug_device, "Enter \r\n"); 

    struct trans_device * p_device = NULL;   
    
    trans_device_get_plus(p_mac, &p_info);
    
    if (NULL == p_info)
    {
        FLOG_ERROR("Dn not find the device info!\r\n");
        return 0;    
    }
    
    p_device = (struct trans_device *)p_info;
    
    if (TRANS_DEVICE_DELETE == p_device->uc_delete_flag)
    {
        FLOG_DEBUG_TRANS(g_trans_debug_device, "The device is delete!\r\n");
        return 0;    
    }

    pthread_mutex_lock(&(p_device->dev_mutex));  
    
    uw_transaction = p_device->uw_transaction;

    (p_device->uw_transaction)++;

    /*Can't set the g_trans_rrh_serial_number = 0, 0 means nothing*/
    if (0xffffffff == p_device->uw_transaction)
    {
        p_device->uw_transaction = 1;

    }
    
    pthread_mutex_unlock(&(p_device->dev_mutex));
    
    trans_device_get_sub(p_info);

    FLOG_DEBUG_TRANS(g_trans_debug_device, "Exit uw_transaction = %d\r\n", uw_transaction); 
    return(uw_transaction);
}


/*****************************************************************************+
* Function: trans_device_get_socket()
* Description: calculate and return the socket ID
* Parameters:
*           NONE
* Return Values:
*           uw_serial_num
*
*  
*  Data:    2011-09-14
* 
+*****************************************************************************/
int32_t  trans_device_get_socket(u_int8_t *p_mac) 
{
    //struct trans_device_ordered_list *p_ordered_list = g_trans_device_list;
    int32_t w_sockfd = 0;
    //u_int32_t uw_ret = 0;
    void * p_info = NULL;
    
    struct trans_device * p_device = NULL;   
    
    FLOG_DEBUG_TRANS(g_trans_debug_device, "Enter \r\n"); 

    trans_device_get_plus(p_mac, &p_info);
    
    if (NULL == p_info)
    {
        FLOG_ERROR("Dn not find the device info!\r\n");
        return -1;    
    }
    
    p_device = (struct trans_device *)p_info;

    if (TRANS_DEVICE_DELETE == p_device->uc_delete_flag)
    {
        FLOG_DEBUG_TRANS(g_trans_debug_device, "The device is delete!\r\n");
        return 0;    
    }
    
    pthread_mutex_lock(&(p_device->dev_mutex));  
    
    w_sockfd = p_device->w_sockfd;
    
    pthread_mutex_unlock(&(p_device->dev_mutex));

    trans_device_get_sub(p_info);
    
    FLOG_DEBUG_TRANS(g_trans_debug_device, "Exit w_sockfd = %d\r\n", w_sockfd); 

    return(w_sockfd);
}

/*****************************************************************************+
* Function: trans_device_get_type()
* Description: calculate and return the device type
* Parameters:
*           NONE
* Return Values:
*           uw_serial_num
*
*  
*  Data:    2011-09-14
* 
+*****************************************************************************/
int32_t  trans_device_get_type(u_int8_t *p_mac) 
{
    //struct trans_device_ordered_list *p_ordered_list = g_trans_device_list;
    int32_t w_type = 0;
    //u_int32_t uw_ret = 0;
    void * p_info = NULL;
    
    FLOG_DEBUG_TRANS(g_trans_debug_device, "Enter \r\n"); 

    struct trans_device * p_device = NULL;   
    
    trans_device_get_plus(p_mac, &p_info);

    if (NULL == p_info)
    {
        FLOG_ERROR("Dn not find the device info!\r\n");
        return -1;    
    }
    
    p_device = (struct trans_device *)p_info;

    if (TRANS_DEVICE_DELETE == p_device->uc_delete_flag)
    {
        FLOG_DEBUG_TRANS(g_trans_debug_device, "The device is delete!\r\n");
        return 0;    
    }
    
    pthread_mutex_lock(&(p_device->dev_mutex));  
    
    w_type = p_device->uc_module_type;
    
    pthread_mutex_unlock(&(p_device->dev_mutex));

    trans_device_get_sub(p_info);
    
    FLOG_DEBUG_TRANS(g_trans_debug_device, "Exit w_type = %d\r\n", w_type); 

    return(w_type);
}

/*****************************************************************************+
* Function: trans_device_common_add()
* Description: 
* Parameters:
*           NONE
* Return Values:
*           TRANS_FAILD : 1
*           TRANS_SUCCESS  : 0
*
*  
*  Data:    2011-12-01
* 
+*****************************************************************************/
static void trans_device_common_add(int32_t w_sockfd,
                            u_int8_t uc_module_type,
                            u_int8_t *p_mac)
{
    u_int32_t uw_use_num = 0;
    
    if (0 >= w_sockfd)
    {
        FLOG_ERROR("Sockfd error :%d! \r\n", w_sockfd);
        return ;
    }
    
    if (NULL == p_mac)
    {
        FLOG_ERROR("NULL PTR! \r\n");
        return ;
    }  

    pthread_mutex_lock(&(g_trans_common_sockfd.s_mutex));  

    uw_use_num = g_trans_common_sockfd.uw_num;
           
    g_trans_common_sockfd.st_sockfd[uw_use_num].w_sockfd= w_sockfd;
    
    g_trans_common_sockfd.st_sockfd[uw_use_num].uc_module_type = uc_module_type;
    
    memcpy(g_trans_common_sockfd.st_sockfd[uw_use_num].a_mac,
                    p_mac, TRANS_MAC_ADDR_LEN);
    
    g_trans_common_sockfd.uw_num++;  
    
    pthread_mutex_unlock(&(g_trans_common_sockfd.s_mutex));  

    
    FLOG_DEBUG_TRANS(g_trans_debug_device, "Num: %d, Device info:  mac = 0x%x:0x%x:0x%x:0x%x:0x%x:0x%x.\n", 
            g_trans_common_sockfd.uw_num,
            g_trans_common_sockfd.st_sockfd[uw_use_num].a_mac[0], 
            g_trans_common_sockfd.st_sockfd[uw_use_num].a_mac[1], 
            g_trans_common_sockfd.st_sockfd[uw_use_num].a_mac[2], 
            g_trans_common_sockfd.st_sockfd[uw_use_num].a_mac[3], 
            g_trans_common_sockfd.st_sockfd[uw_use_num].a_mac[4], 
            g_trans_common_sockfd.st_sockfd[uw_use_num].a_mac[5]);

    return ;

}

/*****************************************************************************+
* Function: trans_device_common_delete()
* Description: 
* Parameters:
*           NONE
* Return Values:
*           TRANS_FAILD : 1
*           TRANS_SUCCESS  : 0
*
*  
*  Data:    2011-12-01
* 
+*****************************************************************************/
static void trans_device_common_delete(u_int8_t *p_mac)
{
    u_int32_t uw_index = 0;
    u_int32_t uw_tmp = 0xffff;
    
    if (NULL == p_mac)
    {
        FLOG_ERROR("NULL PTR! \r\n");
        return;
    }  

    pthread_mutex_lock(&(g_trans_common_sockfd.s_mutex));  

    for (uw_index = 0; uw_index < g_trans_common_sockfd.uw_num; uw_index ++)
    {
        FLOG_DEBUG_TRANS(g_trans_debug_device, "sockfd:%d, index:%d, type:%d.\r\n", 
                g_trans_common_sockfd.st_sockfd[uw_index].w_sockfd,
                uw_index,
                g_trans_common_sockfd.st_sockfd[uw_index].uc_module_type); 
                        
        if (trans_mac_addr_cmp(p_mac, g_trans_common_sockfd.st_sockfd[uw_index].a_mac))
        {
            uw_tmp = uw_index;
            
            FLOG_DEBUG_TRANS(g_trans_debug_device, "sockfd:%d, uw_index:%d.\r\n", 
                g_trans_common_sockfd.st_sockfd[uw_index].w_sockfd, uw_index); 

            break;
        }
    }

    if (0xffff == uw_tmp)
    {
        FLOG_ERROR("Dn not find the sockfd info in g_trans_common_sockfd!\r\n");
    }
    else
    {
        g_trans_common_sockfd.uw_num--;  

        memcpy(g_trans_common_sockfd.st_sockfd + uw_tmp,
            g_trans_common_sockfd.st_sockfd + g_trans_common_sockfd.uw_num,
            SIZEOF_TRANS_COMMON_SOCKFD_INFO);

         memset(g_trans_common_sockfd.st_sockfd + g_trans_common_sockfd.uw_num, 
            0, 
            SIZEOF_TRANS_COMMON_SOCKFD_INFO);
    }    

    pthread_mutex_unlock(&(g_trans_common_sockfd.s_mutex));  

    return;
}


/*****************************************************************************+
* Function: trans_device_add()
* Description: 
* Parameters:
*           NONE
* Return Values:
*           TRANS_FAILD : 1
*           TRANS_SUCCESS  : 0
*
*  
*  Data:    2011-12-01
* 
+*****************************************************************************/
u_int32_t trans_device_add(struct trans_device_info *p_device_info,
                            void ** pp_ptr)
{
    struct trans_device * p_device = NULL;     
    struct trans_unordered_list * p_unordered_list = g_trans_device_list;
    //u_int32_t uw_num_elems = 0;
    u_int32_t uw_ret = 0;

    //struct timeval st_tv;
    
    FLOG_DEBUG_TRANS(g_trans_debug_device, "Enter \r\n");    

    (*pp_ptr) = NULL;
    
    if (NULL == p_device_info)
    {
        FLOG_ERROR("NULL PTR! \r\n");
        return TRANS_FAILD;
    }  

    trans_device_get_plus(p_device_info->a_mac, pp_ptr);

    if (NULL != *pp_ptr)
    {
        FLOG_ERROR("The key mac is exist: 0x%x : 0x%x:0x%x:0x%x:0x%x:0x%x. error\r\n",
            p_device_info->a_mac[0], p_device_info->a_mac[1], p_device_info->a_mac[2], p_device_info->a_mac[3], p_device_info->a_mac[4], p_device_info->a_mac[5]);

        trans_device_get_sub(*pp_ptr);
            
        return TRANS_FAILD;
    }

    /* Allocate a memory.  */
    p_device = (struct trans_device *)malloc(SIZEOF_TRANS_DEVICE);
    if (NULL == p_device)
    {
        FLOG_ERROR("Malloc p_device error! \r\n");
        return TRANS_FAILD;   
    }
    
    memset((u_int8_t*)p_device, 0, SIZEOF_TRANS_DEVICE);

    /*Index*/
    p_device->us_index = trans_device_get_index();       
    /*Device Type: WMA, WMB, Monitor, RRH, Agent*/
    p_device->uc_module_type = p_device_info->uc_module_type; 
    /*States*/
    p_device->uc_states = p_device_info->uc_states;      
    /*MAC Address*/
    memcpy(p_device->a_mac, p_device_info->a_mac, TRANS_MAC_ADDR_LEN);

    /*Socket fd*/
    p_device->w_sockfd = p_device_info->w_sockfd;       
    /*Transaction ID*/
    p_device->uw_transaction = 2;  /*1 for registration message */
    /*Serial Number*/
    p_device->uw_serial_num = 2;  /*1 for registration message*/
    /*congestion Number*/
    p_device->uw_congestion_num = 0;  
    /*mutex*/
    if(pthread_mutex_init(&(p_device->dev_mutex), NULL)) 
    {
        FLOG_ERROR("Initializing p_device->dev_mutex mutex error! \r\n");

        return TRANS_FAILD;
    }

    p_device->uc_delete_flag = TRANS_DEVICE_EXIST;
    p_device->uw_used_num = 0;

    //trans_debug_msg_print(p_device, 30, g_trans_debug_device);

    //Insert it into to timer/event queue
    uw_ret = trans_unordered_list_add(p_unordered_list, 
                                    SIZEOF_TRANS_DEVICE, 
                                    p_device);
    if(TRANS_SUCCESS != uw_ret) 
    {
        //TRACE(4, "Error: inserting a timer in dll ordered lsit\n");
        FLOG_ERROR("Call trans_unordered_list_add error! \r\n");
        return uw_ret;
    }

    if (0 < p_device->w_sockfd)
    {
        trans_device_common_add(p_device->w_sockfd, p_device->uc_module_type, p_device->a_mac);

    }

    *pp_ptr = (void*)p_device;

    FLOG_DEBUG_TRANS(g_trans_debug_device, "Completed add device , index = %d, num = %d.\n", 
                        p_device->us_index, p_unordered_list->uw_node_num);
    
    FLOG_DEBUG_TRANS(g_trans_debug_device, "Device info:  module_type = %d, sockfd = %d.\n", 
                        p_device_info->uc_module_type, p_device_info->w_sockfd);
    
    FLOG_DEBUG_TRANS(g_trans_debug_device, "Device info:  mac = 0x%x:0x%x:0x%x:0x%x:0x%x:0x%x.\n", 
            p_device->a_mac[0], p_device->a_mac[1], p_device->a_mac[2], p_device->a_mac[3], p_device->a_mac[4], p_device->a_mac[5]);


    FLOG_DEBUG_TRANS(g_trans_debug_device, "Exit \r\n"); 

    return TRANS_SUCCESS;

}


/*****************************************************************************+
* Function: trans_device_delete()
* Description: 
* Parameters:
*           NONE
* Return Values:
*           TRANS_FAILD : 1
*           TRANS_SUCCESS  : 0
*
*  
*  Data:    2011-12-01
* 
+*****************************************************************************/
u_int32_t trans_device_delete(u_int8_t *p_mac)
{
    struct trans_unordered_list * p_unordered_list = g_trans_device_list;
    
    struct trans_device * p_device = NULL;

    u_int32_t uw_ret = 0;
    void* p_device_ptr = NULL;
   
    trans_device_get_plus(p_mac, &p_device_ptr);
    
    if (NULL == p_device_ptr)
    {
        FLOG_ERROR("Do not find the device info!\r\n");
        return TRANS_FAILD;    
    }

    p_device = (struct trans_device *)p_device_ptr;
    
    FLOG_DEBUG_TRANS(g_trans_debug_device, "Deldte :p_device = %p, w_sockfd = %d\n", 
                p_device, p_device->w_sockfd);

    if (0 < p_device->w_sockfd)
    {
        trans_device_common_delete(p_device->a_mac);
    }

    trans_device_get_sub(p_device_ptr);

    p_device->uc_delete_flag = TRANS_DEVICE_DELETE;

    /*Delete Node from list*/
    if ((0 == p_device->uw_used_num) && (TRANS_DEVICE_DELETE == p_device->uc_delete_flag))
    {
        uw_ret =  trans_device_get_out(p_unordered_list, p_device->a_mac, &p_device_ptr); 
        if(TRANS_SUCCESS != uw_ret) 
        {
            FLOG_ERROR("Call trans_device_get_out error!uw_ret = %d \r\n", uw_ret);
            return TRANS_FAILD;    
        }
    
        if (NULL == p_device_ptr)
        {
            FLOG_ERROR("Do not find the device info!\r\n");
            return TRANS_FAILD;    
        }
        
        free (p_device);
    }
   
    return TRANS_SUCCESS;
}


/*****************************************************************************+
* Function: trans_device_init()
* Description: 
* Parameters:
*           NONE
* Return Values:
*           TRANS_FAILD : 1
*           TRANS_SUCCESS  : 0
*
*  
*  Data:    2011-12-01
* 
+*****************************************************************************/
u_int32_t trans_device_init(u_int8_t *p_mac)
{
    u_int32_t uw_ret = 0;
    //FLOG_DEBUG_TRANS(g_trans_debug_timer, "Enter \r\n"); 
    
    uw_ret = trans_unordered_list_init(&g_trans_device_list);
    /*Error*/
    if (TRANS_SUCCESS != uw_ret)
    {
        FLOG_ERROR("Call trans_ordered_list_init error! \r\n");
    
        return uw_ret;        
    }  
    
    pthread_mutex_init(&g_trans_device_id_mutex, NULL);

    g_execute_device = 1;    
    g_trans_device_id = 1;  

    memcpy(g_trans_local_device_info.a_mac, p_mac, TRANS_MAC_ADDR_LEN);
    
    #ifdef TRANS_MS_COMPILE
    g_trans_local_device_info.uc_module_type = TRANS_MOUDLE_MS;
    g_trans_local_device_info.uc_device_type = TRANS_MONITOR_DEVICE_TYPE_WMB;
    #endif

    #ifdef TRANS_BS_COMPILE
    g_trans_local_device_info.uc_module_type = TRANS_MOUDLE_BS;
    g_trans_local_device_info.uc_device_type = TRANS_MONITOR_DEVICE_TYPE_WMA;
    #endif

    #ifdef TRANS_MONITOR_TEST_COMPILE
    g_trans_local_device_info.uc_module_type = TRANS_MOUDLE_MONITOR;
    g_trans_local_device_info.uc_device_type = TRANS_MONITOR_DEVICE_TYPE_MONITOR;
    #endif

    #ifdef TRANS_UI_COMPILE
    g_trans_local_device_info.uc_module_type = TRANS_MOUDLE_UI;
    g_trans_local_device_info.uc_device_type = TRANS_MONITOR_DEVICE_TYPE_UI;
    #endif


    g_trans_local_device_info.w_sockfd = 0;
    
    return TRANS_SUCCESS;
}


/*****************************************************************************+
* Function: trans_device_release()
* Description: 
* Parameters:
*           NONE
* Return Values:
*           TRANS_FAILD : 1
*           TRANS_SUCCESS  : 0
*
*  
*  Data:    2011-12-01
* 
+*****************************************************************************/
u_int32_t trans_device_release(void)
{
    struct trans_unordered_list * p_unordered_list = NULL;
    struct trans_device *p_device = NULL;
    void * p_field = NULL;
    //void * p_curr = NULL;
    static u_int8_t  uc_num = 0;
        
    p_unordered_list = g_trans_device_list;

    //cleaup the List
    pthread_mutex_lock(&(p_unordered_list->qmutex));
    
    TRANS_UNORDERED_LIST_FOREACH(p_unordered_list, p_field)
    {
        p_device = TRANS_UNORDERED_LIST_DATA(p_field);
        
        if (NULL != p_device)
        {
            p_device->uc_delete_flag = TRANS_DEVICE_DELETE;

            /*Delete Node from list*/
            if ((0 == p_device->uw_used_num) && (TRANS_DEVICE_DELETE == p_device->uc_delete_flag))
            {
                TRANS_UNORDERED_LIST_DELETE(p_unordered_list, p_field);
                    
                //pthread_mutex_destory(&(p_device->dev_mutex);
                free (p_device);

            }
        }
    }
    
    pthread_mutex_unlock(&(p_unordered_list->qmutex));

    uc_num ++;

    if ((0 < TRANS_UNORDERED_LIST_NUM(p_unordered_list)) 
        || (3 > uc_num))
    {
        trans_device_release();
    }

    /*Free Timer List Head*/
    free(g_trans_device_list);
    
    g_trans_device_list = NULL;
 
    return TRANS_SUCCESS;
}




