/* ----------------------------------------------------------------------------
   IBM Confidential

   IBM Wireless M2M Platform   

   (C)Copyright IBM Corp. 2009, 2010, 2011

   All Rights Reserved.

   File Name: phy_dl_muxofdmsym.c

   Function: This function is used to multiplex the constellation-mapped data
             sequences and pilots into the physical subcarriers according to
             the zone permutation result.

   ----------------------------------------------------------------------------
   PROLOG END TAG zYx                                                         */


#include <stdio.h>
#include "phy_dl_muxofdmsym.h"
#include "flog.h"

/**----------------------------------------------------------------------------
   Function:    phy_dl_muxofdmsym()

   Description: This function is used for subcar allocation for datasubcar and pilot
                subcar according to the results of zone permutation.The constant gain
                (4.0/3) is for power adjustment.

   Parameters:

       Input: struct struct phy_dl_tx_syspara *para;  data structure for system parameters transmission;
              const float *modulation_r [756*2]      The real part of modulation result;
              const float *modulation_i [756*2]      The imag part of modulation result;
              const int *datasubcar           The position of data subcarrier;
              const int *pilotsubcar          The position of pilot subcarrier;

              The parameters used for ofdm symbol multiplexing include:
                 para.ofdma_nused_no_dc = 756;   used subcarrier for one OFDM symbol;
                 para.symbol_per_slot = 3; symbol number of per slot for AMC
                 

       Output: float *muxofdmsym_r     map constellation result (real part);
               float *mucofdmsym_i     map constellation result (imag part);

   Return Value:
                0       Success
                150     Error

   ----------------------------------------------------------------------------
   LOG END TAG zYx                                                            */

int phy_dl_muxofdmsym(struct phy_dl_tx_syspara  *para,
                      u_int32_t num_unused_subch,
                      const float (*modulation_r),
                      const float (*modulation_i),
                      const u_int32_t (*datasubcar),
                      const u_int32_t (*pilotsubcar),
                      float *muxofdmsym_r,
                      float *muxofdmsym_i)

{
    const float power_factor = 4.0/3.0; //constant gain for power adjustment
    unsigned int pilot_insert; 

    u_int32_t nused, nsym;
    u_int32_t pilotlength, datalength;
    u_int32_t i;

    nused = para->ofdma_nused_no_dc; //756
    nsym = para->symbol_per_slot; //3
    pilotlength = (para->num_subch-num_unused_subch)*6; //240
    datalength = (para->num_subch-num_unused_subch)*para->num_subcar_of_subch; 
    pilot_insert = para->pilot_insert; //1--insert pilot; 0--no;
    if (modulation_r == NULL || modulation_i == NULL)
    {
        FLOG_ERROR("E001_muxofdmsym: the pointer refer to input buffer is null!\n");
        return ERROR_CODE;
    }

    if (datasubcar == NULL || pilotsubcar == NULL) 
    {
        FLOG_ERROR("E002_muxofdmsym: the pointer refer to zone permutation result is null!\n");
        return ERROR_CODE;
    }

    if (muxofdmsym_r == NULL || muxofdmsym_i == NULL) 
    {
        FLOG_ERROR("E003_muxofdmsym: the pointer refer to output buffer is null!\n");
        return ERROR_CODE;
    }
    /* for data subcarrier mapping */
    for (i=0; i<datalength;i++)
    {
        if (datasubcar[i] < 2048)
        {
            muxofdmsym_r[datasubcar[i]-1024] = modulation_r[i];
            muxofdmsym_i[datasubcar[i]-1024] = modulation_i[i];
        }
        else if(datasubcar[i] >= 3072)
        {
            muxofdmsym_r[2*nused+datasubcar[i]-3072] = modulation_r[i];
            muxofdmsym_i[2*nused+datasubcar[i]-3072] = modulation_i[i];
        }
        else
        {
             muxofdmsym_r[nused+datasubcar[i]-2048] = modulation_r[i];
             muxofdmsym_i[nused+datasubcar[i]-2048] = modulation_i[i];
        }
    }

  /* for pilot inserting */
    for (i=0;i<pilotlength;i++) {
        if (pilotsubcar[i]<2048) {
            if(pilot_insert == 1) {
                muxofdmsym_r[pilotsubcar[i]-1024] = 1.0 * power_factor;}
            else {
                muxofdmsym_r[pilotsubcar[i]-1024] = 0.0;
            }
            muxofdmsym_i[pilotsubcar[i]-1024] = 0.0;
        }
        else if (pilotsubcar[i]>=3072)
        {
            if(pilot_insert == 1) {
                muxofdmsym_r[2*nused+pilotsubcar[i]-3072] = 1.0 * power_factor;}
            else {
                muxofdmsym_r[2*nused+pilotsubcar[i]-3072] = 0.0;
            }
            muxofdmsym_i[2*nused+pilotsubcar[i]-3072] = 0.0;
        }
        else
        {
            if(pilot_insert == 1) {
                muxofdmsym_r[nused+pilotsubcar[i]-2048] = 1.0 * power_factor;}
            else {
                muxofdmsym_r[nused+pilotsubcar[i]-2048] = 0.0;
            }
            muxofdmsym_i[nused+pilotsubcar[i]-2048] = 0.0;
        }
    }

    return 0;

}
