/* ----------------------------------------------------------------------------
   IBM Confidential

   IBM Wireless M2M Platform   

   (C)Copyright IBM Corp. 2010, 2011

   All Rights Reserved.

   File Name: mac_ss_ranging.h

   Change Activity:

   Date             Description of Change                   By
   -----------      ---------------------					--------
   1-July.2010		Created                                 Parul Gupta

   ----------------------------------------------------------------------------
   PROLOG END TAG zYx                                                         */

#ifndef _MAC_SS_INIT_RANGING_H_
#define _MAC_SS_INIT_RANGING_H_

#include <stdio.h>
#include "constants.h"
#include "mac.h"
#include "ranging_mm.h"
#include "sll_fifo_q.h"
#include "mac_sdu_queue.h"
#include "app_timer.h"
#include "phy_params.h"
#include "ulmap.h"
#include "ul_mgt_msg_queue.h"
#include "mac_common_data.h"

int init_ss_nw_entry();
void* ss_init_ranging(void*);
int set_rng_info(ranging_info *p_ranging_info);
int init_ss_periodic_rng();

void* ss_periodic_ranging(void*);

int t4_expired();
int t3_expired();
int trigger_cdmacode_now();
int set_phy_corrections(rng_rsp_msg *rng_rsp);
#endif

