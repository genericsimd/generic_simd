/* ----------------------------------------------------------------------------
   IBM Confidential

   IBM Wireless M2M Platform   

   (C)Copyright IBM Corp. 2008, 2009, 2010, 2011

   All Rights Reserved.

   File Name: mac_scheduling.h

   Change Activity:

   Date             Description of Change                   By
   -----------      ---------------------					--------
   1-Oct.2008		Created                                 Parul Gupta

   ----------------------------------------------------------------------------
   PROLOG END TAG zYx                                                         */

#ifndef __MAC_SCHEDULING_H__
#define __MAC_SCHEDULING_H__

#include "mac_frame.h"
#include "mac_sdu_queue.h"
#include "br_queue.h"

int bs_scheduling(sdu_queue* dl_sduq, br_queue** brqlist, logical_dl_subframe_map * frame_map,int num_dl_subch,int num_ul_subch);

#endif
