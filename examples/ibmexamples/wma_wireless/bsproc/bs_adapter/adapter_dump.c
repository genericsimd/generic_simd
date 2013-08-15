/* ----------------------------------------------------------------------------
 * IBM Confidential

 IBM Wireless M2M Platform

 (C)Copyright IBM Corp. 2009, 2010, 2011

 All Rights Reserved.

 File Name: adaptr_dump.c

 Change Activity:

 Date             Description of Change                            By
 -----------      ---------------------                            --------
 20-Jan 2011       Created                                         jian jun chang 

 ----------------------------------------------------------------------------
 PROLOG END TAG zYx                                                         */
#include "adapter_dump.h"

FILE* open_file(char *p_file_name)
{
    FILE *fp;
    fp = fopen(p_file_name, "w+");
    if(fp == NULL)
    {
        printf("open dump file failed \n");
        return NULL;
    }
    return fp;
}
int close_file(FILE *fp)
{
    fclose(fp);
    return 0;
}

void   dump_map_ie_node_fp(FILE *fp,  dl_map_ie *p_node)
{
    fprintf(fp, "ie_index = %d\n",p_node->ie_index);
    fprintf(fp, "diuc = %u\n",p_node->diuc);
    if(p_node -> extended_ie != NULL)
    {
        fprintf(fp, "extended_ie->extended_diuc = %u \n" ,p_node -> extended_ie->extended_diuc);
        fprintf(fp, "extended_ie->length = %u \n" ,p_node -> extended_ie->length);
    }
    else
    {
        fprintf(fp, "extended_ie = NULL \n");
    }
    if(p_node -> normal_ie != NULL)
    {
        fprintf(fp, "normal_ie->n_cid = %d \n" ,p_node -> normal_ie->n_cid);
        if(p_node -> normal_ie->cid != NULL)
            fprintf(fp, "normal_ie->cid = %d \n" ,*(p_node -> normal_ie->cid));
        else
            fprintf(fp, "normal_ie->cid = NULL \n");
        fprintf(fp, "normal_ie->ofdma_symbol_offset = %u \n" ,p_node -> normal_ie->ofdma_symbol_offset);
        fprintf(fp, "normal_ie->subchannel_offset = %u \n" ,p_node -> normal_ie->subchannel_offset);
        fprintf(fp, "normal_ie->boosting = %u \n" ,p_node -> normal_ie->boosting);
        fprintf(fp, "normal_ie->ofdma_triple_symbol_num = %u \n" ,p_node -> normal_ie->ofdma_triple_symbol_num);
        fprintf(fp, "normal_ie->ofdma_Symbols_num = %u \n" ,p_node -> normal_ie->ofdma_Symbols_num);
        fprintf(fp, "normal_ie->subchannels_num = %u \n" ,p_node -> normal_ie->subchannels_num);
        fprintf(fp, "normal_ie->repetition_coding_indication = %u \n" ,p_node -> normal_ie->repetition_coding_indication);
    }
    else
    {
        fprintf(fp, "normal_ie = NULL \n" );
    }
    
    
}
void   dump_dlmap_ie_link(dl_map_ie *p_dl_map_ie_head,int itimes,int nums)
{
    FILE    *fp;
    char *cat = "./data/output/dl_map", *suf = ".dump";
    
    char file[256];
    
    strcpy(file, cat);
    strcat(file, suf);

    if(itimes == 0)
    {
        fp = fopen(file, "w+");
        if(fp == NULL)
        {
            printf("open file failed \n");
            return;
        }
    } 
    else
    {
        fp = fopen(file, "ab+");
        if(fp == NULL)
        {
            printf("open file failed \n");
            return;
        }

        fseek(fp,-1,SEEK_END);
    }
    fprintf(fp,"--------------------------the %dth to record--------------------\n",itimes);

    dl_map_ie *p_iter_node = p_dl_map_ie_head;
    int icount = 0;
    while(p_iter_node != NULL)
    {
        dump_map_ie_node_fp(fp,p_iter_node);
        p_iter_node = p_iter_node->next;
        fprintf(fp,"%s","--------------------------------------------------------------------------\n");
        if(icount == nums)
            break;
        icount++;
    }
    fclose(fp);
}

void   dump_burst_node_fp(FILE *fp, phy_burst *p_burst_head)
{
    
    fprintf(fp, "length = %d\n",p_burst_head->length);
    /*fprintf(fp, "burst_payload\n");
    for(;i < p_burst_head->length; i++)
    {
        fprintf(fp,"%u",p_burst_head->burst_payload + i);
        
        if(i % 80 ==0 && i != 0)
            fprintf(fp,"\n");
    }
    fprintf(fp,"\n");*/
    fprintf(fp,"map_burst_index = %d\n",p_burst_head->map_burst_index);    
}
void   dump_burst_link(phy_burst *p_burst_head,int itimes,int nums)
{
    FILE    *fp;
    char *cat = "./data/output/phy_burst", *suf = ".dump";
    char file[256];

    strcpy(file, cat);
    strcat(file, suf);
    if(itimes == 0)
    {
        fp = fopen(file, "w+");
        if(fp == NULL)
        {
            printf("open file failed \n");
            return;
        }
    } 
    else
    {
        fp = fopen(file, "ab+");
        if(fp == NULL)
        {
            printf("open file failed \n");
            return;
        }

        fseek(fp,-1,SEEK_END);
    }
    fprintf(fp,"--------------------------the %dth to record--------------------\n",itimes);

    fprintf(fp,"the burst number is %d\n",nums);
    phy_burst *p_iter_node = p_burst_head;
    int icount = 0;
    while(p_iter_node != NULL)
    {
        dump_burst_node_fp(fp,p_iter_node);
        p_iter_node = p_iter_node->next;
        fprintf(fp,"%s","--------------------------------------------------------------------------\n");
        if(icount == nums)
            break;
        icount++;
    }
    fclose(fp);
}

void   dump_map_ie_node(dl_map_ie *p_node,int itimes)
{
    FILE    *fp;
    char *cat = "./data/output/dl_map_ie_node", *suf = ".dump";
    
    char file[256];// = ".\\data\\output\\dl_map_ie_node.dump";
    memset(file,0,sizeof(char)*256);
    
    strcpy(file, cat);
    strcat(file, suf);

    if(itimes == 0)
    {
        fp = fopen(file, "w+");
        if(fp == NULL)
        {
            printf("open file failed \n");
            return;
        }
    } 
    else
    {
        fp = fopen(file, "ab+");
        if(fp == NULL)
        {
            printf("open file failed \n");
            return;
        }

        fseek(fp,-1,SEEK_END);
    }
    fprintf(fp,"--------------------------the %dth to record--------------------\n",itimes);

    fprintf(fp, "ie_index = %d\n",p_node->ie_index);
    fprintf(fp, "diuc = %u\n",p_node->diuc);
    if(p_node -> extended_ie != NULL)
    {
        fprintf(fp, "extended_ie->extended_diuc = %u \n" ,p_node -> extended_ie->extended_diuc);
        fprintf(fp, "extended_ie->length = %u \n" ,p_node -> extended_ie->length);
    }
    else
    {
        fprintf(fp, "extended_ie = NULL \n");
    }
    if(p_node -> normal_ie != NULL)
    {
        fprintf(fp, "normal_ie->n_cid = %d \n" ,p_node -> normal_ie->n_cid);
        if(p_node -> normal_ie->cid != NULL)
            fprintf(fp, "normal_ie->cid = %d \n" ,*(p_node -> normal_ie->cid));
        else
        fprintf(fp, "normal_ie->cid = NULL \n");
        fprintf(fp, "normal_ie->ofdma_symbol_offset = %u \n" ,p_node -> normal_ie->ofdma_symbol_offset);
        fprintf(fp, "normal_ie->subchannel_offset = %u \n" ,p_node -> normal_ie->subchannel_offset);
        fprintf(fp, "normal_ie->boosting = %u \n" ,p_node -> normal_ie->boosting);
        fprintf(fp, "normal_ie->ofdma_triple_symbol_num = %u \n" ,p_node -> normal_ie->ofdma_triple_symbol_num);
        fprintf(fp, "normal_ie->ofdma_Symbols_num = %u \n" ,p_node -> normal_ie->ofdma_Symbols_num);
        fprintf(fp, "normal_ie->subchannels_num = %u \n" ,p_node -> normal_ie->subchannels_num);
        fprintf(fp, "normal_ie->repetition_coding_indication = %u \n" ,p_node -> normal_ie->repetition_coding_indication);
    }
    else
    {
        fprintf(fp, "normal_ie = NULL \n" );
    }
    fclose(fp);
}
void   dump_burst_node(phy_burst *p_burst_node,int itimes)
{
    FILE    *fp;
    char *cat = "./data/output/phy_burst_node", *suf = ".dump";
    char file[256];
    int i = 0; 

    strcpy(file, cat);
    strcat(file, suf);

    if(itimes == 0)
    {
        fp = fopen(file, "w+");
        if(fp == NULL)
        {
            printf("open file failed \n");
            return;
        }
    } 
    else
    {
        fp = fopen(file, "ab+");
        if(fp == NULL)
        {
            printf("open file failed \n");
            return;
        }

        fseek(fp,-1,SEEK_END);
    }
    fprintf(fp,"--------------------------the %dth to record--------------------\n",itimes);

    fprintf(fp, "length = %d\n",p_burst_node->length);
    fprintf(fp, "burst_payload\n");
    for(;i < p_burst_node->length; i++)
    {
        fprintf(fp,"%u",*(unsigned int*)(p_burst_node->burst_payload + i));
        if(i % 80 ==0)
            fprintf(fp,"\n");
    }
    fprintf(fp,"\n");
    fprintf(fp,"map_burst_index = %d\n",p_burst_node->map_burst_index); 
    fclose(fp);   
}

void adapter_dump_slot_symbol_data(FILE *fp,struct phy_dl_slotsymbol *phybuffer,int ntimes);
void adapter_dump_phy_dl_slot(FILE *fp,struct phy_dl_slot *p_slot);
void adapter_dump_phy_dl_symbolslot(FILE *fp,struct phy_dl_slotsymbol *p_symbolslot);
void adapter_dump_slot_symbol_data(FILE *fp,struct phy_dl_slotsymbol *phybuffer,int ntimes)
{

    if(phybuffer == NULL)
        return ;

    struct phy_dl_slot *p_slot = phybuffer->slot_header;

    fprintf(fp,"\n--------------------------the %dth to record--------------------\n",ntimes);

    adapter_dump_phy_dl_symbolslot(fp,phybuffer);
    int icount = 0;
    while(p_slot != NULL)
    {
        fprintf(fp,"---------------------the %d slot-----------------------------\n",icount);
        adapter_dump_phy_dl_slot(fp,p_slot);
        fprintf(fp,"------------------------------------------------------\n");

        
        p_slot = p_slot->next;
        icount++;
    }
    
}
    


void adapter_dump_phy_dl_symbolslot(FILE *fp,struct phy_dl_slotsymbol *p_symbolslot)
{
    fprintf(fp,"------------------------ phy_symboleslot -----------------------------\n");

    fprintf(fp, "is_broadcast = %d\n",p_symbolslot->is_broadcast);
    fprintf(fp, "dl_subframe_end_flag = %d\n",p_symbolslot->dl_subframe_end_flag);
    fprintf(fp, "mimo_mode = %d\n",p_symbolslot->mimo_mode);
    fprintf(fp, "symboloffset = %d\n",p_symbolslot->symboloffset);
    fprintf(fp, "slotlength = %d\n",p_symbolslot->slotlength);
    fprintf(fp, "frame_index = %d\n",p_symbolslot->frame_index);
    fprintf(fp,"--------------------------------------------------------------------\n");
}

void adapter_dump_phy_dl_slot(FILE *fp,struct phy_dl_slot *p_slot)
{
    int i = 0; 
    fprintf(fp, "code_id = %d\n",p_slot->code_id);
    fprintf(fp, "bytelength = %d\n",p_slot->bytelength);
    fprintf(fp, "repetition_coding_indication = %d\n",p_slot->repetition_coding_indication);
    fprintf(fp, "is_broadcast = %d\n",p_slot->is_broadcast);
    fprintf(fp, "coding_type = %d\n",p_slot->coding_type);
    fprintf(fp, "mimo_mode = %d\n",p_slot->mimo_mode);
    fprintf(fp, "block_id = %d\n",p_slot->block_id);
    fprintf(fp, "unused_flag = %d\n",p_slot->unused_flag);
    for(; i < p_slot->bytelength; i++)
    {
        fprintf(fp,"%x",p_slot->payload[i]);
    }
    fprintf(fp, "\n");
}

void adapter_dump_phy_dl_slot_bench(FILE *fp,struct phy_dl_slotsymbol *phybuffer)
{
    int i = 0;
    struct phy_dl_slot *p_slot = phybuffer->slot_header;
    while(p_slot != NULL)
    { 
        //printf("slot_byte_length %d \n",p_slot->bytelength);
        for(i = 0; i < p_slot->bytelength; i++)
        {
            fprintf(fp,"%d\n",p_slot->payload[i]);
        }
        p_slot = p_slot->next;;
    }
    //fprintf(fp, "\n");
}
