/* ----------------------------------------------------------------------------
   IBM Confidential

   IBM Wireless M2M Platform   

   (C)Copyright IBM Corp. 2011

   All Rights Reserved.

   File Name: mac_dsa_sm.h

   Change Activity:

   Date             Description of Change                   By
   -----------      ---------------------					--------
   1-Mar.2011		Created                                 Parul Gupta

   ----------------------------------------------------------------------------
   PROLOG END TAG zYx                                                         */

#ifndef _MAC_DSA_SM_H
#define _MAC_DSA_SM_H

#include <assert.h>
#include "constants.h"
#include "phy_params.h"
#include "ul_mgt_msg_queue.h"
#include "mac_frame.h"
#include "memmgmt.h"
#include "mac_sdu_queue.h"
#include "mac_qos_mm.h"
#include "mac_dsa_list.h"
#include "scheduler.h"
#include "mac_connection.h"

#include "mac_sf_api.h"

typedef enum
{
	DSA_BEGIN = 0,

	/* for remote initiated DSA */
	DSA_ACK_PENDING,
	RMT_DSA_HOLDING_DOWN,
	RMT_DSA_DELETING_SF,

	/* for local initiated DSA */
	DSA_RSP_PENDING,
	LCL_DSA_HOLDING_DOWN,
	LCL_DSA_RETRIES_EXHAUSTED,
	LCL_DSA_DELETING_SF,

	/* usage common */
	DSA_FAILED,
	DSA_ERRED,
	DSA_SUCCESSFUL
} dsa_state;

//List of SF status possible. Ref Fig 115 in Wimax Spec, Rev2D5
#define NULL_STATE 0
#define ADDING_LOCAL 1
#define ADDING_REMOTE 2
#define ADD_FAILED 3
#define NOMINAL 4
#define CHANGING_LOCAL 5
#define CHANGING_REMOTE 6
#define DELETING 6
#define DELETED 7


// Parameters to decide whether a service flow request can be accommodated
#define MAX_BURST_SIZE 1000
#define JITTER_THRESH 0
#define LATENCY_THRESH 0

int dsa_primitive_handler(primitivetype primit, struct transaction_node* trans_node);
int dsa_msg_handler(void* dsa_msg, int msg_type, struct transaction_node* trans_node);
#endif
