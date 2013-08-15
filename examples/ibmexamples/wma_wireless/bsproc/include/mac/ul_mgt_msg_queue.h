/* ----------------------------------------------------------------------------
   IBM Confidential

   IBM Wireless M2M Platform   

   (C)Copyright IBM Corp. 2008, 2009, 2010, 2011

   All Rights Reserved.

   File Name: ul_mgt_msg_queue.h

   Change Activity:

   Date             Description of Change                   By
   -----------      ---------------------					--------
   3-Aug.2008       Created                                 Chen Lin

   ----------------------------------------------------------------------------
   PROLOG END TAG zYx                                                         */

#ifndef __UL_MGT_MSG_QUEUE__
#define __UL_MGT_MSG_QUEUE__
#include "ucd_dcd_parser.h"


// UL MAC Management Message Queues (note there are multiple such queues)

#include <string.h>
#include "mac.h"
#include "logical_packet.h"
#include "test_routines.h"
#include "free_maps.h"

#define MAX_MMM_CLASS 12
#define NETWORK_ENTRY_MMM_INDEX 0
// power control and sleep
#define POWER_CONTROL_MMM_INDEX 1
// bandwidth request
#define BR_MMM_INDEX 2
// cqich report, channel feedback
#define CHANNEL_REPORT_MMM_INDEX 3
// handover related
#define HO_MMM_INDEX 4
// dynamic service
#define DS_MMM_INDEX 5
// arq related
#define ARQ_MMM_INDEX 6

// For SS, UL mgmt mesg queues are needed for ULMAP, DLMAP, UCD, DCD etc also
#define UCD_MMM_INDEX 7
#define DCD_MMM_INDEX 8
#define ULMAP_MMM_INDEX 9
#define DLMAP_MMM_INDEX 10
#define PKM_MMM_INDEX 11
// Broadcast MMMs
#define UCD 0
#define DCD 1
#define DLMAP 2
#define ULMAP 3

// network entry
#define RNG_REQ 4
#define RNG_RSP 5
#define REG_REQ 6
#define REGN_RSP 7
#define PKM_REQ 9
#define PKM_RSP 10

#define SBC_REQ 26
#define SBC_RSP 27

// dynamic service
#define DSA_REQ 11
#define DSA_RSP 12
#define DSA_ACK 13
#define DSC_REQ 14
#define DSC_RSP 15
#define DSC_ACK 16
#define DSD_REQ 17
#define DSD_RSP 18
#define DSX_RVD 30

// channel measurement
#define REP_REQ 36
#define REP_RSP 37

// power control
#define MOB_SLP_REQ 50
#define MOB_SLP_RSP 51
#define PCM_REQ 63
#define PCM_RSP 64

// handover
#define MOB_BSHO_REQ 56
#define MOB_MSHO_REQ 57
#define MOB_BSHO_RSP 58
#define MOB_HO_IND 59

// ARQ
#define ARQ_FEEDBACK 33
#define ARQ_DISCARD 34
#define ARQ_RESET 35

// msg_type in struct mgtmsg (see below)
#define BR_INCREMENTAL_HEADER 300
#define BR_AGGREGATE_HEADER 301
#define PHY_CHANNEL_REPORT_HEADER 302
#define BR_UL_TX_POWER_REPORT_HEADER 303
#define BR_CINR_REPORT_HEADER 304
#define BR_UL_SLEEP_CONTROL_HEADER 305
#define SN_REPORT_HEADER 306
#define CQICH_ALLOCATION_REQUEST_HEADER 307
#define FEEDBACK_HEADER 308
#define MIMO_CHANNEL_FEEDBACK_HEADER 309
#define GRANT_MANAGEMENT_SUBHEADER 310
#define ARQ_FEEDBACK_IE 311
#define MIMO_MODE_FEEDBACK_EXTENDED_SUBHEADER 312
#define UL_TX_POWER_REPORT_EXTENDED_SUBHEADER 313
#define MINI_FEEDBACK_EXTENDED_SUBHEADER 314
#define UNFRAG_MANAGEMENT_MESSAGE 315
#define FRAG_MANAGEMENT_MESSAGE 316

typedef struct mgtmsg mgt_msg;

struct mgtmsg{
    int cid;
    int msg_type;  //according to the #defines above 
    int length;
    void* data;
    int rcv_frame_num; // frame number in which the message was received
    mgt_msg* next;
    mgt_msg* prev;
};

typedef struct {
    int length;
    mgt_msg *head;
    mgt_msg *tail;
    pthread_mutex_t qmutex;
    pthread_cond_t notempty;
}mgt_msg_queue;

//mgt_msg_queue* ul_gen_mgt_msg_queue; // mgt msgs other than cqi and power mgt

//mgt_msg_queue* ul_cqi_mgt_msg_queue;

//mgt_msg_queue* ul_power_mgt_msg_queue;

mgt_msg_queue *ul_msg_queue;

// initialize the queue
extern mgt_msg_queue* ul_mgt_msg_queue_init();

extern int enqueue_ul_mgt_msg(mgt_msg_queue* mgt_queue, int frame_num, int cid, int msg_type, int length, void* data);

// waits if the queue is empty
extern mgt_msg *dequeue_ul_mgt_msg_queue(mgt_msg_queue *mgt_queue);

// returns immediately if the queue is empty
//extern mgtmsg *dequeue_non_blocking(mgt_msg_queue *mgt_queue);
int get_ul_mgt_msg_queue(mgt_msg_queue** ul_msgq);
int enqueue_specific_queue(mgt_msg_queue* mgt_queue, int frame_num, int cid, int msg_type, int length, void* data);
mgt_msg *dequeue_ul_mmq_nowait(mgt_msg_queue *mgt_queue);
int free_ul_mm_queue(mgt_msg_queue **p_mgt_q);

#endif

