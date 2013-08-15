/* ----------------------------------------------------------------------------
 * IBM Confidential

 IBM Wireless M2M Platform

 (C)Copyright IBM Corp. 2009, 2010, 2011

 All Rights Reserved.

 File Name: stube_mac.c

 Change Activity:

 Date             Description of Change                            By
 -----------      ---------------------                            --------
 23-JUL 2011       Created                                         jian jun chang 

 ----------------------------------------------------------------------------
 PROLOG END TAG zYx                                                         */
#include "stube_mac.h"
#include "mac_headermsg_builder.h"
#include "adapter_ul_stube_test.h"
#include "bs_cfg.h"
#include "prephy_proc.h"


#define CREATELINKNODE(phead,pcurrentnode,pnextnode)             if(phead == NULL)\
                                                                 {\
                                                                     phead = pnextnode;\
                                                                 }\
                                                                 if(pcurrentnode != NULL)\
                                                                    pcurrentnode->next = pnextnode;\
                                                                 pcurrentnode = pnextnode;


enum  data_attri{
  SYM_OFFSET  = 0,
  SUBCH_OFFSET,
  SYM_NUM,
  SUBCH_NUM,
  CID,
  IE_INDEX,
  DIUC
};

unsigned char fch_dl_map_buf[232] = 
{
0x0,0x0,0xd0,0x0,0x0,0x50,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,
0x0,0x40,0x37,0x32,0x8a,0x0,0x2,0x4,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x1,0xa,0xf4,0x0,0x1,0x0,0x66,0x4,0x0,0x0,0x80,0x20,0x10,0x6,0x70,0x70,0x0,0x1,0x40,0x1,0x0,0x68,0x7,0x6,0x0,0x3c,0x40,0x10,0x6,0x90,0x71,0x50,0x5,0x80,0x22,0x99,0x53,0x7c,
0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,
0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,
0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,
0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,
0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,
0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,
0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,
};
const int subch_sym[][7]= 
  {{4,0,3,32,102,0,0},{7,0,3,5,103,1,2},{7,6,3,15,104,2,0},{7,21,3,22,105,3,4}};
int  generate_physubframe(physical_subframe **mac_frame)
{
    
    phy_burst   *pnode = NULL;
    int i = 0, ulmap_len;
    ul_map_msg *p_ul_map_stube = NULL;
    void *p_ulmap = malloc (4096);
    struct fake_ul_map st_fmap;
    int fake_ulmap_uiuc = 0;
    int fake_ulmap_duration = 0;
    int ret;

#ifdef _BER_TEST_

    st_fmap.ie.duration = g_fake_ulmap_duration;
    st_fmap.ie.uiuc = g_fake_ulmap_uiuc;
//    printf("st_fmap.ie.uiuc = %d\n", st_fmap.ie.uiuc);

#else
    ret = get_global_param ("FAKE_ULMAP_DURATION", &fake_ulmap_duration);
    if (ret != 0)
    {
        FLOG_WARNING ("get parameters FAKE_ULMAP_DURATION error\n");
    }

    ret = get_global_param ("FAKE_ULMAP_UIUC", &fake_ulmap_uiuc);
    if (ret != 0)
    {
        FLOG_WARNING ("get parameters FAKE_ULMAP_UIUC error\n");
    }
    st_fmap.ie.duration = fake_ulmap_duration;
    st_fmap.ie.uiuc = fake_ulmap_uiuc;
#endif


    st_fmap.ss_num = 1;
    st_fmap.ie.bcid = 11;
    st_fmap.ie.next = NULL;

    p_ul_map_stube = (ul_map_msg*) malloc (sizeof(ul_map_msg));
    memset (p_ul_map_stube, 0, sizeof(ul_map_msg));
    adapter_build_ul_map (p_ul_map_stube, &st_fmap);
    memset (p_ulmap, 0, 4096);

    build_ul_map_msg (p_ul_map_stube, p_ulmap, (unsigned int *)&ulmap_len);

    *mac_frame = malloc(sizeof(physical_subframe));
    memset(*mac_frame,0,sizeof(physical_subframe));
    (*mac_frame)->burst_header = NULL;
    (*mac_frame)->fch_dl_map = malloc(232);
    memcpy((*mac_frame)->fch_dl_map,fch_dl_map_buf,232);
    (*mac_frame)->fch_dl_map_len = 232; 
    (*mac_frame)->raw_ul_map = p_ulmap;
    (*mac_frame)->raw_ul_map_len = ulmap_len;
    
    phy_burst *phynode = NULL;
    
#if 1
    phynode = (phy_burst*)malloc(sizeof(phy_burst));
    memset(phynode,0,sizeof(phy_burst));
    
    phynode->length =192;
    phynode->burst_payload = (u_int8_t*)malloc(sizeof(u_int8_t)*192);

#ifdef  _DUMP_SRC_
    FILE *fp;
    fp = fopen("bin.dat", "w+");
    if(fp == NULL)
    {
        printf("open dump file failed \n");
    }
#endif 
    /* initialize random seed: */
    srand ( time(NULL) );

  /* generate random number: */
//    int iSecret = rand() % 255 + 1;

    for(i = 0; i < 192; i++)
    {
       
        phynode->burst_payload[i] = 0xFF;
    }

#ifdef  _DUMP_SRC_
        fwrite(phynode->burst_payload,sizeof(uint8_t),192,fp);
#endif

    (*mac_frame)->burst_header =  phynode;
    phynode->map_burst_index = 0;
    pnode = phynode;
    phynode->next = NULL;
//4  data burst

    phynode = (phy_burst*)malloc(sizeof(phy_burst));
    memset(phynode,0,sizeof(phy_burst));
    
    phynode->length = 60;
    phynode->burst_payload = (u_int8_t*)malloc(sizeof(u_int8_t)*60);

    
    

    for(i = 0; i < 60; i++)
    {
        phynode->burst_payload[i] = 0xFF;
    }
    
#ifdef  _DUMP_SRC_
        fwrite(phynode->burst_payload,sizeof(uint8_t),60,fp);
#endif

    phynode->map_burst_index = 1;
    pnode->next = phynode;
    pnode = phynode;
    phynode->next = NULL;

//5  data burst

    phynode = (phy_burst*)malloc(sizeof(phy_burst));
    memset(phynode,0,sizeof(phy_burst));
    
    phynode->length = 90;
    phynode->burst_payload = (u_int8_t*)malloc(sizeof(u_int8_t)*90);

    
    

    for(i = 0; i < 90; i++)
    {
        phynode->burst_payload[i] = 0xFF;
    }
    
#ifdef  _DUMP_SRC_
        fwrite(phynode->burst_payload,sizeof(uint8_t),90,fp);
#endif

    phynode->map_burst_index = 1;
    pnode->next = phynode;
    pnode = phynode;
    phynode->next = NULL;

    (*mac_frame)->bursts_num = 3;

    phynode = (phy_burst*)malloc(sizeof(phy_burst));
    memset(phynode,0,sizeof(phy_burst));
    
    phynode->length = 396;
    phynode->burst_payload = (u_int8_t*)malloc(sizeof(u_int8_t)*396);

    
    

    for(i = 0; i < 396; i++)
    {
        phynode->burst_payload[i] = 0xFF;
    }
    
#ifdef  _DUMP_SRC_
        fwrite(phynode->burst_payload,sizeof(uint8_t),396,fp);
#endif

    phynode->map_burst_index = 1;
    pnode->next = phynode;
    pnode = phynode;
    phynode->next = NULL;

    (*mac_frame)->bursts_num = 4;
#ifdef  _DUMP_SRC_
    fclose(fp);
#endif
#endif
    
    return 0;
}

int  generate_dlmap(dl_map_msg **pp_dlmap)
{
//    static int frame_number = 0;
    //read_data_from_file("./dl_map.dump",dl_map);
    dl_map_ie  *ie_node = NULL;
    //dl_map->ie_head = (struct dl_map_ie*)malloc(sizeof(dl_map_ie));
    dl_map_ie *p_node = NULL;
    int i = 0;
    *pp_dlmap = malloc(sizeof(dl_map_msg));
    memset(*pp_dlmap,0,sizeof(dl_map_msg));
    (*pp_dlmap)->manage_msg_type = 2;
    (*pp_dlmap)->frame_duration_code = 4;
    (*pp_dlmap)->frame_number = 0 ;
    (*pp_dlmap)->dcd_count = 0;
    (*pp_dlmap)->bs_id = 1;
    (*pp_dlmap)->ofdma_symbols_num = 10;
 
    
    ie_node =  (dl_map_ie*) malloc(sizeof(dl_map_ie));
    memset(ie_node,0,sizeof(dl_map_ie));
    ie_node->ie_index = 0;
    ie_node->diuc = 15;
    ie_node->extended_ie = malloc(sizeof(extended_diuc_ie));
    memset(ie_node->extended_ie,0,sizeof(extended_diuc_ie));
    ie_node->extended_ie->extended_diuc = 4;
    ie_node->extended_ie->length = 0;
    (*pp_dlmap)->ie_head = ie_node;
    p_node = ie_node;
    for(; i < 4; i++)
    {
        ie_node = (dl_map_ie*) malloc(sizeof(dl_map_ie));
        memset(ie_node,0,sizeof(dl_map_ie));
        ie_node->ie_index = subch_sym[i][IE_INDEX];
        ie_node->diuc = subch_sym[i][DIUC];
        ie_node->normal_ie = (normal_diuc_ie*)malloc(sizeof(normal_diuc_ie));
        {
            ie_node->normal_ie->ofdma_symbol_offset = subch_sym[i][SYM_OFFSET];
            ie_node->normal_ie->subchannel_offset = subch_sym[i][SUBCH_OFFSET];
            ie_node->normal_ie->ofdma_Symbols_num = subch_sym[i][SYM_NUM];
            ie_node->normal_ie->subchannels_num = subch_sym[i][SUBCH_NUM];
            ie_node->normal_ie->cid = (u_int16_t*) malloc(sizeof(u_int16_t));;
            *ie_node->normal_ie->cid = subch_sym[i][CID];
            ie_node->normal_ie->n_cid = 1;
        }
        if(p_node != NULL)
        {
            p_node->next = ie_node;
            p_node = ie_node;
        }
        else
        {
            (*pp_dlmap)->ie_head = ie_node;
            p_node = ie_node;
        }
    }
    return 0;
}
