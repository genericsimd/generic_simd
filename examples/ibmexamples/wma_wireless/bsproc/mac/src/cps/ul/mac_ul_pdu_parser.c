/* ----------------------------------------------------------------------------
   IBM Confidential

   IBM Wireless M2M Platform   

   (C)Copyright IBM Corp. 2008, 2009, 2010, 2011

   All Rights Reserved.

   File Name: mac_ul_pdu_parser.c

   Change Activity:

   Date             Description of Change                       By
   -----------      ---------------------                       --------
   03-Aug.2008		Created                                     Chen Lin

   ----------------------------------------------------------------------------
   PROLOG END TAG zYx                                                         */



#include "mac_ul_pdu_parser.h"
#include "mac_connection.h"

#include "mac_amc.h"
#include "metric_proc.h"
#include "monitor_proc.h"

#include "dump_util.h"
#include "mac.h"

int parse_frame_pdu(physical_subframe* phy_subframe, pdu_frame_queue* ul_pduq,  ul_br_queue* brqlist, mgt_msg_queue *ul_msgq){
    unsigned int frame_num = phy_subframe->frame_num;

    phy_burst* cur_burst = phy_subframe->burst_header;

    while (cur_burst!= NULL)
    {
#ifndef AMC_ENABLE
        parse_burst_pdu(frame_num, cur_burst->burst_payload, cur_burst->cid, cur_burst->length, ul_pduq, brqlist, ul_msgq);
#else
	int ret = parse_burst_pdu(frame_num, cur_burst->burst_payload, cur_burst->cid, cur_burst->length, ul_pduq, brqlist, ul_msgq);
//	if (ret == 0)
	{
		FLOG_DEBUG("parse cid[%d]'s burst ok\n", cur_burst->cid);
		update_ss_ul_link_quality(cur_burst->cid, cur_burst->cinr, 0);
	}
#endif
        cur_burst = cur_burst->next;
    }
    return 0;

}

#ifdef AMC_ENABLE
int packet_counter(int my_cid)
{
	if (my_cid <PRIMARY_CID_MIN_VALUE || my_cid > BE_CID_MAX_VALUE) return 0;
	int cid;
	get_basic_cid(my_cid, &cid);
#ifdef SS_TX
	int index = 0;
#else
	int index = cid - BASIC_CID_MIN_VALUE;
#endif
	    	packet_count[index] += 1; 	
	FLOG_DEBUG("Received Packet count for CID %d is %d\n",cid, packet_count[index]);
	    
/*
		if (packet_count[index] < PACKET_UPDATE_MODULUS)
	    	{
			update_average_crc_table_rx(cid, crc_error_count_table[index][ASIZE-1],packet_count[index]);
			return 0 ;
		}	
*/
	    	if (packet_count[index] % PACKET_UPDATE_MODULUS == 0)
		{
		    //Update CRC Main Table here
		    update_average_crc_table_rx(cid, crc_error_count_table[index][ASIZE-1], packet_count[index]);
                    crc_error_count_table[index][ASIZE-1] = 0;
                    packet_count[index] = 0;
		}		
		return 0;
 
}
int error_counter(int my_cid)
{
	if (my_cid <PRIMARY_CID_MIN_VALUE || my_cid > BE_CID_MAX_VALUE) return 0;
	int cid;
	get_basic_cid(my_cid, &cid);
#ifdef SS_TX
	int index = 0;
#else
	int index = cid - BASIC_CID_MIN_VALUE;
#endif
	pthread_mutex_lock(&crc_table_lock);
	    crc_error_count_table[index][ASIZE-1] += 1;
	FLOG_DEBUG("CRC Error count for CID %d is %d\n",cid, crc_error_count_table[index][ASIZE-1]);
	pthread_mutex_unlock(&crc_table_lock);
	   packet_counter(my_cid);
	return 0;
}
#endif

//// parse pdu from the burst, when encounter mac signalling header, directly enqueue them into ul-management msg queue
// return error code 1, hcs error, dicard the burst
int parse_burst_pdu(unsigned int frame_num, u_char* burst, unsigned short bur_cid, int length, pdu_frame_queue* ul_pduframeq,  ul_br_queue* brq, mgt_msg_queue *ul_msgq)
{
    u_char* current = burst;
    u_int8_t is_crc;
    int burst_len = length;
    u_int16_t cid;
    u_int16_t paddingcid;
    u_int8_t ht;
    u_int8_t type;
    int pdu_len;
    int msg_type;
    u_int16_t curd;
    logical_element* le;
    u_int16_t pre_cid;
    cid = 0;
    pre_cid = -1;
    int hdr_len;
    int crc_len;
    int parsed_pdu_count = 0;

    unsigned int hook_count = 0;

    pdu_cid_queue* pducidq;
    hdr_len = GENERIC_MAC_HEADER_LEN;
    crc_len = MAC_CRC_LEN;
    paddingcid = PADDING_CID;
    //get_ul_mgt_msg_queue(&ul_msgq);
    while (burst_len >= hdr_len)
    {
        // first check the header check sequence

        if (hcs_verification(current, hdr_len-1, (current+hdr_len-1)))
        {
            // header check incorrect
            FLOG_DEBUG("HCS verification error: count %d\n", ++hcs_error_count);
	    if (parsed_pdu_count > 0)
	    {
		return 0;
	    }
	    else
	    {
            	return 1;
	    }
        }

	FLOG_DEBUG("HCS verification passed");
        // first check header type
        ht = ((u_int8_t)(*current)) >>7;
        curd = current[3];
        cid = (curd <<8) + current[4];

#ifdef INTEGRATION_TEST
        if ( (cid > MAX_CIDS) && (cid != paddingcid) )
        {
            FLOG_DEBUG("Wrong CID %d\n", cid);
            if (parsed_pdu_count > 0)
            {
                return 0;
            }
            else
            {
                return 1;
            }
        }

        if (cid != paddingcid)
        {
            if (find_connection(cid) == NULL )
            {
                FLOG_DEBUG("Wrong CID %d\n", cid);
		if (parsed_pdu_count > 0)
            	{
                	return 0;
            	}
            	else
            	{
                	return 1;
            	}
            }
        }
#endif

        if (ht)
        {

#ifdef INTEGRATION_TEST
            FLOG_DEBUG("management CID %d\n", cid);
#endif
            // it is a MAC signaling header without payload and CRC
            // we now do not care about the encryption
            type = ((((u_int8_t)(*current)))>>3)&0x07;
           
            switch (type)
            {
                case 0:
                    msg_type = BR_INCREMENTAL_HEADER;
                    enqueue_br_queue(brq, frame_num, cid, msg_type, hdr_len, current);
                    break;
                case 1:
                    msg_type = BR_AGGREGATE_HEADER;
                    enqueue_br_queue(brq, frame_num, cid, msg_type, hdr_len, current);
                    break;
                case 2:
                    msg_type = PHY_CHANNEL_REPORT_HEADER;
                    // enqueue into the channel measurement queue
                    break;
                case 3:
                    msg_type = BR_UL_TX_POWER_REPORT_HEADER;
                    enqueue_br_queue(brq, frame_num, cid, msg_type, hdr_len, current);
                    // enqueue the mac signalling header into the power control related queue
                    break;
                case 4:
                    msg_type = BR_CINR_REPORT_HEADER;
                    enqueue_br_queue(brq, frame_num, cid, msg_type, hdr_len, current);
                    //. enqueue into the channel measurement queue
                    //enqueue_cqi_mgt_msg( ul_cqi_msg_queue, frame_num,cid, msg_type, GENERIC_MAC_HEADER_LEN, current);
                    break;
                case 5:
                    msg_type = BR_UL_SLEEP_CONTROL_HEADER;
                    enqueue_br_queue(brq, frame_num, cid, msg_type, hdr_len, current);
                    // enqueue into the power control related queue
                    break;
                case 6:
                    msg_type = SN_REPORT_HEADER;
                    // enqueue into the channel measurement queue
                case 7:
                    msg_type = CQICH_ALLOCATION_REQUEST_HEADER;
                    // enqueue the mac signalling header into the uplink management message queue
                    enqueue_ul_mgt_msg(ul_msgq, frame_num,cid, msg_type, hdr_len, current);
                    break;
                default:
                    FLOG_ERROR("uplink pdu parsing: error mac signalling header type!");
                    return 1;

            }

            current += GENERIC_MAC_HEADER_LEN;
            burst_len -=GENERIC_MAC_HEADER_LEN;

	    parsed_pdu_count++;
        }
        else 
        {

#ifdef INTEGRATION_TEST
            FLOG_DEBUG("Transport CID = %d, paddingcid = %d\n", cid, paddingcid);
#endif
            if (cid != paddingcid)
            {
                if (cid != pre_cid)
                {
                    get_pducidq(ul_pduframeq, cid, &(pducidq));
                }
            }

            // it is a generic mac header

            // check whether the crc is enabled or not
            is_crc = (current[1] & 0x40) >> 6;

            // check the pdu length
            curd = current[1];
            pdu_len = ((curd & 0x07 ) <<8) +current[2];
			
            if(pdu_len == 0)
            {
                FLOG_DEBUG("In parse_burst_pdu: pdu_len=0. Discarding remaining burst\n");
                break;
            }
#ifdef INTEGRATION_TEST
            if (pdu_len > burst_len)
            {
                FLOG_DEBUG("Burst len parsing wrong CID %d; len %d\n", cid, pdu_len);
                break;
            }
#endif
            // now check the crc, whether it is right or not
            if (is_crc)
            {
		FLOG_DEBUG("\nCRC is enabled");
                if (crc_verification(current, pdu_len-crc_len, current+(pdu_len-crc_len)))
                {
                    // the crc verification is wrong, so skip this pdu
                    current += pdu_len;
                    burst_len -=pdu_len;
#ifdef AMC_ENABLE
		    error_counter(cid);
#endif
                    FLOG_INFO("CRC error!\n");

                    hook_count = hook_debug_trace(HOOK_CRC_COUNT_IDX, NULL, 0, 0) + 1;
                    hook_debug_trace(HOOK_CRC_COUNT_IDX, &hook_count, sizeof(hook_count), 1);
//                    hook_count = hook_debug_trace(HOOK_CRC_COUNT_IDX, NULL, -1);
                    DO_DUMP(DUMP_RX_SELECT_POOL_ID, frame_num, NULL, 0);

                    continue;
                }
            }
	    FLOG_DEBUG("CRC verification passed");
	    pdu_total_count++;
            // the crc verification is right, so send this pdu to the pdu queue related with this connection if it is not the padding pdu
#ifdef AMC_ENABLE
#ifndef SIMULATE_CRC_ERROR
		packet_counter(cid);
#else
		int a = rand();
		if (a %10 ==0)
		{
			error_counter(cid);
		}
		else packet_counter(cid);
#endif
#endif
            if (cid != paddingcid)
            {
                le = (logical_element*) malloc(sizeof(logical_element));
                le->data = current;
                le->length = pdu_len;
                le->type = MAC_PDU;
                enqueue_pducidq(pducidq, le);
            }
            current +=pdu_len;
            burst_len -=pdu_len;
            pre_cid = cid;

	    parsed_pdu_count++;
        }
        
    }
    return 0;

}


