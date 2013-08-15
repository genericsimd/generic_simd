/* ----------------------------------------------------------------------------
   IBM Confidential

   IBM Wireless M2M Platform   

   (C)Copyright IBM Corp. 2008, 2009, 2010, 2011

   All Rights Reserved.

   File Name: mac_amc.h

   Change Activity:

   Date             Description of Change                   By
   -----------      ---------------------					--------
   1-Oct.2008       Created                                 Partha Dutta

   ----------------------------------------------------------------------------
   PROLOG END TAG zYx                                                         */

#ifndef __MAC_AMC_H__
#define __MAC_AMC_H__
#include <sys/types.h>
#include "mac_config.h"
#include "mac.h"
#include "app_timer.h"

//#define AMC_TEST_PRINT

#define PACKET_UPDATE_MODULUS 100
#define ALPHA_CINR 0.0625

#define REP_REQ_PERIOD 3000000L //in microseconds.
//#define REP_REQ_PERIOD 1000000L //in microseconds.
#define REP_REQ_RETRY_TIMEOUT 500000L //in microseconds
#define NUM_REP_REQ_RETRIES 3

#ifdef SS_TX
#define ASIZE 1
#else
#define ASIZE 2
#endif

#ifndef SS_TX
#define TABLE_SIZE BASIC_CID_MAX_VALUE - BASIC_CID_MIN_VALUE
#else
#define TABLE_SIZE 1
#endif

#ifdef SS_TX
	int prev_dl_amc;
#endif
app_timer* rep_req_timer[TABLE_SIZE];
app_timer* rep_retry_timer[TABLE_SIZE];
int num_rep_retries_left[TABLE_SIZE];
pthread_mutex_t cinr_table_lock;
pthread_mutex_t crc_table_lock;
//If BS, then DL CINR is indexed 0 and UL CINR is indexed 1.
//If SS< DL CINR is indexed 0, UL CINR is absent
int shared_cinr_table[TABLE_SIZE][ASIZE];//dl cinr and then ul cinr
int shared_rssi_table[TABLE_SIZE][ASIZE];//dl rssi and then ul rssi
float shared_crc_table[TABLE_SIZE][ASIZE];//dl crc error rate, ul crc error rate

float average_cinr_table[TABLE_SIZE][ASIZE];//averaged dl_cinr,ul cinr
float average_rssi_table[TABLE_SIZE][ASIZE];//averaged dl_rssi,ul_rssi
float prior_average_cinr_table[TABLE_SIZE][ASIZE];

int crc_error_count_table[TABLE_SIZE][ASIZE];//averaged dl_crc_error_rate,ul_crc_error_rate

int total_crc_cinr_adjust[TABLE_SIZE][ASIZE];
int rep_cinr_adjust[TABLE_SIZE][ASIZE];

int transmit_rep_req(void* arg);
int rep_req_retry(void* arg);
void* rep_state_machine();

int cpe_packet_count[TABLE_SIZE];
int cpe_crc_error_count[TABLE_SIZE];

int current_mcs_table[TABLE_SIZE][ASIZE];

typedef struct ssamcinfo ss_amc_info;
struct ssamcinfo{
u_int64_t ss_mac;
int subchannel_num;
int ul_fec_code_modulation_type;
int dl_fec_code_modulation_type;
ss_amc_info* next;
};

typedef struct{
    int ss_num;
    ss_amc_info* ss_amc_head;
    pthread_mutex_t amc_lock;
}amc_info;

amc_info* amc_list;

extern int initialize_amc_info();

extern int get_amc_info(amc_info** amc_info_header);

extern int free_amc_info();

unsigned int packet_count[TABLE_SIZE];

extern int update_rep_cinr_adjust(int basic_cid);
extern int reset_rep_cinr_adjust(int basic_cid);

int update_ss_dl_link_quality_cpe(int basic_cid, int cinr, int rssi);
int update_ssl_ul_link_quality(int basic_cid, int cinr, int rssi);
int init_shared_tables();
int update_ul_crc_rate(int cid);
int update_average_crc_table_rx(int basic_cid, int crc_errors, int packets);
int update_dl_amcs(amc_info* amc_info_header);
int update_dl_amcs(amc_info* amc_info_header);
int update_amc_before_bs_scheduling(amc_info* amc_info_header);
int update_dl_link_quality_from_rep_rsp(int cid, int cinr, int rssi);
int update_dl_crc_rate_from_rep_rsp(int cid, int crc_error_num, int packet_num);
ss_amc_info* get_ss_amc_info(amc_info* amc, int ss_num);

extern int get_ss_mcs(int ss_index, ModulCodingType *dl_mcs, ModulCodingType *ul_mcs);
extern int set_ss_dl_mcs(int ss_index, ModulCodingType dl_mcs);
extern int set_ss_ul_mcs(int ss_index, ModulCodingType ul_mcs);

int bs_amc_test();
int ss_amc_test();

//CINR Transitions
#if 1
#define DL_QPSK_12_ENTER -100
#define DL_QPSK_34_ENTER 100
#define DL_QAM16_12_ENTER 13
#define DL_QAM16_34_ENTER 100
#define DL_QAM64_12_ENTER 21
#define DL_QAM64_23_ENTER 24
#define DL_QAM64_34_ENTER 27

#define DL_QPSK_12_LEAVE 100
#define DL_QPSK_34_LEAVE -100
#define DL_QAM16_12_LEAVE 10
#define DL_QAM16_34_LEAVE -100
#define DL_QAM64_12_LEAVE 17
#define DL_QAM64_23_LEAVE 21
#define DL_QAM64_34_LEAVE 24
#endif

#if 0
#define DL_QPSK_12_ENTER -100
#define DL_QPSK_34_ENTER 100
#define DL_QAM16_12_ENTER 100
#define DL_QAM16_34_ENTER 100
#define DL_QAM64_12_ENTER 100
#define DL_QAM64_23_ENTER 100
#define DL_QAM64_34_ENTER 100

#define DL_QPSK_12_LEAVE 100
#define DL_QPSK_34_LEAVE -100
#define DL_QAM16_12_LEAVE -100
#define DL_QAM16_34_LEAVE -100
#define DL_QAM64_12_LEAVE -100
#define DL_QAM64_23_LEAVE -100
#define DL_QAM64_34_LEAVE -100
#endif

#if 1
#define UL_QPSK_12_ENTER -100
#define UL_QPSK_34_ENTER 100
#define UL_QAM16_12_ENTER 15
#define UL_QAM16_34_ENTER 100
#define UL_QAM64_12_ENTER 21
#define UL_QAM64_23_ENTER 24
#define UL_QAM64_34_ENTER 27

#define UL_QPSK_12_LEAVE 100
#define UL_QPSK_34_LEAVE -100
#define UL_QAM16_12_LEAVE 12
#define UL_QAM16_34_LEAVE -100
#define UL_QAM64_12_LEAVE 18
#define UL_QAM64_23_LEAVE 21
#define UL_QAM64_34_LEAVE 24
#endif

#if 0
#define UL_QPSK_12_ENTER -100
#define UL_QPSK_34_ENTER 100
#define UL_QAM16_12_ENTER 100
#define UL_QAM16_34_ENTER 100
#define UL_QAM64_12_ENTER 100
#define UL_QAM64_23_ENTER 100
#define UL_QAM64_34_ENTER 100

#define UL_QPSK_12_LEAVE 100
#define UL_QPSK_34_LEAVE -100
#define UL_QAM16_12_LEAVE -100
#define UL_QAM16_34_LEAVE -100
#define UL_QAM64_12_LEAVE -100
#define UL_QAM64_23_LEAVE -100
#define UL_QAM64_34_LEAVE -100
#endif

//CRC Adjust values
#define CRC_GREATER_THAN_90_PRC -3
#define CRC_GREATER_THAN_10_PRC -1
#define CRC_LESS_THAN_1_PRC 0
#define CRC_LESS_THAN_POINT_1_PRC 1

#define TOTAL_CRC_ADJUST_UPPER_LIMIT 3
#define TOTAL_CRC_ADJUST_LOWER_LIMIT -6


//#define SIMULATE_CRC_ERROR
#endif
