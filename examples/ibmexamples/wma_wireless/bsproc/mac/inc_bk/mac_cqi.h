/* ----------------------------------------------------------------------------
   IBM Confidential

   IBM Wireless M2M Platform   

   (C)Copyright IBM Corp. 2008, 2009, 2010, 2011

   All Rights Reserved.

   File Name: mac_br_queue.h

   Change Activity:

   Date             Description of Change                   By
   -----------      ---------------------					--------
   1-Oct.2008       Created                                 Chen Lin

   ----------------------------------------------------------------------------
   PROLOG END TAG zYx                                                         */
#ifndef __MAC_CQI_H__
#define __MAC_CQI_H__

#include "ul_mgt_msg_queue.h"

typedef struct channelqualityinfo cqi;

struct channelqualityinfo{
    int ss_id;
    int direction; // downstream or upstream
    int history_index; // identifies the histogram samples in the table for each subscriber station

    int start_frame;
    int duration; // cumulative measurement duration on the channel in multiple of Ts.
    int basic_report;
    int mean_cinr;
    int mean_rssi;
    int std_deviation_cinr;
    int std_deviation_rssi;
    cqi* next;
};

typedef struct cqiqueue cqi_queue;
struct cqiqueue{
    int channel_num;
    int history_num;
    cqi* cqi_header;
    cqi* cqi_tail;
    cqi_queue* next;
};


cqi_queue* ul_cqiq_header;
cqi_queue* dl_cqiq_header;


typedef struct burstcqi burst_cqi;
struct burstcqi {
    int ssid;
    int num_ofdm_symbols;
    int num_subcarriers;
    int ofdma_symbol_offset;
    int subcarrier_offset;
    struct cqi_element **cqi_matrix;
    burst_cqi* next;
};

struct cqi_element {
u_int8_t cir_mag;  // The magnitude of the estimated Channel Impulse Response
u_int8_t cinr;     // Carrier to Interference & Noise Ratio
}


//mgt_msg_queue* ul_cqi_msg_queue;

//extern int enqueue_cqi_mgt_msg(mgt_msg_queue* ul_msg_queue, int frame_num, int cid, int msg_type, int length, void* data);

//extern int get_cqi_matrix(burst_cqi* cqi_info_header);

#endif
