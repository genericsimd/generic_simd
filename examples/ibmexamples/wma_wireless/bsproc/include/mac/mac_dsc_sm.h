/* ----------------------------------------------------------------------------
   IBM Confidential

   IBM Wireless M2M Platform   

   (C)Copyright IBM Corp. 2011

   All Rights Reserved.

   File Name: mac_dsc_sm.h

   Change Activity:

   Date                      Description of Change                   By
   -----------      --------------------- 		--------
   30-Jan.2012		Created                          		Xianwei. Yi

   ----------------------------------------------------------------------------
   PROLOG END TAG zYx                                                         */

#ifndef _MAC_DSC_SM_H
#define _MAC_DSC_SM_H

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

/* refer to figure 118 and figure 119 in Wimax sped, Rev2D5 */
typedef enum
{
	/* for local initiated transaction state */
	DSC_RSP_PENDING = 0U,
	DSC_LOCAL_RETRIES_EXHAUSTED,
	DSC_LOCAL_HOLDING_DOWN, 
	DSC_LOCAL_DELETING_SF,

	/* for remote initiated transaction state */
	DSC_ACK_PENDING,
	DSC_REMOTE_HOLDING_DOWN,
	DSC_REMOTE_DELETING_SF,

	/* common for both local initiated and remote initiated */
	DSC_BEGIN,
	DSC_FAILED,
	DSC_ERRED,
	DSC_SUCCESSFUL,
} dsc_state;

extern int dsc_primitive_handler(primitivetype primit, struct transaction_node* trans_node);
extern int dsc_msg_handler(void *dsc_msg, int msg_type, struct transaction_node* trans_node);

#endif

