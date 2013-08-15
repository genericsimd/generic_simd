/* ----------------------------------------------------------------------------
   IBM Confidential

   IBM Wireless M2M Platform   

   (C)Copyright IBM Corp. 2011

   All Rights Reserved.

   File Name: mac_acm_coding.h

   Change Activity:

   Date             Description of Change               By
   ----------------------------------------------------------------------------
   13-Aug.2012	    Created 	                    Xianwei. Yi

   ----------------------------------------------------------------------------
   PROLOG END TAG zYx                                                         */

#ifndef _MAC_ACM_CODING_H_
#define _MAC_ACM_CODING_H_

#include <sys/types.h>

#include "mac_qos_mm.h"

#define MAX_REP_REQ_SIZE    (62U + 1U)

#define MAX_REP_RSP_SIZE    (64U + 1U)

#define REP_REQ_TYPE        (1U)

#define REP_RSP_TYPE        (1U)

typedef struct acm_msg
{
    u_int8_t            type;       /* REP-REQ or REP-RSP */
    u_int32_t           length;     /* overall length */ 
    struct tlv_info     *tlv_head;  /* tlv fields */
} acm_msg_t;

typedef enum
{
    REPORT_TYPE = 1U,
    CHANNEL_NUM = 2U,
    CHANNEL_TYPE_REQ = 3U,
    ZONE_PHYS_CINR_REQ = 4U,
    PREAMBLE_PHYS_CINR_REQ = 5U,
    ZONE_EFFECTIVE_CINR_REQ = 6U,
    PREAMBLE_EFFECTIVE_CINR_REQ = 7U,
    CHANNEL_SELECTIVITY_REPORT = 8U,
    MIDAMBLE_PHYS_CINR_REQ = 9U
} acm_rep_req_field_type_t;

typedef enum
{
    CHAN_NUM = 1U,
    START_FRAME = 2U,
    DURATION = 3U,
    BASIC_REPORT = 4U,
    CINR_REPORT = 5U,
    RSSI_REPORT = 6U,
    CRC_REPORT = 7U,
    TOTAL_PACKET_NUM = 8U
} acm_rep_rsp_field_type_t;

#define INCLUDE_BASIC_REPORT    (1U << 0U)
#define INCLUDE_CINR_REPORT     (1U << 1U)
#define INCLUDE_RSSI_REPORT     (1U << 2U)
typedef struct acm_rep_req
{
    u_int8_t    report_type;                    /* report type */
    u_int8_t    channel_num;                    /* physical channel number to be reported on */
    u_int8_t    channel_type_req;               /* requested channel type */
    u_int8_t    zone_phys_cinr_req[3];          /* zone physical cinr request */
    u_int8_t    preamble_phys_cinr_req;         /* zone physical cinr request */
    u_int8_t    zone_effective_cinr_req[2];     /* zone effective cinr request */
    u_int8_t    preamble_effective_cinr_req;    /* zone effective cinr request */
    u_int8_t    channel_selectivity_report;     /* include frequency selectivity report */
    u_int8_t    midamble_phys_cinr_req;         /* midamble physical cinr request */
} acm_rep_req_t;

typedef struct acm_req_rsp
{
    u_int8_t    channel_num;                    /* physical channel number to be reported on */
    u_int16_t   start_frame;                    /* 16 LSB of frame number in which measurement for this channel started */
    u_int32_t   duration;                       /* in units of second, measurement duration */
    u_int8_t    basic_report;                   
    int8_t      cinr_report;                    /* cinr report */
    int8_t      rssi_report;                    /* rssi report */
    u_int32_t   crc_error_num;                  /* CRC error number */
    u_int32_t   total_packet_num;               /* total packet number */
} acm_rep_rsp_t;

extern int build_rep_req(acm_rep_req_t *rep_req, u_int8_t *payload, u_int32_t *len);
extern int parse_rep_req(u_int8_t *payload, u_int32_t len, acm_rep_req_t *req);
extern int build_rep_rsp(acm_rep_req_t *rep_req, acm_rep_rsp_t *rep_rsp, u_int8_t *payload, u_int32_t *len);
extern int parse_rep_rsp(u_int8_t *payload, u_int32_t len, acm_rep_rsp_t *rsp);

#endif
   
