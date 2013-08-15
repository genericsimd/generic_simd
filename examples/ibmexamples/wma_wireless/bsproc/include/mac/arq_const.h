/* ----------------------------------------------------------------------------
   IBM Confidential

   IBM Wireless M2M Platform   

   (C)Copyright IBM Corp. 2008, 2009, 2010, 2011

   All Rights Reserved.

   File Name: arq_const.h

   Change Activity:

   Date             Description of Change                            By
   -----------      ---------------------                            --------
   1-Oct.2008       Created                                          Uma Devi

   ----------------------------------------------------------------------------
   PROLOG END TAG zYx                                                         */
#ifndef _ARQ_CONST_H
#define _ARQ_CONST_H

#include <time.h>

#define ARQ_BSN_MODULUS	2048

#define ARQ_FEEDBACK_MSG	33
#define ARQ_DISCARD_MSG		34
#define ARQ_RESET_MSG		35

#define ARQ_WINDOW_SIZE_DEF			(ARQ_BSN_MODULUS >> 1)
#define ARQ_BLOCK_SIZE_DEF  		256
#define ARQ_BLOCK_LIFETIME_DEF		(struct timespec){ 0, 600000000 }
#define ARQ_SYNC_LOSS_TIMEOUT_DEF	(struct timespec){30,0}
#define ARQ_RETRY_TIMEOUT_DEF		(struct timespec){ 0, 70000000 }
#define ARQ_PURGE_TIMEOUT_DEF		(struct timespec){ 0, 600000000 }
#define ARQ_RESET_TIMEOUT_DEF		(struct timespec){0,70000000}
#define ARQ_ACK_INTERVAL			(struct timespec){ 0, 10000000 }
#define ARQ_ACK_INTERVAL_INIT		(struct timespec){ 0, 10000000 }

#define ARQ_SELECTIVE_ACK		0
#define ARQ_CUMULATIVE_ACK		1
#define ARQ_CUM_SEL_ACK			2
#define ARQ_CUM_BLK_SEQ_SEL_ACK	3


#define ARQ_MGMT_MSG_Q_SIZE		4096

int dl_timer_thread_done ;
int ul_timer_thread_done;
//For local use only
#define ARQ_BLK_LOSS_PROB		0.05
#define MAX_CONNS_PER_THREAD	1024

#define MAX_DL_TIMER_THREADS	1
#define MAX_UL_TIMER_THREADS	1

#define MAX_RX_THREADS  1
#define MAX_TX_THREADS  1

//#define TEST_DL_SYNC_LOSS
//#define TEST_RETRY_TIMEOUT
//#define TESTING_PURGE_TIMEOUT
//#define SIMULATE_ARQ_LOSS
//#define FEEDBACK_TEST 
//#define TEST_BLOCK_LIFETIME_WITH_DISCARD
//#define TEST_UL_SYNC_LOSS

#endif // _ARQ_CONST_H
