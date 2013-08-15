/* ----------------------------------------------------------------------------
   IBM Confidential

   IBM Wireless M2M Platform   

   (C)Copyright IBM Corp. 2008, 2009, 2010, 2011

   All Rights Reserved.

   File Name: CS.h

   Change Activity:

   Date             Description of Change                   By
   -----------      ---------------------					--------
   1-Oct.2008       Created                                 Malolan Chetlur

   ----------------------------------------------------------------------------
   PROLOG END TAG zYx                                                         */

#ifndef _CS_MAC__
#define _CS_MAC__

#include <stdlib.h>
#include "mac.h"
#include "logical_packet.h"
#include "mac_sdu_queue.h"
#include "sdu_cid_queue.h"
#include "cs_sdu_header.h"
#include "classifier.h"
#include "mac_connection.h"
#include "bs_ss_info.h"
#include "memmgmt.h"
#include "mac_hash.h"
#include "dl_exp_params.h"

// All global declarations of data strcutures and functions 
// pertaining to CS layer

extern sdu_queue* dl_sdu_queue; // the MAC SDU for the downlink, 

extern void* CS(void*);

//initialize CSL data structures and parametes
extern void cs_init();

//invokeCS layer call for IPV4 in downlink
extern void IPV4_CS_DL(cs_sdu_header* sdu);

//perform classification in DL
extern void classify(connection_classifier_info** conn_clsfr_array, cls_func_ptr* f_ptr, cs_sdu_header* sdu);

//perform packet header suppression
extern void performPHS(connection_classifier_info** conn_clsfr_array, cls_func_ptr* f_ptr, cs_sdu_header* sdu);

//enqueue the higher layer SDU to CID queues
extern void enqueue_cs_dl(cs_sdu_header* mac_sdu);

#endif
