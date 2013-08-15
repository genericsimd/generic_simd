/* ----------------------------------------------------------------------------
   IBM Confidential

   IBM Wireless M2M Platform   

   (C)Copyright IBM Corp. 2011

   All Rights Reserved.

   File Name: mac_dsa_sm.c

   Change Activity:

   Date             Description of Change                   		By
   -----------      ---------------------		--------
   1-Mar.2011       Created                                 		Parul Gupta
   03-Feb.2012	modified for the support of dsc		Xianwei. Yi

   ----------------------------------------------------------------------------
   PROLOG END TAG zYx                                                         */

#ifndef __MAC_DSD_PROCESSING_
#define __MAC_DSD_PROCESSING_

#include <stdio.h>
#include <assert.h>
#include "constants.h"
#include "sll_fifo_q.h"
#include "mac_sdu_queue.h"
#include "dl_exp_params.h"
#include "ul_mgt_msg_queue.h"
#include "mac_qos_mm.h"
#include "mac_dsa_list.h"
#include "app_timer.h"
#include "util.h"
#include "mac_connection.h"
#include "memmgmt.h"

typedef enum
{
	DSD_LOCAL_RSP_PENDING = 98,
	DSD_ERRED = 97,
	DSD_ENDED = 96,
	DSD_SUCCESSFUL = 95,
	DSD_REMOTE_HOLDING_DOWN = 94,
	DSD_LOCAL_HOLDING_DOWN = 93,
	DSD_BEGIN = 92,
} dsd_state;

int dsd_primitive_handler(struct transaction_node *trans_node, primitivetype  primit);
int dsd_msg_handler(void *dsd_msg, int msg_type, struct transaction_node *trans_node);

#endif
