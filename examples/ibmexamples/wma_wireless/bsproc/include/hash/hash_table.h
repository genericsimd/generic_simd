/* ----------------------------------------------------------------------------
 * IBM Confidential

 IBM Wireless M2M Platform

 (C)Copyright IBM Corp. 2009, 2010, 2011

 All Rights Reserved.

 File Name: hast_table.h 

 Change Activity:

 Date             Description of Change                            By
 -----------      ---------------------                            --------
 14-May 2011       Created                                         jian jun chang 

 ----------------------------------------------------------------------------
 PROLOG END TAG zYx                                                         */
#ifndef  _HASH_TABLE_H_
#define  _HASH_TABLE_H_
#define  _CONTANINER_NUM_         6151
// Note: assumes long is at least 32 bits.
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>

typedef size_t (*hash_fun)(size_t key);
typedef size_t (*gen_key) (void *p_value);


struct hash_table_node {              
    void    *p_val;
    struct  hash_table_node  *next;   
};

struct  hash_table
{
    struct hash_table_node *m_buckets[_CONTANINER_NUM_];
    hash_fun      p_hash_func;
    gen_key       p_gen_key;
    u_int32_t     m_ele_size;
    u_int32_t     hash_factor;
};



int    init_hash_table(struct hash_table **p_table,hash_fun p_hash_fun,gen_key p_key_fun);
int    insert_hash_table(struct hash_table *p_table,void *p_val);
//void   *search_hash_node_key(struct  hash_table *p_table,size_t key);
void   *search_hash_node(struct  hash_table *p_table,void *p_val);
int    del_hash_table(struct hash_table **p_table);
void *  release_hash_table_node(struct  hash_table *p_table,void *p_val);
    

#endif
