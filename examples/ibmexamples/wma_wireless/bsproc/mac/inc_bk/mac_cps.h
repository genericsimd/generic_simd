/* ----------------------------------------------------------------------------
   IBM Confidential

   IBM Wireless M2M Platform   

   (C)Copyright IBM Corp. 2008, 2009, 2010, 2011

   All Rights Reserved.

   File Name: mac_cps.h

   Change Activity:

   Date             Description of Change                   By
   -----------      ---------------------					--------
   3-Aug.2008       Created                                 Chen Lin

   ----------------------------------------------------------------------------
   PROLOG END TAG zYx                                                         */


#ifndef __MAC_CPS_H__
#define __MAC_CPS_H__

#include <pthread.h>
#include "mac_dl_cps_controller.h"
#include "mac_ul_cps_controller.h"
#include "br_queue.h"
#include "mac_amc.h"
#include "mac_br_queue.h"
#include "thread_sync.h"
//#include "mac_timer_schedulertrigger.h"

//timer_queue *  mac_cps_timerq;

//pthread_t mac_cps_timer_thread;


void* mac_cps_handler();

void* dts_timer_thread();

int release_mac_cps_handler();

#endif
