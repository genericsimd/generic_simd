/* ----------------------------------------------------------------------------
   IBM Confidential

   IBM Wireless M2M Platform   

   (C)Copyright IBM Corp. 2008, 2009, 2010, 2011

   All Rights Reserved.

   File Name: mac_frame.c

   Change Activity:

   Date             Description of Change                   By
   -----------      ---------------------					--------
   3-Aug.2008       Created                                 Chen Lin

   ----------------------------------------------------------------------------
   PROLOG END TAG zYx                                                         */

#include "mac_frame.h"

int initialize_logical_subframe_map(logical_dl_subframe_map** subframe_map){
    (*subframe_map) = (logical_dl_subframe_map*) malloc(sizeof(logical_dl_subframe_map));
    memset((*subframe_map), 0, sizeof(logical_dl_subframe_map));
    
    return 0;
}

int release_logical_subframe_map(logical_dl_subframe_map* subframe_map){
    logical_burst_map* cur_burst_map;
    logical_burst_map* pre_burst_map;
    logical_pdu_map* cur_pdu_map;
    logical_pdu_map* pre_pdu_map;
    if (subframe_map)
    {
        if (subframe_map->fch)
        {
            free(subframe_map->fch);
            subframe_map->fch = NULL;
        }
        cur_burst_map = subframe_map->burst_header;
        while (cur_burst_map)
        {
            pre_burst_map = cur_burst_map;
            cur_burst_map = pre_burst_map->next;
            cur_pdu_map = pre_burst_map->pdu_map_header;
            while (cur_pdu_map)
            {
                pre_pdu_map = cur_pdu_map;
                cur_pdu_map = pre_pdu_map->next;
                // do not need to release the logical packet container which has already been released when doing the frag and pack
                if (pre_pdu_map->transport_sdu_map)
                {
                    free(pre_pdu_map->transport_sdu_map);
                    pre_pdu_map->transport_sdu_map = NULL;
                }
                if (pre_pdu_map->mac_msg_map)
                {
                    free(pre_pdu_map->mac_msg_map);
                    pre_pdu_map->mac_msg_map = NULL;
                }
                if (pre_pdu_map->arq_sdu_map)
                {
                    free(pre_pdu_map->arq_sdu_map);
                    pre_pdu_map->arq_sdu_map = NULL;
                }
                free(pre_pdu_map);
                pre_pdu_map = NULL;
            }
            free(pre_burst_map);
            pre_burst_map = NULL;
        }
        free(subframe_map);
        subframe_map = NULL;
        
    }
    

    return 0;
}
