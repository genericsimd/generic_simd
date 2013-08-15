/* ----------------------------------------------------------------------------
   IBM Confidential

   IBM Wireless M2M Platform   

   (C)Copyright IBM Corp. 2008, 2009, 2010, 2011

   All Rights Reserved.

   File Name: free_maps.h

   Change Activity:

   Date             Description of Change                   By
   -----------      ---------------------					--------
   1-Oct.2008       Created                                 Parul Gupta

   ----------------------------------------------------------------------------
   PROLOG END TAG zYx                                                         */

#ifndef _FREE_MAPS_H
#define _FREE_MAPS_H

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include "mac.h"
#include "mac_sdu_queue.h"
#include "phy_params.h"
#include "br_queue.h"
#include "mac_frame.h"
#include "mac_amc.h"
#include "memmgmt.h"
#include "scheduler.h"
#include "mac_connection.h"


// The memory for ULMAP IE is dynamically allocated per frame as needed
// needs to be freed after the frame's lifetime
int free_ulmap(ul_map_msg* ul_map);

// The memory for DLMAP IE is dynamically allocated per frame as needed
// needs to be freed after the frame's lifetime
int free_dlmap(dl_map_msg* dl_map);

int free_dl_subframe_map(logical_dl_subframe_map* frame_map);

int free_logical_packet(logical_packet* lp);
int free_logical_burst_map(logical_burst_map* lbm);

#endif
