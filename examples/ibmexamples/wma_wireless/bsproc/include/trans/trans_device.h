/* ----------------------------------------------------------------------------
 IBM Confidential

 IBM Wireless M2M Platform

 (C)Copyright IBM Corp. 2009, 2010, 2011

 All Rights Reserved.

 File Name: trans_device.h

 Change Activity:

 Date             Description of Change                            By
 -----------      ---------------------                            --------
 01-Dec.2011      Created                                          E Wulan

 ----------------------------------------------------------------------------
 PROLOG END TAG zYx                                                         */


#ifndef TRANS_DEVICE_H_
#define TRANS_DEVICE_H_



/*****************************************************************************+
*Macro
+*****************************************************************************/
enum trans_device_active_enum
{   
   TRANS_DEVICE_ACTIVE = 0,       /*Active */
   TRANS_DEVICE_DEACTIVE = 1     /*Deactive*/

};

enum trans_device_delete_enum 
{   
   TRANS_DEVICE_EXIST = 0,       
   TRANS_DEVICE_DELETE = 1     

};


#define trans_mac_addr_cmp(mac1, mac2) \
                    ((((u_int8_t *)(mac1))[0] == ((u_int8_t *)(mac2))[0])   \
                    &&(((u_int8_t *)(mac1))[1] == ((u_int8_t *)(mac2))[1])   \
                    &&(((u_int8_t *)(mac1))[2] == ((u_int8_t *)(mac2))[2])   \
                    &&(((u_int8_t *)(mac1))[3] == ((u_int8_t *)(mac2))[3])   \
                    &&(((u_int8_t *)(mac1))[4] == ((u_int8_t *)(mac2))[4])   \
                    &&(((u_int8_t *)(mac1))[5] == ((u_int8_t *)(mac2))[5]))


/*****************************************************************************+
*Data structure
+*****************************************************************************/
struct trans_device_list 
{
    u_int8_t     a_mac[TRANS_MAC_ADDR_LEN];      /*kye : mac add  */
    struct trans_device_list* p_next; // next ptr in list
    void* p_data;    /*data of the list element : struct trans_device */
};
    
#define SIZEOF_TRANS_DEVICE_LIST   sizeof(struct trans_device_list)
    
    
struct trans_device_ordered_list
{
    struct trans_device_list* p_head;
    u_int32_t   uw_node_num;
    pthread_mutex_t   qmutex;
};
    
#define SIZEOF_TRANS_DEVICE_ORDERED_LIST   sizeof(struct trans_device_ordered_list)


struct trans_device_info
{
    u_int8_t     uc_module_type; /*Module Type: WMA, WMB, Monitor, RRH, Agent*/
    u_int8_t     uc_device_type; /*Device Type: WMA, WMB, Monitor*/
    u_int8_t     a_mac[TRANS_MAC_ADDR_LEN];       /*MAC Address*/
    u_int8_t     uc_states;      /*States*/
    //u_int16_t   us_port;          /*TCP Port*/
    //u_int32_t   uw_ip_addr;   /*IP Address*/
    int32_t       w_sockfd;       /*Socket fd*/

};
    
    
#define SIZEOF_TRANS_DEVICE_INFO   sizeof(struct trans_device_info)


struct trans_device
{
    u_int16_t   us_index;       /*Index*/
    //struct timeval st_tv;      /* tv kye */
    int32_t       w_sockfd;       /*Socket fd*/
    u_int8_t     uc_states;      /*States*/
    u_int8_t     uc_module_type; /*Device Type: WMA, WMB, Monitor, RRH, Agent*/
    u_int8_t     a_mac[TRANS_MAC_ADDR_LEN];       /*MAC Address*/
    //u_int16_t   us_port;          /*TCP Port*/
    //u_int32_t   uw_ip_addr;   /*IP Address*/
    //u_int16_t   uw_connect_num;/*Connect Number total */
    u_int32_t   uw_transaction;  /*Transaction ID*/
    u_int32_t   uw_serial_num;  /*Serial Number*/
    u_int32_t   uw_congestion_num;  /*congestion Number*/
    pthread_mutex_t   dev_mutex; /*mutex*/
    u_int32_t   uw_used_num;
    u_int8_t     uc_delete_flag;

};


#define SIZEOF_TRANS_DEVICE   sizeof(struct trans_device)


/*****************************************************************************+
*extern
+*****************************************************************************/

extern u_int32_t trans_device_init(u_int8_t *p_mac);

extern u_int32_t trans_device_release(void);

extern u_int32_t  trans_device_get_transaction_id(u_int8_t *p_mac); 

extern u_int32_t  trans_device_get_serial_num(u_int8_t *p_mac) ;

extern int32_t  trans_device_get_socket(u_int8_t *p_mac);

extern int32_t  trans_device_get_type(u_int8_t *p_mac);

extern u_int32_t  trans_device_get_congestion_num(u_int8_t *p_mac, u_int8_t uc_flag); 


extern u_int32_t trans_device_add(struct trans_device_info *p_device_info,
                            void ** pp_ptr);
                            
extern u_int32_t trans_device_delete(u_int8_t *p_mac);
    

//struct trans_ordered_list *g_trans_device_list;

#endif /* TRANS_DEVICE_H_ */

