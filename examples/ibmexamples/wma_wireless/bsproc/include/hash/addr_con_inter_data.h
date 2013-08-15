/* ----------------------------------------------------------------------------
 * IBM Confidential

 IBM Wireless M2M Platform

 (C)Copyright IBM Corp. 2009, 2010, 2011

 All Rights Reserved.

 File Name: addr_con_inter_data.h 

 Change Activity:

 Date             Description of Change                            By
 -----------      ---------------------                            --------
 13-May 2011       Created                                         jian jun chang 

 ----------------------------------------------------------------------------
 PROLOG END TAG zYx                                                         */
#ifndef _ADDR_CON_INTER_DATA_H_
#define _ADDR_CON_INTER_DATA_H_
#include <stdint.h>
#include <stdlib.h>
struct addr_con_node
{
    char            addr[32];
    u_int32_t       con_id;
};

size_t   addr_hash_fun(size_t key);
size_t   addr_gen_key(void *);
#endif
