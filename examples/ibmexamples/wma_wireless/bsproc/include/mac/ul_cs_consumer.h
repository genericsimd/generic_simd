/* ----------------------------------------------------------------------------
   IBM Confidential

   IBM Wireless M2M Platform   

   (C)Copyright IBM Corp. 2008, 2009, 2010, 2011

   All Rights Reserved.

   File Name: ul_cs_consumer.h

   Change Activity:

   Date             Description of Change                       By
   -----------      ---------------------                       --------
   3-Aug.2008       Created                                     Chen Lin

   ----------------------------------------------------------------------------
   PROLOG END TAG zYx                                                         */

#ifndef __UL_CS_CONSUMER_H__
#define __UL_CS_CONSUMER_H__

#include "mac_sdu_queue.h"
#include <pthread.h>
#include <sys/time.h>
#include <unistd.h>
#include "thread_sync.h"

#define CONSUME_INTERVAL 7

void* ul_cs(void* arg);

#endif
