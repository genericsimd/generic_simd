/* ----------------------------------------------------------------------------
   IBM Confidential

   IBM Wireless M2M Platform   

   (C)Copyright IBM Corp. 2008, 2009, 2010, 2011

   All Rights Reserved.

   File Name: mac_arq.c

   Change Activity:

   Date             Description of Change                   By
   -----------      ---------------------					--------
   1-Oct.2008       Created                                 Uma Devi

   ----------------------------------------------------------------------------
   PROLOG END TAG zYx                                                         */

#include "mac_arq.h"

int initialize_arq(int cid, arq_state** arq){
    (*arq) = malloc(sizeof(arq_state));
    memset(*arq, sizeof(arq_state), 0);
    (*arq)->cid = cid;
    (*arq)->rx_highest_bsn = 0;
    (*arq)->rx_window_start = 0;
    (*arq)->arq_window_size = 4;
    return 0;
}


