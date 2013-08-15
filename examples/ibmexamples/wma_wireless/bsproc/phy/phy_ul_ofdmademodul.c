/* ----------------------------------------------------------------------------
   
   IBM Confidential

   IBM Wireless M2M Platform

   (C)Copyright IBM Corp. 2009, 2010, 2011

   All Rights Reserved.

   File Name: phy_ul_demodulation.c

   Function: Do OFDMA demodulation.

   Change Activity:


   Date             Description of Change                            By

   -----------      ---------------------                            --------
   ----------------------------------------------------------------------------

   PROLOG END TAG zYx                                                         */



#include <memory.h>

#include <stdlib.h>

#include <stdio.h>

#include <math.h>

#include "fft.h"

#include "phy_ul_rx_common.h"

#include "phy_ul_ofdmademodul.h"

#include "flog.h"

#ifdef SSE2OPT
#include <xmmintrin.h>
#include <emmintrin.h>
#endif


/* turn on/off dump according to DUMP_PHY_UL_OFDM_DEMOD setting */
#ifndef DUMP_PHY_UL_OFDM_DEMOD

#undef LOCAL_DUMP_ENABLE

#ifdef _DUMP_UTIL_ENABLE_
#define LOCAL_DUMP_ENABLE
#endif

#undef _DUMP_UTIL_ENABLE_

#endif

#include "dump_util.h"

#define _BOOST_

/**----------------------------------------------------------------------------

   Function:    phy_ul_ofdmademodul()



   Description: Do OFDMA demodulation, which includes remove CP, IFFT, remove

                DC and guard band, recover center-frequency component DFT to

                the zero of spectrum.



   Parameters:

                Input-  [const struct phy_ul_rx_syspara *para]  The pointer refer to the

                        struct of system parameters.

                        [const float *input_r]  The pointer refer to the real

                        part of 3 OFDMA symbols, each of 1152 samples.

                        [const float *input_i]  The pointer refer to the

                        imaginary part of 3 OFDMA symbols, each of 1152 samples.



                Output- [float *output_r]  The pointer refer to the real part

                        of output data, 3*840 samples.

                        [float *output_i]  The pointer refer to the imaginary

                        part of output data, 3*840 samples.



   Return Value:

                0       Success

                150     Error



   ----------------------------------------------------------------------------

   LOG END TAG zYx                                                            */


#ifdef IPP_OPT_TEMP730

#include "phy_ul_ipp_para.h"


int32_t phy_ul_ofdmademodul(const struct phy_ul_rx_syspara *para,
                            const float *input_r, const float *input_i,
                            float *output_r, float *output_i)

{

    unsigned int symbol_count, sample_count, N_2, residual;
    const float *sym_src_r, *sym_src_i;
    float *sym_dst_r, *sym_dst_i;

#ifdef SSE2OPT
    Ipp32f sym_tr[OFDMA_SYMBOL_SIZE] __attribute__ ((aligned (128)));
	Ipp32f sym_ti[OFDMA_SYMBOL_SIZE] __attribute__ ((aligned (128)));
    Ipp32f sym_fr[OFDMA_SYMBOL_SIZE] __attribute__ ((aligned (128)));
	Ipp32f sym_fi[OFDMA_SYMBOL_SIZE] __attribute__ ((aligned (128)));
#else
#ifdef VSXOPT
	Ipp32f sym_tr[OFDMA_SYMBOL_SIZE] __attribute__ ((aligned (128))), sym_ti[OFDMA_SYMBOL_SIZE] __attribute__ ((aligned (128)));
	Ipp32f sym_fr[OFDMA_SYMBOL_SIZE] __attribute__ ((aligned (128))), sym_fi[OFDMA_SYMBOL_SIZE] __attribute__ ((aligned (128)));
#else
	Ipp32f sym_tr[OFDMA_SYMBOL_SIZE], sym_ti[OFDMA_SYMBOL_SIZE];
	Ipp32f sym_fr[OFDMA_SYMBOL_SIZE], sym_fi[OFDMA_SYMBOL_SIZE];
#endif
#endif
    u_int32_t i;



    if (input_r == NULL || input_i == NULL) {
        FLOG_ERROR("E001_demod: the pointer refer to input buffer is null!\n");
        return ERROR_CODE;
    }

    if (output_r == NULL || output_i == NULL) {
        FLOG_ERROR("E002_demod: the pointer refer to output buffer is null!\n");
        return ERROR_CODE;
    }
    N_2 = para->ofdma_nused_no_dc/2;

    if ( (residual = N_2 % 2) != 0 ) {
        FLOG_ERROR("E003_demod: the used subcarriers must be a even number!\n");
        return ERROR_CODE;
    }


    for (symbol_count=0; symbol_count<para->symbol_per_slot; symbol_count++) {

        /* Remove CP */
		int idx_tmp= symbol_count * para->ofdma_sym_len_with_guard + para->ofdma_ncp;
        sym_src_r = input_r + idx_tmp;
        sym_src_i = input_i + idx_tmp;
		
		idx_tmp= symbol_count * para->ofdma_nused_no_dc;
        sym_dst_r = output_r + idx_tmp;
        sym_dst_i = output_i + idx_tmp;

#ifdef SSE2OPT

		__m128 vdemod_gain = _mm_set1_ps(para->demod_gain);
		for (sample_count=0; sample_count<para->ofdma_nfft; sample_count+=4) {
			__m128 vsym_src_r;
			__m128 vsym_src_i;

			vsym_src_r = _mm_load_ps( &sym_src_r[sample_count] );
			vsym_src_i = _mm_load_ps( &sym_src_i[sample_count] );
			_mm_store_ps( &sym_tr[sample_count], _mm_mul_ps( vsym_src_r, vdemod_gain ));
			_mm_store_ps( &sym_ti[sample_count], _mm_mul_ps( vsym_src_i, vdemod_gain ));
		}

#else

		for (sample_count=0; sample_count<para->ofdma_nfft; sample_count++) {
			sym_tr[sample_count] = sym_src_r[sample_count] * para->demod_gain;
			sym_ti[sample_count] = sym_src_i[sample_count] * para->demod_gain;
		}

#endif

/*
#ifdef _DEBUG_PHY_
    memset(test_data_file, 0, sizeof(char)*256);
    sprintf(test_data_file, "%s/%s",data_write_path,"fft_input_r.dat");

    if( (fpw_r = fopen(test_data_file, "a+t")) == NULL)
    {
        printf( "E011_test: Can not open file to write!\n");
        return ERROR_CODE;
    }

    memset(test_data_file, 0, sizeof(char)*256);
    sprintf(test_data_file, "%s/%s",data_write_path,"fft_input_i.dat");
    if( (fpw_i = fopen(test_data_file, "a+t")) == NULL)
    {
        printf( "E011_test: Can not open file to write!\n");
        return ERROR_CODE;
    }

    for (i=0; i<  1024; i++) {
         fprintf(fpw_r, "%f\n", sym_tr[i]);
         fprintf(fpw_i, "%f\n", sym_ti[i]);
     }

     fclose (fpw_r);
     fclose (fpw_i);

#endif
*/
    ippsFFTFwd_CToC_32f(sym_tr, sym_ti, sym_fr, sym_fi, pFFTSpecFwd_ul, BufFwd_ul);
#ifdef _BOOST_

#ifdef SSE2OPT
	__m128 vfactor = _mm_set1_ps(sqrt(para->ofdma_nfft));
	for (i=0; i<para->ofdma_nfft; i+=4)
	{
		__m128 vsym_fr;
		__m128 vsym_fi;

		vsym_fr = _mm_load_ps( &sym_fr[i] );
		vsym_fi = _mm_load_ps( &sym_fi[i] );
		_mm_store_ps( &sym_fr[i], _mm_div_ps( vsym_fr, vfactor ));
		_mm_store_ps( &sym_fi[i], _mm_div_ps( vsym_fi, vfactor ));
		
	}

#else
    float factor;
    factor = sqrt(para->ofdma_nfft);
    for (i=0; i<para->ofdma_nfft; i++)
    {
        sym_fr[i] = sym_fr[i]/factor;
        sym_fi[i] = sym_fi[i]/factor;
    }
#endif

#endif

	 DO_DUMP(DUMP_PHY_OFDM_DEMOD_FFT_R_ID, 0, sym_fr, 1024);
	 DO_DUMP(DUMP_PHY_OFDM_DEMOD_FFT_I_ID, 0, sym_fi, 1024);


        /* Remove DC and guard band,

           recover center-frequency component DFT to the zero of spectrum */

        /* sym_used = [sym_f((fft_length-N/2:fft_length-1)+1) sym_f((1:N/2)+1)] */

        for (sample_count=0; sample_count<=N_2-1; sample_count++) {

            sym_dst_r[sample_count] = sym_fr[(para->ofdma_nfft-N_2)+sample_count];

            sym_dst_i[sample_count] = sym_fi[(para->ofdma_nfft-N_2)+sample_count];

        }

        for (sample_count=N_2; sample_count<=para->ofdma_nused_no_dc-1; sample_count++) {

            sym_dst_r[sample_count] = sym_fr[sample_count - (N_2-1)];

            sym_dst_i[sample_count] = sym_fi[sample_count - (N_2-1)];

        }

    }



    return SUCCESS_CODE;

}


#else

#include "fft.h"

int32_t phy_ul_ofdmademodul(const struct phy_ul_rx_syspara *para,

                              const float *input_r, const float *input_i,

                              float *output_r,      float *output_i)

{

    unsigned int symbol_count, sample_count, N_2, residual;

    const float *sym_src_r, *sym_src_i;

    float *sym_dst_r, *sym_dst_i;
#ifdef VSXOPT
    float sym_tr[OFDMA_SYMBOL_SIZE *2] __attribute__ ((aligned (128))), sym_ti[OFDMA_SYMBOL_SIZE*2] __attribute__ ((aligned (128)));

    float sym_fr[OFDMA_SYMBOL_SIZE*2] __attribute__ ((aligned (128))), sym_fi[OFDMA_SYMBOL_SIZE*2] __attribute__ ((aligned (128)));

#else
    float sym_tr[OFDMA_SYMBOL_SIZE], sym_ti[OFDMA_SYMBOL_SIZE];

    float sym_fr[OFDMA_SYMBOL_SIZE], sym_fi[OFDMA_SYMBOL_SIZE];
#endif



    if (input_r == NULL || input_i == NULL) {

        FLOG_ERROR("E001_demod: the pointer refer to input buffer is null!\n");

        return ERROR_CODE;

    }



    if (output_r == NULL || output_i == NULL) {

        FLOG_ERROR("E002_demod: the pointer refer to output buffer is null!\n");

        return ERROR_CODE;

    }



    N_2 = para->ofdma_nused_no_dc/2;

    if ( (residual = N_2 % 2) != 0 ) {

        FLOG_ERROR("E003_demod: the used subcarriers must be a even number!\n");

        return ERROR_CODE;

    }



    for (symbol_count=0; symbol_count<para->symbol_per_slot; symbol_count++) {

        /* Remove CP */

        sym_src_r = input_r + symbol_count * para->ofdma_sym_len_with_guard + para->ofdma_ncp;

        sym_src_i = input_i + symbol_count * para->ofdma_sym_len_with_guard + para->ofdma_ncp;



        sym_dst_r = output_r + symbol_count * para->ofdma_nused_no_dc;

        sym_dst_i = output_i + symbol_count * para->ofdma_nused_no_dc;



        for (sample_count=0; sample_count<para->ofdma_nfft; sample_count++) {

            sym_tr[sample_count] = sym_src_r[sample_count] * para->demod_gain;

            sym_ti[sample_count] = sym_src_i[sample_count] * para->demod_gain;

        }



        /* Frequency domain symbol */

#ifdef VSXOPT
        fft_p1(para->ofdma_nfft, sym_tr, sym_ti, sym_fr, sym_fi,para->XX, para->x, para->X);
#else
        fft(para->ofdma_nfft, sym_tr, sym_ti, sym_fr, sym_fi,para->XX, para->x, para->X);
#endif
#ifdef _BOOST_
    float factor;
    u_int32_t i;
    factor = sqrt(para->ofdma_nfft);
    for (i=0; i<para->ofdma_nfft; i++)
    {
        sym_fr[i] = sym_fr[i]/factor;
        sym_fi[i] = sym_fi[i]/factor;
    }
#endif

        /* Remove DC and guard band,

           recover center-frequency component DFT to the zero of spectrum */

        /* sym_used = [sym_f((fft_length-N/2:fft_length-1)+1) sym_f((1:N/2)+1)] */

        for (sample_count=0; sample_count<=N_2-1; sample_count++) {

            sym_dst_r[sample_count] = sym_fr[(para->ofdma_nfft-N_2)+sample_count];

            sym_dst_i[sample_count] = sym_fi[(para->ofdma_nfft-N_2)+sample_count];

        }

        for (sample_count=N_2; sample_count<=para->ofdma_nused_no_dc-1; sample_count++) {

            sym_dst_r[sample_count] = sym_fr[sample_count - (N_2-1)];

            sym_dst_i[sample_count] = sym_fi[sample_count - (N_2-1)];

        }

    }



    return SUCCESS_CODE;

}

#endif



/* restore _DUMP_UTIL_ENABLE_ setting */
#ifndef DUMP_PHY_UL_OFDM_DEMOD

#ifdef LOCAL_DUMP_ENABLE
#define _DUMP_UTIL_ENABLE_
#endif

#undef LOCAL_DUMP_ENABLE
#endif


