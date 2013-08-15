/* ----------------------------------------------------------------------------
   IBM Confidential

   IBM Wireless M2M Platform

   (C)Copyright IBM Corp. 2009, 2010, 2011

   All Rights Reserved.

   File Name: phy_ul_zonepermutation.c

   Function: Zone permutation for UL_PUSC zone

   Change Activity:

   Date             Description of Change                            By
   -----------      ---------------------                            --------
   
   ----------------------------------------------------------------------------
   PROLOG END TAG zYx                                                                 */



#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <memory.h>
#include "phy_ul_zonepermutation.h"
#include "flog.h"

/* turn on/off dump according to DUMP_PHY_UL_ZONE_PERMUTATION setting */
#ifndef DUMP_PHY_UL_ZONE_PERMUTATION

#undef LOCAL_DUMP_ENABLE

#ifdef _DUMP_UTIL_ENABLE_
#define LOCAL_DUMP_ENABLE
#endif

#undef _DUMP_UTIL_ENABLE_

#endif

#include "dump_util.h"

#define _SUBCAR_ROTATION_
/**----------------------------------------------------------------------------
   Function:    phy_ul_zonepermutation()

   Description: This function is used to do zone permutation for DL_PUSC @1024 FFT.
                Supporting

   Parameters:

       Input: struct  phy_ul_rx_syspara *para;  data structure for system parameters transmission;


       Output: float *pilot_allocation [120*2]     pilot subcar allocation ;
               float *data_allocation [720*2]      data subcar allocation;

   Return Value:
                0       Success
                150     Error

   ----------------------------------------------------------------------------
   LOG END TAG zYx   */


int32_t cmp ( const void *a , const void *b )
{
    return *(int *)a - *(int *)b;
}

int32_t phy_ul_zonepermutation(struct phy_ul_rx_syspara *para,
                               char *dts_selbit,
                               u_int32_t *ava_subch,
                               u_int32_t *rotation_posindex,
                               u_int32_t *pilot_allocation,
                               u_int32_t *data_allocation,
			       u_int32_t *ranging_allocation)
{
    u_int32_t active_channel[35]; 
    u_int32_t init_pilot_index[840],pilot_index_zone[31*24*16];
    u_int32_t init_data_index[840*2],data_rotation_index[840*2],data_index_zone[840*2*16];
    u_int32_t ranging_slotindex[6*48],ranging_zone_index[840*48];
    u_int32_t subch_index[35*16], subch_index_con[35*16];
    u_int32_t temp,used_len,temp_index[35];
    u_int32_t rotation_index[35*16];// rotation_posindex[35*16];
    u_int32_t tileseq[35], tilesub_index[756], index_tmp[756],tilephy_index[6][35];
    u_int32_t dataplt_tmpindex[24][35], dataplt_index[24][35], dataphy_index[24*35];
    u_int32_t i,j,k;
    u_int32_t subch_num,rotdata_index;
    u_int32_t used_datanum, pilot_len;
    u_int32_t symoffset, subchoffset,ranging_sym,ranging_subch;
    u_int32_t num_used_subch = 0;
    u_int32_t num_unused_subch = 0;
    u_int32_t count = 0;
    u_int32_t sum;
    u_int32_t usedlen[16];
    u_int32_t ul_unused_subch[1];
				 
    u_int32_t P0[35] = {11,19,12,32,33,9,30,7,4,2,13,8,17,23,27,5,15,34,22,14,21,1,0,24,3,
                        26,29,31,20,25,16,10,6,28,18};
    
    temp = 0;
    used_len = 0;

    /* calculate the available sunchannels for UL link */
    for (i=0; i<para->ofdma_nused-1; i++)
    {
        index_tmp[i] = i+1;
    }
    for (i=0;i<21; i++)
    {
        if (dts_selbit[i] == 1)
	{
	    count++;
            for (j=0; j<36; j++)
            {
                index_tmp[i*36+j] = 0;
            }
	}
    }	
    subch_num = floor((756-36*count)/(para->num_tiles_perslot*para->num_subcar_pertileF));
    ava_subch[0]=subch_num; 
//  FLOG_DEBUG("count = %d, subch_num = %d\n", count, subch_num);
    
     /*calculate the available subcarrier without interference */
    count = 0;
    for(i=0; i<756; i++)
    {
        if(index_tmp[i] != 0)
        {
            tilesub_index[i-count]=index_tmp[i]-1; 
        }
        else
        {
            count++;
        }
    }

	DO_DUMP(DUMP_PHY_UL_ZONEPERM_TILESUM_IDX_ID, 0, tilesub_index, 19*36);

    /* calculate the new Tile Permutation Sequence */
    count = 0;
    for (i=0; i<35; i++)
    {
        if (P0[i] < subch_num)
        {
            tileseq[i-count] = P0[i];
	}
	else
	{
	    count++;
	}			
    }
	
   /* getting the real active logic channel tmap*/
   

    for (i=0; i< subch_num; i++)
    {
        active_channel[i] = para->ul_bitmap[i]; //1-can use, 0--can not be used
        //FLOG_DEBUG("active_channel[%d]= %d\n", i, active_channel[i]);
    }
    

    /* generate the slot index for the whole ul_subframe (28*slotsymbol_index), 0--for unused slot*/

    for (j=0; j< para->zone_len; j++)
    {
        count = 0;
        for (i=0; i<subch_num; i++)
	{
	    subch_index[j*subch_num+i] = (j*subch_num+i+1) * active_channel[i];
	    if ( subch_index[j*subch_num+i] != 0)
            {
	        subch_index_con[j*(subch_num-temp)+i-count] = subch_index[j*subch_num+i];
	    }
	    else
	    {
	        count++;
	    }
	}
        temp = count;
    }

    num_unused_subch = count;
    num_used_subch = subch_num - num_unused_subch;
    para->usedsubch_num = num_used_subch;
    ul_unused_subch[0] = para->num_subch - num_used_subch;
    
   /* consider the ranging channel */
    symoffset = 0;//para->ranging_symoffset - para->symbol_offset;
    subchoffset = para->ranging_subchoffset;
    ranging_sym = para->ranging_sym;
    ranging_subch = para->ranging_subch;
    count = 0;
    for (j=symoffset/para->symbol_per_slot; j<(symoffset+ranging_sym)/para->symbol_per_slot; j++)
    {
        for (i= subchoffset; i<subchoffset+ranging_subch; i++)
	{
	    ranging_slotindex[j*ranging_subch+i] = subch_index_con[j*num_used_subch+i] - 1;
            count++;
	}
    }
	
    for (i=0; i<count; i++)
    {
        subch_index[ranging_slotindex[i]] = 0;
    }
    /* subchannel rotation index */
   
    used_len = 0;
    sum = 0;
    for (j=0; j< para->zone_len; j++)
    {
        count = 0;
        for (i=0; i<subch_num; i++)
	{
	    if ( subch_index[j*subch_num+i] != 0)
	    {
	        subch_index_con[sum+i-count] = subch_index[j*subch_num+i]-1;
                rotation_posindex[sum+i-count] = j*subch_num+i;
	    }
	    else
	    {
	        count++;
	    }
        }
	used_len = subch_num-count;
        usedlen[j] = used_len;
        sum = sum + used_len;
    }
  //  FLOG_DEBUG("sum = %d\n", sum);

	DO_DUMP(DUMP_PHY_UL_ZONEPERM_SUBCH_IDX_CON_ID, 0, subch_index_con, sum);


    sum = 0;
    for (j=0; j<para->zone_len; j++)
    {
        for (i=0; i<usedlen[j]; i++)
        {
            temp = i;
            temp_index[i] = (temp + 13*j) % usedlen[j];
	    rotation_index[sum+temp_index[i]] = subch_index_con[sum+i];
	}
        sum = sum + usedlen[j];
    }


	DO_DUMP(DUMP_PHY_UL_ZONEPERM_ROTA_POSIDX_ID, 0, rotation_posindex, sum);
	DO_DUMP(DUMP_PHY_UL_ZONEPERM_ROTA_IDX_ID, 0, rotation_index, sum);
	
    /* calculate the physical tile index, as equation 8.4.6.2.1(78) */
    for (j=0; j<para->num_tiles_perslot; j++)
    {
        for (i=0; i<subch_num; i++)
	{
	    tilephy_index[j][i] =  para->num_subcar_pertileF * (subch_num*j + (tileseq[(i+j)%subch_num]
                                   +para->ul_permbase)%subch_num);
	/* calculate the pilot, data mapping index */					   
            dataplt_tmpindex[j*para->num_subcar_pertileF][i] = tilephy_index[j][i];
	    dataplt_tmpindex[j*para->num_subcar_pertileF+1][i] = tilephy_index[j][i]+1;
	    dataplt_tmpindex[j*para->num_subcar_pertileF+2][i] = tilephy_index[j][i]+2;
	    dataplt_tmpindex[j*para->num_subcar_pertileF+3][i] = tilephy_index[j][i]+3;
			
	    dataplt_index[j*para->num_subcar_pertileF][i] = tilesub_index[dataplt_tmpindex[j*para->num_subcar_pertileF][i]];
	    dataplt_index[j*para->num_subcar_pertileF+1][i] = tilesub_index[dataplt_tmpindex[j*para->num_subcar_pertileF+1][i]];
	    dataplt_index[j*para->num_subcar_pertileF+2][i] = tilesub_index[dataplt_tmpindex[j*para->num_subcar_pertileF+2][i]];
	    dataplt_index[j*para->num_subcar_pertileF+3][i] = tilesub_index[dataplt_tmpindex[j*para->num_subcar_pertileF+3][i]];
			
	}
    }

    for (i=0; i<subch_num; i++)
    {
        for (j=0; j<para->num_subcar_of_subch;j++)
	{
	    dataphy_index[i*para->num_subcar_of_subch+j] = dataplt_index[j][i];
	}
    }

   DO_DUMP(DUMP_PHY_UL_ZONEPERM_DATAPLT_IDX_ID, 0, dataphy_index, 24*subch_num);
   
   /* pilot index generation for one slotsymbol */
   for (j=0; j<subch_num; j++)
   {
        for (i=0; i< para->num_tiles_perslot; i++)
        {		
            init_pilot_index[j*para->num_pilot_perslot+4*i] = dataphy_index[j*para->num_subcar_of_subch+4*i];
	    init_pilot_index[j*para->num_pilot_perslot+4*i+1] = dataphy_index[j*para->num_subcar_of_subch+4*i+3];
            init_pilot_index[j*para->num_pilot_perslot+4*i+2] = dataphy_index[j*para->num_subcar_of_subch+4*i] 
	                                                        + 2*para->ofdma_nfft;
	    init_pilot_index[j*para->num_pilot_perslot+4*i+3] = dataphy_index[j*para->num_subcar_of_subch+4*i+3] 
	                                                        + 2*para->ofdma_nfft;
	}
   }
   
   /*pilot index generation for the whole UL Zone */
    pilot_len = para->num_subcar_of_subch*subch_num;   //672
    for (i=0; i<para->zone_len; i++)
    {
        for (j=0; j< pilot_len; j++)
	{
	    pilot_index_zone[i*pilot_len+j] = init_pilot_index[j] + i*para->ofdma_nfft*3;
         //   pilot_allocation[i*pilot_len+j] = init_pilot_index[j] + i*para->ofdma_nfft*3;

	}
    }

    para->chan_sum = sum;
    /* processing for subchanel rotation */
    memcpy(pilot_allocation, pilot_index_zone,pilot_len*para->zone_len*sizeof(u_int32_t));

    for (j=0; j<sum; j++)
    {
        for (i=0; i<24; i++)
        {
            pilot_allocation[rotation_index[j]*24+i]=pilot_index_zone[rotation_posindex[j]*24+i];
        }

    }

	
    /* data index generation for one slotsymbol */
    for (j=0; j<subch_num; j++)
    {
    /* data index for the 1st symbol */
	for (i=0; i<para->num_tiles_perslot; i++)
       {       
	    init_data_index[j*para->num_datasubcar_perslot+2*i] = dataphy_index[j*para->num_subcar_of_subch+4*i+1];
	    init_data_index[j*para->num_datasubcar_perslot+2*i+1] = dataphy_index[j*para->num_subcar_of_subch+4*i+2];
        }
    /* data index for the 2nd symbol */
	for (i=0; i<para->num_tiles_perslot; i++)
       {    
	    init_data_index[j*para->num_datasubcar_perslot+12+4*i] = dataphy_index[j*para->num_subcar_of_subch+4*i] 
                                                                     + para->ofdma_nfft;
	    init_data_index[j*para->num_datasubcar_perslot+12+4*i+1] = dataphy_index[j*para->num_subcar_of_subch+4*i+1]
                                                       		       + para->ofdma_nfft;
            init_data_index[j*para->num_datasubcar_perslot+12+4*i+2] = dataphy_index[j*para->num_subcar_of_subch+4*i+2] 
	                                                               + para->ofdma_nfft;
	    init_data_index[j*para->num_datasubcar_perslot+12+4*i+3] = dataphy_index[j*para->num_subcar_of_subch+4*i+3] 
	                                                               + para->ofdma_nfft;
       }
		/* data index for the 3rd symbol */
        for (i=0; i<para->num_tiles_perslot; i++)
        {       
	    init_data_index[j*para->num_datasubcar_perslot+36+2*i] = dataphy_index[j*para->num_subcar_of_subch+4*i+1] 
	                                                             + para->ofdma_nfft * 2;
	    init_data_index[j*para->num_datasubcar_perslot+36+2*i+1] = dataphy_index[j*para->num_subcar_of_subch+4*i+2] 
	                                                             + para->ofdma_nfft * 2;
        }
    }


DO_DUMP(DUMP_PHY_UL_ZONEPERM_INIT_DATA_IDX_ID, 0, init_data_index, 48*subch_num);


#ifdef _SUBCAR_ROTATION_
    for (j=0; j<subch_num; j++)
    {
        for (i=0;i<para->num_datasubcar_perslot;i++)
        {
            rotdata_index = (i+13*j) % para->num_datasubcar_perslot;
            data_rotation_index[j*48+i]=init_data_index[j*48+rotdata_index];
        }
    } 

DO_DUMP(DUMP_PHY_UL_ZONEPERM_ROTA_DATA_IDX_ID, 0, data_rotation_index, 48*subch_num);


#endif
	
	/* data index for the whole UL Zone */
    used_datanum = subch_num * para->num_datasubcar_perslot;//1344
    for (i=0; i<para->zone_len; i++)
    {
        for (j=0; j< used_datanum; j++)
	{
#ifdef _SUBCAR_ROTATION_
            data_index_zone[i*used_datanum+j] = data_rotation_index[j] + i*para->ofdma_nfft*3;
#else
            data_index_zone[i*used_datanum+j] = init_data_index[j] + i*para->ofdma_nfft*3;
#endif
	}
    }

/* processing for subchanel rotation */
    memcpy(data_allocation, data_index_zone,used_datanum*para->zone_len*sizeof(u_int32_t));

    for (j=0; j<sum; j++)
    {
        for (i=0; i<48; i++)
        {
            data_allocation[rotation_index[j]*48+i]=data_index_zone[rotation_posindex[j]*48+i];
        }

    }
	
	/* generate Ranging_index */
    used_datanum = para->num_subcar_of_subch*subch_num;
//    printf("used_datanum = %d\n", used_datanum);
    for (i=0; i<para->symbolnum_per_ul_frame; i++)
    {
        for (j=0; j<used_datanum; j++)
	{
	    ranging_zone_index[i*used_datanum+j] = dataphy_index[j] + i*para->ofdma_nfft;  
	}
    }
	
	//144-ranging_index number for one symbol; 672--subcar number for one symbol; 24--para->num_subcar_of_subch
    for (j=0; j<ranging_sym; j++)
    {	    
        for (i=0; i<ranging_subch; i++)
	{
	    for (k=0; k<24; k++)
            {
	        ranging_allocation[144*j+24*i+k] = ranging_zone_index[(j+para->ranging_symoffset)*used_datanum+ranging_slotindex[i]*24+k];
	    }
	}
    }
	
    /* Sorting the ranging index */
    qsort(ranging_allocation,144*ranging_sym,sizeof(int32_t),cmp);
   
    return SUCCESS_CODE;
}



/* restore _DUMP_UTIL_ENABLE_ setting */
#ifndef DUMP_PHY_UL_ZONE_PERMUTATION

#ifdef LOCAL_DUMP_ENABLE
#define _DUMP_UTIL_ENABLE_
#endif

#undef LOCAL_DUMP_ENABLE
#endif


