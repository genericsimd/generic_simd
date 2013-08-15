#ifndef __MAC_DSD_PROCESSING_
#define __MAC_DSD_PROCESSING_

#include <stdio.h>
#include <assert.h>
#include "constants.h"
#include "sll_fifo_q.h"
#include "mac_sdu_queue.h"
#include "dl_exp_params.h"
#include "ul_mgt_msg_queue.h"
#include "mac_qos_mm.h"
#include "mac_dsa_list.h"
#include "app_timer.h"
#include "util.h"
#include "mac_connection.h"
#include "memmgmt.h"

#define DSD_LOCAL_RSP_PENDING 98
#define DSD_ERRED 97
#define DSD_ENDED 96
#define DSD_SUCCEEDED 95
#define DSD_REMOTE_HOLDING_DOWN 94
#define DSD_LOCAL_HOLDING_DOWN 93

/*#define SF_DELETED 92
#define SF_DSD_REQ_LOST 91
#define SF_DELETE_REMOTE 90
#define SF_DELETE 89
*/


extern sll_fifo_q *dsd_q;
extern struct transaction_list* dsd_trans_list;


int t7_expired(void* trans_id_ptr);
int t10_expired(void *trans_id_ptr);
int dsd_local(u_int32_t sfid, primitivetype  primit);
int dsd_msg_processing(void* dsd_msg1, int msg_length, struct sf_status_node *sf_st_node);
int mac_dsd_init(int* sfid);

#endif
