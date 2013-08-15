/* ----------------------------------------------------------------------------
 * IBM Confidential

 IBM Wireless M2M Platform

 (C)Copyright IBM Corp. 2009, 2010, 2011

 All Rights Reserved.

 File Name: adapter_test_stube.c

 Change Activity:

 Date             Description of Change                            By
 -----------      ---------------------                            --------
 22-Apr 2011       Created                                         jian jun chang 

 ----------------------------------------------------------------------------
 PROLOG END TAG zYx                                                         */
#include <string.h>
#include <stdlib.h>
#include "adapter_test_stube.h"

#define PAYLOADDATANUM 10


#define CREATELINKNODE(phead,pcurrentnode,pnextnode)             if(phead == NULL)\
                                                                 {\
                                                                     phead = pnextnode;\
                                                                 }\
                                                                 if(pcurrentnode != NULL)\
                                                                    pcurrentnode->next = pnextnode;\
                                                                 pcurrentnode = pnextnode;
void setdcdmsgdata(dcd_msg * dcdmsg)
{
    memset(dcdmsg,0,sizeof(dcd_msg));
   
    dl_burst_profile*   pnode = NULL;
    int i = 0;

    //dcdmsg->profile_header = (struct dl_burst_profile*)malloc(sizeof(dl_burst_profile));
    for(; i < PAYLOADDATANUM; i++)
    {
        dl_burst_profile *dlburstnode = (dl_burst_profile *)malloc(sizeof(dl_burst_profile));
        dlburstnode->next = NULL;
        dlburstnode->diuc = i;
        dlburstnode->fec_code_modulation.value = OFDM_16QAM_1_2;

        if(dcdmsg->profile_header == NULL)
        {
            dcdmsg->profile_header = dlburstnode;
        }
        if(pnode != NULL)
           pnode->next = dlburstnode;
        pnode = dlburstnode;
        //CREATELINKNODE(dcdmsg->profile_header,pnode,dlburstnode);
     }
}

void  freedcdmsgdata(dcd_msg **dcdmsg)
{
    if(*dcdmsg == NULL)
        return;
    dl_burst_profile *p_burstnode = (*dcdmsg)->profile_header, *p_nextburstnode = NULL;
    while(p_burstnode != NULL)
    {
        p_nextburstnode = p_burstnode->next;
        free(p_burstnode);
        p_burstnode = p_nextburstnode;
    }
    free(*dcdmsg);
    *dcdmsg = NULL;
}


int set_dcd_msg_stube(dcd_msg* dcdmsg)
{  
    if (dcd_stube)
    {
        free(dcd_stube);
    }
    dcd_stube = dcdmsg;
    return 0;
}
int set_ucd_msg_stube(ucd_msg* ucdmsg)
{
    if (ucd_stube)
    {
        free(ucd_stube);
    }
    ucd_stube = ucdmsg;
    return 0;
}
int set_dl_map_msg_stube(dl_map_msg* dlmap)
{
  // the memory is freed by scheduler (free_maps.c)
/*     if (dl_map) */
/*     { */
/*         free(dl_map); */
/*     } */
    dl_map_stube = dlmap;
    return 0;
}
int set_ul_map_msg_stube(ul_map_msg* ulmap)
{
  // the memory is freed by scheduler (free_maps.c)
/*     if (ul_map) */
/*     { */
/*         free(ul_map); */
/*     } */
    ul_map_stube = ulmap;
    return 0;
}

int get_dcd_msg_stube(dcd_msg** dcdmsg)
{
    (*dcdmsg) = dcd_stube;
    return 0;
}
int get_ucd_msg_stube(ucd_msg** ucdmsg)
{
    (*ucdmsg) = ucd_stube;
    return 0;
}
int get_dl_map_msg_stube(dl_map_msg** dlmap)
{
    (*dlmap) = dl_map_stube;
    return 0;
}
int get_ul_map_msg_stube(ul_map_msg** ulmap)
{
    (*ulmap) = ul_map_stube;
    return 0;
}

