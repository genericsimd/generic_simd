/* ----------------------------------------------------------------------------
 * IBM Confidential

 IBM Wireless M2M Platform

 (C)Copyright IBM Corp. 2009, 2010, 2011

 All Rights Reserved.

 File Name: addr_con_inter_data.c

 Change Activity:

 Date             Description of Change                            By
 -----------      ---------------------                            --------
 14-May 2011       Created                                         jian jun chang 

 ----------------------------------------------------------------------------
 PROLOG END TAG zYx                                                         */
#include "addr_con_inter_data.h"

size_t   addr_hash_fun(size_t key)
{
    return key;
}
size_t   addr_gen_key(void *p_val)
{
    struct addr_con_node *p_node = (struct addr_con_node *)p_val;
    char *s = p_node->addr;
    unsigned long h = 0; 
    for ( ; *s; ++s)
        h =  5 * h + *s;
    return h;
}

