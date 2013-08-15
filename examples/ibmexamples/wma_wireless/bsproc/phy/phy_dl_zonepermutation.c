/* ----------------------------------------------------------------------------
   IBM Confidential

   IBM Wireless M2M Platform   

   (C)Copyright IBM Corp. 2009, 2010, 2011

   All Rights Reserved.

   File Name: phy_dl_zonepermutation.c

   Function: Zone permutation for DL_PUSC zone

   Change Activity:

   Date             Description of Change                            By
   -----------      ---------------------                            --------
   
   ----------------------------------------------------------------------------
   PROLOG END TAG zYx                                                                 */



#include <stdio.h>
#include <math.h>
#include "phy_dl_zonepermutation.h"
#include "flog.h"

/* turn on/off dump according to DUMP_PHY_DL_ZONE_PERMUTATION setting */
#ifndef DUMP_PHY_DL_ZONE_PERMUTATION

#undef LOCAL_DUMP_ENABLE

#ifdef _DUMP_UTIL_ENABLE_
#define LOCAL_DUMP_ENABLE
#endif

#undef _DUMP_UTIL_ENABLE_

#endif

#include "dump_util.h"

/**----------------------------------------------------------------------------
   Function:    phy_dl_zonepermutation()

   Description: This function is used to do zone permutation for AMC @1024 FFT.

   Parameters:

       Input: struct struct phy_dl_tx_syspara *para;  data structure for system parameters transmission;


       Output: u_int32_t *pilot_allocation      pilot subcar allocation ;
               u_int32_t *data_allocation       data subcar allocation;

   Return Value:
                0       Success
                150     Error

   ----------------------------------------------------------------------------
   LOG END TAG zYx                                                            */

int32_t phy_dl_zonepermutation(struct phy_dl_tx_syspara *para,
                               char *dts_selbit,
                               u_int32_t *pilot_allocation,
                               u_int32_t *data_allocation)
{
    u_int32_t fch_startphysub; //FCH physical start subchan
    int8_t fch_logsub_selbit[21]; //FCH logic bit selection
    int8_t bitsel_index[21]; //bit selection from FCH Bitmap
    int8_t active_band[21]; //active band, 0-active, 1--can not be used
    u_int32_t init_pilot_index[84], pilot_per_slotsym[84*3];
    u_int32_t data_per_slotsym[672*3];// data_index[672*3];
    u_int32_t data_allocation_temp[672*3*3], data_allocation_reshape[672*3*3];//maximal DL symbol number is 9 without Preamble
    u_int32_t subch_phyindex[42], subch_logindex[42], subch_lognon[42];
    u_int32_t i,j,k;
    u_int32_t subband_num, slot_duration;
    u_int32_t used_pilotnum;// used_datanum;
    u_int32_t temp, offset, sig_digital, ten_digital;
    u_int32_t P0_cyc[48];
    u_int32_t num_used_subch = 0;
    u_int32_t num_unused_subch = 0;
	
    u_int32_t P0[48]={1, 22, 46, 52, 42, 41, 26, 50, 5, 33, 62, 43,
                 63, 65, 32, 40, 4, 11, 23, 61, 21, 24, 13, 60,
                 6, 55, 31, 25, 35, 36, 51, 20, 2, 44, 15, 34, 
                 14, 12, 45, 30, 3, 66, 54, 16, 56, 53, 64, 10};
    subband_num = para->num_phyband;



    /* calculate the FCH start Physical Subcahhnel and Logic Subchannel Selection Bit */ 
    fch_startphysub = 14*para->segment_index + 4*(para->id_cell%3);
    if (fch_startphysub == 0)
    {
        for (i=0; i<subband_num; i++)
        {        
	    fch_logsub_selbit[i] = dts_selbit[i];
        }
    }
    else
    {
        for (i=0; i<(subband_num -fch_startphysub/2); i++)
	{
	    fch_logsub_selbit[i] = dts_selbit[fch_startphysub/2 + i];
	}
	for (i=0; i<fch_startphysub/2; i++)
	{
            fch_logsub_selbit[subband_num -fch_startphysub/2+i] = dts_selbit[i];
	}
    }
	
    /* input bitmap information */
    for (i=0; i<6; i++)
    {
        bitsel_index[i] = para->fch_bitmap[0];
	bitsel_index[7+i] = para->fch_bitmap[2];
	bitsel_index[14+i] = para->fch_bitmap[4];
    }
    bitsel_index[6] = para->fch_bitmap[1];
    bitsel_index[13] = para->fch_bitmap[3];
    bitsel_index[20] = para->fch_bitmap[5];
	
   /* getting the real active logic channel */
    for (i=0; i< subband_num; i++)
    {
        active_band[i] = dts_selbit[i] ^ bitsel_index[i]; //1-can use, 0--can not be used
    }
    /* generate the intial pilot index and datasubcar index for one symbol */
    for (i=0; i<para->num_bins; i++)
    {
        init_pilot_index[i] = 9*i + 1; //the interval between two pilot (in two bins) is 9
    }
	
    /* getting the continous logic subchannel with physical subchannel index as content */
    for (i=0; i< para->num_phyband; i++)
    {
        subch_phyindex[2*i] = (2*i+1) * active_band[i];
	subch_phyindex[2*i+1] = 2*(i+1) * active_band[i];
    }
    /* starting from the starting physical channel */
    for (i=0; i<(para->num_subch -fch_startphysub); i++)
    {
	subch_logindex[i] = subch_phyindex[fch_startphysub + i];
    }
    for (i=0; i<fch_startphysub; i++)
    {
        subch_logindex[i+para->num_subch-fch_startphysub] = subch_phyindex[i];
    }
    /*delet the occupied subchannel and get the contiunous logic channel */
    for (i=0; i< para->num_subch; i++)
    {
        if (subch_logindex[i] != 0)
	{
	    subch_lognon[i-num_unused_subch] = subch_logindex[i]-1;
	    num_used_subch ++; //get the num_used_subchannel
	}
	else
	{
	    num_unused_subch ++;//get the num_unused_subchannel
	}
    }
#ifdef DUMP_PHY_DL_ZONE_PERMUTATION
    FLOG_INFO("the used subchannel number = %d\n", num_used_subch);
#endif
#ifdef _ZONE_BOOST_
    float zone_boost;
    zone_boost = sqrt( para->num_subch / num_used_subch);
#endif

    slot_duration = para->zone_len/para->symbol_per_slot;
    temp = num_used_subch*2;
    used_pilotnum = temp*3;
    /*generate the pilot index for one slot symbol */
    for (i=0; i< para->symbol_per_slot; i++)
    {
        for (j=0; j< para->num_bins; j++)
	{	
	    pilot_per_slotsym[para->num_bins*i+j] = init_pilot_index[j] + 3*i;
	}
    } 

 DO_DUMP(DUMP_PHY_DL_ZONEPERM_PILOT_TMP_ID, 0, pilot_per_slotsym, 84*3);
 
 /*select the pilot for used subchannels */
    for (i=0; i< para->symbol_per_slot; i++)
    {	
	for (k=0; k<num_used_subch;k++)
        {
            pilot_allocation[temp*i+2*k]=pilot_per_slotsym[para->num_bins*i+subch_lognon[k]*2]
                                        +(i+1)*para->ofdma_nfft;
            pilot_allocation[temp*i+2*k+1]=pilot_per_slotsym[para->num_bins*i+subch_lognon[k]*2+1]
                                          +(i+1)*para->ofdma_nfft;

	}
    }


	DO_DUMP(DUMP_PHY_DL_ZONEPERM_PILOT_USED_SLOTSYM_ID, 0, pilot_allocation, num_used_subch*3);
    
    /* generate the datasubcar index for one slot symbol */
    /* the 1st symbol */
    for (i=0; i<para->num_bins; i++)
    {
	data_per_slotsym[8*i] = 9*i;
	data_per_slotsym[8*i+1] = 9*i+2;
	data_per_slotsym[8*i+2] = 9*i+3;
	data_per_slotsym[8*i+3] = 9*i+4;
	data_per_slotsym[8*i+4] = 9*i+5;
	data_per_slotsym[8*i+5] = 9*i+6;
	data_per_slotsym[8*i+6] = 9*i+7;
	data_per_slotsym[8*i+7] = 9*i+8;
    }
    /* the 2nd symbol */
    for (i=0; i<para->num_bins; i++)
    {
	data_per_slotsym[para->num_data_subcar+8*i] = 9*i;
	data_per_slotsym[para->num_data_subcar+8*i+1] = 9*i+1;
	data_per_slotsym[para->num_data_subcar+8*i+2] = 9*i+2;
	data_per_slotsym[para->num_data_subcar+8*i+3] = 9*i+3;
	data_per_slotsym[para->num_data_subcar+8*i+4] = 9*i+5;
	data_per_slotsym[para->num_data_subcar+8*i+5] = 9*i+6;
	data_per_slotsym[para->num_data_subcar+8*i+6] = 9*i+7;
	data_per_slotsym[para->num_data_subcar+8*i+7] = 9*i+8;
    }
	
    /* the 3rd symbol */
    for (i=0; i<para->num_bins; i++)
    {
	data_per_slotsym[para->num_data_subcar*2+8*i] = 9*i;
	data_per_slotsym[para->num_data_subcar*2+8*i+1] = 9*i+1;
	data_per_slotsym[para->num_data_subcar*2+8*i+2] = 9*i+2;
	data_per_slotsym[para->num_data_subcar*2+8*i+3] = 9*i+3;
	data_per_slotsym[para->num_data_subcar*2+8*i+4] = 9*i+4;
	data_per_slotsym[para->num_data_subcar*2+8*i+5] = 9*i+5;
	data_per_slotsym[para->num_data_subcar*2+8*i+6] = 9*i+6;		
	data_per_slotsym[para->num_data_subcar*2+8*i+7] = 9*i+8;
    }
    

	DO_DUMP(DUMP_PHY_DL_ZONEPERM_DATA_TMP_ID, 0, data_per_slotsym, 672*3);


    /*select the datasubcar for used subchannels */
    temp = num_used_subch*16;
    for (i=0; i< para->symbol_per_slot; i++)
    {
	for (k=0; k<num_used_subch;k++)
	{    
            for (j=0; j<16; j++)
		{
                    data_allocation_temp[temp*i+16*k+j]=data_per_slotsym[para->num_data_subcar*i+subch_lognon[k]*16+j]
                                          +(i+1)*para->ofdma_nfft;

		}
	}
    }
#if 0 // _DEBUG_PHY_
    memset(test_data_file, 0, sizeof(char)*256);
    sprintf(test_data_file, "%s/%s",data_write_path,"data_persym.dat");

    if( (fpw_r = fopen(test_data_file, "w+t")) == NULL)
    {
        FLOG_ERROR( "E011_test: Can not open file to write!\n");
        return ERROR_CODE;
    }


    for (i=0; i< temp*3; i++)
    {
         fprintf(fpw_r, "%d\n", data_index[i]);
    }
     fclose (fpw_r);

#endif
//Fixme: 

//	DO_DUMP(DUMP_PHY_DL_ZONEPERM_DATA_PERSYM_ID, 0, data_index, temp*3);


    /* reshape the data_subcar as slot unit for each slot symbol */
   temp = num_used_subch * 16;
   for (k=0; k<num_used_subch;k++)
   {
       for (j=0;j<16; j++)
       {
           data_allocation_reshape[k*48+j] = data_allocation_temp[k*16+j];
           data_allocation_reshape[k*48+16+j] = data_allocation_temp[temp+k*16+j];
           data_allocation_reshape[k*48+32+j] = data_allocation_temp[temp*2+k*16+j];
       }
   }

#if 0 //_DEBUG_PHY_
    memset(test_data_file, 0, sizeof(char)*256);
    sprintf(test_data_file, "%s/%s",data_write_path,"data_zone.dat");

    if( (fpw_r = fopen(test_data_file, "w+t")) == NULL)
    {
        FLOG_ERROR( "E011_test: Can not open file to write!\n");
        return ERROR_CODE;
    }

    used_datanum = temp * 3;
    for (i=0; i< used_datanum; i++)
    {
         fprintf(fpw_r, "%d\n", data_allocation_reshape[i]);
    }
     fclose (fpw_r);

#endif
//	DO_DUMP(DUMP_PHY_DL_ZONEPERM_DATA_ZONE_ID, 0, data_allocation_reshape, used_datanum);

#ifdef _AMC_INTERLEAVER_
    if (para->frame_index == 1){
        FLOG_INFO("using AMC INterleaver\n");
    }
    u_int32_t amc_permbase[42] = {38,14,7,6,34,27,36,11,30,16,20,28,22,1,18,15,4,35,
                                  26,13,19,8,32,5,12,21,23,24,25,3,2,39,9,17,40,31,
                                  41,29,0,10,37,33};
    u_int32_t permbase_1st[42];
    u_int32_t dataindex_1st[672], dataindex_others[672];
    u_int32_t data_amc_rotation[672*3*3];
    u_int32_t temp_base[42];
    u_int32_t subchnum_1st,temp_datanum;
    u_int32_t amc_count, nk;

    subchnum_1st = num_used_subch-4; 
    amc_count = 0;
    for (i=0; i<42; i++)
    {
        if (amc_permbase[i] < subchnum_1st)
        {
            permbase_1st[i-amc_count] = amc_permbase[i];
        }
        else
        {
            amc_count++;
        }
    }

    for (j=0; j<subchnum_1st; j++)
    {   
        for (i=0; i<para->datasub_persubsym; i++)
        {
            /* AMC Permbase Rorartion */
            if ((j+i)<subchnum_1st)
            {
                temp_base[i] = permbase_1st[j+i];
            }
            else
            {
                temp_base[i] = permbase_1st[j+i-subchnum_1st];
            }
        }
 
        for (i=0; i<para->datasub_persubsym; i++)
        {
            nk = (i+13*j) % para->datasub_persubsym;
            
            dataindex_1st[j*para->datasub_persubsym+i] =  subchnum_1st * nk + 
                                                         (temp_base[nk%subchnum_1st]+para->dl_permbase)%subchnum_1st;    
                        
        }
    }


	DO_DUMP(DUMP_PHY_DL_ZONEPERM_DATAIDX_1ST_ID, 0, dataindex_1st, subchnum_1st*16);

    /* generate dataindex for other symbols (not the 1st slotsymbol) */
    amc_count = 0;
    for (i=0; i<42; i++)
    {
        if (amc_permbase[i] < num_used_subch)
        {
            permbase_1st[i-amc_count] = amc_permbase[i];
        }
        else
        {
            amc_count++;
        }
    }

    for (j=0; j<num_used_subch; j++)
    {
        for (i=0; i<para->datasub_persubsym; i++)
        {
            /* AMC Permbase Rorartion */
            if ((j+i)<num_used_subch)
            {
                temp_base[i] = permbase_1st[j+i];
            }
            else
            {
                temp_base[i] = permbase_1st[j+i-num_used_subch];
            }
        }

        for (i=0; i<para->datasub_persubsym; i++)
        {
            nk = (i+13*j) % para->datasub_persubsym;

            dataindex_others[j*para->datasub_persubsym+i] =  num_used_subch * nk +
                                                         (temp_base[nk%num_used_subch]+para->dl_permbase)%num_used_subch;

        }
    }
    


	DO_DUMP(DUMP_PHY_DL_ZONEPERM_DATAIDX_OTHER_ID, 0, dataindex_others, num_used_subch*16);

    
    /* rotation according to data_index generated */
    temp_datanum = num_used_subch * para->datasub_persubsym;
    if (para->symbol_offset == 1)
    {
        for(j=0;j<3;j++)
        {
            for(i=0; i<64; i++)
            {
                data_amc_rotation[j*temp_datanum+i] = data_allocation_temp[j*temp_datanum+i];
            }
            for(i=64; i<temp_datanum; i++)
            {
                data_amc_rotation[j*temp_datanum+i] = data_allocation_temp[j*temp_datanum+64+dataindex_1st[i-64]];
            } 

        }

	DO_DUMP(DUMP_PHY_DL_ZONEPERM_DATAROTA_1ST_ID, 0, data_amc_rotation, num_used_subch*48);

    }
    else
    {
        for(j=0;j<3;j++)
        {
            for(i=0; i<temp_datanum; i++)
            {
                data_amc_rotation[j*temp_datanum+i] = data_allocation_temp[j*temp_datanum+dataindex_others[i]];
            }

        }

	DO_DUMP(DUMP_PHY_DL_ZONEPERM_DATAROTA_OTHERS_ID, 0, data_amc_rotation, num_used_subch*48);

    }

   temp = num_used_subch * 16;
   for (k=0; k<num_used_subch;k++)
   {
       for (j=0;j<16; j++)
       {
           data_allocation_reshape[k*48+j] = data_amc_rotation[k*16+j];
           data_allocation_reshape[k*48+16+j] = data_amc_rotation[temp+k*16+j];
           data_allocation_reshape[k*48+32+j] = data_amc_rotation[temp*2+k*16+j];
       }
   }


#endif

	
    temp = para->dl_permbase % 48;
    for (i=0; i<48-temp; i++)
    {
        P0_cyc[i] = P0[i+temp];
    }
    for (i=0; i<temp; i++)
    {
        P0_cyc[48-temp+i] = P0[i];
    }
	
    offset = (int)floor(para->dl_permbase/48) % 49;
    for (i=0; i<48; i++)
    {
        sig_digital = (P0_cyc[i]%10 + offset%10) % 7;
	ten_digital = ((int)floor(P0_cyc[i]/10) + (int)floor(offset/10)) % 7;
	P0_cyc[i] = ten_digital*10 + sig_digital;
	if (P0_cyc[i] == 0)
	{
	    P0_cyc[i] = offset;
	}
	P0_cyc[i] =(int) floor(P0_cyc[i]/10)*7 + P0_cyc[i]%10 -1;
    }
    /* subcarrier rotation in a slot */	
    for (i=0; i<num_used_subch; i++)
    {
        for (j=0; j<48; j++)
        {
	    data_allocation[i*48+j] = data_allocation_reshape[i*48+P0_cyc[j]];

        }
    }


    return SUCCESS_CODE;
}



/* restore _DUMP_UTIL_ENABLE_ setting */
#ifndef DUMP_PHY_DL_ZONE_PERMUTATION

#ifdef LOCAL_DUMP_ENABLE
#define _DUMP_UTIL_ENABLE_
#endif

#undef LOCAL_DUMP_ENABLE
#endif

