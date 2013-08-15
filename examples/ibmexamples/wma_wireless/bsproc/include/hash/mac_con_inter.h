/* ----------------------------------------------------------------------------
 * IBM Confidential

 IBM Wireless M2M Platform

 (C)Copyright IBM Corp. 2009, 2010, 2011

 All Rights Reserved.

 File Name: mac_con_inter.h 

 Change Activity:

 Date             Description of Change                            By
 -----------      ---------------------                            --------
 14-May 2011       Created                                         jian jun chang 

 ----------------------------------------------------------------------------
 PROLOG END TAG zYx                                                         */
#ifndef  _MAC_CON_INTER_H_
#define  _MAC_CON_INTER_H_
#include <stdint.h>
#include <stdlib.h>
#include "mac_con_inter_data.h"
#include "db_ht_link.h"

int    init_mac_con_info(void **p_table);
int    del_mac_con_info(void **p_table);
struct mac_ip_node*   get_mac_con_id(void *p_table,u_int32_t des_mac, u_int32_t des_ip,u_int32_t src_mac,u_int32_t src_ip,u_int16_t port );
struct mac_ip_node*   get_ip_data(void *p_table,u_int32_t con_id);
int    insert_mac_con_info(void *p_table,u_int32_t des_mac, u_int32_t des_ip,u_int32_t src_mac,u_int32_t src_ip,u_int16_t port,u_int32_t con_id);
int    rel_mac_con_info(void *p_table,u_int32_t des_mac, u_int32_t des_ip,u_int32_t src_mac,u_int32_t src_ip,u_int16_t port,u_int32_t con_id);
#endif
