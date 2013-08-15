/* ----------------------------------------------------------------------------
 *    IBM Confidential

   IBM Wireless M2M Platform

   (C)Copyright IBM Corp. 2009, 2010, 2011

   All Rights Reserved.

   File Name: rru_adapter.h

   Change Activity:

   Date             Description of Change                            By
   -----------      ---------------------                            --------
   5-May 2011       Created                                        Zhu, Zhen Bo

   ----------------------------------------------------------------------------
   PROLOG END TAG zYx                                                         */

#ifndef _RRU_ADAPTER_H
#define _RRU_ADAPTER_H

#include "global.h"

// Frame length.
#define FIX_PAYLOAD_LEN 1104
#define FIX_DATA_LEN 1088
#define DATA_HDR_LEN 16
#define CMD_PAYLOAD_LEN 30

#define PACKET_PER_SYMBOL 8
#define PACKET_PER_ANT 4
#define SAMPLE_PER_PACKET 272
#define SAMPLE_PER_UL_FRM 272

#define CMD_TYPE_FLAG 0xFF
#define UL_TYPE_FLAG 0x01
#define DL_TYPE_FLAG 0x00
#define CHANNEL_TYPE 1

#define HEAD_FLAG 1
#define BODY_FLAG 2
#define TAIL_FLAG 3

#define RECV_BUFFER_LEN (256 * 1024 * 1024)

//#define RRU_SYS_POWER_COMPILE                                      
                                                                     
                                                                     
#ifdef RRU_ADAPTER_SYS_POWER_COMPILE                               
                                                                   
#define RRU_ADAPTER_NTOHL                                          
                                                                   
#define RRU_ADAPTER_HTONL                                          
                                                                   
#define RRU_ADAPTER_NTOHS                                          
                                                                   
#define RRU_ADAPTER_HTONS                                          
                                                                   
#else                                                              
                                                                   
#define RRU_ADAPTER_NTOHL    ntohl                                 
                                                                   
#define RRU_ADAPTER_HTONL    htonl                                 
                                                                   
#define RRU_ADAPTER_NTOHS    ntohs                                 
                                                                   
#define RRU_ADAPTER_HTONS    htons                                 
                                                                   
#endif                                                             
                                                                   
                                                                     
extern pthread_mutex_t mutex_rru_frm_idx;
extern unsigned short g_rruframe_num;

#pragma pack(1)

struct rru_cmd_payload
{
    unsigned short pkt_crc_count;
    unsigned short pkt_lost_count;
    unsigned short pkt_late_count;
    unsigned int pkt_start_time;
    unsigned int pkt_end_time;
};

#pragma pack()


/* keep 32 bit x align */
struct rru_data_param
{
    float dgain[ANTENNA_NUM];
    unsigned short frm_num;
    unsigned short end_flag;
    unsigned int pkt_count;
    unsigned int carrier;  
};

#pragma pack(1)

struct rru_data_hdr
{
    unsigned int rru_id;
    unsigned char data_type;
    unsigned short length;
    unsigned char chan;
    unsigned short frm_num;
    unsigned short seq_no;
    unsigned char frg_flag;
    char dgain;
#ifdef _NEW_TRANS_GREN_ENABLE_
    unsigned char freq;
    unsigned char carrier:4;
    unsigned char sample_rate:4;
#else
    unsigned short resv;
#endif
};
// __attribute__((packed));
#pragma pack()

int ethrru_init (void);
int ethrru_close (void);

int ethrru_data_tx (char * p_tx_buf, int symbol_num,
        struct rru_data_param * param);

int ethrru_data_rx_pkt(char * p_rx_buf, int data_len);

int ethrru_data_rx (char * p_rx_buf,
                    int symbol_num,
                    struct rru_data_param * param,
                    int flag,
                    int tdd);

#ifdef ENABLE_NEW_TRANS_GREN
void ethrru_symbol_rx_reset ();
#endif

int ethrru_symbol_rx (char * rx_buf, int symbol_num, struct rru_data_param * param);
#endif
