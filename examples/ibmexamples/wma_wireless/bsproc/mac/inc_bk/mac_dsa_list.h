/* ----------------------------------------------------------------------------
   IBM Confidential

   IBM Wireless M2M Platform   

   (C)Copyright IBM Corp. 2011

   All Rights Reserved.

   File Name: mac_dsa_list.h

   Change Activity:

   Date             Description of Change                   By
   -----------      ---------------------					--------
   1-Mar.2011		Created                                 Parul Gupta

   ----------------------------------------------------------------------------
   PROLOG END TAG zYx                                                         */

#ifndef _MAC_DSA_LIST_H_
#define _MAC_DSA_LIST_H_

#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <pthread.h>
#include "mac_qos_mm.h"
#include "mac_serviceflow.h"
#include "app_timer.h"

extern pthread_mutex_t last_tid_mutex, last_sfid_mutex;
extern int last_tid, last_tsfid;

struct sf_list
{
    struct service_flow * head;
    int count;
    pthread_mutex_t sf_list_mutex;
};

struct sf_status_node
{
    int sfid;//if still no sfid, set sfid=TEMPSFID;
    int sfid_temp; //if sfid=TEMPSFID, sfid_temp has none-zero value
    int sf_status;
    struct service_flow * sf;// point to either temp service flow or completed service flow
    struct sf_status_node * next;
};
struct sf_status_list
{
    struct sf_status_node *head;
    int count;
    pthread_mutex_t  sf_status_mutex;
};

struct retrans_msg
{
    int type;
    void* msg;//dsa_req, dsa_rsp, dsa_ack
	int length;
    int retrytimes;
};
struct transaction_node
{
    u_int16_t trans_id;
    int trans_status;
    struct sf_status_node *sf_st;
    struct retrans_msg* re_msg; // for retries
	app_timer *t7_timer;
	app_timer *t8_timer;
	app_timer *t10_timer;
	app_timer *t14_timer;
	int primary_mm_cid; // need to associate this transaction with an SS
    struct transaction_node * next;
};

struct transaction_list
{
    struct transaction_node * head;
    int count;
    pthread_mutex_t trans_mutex;
};

struct transaction_list *dsa_trans_list;
struct transaction_list *dsd_trans_list;
struct sf_status_list *sf_st_list;
struct sf_list *sf_tmp_list;

extern int initiate_trans_list(struct transaction_list** trans_list_ptr);
extern struct transaction_node* find_trans_node(struct transaction_list* transaction_list,u_int16_t trans_id);
extern struct transaction_node* find_trans_node_from_sfid(struct transaction_list* transaction_list,u_int32_t sfid);
extern int add_trans_node(struct transaction_list* transaction_list, struct transaction_node* trans_node);
extern int delete_trans_node(struct transaction_list* transaction_list, u_int16_t trans_id);
extern int initiate_sf_st_list();
extern int add_sf_st_node(struct sf_status_node* sf_st_node);
extern int initiate_sf_tmp_list();
extern int add_sf_tmp_node(serviceflow* sf_node);
extern int del_sf_status_node(int sfid, int tempsfid);
extern struct sf_status_node* find_sf_st_node(u_int32_t sfid);
extern int delete_sf_tmp_node(serviceflow* delete_node);
extern serviceflow* move_sf_tmp_node(serviceflow* move_node);
extern int incr_and_read_tsfid();
extern int incr_and_read_tid();
extern int free_trans_list(struct transaction_list** trans_list_ptr);
int free_sf_st_list();
int free_sf_tmp_list();


#endif


