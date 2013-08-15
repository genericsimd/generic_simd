/* ----------------------------------------------------------------------------
 * IBM Confidential

 IBM Wireless M2M Platform

 (C)Copyright IBM Corp. 2009, 2010, 2011

 All Rights Reserved.

 File Name: hash_table.c 

 Change Activity:

 Date             Description of Change                            By
 -----------      ---------------------                            --------
 14-May 2011       Created                                         jian jun chang 

 ----------------------------------------------------------------------------
 PROLOG END TAG zYx                                                         */
#include "hash_table.h"
#include <string.h>

enum {num_primes = 28 };

unsigned  long   lower_bound(unsigned long *first, long nums , unsigned long value)
{
    int  start = 0,mid = 0,end = nums;
    //unsigned long * mid = NULL;
    mid = (end - start )/2;
    while (1)
    {
        //printf("start %d mid %d, end %d\n",start,mid,end);
        if(first[mid] > value)
            end = mid;
        else 
            start = mid;
        mid = (end + start )/2;
        if(mid == start || mid == end)
            break;
    }
    if(mid == nums)
        return first[mid - 1];
    
    return first[mid]; 
}

static const unsigned long prime_list[num_primes] =
{
  53ul,         97ul,         193ul,       389ul,       769ul,
  1543ul,       3079ul,       6151ul,      12289ul,     24593ul,
  49157ul,      98317ul,      196613ul,    393241ul,    786433ul,
  1572869ul,    3145739ul,    6291469ul,   12582917ul,  25165843ul,
  50331653ul,   100663319ul,  201326611ul, 402653189ul, 805306457ul, 
  1610612741ul, 3221225473ul, 4294967291ul
};

inline unsigned long  next_prime(unsigned long n)
{
  unsigned long* first = (unsigned long *)prime_list;
  //const unsigned long* last = prime_list + (int)num_primes;
  return lower_bound(first, num_primes,n);
  //return pos == last ? *(last - 1) : *pos;
}




//{
//    return key;
//}

size_t  map_bkt_num(size_t  key, size_t n,hash_fun p_fun)
{
    size_t hash_key = p_fun(key);
    return hash_key%n;
}

/*static size_t  common_hash_fun(size_t key)
{
    return key;
}*/

struct hash_table_node * create_hash_node(void *p_val)
{
    struct hash_table_node *p_node = (struct hash_table_node *)malloc(sizeof(struct hash_table_node ));
    p_node ->p_val = p_val;
    return p_node;
}


int init_hash_table(struct hash_table **p_table,hash_fun p_hash_fun,gen_key p_key_fun)
{
    *p_table = (struct hash_table *) malloc(sizeof(struct hash_table));
    //*((*p_table)->m_buckets) = malloc(sizeof(struct hash_table_node) * next_prime(_CONTANINER_NUM_));
    memset((*p_table)->m_buckets, 0, sizeof(struct hash_table_node*) * next_prime(_CONTANINER_NUM_));
    (*p_table)->p_hash_func = p_hash_fun;
    (*p_table)->hash_factor = next_prime(_CONTANINER_NUM_);
    (*p_table)->p_gen_key = p_key_fun;
    (*p_table)->m_ele_size = 0;
    return 0;
}

int del_hash_table(struct hash_table **p_table)
{
    int n = 0;
    int size = 0;
    struct hash_table_node *p_node_iter;
    if((*p_table)->m_ele_size < _CONTANINER_NUM_)
        size = next_prime(_CONTANINER_NUM_);
    else
        size = next_prime((*p_table)->m_ele_size);
    for(; n < size; n++)
    {
         while((*p_table)->m_buckets[n] != NULL)
         {
             p_node_iter = (*p_table)->m_buckets[n];
             (*p_table)->m_buckets[n] = p_node_iter->next;
             free(p_node_iter);
         }
    }
    free(*p_table);
    *p_table = NULL;
    return 0;
}

/*
int resize_hash_table(struct hash_table *p_table, u_int32_t num_ele_hint)
{
    size_t bucket = 0;
    const size_t old_n = p_table->m_ele_size;
    
    if (num_ele_hint > old_n) {
        const size_t n = next_prime(num_ele_hint);
        if (n > old_n) {
            static struct hash_table_node *tmp[n];
            //struct hash_table_node *tmp = malloc(sizeof(struct hash_table_node) * n);
            for (bucket = 0; bucket < old_n; ++bucket) {
                struct hash_table_node *first = p_table->m_buckets[bucket];
                while (first) {
                    size_t new_bucket = map_bkt_num(p_table->p_key_func(first->p_val), n,p_table->p_hash_func);
                    p_table->m_buckets[bucket] = first->next;
                    first->next = tmp[new_bucket];
                    tmp[__new_bucket] = first;
                    first = p_table->m_buckets[bucket];          
                }
            }
            free(p_table->m_buckets);
            p_table->m_buckets = tmp;
        }
    }
    return 0;
}
*/

int  insert_hash_table(struct hash_table *p_table,void *p_val)
{
    const size_t n = map_bkt_num(p_table->p_gen_key (p_val),p_table->hash_factor,p_table->p_hash_func);
    struct hash_table_node *first = p_table->m_buckets[n];
    struct hash_table_node *cur = first;
    for (; cur; cur = cur->next) 
        if (p_table->p_gen_key (cur->p_val)==p_table->p_gen_key(p_val))
           return -1;

    struct hash_table_node *tmp = create_hash_node(p_val);
    tmp->next = first;
    p_table->m_buckets[n] = tmp;
    p_table->m_ele_size ++;
    return 0;
}
void *  release_hash_table_node(struct  hash_table *p_table,void *p_val)
{
    size_t key = p_table->p_gen_key(p_val);
    size_t n = map_bkt_num(key,p_table->hash_factor,p_table->p_hash_func);
    struct hash_table_node *find_n = NULL,*prev = NULL;
    void *p_result = NULL;
    for ( find_n = p_table->m_buckets[n];find_n && (p_table->p_gen_key(find_n->p_val) != key);)
    {
        prev = find_n;
        find_n = find_n->next;
    }
    if(find_n != NULL)
    {
        if(prev != NULL)
        {
            prev->next = find_n->next;
        }
        else
        {
            p_table->m_buckets[n] = find_n->next;
        }
        //delete(find_n->p_val);
        p_result = find_n->p_val;
        free(find_n);

        return p_result;
    }
    else
    {
        printf("error in del node from hash table, the node not exist in hash table\n");
        return NULL;
    }
    return NULL;
}
/*
void * search_hash_node(struct  hash_table *p_table,size_t key)
{
    size_t n = map_bkt_num(key,p_table->hash_factor,p_table->p_hash_func);
    const struct hash_table_node *first;
    for ( first = p_table->m_buckets[n];first && (p_table->p_key_func(first->p_val) != key);
          first = first->next)
    {
    }
    if(first == NULL)
    {
        printf("search NULL\n");
        return NULL;
    }
    return first->p_val;
}*/
void   *search_hash_node(struct  hash_table *p_table,void *p_val)
{
    size_t key = p_table->p_gen_key(p_val);
    size_t n = map_bkt_num(key,p_table->hash_factor,p_table->p_hash_func);
    const struct hash_table_node *first;
    for ( first = p_table->m_buckets[n];first && (p_table->p_gen_key(first->p_val) != key);
          first = first->next)
    {
    }
    if(first == NULL)
    {
//        printf("search NULL\n");
        return NULL;
    }
    return first->p_val;
}
