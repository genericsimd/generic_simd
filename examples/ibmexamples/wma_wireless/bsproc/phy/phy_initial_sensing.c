/* ----------------------------------------------------------------------------
   IBM Confidential

   IBM Wireless M2M Platform   

   (C)Copyright IBM Corp. 2009, 2010, 2011

   All Rights Reserved.

   File Name: phy_initial_sensing.c

   Function: Form a DL frame at transmitter side.

   Change Activity:

   Date             Description of Change                            By
   -----------      -------------------------------                  --------

   ----------------------------------------------------------------------------
   PROLOG END TAG zYx                                                         */

#include <stdlib.h>
#include <stdio.h>
#include <math.h>

#include "flog.h"
#include "initial_sensing_proc.h"
#include "phy_initial_sensing.h"
#include "phy_ul_rx_common.h"


#ifdef IPP_OPT_TEMP730
  #include "phy_ul_ipp_para.h"
#else
  #include "fft.h"
#endif

static float * fftout_r = NULL;
static float * fftout_i = NULL;
static float * is_temp_power = NULL;
#ifndef IPP_OPT_TEMP730
static float **is_x0;
static float **is_x1;
static float **is_x2;
#endif

int init_spectrum_ed_scan(void)
{
    fftout_r = (float *)malloc(IS_NFFT * 8);
    fftout_i = (float *)malloc(IS_NFFT * 8);

    is_temp_power = (float *)malloc(IS_NFFT * 8);
    
    if (fftout_r== NULL || fftout_i == NULL || is_temp_power == NULL ) 
    {
        FLOG_ERROR("malloc failed in init_spectrum_ed_scan\n");
        return ERROR_CODE;
    }
#ifdef IPP_OPT_TEMP730
    ippsFFTInitAlloc_C_32f(&pFFTSpecFwd_ul, (int)(log(IS_NFFT)/log(2)), IPP_FFT_NODIV_BY_ANY, ippAlgHintFast);

    int BufSizeFwd;
    ippsFFTGetBufSize_C_32f(pFFTSpecFwd_ul, &BufSizeFwd);

    BufFwd_ul = malloc(BufSizeFwd*sizeof(Ipp8u));
    if (BufFwd_ul == NULL ) 
    {
        FLOG_ERROR("malloc failed in init_spectrum_ed_scan\n");
        return ERROR_CODE;
    }
#else
    fft_init(IS_NFFT, &is_x0, &is_x1, &is_x2 );
#endif

    return 0;
}

int release_spectrum_ed_scan(void)
{
#ifdef IPP_OPT_TEMP730
    ippsFFTFree_C_32f(pFFTSpecFwd_ul);
    free(BufFwd_ul);
#else
    fft_quit(is_x0, is_x1, is_x2);
#endif

    free(fftout_r);
    free(fftout_i);
    free(is_temp_power);
    return 0;
}

int spectrum_ed_scan( float *input_r,
                      float *input_i,
                      struct spectrum_scan_state *conf)
{
    int m, N;
    float point_per_subch;
    int k,k_start,k_end;

#ifdef IPP_OPT_TEMP730
    ippsFFTFwd_CToC_32f(input_r, input_i, fftout_r, fftout_i, pFFTSpecFwd_ul, BufFwd_ul);
#else
    fft( IS_NFFT, input_r, input_i, fftout_r, fftout_i, is_x0, is_x1, is_x2 );
#endif

    for ( m = 0; m < IS_NFFT; m++ )
    {
    	conf->power[m] = sqrt( fftout_r[m] * fftout_r[m] + fftout_i[m] * fftout_i[m] );
    }	
    /*DC offset removal*/
    if (conf->power[0]/conf->power[3]>10)
    {
	conf->power[0]=conf->power[3];
    }

    /*	POWER in 1.1MHz   806 POINTS	*/
    N= 756/2;
    for ( m=0; m < N; m++ )
    {
	is_temp_power[ N - 1 - m] = conf->power[IS_NFFT - 1 - m] ;
        is_temp_power[ N + m] = conf->power[m] ;
    }
	
    /* interference */
    point_per_subch = N*2/((float)NSUBCHAN);
    for (m=0; m<NSUBCHAN; m++)
    {	
        k_start = floor(point_per_subch * m);
	k_end = floor(point_per_subch * (m+1));
	conf->intf[m]=0;
	for (k=k_start; k<k_end ; k++)
        {
            conf->intf[m] += is_temp_power[k];
        }
        conf->intf[m] /=(k_end-k_start+1);
    }

    return SUCCESS_CODE;

}

