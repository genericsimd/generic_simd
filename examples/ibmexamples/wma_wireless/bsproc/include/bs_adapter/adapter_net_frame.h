/* ----------------------------------------------------------------------------
 * IBM Confidential

 IBM Wireless M2M Platform

 (C)Copyright IBM Corp. 2009, 2010, 2011

 All Rights Reserved.

 File Name: adapter_net_frame.h

 Change Activity:

 Date             Description of Change                            By
 -----------      ---------------------                            --------
 20-May 2011       Created                                         jian jun chang 

 ----------------------------------------------------------------------------
 PROLOG END TAG zYx                                                         */
#ifndef   _NET_FRAME_DEFINE_H_
#define   _NET_FRAME_DEFINE_H_

#include "adapter_config.h"
#define PACKLENGTHSTARTPOS         0
#define PACKLENGTHSIZE             sizeof(u_int16_t)
#define PACKNUMSTARTPOS            (sizeof(u_int16_t) + 0)
#define PACKNUMSIZE                sizeof(u_int8_t)
#define PACKTYPESTARTPOS           (sizeof(u_int8_t)+sizeof(u_int16_t)) 
#define PACKTYPESIZE               sizeof(u_int16_t)   
#define PARAMETERNUMSTARTPOS       (sizeof(u_int8_t)+sizeof(u_int16_t) + sizeof(u_int16_t) ) 
#define PARAMETERNUMSIZE           sizeof(u_int16_t)   


#define    DL_NETFRAME_BUFFER_SIZE              2048
#define    DL_DATASTARTPOS                      4   
struct   net_node
{
    int             ibytelength;  
    int             ibitlength;
    int             ibitoffset;
    u_int8_t        *p_payload;
    struct net_node         *next;
};


struct net_frame
{
    long            frame_num;
    u_int8_t        operator_version;
    struct net_node         *p_net_node_data;
};

struct net_frame*  get_new_net_frame();
int free_net_frame(struct net_frame *p_net_frame);
#endif 

