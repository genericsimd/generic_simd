/* ----------------------------------------------------------------------------
   IBM Confidential

   IBM Wireless M2M Platform   

   (C)Copyright IBM Corp. 2009, 2010, 2011

   All Rights Reserved.

   File Name: mac_bs_ranging_utm.c

   Change Activity:

   Date             Description of Change                   By
   -----------      ---------------------					--------
   1-Jul.2009       Created                                 Parul Gupta

   ----------------------------------------------------------------------------
   PROLOG END TAG zYx                                                         */


#include <stdio.h>
#include "mac_bs_ranging.h"
#include "mac_bs_ranging_utm.h"
#include "ul_mgt_msg_queue.h"
#include "debug.h"

sll_fifo_q *bs_ranging_q;
sll_fifo_q *ranging_scheduler_q;
extern sdu_queue* dl_sdu_queue;
sll_fifo_q *bs_ranging_test_q;



#ifndef SS_TX
#ifndef SS_RX
static u_int8_t frame_num_index = 14;
static u_int8_t ranging_code = 1;
static u_int8_t ranging_symbol = 2;
static u_int8_t ranging_subchannel = 3;

ranging_info *p_ranging_info; 
int mac_bs_rng_utm()
{
	// Test 1: Enqueue ranging_adjust_ie with a high correction value and check for proper RNG_RSP
	int32_t timing_adjust = 10; 
	int8_t power_adjust = 5;
	int32_t frequency_adjust = 10;

	FLOG_INFO("Starting Test 1\n");
	mac_bs_rng_adjust(timing_adjust, power_adjust, frequency_adjust);

	// The mac_bs_rng_adjust function above enqueues RNG_RSP in the INIT_RNG_CID
	// sdu queue, which should get transmitted in the phy frame and com back via
	// the UL chain in BS_TX-BS_RX loopback test
	int result = test_rng_rsp(timing_adjust, power_adjust, frequency_adjust);
	FLOG_INFO("Test 1 code attributes match result: %d\n", result);

	// Test 2: Enqueue ranging_adjust_ie with zero correction and success status, 
	// check for proper RNG_RSP and CDMA allocation IE in ULMAP
	timing_adjust = 0; 
	power_adjust = 0;
	frequency_adjust = 0;

	mac_bs_rng_adjust(timing_adjust, power_adjust, frequency_adjust);
	// Ideally the ULMAP should be received in the UL first, hence CDMA alloc IE
	// should be enqueued first in the bs_ranging_test_q, then the RNG_RSP msg.
	// But current code works otherwise, so dequeuing & testing for RNG_RSP first
	//result = test_rng_rsp(timing_adjust, power_adjust, frequency_adjust);
	//FLOG_INFO("Test 2 code attributes match result: %d\n", result);
	FLOG_INFO("Started Test 2, Waiting for CDMA alloc IE\n");	
	// Now test for CDMA alloc IE
	q_container *q_cntr = NULL; 
	rng_rsp_msg *rng_rsp = NULL;
	sll_fifo_q_dequeue_with_wait(bs_ranging_test_q, &q_cntr);

    cdma_alloc_ie *cdma_ie = NULL;
	cdma_ranging_ie *cdma_rng_ie = NULL;

	if(q_cntr->data_type == CDMA_ALLOC_UIUC)
	{
		cdma_rng_ie = q_cntr->data;
		cdma_ie = cdma_rng_ie->cdma_ie;

		if ((cdma_ie->ranging_code == ranging_code) &&  \
		(cdma_ie->ranging_subchannel == ranging_subchannel) && \
		(cdma_ie->ranging_symbol == ranging_symbol))
		{
			FLOG_INFO("Test 2: CDMA alloc IE found in ULMAP\n");
		}
		free(cdma_ie);
		free(cdma_rng_ie);
	}
	else
	{
		FLOG_INFO("Test 2: Message dequeued is not a CDMA IE\n");
	}
	free(q_cntr);
	q_cntr = NULL;
	
	//Test 3: Enqueue RNG_REQ on INIT_RNG_CID, w/ ss MAC. check for proper RNG_RSP w/ status success & basic/prim CIDs allocated.
	u_char *payload = NULL, *p_to_enq = NULL;
	int mm_len = 0;

	// Because exact length of rng_req is known only after build, we need to 
	// mac_sdu_malloc p_to_enq with this exact size and use that to enqueue
	// in SDU queue. payload needs to be freed after that.
	build_rng_req(&payload, &mm_len, 100);
	p_to_enq = mac_sdu_malloc(mm_len, 0);
	memcpy(p_to_enq, payload, mm_len);

	FLOG_INFO("length from build_rng_req: %d\n", mm_len);fflush(stdout);
	enqueue_transport_sdu_queue(dl_sdu_queue, INIT_RNG_CID, mm_len, p_to_enq);
	free(payload);

	sll_fifo_q_dequeue_with_wait(bs_ranging_test_q, &q_cntr);
	parse_rng_rsp(q_cntr->data, q_cntr->len, &rng_rsp); 
	connection *conn = find_connection(42);
	if (conn != NULL) {FLOG_INFO("Conn for pcid 42 found\n");}
	FLOG_INFO("Ranging status: %d, MAC addr; %d.%d.%d.%d.%d.%d, Basic CID: %d, Primary CID: %d\n", \
	rng_rsp->ranging_status.value, rng_rsp->ss_mac.value[0],  rng_rsp->ss_mac.value[1], \
	rng_rsp->ss_mac.value[2], rng_rsp->ss_mac.value[3], rng_rsp->ss_mac.value[4], \
	rng_rsp->ss_mac.value[5], rng_rsp->basic_cid.value, rng_rsp->primary_cid.value);
	free(rng_rsp);
	free(q_cntr->data);
	free(q_cntr);
	return 0;
}

int test_rng_rsp(int32_t timing_adjust, int8_t power_adjust, int32_t frequency_adjust)
{
	q_container *q_cntr = NULL; 
	rng_rsp_msg *rng_rsp = NULL;
	int ofdm_sym = 0, ofdm_subchan = 0, rng_code = 0, frame_num = 0; 

	FLOG_DEBUG("In test_rng_rsp: Waiting to dequeue msg from bs_ranging_test_q\n");
	sll_fifo_q_dequeue_with_wait(bs_ranging_test_q, &q_cntr);
	if(q_cntr->data_type != RNG_RSP)
	{
		FLOG_INFO("In test_rng_rsp, message dequeued is not a RNG_RSP\n");
		return FALSE;
	}
	parse_rng_rsp(q_cntr->data, q_cntr->len, &rng_rsp); 
	free(q_cntr->data);
	free(q_cntr);
	//printf("In test_rng_rsp: Ranging status: %d\n", rng_rsp->ranging_status.value);
	if((timing_adjust !=0 ) && (rng_rsp->timing_adjust.value != timing_adjust)) return FALSE;
	if((power_adjust != 0) && (rng_rsp->power_adjust.value != power_adjust)) return FALSE;
	if((frequency_adjust != 0) && (rng_rsp->frequency_adjust.value != frequency_adjust)) return FALSE;

	if (rng_rsp->ranging_code_attributes.length == 4)
	{
		ofdm_sym = (rng_rsp->ranging_code_attributes.value & 0xFFC00000) >> 22; 
		ofdm_subchan = (rng_rsp->ranging_code_attributes.value & 0x3F0000) >> 16;
		rng_code = (rng_rsp->ranging_code_attributes.value & 0xFF00) >> 8;
		frame_num = rng_rsp->ranging_code_attributes.value & 0xFF;
		
		//printf("In test_rng_rsp: %d %d %d %d\n", frame_num, rng_code, ofdm_sym, ofdm_subchan);
		if ((frame_num == frame_num_index) && (rng_code == ranging_code) &&  (ofdm_subchan == ranging_subchannel) && (ofdm_sym == ranging_symbol))
		{
			free(rng_rsp);
			return TRUE;
		}
	}
	else
	{	
		FLOG_INFO("In test_rng_rsp: TLV 150 length is incorrect, can't decode\n"); 
		free(rng_rsp);

		return FALSE;
	}
	free(rng_rsp);
	return FALSE;
}

int mac_bs_rng_adjust(int32_t timing_adjust, int8_t power_adjust, int32_t frequency_adjust)
{
	p_ranging_info = (ranging_info*)malloc(sizeof(ranging_info));
	memset(p_ranging_info, 0, sizeof(ranging_info));
	p_ranging_info->num_ranging_adjust_ie = 1;
	ranging_adjust_ie *p_rng_ie = (ranging_adjust_ie*)malloc(sizeof(ranging_adjust_ie));
	p_ranging_info->p_ranging_ie_list = p_rng_ie;
	
	// Populate the IE with correction values
	p_rng_ie->timing_adjust = timing_adjust;
	p_rng_ie->power_adjust = power_adjust;
	p_rng_ie->frequency_adjust = frequency_adjust;

	p_rng_ie->frame_num_index = frame_num_index;
	p_rng_ie->ranging_code = ranging_code;
	p_rng_ie->ranging_symbol = ranging_symbol;
	p_rng_ie->ranging_subchannel = ranging_subchannel;
	p_rng_ie->p_next = NULL;
	sll_fifo_q_enqueue(bs_ranging_q, p_ranging_info, sizeof(ranging_info), RANGING_ADJUST);
	return 0;
}
#endif
#endif

