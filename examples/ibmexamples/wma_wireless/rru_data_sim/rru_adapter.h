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

#ifndef _RRH_ADAPTER_H
#define _RRH_ADAPTER_H

// Frame length.
#define FIX_PAYLOAD_LEN 1104
#define FIX_DATA_LEN 1088
#define DATA_HDR_LEN 16
#define PACKET_PER_SYMBOL 8
#define PACKET_PER_ANT 4
#define SAMPLE_PER_PACKET 272

#define CMD_TYPE_FLAG 0xFF
#define UL_TYPE_FLAG 0x01
#define DL_TYPE_FLAG 0x00
#define CHANNEL_TYPE 1


#define RECV_BUFFER_LEN (256 * 1024 * 1024)

#pragma pack(1)

struct rrh_data_hdr
{
    unsigned int rru_id;
    unsigned char data_type;
    unsigned short length;
    unsigned char chan;
    unsigned short frm_num;
    unsigned short seq_no;
    unsigned char frg_flag;
    unsigned char dgain;
    unsigned short resv;
};
// __attribute__((packed));
#pragma pack()

int ethrru_init (void);
int ethrru_close (void);
int ethrru_data_tx (void * p_tx_buf, int symbol_num, unsigned int cmd_time);
int ethrru_data_rx (char * rx_buf, int symbol_num);

#endif
