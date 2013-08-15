/* ----------------------------------------------------------------------------
   IBM Confidential

   IBM Wireless M2M Platform   

   (C)Copyright IBM Corp. 2011

   All Rights Reserved.

   File Name: mac_acm_coding.c

   Change Activity:

   Date             Description of Change               By
   ----------------------------------------------------------------------------
   13-Aug.2012      Created                         Xianwei. Yi

   ----------------------------------------------------------------------------
   PROLOG END TAG zYx                                                         */
  
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
 
#include "mac_acm_coding.h"

#ifndef IN
#define IN
#endif

#ifndef OUT
#define OUT
#endif

#ifndef INOUT
#define INOUT
#endif

/* write a 32-bit value into buffer according to big endian */
#define WRITE_BE_32(ptr, value)                                             \
do                                                                          \
{                                                                           \
    ((u_int8_t *)(ptr))[3] = (u_int8_t)((value));                         \
    ((u_int8_t *)(ptr))[2] = (u_int8_t)(((value) & 0x0000FF00U) >> 8U);       \
    ((u_int8_t *)(ptr))[1] = (u_int8_t)(((value) & 0x00FF0000U) >> 16U);  \
    ((u_int8_t *)(ptr))[0] = (u_int8_t)(((value) & 0xFF000000U) >> 24U);  \
} while (0);

/* read a 32-bit value from buffer according to big endian */
#define READ_BE_32(ptr, value)                              \
do                                                          \
{                                                           \
    (value) = ((u_int32_t)(((u_int8_t *)(ptr))[3])) |     \
    ((u_int32_t)(((u_int8_t *)(ptr))[2]) << 8U) |         \
    ((u_int32_t)(((u_int8_t *)(ptr))[1]) << 16U) |        \
    ((u_int32_t)(((u_int8_t *)(ptr))[0]) << 24U );        \
} while (0);

/* write a 16-bit value into buffer according to big endian */
#define WRITE_BE_16(ptr, value)                                       \
do                                                                    \
{                                                                     \
    ((u_int8_t *)(ptr))[1] = (u_int8_t)(((value) & 0x00FFU) >> 0U);  \
    ((u_int8_t *)(ptr))[0] = (u_int8_t)(((value) & 0xFF00U) >> 8U);  \
} while (0);

/* read a 16-bit value from buffer according to big endian */
#define READ_BE_16(ptr, value)                                \
do                                                            \
{                                                             \
    (value) = ((u_int16_t)(((u_int8_t *)(ptr))[1]) << 0U) | \
    ((u_int16_t)(((u_int8_t *)(ptr))[0]) << 8U );           \
} while (0);

/*
 * destroy_tlvs - to destroy the memory allocated to the tlv list
 * @head: tlv list head
 *
 * The API is used to destroy the memory allocated to the tlv list
 */
static void destroy_tlvs
(
    IN struct tlv_info *head
)
{
    struct tlv_info *next;
    
    while (head != NULL)
    {
        next = head->next;
        if (head->value != NULL)
        {
            free(head->value);
        }
        free(head);
        head = next;
    }
}

/*
 * init_rep_req - convert channel measurement report request into tlvs
 * @rep_req: channel measurement report request
 * @msg: tlv list to be returned
 *
 * The API is used to convert channel measurement report request into tlvs
 *
 * Return:
 *      0 if successful
 *      -1 if error happened
 */
static int init_rep_req
(
    IN acm_rep_req_t    *rep_req, 
    OUT acm_msg_t       *msg
)
{
    struct tlv_info *curr_tlv;
    u_int32_t       tlv_length;
    
    if ((rep_req == NULL) || (msg == NULL))
    {
        return -1;
    }
    msg->type = REP_REQ_TYPE;
    tlv_length = 0;

    /* report type */
    msg->tlv_head = (struct tlv_info *)malloc(sizeof(struct tlv_info));
    if (msg->tlv_head != NULL)
    {
        curr_tlv = msg->tlv_head;
        curr_tlv->type = REPORT_TYPE;
        curr_tlv->length = sizeof(u_int8_t);
        curr_tlv->next = NULL;
        curr_tlv->value = (void *)malloc(sizeof(u_int8_t));
        if (curr_tlv->value != NULL)
        {
            *((u_int8_t *)(curr_tlv->value)) = rep_req->report_type;
        }
        else
        {
            return -1;
        }
        tlv_length += sizeof(u_int8_t) + sizeof(u_int32_t) + curr_tlv->length;
    }
    else
    {
        destroy_tlvs(msg->tlv_head);
        return -1;
    }

    /* channel number */
    curr_tlv->next = (struct tlv_info *)malloc(sizeof(struct tlv_info));
    if (curr_tlv->next != NULL)
    {
        curr_tlv = curr_tlv->next;
        curr_tlv->type = CHANNEL_NUM;
        curr_tlv->length = sizeof(u_int8_t);
        curr_tlv->value = (void *)malloc(sizeof(u_int8_t));
        if (curr_tlv->value != NULL)
        {
            *((u_int8_t *)(curr_tlv->value)) = rep_req->channel_num;
        }
        else
        {
            destroy_tlvs(msg->tlv_head);
            return -1;
        }
        tlv_length += sizeof(u_int8_t) + sizeof(u_int32_t) + curr_tlv->length;
    }
    else
    {
        destroy_tlvs(msg->tlv_head);
        return -1;
    }

    /* channel type request */
    curr_tlv->next = (struct tlv_info *)malloc(sizeof(struct tlv_info));
    if (curr_tlv->next != NULL)
    {
        curr_tlv = curr_tlv->next;
        curr_tlv->type = CHANNEL_TYPE_REQ;
        curr_tlv->length = sizeof(u_int8_t);
        curr_tlv->next = NULL;
        curr_tlv->value = (void *)malloc(sizeof(u_int8_t));
        if (curr_tlv->value != NULL)
        {
            *((u_int8_t *)(curr_tlv->value)) = rep_req->channel_type_req;
        }
        else
        {
            destroy_tlvs(msg->tlv_head);
            return -1;
        }
        tlv_length += sizeof(u_int8_t) + sizeof(u_int32_t) + curr_tlv->length;
    }
    else
    {
        destroy_tlvs(msg->tlv_head);
        return -1;
    }

    /* zone specific physical cinr request */
    curr_tlv->next = (struct tlv_info *)malloc(sizeof(struct tlv_info));
    if (curr_tlv->next != NULL)
    {
        curr_tlv = curr_tlv->next;
        curr_tlv->type = ZONE_PHYS_CINR_REQ;
        curr_tlv->length = 3 * sizeof(u_int8_t);
        curr_tlv->next = NULL;
        curr_tlv->value = (void *)malloc(3 * sizeof(u_int8_t));
        if (curr_tlv->value != NULL)
        {
            memcpy(curr_tlv->value, rep_req->zone_phys_cinr_req, sizeof(rep_req->zone_phys_cinr_req));
        }
        else
        {
            destroy_tlvs(msg->tlv_head);
            return -1;
        }
        tlv_length += sizeof(u_int8_t) + sizeof(u_int32_t) + curr_tlv->length;
    }
    else
    {
        destroy_tlvs(msg->tlv_head);
        return -1;
    }

    /* preamble specific physical cinr request */
    curr_tlv->next = (struct tlv_info *)malloc(sizeof(struct tlv_info));
    if (curr_tlv->next != NULL)
    {
        curr_tlv = curr_tlv->next;
        curr_tlv->type = PREAMBLE_PHYS_CINR_REQ;
        curr_tlv->length = sizeof(u_int8_t);
        curr_tlv->next = NULL;
        curr_tlv->value = (void *)malloc(sizeof(u_int8_t));
        if (curr_tlv->value != NULL)
        {
            *((u_int8_t *)(curr_tlv->value)) = rep_req->preamble_phys_cinr_req;
        }
        else
        {
            destroy_tlvs(msg->tlv_head);
            return -1;
        }
        tlv_length += sizeof(u_int8_t) + sizeof(u_int32_t) + curr_tlv->length;
    }
    else
    {
        destroy_tlvs(msg->tlv_head);
        return -1;
    }

    /* zone effective physical cinr request */
    curr_tlv->next = (struct tlv_info *)malloc(sizeof(struct tlv_info));
    if (curr_tlv->next != NULL)
    {
        curr_tlv = curr_tlv->next;
        curr_tlv->type = ZONE_EFFECTIVE_CINR_REQ;
        curr_tlv->length = 2 * sizeof(u_int8_t);
        curr_tlv->next = NULL;
        curr_tlv->value = (void *)malloc(2 * sizeof(u_int8_t));
        if (curr_tlv->value != NULL)
        {
            memcpy(curr_tlv->value, rep_req->zone_effective_cinr_req, sizeof(rep_req->zone_effective_cinr_req));
        }
        else
        {
            destroy_tlvs(msg->tlv_head);
            return -1;
        }
        tlv_length += sizeof(u_int8_t) + sizeof(u_int32_t) + curr_tlv->length;
    }
    else
    {
        destroy_tlvs(msg->tlv_head);
        return -1;
    }

    /* preamble effective physical cinr request */
    curr_tlv->next = (struct tlv_info *)malloc(sizeof(struct tlv_info));
    if (curr_tlv->next != NULL)
    {
        curr_tlv = curr_tlv->next;
        curr_tlv->type = PREAMBLE_EFFECTIVE_CINR_REQ;
        curr_tlv->length = sizeof(u_int8_t);
        curr_tlv->next = NULL;
        curr_tlv->value = (void *)malloc(sizeof(u_int8_t));
        if (curr_tlv->value != NULL)
        {
            *((u_int8_t *)(curr_tlv->value)) = rep_req->preamble_effective_cinr_req;
        }
        else
        {
            destroy_tlvs(msg->tlv_head);
            return -1;
        }
        tlv_length += sizeof(u_int8_t) + sizeof(u_int32_t) + curr_tlv->length;
    }
    else
    {
        destroy_tlvs(msg->tlv_head);
        return -1;
    }

    /* channel selectivity report */
    curr_tlv->next = (struct tlv_info *)malloc(sizeof(struct tlv_info));
    if (curr_tlv->next != NULL)
    {
        curr_tlv = curr_tlv->next;
        curr_tlv->type = CHANNEL_SELECTIVITY_REPORT;
        curr_tlv->length = sizeof(u_int8_t);
        curr_tlv->next = NULL;
        curr_tlv->value = (void *)malloc(sizeof(u_int8_t));
        if (curr_tlv->value != NULL)
        {
            *((u_int8_t *)(curr_tlv->value)) = rep_req->channel_selectivity_report;
        }
        else
        {
            destroy_tlvs(msg->tlv_head);
            return -1;
        }
        tlv_length += sizeof(u_int8_t) + sizeof(u_int32_t) + curr_tlv->length;
    }
    else
    {
        destroy_tlvs(msg->tlv_head);
        return -1;
    }

    /* midamble physical cinr request */
    curr_tlv->next = (struct tlv_info *)malloc(sizeof(struct tlv_info));
    if (curr_tlv->next != NULL)
    {
        curr_tlv = curr_tlv->next;
        curr_tlv->type = MIDAMBLE_PHYS_CINR_REQ;
        curr_tlv->length = sizeof(u_int8_t);
        curr_tlv->next = NULL;
        curr_tlv->value = (void *)malloc(sizeof(u_int8_t));
        if (curr_tlv->value != NULL)
        {
            *((u_int8_t *)(curr_tlv->value)) = rep_req->midamble_phys_cinr_req;
        }
        else
        {
            destroy_tlvs(msg->tlv_head);
            return -1;
        }
        tlv_length += sizeof(u_int8_t) + sizeof(u_int32_t) + curr_tlv->length;
    }
    else
    {
        destroy_tlvs(msg->tlv_head);
        return -1;
    }
    
    msg->length = sizeof(u_int8_t) + sizeof(u_int32_t) + tlv_length;
    return 0;
}

/*
 * build_acm_packet - compose acm request or response into a buffer
 * @rep_req: channel measurement report request
 * @mngt_type: management type
 * @payload: write buffer
 * @len: composed length to be returned
 *
 * The API is used to compose acm request or response into a buffer, 
 * after the composion, the tlv list of the acm request or response will be destroyed. 
 *
 * Return:
 *      0 if successful
 *      -1 if error happened
 */
static int build_acm_packet
(
    IN acm_msg_t    *msg, 
    IN u_int8_t     mngt_type,
    OUT u_int8_t    *payload, 
    OUT u_int32_t   *len
)
{
    u_int8_t        *ptr;
    struct tlv_info *tlv;
    struct tlv_info *next;
    
    if ((msg == NULL) || (payload == NULL) || (len == NULL))
    {
        return -1;
    }

    ptr = payload;
    *ptr = mngt_type;
    ptr += sizeof(u_int8_t);
    *ptr = msg->type;
    ptr += sizeof(u_int8_t);

    WRITE_BE_32(ptr, msg->length);
    ptr += sizeof(u_int32_t);

    tlv = msg->tlv_head;
    while (tlv != NULL)
    {
        next = tlv->next;
        
        *ptr = tlv->type;
        ptr += sizeof(u_int8_t);
        WRITE_BE_32(ptr, tlv->length);
        ptr += sizeof(u_int32_t);
        memcpy(ptr, tlv->value, tlv->length);
        ptr += tlv->length;

        tlv = next;
    }

    destroy_tlvs(msg->tlv_head);
    msg->tlv_head = NULL;
    
    *len = ptr - payload;
    return 0;
}

/*
 * build_rep_req - compose channel measurement report request into a buffer
 * @rep_req: channel measurement report request
 * @payload: write buffer
 * @len: composed length to be returned
 *
 * The API is used to compose channel measurement report request into a buffer
 *
 * Return:
 *      0 if successful
 *      -1 if error happened
 */
int build_rep_req
(
    IN acm_rep_req_t    *rep_req, 
    OUT u_int8_t        *payload, 
    OUT u_int32_t       *len
)
{
    acm_msg_t   msg;
    int         ret;
    
    if ((rep_req == NULL) || (payload == NULL) || (len == NULL))
    {
        return -1;
    }

    ret = init_rep_req(rep_req, &msg);
    if (ret != 0)
    {
        return -1;
    }

    ret = build_acm_packet(&msg, REP_REQ, payload, len);
    return ret;
}

/*
 * parse_acm_packet - parse the byte stream into acm message packet format
 * @payload: payload buffer
 * @len: payload buffer length
 * @mngt_type: management type
 * @msg: message packet to be returned
 *
 * The API is used to parse the byte stream into acm message packet format, 
 * after the parsing, tlv list may be created in the msg data structure
 *
 * Return:
 *      0 if successful
 *      -1 if error happened
 */
static int parse_acm_packet
(
    IN u_int8_t *payload, 
    IN u_int32_t len,
    IN u_int8_t mngt_type, 
    OUT acm_msg_t *msg
)
{
    u_int8_t        *ptr;
    struct tlv_info *prev_tlv;
    struct tlv_info *curr_tlv;
    
    if ((payload == NULL) || (msg == NULL))
    {
        return -1;
    }

    ptr = payload;
    if (*ptr != mngt_type)
    {
	return -1;
    }
    else
    {
	ptr += sizeof(u_int8_t);
    }
    msg->type = *ptr;
    ptr += sizeof(u_int8_t);
    READ_BE_32(ptr, msg->length);
    ptr += sizeof(u_int32_t);

    if ((ptr - payload) >= len)
    {
        return -1;
    }
    
    msg->tlv_head = NULL;
    prev_tlv = NULL;
    while ((ptr - payload) < len)
    {
        curr_tlv = (struct tlv_info *)malloc(sizeof(struct tlv_info));
        if (curr_tlv != NULL)
        {
            curr_tlv->type = *ptr;
            ptr += sizeof(u_int8_t);
            READ_BE_32(ptr, curr_tlv->length);
            ptr += sizeof(u_int32_t);
            curr_tlv->next = NULL;
            curr_tlv->value = (void *)malloc(sizeof(curr_tlv->length));
            if (curr_tlv->value != NULL)
            {
                memcpy(curr_tlv->value, ptr, curr_tlv->length);
                ptr += curr_tlv->length;

                if (prev_tlv == NULL)
                {
                    msg->tlv_head = curr_tlv;
                }
                else
                {
                    prev_tlv->next = curr_tlv;
                }
                prev_tlv = curr_tlv;
            }
            else
            {
                free(curr_tlv);
                destroy_tlvs(msg->tlv_head);
                return -1;
            }
        }
        else
        {
            destroy_tlvs(msg->tlv_head);
            return -1;
        }
    }

    if ((ptr - payload) != len)
    {
        destroy_tlvs(msg->tlv_head);
        return -1;
    }
    else
    {
        return 0;
    }
}

/*
 * parse_rep_req - parse the byte stream into acm channel measurement report request
 * @payload: payload buffer
 * @len: payload buffer length
 * @req: report request to be returned
 *
 * The API is used to parse the byte stream into acm channel measurement report request
 *
 * Return:
 *      0 if successful
 *      -1 if error happened
 */
int parse_rep_req
(
    IN u_int8_t         *payload, 
    IN u_int32_t        len, 
    OUT acm_rep_req_t   *req
)
{
    acm_msg_t       msg;
    int             ret;
    struct tlv_info *curr_tlv;
    
    if ((payload == NULL) || (req == NULL))
    {
        return -1;
    }

    ret = parse_acm_packet(payload, len, REP_REQ, &msg);
    if (ret != 0)
    {
        return -1;
    }

    curr_tlv = msg.tlv_head;
    memset(req, 0, sizeof(acm_rep_req_t));
    while ((curr_tlv != NULL) && (ret == 0))
    {
        switch (curr_tlv->type)
        {
            case REPORT_TYPE:
                if (curr_tlv->length == sizeof(req->report_type))
                {
                    req->report_type = *((u_int8_t *)curr_tlv->value);
                }
                else
                {
                    ret = -1;
                }
                break;
            case CHANNEL_NUM:
                if (curr_tlv->length == sizeof(req->channel_num))
                {
                    req->channel_num = *((u_int8_t *)curr_tlv->value);
                }
                else
                {
                    ret = -1;
                }
                break;
            case CHANNEL_TYPE_REQ:
                if (curr_tlv->length == sizeof(req->channel_type_req))
                {
                    req->channel_type_req = *((u_int8_t *)curr_tlv->value);
                }
                else
                {
                    ret = -1;
                }
                break;
            case ZONE_PHYS_CINR_REQ:
                if (curr_tlv->length == sizeof(req->zone_phys_cinr_req))
                {
                    memcpy(req->zone_phys_cinr_req, curr_tlv->value, curr_tlv->length);
                }
                else
                {
                    ret = -1;
                }
                break;
            case PREAMBLE_PHYS_CINR_REQ:
                if (curr_tlv->length == sizeof(req->preamble_phys_cinr_req))
                {
                    req->preamble_phys_cinr_req = *((u_int8_t *)curr_tlv->value);
                }
                else
                {
                    ret = -1;
                }
                break;
            case ZONE_EFFECTIVE_CINR_REQ:
                if (curr_tlv->length == sizeof(req->zone_effective_cinr_req))
                {
                    memcpy(req->zone_effective_cinr_req, curr_tlv->value, curr_tlv->length);
                }
                else
                {
                    ret = -1;
                }
                break;
            case PREAMBLE_EFFECTIVE_CINR_REQ:
                if (curr_tlv->length == sizeof(req->preamble_effective_cinr_req))
                {
                    req->preamble_effective_cinr_req = *((u_int8_t *)curr_tlv->value);
                }
                else
                {
                    ret = -1;
                }
                break;
            case CHANNEL_SELECTIVITY_REPORT:
                if (curr_tlv->length == sizeof(req->channel_selectivity_report))
                {
                    req->channel_selectivity_report = *((u_int8_t *)curr_tlv->value);
                }
                else
                {
                    ret = -1;
                }
                break;
            case MIDAMBLE_PHYS_CINR_REQ:
                if (curr_tlv->length == sizeof(req->midamble_phys_cinr_req))
                {
                    req->midamble_phys_cinr_req = *((u_int8_t *)curr_tlv->value);
                }
                else
                {
                    ret = -1;
                }
                break;
            default:
                ret = -1;
                break;
        }
        curr_tlv = curr_tlv->next;
    }

    destroy_tlvs(msg.tlv_head);
    return ret;
}

/*
 * parse_rep_rsp - parse the byte stream into acm channel measurement report response
 * @payload: payload buffer
 * @len: payload buffer length
 * @rsp: report response to be returned
 *
 * The API is used to parse the byte stream into acm channel measurement report response
 *
 * Return:
 *      0 if successful
 *      -1 if error happened
 */
int parse_rep_rsp
(
    IN u_int8_t         *payload, 
    IN u_int32_t        len, 
    OUT acm_rep_rsp_t   *rsp
)
{
    acm_msg_t       msg;
    int             ret;
    struct tlv_info *curr_tlv;
    
    if ((payload == NULL) || (rsp == NULL))
    {
        return -1;
    }

    ret = parse_acm_packet(payload, len, REP_RSP, &msg);
    if (ret != 0)
    {
        return -1;
    }

    curr_tlv = msg.tlv_head;
    memset(rsp, 0, sizeof(acm_rep_rsp_t));
    while ((curr_tlv != NULL) && (ret == 0))
    {
        switch (curr_tlv->type)
        {
            case CHAN_NUM:
                if (curr_tlv->length == sizeof(rsp->channel_num))
                {
                    rsp->channel_num = *((u_int8_t *)curr_tlv->value);
                }
                else
                {
                    ret = -1;
                }
                break;
            case START_FRAME:
                if (curr_tlv->length == sizeof(rsp->start_frame))
                {
                    READ_BE_16(curr_tlv->value, rsp->start_frame);
                }
                else
                {
                    ret = -1;
                }
                break;
            case DURATION:
                if (curr_tlv->length == 3)
                {
                    rsp->duration |= ((u_int32_t)(*(((u_int8_t *)curr_tlv->value) + 0))) << 16U;
                    rsp->duration |= ((u_int32_t)(*(((u_int8_t *)curr_tlv->value) + 1))) << 8U;
                    rsp->duration |= ((u_int32_t)(*(((u_int8_t *)curr_tlv->value) + 2))) << 0U;
                }
                else
                {
                    ret = -1;
                }
                break;
            case BASIC_REPORT:
                if (curr_tlv->length == sizeof(rsp->basic_report))
                {
                    rsp->basic_report = *((u_int8_t *)curr_tlv->value);
                }
                else
                {
                    ret = -1;
                }
                break;
            case CINR_REPORT:
                if (curr_tlv->length == 2)
                {
                    rsp->cinr_report = *(((u_int8_t *)curr_tlv->value) + 1);
                }
                else
                {
                    ret = -1;
                }
                break;
            case RSSI_REPORT:
                if (curr_tlv->length == 2)
                {
                    rsp->rssi_report = *(((u_int8_t *)curr_tlv->value) + 1);
                }
                else
                {
                    ret = -1;
                }
                break;
            case CRC_REPORT:
                if (curr_tlv->length == sizeof(rsp->crc_error_num))
                {
                    READ_BE_32(curr_tlv->value, rsp->crc_error_num);
                }
                else
                {
                    ret = -1;
                }
                break;
            case TOTAL_PACKET_NUM:
                if (curr_tlv->length == sizeof(rsp->total_packet_num))
                {
                    READ_BE_32(curr_tlv->value, rsp->total_packet_num);
                }
                else
                {
                    ret = -1;
                }
                break;
            default:
                ret = -1;
                break;
        }
        curr_tlv = curr_tlv->next;
    }
    
    destroy_tlvs(msg.tlv_head);
    return ret;
}

/*
 * init_rep_rsp - convert channel measurement report response into tlvs
 * @rep_req: channel measurement report request
 * @rep_rsp: channel measurement report response
 * @msg: tlv list to be returned
 *
 * The API is used to convert channel measurement report response into tlvs
 *
 * Return:
 *      0 if successful
 *      -1 if error happened
 */
static int init_rep_rsp
(
    IN acm_rep_req_t    *rep_req,
    IN acm_rep_rsp_t    *rep_rsp,
    OUT acm_msg_t       *msg
)
{
    struct tlv_info *curr_tlv;
    u_int32_t       tlv_length;
    u_int32_t       duration;
    
    if ((rep_req == NULL) || (rep_rsp == NULL) || (msg == NULL))
    {
        return -1;
    }

    msg->type = REP_RSP_TYPE;
    tlv_length = 0;

    /* channel number */
    msg->tlv_head = (struct tlv_info *)malloc(sizeof(struct tlv_info));
    if (msg->tlv_head != NULL)
    {
        curr_tlv = msg->tlv_head;
        curr_tlv->type = CHAN_NUM;
        curr_tlv->length = sizeof(u_int8_t);
        curr_tlv->next = NULL;
        curr_tlv->value = (void *)malloc(sizeof(u_int8_t));
        if (curr_tlv->value != NULL)
        {
            *((u_int8_t *)(curr_tlv->value)) = rep_rsp->channel_num;
        }
        else
        {
            return -1;
        }

        curr_tlv = msg->tlv_head;
        tlv_length += sizeof(u_int8_t) + sizeof(u_int32_t) + curr_tlv->length;
    }
    else
    {
        destroy_tlvs(msg->tlv_head);
        return -1;
    }

    /* start frame */
    curr_tlv->next = (struct tlv_info *)malloc(sizeof(struct tlv_info));
    if (curr_tlv->next != NULL)
    {
        curr_tlv = curr_tlv->next;
        curr_tlv->type = START_FRAME;
        curr_tlv->length = sizeof(u_int16_t);
        curr_tlv->next = NULL;
        curr_tlv->value = (void *)malloc(curr_tlv->length);
        if (curr_tlv->value != NULL)
        {
            WRITE_BE_16(curr_tlv->value, rep_rsp->start_frame)
        }
        else
        {
            destroy_tlvs(msg->tlv_head);
            return -1;
        }
        tlv_length += sizeof(u_int8_t) + sizeof(u_int32_t) + curr_tlv->length;
    }
    else
    {
        destroy_tlvs(msg->tlv_head);
        return -1;
    }

    /* duration */
    curr_tlv->next = (struct tlv_info *)malloc(sizeof(struct tlv_info));
    if (curr_tlv->next != NULL)
    {
        curr_tlv = curr_tlv->next;
        curr_tlv->type = DURATION;
        curr_tlv->length = 3;
        curr_tlv->next = NULL;
        curr_tlv->value = (void *)malloc(curr_tlv->length);
        if (curr_tlv->value != NULL)
        {
            duration = rep_rsp->duration;
            if (duration > 0x00FFFFFFU)
            {
                duration = 0x00FFFFFFU;
            }
            *(((u_int8_t *)curr_tlv->value) + 0) = (u_int8_t)((duration & 0x00FF0000U) >> 16U);
            *(((u_int8_t *)curr_tlv->value) + 1) = (u_int8_t)((duration & 0x0000FF00U) >> 8U);
            *(((u_int8_t *)curr_tlv->value) + 2) = (u_int8_t)((duration & 0x000000FFU) >> 0U);
        }
        else
        {
            destroy_tlvs(msg->tlv_head);
            return -1;
        }
        tlv_length += sizeof(u_int8_t) + sizeof(u_int32_t) + curr_tlv->length;
    }
    else
    {
        destroy_tlvs(msg->tlv_head);
        return -1;
    }

    /* basic report */
    if (rep_req->report_type & INCLUDE_BASIC_REPORT)
    {
        curr_tlv->next = (struct tlv_info *)malloc(sizeof(struct tlv_info));
        if (curr_tlv->next != NULL)
        {
            curr_tlv = curr_tlv->next;
            curr_tlv->type = BASIC_REPORT;
            curr_tlv->length = sizeof(u_int8_t);
            curr_tlv->next = NULL;
            curr_tlv->value = (void *)malloc(curr_tlv->length);
            if (curr_tlv->value != NULL)
            {
                *((u_int8_t *)(curr_tlv->value)) = rep_rsp->basic_report;
            }
            else
            {
                destroy_tlvs(msg->tlv_head);
                return -1;
            }
        }
        else
        {
            destroy_tlvs(msg->tlv_head);
            return -1;
        } 
        tlv_length += sizeof(u_int8_t) + sizeof(u_int32_t) + curr_tlv->length;  
    }

    /* cinr report */
    if (rep_req->report_type & INCLUDE_CINR_REPORT)
    {
        curr_tlv->next = (struct tlv_info *)malloc(sizeof(struct tlv_info));
        if (curr_tlv->next != NULL)
        {
            curr_tlv = curr_tlv->next;
            curr_tlv->type = CINR_REPORT;
            curr_tlv->length = 2;
            curr_tlv->next = NULL;
            curr_tlv->value = (void *)malloc(curr_tlv->length);
            if (curr_tlv->value != NULL)
            {
                *(((u_int8_t *)curr_tlv->value) + 0) = 0;
                *(((u_int8_t *)curr_tlv->value) + 1) = rep_rsp->cinr_report;
            }
            else
            {
                destroy_tlvs(msg->tlv_head);
                return -1;
            }
        }
        else
        {
            destroy_tlvs(msg->tlv_head);
            return -1;
        }
        tlv_length += sizeof(u_int8_t) + sizeof(u_int32_t) + curr_tlv->length;   
    }

    /* rssi report */
    if (rep_req->report_type & INCLUDE_RSSI_REPORT)
    {
        curr_tlv->next = (struct tlv_info *)malloc(sizeof(struct tlv_info));
        if (curr_tlv->next != NULL)
        {
            curr_tlv = curr_tlv->next;
            curr_tlv->type = RSSI_REPORT;
            curr_tlv->length = 2;
            curr_tlv->next = NULL;
            curr_tlv->value = (void *)malloc(curr_tlv->length);
            if (curr_tlv->value != NULL)
            {
                *(((u_int8_t *)curr_tlv->value) + 0) = 0;
                *(((u_int8_t *)curr_tlv->value) + 1) = rep_rsp->rssi_report;
            }
            else
            {
                destroy_tlvs(msg->tlv_head);
                return -1;
            }
        }
        else
        {
            destroy_tlvs(msg->tlv_head);
            return -1;
        } 
        tlv_length += sizeof(u_int8_t) + sizeof(u_int32_t) + curr_tlv->length;  
    }

    /* crc number report */
    curr_tlv->next = (struct tlv_info *)malloc(sizeof(struct tlv_info));
    if (curr_tlv->next != NULL)
    {
        curr_tlv = curr_tlv->next;
        curr_tlv->type = CRC_REPORT;
        curr_tlv->length = sizeof(u_int32_t);
        curr_tlv->next = NULL;
        curr_tlv->value = (void *)malloc(curr_tlv->length);
        if (curr_tlv->value != NULL)
        {
            WRITE_BE_32(curr_tlv->value, rep_rsp->crc_error_num)
        }
        else
        {
            destroy_tlvs(msg->tlv_head);
            return -1;
        }
        tlv_length += sizeof(u_int8_t) + sizeof(u_int32_t) + curr_tlv->length;
    }
    else
    {
        destroy_tlvs(msg->tlv_head);
        return -1;
    }

    /* total packet number */
    curr_tlv->next = (struct tlv_info *)malloc(sizeof(struct tlv_info));
    if (curr_tlv->next != NULL)
    {
        curr_tlv = curr_tlv->next;
        curr_tlv->type = TOTAL_PACKET_NUM;
        curr_tlv->length = sizeof(u_int32_t);
        curr_tlv->next = NULL;
        curr_tlv->value = (void *)malloc(curr_tlv->length);
        if (curr_tlv->value != NULL)
        {
            WRITE_BE_32(curr_tlv->value, rep_rsp->total_packet_num)
        }
        else
        {
            destroy_tlvs(msg->tlv_head);
            return -1;
        }
        tlv_length += sizeof(u_int8_t) + sizeof(u_int32_t) + curr_tlv->length;
    }
    else
    {
        destroy_tlvs(msg->tlv_head);
        return -1;
    }

    msg->length = tlv_length + sizeof(u_int8_t) + sizeof(u_int32_t);
    return 0;
}

/*
 * build_rep_rsp - compose channel measurement report response into a buffer
 * @rep_req: channel measurement report request
 * @rep_rsp: channel measurement report response
 * @payload: write buffer
 * @len: composed length to be returned
 *
 * The API is used to compose channel measurement report response into a buffer
 *
 * Return:
 *      0 if successful
 *      -1 if error happened
 */
int build_rep_rsp
(
    IN acm_rep_req_t    *rep_req,
    IN acm_rep_rsp_t    *rep_rsp,
    OUT u_int8_t        *payload, 
    OUT u_int32_t       *len
)
{
    acm_msg_t   msg;
    int         ret;
    
    if ((rep_req == NULL) || (rep_rsp == NULL) || (payload == NULL) || (len == NULL))
    {
        return -1;
    }

    ret = init_rep_rsp(rep_req, rep_rsp,&msg);
    if (ret != 0)
    {
        return -1;
    }

    ret = build_acm_packet(&msg, REP_RSP, payload, len);

    return ret;
}

