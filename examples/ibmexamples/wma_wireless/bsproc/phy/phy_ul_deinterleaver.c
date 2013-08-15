/* ----------------------------------------------------------------------------

    IBM Confidential

   IBM Wireless M2M Platform

   (C)Copyright IBM Corp. 2009, 2010, 2011

   All Rights Reserved.

   File Name: phy_ul_deinterleaver.c

   Function:  DeInterleaver in the FEC decoding 

   Change Activity:


   Date             Description of Change                            By

   -----------      ---------------------                            --------


   ----------------------------------------------------------------------------

   PROLOG END TAG zYx                                                         */

   

#include <math.h>

#include <stdlib.h>

#include <stdio.h>

#include "phy_ul_rx_interface.h"
#include "phy_ul_fec_decoding.h"

#include "phy_ul_deinterleaver.h"

static struct deinterleave_setting g_deinterleave_setting;

int phy_ul_init_deinterleave_setting()
{
    int mj; // the index after the first and before the second permutation as defined in 8.4.9.3
    int kj; // the index after the second permutation as defined in 8.4.9.3
    int d = 16;  // the modulo used for permutation as defined in 8.4.9.3
    int table_id[]={48,96,144,192,240,288,336,384,432,480,528,576};
    int ii = 0;
    int jj = 0;
    int offset = 0;
    int s = 1;
    for(ii = 0; ii< 12; ii++)
    {
        g_deinterleave_setting.table_index[ii] = table_id[ii];
        g_deinterleave_setting.table_offset[ii] = offset;
        for(jj = 0; jj < table_id[ii]; jj++)
        {
             mj= s * floor(jj/s) + ((int)(jj+floor(d*jj/table_id[ii])))%s;
             kj= d*mj- (table_id[ii]-1)*floor(d*mj/table_id[ii]);
             g_deinterleave_setting.table_kj[offset + jj] = kj;
             
        }
        offset += table_id[ii];
    }

    s = 2;
    for(ii = 0; ii< 12; ii++)
    {
        g_deinterleave_setting.table_index[ii + 12] = table_id[ii];
        g_deinterleave_setting.table_offset[ii + 12] = offset;
        for(jj = 0; jj < table_id[ii]; jj++)
        {
             mj= s * floor(jj/s) + ((int)(jj+floor(d*jj/table_id[ii])))%s;
             kj= d*mj- (table_id[ii]-1)*floor(d*mj/table_id[ii]);
             g_deinterleave_setting.table_kj[offset + jj] = kj;
        }
        offset += table_id[ii];
    }
    
    s = 3;
    for(ii = 0; ii< 12; ii++)
    {
        g_deinterleave_setting.table_index[ii + 24] = table_id[ii];
        g_deinterleave_setting.table_offset[ii + 24] = offset;
        for(jj = 0; jj < table_id[ii]; jj++)
        {
             mj= s * floor(jj/s) + ((int)(jj+floor(d*jj/table_id[ii])))%s;
             kj= d*mj- (table_id[ii]-1)*floor(d*mj/table_id[ii]);
             g_deinterleave_setting.table_kj[offset + jj] = kj;
        }
        offset += table_id[ii];
    }
    g_deinterleave_setting.map_count = 12;
    return 0;
}

//DeInterleaver

int get_deinterleave_table_offset(int ncbps,int s)
{
    int ii = 0;
    for(ii = 0; ii < g_deinterleave_setting.map_count; ii ++)
        if(g_deinterleave_setting.table_index[ii] == ncbps)
        {
            return g_deinterleave_setting.table_offset[ii + (s - 1) * g_deinterleave_setting.map_count];
        }

    printf("error in search deinterleave_table\n");
    return -1;
}


int32_t phy_ul_deinterleaver( struct  phy_ul_fec_para *para, 

			      float *interleave_output, 

			      float *deinterleave_output)

{
    int i = 0;
    int k = 0;
    
    float code_rate = (float)para->burst_len_repeated/(float)para->bits_slotsymbol;
    int s=para->ncpc/2;
    int offset = 0;
    int *p_offset = NULL;
    int Ncbps0 = ceil(para->blocksizejslot * code_rate);
    float *p_in = interleave_output;
    float *p_out = deinterleave_output;
    if(Ncbps0 != 0)
    {
        offset = get_deinterleave_table_offset(Ncbps0,s);
        p_offset = g_deinterleave_setting.table_kj + offset;
        int k0 = para->blockjslot;
        for ( i =0; i<k0; i++){
            for(k=0;k<Ncbps0;k++){
                p_out[p_offset[k]] = p_in[k];
            }
            p_out += Ncbps0;
            p_in += Ncbps0;
        }
    }
    
    int Ncbps1 = ceil(para->blocksizeceilslot * code_rate);
    if(Ncbps1 != 0)
    {
        offset = get_deinterleave_table_offset(Ncbps1,s);
        p_offset = g_deinterleave_setting.table_kj + offset;
        for(k=0;k<Ncbps1;k++){
            p_out[p_offset[k]] = p_in[k];
        }
        p_in += Ncbps1;
        p_out += Ncbps1;
    }
    int Ncbps2 = ceil(para->blocksizefloorslot * code_rate);
    if(Ncbps2 != 0)
    {
        offset = get_deinterleave_table_offset(Ncbps2,s);
        p_offset = g_deinterleave_setting.table_kj + offset;
        for(k=0;k<Ncbps2;k++){
            p_out[p_offset[k]] = p_in[k];
        }
        p_out += Ncbps2;
        p_in += Ncbps2;
    }
    return 0;
}



