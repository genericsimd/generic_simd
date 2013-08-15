/* ----------------------------------------------------------------------------
 * IBM Confidential

 IBM Wireless M2M Platform

 (C)Copyright IBM Corp. 2009, 2010, 2011

 All Rights Reserved.

 File Name: db_ht_link.h 

 Change Activity:

 Date             Description of Change                            By
 -----------      ---------------------                            --------
 13-May 2011       Created                                         jian jun chang 

 ----------------------------------------------------------------------------
 PROLOG END TAG zYx                                                         */
#ifndef  _DB_HT_LINK_H_
#define  _DB_HT_LINK_H_
#include "hash_table.h"

struct  db_node
{
    void *p_val;
    struct db_node *next;
};
struct  db_ht_table
{
    struct   hash_table    *p_con_ht;
    struct   hash_table    *p_ip_ht;
    struct   db_node       *p_node_head;
};

int init_db_ht(struct db_ht_table **p_table,hash_fun p_ip_hash_fun,gen_key p_ip_key_fun,hash_fun p_con_hash_fun,gen_key p_con_key_fun);
int del_db_ht(struct db_ht_table **p_table);
int insert_con_db_node(struct db_ht_table *p_db_table,void *p_val);
//int search_con_db_node(struct db_ht_table *p_db_table,size_t  key);
void * search_con_db_node(struct db_ht_table *p_db_table,void *p_val);
int insert_ip_db_node(struct db_ht_table *p_db_table,void *p_val);
//int search_ip_db_node(struct db_ht_table *p_db_table,size_t  key);
void * search_ip_db_node(struct db_ht_table *p_db_table,void *p_val);

int insert_db_node(struct db_ht_table *p_db_table,void *p_val);

void * release_db_node (struct db_ht_table *p_db_table,void *p_val);

#endif
