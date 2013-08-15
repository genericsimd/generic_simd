/* ----------------------------------------------------------------------------
 * IBM Confidential

 IBM Wireless M2M Platform

 (C)Copyright IBM Corp. 2009, 2010, 2011

 All Rights Reserved.

 File Name: adapter_ul_stube_test.c

 Change Activity:

 Date             Description of Change                            By
 -----------      ---------------------                            --------
 30-Apr 2011       Created                                         jian jun chang 

 ----------------------------------------------------------------------------
 PROLOG END TAG zYx                                                         */
#include "adapter_ul_stube_test.h"
#include "mac_shift_bits.h"

#include "ul_scheduler.h"

//the ul first test case allocate 23*2 slot  and modulation is 16_QAM_1_2
#define TEST_CASE_ONE_DURATION          46
#define TEST_CASE_ONE_DATA_SIZE         4416  

#define NUM_RANGING_SUBCHANNELS 6

#define UL_STC_MATRIX_TYPE      0
#define  _SINGLE_TEST_CASE_
int adapter_init_ulmap(ul_map_msg *ul_map)
{
  // This function initializes the UL Map message 
  ul_map->manage_msg_type=3;

  // Uplink Channel ID. Not used currently
  ul_map->rsv=0;

  // This count will be generated and maintained by the UCD builder module
  ul_map->ucd_count=0;

  // Unit is number of Physical Slots(PS) since the start of DL frame. 
  // For OFDMA one PS=4/Fs (Sec 10.3.4.2)
  // Min value of allocation start time is 10 OFDMA symbols
  // Ref: Sec 10.3.4.1 of standard for OFDMA
  ul_map->alloc_start_time=0;

  ul_map->ulmap_ie_num=0;
  ul_map->ie=NULL;

  return 0;
}

int adapter_init_ulmap_ie(ul_map_ie *ie)
{
  ie->ie_index = 0;
  ie->cid = 0;
  ie->uiuc = 0;
  ie->uiuc_extend_ie = NULL;
  ie->uiuc_12_ie = NULL;
  ie->uiuc_13_ie = NULL;
  ie->uiuc_14_ie = NULL;
  ie->uiuc_15_ie = NULL;
  ie->uiuc_0_ie = NULL;
  ie->uiuc_other_ie = NULL;
  ie->next = NULL; 
  return 0;
}
int ul_map_builder_1(int* ul_ss_allocation,int* ul_ss_uiuc, int * ul_ss_cid, int broadcast_cid, int num_ul_symbols, int num_ss, ul_map_msg* ul_map)
{
  int ii, ie_index = 0;;
  num_ul_symbols = 0;

  // Mgmt message type of ULMAP is 3
  ul_map->manage_msg_type = 3;

  // This count will be generated and maintained by the UCD builder module
  ul_map->ucd_count = 1;

  // The fixed fields defined above occupy 7 bytes
  // Plus the overhead of packing ULMAP (GMH, CRC) in a MAC PDU addressed to 
  // broadcast CID

  ul_map->ulmap_ie_num=0;
  ul_map_ie *ie=NULL, *last_ulmap_ie=NULL;

  // Add a ULMAP-IE for ranging subchannel
  ie=(ul_map_ie*)malloc(sizeof(ul_map_ie));
  adapter_init_ulmap_ie(ie);

  ie->cid = broadcast_cid;
  ie->ie_index = ie_index;
  ie_index++;
  ie->uiuc = RANGING_UIUC;

  ie->uiuc_12_ie = (uiuc12_ie*)malloc(sizeof(uiuc12_ie));
  ie->uiuc_12_ie->ofdma_symbol_offset = 0;
  ie->uiuc_12_ie->subchannel_offset = 0;
  ie->uiuc_12_ie->ofdma_symbol_num = 3;
  ie->uiuc_12_ie->subchannel_num = NUM_RANGING_SUBCHANNELS;
  ie->uiuc_12_ie->ranging_method = 0;
  ie->uiuc_12_ie->dedicated_ranging_indicator = 0;


  ul_map->ie = ie;
  last_ulmap_ie = ie;
  ul_map->ulmap_ie_num++;
 
  // assume all special UIUCs - 0, 12-15 are processed first then all SS 
  // allocations are processed together

  // For UIUC 1 to 10
  for(ii=0;ii<num_ss;ii++)
  {
	  // If this SS has any bandwidth allocated in the current UL subframe
	  // construct a ULMAP-IE for it
	  ie=(ul_map_ie*)malloc(sizeof(ul_map_ie));
	  adapter_init_ulmap_ie(ie);

	  if(ul_map->ulmap_ie_num==0) {ul_map->ie=ie;}
	  else {last_ulmap_ie->next=ie;}

	  ie->cid = ul_ss_cid[ii];
	  ie->ie_index = ie_index;
	  ie_index++;

	  // The UIUC value will be provided by AMC module. Currently, since
	  // we choose the same uplink burst profile for all CID's, set to 1
	  ie->uiuc=ul_ss_uiuc[ii];
	  ie->uiuc_other_ie = (other_uiuc_ie*)malloc(sizeof(other_uiuc_ie));
	  ie->uiuc_other_ie->duration=ul_ss_allocation[ii];

	  // Currently assume no repetition coding. According to PHY feedback, can 
	  // use repetition coding but UL scheduler proportionately needs to
	  // allocate more resources
	  ie->uiuc_other_ie->repetition_coding_indication=0;

	  // Not used here - relevant only for AAS or AMC. Set to 0
	  ie->uiuc_other_ie->slot_offset = 0;

	  ul_map->ulmap_ie_num++;

	  last_ulmap_ie=ie;
	
    }

  return 0;
}
// This function constructs the UL Map, given the allocation from ul_scheduler 
int ul_map_builder_2(int* ul_ss_allocation,int* ul_ss_uiuc, int * ul_ss_cid, int broadcast_cid, int num_ul_symbols, int num_ss, ul_map_msg* ul_map)
{
  int ii, ie_index = 0, basic_cid = 0;

  // Mgmt message type of ULMAP is 3
  ul_map->manage_msg_type = 3;

  // This count will be generated and maintained by the UCD builder module
  ul_map->ucd_count = 1;

  ul_map->num_ofdma_sym = num_ul_symbols;
  // The fixed fields defined above occupy 7 bytes
  // Plus the overhead of packing ULMAP (GMH, CRC) in a MAC PDU addressed to 
  // broadcast CID

  ul_map->ulmap_ie_num=0;
  ul_map_ie *ie=NULL, *last_ulmap_ie=NULL;

  // Add a ULMAP-IE for ranging subchannel
  ie=(ul_map_ie*)malloc(sizeof(ul_map_ie));
  adapter_init_ulmap_ie(ie);

  ie->cid = broadcast_cid;
  ie->ie_index = ie_index;
  ie_index++;
  ie->uiuc = RANGING_UIUC;

  ie->uiuc_12_ie = (uiuc12_ie*)malloc(sizeof(uiuc12_ie));
  ie->uiuc_12_ie->ofdma_symbol_offset = 0;
  ie->uiuc_12_ie->subchannel_offset = 0;
  ie->uiuc_12_ie->ofdma_symbol_num = num_ul_symbols;
  ie->uiuc_12_ie->subchannel_num = NUM_RANGING_SUBCHANNELS;
  ie->uiuc_12_ie->ranging_method = 0;
  ie->uiuc_12_ie->dedicated_ranging_indicator = 0;

  ul_map->ie = ie;
  last_ulmap_ie = ie;
  ul_map->ulmap_ie_num++;
 
  // assume all special UIUCs - 0, 12-15 are processed first then all SS 
  // allocations are processed together

  // For UIUC 1 to 10
  for(ii=0;ii<num_ss;ii++)
    {
      if(ul_ss_allocation[ii]>0)
	{
	  // If this SS has any bandwidth allocated in the current UL subframe
	  // construct a ULMAP-IE for it
	  ie=(ul_map_ie*)malloc(sizeof(ul_map_ie));
	  adapter_init_ulmap_ie(ie);

	  if(ul_map->ulmap_ie_num==0) {ul_map->ie=ie;}
	  else {last_ulmap_ie->next=ie;}

	  //basic_cid = get_basic_cid_from_ss_index(ii);
	  ie->cid=ul_ss_cid[ii];
	  ie->ie_index = ie_index;
	  ie_index++;
      if (UL_STC_MATRIX_TYPE !=-1)
      {
        // MIMO enabled. Insert MIMO UL Basic IE. UIUC = 11 means extended 2 UIUC
    	ie->uiuc=11;
        ie->uiuc_extend_ie = (extended_uiuc_ie*)malloc(sizeof(extended_uiuc_ie));
        ie->uiuc_extend_ie->extended_uiuc = MIMO_UL_BASIC_IE;
        // 4 bits for num of assign bursts (=1 per SS acc to current algo). CSM not supported yet
        // TODO: Although there is no mention of padding in the spec, since length of Ext2 UIUC IEs are 
        // given in bytes, padding should be needed

        mimo_ul_basic_ie *mubi = (mimo_ul_basic_ie*)malloc(sizeof(mimo_ul_basic_ie));
        ie->uiuc_extend_ie->unspecified_data = mubi;
        
        mubi->num_assign = 0; // In current algorithm only one burst per SS. May change after AMC is implemented

        assigned_burst_attri *burst_attr = (assigned_burst_attri*)malloc(sizeof(assigned_burst_attri));
        mubi->assigned_burst_header = burst_attr;
        burst_attr->next = NULL; 
        //Collaborative SM is not supported yet
        burst_attr->collaborative_sm_indication = 0;

        burst_attr->cid = basic_cid;
	    // The UIUC value will be provided by AMC module. Currently, since
    	// we choose the same uplink burst profile for all CID's, set to 1
        burst_attr->uiuc = ul_ss_uiuc[ii];
        burst_attr->repetition_coding_indication = 0;

        if (UL_STC_MATRIX_TYPE == 0)
        {
            // If Matrix type A
            burst_attr->mimo_control = 0;
        }
        else
        {
            // If Matrix Type B
            burst_attr->mimo_control = 1;
        }
        burst_attr->duration = ul_ss_allocation[ii];

	    // 16(CID) +4(UIUC) + 4 (Ext2 UIUC) + 8(Length) = 32 bits = 4 bytes taken up by the ULMAP-IE and Ext2 IE headers

      }
      else
      {

	  // The UIUC value will be provided by AMC module. Currently, since
	  // we choose the same uplink burst profile for all CID's, set to 1
	  ie->uiuc=ul_ss_allocation[ii];
	  ie->uiuc_other_ie = (other_uiuc_ie*)malloc(sizeof(other_uiuc_ie));
	  ie->uiuc_other_ie->duration=ul_ss_allocation[ii];

	  // Currently assume no repetition coding. According to PHY feedback, can 
	  // use repetition coding but UL scheduler proportionately needs to
	  // allocate more resources
	  ie->uiuc_other_ie->repetition_coding_indication=0;
		
	  // Not used here - relevant only for AAS or AMC. Set to 0
	  ie->uiuc_other_ie->slot_offset = 0;
		
	  // Since data allocations have 16+4+10+2 = 32 bits = 4 bytes, 
	  // padding nibble is not needed for UIUC=1 to 10

      }
	  ul_map->ulmap_ie_num++;

	  last_ulmap_ie=ie;
	}
    }

  return 0;
}

int adapter_build_ul_map(ul_map_msg *ul_map, struct fake_ul_map * p_fake_ulmap)
{
    int * ul_ss_allocation = NULL;
    int * ul_ss_uiuc = NULL;
    int * ul_ss_cid = NULL;

    int num_ul_symbols = 15;
    int num_ss = 0;
    int broadcast_cid = 12911;

    struct fake_ulmap_ie * p_ie;
    int i = 0;

    if (p_fake_ulmap == NULL)
    {
        FLOG_ERROR("Fake ulmap error (pointer is NULL)\n");
        return 1;
    }

    num_ss = p_fake_ulmap->ss_num;
    p_ie = &(p_fake_ulmap->ie);

    if (num_ss != 0)
    {
        ul_ss_allocation = malloc(num_ss * sizeof (int));
        memset(ul_ss_allocation, 0, num_ss * sizeof (int));
        ul_ss_uiuc = malloc(num_ss * sizeof (int));
        memset(ul_ss_uiuc, 0, num_ss * sizeof (int));
        ul_ss_cid = malloc(num_ss * sizeof (int));
        memset(ul_ss_cid, 0, num_ss * sizeof (int));

        while (p_ie != NULL)
        {
            ul_ss_allocation[i] = p_ie->duration;
            ul_ss_uiuc[i] = p_ie->uiuc;
            ul_ss_cid[i] = p_ie->bcid;
            p_ie = p_ie->next;
            i++;
        }
    }

#ifdef _SINGLE_TEST_CASE_
    ul_map_builder_1(ul_ss_allocation,ul_ss_uiuc, ul_ss_cid, broadcast_cid, num_ul_symbols, num_ss, ul_map);
#else
    ul_map_builder_2(ul_ss_allocation,ul_ss_uiuc, ul_ss_cid, broadcast_cid, num_ul_symbols, num_ss, ul_map);
#endif

//    print_ulmap(ul_map);

    if (ul_ss_allocation != NULL)
    {
        free (ul_ss_allocation);
    }

    if (ul_ss_uiuc != NULL)
    {
        free (ul_ss_uiuc);
    }

    if (ul_ss_cid != NULL)
    {
        free (ul_ss_cid);
    }

    return 0;
}


int adapter_build_phy_subframe(physical_subframe* phy_subframe)
{
    u_int8_t *p_buffer = (u_int8_t *) malloc(sizeof(u_int8_t) * 8832);
    u_int8_t *p_buffer1 = (u_int8_t *)malloc(sizeof(u_int8_t) * 4416);
    int iloop = 0;
    int count = 0;
    memset(phy_subframe,0,sizeof(*phy_subframe));

    phy_burst * burst = (phy_burst*)malloc(sizeof(phy_burst));


    u_int8_t * payload = (u_int8_t *)malloc(sizeof(u_int8_t) * TEST_CASE_ONE_DATA_SIZE);
    FILE *fp = NULL;
    if((fp = fopen("0_input_org.dat","r")) != NULL)
    {//p
        printf("open file sucuessful\n");
    }//p
    //int isize = fread(p_buffer,sizeof(u_int8_t),8832,fp);
    for(; iloop < 8832; iloop += 2)
        p_buffer1[count++] = p_buffer[iloop] - '0';
    bits_to_byte(p_buffer1,4416,payload,0,0);
    burst->length = 552;
    burst->burst_payload = payload;
    burst->map_burst_index = 0;
    burst->next = NULL;

    phy_subframe->burst_header = burst;
    return 0;  
}

int adapter_print_ulmap(ul_map_msg *ul_map)
{
    ul_map_ie *ie = ul_map->ie;
    mimo_ul_basic_ie *mubi;
    printf("Starting print_ulmap for UCD CCC: %d, Number of ULMAP IEs: %d\n", ul_map->ucd_count, ul_map->ulmap_ie_num);
    printf("Now traversing the ULMAP-IEs:\n");
    while (ie != NULL)
    {
        printf("IE index: %d, CID: %d, UIUC: %d\n", ie->ie_index, ie->cid, ie->uiuc);

        // These IEs are not supported yet
        if (ie->uiuc_13_ie != NULL || ie->uiuc_14_ie != NULL || ie->uiuc_15_ie != NULL || ie->uiuc_0_ie)
        {
            printf("Error: Unexpected IE\n");
            return -1;
        }
        if(ie->uiuc_12_ie)
		{
	  		printf("OFDMA symbol offset: %d, Subchannel offset: %d, Num OFDMA symbols: %d, Num subchannels: %d, Ranging method: %d, Ranging Indicator: %d\n", \
		 	ie->uiuc_12_ie->ofdma_symbol_offset, ie->uiuc_12_ie->subchannel_offset, \
		 	ie->uiuc_12_ie->ofdma_symbol_num, ie->uiuc_12_ie->subchannel_num, \
		 	ie->uiuc_12_ie->ranging_method, ie->uiuc_12_ie->dedicated_ranging_indicator);
		}

        if (ie->uiuc == 11)
        {
            if(ie->uiuc_other_ie != NULL) 
            {
                printf("Error: Normal IE present with UIUC=11");
                return -1;
            }
            printf("Extended UIUC: %d, Length: %d\n", ie->uiuc_extend_ie->extended_uiuc, ie->uiuc_extend_ie->length);
            if(ie->uiuc_extend_ie->unspecified_data != NULL)
            {
                switch(ie->uiuc_extend_ie->extended_uiuc)
                {
                // TODO: Need to add more case blocks for other types of extd UIUC IDs
                case 9: // MIMO_UL_BASIC_IE
                    mubi = ie->uiuc_extend_ie->unspecified_data;
                    printf("Number of assigned regions: %d, ", mubi->num_assign);
                        assigned_burst_attri *aba = mubi->assigned_burst_header;
                    while(aba != NULL)
                    {
                        printf("CSM: %d, Duration: %d, CID: %d, UIUC: %d\n", aba->collaborative_sm_indication, aba->duration, aba->cid, aba->uiuc);
                        aba = aba->next;
                        
                    }
                    break;
                default:
                    printf("Can't print extended UIUC IE. Unknown type: %d\n", ie->uiuc_extend_ie->extended_uiuc);
                    return -1;
                }
            }
        }
        else
        {
            if(ie->uiuc_extend_ie != NULL) 
            {
                printf("Error: UIUC 11 IE present with UIUC=1 to 10");
                return -1;
            }
            if (ie->uiuc_other_ie != NULL)
            {
                printf("Duration: %d, RCI: %d, Slot offset: %d\n", ie->uiuc_other_ie->duration, ie->uiuc_other_ie->repetition_coding_indication, ie->uiuc_other_ie->slot_offset);
            }
        }
        ie = ie->next;
    }
    return 0;
}
