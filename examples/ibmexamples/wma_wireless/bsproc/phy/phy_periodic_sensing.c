/* ----------------------------------------------------------------------------
   IBM Confidential

   IBM Wireless M2M Platform

   (C)Copyright IBM Corp. 2009, 2010, 2011

   All Rights Reserved.

   File Name: phy_perodic_sensini.c

   Function: Peroidical Sensing at Rx 

   Change Activity:

   Date             Description of Change                            By
   -----------      -------------------------------                  --------

   ----------------------------------------------------------------------------
   PROLOG END TAG zYx                                                         */

#include <memory.h>
#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include <string.h>

#include <pthread.h>

#include "flog.h"
#include "phy_ul_rx_common.h"
#include "phy_periodic_sensing.h"
#include "phy_proc.h"
#include "prephy_proc.h"
#include "clip.h"

#ifdef IPP_OPT_TEMP730
  #include "phy_ul_ipp_para.h"
#else
  #include "fft.h"
#endif

#define PS_IMPROVE
#define NOISE_FIGURE
#define NOISE_MAXHOLD

#ifdef NOISE_MAXHOLD
#define SPECFACTOR (28.7852)
#endif
//#define SMOOTH_FILTER

#define ALPHA (0.95)

struct phy_ps_hdr * gs_phy_ps_history = NULL;

int32_t spectrum_per_scan(struct phy_ul_rx_syspara *para,
                          float *input_r,
                          float *input_i,
                          float *power_output)
{
    float fftout_r[PS_NFFT];
    float fftout_i[PS_NFFT];
    float pwr_temp[PS_NFFT];
    float *sym_src_r, *sym_src_i;
    unsigned int m, n;
   
    n = 0;
    memset(pwr_temp, 0, sizeof(float)*PS_NFFT);

    for (n=0; n<para->symbol_per_slot; n++)
    {
        sym_src_r = input_r + n * para->ofdma_sym_len_with_guard;
        sym_src_i = input_i + n * para->ofdma_sym_len_with_guard;


	/*FFT and abs*/
#ifdef IPP_OPT_TEMP730	
        ippsFFTFwd_CToC_32f(sym_src_r, sym_src_i, fftout_r, fftout_i, pFFTSpecFwd_ul, BufFwd_ul);
#else	
        fft( PS_NFFT, sym_src_r, sym_src_i, fftout_r, fftout_i, para->XX, para->x, para->X );
#endif
        for ( m = 0; m < PS_NFFT; m++ )
        {   
          pwr_temp[m] = pwr_temp[m] + (fftout_r[m] * fftout_r[m] + fftout_i[m] * fftout_i[m])/PS_NFFT;
        }
    } 
    
    memcpy(power_output, pwr_temp, sizeof(float)*PS_NFFT);    


    return SUCCESS_CODE;
}


int32_t per_dts_update(struct phy_ul_rx_syspara *para,
                       float *input,
                       struct phy_dts_info *dtspara)
{

    float threshold, point_per_subch;
    int N;
    int m,k,k_start,k_end;
    float temp_power[PS_NFFT];
    char channel[42];
    int dl_unused_subch=0;
    int ul_used_subch=0;
    int ul_active_subch=0;
    int ul_subch_num = 0;
    float sum_pwr = 0;
    int i;

    struct phy_ps_node * p_ps_node = NULL;
    struct phy_ps_node * p_ps_history = NULL;

    float temp_f, spec_max;

    int band_count = 0;
    float noise = 0.0F;
    float noise_subcar = 0.0F;

    float temp_intf[21] = {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};
    char active_band[21] = {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};

    if (g_periodic_sensing_drop == 0)
    {

        p_ps_node = phy_ps_get_current();

        if (p_ps_node == NULL)
        {
           FLOG_ERROR("PS Node failed");
            return 1;
        }

        memset(temp_power, 0, sizeof(float)*PS_NFFT);
 
        /*DC offset removal*/
        if (input[0]/input[3]>10)
        {
	    input[0]=input[3];
        }	

   
        N= para->ofdma_nused_no_dc/2;

#ifdef SMOOTH_FILTER
        p_ps_history = phy_ps_get_history(-1);

        if (p_ps_history == NULL)
        {
            memcpy(p_ps_node->avg_pwr, input, sizeof (float) * PS_NFFT);
        }
        else
        {
            for (i=0; i<PS_NFFT; i++)
            {
                p_ps_node->avg_pwr[i] = p_ps_history->avg_pwr[i]*ALPHA+(1-ALPHA)* input[i];
            }
        }
#else
        memcpy(p_ps_node->avg_pwr, input, sizeof(float)* PS_NFFT);
#endif


#ifdef PS_IMPROVE
        for ( m=0; m < N; m++ )
        {
            temp_power[ N - 1 - m] = p_ps_node->avg_pwr[PS_NFFT-1-para->right_guard - m] ;
            temp_power[ N + m] = p_ps_node->avg_pwr[para->left_guard+m] ;
        }
#else
        for ( m=0; m < N; m++ )
        {
	    temp_power[ N - 1 - m] = p_ps_node->avg_pwr[PS_NFFT - 1 - m] ;
            temp_power[ N + m] = p_ps_node->avg_pwr[m] ;
        }
#endif

#ifdef NOISE_MAXHOLD
        spec_max = getmax(temp_power, para->ofdma_nused_no_dc);
        para->noise_maxhold = para->CaliAnaPwr0 - 10*log10f(para->CaliDigiPwr0/spec_max)
                              -SPECFACTOR;
     //   FLOG_INFO("noise maxhold = %f\n", para->noise_maxhold);

#endif
        for (m=0; m<2*N; m++)
        {
            sum_pwr +=temp_power[m];
        }
    
        threshold = sum_pwr/(2*N) * para->ps_thd;
	
        /* interference */
        point_per_subch = N*2/((float)NSUBCHAN);

        for (m=0; m<NSUBCHAN; m++)
        {	
            k_start = floor(point_per_subch * m);
	    k_end = floor(point_per_subch * (m+1));
	    temp_intf[m]=0;
	    for (k=k_start; k<k_end ; k++)
            {
	        temp_intf[m] += temp_power[k];
            }

            temp_f = temp_intf[m]/point_per_subch;
	    active_band[m] = temp_f>threshold? 1:0;

#ifdef NOISE_FIGURE
            if (active_band[m] == 0)
            {
                noise = noise + temp_intf[m];
                band_count ++;
            }
#endif
        }

#ifdef NOISE_FIGURE
        noise_subcar = noise/((float)band_count*point_per_subch);
      //FLOG_INFO("para->dgain0= %f\n", para->dgain0);
        p_ps_node->noise_figure = para->CaliAnaPwr0 - 10*log10f(para->CaliDigiPwr0/noise_subcar);
                                 // -para->dgain0; 
        para->noise_figure = p_ps_node->noise_figure;
#endif

        phy_ps_update();
    }

    for (i = 0; i < 21; i++)
    {
        active_band[i] = active_band[i] | para->fixed_active_band[i];
//        FLOG_INFO("%d", active_band[i]);
    }

	/*update dts*/
    memcpy(dtspara->active_band, active_band, NSUBCHAN*sizeof(int8_t));
    /*dl ul unused subchan*/
    /*dl unused subchan = fch_forbidden + active */
    for (m=0; m<NSUBCHAN; m++)
    {	
        if(active_band[m]==1)
	{	
            channel[2*m]=1;
	    channel[2*m+1]=1;
	    ul_active_subch+=1;
	}
	else
	{
            channel[2*m]=0;
	    channel[2*m+1]=0;
	}
     }		
    for (m=0;m<3;m++)
    {	
        if(para->fch_bitmap[m*2]==0)
        {
	    for(k=0;k<12;k++)
            {     
	        channel[m*14+k]=1;
            }
        }
	if(para->fch_bitmap[m*2+1]==0)		
        {
            for(k=12;k<14;k++)
	    {
		channel[m*14+k]=1;
            }			
	}
    }

    for (m=0;m<42;m++)
    {	
        if(channel[m]==1)
	{
            dl_unused_subch+=1;
        }
    }		

    dtspara->dl_unused_subch = dl_unused_subch;
	
/*ul unused subchan =  active + ul_bitmap*/	
    ul_subch_num = floor((756-36*ul_active_subch)/(6*4)); 
    for (m=0;m<ul_subch_num;m++)
    {	
        if(para->ul_bitmap[m]==1)
        {
	    ul_used_subch+=1;
        }
    }

    dtspara->ul_unused_subch = para->num_subch - ul_used_subch;

    /* update the dtsinfo in system parameters */
    para->ul_unused_subch = dtspara->ul_unused_subch;
    memcpy(para->active_band, dtspara->active_band, sizeof(char)*21);
    

#ifdef PS_FAKE
     /* fake version--fixed input */
    memset(active_band, 0, sizeof(int8_t)*21);
    memcpy(dtspara->active_band, active_band, 21*sizeof(int8_t));

    dtspara->dl_unused_subch = 0;
    dtspara->ul_unused_subch = 0;

#endif

    (void) p_ps_history;

    return SUCCESS_CODE;
}

int phy_ps_init_history()
{
    int i = 0;
    struct phy_ps_node * node = NULL;

    gs_phy_ps_history = (struct phy_ps_hdr *) malloc (sizeof (struct phy_ps_hdr));

    if (gs_phy_ps_history == NULL)
    {
        FLOG_ERROR("NULL PTR of malloc for gs_phy_ps_history");
        return 1;
    }

    node = (struct phy_ps_node *)malloc(sizeof (struct phy_ps_node));

    if (node == NULL)
    {
        FLOG_ERROR("NULL PTR of malloc for node");
        free (gs_phy_ps_history);
        gs_phy_ps_history = NULL;
        return 1;
    }

    memset(node, 0, sizeof (struct phy_ps_node));

    node->avg_pwr = (float *)malloc(PS_NFFT * sizeof(float));

    if (node->avg_pwr == NULL)
    {
        FLOG_ERROR("NULL PTR of malloc for node power");
        free (gs_phy_ps_history);
        gs_phy_ps_history = NULL;
        return 1;
    }

    pthread_mutex_init(&(gs_phy_ps_history->mutex), NULL);

    gs_phy_ps_history->next = node;

    for (i = 0; i < AVG_POWER_HISTORY; i++)
    {
        node->next = (struct phy_ps_node *)malloc(sizeof (struct phy_ps_node));

        if (node->next == NULL)
        {
            FLOG_ERROR("NULL PTR of malloc for node");
            free (gs_phy_ps_history);
            gs_phy_ps_history = NULL;
            return 1;
        }

        memset(node->next, 0, sizeof (struct phy_ps_node));

        node->next->avg_pwr = (float *)malloc(PS_NFFT * sizeof(float));

        if (node->next->avg_pwr == NULL)
        {
            FLOG_ERROR("NULL PTR of malloc for node power");
            free (gs_phy_ps_history);
            gs_phy_ps_history = NULL;
            return 1;
        }

        node = node->next;
    }

    return 0;
}

int phy_ps_deinit_history()
{
    struct phy_ps_node * node = NULL;

    if (gs_phy_ps_history == NULL)
    {
        FLOG_ERROR("PS_HISTORY did not initiallized\n");
    }

    pthread_mutex_lock(&(gs_phy_ps_history->mutex));

    node = gs_phy_ps_history->next;

    while(gs_phy_ps_history->next != NULL)
    {
        node = gs_phy_ps_history->next;
        gs_phy_ps_history->next = node->next;
        free(node);
    }

    pthread_mutex_unlock(&(gs_phy_ps_history->mutex));
    pthread_mutex_destroy(&(gs_phy_ps_history->mutex));

    free (gs_phy_ps_history);
    gs_phy_ps_history = NULL;

    return 0;
}

struct phy_ps_node * phy_ps_get_current()
{
    if (gs_phy_ps_history == NULL)
    {
        FLOG_ERROR("history not initiallized");
        return NULL;
    }

    if (gs_phy_ps_history->next == NULL)
    {
        FLOG_ERROR("history not initiallized");
        return NULL;
    }

    return (gs_phy_ps_history->next);
}

int phy_ps_update()
{
    struct phy_ps_node * node = NULL;
    struct phy_ps_node * tmp_node = NULL;

    if (gs_phy_ps_history == NULL)
    {
        FLOG_ERROR("history not initiallized");
        return 1;
    }

    if (gs_phy_ps_history->next == NULL)
    {
        FLOG_ERROR("history not initiallized");
        return 1;
    }

    if (gs_phy_ps_history->next->next == NULL)
    {
        FLOG_ERROR("history not exist");
        return 1;
    }

    node = gs_phy_ps_history->next;
    while(node->next != NULL)
    {
        node = node->next;
    }

    tmp_node = gs_phy_ps_history->next;

    gs_phy_ps_history->next = tmp_node->next;

    tmp_node->next = NULL;

    node->next = tmp_node;

    gs_phy_ps_history->next->noise_figure = tmp_node->noise_figure;
    gs_phy_ps_history->next->expected_pwr = 0.0F;

    return 0;
}

struct phy_ps_node * phy_ps_get_history(int index)
{
    int abs_idx = 0;
    struct phy_ps_node * p_node = NULL;
    int i = 0;

    if (index >= 0)
    {
        FLOG_WARNING("wrong index %d\n");
        return NULL;
    }

    abs_idx = abs(index);

    if (abs_idx > AVG_POWER_HISTORY)
    {
        FLOG_WARNING("exceed history: -1 ~ -%d\n", AVG_POWER_HISTORY);
        return NULL;
    }

    if (gs_phy_ps_history == NULL)
    {
        FLOG_ERROR("history not initiallized");
        return NULL;
    }

    if (gs_phy_ps_history->next == NULL)
    {
        FLOG_ERROR("history not initiallized");
        return NULL;
    }

    if (gs_phy_ps_history->next->next == NULL)
    {
        FLOG_ERROR("history not exist");
        return NULL;
    }

    p_node = gs_phy_ps_history->next;

    for (i = 0; i < AVG_POWER_HISTORY - abs_idx + 1; i ++)
    {
        p_node = p_node->next;
    }

    return (p_node);
}


int update_active_back(int type, int len, void * p_buf)
{
    char tmp_string[128];
    int i = 0;

    strcpy(tmp_string, (char *)p_buf);

    tmp_string[21] = 0;

    for (i = 20; i >= 0; i--)
    {
        phy_ul_para->fixed_active_band[i] = atoi (&(tmp_string[i]));
        tmp_string[i] = 0;
    }

    g_periodic_sensing_enable = 1;
    g_periodic_sensing_reset = 1;

    (void) type;
    (void) len;

    return 0;
}

