#include "phy_dl_interleaver.h"
#include "phy_dl_tx.h"
#include <math.h>
#include <stdlib.h>
#include <stdio.h>
static struct interleave_setting g_interleave_setting;

int phy_dl_init_interleave_setting()
{
    int mk; // the index after the first and before the second permutation as defined in 8.4.9.3
    int jk; // the index after the second permutation as defined in 8.4.9.3
    int d = 16;  // the modulo used for permutation as defined in 8.4.9.3
    int table_id[]={48,96,144,192,240,288,336,384,432,480,528,576};
    int ii = 0;
    int jj = 0;
    int offset = 0;
    int s = 1;
    g_interleave_setting.map_count = 12;
    for(ii = 0; ii< g_interleave_setting.map_count; ii++)
    {
        g_interleave_setting.table_index[ii] = table_id[ii];
        g_interleave_setting.table_offset[ii] = offset;
        for(jj = 0; jj < table_id[ii]; jj++)
        {
             mk= ((table_id[ii]/d) * (jj%d)+ floor(jj/d));
             jk= s * floor(mk/s) + ((int)(mk+table_id[ii] - floor(d*mk/(table_id[ii]))))%s;
             g_interleave_setting.table_jk[offset + jj] = jk;
             
        }
        offset += table_id[ii];
    }

    s = 2;
    for(ii = 0; ii< g_interleave_setting.map_count; ii++)
    {
        g_interleave_setting.table_index[ii + 12] = table_id[ii];
        g_interleave_setting.table_offset[ii + 12] = offset;
        for(jj = 0; jj < table_id[ii]; jj++)
        {
             mk= ((table_id[ii]/d) * (jj%d)+ floor(jj/d));
             jk= s * floor(mk/s) + ((int)(mk+table_id[ii] - floor(d*mk/(table_id[ii]))))%s;
             g_interleave_setting.table_jk[offset + jj] = jk;
        }
        offset += table_id[ii];
    }
    
    s = 3;
    for(ii = 0; ii< g_interleave_setting.map_count; ii++)
    {
        g_interleave_setting.table_index[ii + 24] = table_id[ii];
        g_interleave_setting.table_offset[ii + 24] = offset;
        for(jj = 0; jj < table_id[ii]; jj++)
        {
             mk= ((table_id[ii]/d) * (jj%d)+ floor(jj/d));
             jk= s * floor(mk/s) + ((int)(mk+table_id[ii] - floor(d*mk/(table_id[ii]))))%s;
             g_interleave_setting.table_jk[offset + jj] = jk;
        }
        offset += table_id[ii];
    }
    return 0;
}

int get_interleave_table_offset(int ncbps,int s)
{
    int ii = 0; 
    for(ii = 0; ii < g_interleave_setting.map_count; ii ++)
        if(g_interleave_setting.table_index[ii] == ncbps)
        {
            return g_interleave_setting.table_offset[ii + (s - 1) * g_interleave_setting.map_count];
        }

    printf("error in search interleave_table\n");
    return -1;
}

int phy_dl_interleaver(const struct phy_dl_fec_para *para,
                       unsigned char *interleave_input,
                       unsigned char *interleave_output)
{
    int i = 0;
    int k = 0;
    
    float code_rate = (float)para->burst_len_punctured/(float)para->bits_slotsymbol;
    int s=para->ncpc/2;
    int offset = 0;
    int *p_offset = NULL;
    int Ncbps0 = ceil(para->blocksizejslot * code_rate);
    unsigned char *p_in = interleave_input;
    unsigned char *p_out = interleave_output;
    if(Ncbps0 != 0)
    {
        offset = get_interleave_table_offset(Ncbps0,s);
        p_offset = g_interleave_setting.table_jk + offset;
        int k0 = para->blockjslot;
        for ( i =0; i<k0; i++){
            for(k=0;k<Ncbps0;k++){
                p_out[p_offset[k]] = p_in[k];
            }
            p_in += Ncbps0;
            p_out += Ncbps0;
        }
    }
    
    int Ncbps1 = ceil(para->blocksizeceilslot * code_rate);
    if(Ncbps1 != 0)
    {
        offset = get_interleave_table_offset(Ncbps1,s);
        p_offset = g_interleave_setting.table_jk + offset;
        for(k=0;k<Ncbps1;k++){
            p_out[p_offset[k]] = p_in[k];
        }
        p_out += Ncbps1;
        p_in += Ncbps1;
        
    }

    int Ncbps2 = ceil(para->blocksizefloorslot * code_rate);
    if(Ncbps2 != 0)
    {
        offset = get_interleave_table_offset(Ncbps2,s);
        p_offset = g_interleave_setting.table_jk + offset;
        for(k=0;k<Ncbps2;k++){
            p_out[p_offset[k]] = p_in[k];
        }
        p_in += Ncbps2;
        p_out += Ncbps2;
    }
    
    return 0;
}
