/* ----------------------------------------------------------------------------
   IBM Confidential

   IBM Wireless M2M Platform   

   (C)Copyright IBM Corp. 2009, 2010, 2011

   All Rights Reserved.
   
   File Name: phy_dl_ofdmamodul.c

   Function: Do OFDMA modulation.

   Change Activity:

   Date             Description of Change                            By
   -----------      ---------------------                            --------


   ----------------------------------------------------------------------------
   PROLOG END TAG zYx                                                         */

#include <memory.h>
#include <stdlib.h>
#include <stdio.h>
#include "phy_dl_ofdmamodul.h"
#include "flog.h"

/* turn on/off dump according to DUMP_PHY_DL_OFDM_MOD setting */
#ifndef DUMP_PHY_DL_OFDM_MOD

#undef LOCAL_DUMP_ENABLE

#ifdef _DUMP_UTIL_ENABLE_
#define LOCAL_DUMP_ENABLE
#endif

#undef _DUMP_UTIL_ENABLE_

#endif

#include "dump_util.h"

/**----------------------------------------------------------------------------
   Function:    phy_dl_ofdmamodul()

   Description: Do OFDMA modulation, which includes insert DC, move
                zero-frequency component DFT to the center of spectrum, FFT,
                and insert guard band.

   Parameters:
                Input-  [const struct phy_dl_tx_syspara *para]  The pointer refer to the
                        struct of system parameters.
                        [const float *input_r]  The pointer refer to the real
                        part of 3 OFDMA symbols, each of 756 samples.
                        [const float *input_i]  The pointer refer to the
                        imaginary part of 3 OFDMA symbols, each of 756 samples.

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
int phy_dl_ofdmamodul(const struct phy_dl_tx_syspara *para,
                      const float *input_r,
                      const float *input_i,
                      float *output_r,
                      float *output_i)
{
    unsigned int symbol_count, sample_count, N_2, residual;
    const Ipp32f *sym_src_r, *sym_src_i;
    Ipp32f *sym_dst_r, *sym_dst_i;
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


		DO_DUMP(DUMP_PHY_OFDM_MOD_SHIFT_R_ID, 0, sym_shift_r, 1024);
		DO_DUMP(DUMP_PHY_OFDM_MOD_SHIFT_I_ID, 0, sym_shift_i, 1024);

        /* Time domain symbol */
     ippsFFTInv_CToC_32f(sym_shift_r, sym_shift_i, sym_dst_r+para->ofdma_ncp, sym_dst_i+para->ofdma_ncp, g_pFFTSpecInv, g_BufInv);

		DO_DUMP(DUMP_PHY_OFDM_MOD_IFFT_R_ID, 64, sym_dst_r, 1088);
		DO_DUMP(DUMP_PHY_OFDM_MOD_IFFT_I_ID, 64, sym_dst_i, 1088);


        /* Add CP */
        for (sample_count=0; sample_count<para->ofdma_ncp; sample_count++) {
            sym_dst_r[sample_count] =  sym_dst_r[para->ofdma_nfft + sample_count];
            sym_dst_i[sample_count] =  sym_dst_i[para->ofdma_nfft + sample_count];
        }
    }

    return SUCCESS_CODE;
}
#else
#include "fft.h"
int phy_dl_ofdmamodul(const struct phy_dl_tx_syspara *para,
                      const float *input_r,
                      const float *input_i,
                      float *output_r,
                      float *output_i)
{
    unsigned int symbol_count, sample_count, N_2, residual;
    const float *sym_src_r, *sym_src_i;
    float *sym_dst_r, *sym_dst_i;

//#ifdef VSXOPT
#if 0 //Fixme
    float sym_dst_r_temp[OFDMA_SYMBOL_SIZE] __attribute__ ((aligned (128))), sym_dst_i_temp[OFDMA_SYMBOL_SIZE] __attribute__ ((aligned (128)));
    float sym_shift_r[OFDMA_SYMBOL_SIZE] __attribute__ ((aligned (128))), sym_shift_i[OFDMA_SYMBOL_SIZE] __attribute__ ((aligned (128)));
#else
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
//#ifdef VSXOPT

#if 0 //Fixme
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
             sym_dst_r + para->ofdma_ncp,
             sym_dst_i + para->ofdma_ncp,
             sym_shift_r,
             sym_shift_i,
             para->XX,
             para->x,
             para->X);
#endif

        /* Add CP */
//#ifdef VSXOPT
#if 0 //FIXME
        int len1 = para->ofdma_ncp;
        int len2 = para->ofdma_nfft;

        memcpy( sym_dst_r, sym_dst_r_temp + ( para->ofdma_nfft - len1), sizeof(float) * len1);
        memcpy( sym_dst_r + len1, sym_dst_r_temp, sizeof(float) * len2);
        memcpy( sym_dst_i, sym_dst_i_temp + ( para->ofdma_nfft - len1), sizeof(float) * len1);
        memcpy( sym_dst_i + len1, sym_dst_i_temp, sizeof(float) * len2);

#else
        for (sample_count=0; sample_count<para->ofdma_ncp; sample_count++) {
            sym_dst_r[sample_count] =  sym_dst_r[para->ofdma_nfft + sample_count];
            sym_dst_i[sample_count] =  sym_dst_i[para->ofdma_nfft + sample_count];
        }
#endif
    }

    return SUCCESS_CODE;
}


#endif

int dump_phy_dl_ofdm_mod(int flag, char * name, FILE * fd, int len, void *p_buf)
{
	(void) name;
	float *data = (float *)p_buf;
	
    int mm = 0;
    for (mm =flag; mm<len; mm++)
    {
        fprintf(fd, "%f\n", data[mm]);
    }
						
	return 0;
}


/* restore _DUMP_UTIL_ENABLE_ setting */
#ifndef DUMP_PHY_DL_OFDM_MOD

#ifdef LOCAL_DUMP_ENABLE
#define _DUMP_UTIL_ENABLE_
#endif

#undef LOCAL_DUMP_ENABLE
#endif



