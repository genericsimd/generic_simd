/* ----------------------------------------------------------------------------
   IBM Confidential

   IBM Wireless M2M Platform   

   (C)Copyright IBM Corp. 2008, 2009, 2010, 2011

   All Rights Reserved.

   File Name: mac_ul_pdu_parser.h

   Change Activity:

   Date             Description of Change                       By
   -----------      ---------------------                       --------
   03-Aug.2008		Created                                     Chen Lin

   ----------------------------------------------------------------------------
   PROLOG END TAG zYx                                                         */

#ifndef __MAC_UL_PDU_PARSER_H__
#define __MAC_UL_PDU_PARSER_H__
#include "mac_header.h"
#include "mac_config.h"
#include "mac_hcs.h"
#include "mac_crc.h"
#include "mac_ul_pdu_queue.h"
#include "ul_mgt_msg_queue.h"
#include "mac_connection.h"
#include "br_queue.h"
#include "mac_br_queue.h"
#include "debug.h"
int parse_frame_pdu(physical_subframe* phy_subframe, pdu_frame_queue* ul_pduq, ul_br_queue* brqlist, mgt_msg_queue *ul_msgq);

int parse_burst_pdu(int frame_num, u_char* burst, int length, pdu_frame_queue* ul_pduq,  ul_br_queue* brqlist, mgt_msg_queue *ul_msgq);


#endif
