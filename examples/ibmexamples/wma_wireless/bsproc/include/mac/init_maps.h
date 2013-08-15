/* ----------------------------------------------------------------------------
   IBM Confidential

   IBM Wireless M2M Platform   

   (C)Copyright IBM Corp. 2008, 2009, 2010, 2011

   All Rights Reserved.

   File Name: init_maps.h

   Change Activity:

   Date             Description of Change                   By
   -----------      ---------------------					--------
   1-Oct.2008       Created                                 Parul Gupta

   ----------------------------------------------------------------------------
   PROLOG END TAG zYx                                                         */

#ifndef _INIT_MAPS_H
#define _INIT_MAPS_H

#include <math.h>
#include "CS.h"
#include "phy_params.h"
#include "logical_packet.h"
#include "mac_frame.h"

int init_pdu_map(logical_pdu_map* pm);

int init_logical_packet(logical_packet* lp);

int init_burst_map(logical_burst_map* lbm);

int dl_subframe_map_init(logical_dl_subframe_map* frame_map);

int init_dlmap(dl_map_msg *dl_map);

int init_ulmap(ul_map_msg *ul_map);

int init_ulmap_ie(ul_map_ie *ie);
#endif
