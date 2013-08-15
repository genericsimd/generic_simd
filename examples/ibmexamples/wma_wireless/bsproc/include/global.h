/* ----------------------------------------------------------------------------
 * IBM Confidential

   IBM Wireless M2M Platform

   (C)Copyright IBM Corp. 2009, 2010, 2011

   All Rights Reserved.

   File Name: global.h

   Change Activity:

   Date             Description of Change                            By
   -----------      ---------------------                            --------
   12-May 2011       Created                                         Zhu, Zhenbo

   ----------------------------------------------------------------------------
   PROLOG END TAG zYx                                                         */

#ifndef __GLOBAL_H_
#define __GLOBAL_H_

#define SCHED_METHOD SCHED_RR

//#define G_TX (8191.0)

#define RET_ERROR 1
#define RET_NOTMATCH 2

#define THREADHOLD 23000

#define INITIAL_DELAY 20000

#define ANTENNA_NUM 2
#define CARRIER_NUM 4

#define MAX_SYMBOL_LEN 1088

#define UL_MAP_STORE 8
#define UL_MAP_DELAY 2
#define UL_MAP_MAX_LEN 8192

#define FRM_NUM_ADJ 1

#define MAX_FREQ_SET_NUM 11

#define RE_SYNC_THD 5

#endif

