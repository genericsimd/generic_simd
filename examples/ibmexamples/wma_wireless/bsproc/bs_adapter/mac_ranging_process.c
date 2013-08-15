/* ----------------------------------------------------------------------------
 * IBM Confidential

 IBM Wireless M2M Platform

 (C)Copyright IBM Corp. 2009, 2010, 2011

 All Rights Reserved.

 File Name: mac_ranging_process.c

 Change Activity:

 Date             Description of Change                            By
 -----------      ---------------------                            --------
 2-Apr 2011       Created                                         jian jun chang 

 ----------------------------------------------------------------------------
 PROLOG END TAG zYx                                                         */
#include "mac_ranging_process.h"

void* mac_ranging_received_handle()
{
    //the interface is support by framework,use for received in phy-layer ;
    struct queue_obj     *queueobj = NULL;
    enqueueobj("phy_ranging_queue",queueobj,0);
    return NULL;
}

void* mac_ranging_send_handle()
{
    struct queue_obj     *queueobj = NULL;
    dequeueobj("mac_ranging_queue",&queueobj);
    //the interface is support by framework,use for send ;
    return NULL;
}
