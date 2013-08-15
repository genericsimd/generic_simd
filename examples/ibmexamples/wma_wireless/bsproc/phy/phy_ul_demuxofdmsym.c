/* ----------------------------------------------------------------------------
   IBM Confidential

   IBM Wireless M2M Platform

   (C)Copyright IBM Corp. 2009, 2010, 2011

   All Rights Reserved.

   File Name: phy_ul_demuxofdmsym.c


   Function: This function is used to demultiplex the subcarrier according to the 

             data subcar allocation. It's the oppositive process of ofdm multiplex.



   Change Activity:



   Date             Description of Change                            By

   -----------      ---------------------                            --------

   ----------------------------------------------------------------------------

   PROLOG END TAG zYx                                                         */



#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "phy_ul_rx_common.h"

#include "phy_ul_demuxofdmsym.h"
#include "flog.h"



/**----------------------------------------------------------------------------

   Function:    phy_ofdma_ul_demuxofdmsym()



   Description: This function is used to demultiplex the constellation-mapped data 

                sequences and pilots into the physical subcarriers according to the 

                zone permutation result.



   Parameters:

   

       Input: const phy_ul_rx_syspara *para;  data structure for system parameters transmission;

              const float *muxofdm_data_r []      The real part of ofdm symbol; 

              const float *muxofdm_data_i []  The imag part of ofdm symbol;

              const int *datasubcar []            The position of data subcarrier;


       Output: float *demuxofdmsym_r []     demultiplex subcarrier (real part);

               float *demucofdmsym_i []     demultiplex subcarrier (imag part);        



   Return Value:

                0       Success

                150     Error



   ----------------------------------------------------------------------------

   LOG END TAG zYx                                                            */



int32_t phy_ul_demuxofdmsym(struct phy_ul_rx_syspara *para,
                            const float *muxofdm_data_r,
                            const float *muxofdm_data_i,
                            const u_int32_t *datasubcar,
                            const u_int32_t *rotation_posindex,
                            const u_int32_t *slotsym_num,
                            float *demuxofdmsym_r,
                            float *demuxofdmsym_i)
{
    u_int32_t nused;
    u_int32_t i, j, k, count;
    u_int32_t  posindex_used[35*6];
    u_int32_t  idx_tmp, temp;
    u_int32_t  unit_len=48;

    memset(posindex_used, 0, sizeof(u_int32_t)*35*6);


    nused = para->ofdma_nused_no_dc; //757-1=756
    //nsym = para->symbol_per_slot; //3
    

    if (muxofdm_data_r == NULL || muxofdm_data_i == NULL) {
        FLOG_ERROR("E001_demuxofdmsym: the pointer refer to input buffer is null!\n");
        return ERROR_CODE;
    }

    if (datasubcar == NULL) {
        FLOG_ERROR("E002_muxofdmsym: the pointer refer to zone permutation result is null!\n");
        return ERROR_CODE;
    }

    if (demuxofdmsym_r == NULL || demuxofdmsym_i == NULL) {
        FLOG_ERROR("E003_demuxofdmsym: the pointer refer to output buffer is null!\n");
        return ERROR_CODE;
    }

    count = slotsym_num[0]*3*para->ofdma_nfft;

    for(i=0; i<para->ranging_subch; i++)
    {
        posindex_used[i] = i;
    }

    for(i=0; i<para->usedsubch_num*para->zone_len; i++)
    {
        posindex_used[para->ranging_subch+i] = rotation_posindex[i];
    }

    for(j=0; j<para->usedsubch_num; j++)
    {
        temp = posindex_used[j+slotsym_num[0]*para->usedsubch_num]-slotsym_num[0]*para->avasubch_num; 
        for (k=0; k<unit_len; k++)
	{
		i = temp * unit_len + k;

		if (datasubcar[i]<count+1024)
		{
			idx_tmp = datasubcar[i]-count;
		}
		else if (datasubcar[i] >= count+2048)
		{
			idx_tmp = 2*nused + datasubcar[i]-2048-count;
		}
		else
		{
			idx_tmp = nused + datasubcar[i]-1024-count;
		}

 		demuxofdmsym_r[j * unit_len + k] = muxofdm_data_r[idx_tmp];
		demuxofdmsym_i[j * unit_len + k] = muxofdm_data_i[idx_tmp];
	}
    }

    return SUCCESS_CODE;

}

int32_t phy_ul_demuxpilot(struct phy_ul_rx_syspara *para,
                          const float *muxofdm_data_r,
                          const float *muxofdm_data_i,
                          const u_int32_t *pilot_allocation,
                          const u_int32_t *rotation_posindex,
                          const u_int32_t *slotsym_num,
                          float *demuxpilot_r,
                          float *demuxpilot_i)
{
    u_int32_t nused;
    u_int32_t i, j, k, count;
    u_int32_t  posindex_used[35*6];
    u_int32_t  unit_len=24;
    u_int32_t  idx_tmp, temp;

    memset(posindex_used, 0, sizeof(u_int32_t)*35*6);

    nused = para->ofdma_nused_no_dc; //757-1=756
    //nsym = para->symbol_per_slot; //3


    if (muxofdm_data_r == NULL || muxofdm_data_i == NULL) {
        FLOG_ERROR("E001_demuxpilot: the pointer refer to input buffer is null!\n");
        return ERROR_CODE;
    }

    if (pilot_allocation == NULL) {
        FLOG_ERROR("E002_demuxpilot: the pointer refer to zone permutation result is null!\n");
        return ERROR_CODE;
    }

    if (demuxpilot_r == NULL || demuxpilot_i == NULL) {
        FLOG_ERROR("E003_demuxpilot: the pointer refer to output buffer is null!\n");
        return ERROR_CODE;
    }

    for(i=0; i<para->ranging_subch; i++)
    {
        posindex_used[i] = i;
    }

     for(i=0; i<para->usedsubch_num*para->zone_len; i++)
    {
        posindex_used[para->ranging_subch+i] = rotation_posindex[i];
    }

    count = slotsym_num[0]*3*para->ofdma_nfft;
    for(j=0; j<para->usedsubch_num; j++)
    {
	    temp = posindex_used[j+slotsym_num[0]*para->usedsubch_num]-slotsym_num[0]*para->avasubch_num; 
	    for (k=0; k<unit_len; k++)
	    {
		    i = temp * unit_len + k;

		    if (pilot_allocation[i]<count+1024)
		    {
			    idx_tmp = pilot_allocation[i]-count;
		    }
		    else
		    {
			    idx_tmp = nused*2+pilot_allocation[i]-2048-count;
		    }
		    demuxpilot_r[j * unit_len + k] = muxofdm_data_r[idx_tmp];
		    demuxpilot_i[j * unit_len + k] = muxofdm_data_i[idx_tmp];
	    }
    }
    

    return SUCCESS_CODE;

}

