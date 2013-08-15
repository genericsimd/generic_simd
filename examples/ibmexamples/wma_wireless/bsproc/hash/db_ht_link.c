/* ----------------------------------------------------------------------------
 * IBM Confidential

 IBM Wireless M2M Platform

 (C)Copyright IBM Corp. 2009, 2010, 2011

 All Rights Reserved.

 File Name: db_ht_link.c 

 Change Activity:

 Date             Description of Change                            By
 -----------      ---------------------                            --------
 14-May 2011       Created                                         jian jun chang 

 ----------------------------------------------------------------------------
 PROLOG END TAG zYx                                                         */
#include "db_ht_link.h"


int init_db_ht(struct db_ht_table **p_table,hash_fun p_ip_hash_fun,gen_key p_ip_key_fun,hash_fun p_con_hash_fun,gen_key p_con_key_fun)
{
    *p_table = malloc(sizeof (struct db_ht_table));
    init_hash_table(&((*p_table)->p_con_ht),p_con_hash_fun,p_con_key_fun);
    init_hash_table(&((*p_table)->p_ip_ht),p_ip_hash_fun,p_ip_key_fun);
    return 0;
}
int del_db_ht(struct db_ht_table **p_table)
{
    del_hash_table(&(*p_table)->p_con_ht);
    del_hash_table(&(*p_table)->p_ip_ht);
    return 0;
}

int insert_con_db_node(struct db_ht_table *p_db_table,void *p_val)
{
    return insert_hash_table(p_db_table->p_con_ht,p_val);
}


void * search_con_db_node(struct db_ht_table *p_db_table,void *p_val)
{
    return search_hash_node(p_db_table->p_con_ht,p_val);
}
int insert_ip_db_node(struct db_ht_table *p_db_table,void *p_val)
{
    return insert_hash_table(p_db_table->p_ip_ht,p_val);
}
void * search_ip_db_node(struct db_ht_table *p_db_table,void *p_val)
{
    return search_hash_node(p_db_table->p_ip_ht,p_val);
}

void * rel_con_db_node(struct db_ht_table *p_db_table,void *p_val)
{
    return  release_hash_table_node(p_db_table->p_con_ht,p_val);
}

void * rel_ip_db_node(struct db_ht_table *p_db_table,void *p_val)
{
    return  release_hash_table_node(p_db_table->p_ip_ht,p_val);
}
int insert_db_node(struct db_ht_table *p_db_table,void *p_val)
{
    if(search_con_db_node(p_db_table,p_val))
    {
        return 1;
    }
    if(search_ip_db_node(p_db_table,p_val))
    {
        return 1;
    }
    struct db_node *p_node = malloc(sizeof(struct db_node));
    p_node->p_val = p_val;
    p_node->next = p_db_table->p_node_head;
    p_db_table->p_node_head= p_node;
    insert_con_db_node(p_db_table,p_val);
    insert_ip_db_node(p_db_table,p_val);
    return 0;
}
void * release_db_node (struct db_ht_table *p_db_table,void *p_val)
{
    void *p_result1 = NULL,*p_result2 = NULL;
    if(search_con_db_node(p_db_table,p_val))
    {
        return NULL;
    }
    if(search_ip_db_node(p_db_table,p_val))
    {
        return NULL;
    }
    p_result1 = rel_con_db_node(p_db_table,p_val);
    p_result2 = rel_ip_db_node(p_db_table,p_val);
    if(p_result1 != p_result2 )
    {
        printf("release hash node have some error");
        return NULL;
    }
    return p_result1;
} 
