/* ----------------------------------------------------------------------------
 * IBM Confidential

 IBM Wireless M2M Platform

 (C)Copyright IBM Corp. 2009, 2010, 2011

 All Rights Reserved.

 File Name: adapter_stream_transform_netframe.h

 Change Activity:

 Date             Description of Change                            By
 -----------      ---------------------                            --------
 22-Apr 2011       Created                                         jian jun chang 

 ----------------------------------------------------------------------------
 PROLOG END TAG zYx                                                         */
#ifndef _ADAPTER_STREAM_TRANSFORM_NETFRAME_H_
#define _ADAPTER_STREAM_TRANSFORM_NETFRAME_H_
#include "adapter_net_frame.h"

int stream_transform_netframe(void *p_buffer,int *pflag,struct net_frame **p_burstframe);
#endif

