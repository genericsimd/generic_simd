/* ----------------------------------------------------------------------------
   IBM Confidential

   IBM Wireless M2M Platform   

   (C)Copyright IBM Corp. 2008, 2009, 2010, 2011

   All Rights Reserved.

   File Name: mac_connection.h

   Change Activity:

   Date             Description of Change                       By
   -----------      ---------------------                       --------
   1-Oct.2008       Created                                     Zhen Bo Zhu

   ----------------------------------------------------------------------------
   PROLOG END TAG zYx                                                         */

#ifndef _CID_H_
#define _CID_H_


#include <stdlib.h>
#include "mac_serviceflow.h"
#include "mac_arq.h"
#include "mac.h"
#include "bs_ss_info.h"

#define INIT_RNG_CID 0x0000
#define PADDING_CID 0xfffe
#define FRAGMENTABLE_BROADCAST 0xfffd


typedef enum {
  CONN_INIT_RANGING,
  CONN_AAS_INIT_RANGING,
  CONN_MULTICAST_POLLING,
  CONN_PADDING,
  CONN_BROADCAST,
  CONN_BASIC,
  CONN_PRIMARY,
  CONN_SECONDARY,
  CONN_DATA
} ConnectionType;

typedef struct connectionele connection;
struct connectionele{
    int cid;
    serviceflow* sf;
    arq_state* arq;
    ConnectionType con_type;
    bs_ss_info* owner;
    int is_frag_enabled;
    int is_pack_enabled;
    int is_arq_enabled;
    int is_fixed_macsdu_length;
    int is_broadcast_br_enabled;
    int is_multicast_br_enabled;
    int is_piggyback_br_enabled;
    int is_phs_enabled;
    int is_crc_included;
    int is_paging_generated;
    int is_sn_feedback_enabled;
    int is_harq_enabled;
    int is_encrypt_enabled;
    u_char key[KEYLEN+1];
    int macsdu_size;
    int macpdu_size;
    int arq_block_size;
    int fsn_size; // 3 bit or 11 bit;
    int current_seq_no;
    int modulo;
    connection* next;
};

//Global pointer to list of connections
extern connection* connection_list_head;

// array of connection* indexed by cid
extern connection** conn_array;

extern BOOL is_conn_arq_enabled(int cid);
extern BOOL is_conn_frag_enabled(int cid);
extern BOOL is_conn_pack_enabled(int cid);
extern int get_blk_size(int cid);
extern BOOL is_fragmentation_enabled(int cid);
extern BOOL is_be_cid(int cid);
extern BOOL is_ugs_cid(int cid);
extern int get_connection(int cid, connection** conn);
//support functions to get information about connection element

//for a cid return the connection element
extern connection* find_connection(int cid);

//for a cid returns the service_flow associated with it
extern int get_service_flow(int cid, serviceflow** sflow);

//for a cid returns the scheduling type
extern int get_scheduling_type(int cid, SchedulingType* svc_flow_type);

//for a cid returns its bs_ss_info
extern int get_bs_ss_info(int cid, bs_ss_info** ss_info);

//for a cid returns its basic cid
//returns the basic cid associated with the bs_ss_info for any cid
extern int get_basic_cid(int cid, int* basic_cid);

extern int get_primary_cid(int cid, int* primary_cid);

// For a given CID, returns the SS index in the bs_ss_info list
extern int get_ss_index(int cid, int* ss_index);



int is_fix_sdu_length(int cid, u_int8_t* is_fixed_sdu);

int get_sdu_size(int cid, int* sdu_size);

int get_pdu_size(int cid, int* pdu_size);
int get_mac_pdu_size(int cid);

int get_current_seq_no(int cid, int* seq_no);

int set_current_seq_no(int cid, int seq_no);

int get_modulo(int cid, int* modulo);

int is_mgt_con(int cid, u_int8_t* is_mgmt);

int is_frag_enabled(int cid, u_int8_t* is_frag);

int is_pack_enabled(int cid, u_int8_t* is_pack);

int is_crc_enabled(int cid, u_int8_t* is_crc);

void get_encryption_key (int cid, u_char *key,int key_length);

int is_encrypt_enabled(int cid, u_int8_t* is_encrypt);

int is_arq_enabled(int cid, u_int8_t* is_arq);

int get_rx_win_size(int cid, int* win_size);

int get_arq_block_size(int cid, int* block_size);

int is_arq_order_preserved(int cid, u_int8_t* is_order);

int get_fsn_size(int cid, int* fsn_size);

// for CRL testing usage

int add_connection(int cid, u_int8_t is_arq, connection** con);

int delete_connection(int cid);

int modify_connection();

int get_connection(int cid, connection** conn);

int release_connection_queue(connection* connection_list);

int get_total_rsvd_rate();

int get_rx_win_start(int cid, int* win_start);
int add_conection(connection* conn);
int associate_connection_with_svc_flow(connection* conn, int sfid);
int get_cid_from_sfid(int sfid);
int get_primary_cid(int cid, int* primary_cid);
extern int associate_conn_to_bs(connection* conn, u_int64_t mac_addr);

#endif
