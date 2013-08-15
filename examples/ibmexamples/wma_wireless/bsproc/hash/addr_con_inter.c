/* ----------------------------------------------------------------------------
 * IBM Confidential

 IBM Wireless M2M Platform

 (C)Copyright IBM Corp. 2009, 2010, 2011

 All Rights Reserved.

 File Name: addr_con_inter.c 

 Change Activity:

 Date             Description of Change                            By
 -----------      ---------------------                            --------
 14-May 2011       Created                                         jian jun chang 

 ----------------------------------------------------------------------------
 PROLOG END TAG zYx                                                         */
#include "addr_con_inter.h"
#include "addr_con_inter_data.h"
#include "hash_table.h"
#include <string.h>
int    init_addr_con_info(void **p_table)
{
    return init_hash_table((struct hash_table **)p_table,&addr_hash_fun,&addr_gen_key);
}

int    del_addr_con_info(void **p_table)
{
    del_hash_table((struct hash_table **)p_table);
    return 0;
}

int    insert_addr_con(void *p_table,char *p_addr,u_int32_t  con_id)
{
    
    struct addr_con_node *p_node = (struct addr_con_node *)malloc(sizeof(struct addr_con_node));
    strcpy(p_node->addr,p_addr);
    p_node->con_id = con_id;
    
    if(search_hash_node(p_table,p_node))
        return 3;
    
    return insert_hash_table(p_table,p_node);
}
int    release_addr_con(void *p_table,char *p_addr)
{
    
    struct addr_con_node *p_node = (struct addr_con_node *)malloc(sizeof(struct addr_con_node));
    struct addr_con_node *p_result = NULL;
    strcpy(p_node->addr,p_addr);
//    p_node->con_id = con_id;
    
    p_result = release_hash_table_node(p_table,p_node); 
    if(p_result != NULL)
    {
        free(p_node);  
        free(p_result);
        return 0;
    }
    else
    {
        printf("error in release node\n");
        return -1;
    }
}

u_int32_t get_addr_con_id(void *p_table,char *p_addr)
{
    struct addr_con_node node, *p_result;
    strcpy(node.addr,p_addr);
    
    p_result = search_hash_node(p_table,&node);
    if(p_result != NULL)
    {
        return p_result->con_id;   
    }
    return 0;
}
