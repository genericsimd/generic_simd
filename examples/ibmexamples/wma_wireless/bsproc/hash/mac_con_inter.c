/* ----------------------------------------------------------------------------
 * IBM Confidential

 IBM Wireless M2M Platform

 (C)Copyright IBM Corp. 2009, 2010, 2011

 All Rights Reserved.

 File Name: mac_con_inter.c 

 Change Activity:

 Date             Description of Change                            By
 -----------      ---------------------                            --------
 14-May 2011       Created                                         jian jun chang 

 ----------------------------------------------------------------------------
 PROLOG END TAG zYx                                                         */
#include "mac_con_inter.h"
#include "db_ht_link.h"

int    init_mac_con_info(void  **p_table)
{
    return init_db_ht((struct db_ht_table **)p_table,&ip_hash_fun,&ip_gen_key,&con_hash_fun,&con_gen_key);
}
int    del_mac_con_info(void **p_table)
{
    return del_db_ht(((struct db_ht_table **)p_table));
}

struct mac_ip_node*   get_mac_con_id(void *p_table,u_int32_t des_mac, u_int32_t des_ip,u_int32_t src_mac,u_int32_t src_ip,u_int16_t port )
{
   struct mac_ip_node node;
   node.des_mac = des_mac;
   node.des_ip  = des_ip;
   node.src_mac = src_mac;
   node.src_ip  = src_ip;
   node.port    = port;
   return search_ip_db_node((struct db_ht_table *)p_table,&node);
}

struct mac_ip_node*   get_ip_data(void *p_table,u_int32_t con_id)
{
    struct mac_ip_node node; 
    node.con_id = con_id;
    return search_con_db_node((struct db_ht_table *)p_table,&node);
}

int insert_mac_con_info(void *p_table,u_int32_t des_mac, u_int32_t des_ip,u_int32_t src_mac,u_int32_t src_ip,u_int16_t port,u_int32_t con_id)
{
    struct mac_ip_node *p_node = (struct mac_ip_node *)malloc(sizeof(struct mac_ip_node));
    p_node->des_mac = des_mac;
    p_node->des_ip = des_ip;
    p_node->src_mac = src_mac;
    p_node->src_ip = src_ip;
    p_node->port = port;
    p_node->con_id = con_id;
    insert_db_node((struct db_ht_table * )p_table,p_node);
    return 0;
}

int rel_mac_con_info(void *p_table,u_int32_t des_mac, u_int32_t des_ip,u_int32_t src_mac,u_int32_t src_ip,u_int16_t port,u_int32_t con_id)
{
    struct mac_ip_node *p_node = (struct mac_ip_node *)malloc(sizeof(struct mac_ip_node));
    struct mac_ip_node *p_result = NULL;
    p_node->des_mac = des_mac;
    p_node->des_ip = des_ip;
    p_node->src_mac = src_mac;
    p_node->src_ip = src_ip;
    p_node->port = port;
    p_node->con_id = con_id;
    p_result = release_db_node((struct db_ht_table * )p_table,p_node);
    free(p_node);
    free(p_result);
    return 0;
}
