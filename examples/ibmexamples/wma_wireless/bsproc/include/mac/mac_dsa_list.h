/* ----------------------------------------------------------------------------
   IBM Confidential

   IBM Wireless M2M Platform   

   (C)Copyright IBM Corp. 2011

   All Rights Reserved.

   File Name: mac_dsa_list.h

   Change Activity:

   Date             Description of Change                   	By
   -----------      ---------------------		--------
   1-Mar.2011		Created                                 	Parul Gupta
   30-Jan.2012	Modified to make it support dsc	 	Xianwei. Yi

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
#include "mac_sf_api.h"

extern pthread_mutex_t last_tid_mutex;
extern pthread_mutex_t last_sfid_mutex;
extern int last_tid;
extern int last_sfid;

struct sf_list
{
    struct service_flow 	*head;
    pthread_mutex_t 		sf_list_mutex;
};

struct retrans_msg
{
    int 	type;
    void	*msg;	/* dsa_req, dsa_rsp, dsa_ack */
	int 	length;
    int 	retrytimes;
};
struct transaction_node
{
    u_int16_t 				trans_id;			/* transaction id */
	u_int64_t				initiator_mac;		/* initiator mac */
    int 					trans_status;
    struct service_flow		*sf;				/* temporay service flow for dsa and dsc,  
    												  * for dsd, pointer to existing service flow 
    												  */
    struct retrans_msg 		re_msg;				/* for retries */
	app_timer 				*t7_timer;
	app_timer 				*t8_timer;
	app_timer 				*t10_timer;
	app_timer 				*t14_timer;

	pthread_mutex_t			mutex;
	u_int32_t				ref_count;			/* reference count */

	u_int64_t				peer_mac;			/* peer mac address */
	u_int16_t				peer_primary_cid;	/* peer primary cid */			
	sf_notify				notify;				/* callback function when dsx transaction complete */

	struct transaction_node *next;
};

struct transaction_list
{
    struct transaction_node *head;
    pthread_mutex_t 		trans_list_mutex;
};

struct transaction_list dsa_trans_list;
struct transaction_list dsd_trans_list;
struct transaction_list dsc_trans_list;

struct sf_list sf_tmp_list;

extern void initiate_trans_list(struct transaction_list *trans_list);
extern struct transaction_node* alloc_trans_node(void);
extern void put_trans_node(struct transaction_node *trans_node);
extern void free_trans_list(struct transaction_list *trans_list);
extern struct transaction_node* get_trans_node(struct transaction_list *trans_list, \
															u_int64_t initiator_mac, u_int16_t trans_id);
extern struct transaction_node* find_trans_node_from_sfid (struct transaction_list	 *trans_list, \
															u_int64_t initiator_mac,u_int32_t sfid);
extern void add_trans_node(struct transaction_list *trans_list, struct transaction_node *trans_node);
extern void delete_trans_node(struct transaction_list *trans_list, struct transaction_node *trans_node);
extern int incr_and_read_sfid(void);
extern int incr_and_read_tid(void);

#endif


