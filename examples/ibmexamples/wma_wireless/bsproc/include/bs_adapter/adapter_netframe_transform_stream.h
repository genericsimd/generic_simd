/* ----------------------------------------------------------------------------
 * IBM Confidential

 IBM Wireless M2M Platform

 (C)Copyright IBM Corp. 2009, 2010, 2011

 All Rights Reserved.

 File Name: adapter_netframe_transform_stream.h

 Change Activity:

 Date             Description of Change                            By
 -----------      ---------------------                            --------
 20-May 2011       Created                                         jian jun chang 

 ----------------------------------------------------------------------------
 PROLOG END TAG zYx                                                         */
#ifndef _ADAPTER_FRAME_TRANSFORM_STREAM_H_
#define _ADAPTER_FRAME_TRANSFORM_STREAM_H_
#include "adapter_net_frame.h"
#include "adapter_config.h"
int netframe_transform_stream(struct net_frame *p_net_frame,void *p_buffer,int *p_size);
#endif

