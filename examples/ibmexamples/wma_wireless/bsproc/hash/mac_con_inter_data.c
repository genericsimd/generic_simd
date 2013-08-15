/* ----------------------------------------------------------------------------
 * IBM Confidential

 IBM Wireless M2M Platform

 (C)Copyright IBM Corp. 2009, 2010, 2011

 All Rights Reserved.

 File Name: mac_con_inter_data.c 

 Change Activity:

 Date             Description of Change                            By
 -----------      ---------------------                            --------
 14-May 2011       Created                                         jian jun chang 

 ----------------------------------------------------------------------------
 PROLOG END TAG zYx                                                         */
#include "mac_con_inter_data.h"

size_t   ip_hash_fun(size_t  key)
{
    return key;
}

size_t   ip_gen_key(void *p_val)
{
    struct  mac_ip_node *p_node = (struct mac_ip_node *)p_val;
    size_t  key;
    key = p_node->des_mac + p_node->des_ip + p_node->src_mac + p_node->src_ip + p_node->port;
    return key;
}
size_t   con_hash_fun (size_t    key)
{
    return  key;
}

size_t   con_gen_key(void *p_val)
{
    struct  mac_ip_node *p_node = (struct mac_ip_node *)p_val;
    return p_node->con_id;
}
