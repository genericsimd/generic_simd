/* ----------------------------------------------------------------------------
 * IBM Confidential

 IBM Wireless M2M Platform

 (C)Copyright IBM Corp. 2009, 2010, 2011

 All Rights Reserved.

 File Name: addr_con_inter.h 

 Change Activity:

 Date             Description of Change                            By
 -----------      ---------------------                            --------
 13-May 2011       Created                                         jian jun chang 

 ----------------------------------------------------------------------------
 PROLOG END TAG zYx                                                         */
#ifndef _ADDR_CON_H_
#define _ADDR_CON_H_

#include <stdint.h>
#include <stdlib.h>
int    init_addr_con_info(void **p_table);
int    del_addr_con_info(void **p_table);
int    insert_addr_con(void *p_table,char *p_addr,u_int32_t  con_id);
int    release_addr_con(void *p_table,char *p_addr);
u_int32_t get_addr_con_id(void *p_table,char *p_addr);
#endif
