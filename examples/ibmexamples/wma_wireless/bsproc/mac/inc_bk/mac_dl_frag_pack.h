/* ----------------------------------------------------------------------------
   IBM Confidential

   IBM Wireless M2M Platform   

   (C)Copyright IBM Corp. 2008, 2009, 2010, 2011

   All Rights Reserved.

   File Name: mac_dl_frag_pack.h

   Change Activity:

   Date             Description of Change                   By
   -----------      ---------------------					--------
   03-Aug.2008      Created                                 Chen Lin

   ----------------------------------------------------------------------------
   PROLOG END TAG zYx                                                         */

#ifndef __MAC_DL_FRAG_PACK_H__
#define __MAC_DL_FRAG_PACK_H__

#include "mac_config.h"
#include "mac_frame.h"
#include "mac_connection.h"
#include "mac_header.h"
#include "mac_sdu_queue.h"
int fragpack(sdu_queue* sduq, logical_burst_map* burst_map, logical_packet** pdulisthead, logical_element** le_tobe_discard, int* status);

int release_sducontainer(logical_packet* sdulist, u_int8_t is_release_payload, logical_element** le_tobe_discard);

#endif
