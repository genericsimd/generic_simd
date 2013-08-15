/* ----------------------------------------------------------------------------
 * IBM Confidential

 IBM Wireless M2M Platform

 (C)Copyright IBM Corp. 2009, 2010, 2011

 All Rights Reserved.

 File Name: adapter_bs_ul_map_interface.c

 Change Activity:

 Date             Description of Change                            By
 -----------      ---------------------                            --------
 3-Mar 2011       Created                                         jian jun chang 

 ----------------------------------------------------------------------------
 PROLOG END TAG zYx                                                         */
#include <string.h>
#include <strings.h>
#include <stdlib.h>
#include "mac_phy_data.h"
#include "adapter_bs_ul_map_interface.h"
#include "adapter_bs_ul_interface_data.h"
#include "adapter_test_stube.h"
#include "ulmap.h"
#include "mac_headermsg_parser.h"
#include "adapter_bs_ul_map_test.h"
#include "free_maps.h"
#define _RANGING_OFFSET_  12
#define __USE_TEST_STUBE__
#define _DEBUG_PRINTF_
#define UL_MAP_SYMBOL_START_OFFSET       8

struct slot_symbol_ie * get_symbol_slot(int slot_offset,int slot_number);
struct block_data_ie *  get_block_data_ie(u_int32_t uiuc,u_int8_t ofdma_symbol_offset,u_int8_t subchannel_offset, u_int8_t ofdma_symbol_num, 
    u_int8_t       subchannel_num,int8_t burst_id);
struct slot_symbol_ie * get_symbol_slot(int slot_offset,int slot_number)
{
    struct slot_symbol_ie* p_ie = (struct slot_symbol_ie*)malloc(sizeof(struct slot_symbol_ie));
    memset(p_ie,0,sizeof(struct slot_symbol_ie));
    p_ie->slot_offset = slot_offset;
    p_ie->slot_number = slot_number;
    return p_ie;
}
struct block_data_ie *  get_block_data_ie(u_int32_t uiuc,u_int8_t ofdma_symbol_offset,u_int8_t subchannel_offset, u_int8_t       ofdma_symbol_num,
    u_int8_t       subchannel_num,int8_t burst_id)
{
    struct block_data_ie *p_ie = (struct block_data_ie * )malloc(sizeof(struct block_data_ie));
    memset(p_ie,0,sizeof(struct block_data_ie));
    p_ie->uiuc = uiuc;
    if(uiuc >0 )
        p_ie->code_id = uiuc -1 ;
    else
        p_ie->code_id = 0;
    p_ie->ofdma_symbol_offset = ofdma_symbol_offset;
    p_ie->subchannel_offset = subchannel_offset;
    p_ie->ofdma_symbol_num = ofdma_symbol_num;
    p_ie->subchannel_num = subchannel_num;
    p_ie->burst_id = burst_id;
    if(burst_id == -1)
        p_ie->is_used = 0;
    else 
        p_ie->is_used = 1;
    return p_ie;
}

struct union_burst_ie * get_burst_ie(u_int32_t buf_in_len,u_int32_t buf_out_len,u_int32_t uiuc,u_int32_t slots_num,
           u_int32_t repetition_coding_indication,u_int32_t coding_type,u_int32_t mimo_mode,u_int32_t coding_rate,u_int32_t  cid)
{
    struct union_burst_ie *s = (struct union_burst_ie *) malloc(sizeof(struct union_burst_ie));
    memset(s,0,sizeof(struct union_burst_ie));
    s->buf_in_len = buf_in_len;
    s->buf_out_len = buf_out_len;
    s->uiuc = uiuc;
    if(uiuc > 0)
        s->code_id = uiuc -1;
    else
        s->code_id = 0;
    s->slots_num = slots_num;
    s->repetition_coding_indication =repetition_coding_indication;
    s->coding_type = coding_type;
    s->mimo_mode = mimo_mode;
    s->coding_rate = coding_rate;
    s->cid = cid;
    return s;
}             

int initial_assist_data(struct ul_assist_data pp_assist_array[][MAX_UL_SUBFRAME_SLOT_NUM], int xsize,int ysize)
{
    int x = 0, y = 0;


    for(x = 0; x < xsize; x++)
    {
        for(y = 0; y < ysize; y++)
        {
            pp_assist_array[x][y].xoffset = 0;
            pp_assist_array[x][y].yoffset = 0;
            pp_assist_array[x][y].burst_id = 0;
            pp_assist_array[x][y].is_used = 0;
            pp_assist_array[x][y].mode_id = -1;
            pp_assist_array[x][y].data_length = 0;
            pp_assist_array[x][y].af_dem_offset = 0;
            pp_assist_array[x][y].be_dem_offset = 0;
            pp_assist_array[x][y].uiuc = -1;
            pp_assist_array[x][y].slots_num = 0;
            pp_assist_array[x][y].repetition_coding_indication =1;
            pp_assist_array[x][y].coding_type = 0;          //0 CC 1 CTC
            pp_assist_array[x][y].mimo_mode = 0;                 //0 siso 1 stcmatrixa2x1  2 stcmatrixb2x2
            pp_assist_array[x][y].coding_rate = 0;
        }
    }
    return 0;
}

int print_assist_data(struct ul_assist_data (*pp_assist_array)[MAX_UL_SUBFRAME_SLOT_NUM], int xsize,int ysize)
{
    int x = 0, y = 0;
    for(x = 0; x < xsize; x++)
    {
        for(y = 0; y < ysize; y++)
        {
            printf("(%d %d):burst_id %d,is_used %d ,mode_id %d ,uiuc %d,data_length %d \n", x,y ,
                            pp_assist_array[x][y].burst_id,
                            pp_assist_array[x][y].is_used,
                            pp_assist_array[x][y].mode_id,
                            pp_assist_array[x][y].uiuc,
                            pp_assist_array[x][y].data_length);
        }
    }
    return 0;
}

int adapter_ofdma_ul_transform_matrix(ul_map_msg *ulmap_msg, struct ul_assist_data pp_assist_array[][MAX_UL_SUBFRAME_SLOT_NUM])
{
    int irecordoffsetx = 0;
    int irecordoffsety = 0;
    
    ul_map_ie *p_mapienode = ulmap_msg->ie;
    int isymbol_slot_num = UL_SYMBOL_NUMBER/CURRENT_UL_OFDMA_SLOT_LENGTH_INSYM ;
   
    while(p_mapienode != NULL)
    {
        if(p_mapienode->uiuc_12_ie != NULL)
        {
            for(irecordoffsetx = 0; irecordoffsetx < p_mapienode->uiuc_12_ie->subchannel_num;irecordoffsetx ++)
            {
                for(irecordoffsety = 0; irecordoffsety < (int)(p_mapienode->uiuc_12_ie->ofdma_symbol_num/CURRENT_UL_OFDMA_SLOT_LENGTH_INSYM); irecordoffsety ++)
                {
                    pp_assist_array[irecordoffsetx][irecordoffsety].is_used = 1;
                    pp_assist_array[irecordoffsetx][irecordoffsety].burst_id = -1;  
                }
            }
            
        }
        p_mapienode = p_mapienode->next;
    }
    irecordoffsetx = 0; 
    irecordoffsety = 0;
    //*p_validate_x_offset = irecordoffsetx;
    p_mapienode = ulmap_msg->ie;
    while(p_mapienode != NULL)
    {
        if(p_mapienode->uiuc_other_ie != NULL)
        {//process data for duration 
            int  icount = 0;
            while(icount < p_mapienode->uiuc_other_ie->duration)
            {
                if(irecordoffsety >= isymbol_slot_num)
                {//o
                    irecordoffsetx++;
                    irecordoffsety = 0;
                }//o
                if(pp_assist_array[irecordoffsetx][irecordoffsety].is_used == 0)
                { 
                    icount++;
                    pp_assist_array[irecordoffsetx][irecordoffsety].burst_id = p_mapienode->ie_index ;
                    pp_assist_array[irecordoffsetx][irecordoffsety].is_used = 1;
                    pp_assist_array[irecordoffsetx][irecordoffsety].cid = p_mapienode->cid;
                    irecordoffsety ++;
                }
                else
                {
                    irecordoffsety++;
                }
                //printf("%d : %d\n",irecordoffsetx,irecordoffsety);
            }//
        }else if(p_mapienode->uiuc_14_ie != NULL)
        {//process data for duration 
            int  icount = 0;
            while(icount < p_mapienode->uiuc_14_ie->duration)
            {
                if(irecordoffsety >= isymbol_slot_num)
                {//o
                    irecordoffsetx++;
                    irecordoffsety = 0;
                }//o
                if(pp_assist_array[irecordoffsetx][irecordoffsety].is_used == 0)
                {
                    icount++;
                    pp_assist_array[irecordoffsetx][irecordoffsety].burst_id = p_mapienode->ie_index ;
                    pp_assist_array[irecordoffsetx][irecordoffsety].is_used = 1;
                    pp_assist_array[irecordoffsetx][irecordoffsety].cid = p_mapienode->cid;
                    irecordoffsety ++;
                }
                else
                {
                    irecordoffsety++;
                }
                //printf("%d : %d\n",irecordoffsetx,irecordoffsety);
            }//
        }//process data for duration

      
        p_mapienode = p_mapienode->next;
    }
    return 0;
}

int  fill_data_to_slot(ul_map_msg *ulmap_msg,struct ul_assist_data pp_assist_array[][MAX_UL_SUBFRAME_SLOT_NUM],struct ul_frame_ie *p_frame)
{
    int irecordoffsetx = 0;
    int irecordoffsety = 0;
    ul_map_ie *p_mapienode = ulmap_msg->ie;
    
    int  itotal_data_bits = 0;     
    int  itotal_raw_bits = 0;     
    while(p_mapienode != NULL)
    {
        irecordoffsetx = 0;
        irecordoffsety = 0;
       
        if(p_mapienode->uiuc_other_ie != NULL)
        {//process data for duration 
            int  icount = 0;
            int  slot_bits = GET_UL_BITS_PERSUBCH_PERSYM(p_mapienode->uiuc - 1) * CURRENT_UL_OFDMA_SLOT_LENGTH_INSYM ;
            while(icount < p_mapienode->uiuc_other_ie->duration)
            {
                if((int)pp_assist_array[irecordoffsetx][irecordoffsety].burst_id == p_mapienode->ie_index)
                {
                     pp_assist_array[irecordoffsetx][irecordoffsety].af_dem_offset += itotal_data_bits;
                     pp_assist_array[irecordoffsetx][irecordoffsety].be_dem_offset += itotal_raw_bits;
                     pp_assist_array[irecordoffsetx][irecordoffsety].data_length = slot_bits;
                     pp_assist_array[irecordoffsetx][irecordoffsety].uiuc = p_mapienode->uiuc;
                     pp_assist_array[irecordoffsetx][irecordoffsety].repetition_coding_indication = 1;
                     pp_assist_array[irecordoffsetx][irecordoffsety].coding_type = pp_assist_array[irecordoffsetx][irecordoffsety].uiuc -1;          //0 CC 1 CTC
                     pp_assist_array[irecordoffsetx][irecordoffsety].mimo_mode = 0;                 //0 siso 1 stcmatrixa2x1  2 stcmatrixb2x2
                     pp_assist_array[irecordoffsetx][irecordoffsety].coding_rate = 0;
                     itotal_data_bits += slot_bits;
                     itotal_raw_bits += 48;
                     icount ++;
                }
                irecordoffsetx ++;
                if(pp_assist_array[irecordoffsetx][irecordoffsety].is_used == 0 )
                {
                    irecordoffsety ++;
                    irecordoffsetx = 0;
                }
                //printf("%d : %d\n",irecordoffsetx,irecordoffsety);
            }//p
        }//process data for duration
        else if(p_mapienode->uiuc_14_ie != NULL)
        {//process data for duration 
            int  icount = 0;
            int  slot_bits = GET_UL_BITS_PERSUBCH_PERSYM(p_mapienode->uiuc_14_ie->uiuc - 1) * CURRENT_UL_OFDMA_SLOT_LENGTH_INSYM ;
            while(icount < p_mapienode->uiuc_14_ie->duration)
            {
                if((int)pp_assist_array[irecordoffsetx][irecordoffsety].burst_id == p_mapienode->ie_index)
                {
                     pp_assist_array[irecordoffsetx][irecordoffsety].af_dem_offset += itotal_data_bits;
                     pp_assist_array[irecordoffsetx][irecordoffsety].be_dem_offset += itotal_raw_bits;
                     pp_assist_array[irecordoffsetx][irecordoffsety].data_length = slot_bits;
                     pp_assist_array[irecordoffsetx][irecordoffsety].uiuc = p_mapienode->uiuc_14_ie->uiuc;
                     pp_assist_array[irecordoffsetx][irecordoffsety].repetition_coding_indication = 1;
                     pp_assist_array[irecordoffsetx][irecordoffsety].coding_type = pp_assist_array[irecordoffsetx][irecordoffsety].uiuc -1;  //0 CC 1 CTC
                     pp_assist_array[irecordoffsetx][irecordoffsety].mimo_mode = 0;                 //0 siso 1 stcmatrixa2x1  2 stcmatrixb2x2
                     pp_assist_array[irecordoffsetx][irecordoffsety].coding_rate = 0;
                     itotal_data_bits += slot_bits;
                     itotal_raw_bits += 48;
                     icount ++;
                }
                irecordoffsetx ++;
                if(pp_assist_array[irecordoffsetx][irecordoffsety].is_used == 0 )
                {
                    irecordoffsety ++;
                    irecordoffsetx = 0;
                }
                //printf("%d : %d\n",irecordoffsetx,irecordoffsety);
            }//p
        }//process data for duration

        p_mapienode = p_mapienode->next;
        
    }
    p_frame->p_total_buffer_out = malloc(sizeof(u_int8_t) * itotal_data_bits);
    return 0;
    
}

struct slot_symbol_ie* get_map_symbol_slot_ie(struct ul_assist_data pp_assist_array[][MAX_UL_SUBFRAME_SLOT_NUM],int yoffset,u_int8_t *p_buf_src)
{
    struct slot_symbol_ie *p_symbol_slot = get_symbol_slot(yoffset * CURRENT_UL_OFDMA_SLOT_LENGTH_INSYM,31);
    
    struct block_data_ie *p_block_data = NULL,*p_cur_block = NULL;
    int i = 0,record_x = 0;
    int cur_burst_id = -1;
    for(; i < MAX_UL_SUBFRAME_SLOT_NUM; i++)
    {
        if((int)pp_assist_array[i][yoffset].burst_id != -1)
        {
                 cur_burst_id = pp_assist_array[i][yoffset].burst_id;
                 record_x = i;
                 break;
            }
        }
    for(; i < MAX_UL_SUBFRAME_SLOT_NUM; i++)
    {
        if((pp_assist_array[i][yoffset].burst_id != cur_burst_id) && (pp_assist_array[i][yoffset].burst_id != -1))
        {
            p_block_data = get_block_data_ie(pp_assist_array[record_x][yoffset].uiuc,yoffset * CURRENT_UL_OFDMA_SLOT_LENGTH_INSYM,record_x, 3,i - record_x ,cur_burst_id);
            p_block_data->p_data_buffer = p_buf_src + pp_assist_array[record_x][yoffset].af_dem_offset;
            p_block_data->data_offset = pp_assist_array[record_x][yoffset].be_dem_offset;
            p_block_data->pilot_offset = pp_assist_array[record_x][yoffset].be_dem_offset/2;
            
            
            record_x = i;
            cur_burst_id = pp_assist_array[i][yoffset].burst_id;
            if(p_symbol_slot ->p_block_header  == NULL)
            {
                p_symbol_slot->p_block_header = p_block_data;
            }
            else
            {
                p_cur_block->next = p_block_data;
            }
            p_cur_block = p_block_data;
        }
    }
    return p_symbol_slot;
}

int process_symbol_data(struct ul_assist_data pp_assist_array[][MAX_UL_SUBFRAME_SLOT_NUM],struct  ul_frame_ie *p_frame)
{
    int cur_slot_num = UL_SYMBOL_NUMBER/CURRENT_UL_OFDMA_SLOT_LENGTH_INSYM ;
    struct slot_symbol_ie   *p_cur = NULL, *p_symbol_slot = NULL;
    int i = 0;
    //u_int8_t p_src_arr[10] ;
    //p_src_arr[0] = p_frame->p_total_buffer_out;
    for(; i < cur_slot_num; i++)
    {
        p_symbol_slot = get_map_symbol_slot_ie(pp_assist_array,i,p_frame->p_total_buffer_out);
        if(p_cur== NULL)
        {
            p_frame->p_slot_header = p_symbol_slot;
        }
        else
        {
            p_cur->next = p_symbol_slot;
        }
        p_cur = p_symbol_slot;
    } 
    return 0;
}

int process_burst_data(struct ul_assist_data pp_assist_array[][MAX_UL_SUBFRAME_SLOT_NUM], struct  ul_frame_ie *p_frame)
{
    int i = 0, j = 0;
    int cur_slot_num = UL_SYMBOL_NUMBER/CURRENT_UL_OFDMA_SLOT_LENGTH_INSYM ;
    int  burst_id = -1;
    int  record_x = -1,record_y = -1;
    struct union_burst_ie * p_burst_cur = NULL;
    struct union_burst_ie * p_burst_iter = NULL;
    int  burst_length = 0;
    int slot_num = 0;
    u_int8_t  *p_src_arr = NULL;
    for(i = 0 ;i < cur_slot_num ; i++)
    {
        j = 0;
        for(;j < MAX_UL_SUBFRAME_SLOT_NUM ; j++)
        {
            if(pp_assist_array[i][j].burst_id == -1)
                continue;
            else 
            {
                record_x = i;
                record_y = j;
                burst_id = pp_assist_array[i][j].burst_id;
                goto  next_step;
            }
        }
    }
next_step:
    for(i = 0 ;i < MAX_UL_SUBFRAME_SLOT_NUM; i++)
    {
        for(j = 0;j < cur_slot_num ; j++)
        {
            if(pp_assist_array[i][j].burst_id != -1 )
            {
                if(pp_assist_array[i][j].burst_id != burst_id)
                {
                //generate  new  burst and set burst_length
                    p_src_arr = p_frame->p_total_buffer_out + pp_assist_array[record_x][record_y].af_dem_offset;
                
                    p_burst_cur = get_burst_ie(burst_length,burst_length, pp_assist_array[record_x][record_y].uiuc, slot_num,
                          pp_assist_array[record_x][record_y].repetition_coding_indication, pp_assist_array[record_x][record_y].coding_type, 
                          pp_assist_array[record_x][record_y].mimo_mode, pp_assist_array[record_x][record_y].coding_rate,pp_assist_array[record_x][record_y].cid);
    ;
                    p_burst_cur->p_buf_out = p_src_arr;
                    p_burst_cur->data_offset = pp_assist_array[record_x][record_y].be_dem_offset;
                    p_burst_cur->pilot_offset = pp_assist_array[record_x][record_y].be_dem_offset/2;
                    
                    if(p_frame->p_burst_header == NULL)
                        p_frame->p_burst_header = p_burst_cur;
                    else
                        p_burst_iter->next = p_burst_cur;
                    p_burst_iter = p_burst_cur;

                    record_x = i;
                    record_y = j;
                    burst_length = pp_assist_array[i][j].data_length;
                    slot_num = 1;
                    burst_id = pp_assist_array[i][j].burst_id; 
                }
                else
                {
                    burst_length += pp_assist_array[i][j].data_length;
                    if(j < record_y )
                    {
                        record_x = i;
                        record_y = j;
                    }
                    slot_num ++;
                }
            }
        }
    }
    return 0;
}

int parse_ul_map(u_int8_t *p_buf,int len,struct  ul_frame_ie **pp_frame)
{
    struct ul_assist_data assist_array[MAX_UL_SUBCHANNEL_NUM ][MAX_UL_SUBFRAME_SLOT_NUM ];


    ul_map_msg *ulmap_msg = (ul_map_msg*)malloc(sizeof(ul_map_msg));
    memset(ulmap_msg,0,sizeof(ulmap_msg));
    parse_ul_map_msg(p_buf,len,ulmap_msg);
    *pp_frame = (struct ul_frame_ie * )malloc(sizeof(struct ul_frame_ie));
    memset(*pp_frame,0,sizeof(struct ul_frame_ie));
//    struct ul_assist_data    assist_array[MAX_UL_SUBCHANNEL_NUM + 1][MAX_UL_SUBFRAME_SLOT_NUM + 1];
    initial_assist_data(assist_array, MAX_UL_SUBCHANNEL_NUM, MAX_UL_SUBFRAME_SLOT_NUM);

    //print_assist_data(assist_array,MAX_UL_SUBCHANNEL_NUM,MAX_UL_SUBFRAME_SLOT_NUM);
    adapter_ofdma_ul_transform_matrix(ulmap_msg, assist_array);
    //print_assist_data(assist_array,15 ,5);
    fill_data_to_slot(ulmap_msg, assist_array,*pp_frame);
    //print_assist_data(assist_array,15,5);
    free_ulmap(ulmap_msg);
    free(ulmap_msg);
    free(p_buf);

    if(process_symbol_data(assist_array,*pp_frame) == 0)
    {
        process_burst_data(assist_array,*pp_frame);
        //adapter_print_frame_map(*pp_frame);
    }
    else
    {
        return -1;
    }

    return 0;
}

//int  free_adapter_frame(struct  ul_frame_ie **pp_frame)
int  free_adapter_frame(void **p)
{
    struct  ul_frame_ie **pp_frame = (struct  ul_frame_ie **)p;

    struct  slot_symbol_ie  *p_slot_iter = (*pp_frame)->p_slot_header;
    struct  union_burst_ie  *p_burst_iter = (*pp_frame)->p_burst_header;
    struct block_data_ie    *p_block_data = NULL;

    if(*pp_frame !=NULL)
    {
        if((*pp_frame)->p_total_buffer_out != NULL)
            free((*pp_frame)->p_total_buffer_out);

        if ((*pp_frame)->pscan_resflag != 1)
        {
            while(p_burst_iter != NULL)
            {
                (*pp_frame)->p_burst_header = p_burst_iter->next;
                free(p_burst_iter);
                p_burst_iter = (*pp_frame)->p_burst_header;
            }
            while(p_slot_iter != NULL)
            {
                p_block_data = p_slot_iter->p_block_header;

                while(p_block_data != NULL)
                { 
                    p_slot_iter->p_block_header = p_block_data->next;
                    free(p_block_data);
                    p_block_data = p_slot_iter->p_block_header;
                }
                (*pp_frame)->p_slot_header = p_slot_iter->next;
                free(p_slot_iter); 
                p_slot_iter = (*pp_frame)->p_slot_header;
            }
        }

        free(*pp_frame);
    }

    return 0;
}
