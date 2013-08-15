/* ----------------------------------------------------------------------------
   IBM Confidential

   IBM Wireless M2M Platform   

   (C)Copyright IBM Corp. 2008, 2009, 2010, 2011

   All Rights Reserved.

   File Name: scheduler_utils.h

   Change Activity:

   Date             Description of Change                   By
   -----------      ---------------------					--------
   1-Oct.2008       Created                                 Parul Gupta

   ----------------------------------------------------------------------------
   PROLOG END TAG zYx                                                         */

#ifndef _SCHEDULER_UTILS_H
#define _SCHEDULER_UTILS_H

#include <math.h>
#include "util.h"
#include "mac.h"
#include "mac_sdu_queue.h"
#include "mac_connection.h"
#include "arq_ds.h"
#include "mac_header.h"
#include "dl_exp_params.h"

int estimate_sdu_PnF_overhead(int cid, sdu_cid_queue* sdu_cid_q, int data_bytes_needed);

int estimate_arqReTx_PnF_overhead(int cid, ARQ_ReTX_Q_aggr_info arq_info);
int estimate_data_bytes_from_total(int cid, sdu_cid_queue* sdu_cid_q, int total_bytes_available);
int init_subheader_len(short cid, u_int8_t is_frag, u_int8_t is_arq, int *psh_len, int *fsh_len);

#endif

