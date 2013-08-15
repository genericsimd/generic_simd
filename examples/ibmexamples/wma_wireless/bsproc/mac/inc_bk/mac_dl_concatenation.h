/* ----------------------------------------------------------------------------
   IBM Confidential

   IBM Wireless M2M Platform   

   (C)Copyright IBM Corp. 2008, 2009, 2010, 2011

   All Rights Reserved.

   File Name: mac_dl_concatenation.h

   Change Activity:

   Date             Description of Change                   By
   -----------      ---------------------					--------
   03-Aug.2008      Created                                 Chen Lin

   ----------------------------------------------------------------------------
   PROLOG END TAG zYx                                                         */


#ifndef __MAC_DL_CONCATENATION_H__
#define __MAC_DL_CONCATENATION_H__


#include <stdlib.h>


#include "string.h"
#include "mac_config.h"
#include "mac_frame.h"
#include "mac_headermsg_builder.h"
#include "mac_hcs.h"
#include "mac_crc.h"
#include "mac_connection.h"

int concatenation(logical_packet* pdu_list_head, u_char* burst, int length);

int release_logical_pdu_list(logical_packet* pdu_list_head);

#endif
