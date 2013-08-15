/* ----------------------------------------------------------------------------
 * IBM Confidential

 IBM Wireless M2M Platform

 (C)Copyright IBM Corp. 2009, 2010, 2011

 All Rights Reserved.

 File Name: mac_con_interi_data.h 

 Change Activity:

 Date             Description of Change                            By
 -----------      ---------------------                            --------
 14-May 2011       Created                                         jian jun chang 

 ----------------------------------------------------------------------------
 PROLOG END TAG zYx                                                         */
#ifndef  _INTERFACE_DATA_H_
#define  _INTERFACE_DATA_H_
#include <stdlib.h>
struct  mac_ip_node
{
    u_int32_t         con_id;
    u_int32_t         des_mac;
    u_int32_t         des_ip;
    u_int32_t         src_mac;
    u_int32_t         src_ip;
    u_int16_t         port ;
};

size_t   ip_hash_fun(size_t   key);
size_t   ip_gen_key(void *);
size_t   con_hash_fun (size_t key);
size_t   con_gen_key(void *);

#endif
