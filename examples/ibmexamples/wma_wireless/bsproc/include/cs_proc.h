/* ----------------------------------------------------------------------------
 * IBM Confidential

   IBM Wireless M2M Platform

   (C)Copyright IBM Corp. 2009, 2010, 2011

   All Rights Reserved.

   File Name: cs_proc.h

   Change Activity:

   Date             Description of Change                            By
   -----------      ---------------------                            --------
   12-May 2011       Created                                         Zhu, Zhenbo

   ----------------------------------------------------------------------------
   PROLOG END TAG zYx                                                         */

#ifndef __CS_PROC_H_
#define __CS_PROC_H_

#define DEV_TYPE IFF_TAP

#define LINE_MTU_SIZE 10000

#include "hash_table.h"

//extern struct hash_table * gp_hash_table;
void * gp_hash_table;

struct cs_global_param
{
    char if_name[32];
    char wma_subnet[32];
};


int init_cs_table(void);
int release_cs_table(void);

int pkt_classify_process (void);
int pkt_classify_release (void);
int pkt_forward_process (void);
int pkt_forward_release (void);

#endif /* __CS_H_ */
