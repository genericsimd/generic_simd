/* ----------------------------------------------------------------------------
   IBM Confidential

   IBM Wireless M2M Platform   

   (C)Copyright IBM Corp. 2009, 2010, 2011

   All Rights Reserved.

   File Name: phy_dl_modulation.c

   Function:  modulation

   ----------------------------------------------------------------------------
   PROLOG END TAG zYx                                                         */


#include <stdio.h>
#include <assert.h>
#include <math.h>
#include "phy_dl_modulation.h"
#include "flog.h"

int phy_dl_modulation(struct phy_dl_fec_para *para,
                      unsigned char *bit,
                      float *symbol_r,
                      float *symbol_i)
{
    u_int32_t i;
    float sqrt_const;
    switch (para->code_id)
    {
        case CC_QPSK12:
        case CC_QPSK34:
        {
            int QPSK_Modulate[8]=  { 1, 1,  1, -1, -1, 1, -1, -1};
            sqrt_const = 1/sqrt(2);  //1/sqrt(2)   normalize the average power to 1

            assert(para->burst_len_repeated%2==0);

            for ( i=0;i<para->burst_len_repeated/2;i++ ){
                int index=2*i;
                //the first bit shall be mapped to the higher index bit in the constellation
                int tmp=bit[index]*2+bit[index+1];
                symbol_r[ i ] = (float)sqrt_const*QPSK_Modulate[ tmp*2 ];
                symbol_i[ i ] = (float)sqrt_const*QPSK_Modulate[ tmp*2+1];
            }
            para->burst_len_modulated = para->burst_len_repeated/2;
            break;
        }
        case CC_QAM16_12:
        case CC_QAM16_34:
        {
            int QAM16_Modulate[32]={ 1, 1,  1, 3,  1, -1,  1, -3,
                                     3, 1,  3, 3,  3, -1,  3, -3,
                                    -1, 1, -1, 3, -1, -1, -1, -3,
                                    -3, 1, -3, 3, -3, -1, -3, -3};
            sqrt_const = 1/sqrt(10);  //1/sqrt(10)   normalize the average power to 1

            assert(para->burst_len_repeated%4==0);

            for ( i=0;i<para->burst_len_repeated/4;i++ ){
                int index=4*i;
                //the first bit shall be mapped to the higher index bit in the constellation
                int tmp=bit[index]*8+bit[index+1]*4+bit[index+2]*2+bit[index+3];
                symbol_r[ i ] = (float)sqrt_const*QAM16_Modulate[ tmp*2 ];
                symbol_i[ i ] = (float)sqrt_const*QAM16_Modulate[ tmp*2+1];
            }
            para->burst_len_modulated = para->burst_len_repeated/4;
            break;
        }
        case CC_QAM64_12:
        case CC_QAM64_23:
        case CC_QAM64_34:
        {
            int QAM64_Modulate[128]={ 3, 3,  3, 1, 3, 5, 3, 7, 3, -3, 3, -1, 3,-5, 3,-7,
                                      1, 3,  1, 1, 1, 5, 1, 7, 1, -3, 1, -1, 1,-5, 1,-7,
                                      5, 3,  5, 1, 5, 5, 5, 7, 5, -3, 5, -1, 5,-5, 5,-7,
                                      7, 3,  7, 1, 7, 5, 7, 7, 7, -3, 7, -1, 7,-5, 7,-7,
                                     -3, 3, -3, 1,-3, 5,-3, 7,-3, -3,-3, -1,-3,-5,-3,-7,
                                     -1, 3, -1, 1,-1, 5,-1, 7,-1, -3,-1, -1,-1,-5,-1,-7,
                                     -5, 3, -5, 1,-5, 5,-5, 7,-5, -3,-5, -1,-5,-5,-5,-7,
                                     -7, 3, -7, 1,-7, 5,-7, 7,-7, -3,-7, -1,-7,-5,-7,-7};
            sqrt_const = 1/sqrt(42); //1/sqrt(42)   normalize the average power to 1

            assert(para->burst_len_repeated%6==0);

            for ( i=0;i<para->burst_len_repeated/6;i++ ){
                int index=6*i;
                //the first bit shall be mapped to the higher index bit in the constellation
                int tmp=bit[index]*32+bit[index+1]*16+bit[index+2]*8 +bit[index+3]*4 +bit[index+4]*2+bit[index+5] ;
                symbol_r[ i ] = (float)sqrt_const*QAM64_Modulate[ tmp*2 ];
                symbol_i[ i ] = (float)sqrt_const*QAM64_Modulate[ tmp*2+1];
            }
            para->burst_len_modulated = para->burst_len_repeated/6;
            break;
        }

        default:
            FLOG_ERROR("Unsupport code id %d in modulation \n", para->code_id);
            return -1;
    }
    return 0;
}

