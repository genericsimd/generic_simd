/* ----------------------------------------------------------------------------
   IBM Confidential

   IBM Wireless M2M Platform   

   (C)Copyright IBM Corp. 2009, 2010, 2011

   All Rights Reserved.

   File Name: phy_dl_ofdmamodul_cdd.c

   Function: Do OFDMA modulation.

   Change Activity:

   Date             Description of Change                            By
   -----------      ---------------------                            --------


   ----------------------------------------------------------------------------
   PROLOG END TAG zYx                                                         */

#include <memory.h>
#include <stdlib.h>
#include <stdio.h>
#include "phy_dl_ofdmamodul_cdd.h"
#include "flog.h"


/**----------------------------------------------------------------------------
   Function:    phy_dl_ofdmamodul_cdd()

   Description: Do OFDMA modulation, which includes insert DC, move
                zero-frequency component DFT to the center of spectrum, FFT,
                and insert guard band.

   Parameters:
                Input-  [const struct phy_dl_tx_syspara *para]  The pointer refer to the
                        struct of system parameters.
                        [const float *input_r]  The pointer refer to the real
                        part of 3 OFDMA symbols.
                        [const float *input_i]  The pointer refer to the
                        imaginary part of 3 OFDMA symbols.

                Output- [float *output_r]  The pointer refer to the real part
                        of output data, 3*1088 samples.
                        [float *output_i]  The pointer refer to the imaginary
                        part of output data, 3*1088 samples.

   Return Value:
                0       Success
                150     Error

   ----------------------------------------------------------------------------
   LOG END TAG zYx                                                            */

#ifdef IPP_OPT_TEMP730
int phy_dl_ofdmamodul_cdd(const struct phy_dl_tx_syspara *para,
                            const float *input_r,
                            const float *input_i,
                            float *output_r,
                            float *output_i)
{
    unsigned int symbol_count, sample_count, N_2, residual;
    const Ipp32f *sym_src_r, *sym_src_i;
    Ipp32f *sym_dst_r, *sym_dst_i;
    Ipp32f sym_dst_r_temp[1200], sym_dst_i_temp[1200];
    Ipp32f sym_shift_r[OFDMA_SYMBOL_SIZE], sym_shift_i[OFDMA_SYMBOL_SIZE];

    if (para == NULL || input_r == NULL || input_i == NULL) {
        FLOG_ERROR("E001_mod: the pointer refer to input buffer is null!\n");
        return ERROR_CODE;
    }

    if (output_r == NULL || output_i == NULL) {
        FLOG_ERROR("E002_mod: the pointer refer to output buffer is null!\n");
        return ERROR_CODE;
    }

    N_2 = para->ofdma_nused_no_dc/2;
    if ( (residual = N_2 % 2) != 0 ) {
        FLOG_ERROR("E003_mod: the used subcarriers must be a even number!\n");
        return ERROR_CODE;
    }

    for (symbol_count=0; symbol_count<para->symbol_per_slot; symbol_count++) {
        sym_src_r = input_r + symbol_count * para->ofdma_nused_no_dc;
        sym_src_i = input_i + symbol_count * para->ofdma_nused_no_dc;

        sym_dst_r = output_r + symbol_count * para->ofdma_symlen_with_guard;
        sym_dst_i = output_i + symbol_count * para->ofdma_symlen_with_guard;

        /* DC is zero */
        sym_shift_r[0] = sym_shift_i[0] = 0;
        /* sym_shift(1:N/2)=sym_src(N/2:N-1),  where N=N_2*2 */
        for (sample_count=1; sample_count<=N_2; sample_count++) {
            sym_shift_r[sample_count] = sym_src_r[N_2+sample_count-1];
            sym_shift_i[sample_count] = sym_src_i[N_2+sample_count-1];
        }
        /* sym_shift(N/2+1:fft_length-N/2-1)=0 */
        for (sample_count=N_2+1; sample_count<=para->ofdma_nfft-N_2-1; sample_count++) {
            sym_shift_r[sample_count] = 0;
            sym_shift_i[sample_count] = 0;
        }
        /* sym_shift(fft_length-N/2:fft_length-1) = sym_src(0:N/2-1) */
        for (sample_count=para->ofdma_nfft-N_2; sample_count<=para->ofdma_nfft-1; sample_count++) {
            sym_shift_r[sample_count] = sym_src_r[sample_count-(para->ofdma_nfft-N_2)];
            sym_shift_i[sample_count] = sym_src_i[sample_count-(para->ofdma_nfft-N_2)];
        }

        /* Time domain symbol */
        ippsFFTInv_CToC_32f(sym_shift_r, sym_shift_i, sym_dst_r_temp+para->ofdma_ncp+para->cdd_num, sym_dst_i_temp+para->ofdma_ncp+para->cdd_num, g_pFFTSpecInv, g_BufInv);


        /* Add CP */
        for (sample_count=0; sample_count<para->ofdma_ncp+para->cdd_num; sample_count++) {
            sym_dst_r_temp[sample_count] =  sym_dst_r_temp[para->ofdma_nfft + sample_count];
            sym_dst_i_temp[sample_count] =  sym_dst_i_temp[para->ofdma_nfft + sample_count];
        }

         for (sample_count=0; sample_count< para->ofdma_symlen_with_guard; sample_count++) {
            sym_dst_r[sample_count] =  sym_dst_r_temp[sample_count];
            sym_dst_i[sample_count] =  sym_dst_i_temp[sample_count];
        }


    }

    return SUCCESS_CODE;
}
#else
#include "fft.h"
int phy_dl_ofdmamodul_cdd(const struct phy_dl_tx_syspara *para,
                          const float *input_r,
                          const float *input_i,
                          float *output_r,
                          float *output_i)
{
    unsigned int symbol_count, sample_count, N_2, residual;
    const float *sym_src_r, *sym_src_i;
    float *sym_dst_r, *sym_dst_i;
#ifdef VSXOPT
    float sym_dst_r_temp[OFDMA_SYMBOL_SIZE] __attribute__ ((aligned (128))), sym_dst_i_temp[OFDMA_SYMBOL_SIZE] __attribute__ ((aligned (128)));
    float sym_shift_r[OFDMA_SYMBOL_SIZE] __attribute__ ((aligned (128))), sym_shift_i[OFDMA_SYMBOL_SIZE] __attribute__ ((aligned (128)));
#else
    float sym_dst_r_temp[1200], sym_dst_i_temp[1200];
    float sym_shift_r[OFDMA_SYMBOL_SIZE], sym_shift_i[OFDMA_SYMBOL_SIZE];
#endif


    if (para == NULL || input_r == NULL || input_i == NULL) {
        FLOG_ERROR("E001_mod: the pointer refer to input buffer is null!\n");
        return ERROR_CODE;
    }

    if (output_r == NULL || output_i == NULL) {
        FLOG_ERROR("E002_mod: the pointer refer to output buffer is null!\n");
        return ERROR_CODE;
    }

    N_2 = para->ofdma_nused_no_dc/2;
    if ( (residual = N_2 % 2) != 0 ) {
        FLOG_ERROR("E003_mod: the used subcarriers must be a even number!\n");
        return ERROR_CODE;
    }

    for (symbol_count=0; symbol_count<para->symbol_per_slot; symbol_count++) {
        sym_src_r = input_r + symbol_count * para->ofdma_nused_no_dc;
        sym_src_i = input_i + symbol_count * para->ofdma_nused_no_dc;

        sym_dst_r = output_r + symbol_count * para->ofdma_symlen_with_guard;
        sym_dst_i = output_i + symbol_count * para->ofdma_symlen_with_guard;

        /* DC is zero */
        sym_shift_r[0] = sym_shift_i[0] = 0;
        /* sym_shift(1:N/2)=sym_src(N/2:N-1),  where N=N_2*2 */
        for (sample_count=1; sample_count<=N_2; sample_count++) {
            sym_shift_r[sample_count] = sym_src_r[N_2+sample_count-1];
            sym_shift_i[sample_count] = sym_src_i[N_2+sample_count-1];
        }
        /* sym_shift(N/2+1:fft_length-N/2-1)=0 */
        for (sample_count=N_2+1; sample_count<=para->ofdma_nfft-N_2-1; sample_count++) {
            sym_shift_r[sample_count] = 0;
            sym_shift_i[sample_count] = 0;
        }
        /* sym_shift(fft_length-N/2:fft_length-1) = sym_src(0:N/2-1) */
        for (sample_count=para->ofdma_nfft-N_2; sample_count<=para->ofdma_nfft-1; sample_count++) {
            sym_shift_r[sample_count] = sym_src_r[sample_count-(para->ofdma_nfft-N_2)];
            sym_shift_i[sample_count] = sym_src_i[sample_count-(para->ofdma_nfft-N_2)];
        }

        /* Time domain symbol */
#ifdef VSXOPT
        ifft_p2(para->ofdma_nfft,
             sym_dst_r_temp,
             sym_dst_i_temp,
             sym_shift_r,
             sym_shift_i,
             para->XX,
             para->x,
             para->X);
#else
        ifft(para->ofdma_nfft,
             sym_dst_r_temp + para->ofdma_ncp+para->cdd_num,
             sym_dst_i_temp + para->ofdma_ncp+para->cdd_num,
             sym_shift_r,
             sym_shift_i,
             para->XX,
             para->x,
             para->X);
#endif

        /* Add CP */
#ifdef VSXOPT
    if(para->ofdma_symlen_with_guard > (para->ofdma_ncp + para->cdd_num))
    {
        int len1 = para->ofdma_ncp + para->cdd_num;
        int len2 = para->ofdma_symlen_with_guard - len1;

        memcpy( sym_dst_r, sym_dst_r_temp + ( para->ofdma_nfft - len1), sizeof(float) * len1);
        memcpy( sym_dst_r + len1, sym_dst_r_temp, sizeof(float) * len2);
        memcpy( sym_dst_i, sym_dst_i_temp + ( para->ofdma_nfft - len1), sizeof(float) * len1);
        memcpy( sym_dst_i + len1, sym_dst_i_temp, sizeof(float) * len2);
    }
    else
    {
        int len1 = para->ofdma_symlen_with_guard;

        memcpy( sym_dst_r, sym_dst_r_temp + ( para->ofdma_nfft - len1), sizeof(float) * len1);
        memcpy( sym_dst_i, sym_dst_i_temp + ( para->ofdma_nfft - len1), sizeof(float) * len1);
    }

#else
        for (sample_count=0; sample_count<para->ofdma_ncp+para->cdd_num; sample_count++) {
            sym_dst_r_temp[sample_count] =  sym_dst_r_temp[para->ofdma_nfft + sample_count];
            sym_dst_i_temp[sample_count] =  sym_dst_i_temp[para->ofdma_nfft + sample_count];
        }

         for (sample_count=0; sample_count< para->ofdma_symlen_with_guard; sample_count++) {
            sym_dst_r[sample_count] =  sym_dst_r_temp[sample_count];
            sym_dst_i[sample_count] =  sym_dst_i_temp[sample_count];
        }
#endif

    }

    return SUCCESS_CODE;
}
#endif

