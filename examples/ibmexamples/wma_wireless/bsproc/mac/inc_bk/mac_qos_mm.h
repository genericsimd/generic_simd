/* ----------------------------------------------------------------------------
   IBM Confidential

   IBM Wireless M2M Platform   

   (C)Copyright IBM Corp. 2011

   All Rights Reserved.

   File Name: mac_qos_mm.h

   Change Activity:

   Date             Description of Change                       By
   -----------      ---------------------                       --------
   01-Mar.2011		Created                                     Parul Gupta

   ----------------------------------------------------------------------------
   PROLOG END TAG zYx                                                         */

#ifndef QOS_H_INCLUDED
#define QOS_H_INCLUDED

#include <assert.h>
#include "mac_serviceflow.h"
#include "ul_mgt_msg_queue.h"
#define SS_TID_START 0
#define SS_TID_END 0x7FFF
#define BS_TID_START 0x8000
#define BS_TID_END 0xFFFF

// In bytes, comprised of SF & CS TLVs
#define MAX_DSA_RSP_LEN 1000
#define MAX_DSA_REQ_LEN 1000
#define MAX_DSA_ACK_LEN 1000
#define MAX_DSD_RSP_LEN 100
#define MAX_DSD_REQ_LEN 100
#define DSX_RVD_LEN 4

struct  tlv_info
{
    unsigned char type;
    unsigned char length;
    void *value;
    struct tlv_info *next;
};
struct tlv_sf_mgmt_encoding
{
    unsigned char type; // UL or DL

	// Overall length of the encapsulated TLV fields
    int length; //255 is enough now, declare int is for extension easily
    struct tlv_info *encapTLV;
};
typedef struct  
{
    unsigned char mgm_msg_type;//11
    u_int16_t trans_ID;
    struct tlv_sf_mgmt_encoding *tlv_sf;//variable
}dsa_req_msg;

typedef struct 
{
    unsigned char mgm_msg_type;//30
    u_int16_t trans_ID;
    unsigned char cfm_code;
}dsx_rvd_msg;

typedef struct 
{
    unsigned char mgm_msg_type;//12
    u_int16_t trans_ID;
    unsigned char cfm_code;
    struct tlv_sf_mgmt_encoding *tlv_sf;//variable
}dsa_rsp_msg;

typedef struct 
{
    unsigned char mgm_msg_type;//13
    u_int16_t trans_ID;
    unsigned char cfm_code;
    struct tlv_sf_mgmt_encoding *tlv_sf;//variable
}dsa_ack_msg;


typedef struct 
{
    unsigned char mgm_msg_type;//17
    u_int16_t trans_ID;
    u_int32_t sfid;
    struct tlv_sf_mgmt_encoding *tlv_sf;//variable
}dsd_req_msg;

typedef struct  
{
    unsigned char mgm_msg_type;//18
    u_int16_t trans_ID;
    unsigned char cfm_code;
    u_int32_t sfid;
    struct tlv_sf_mgmt_encoding *tlv_sf;//variable
}dsd_rsp_msg;


typedef enum ptype
{
SF_ADD = 99,
SF_CHANGE,
SF_DELETE,
SF_ABORT_ADD,
SF_CHANGE_REMOTE,
SF_DELETE_LOCAL,
SF_DELETE_REMOTE,
SF_DSA_ACK_LOST,
SF_DSC_REQ_LOST,
SF_DSC_ACK_LOST,
SF_DSD_REQ_LOST,
SF_CHANGED,
SF_DELETED
} primitivetype;

// Confirmation code values: Ref Table 599 (Rev2D5 of wimax Spec)
#define CC_SUCCESS 0
#define CC_REJECT_OTHER 1
#define CC_REJECT_UNREC_CONF 2
#define CC_REJECT_RESOURCE 3
#define CC_REJECT_ADMIN 4
#define CC_REJECT_NOT_OWNER 5
#define CC_REJECT_SF_NOT_FOUND 6
#define CC_REJECT_SF_EXISTS 7
#define CC_REJECT_REQD_PARAM_ABSENT 8
#define CC_REJECT_HEADER_SUPPRESSION 9
#define CC_REJECT_UNKNOWN_TID 10
#define CC_REJECT_AUTH_FAILURE 11
#define CC_REJECT_ADD_ABORTED 12
#define CC_REJECT_EXCEED_DSLIMIT 13
#define CC_REJECT_NOT_AUTH_FOR_SAID 14
#define CC_REJECT_SA_FAIL 15
#define CC_REJECT_UNSUPPORTED_PARAM 16
#define CC_REJECT_UNSUPPORTED_PARAM_VALUE 17

extern int init_dsa_req(struct service_flow *sf, dsa_req_msg* dsa_req_m);//in,out
extern int build_dsa_req (dsa_req_msg* dsa_req, unsigned char* payload_m, int* length);//in,out,out
extern int parse_dsa_req (unsigned char *payload, int mm_len, dsa_req_msg *dsa_req);//in,in,out
extern int init_dsa_rsp(dsa_req_msg* dsa_req, serviceflow* sf, int cfm_code, dsa_rsp_msg* dsa_rsp);//in,in,out
extern int build_dsa_rsp (dsa_rsp_msg* dsa_rsp, unsigned char* payload, int* length);//in,out,out
extern int parse_dsa_rsp (unsigned char *payload, int mm_len, dsa_rsp_msg *dsa_rsp);//in,in,out
extern int init_dsa_ack(dsa_rsp_msg *dsa_rsp, dsa_ack_msg *dsa_ack);//in,out
extern int build_dsa_ack (dsa_ack_msg* dsa_ack, unsigned char* payload, int* length);//in,out,out
extern int parse_dsa_ack (unsigned char *payload, int mm_len, dsa_ack_msg *dsa_ack);//in,in,out
extern int init_dsx_rvd(dsa_req_msg* dsa_req, dsx_rvd_msg *dsx_rvd_m);//in,out
extern int build_dsx_rvd (dsx_rvd_msg* dsx_rvd, unsigned char* payload, int* length);//in,out,out
extern int parse_dsx_rvd (unsigned char *payload, int mm_len, dsx_rvd_msg *dsx_rvd_m);//in,in,out
extern int free_dsa_req(dsa_req_msg* dsa_req);
extern int free_dsa_rsp(dsa_rsp_msg* dsa_rsp);
extern int free_dsa_ack(dsa_ack_msg* dsa_ack);
extern int dsa_req_to_sf(dsa_req_msg* dsa_req, serviceflow *sf);
extern int init_dsd_req(u_int32_t sfid, u_int16_t trans_id, dsd_req_msg* dsd_req);
extern int build_dsd_req (dsd_req_msg* dsd_req, unsigned char* payload, int* length);
extern int parse_dsd_req (unsigned char *payload, int mm_len, dsd_req_msg *dsd_req);
extern int build_dsd_rsp (dsd_rsp_msg* dsd_rsp, unsigned char* payload, int* length);
extern int init_dsd_req(u_int32_t sfid, u_int16_t trans_id, dsd_req_msg* dsd_req);
extern int init_dsd_rsp(dsd_req_msg* dsd_req, dsd_rsp_msg* dsd_rsp);
extern int parse_dsd_rsp (unsigned char *payload, int mm_len, dsd_rsp_msg *dsd_rsp);

#endif // DSA_H_INCLUDED
