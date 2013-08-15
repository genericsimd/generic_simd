/* ----------------------------------------------------------------------------
 * IBM Confidential

 IBM Wireless M2M Platform

 (C)Copyright IBM Corp. 2009, 2010, 2011

 All Rights Reserved.

 File Name: mac_phy_data.c

 Change Activity:

 Date             Description of Change                            By
 -----------      ---------------------                            --------
 12-Apr 2011       Created                                         jian jun chang 

 ----------------------------------------------------------------------------
 PROLOG END TAG zYx                                                         */
#include "mac_phy_data.h"


int set_dl_perm(enum permutation_scheme dlperm)
{
    dl_perm = dlperm;
    return 0;
}

int set_ul_perm(enum permutation_scheme ulperm)
{
    ul_perm = ulperm;
    return 0;
}

int get_subch_num(u_int8_t is_dl, int* subchannel_num)
{
    if(is_dl)
        {   
        *subchannel_num =  DL_OFDMA_DATA[dl_perm][OFDMA_SUBCH_NUM];
        }
    else
         {
        *subchannel_num = UL_OFDMA_DATA[ul_perm][OFDMA_SUBCH_NUM];
         }
    return 0;
}

int get_datasubcar_persubch_persym(u_int8_t is_dl, int* data_carrier_num)
{
    if(is_dl)
         {
        *data_carrier_num = DL_OFDMA_DATA[PHYTHER_DL_MODE][OFDMA_USED_SUBCAR_NUM];
        return 0;
         }
    else
          {
        *data_carrier_num = UL_OFDMA_DATA[PHYTHER_UL_MODE][OFDMA_USED_SUBCAR_NUM];
        return 0;
          }
    return 0;
}

// in the unit of ofdma symbol
int get_slot_length(u_int8_t is_dl, int* slot_length)
{

    if(is_dl)
         {
        *slot_length = DL_OFDMA_DATA[PHYTHER_DL_MODE][CURRENT_DL_OFDMA_SLOT_LENGTH_INSYM]; 
         }
    else 
         { 
        *slot_length = UL_OFDMA_DATA[PHYTHER_UL_MODE][CURRENT_UL_OFDMA_SLOT_LENGTH_INSYM];
          }
    return 0;
}

int get_bits_persubch_persym(u_int8_t is_dl, modulation_type mc_type, int* bit_num)
{
    if(is_dl)
    {
        *bit_num = DL_OFDMA_DATA[PHYTHER_DL_MODE][CURRENT_DL_OFDMA_DATASUBCAR_NUM_PERSUBCH_PERSYM]* get_modulation_rate(mc_type);
    }
    else
    {
        *bit_num = UL_OFDMA_DATA[PHYTHER_UL_MODE][CURRENT_UL_OFDMA_DATASUBCAR_NUM_PERSUBCH_PERSYM]* get_modulation_rate(mc_type);
    } 
    return 0;
}
