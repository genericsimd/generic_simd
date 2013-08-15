/* ----------------------------------------------------------------------------
 * IBM Confidential

 IBM Wireless M2M Platform

 (C)Copyright IBM Corp. 2009, 2010, 2011

 All Rights Reserved.

 File Name: adapter_bs_dl_transformer_phy.c

 Change Activity:

 Date             Description of Change                            By
 -----------      ---------------------                            --------
 2-Mar 2011       Created                                         jian jun chang 

 ----------------------------------------------------------------------------
 PROLOG END TAG zYx                                                         */

#include "adapter_bs_dl_transformer_phy.h"


#include "mac_shift_bits.h"

#include "mac_map_phy.h"

#include "mac_assistfunc.h"

#include "mac_modulation_adapter.h"

#include "adapter_test_stube.h"
#include "dlmap.h"
#include "dl_exp_params.h"
#include "flog.h"
#include "adapter_config.h"

#include <sys/time.h>
#include <sys/times.h>
#include <time.h>

#define _USE_STUBE_CODE_DCD_MSG_

#define  PADDING_LENGTH           48
const char  PADDING_BUFFER[6] = {0,0,0,0,0,0};          

#define  FCH_START_X_OFFSET       0
#define  FCH_X_LENGTH             8       //this is solid for fch
#define  FCH_START_Y_OFFSET       0
#define  FCH_Y_LENGTH             1

#define  DL_MAP_START_X_OFFSET    8
#define  DL_MAP_START_Y_OFFSET    0
#define  DL_MAP_Y_LENGTH          1


//        //0 all data fill 0, 1 all data fill 1;  2  some data fill 0 and some data fill 1;

#define  _DEBUG
#define _TEST_INTEGRATE_


static  int  burst_count = 1;

/*#define MEMORY_CHEST(p,length)            long size;\
                                          memcpy(&size,dataunit->burst_payload - sizeof(long),sizeof(long));\
                                          if(size <= length)\
	                                           return 1;\
                                          else return 0;
*/


/*
 *calculate data rect is in that area  
 *if data in return 0,else return 1;
 */
int isvalidinrect(int rectw,int rectl, int datastartx,int datastarty,int dataw,int data);

/*compare new slotnum and subchnum,if need then change
 *
 */
int changeofdma_frame_subchnum_slotnum(struct ofdma_map* p_ofdma_frame,int isubchnum,int islotnum);

/*
 *find dl_map_ie node in dl_map,if the node eque mapindex then return;
 */
//int find_map_ienode_by_mapindex_dl(u_int8_t  mapindex, dl_map_ie **pp_dlmapnode  );

/*
 *find dl_burst_profile in dcdmsg by diuc
 *create by changjj 2008.11.7
 */
int find_burstnode_by_diuc_dl(u_int8_t diuc, dl_burst_profile **pp_burstnode);


/*function: fill_preamfix_data
*this function is for fill FCH,DL_MAP and other infomation that modulation is QPSK_1_2 data format.
*
*/
int fill_preamfix_data(phy_burst *p_dataunit,struct ofdma_map* p_ofdma_frame,int ibitperslot,int xoffset,int yoffset);
/*
 *fill data to special data region with follow rules;
 *the slot is min data in physical frame so 
 *first fill data to symbols,the symbols maybe have one or more than one 
 *seconde fill subchannel the follow is example
 *
 *subch*|-sym0-|-sym1-|-sym2-|-sym3-|
 *subch0|----first--->|--- fourth ->|       
 *subch1|---second--->|----fifth--->| 
 *subch2|____third____|_____sixth___|
 *subch3|
 * 
 *the data have 6 symbols and 3 subchannel,  
 *and perslot have one subchannel and two symbols,so fill data to first slot on  sym0 and sym1 
 *create by changjj 2008.11.7
 */
int fill_ofdma_subframe_dl(dl_map_ie **pp_mapienode,void *unspecified_data, phy_burst *p_dataunit,struct ofdma_map* p_ofdma_frame,int modulation,int offset_ofdma,int *p_x_end_offset,int *p_y_end_offset,int block_id);


//int ofdma_transform_slotlink(ofdma_map* p_ofdma_frame, slot_node **slotlink);


//int slotlink_transform_ofdma(slot_node *slotlink, ofdma_map **p_ofdma_frame);

/*
 *for append data to p_dataunit;
 *
 */
int adjecentdata_dl(struct ofdma_map * p_ofdma_frame,ul_map_ie * pmapienode,phy_burst **dataunit);

/******************************************************DL_TRANSFORM****************************************/
int get_new_dl_map_link(dl_map_msg **pp_dl_map);
int release_new_dl_map_link(dl_map_msg **pp_dl_map);
int get_new_dl_dcd_msg(dcd_msg  **pp_dcdmsg,int dcd_count);
int release_new_dl_dcd_msg(dcd_msg **pp_dcdmsg);


int fill_FCH_in_ofdma_frame(struct ofdma_map* p_ofdma_frame,u_int8_t *p_fch_buffer,int buffer_len,int x_offset,int y_offset,int *p_end_x_offset,int *p_end_y_offset,int block_id);
int fill_dl_map_in_ofdma_frame(struct ofdma_map* p_ofdma_frame,u_int8_t *p_dl_map_buffer,int buff_len,int x_offset,int y_offset,int *p_end_x_offset,int *p_end_y_offset,int block_id);

int phy_subframe_send_transform_dl_process(physical_subframe* p_phy_subframe,struct  ofdma_map* p_ofdma_frame,dl_map_ie *p_map_link);//,dcd_msg *p_dcd_msg);

int phy_subframe_send_transform_dl(physical_subframe* p_phy_subframe,struct ofdma_map* p_ofdma_frame,dl_map_msg *p_dl_map)
{
    ERROR_RETURN(p_phy_subframe,NULL,"sub_frame is null");
  
    ERROR_RETURN(p_ofdma_frame,NULL,"p_ofdma_frame is null");
    
    
    initialize_ofdma_map(p_ofdma_frame);

//this is for FCH and DL_map process
    

#ifdef  _DEBUG
    //ERROR_TRACE("ready to process current frame_num is %d\n",p_phy_subframe->frame_num);
#endif
//for data burst
    
    static int   itimes = 0;
#ifndef  _USE_STUBE_CODE_DL_MAP_
    
       
//    dump_burst_link(p_phy_subframe->burst_header,itimes);
    //printf("record burst_link and map_link\n");
            //itimes++;
            
#endif
    /**********************************************************************/
    /*find dl_map_ie and dl_burst_profile  for current burst;
    */
    
    dcd_msg     *p_dcd_msg = NULL;
    
    

#ifndef  _USE_STUBE_CODE_DL_MAP
    //static int   itimes = 0;
//    dump_dlmap_ie_link(p_dl_map->ie_head,itimes);
#endif
    itimes++;
    
    if(p_dl_map != NULL)
    {
         if(get_new_dl_dcd_msg(&p_dcd_msg,p_dl_map->dcd_count) == -1)
            return -1;
    }
    else 
   {
        if(get_new_dl_dcd_msg(&p_dcd_msg,0) == -1)
            return -1;
   }
    
#ifdef  _DEBUG
    //ERROR_TRACE("processed current frame_num is %d\n",p_phy_subframe->frame_num);
#endif
//    p_ofdma_frame->p_dts_info = p_phy_subframe->interference_info;

    if (p_phy_subframe->interference_info != NULL)
    {
        p_ofdma_frame->p_dts_info = (void *)malloc (sizeof(struct adapter_dts_info));
        memcpy(p_ofdma_frame->p_dts_info, p_phy_subframe->interference_info, sizeof(struct adapter_dts_info));
    }else
    {
        p_ofdma_frame->p_dts_info = NULL;
    }

    u_int8_t  *p_raw_map = malloc(sizeof(u_int8_t) * p_phy_subframe->raw_ul_map_len);
    memcpy(p_raw_map,  p_phy_subframe->raw_ul_map,p_phy_subframe->raw_ul_map_len);
    
    p_ofdma_frame->p_ulmap = p_raw_map;
    p_ofdma_frame->ul_map_len = p_phy_subframe->raw_ul_map_len; 

    p_ofdma_frame->dl_perscan_flag = p_phy_subframe->sense_flag;
//    free(p_raw_map);
//    p_ofdma_frame->p_ulmap = NULL;
    //ERROR_RETURN(get_dl_map_msg(&dlmap),-1,"get dl_map_msg is failed in phy_subframe_send_transform_dl\n");
    int flag = phy_subframe_send_transform_dl_process(p_phy_subframe,p_ofdma_frame,p_dl_map->ie_head);//,p_dcd_msg);

    p_ofdma_frame->slot_num = SLOTSYMBOL_NUM;

    //release_new_dl_map_link(&p_dl_map);
    //free(p_dl_map);    
    return flag;

}


int get_new_dl_map_link(dl_map_msg **pp_dl_map)
{
    dl_map_msg    *dlmap;
    dl_map_ie      *ptmp = NULL,*ptmp2 = NULL,*p_dl_head = NULL,*ptemp = NULL;
#ifdef  _USE_STUBE_CODE_DL_MAP_
    
    get_dl_map_msg_stube(&dlmap);
    if(dlmap == NULL)
        return -1;
    //*pp_dl_map = dlmap;
    //return 0;
#else
    ERROR_RETURN(get_dl_map_msg(&dlmap,frame_num),-1,"get dl_map_msg is failed in phy_subframe_send_transform_dl\n");
#endif
    *pp_dl_map = (dl_map_msg*)malloc(sizeof(dl_map_msg));
    
    (*pp_dl_map)->manage_msg_type = dlmap->manage_msg_type;
    (*pp_dl_map)->frame_duration_code = dlmap->frame_duration_code;
    (*pp_dl_map)->frame_number = dlmap->frame_number;
    (*pp_dl_map)->dcd_count = dlmap->dcd_count;
    (*pp_dl_map)->bs_id = dlmap->bs_id;
    (*pp_dl_map)->ofdma_symbols_num = dlmap->ofdma_symbols_num;
    ptmp = dlmap->ie_head;
    while(ptmp != NULL)
    {
        ptmp2 = (dl_map_ie *)malloc(sizeof(dl_map_ie)); 
        memset(ptmp2,0,sizeof(dl_map_ie)); 
        ptmp2->ie_index = ptmp->ie_index;
        ptmp2->diuc = ptmp->diuc;
        if(ptmp->extended_ie != NULL)
        {
            ptmp2->extended_ie = (extended_diuc_ie *)malloc(sizeof(extended_diuc_ie));
            ptmp2->extended_ie->extended_diuc = ptmp->extended_ie->extended_diuc;
            ptmp2->extended_ie->length = ptmp->extended_ie->length;
            ptmp2->extended_ie->unspecified_data = ptmp->extended_ie->unspecified_data;
        }
        if(ptmp -> normal_ie != NULL)
        {
            ptmp2->normal_ie = (normal_diuc_ie*) malloc(sizeof(normal_diuc_ie));
            ptmp2->normal_ie->n_cid = ptmp->normal_ie->n_cid;
            ptmp2->normal_ie->cid = ptmp->normal_ie->cid;
            ptmp2->normal_ie->ofdma_symbol_offset = ptmp->normal_ie->ofdma_symbol_offset;
            ptmp2->normal_ie->subchannel_offset = ptmp->normal_ie->subchannel_offset;
            ptmp2->normal_ie->boosting = ptmp->normal_ie->boosting;
            ptmp2->normal_ie->ofdma_triple_symbol_num = ptmp->normal_ie->ofdma_triple_symbol_num;
            ptmp2->normal_ie->ofdma_Symbols_num = ptmp->normal_ie->ofdma_Symbols_num;
            ptmp2->normal_ie->subchannels_num = ptmp->normal_ie->subchannels_num;
            ptmp2->normal_ie->repetition_coding_indication = ptmp->normal_ie->repetition_coding_indication;
        }
        
        if(p_dl_head == NULL)
        {
            p_dl_head = ptmp2;
        }
        else 
            ptemp->next = ptmp2;
        ptemp = ptmp2;
        
        ptmp = ptmp->next;
    }
    (*pp_dl_map) ->ie_head = p_dl_head;

#ifndef  _USE_STUBE_CODE_DL_MAP_
       //static int   itimes = 0;
       //dump_dlmap_ie_link(p_dl_head,itimes);
#endif
    return 0;
}

int release_new_dl_map_link(dl_map_msg **pp_dl_map)
{
    
    dl_map_ie      *ptmp = (*pp_dl_map) -> ie_head,*ptmp2 = NULL;
#ifdef _USE_STUBE_CODE_DL_MAP_
    //return 0;
#else
#endif
    
    while(ptmp != NULL)
    {
        ptmp2 = ptmp->next;
        if(ptmp ->normal_ie != NULL)
        {
            free(ptmp -> normal_ie->cid);
            free(ptmp -> normal_ie);
        }
        if(ptmp -> extended_ie != NULL)
        {
            free(ptmp -> extended_ie);
        }
        free(ptmp);
        
        ptmp = ptmp2;
    }
    free(*pp_dl_map);
    return 0;
    
}


int get_new_dl_dcd_msg(dcd_msg  **pp_dcdmsg,int dcd_count)
{
#ifdef  _USE_STUBE_CODE_DCD_MSG_
    dcd_count = 1;
    get_dcd_msg_stube(pp_dcdmsg);

    return 0;
#else
    *pp_dcdmsg = dcd_msg_query(dcd_count);


    ERROR_RETURN(pp_dcdmsg,NULL,"get dcd_msg is NULL in phy_subframe_send_transform\n");
#endif 
    return 0;
}
int release_new_dl_dcd_msg(dcd_msg **pp_dcdmsg)
{
    *pp_dcdmsg = NULL;
    return 0;
}


int fill_buffer_ofdma_frame_slot_QPSK(struct ofdma_map* p_ofdma_frame,u_int8_t *p_buffer,int buffer_len,int x_offset,int y_offset,int *p_end_x_offset,int *p_end_y_offset,int block_id)
{
    int  ibitperslot = 48;
    int  ibitscount = 0;
    int  i = y_offset,j = x_offset;
    for(; i < MAX_SUBFRAME_SLOT_NUM;i++)
    {
        for(;j < (int)CURRENT_DL_OFDMA_SUBCH_NUM; j++)
        {
            int ioffsetbyte = ibitscount/PERBITINBYTE;
            p_ofdma_frame->ele[j][i].offset_for_uchar = (ibitscount - ioffsetbyte*PERBITINBYTE);

            p_ofdma_frame->ele[j][i].len_in_bit = ((ibitscount + ibitperslot)/PERBITINBYTE <= buffer_len)? \
                                                 ibitperslot:(buffer_len*PERBITINBYTE - ibitscount) ;;
            
            p_ofdma_frame->ele[j][i].payload = p_buffer + ioffsetbyte;
            p_ofdma_frame->ele[j][i].mc_type = OFDM_QPSK_1_2;
            p_ofdma_frame->ele[j][i].block_id = block_id;
            ibitscount += ibitperslot;

                //calculate the byte in data;
            p_ofdma_frame->ele[j][i].count_in_bytes = ibitscount/PERBITINBYTE - ioffsetbyte;
            if(ibitscount%PERBITINBYTE != 0)
                    p_ofdma_frame->ele[j][i].count_in_bytes++;
                
            if(ibitscount/PERBITINBYTE >= buffer_len)
            {
	      // isfullpad = 0;
                *p_end_y_offset = i;
                *p_end_x_offset = ++j;
                return 0;;
            }
        }
    }
    *p_end_y_offset = i;
    *p_end_x_offset = ++j;
    return 0;
}

int get_mode_by_diuc(int diuc)
{
    if(diuc == 0)
       return diuc;
    else 
       return diuc + 1;
}

int fill_FCH_in_ofdma_frame(struct ofdma_map* p_ofdma_frame,u_int8_t *p_fch_buffer,int buffer_len,int x_offset,int y_offset,int *p_end_x_offset,int *p_end_y_offset,int block_id)
{
    return fill_buffer_ofdma_frame_slot_QPSK(p_ofdma_frame,p_fch_buffer,buffer_len,  \
                                             x_offset,y_offset,p_end_x_offset,p_end_y_offset,block_id);
}
int fill_dl_map_in_ofdma_frame(struct ofdma_map* p_ofdma_frame,u_int8_t *p_dl_map_buffer,int buffer_len,int x_offset,int y_offset,int *p_end_x_offset,int *p_end_y_offset,int block_id)
{
    return fill_buffer_ofdma_frame_slot_QPSK(p_ofdma_frame,p_dl_map_buffer,buffer_len,\
                                             x_offset,y_offset,p_end_x_offset,p_end_y_offset,block_id);
}

int find_zone_ie(dl_map_ie *p_mapienode,stc_dl_zone_ie **pp_stc_zone_ie);
int phy_subframe_send_transform_dl_process(physical_subframe* p_phy_subframe,struct ofdma_map* p_ofdma_frame,dl_map_ie *p_map_link)//,dcd_msg *p_dcd_msg)
{
    
    phy_burst *p_dataunit = NULL,*p_burst_head = NULL;
   
    initialize_ofdma_map(p_ofdma_frame);
    p_burst_head = p_phy_subframe->burst_header;
    ERROR_RETURN(p_burst_head,NULL,"the header of p_phy_subframe is null");

    p_dataunit = p_burst_head;
    int block_id = 1;
#ifdef _INTEGRATE_PRE_SUBFRAME_

    int x_offset = 0, y_offset = 0,x_end_offset = 0,y_end_offset = 0;
    if(p_phy_subframe->fch_dl_map != NULL)
    {
        //ERROR_TRACE("the burst in phy_subframe isn't validate\n");
        //ERROR_TRACE("the fch_dl_map length is %d  ",p_phy_subframe->fch_dl_map_len);
        //u_int8_t buffer[24];
        //memset(buffer,0,sizeof(buffer));
        //memcpy(buffer,p_phy_subframe->fch_dl_map,6);
        fill_FCH_in_ofdma_frame(p_ofdma_frame,p_phy_subframe->fch_dl_map,6,x_offset,y_offset, &x_end_offset,&y_end_offset,block_id);
        block_id ++;
        x_offset = x_end_offset;
        y_offset = y_end_offset;
        p_ofdma_frame->ele[0][0].repetition_coding_indicating = 2;
        x_offset += 3;
    //p_dataunit = p_dataunit->next;
    
        fill_dl_map_in_ofdma_frame(p_ofdma_frame,p_phy_subframe->fch_dl_map + 24,p_phy_subframe->fch_dl_map_len - 24,x_offset,y_offset, &x_end_offset,&y_end_offset,block_id);
        block_id ++;
    }
#endif
    dl_map_ie     *pmapienode = p_map_link;    //point to dlmap's head,didn't any data ;


#ifndef _USE_STUBE_CODE_DL_MAP_
    //pmapienode = pmapienode->next;
#endif
    //pmapienode = pmapienode->next;
    //p_dataunit = p_dataunit->next;

    stc_dl_zone_ie *p_stc_zone_ie = NULL;
    find_zone_ie(pmapienode,&p_stc_zone_ie);
    //dl_burst_profile * pburstnode = p_dcd_msg->profile_header;
    while(p_dataunit != NULL)
    {
     
        
       
        //ERROR_RETURN(pmapienode,NULL,"error in dlmapmsg,the pointer of ie_head is null so return");


        //ERROR_RETURN(find_map_ienode_by_mapindex_dl(p_dataunit->map_burst_index,&pmapienode),-1,"failture in searching mapindex in function find_map_ienode_by_mapindex_dl");

#ifdef   _DEBUG

#else        
        ERROR_RETURN(pburstnode,NULL,"error in dcdmsg,didn't have any profile_header");
        
        ERROR_RETURN(find_burstnode_by_diuc_dl(pmapienode->diuc, &pburstnode),-1,"can't find burst_profile with the DIUC");
#endif

       
        //ERROR_RETURN(pmapienode->normal_ie,NULL,"error in normal_ie");
       
       

        int  x_end_offset = 0,y_end_offset = 0;
        //int mode = get_mode_by_diuc(pmapienode->diuc);

        int iresult = fill_ofdma_subframe_dl(&pmapienode,p_stc_zone_ie,p_dataunit,p_ofdma_frame,pmapienode->diuc,0,&x_end_offset,&y_end_offset,block_id); 
        //pburstnode = pburstnode->next;
	//change ofdma's slot and subch;
	changeofdma_frame_subchnum_slotnum(p_ofdma_frame, CURRENT_DL_OFDMA_SUBCH_NUM,y_end_offset);
             
        if(iresult == 0)
        {
            p_dataunit = p_dataunit->next; 

            pmapienode = pmapienode->next;
            block_id ++;
        }
    }
    //static int i_temp = 0;
    return 0;
}



int find_burstnode_by_diuc_dl(u_int8_t diuc, dl_burst_profile **pp_burstnode)
{
    while(pp_burstnode != NULL)
    {//find dl_burst_profile in dcdmsg	   
            
        ERROR_RETURN(pp_burstnode,NULL,"error in find dl_burst_profile");
        if((*pp_burstnode)->diuc == diuc)
        {
	     return 0;
        }
        (*pp_burstnode) = (*pp_burstnode)->next;
    }
    return -1;
       
}

int fill_normal_data(dl_map_ie **pp_mapienode,phy_burst *p_dataunit,struct ofdma_map* p_ofdma_frame,int modulation,int offset_ofdma,int *p_x_end_offset,int *p_y_end_offset,int block_id)
{
    int  bitscount = 0;
    int  ibitperslot = 0;
    while((*pp_mapienode) != NULL)
    {
        if((*pp_mapienode)->normal_ie != NULL)
        {
            if(*(*pp_mapienode)->normal_ie->cid == 0)
                modulation = OFDM_QPSK_1_2;
            ibitperslot=  GET_DL_BITS_PERSUBCH_PERSYM(modulation) *  CURRENT_DL_OFDMA_SLOT_LENGTH_INSYM;
            int temp = ((*pp_mapienode)->normal_ie->subchannels_num + (*pp_mapienode)->normal_ie->subchannel_offset);
            *p_x_end_offset = (*p_x_end_offset) > temp ?*p_x_end_offset:temp;
            temp = ((*pp_mapienode)->normal_ie->ofdma_Symbols_num + (*pp_mapienode)->normal_ie->ofdma_symbol_offset)/CURRENT_DL_OFDMA_SLOT_LENGTH_INSYM;
            *p_y_end_offset = (*p_y_end_offset)>temp?(*p_y_end_offset):temp;
            int i = (*pp_mapienode)->normal_ie->ofdma_symbol_offset/CURRENT_DL_OFDMA_SLOT_LENGTH_INSYM + offset_ofdma;
            int islotsnum = (*pp_mapienode)->normal_ie->ofdma_Symbols_num/CURRENT_DL_OFDMA_SLOT_LENGTH_INSYM + i;;

        //VERIFY_ERROR(itotalbits,p_dataunit->length,"fill data have error\n");;
        //
            for(; i < islotsnum ; i++)
            {
    
                int j = (*pp_mapienode)->normal_ie->subchannel_offset;
                for(; j < (*pp_mapienode)->normal_ie->subchannels_num + (*pp_mapienode)->normal_ie->subchannel_offset; j++)
                { 
	             //ERROR_RETURN(p_dataunit->burst_payload,NULL,"data's pointer is null\n"); 
            
      
                     int ioffsetbyte = bitscount/PERBITINBYTE;
                     p_ofdma_frame->ele[j][i].offset_for_uchar = (bitscount - ioffsetbyte*PERBITINBYTE);
                
                                  
                     p_ofdma_frame->ele[j][i].len_in_bit = ((bitscount + ibitperslot)/PERBITINBYTE <= p_dataunit->length)? \
                                                  ibitperslot:((p_dataunit->length*PERBITINBYTE - bitscount)) ;;
                    p_ofdma_frame->ele[j][i].block_id = block_id;
                    p_ofdma_frame->ele[j][i].repetition_coding_indicating = (*pp_mapienode)->normal_ie->repetition_coding_indication;
                //if((bitscount + ibitperslot)/PERBITINBYTE > p_dataunit->length)
                //{


                //}

                    p_ofdma_frame->ele[j][i].payload = p_dataunit->burst_payload + ioffsetbyte;
                    p_ofdma_frame->ele[j][i].mc_type = modulation;
                    bitscount += ((bitscount + ibitperslot)/PERBITINBYTE <= p_dataunit->length)? \
                                  ibitperslot:((p_dataunit->length*PERBITINBYTE - bitscount)) ;;;

                //calculate the byte in data;
                    p_ofdma_frame->ele[j][i].count_in_bytes = bitscount/PERBITINBYTE - ioffsetbyte;
                    if(bitscount%PERBITINBYTE != 0)
                        p_ofdma_frame->ele[j][i].count_in_bytes++;
                   if((*pp_mapienode)->normal_ie->cid == 0)
                       p_ofdma_frame->ele[j][i].is_broadcast = 1;
                   else
                       p_ofdma_frame->ele[j][i].is_broadcast = 0;
               //to do 
               //p_ofdma_frame->ele[j][i].mimo_mode = 0;
                    
               
                    if(bitscount/PERBITINBYTE >= p_dataunit->length)
                   {
                        return 0;;
                   }
                
                   }
                   burst_count ++;
              }
            if(bitscount/PERBITINBYTE < p_dataunit->length)     //for burst map to two map_ie  
            {
                 (*pp_mapienode) = (*pp_mapienode)->next;
            }
        }
     }
    //VERIFY_ERROR(isfullpad,-1,"data can't fill to rect completly");
    burst_count ++;
    return -1;
}
int find_zone_ie(dl_map_ie *p_mapienode,stc_dl_zone_ie **pp_stc_zone_ie)
{
    stc_dl_zone_ie *stc_ie;
    while(p_mapienode != NULL)
    {
        if(p_mapienode->extended_ie != NULL)
        {
            switch(p_mapienode->extended_ie->extended_diuc)
            {
                // Need to add more case blocks for other types of extd DIUC IDs
                case STC_ZONE_IE:
                    stc_ie = p_mapienode->extended_ie->unspecified_data;
#if 0
                    printf("STC Zone IE: OFDMA sym offset: %d, Perm: %d, All SC: %d, STC: %d, Matrix: %d, PermBase: %d, PRBS ID: %d,\
                        AMC: %d, Midamble present: %d, Midamble boost: %d, #Antenna: %d, Dedicated Pilots: %d\n", stc_ie->ofdma_symbol_offset,\
                        stc_ie->permutation, stc_ie->use_all_sc_indicator, stc_ie->stc, stc_ie->matrix_indicator ,stc_ie->dl_permbase ,\
                        stc_ie->prbs_id ,stc_ie->amc_type ,stc_ie->midamble_presence , stc_ie->midamble_boosting, stc_ie->num_antenna_select,\
                        stc_ie->dedicated_pilots); 
#endif
                    *pp_stc_zone_ie = stc_ie;
                    return 0;
                break;
                //default:
                    //printf("Can't print extended DIUC IE. Unknown type\n");
            } // end swicth
        }
        p_mapienode = p_mapienode->next;
    }
    return -1;
}
int fill_extended_data(dl_map_ie **pp_mapienode,stc_dl_zone_ie *p_stc_ie,phy_burst *p_dataunit,struct ofdma_map* p_ofdma_frame,int modulation,int offset_ofdma,int *p_x_end_offset,int *p_y_end_offset,int block_id)
{
    int  bitscount = 0;
    mimo_dl_basic_ie *mubi;
    if(modulation == 15 || modulation == 14)
        return 1;
    int ibitperslot=  GET_DL_BITS_PERSUBCH_PERSYM(modulation) *  CURRENT_DL_OFDMA_SLOT_LENGTH_INSYM;
    
    while((*pp_mapienode) != NULL)
    {
        if ((*pp_mapienode)->extended_ie != NULL)
        {
            if((*pp_mapienode)->extended_ie->unspecified_data != NULL)
            {
                if((*pp_mapienode)->diuc == 14 && p_stc_ie != NULL)
                { // for extended_ie will fill data
                    switch((*pp_mapienode)->extended_ie->extended_diuc)
                    {
                        case MIMO_DL_BASIC_IE:
                            mubi = (*pp_mapienode)->extended_ie->unspecified_data;
                            //printf("MIMO DL BASIC IE: Num region: %d\n", mubi->num_region);
                            region_attri *ra = mubi->region_header;
                            while (ra != NULL)
                            {
#if 0
                                printf("OFDM sym offset: %d, Subch offset: %d, boost: %d, #symbols: %d, #Subchannels: %d, Matrix: %d, #Layers: %d \n",\
                                  ra->ofdma_symbol_offset, ra->subchannel_offset, ra->boosting, ra->ofdma_symbols_num, ra->subchannels_num, ra->matrix_indicator,\
                                   ra->num_layer);
#endif

                                layer_attri *la = ra->layer_header;
                                while(la != NULL)
                                {
//                                     printf("CID: %d, Layer Index: %d, DIUC: %d, RCI: %d\n", la->cid, la->layer_index, la->diuc, la->repetition_coding_indication);
                                    la = la->next;
                                }

                                int temp = (ra->subchannels_num + ra->subchannel_offset);
                                *p_x_end_offset = (*p_x_end_offset) > temp ?*p_x_end_offset:temp;
                                temp = (ra->ofdma_symbols_num + ra->ofdma_symbol_offset)/CURRENT_DL_OFDMA_SLOT_LENGTH_INSYM;
                                *p_y_end_offset = (*p_y_end_offset)>temp?(*p_y_end_offset):temp;
                                int i = ra->ofdma_symbol_offset/CURRENT_DL_OFDMA_SLOT_LENGTH_INSYM + offset_ofdma;
                                int islotsnum = ra->ofdma_symbols_num/CURRENT_DL_OFDMA_SLOT_LENGTH_INSYM + i;;
                                for(; i < islotsnum ; i++)
                                {
                                    int j = ra->subchannel_offset;
                                    for(; j < ra->subchannels_num + ra->subchannel_offset; j++)
                                    { 
                                       //ERROR_RETURN(p_dataunit->burst_payload,NULL,"data's pointer is null\n"); 
                                       int ioffsetbyte = bitscount/PERBITINBYTE;
                                       p_ofdma_frame->ele[j][i].offset_for_uchar = (bitscount - ioffsetbyte*PERBITINBYTE);                
                                  
                                       p_ofdma_frame->ele[j][i].len_in_bit = ((bitscount + ibitperslot)/PERBITINBYTE <= p_dataunit->length)? \
                                                  ibitperslot:((p_dataunit->length*PERBITINBYTE - bitscount)) ;;
                                       p_ofdma_frame->ele[j][i].block_id = block_id;
                                       p_ofdma_frame->ele[j][i].payload = p_dataunit->burst_payload + ioffsetbyte;
                                       p_ofdma_frame->ele[j][i].mc_type = modulation;
                                       bitscount += ((bitscount + ibitperslot)/PERBITINBYTE <= p_dataunit->length)? \
                                           ibitperslot:((p_dataunit->length*PERBITINBYTE - bitscount)) ;;;

                                       p_ofdma_frame->ele[j][i].count_in_bytes = bitscount/PERBITINBYTE - ioffsetbyte;
                                       if(bitscount%PERBITINBYTE != 0)
                                       p_ofdma_frame->ele[j][i].count_in_bytes++;
                                       p_ofdma_frame->ele[j][i].is_broadcast = 0;
                                       p_ofdma_frame->ele[j][i].repetition_coding_indicating = ra->layer_header->repetition_coding_indication;
                                       //printf("matrix_indic %d\n", p_stc_ie->matrix_indicator );
                                       p_ofdma_frame->ele[j][i].mimo_mode = p_stc_ie->matrix_indicator + 1 ;
               
                                       if(bitscount/PERBITINBYTE >= p_dataunit->length)
                                       {
                                           return 0;;
                                       }
                                    }
                                   burst_count ++;
                                }
                                ra = ra->next;
                            }
                            if(bitscount/PERBITINBYTE < p_dataunit->length)     //for burst map to two map_ie  
                            {
                                (*pp_mapienode) = (*pp_mapienode)->next; // not  will do it 
                            }
                            burst_count ++;
                            break;
                        default:
                            FLOG_ERROR("Can't print extended DIUC 2 IE. Unknown type\n");
                    }
                }
                else 
                {
                    return 1;
                }
            }
            else 
                return 1;
        }
    }
    //
    //printf("the data length is %d \n",p_dataunit->length);
    //
    return -1;
}

int fill_ofdma_subframe_dl(dl_map_ie **pp_mapienode,void *unspecified_data,phy_burst *p_dataunit,struct ofdma_map* p_ofdma_frame,int modulation,int offset_ofdma,int *p_x_end_offset,int *p_y_end_offset,int block_id)
{
    int result = 0;
    stc_dl_zone_ie *p_stc_zone_ie = (stc_dl_zone_ie *)unspecified_data;
    if((*pp_mapienode)->normal_ie != NULL)
        result = fill_normal_data(pp_mapienode,p_dataunit,p_ofdma_frame,modulation,offset_ofdma,p_x_end_offset,p_y_end_offset,block_id);
    if((*pp_mapienode)->extended_ie != NULL)
        result = fill_extended_data(pp_mapienode,p_stc_zone_ie,p_dataunit,p_ofdma_frame,modulation,offset_ofdma,p_x_end_offset,p_y_end_offset,block_id);
    if(result == 1)
    {
       (*pp_mapienode) = (*pp_mapienode)->next;
    }
    return result;
}


int isvalidinrect(int rectw,int rectl, int datastartx,int datastarty,int dataw,int datal)
{
    if(datastartx <= 0 || datastarty <= 0 || rectw <= 0 || rectl <=0 || dataw <=0 || datal <=0)
        return 1;
    if(datastartx > rectw ||  (datastartx + dataw) > rectw)
        return 1;
    if(datastarty > rectl || (datastarty + dataw) > rectl)
        return 1;
    return 0;
}


int changeofdma_frame_subchnum_slotnum(struct ofdma_map* p_ofdma_frame, int isubchnum,int islotnum)
{
  //if(p_ofdma_frame->subchannel_num < isubchnum)
  //    p_ofdma_frame->subchannel_num = isubchnum;
     p_ofdma_frame->subchannel_num = isubchnum; 
  if(p_ofdma_frame->slot_num < islotnum)
      p_ofdma_frame->slot_num = islotnum;
  return 0;
}

int transform_symbolslot_singal(struct ofdma_map  *p_ofdma_frame,int islotnum,int frame_index,struct phy_dl_slotsymbol ** p_phy)
{
    int iloop = 0;
    u_int8_t  mimo_flag = 0;
    u_int8_t  broadcast_flag = 0;
    struct phy_dl_slot *p_slot_phy_node = NULL,*p_pre_slot_node = NULL;;
    int countbyte = 0;
    int ioffset = 0;
    if(adapter_malloc_physlotsymbol(p_phy) == -1)
    {
         FLOG_FATAL("system malloc memory failed\n");
         return -1;
    }
    
    //printf("count  ");
    for(iloop = 0; iloop < p_ofdma_frame->subchannel_num; iloop++)
    {
        if (p_ofdma_frame->ele[iloop][islotnum].len_in_bit < 0 )
        {
            FLOG_FATAL("error in process ofdma_transform_phy_slot \n");
            return -1;
        }
        


        if(p_ofdma_frame->ele[iloop][islotnum].len_in_bit != 0)
        {
             int perslotbits = GET_DL_BITS_PERSUBCH_PERSYM(p_ofdma_frame->ele[iloop][islotnum].mc_type) *  CURRENT_DL_OFDMA_SLOT_LENGTH_INSYM;

            if(p_ofdma_frame->ele[iloop][islotnum].len_in_bit > perslotbits)
                countbyte += p_ofdma_frame->ele[iloop][islotnum].len_in_bit;
            else 
                countbyte += perslotbits;
        }
        else
            countbyte += PADDING_LENGTH;
        
    }
    u_int8_t *ptotalbuff = (u_int8_t*)malloc(sizeof(u_int8_t) * countbyte);
    (*p_phy)->p_payload_buf = ptotalbuff;
    for(iloop = 0; iloop < p_ofdma_frame->subchannel_num; iloop++)
    {	  
        if(adapter_malloc_physlotnode(&p_slot_phy_node) == -1)
            return -1;
        //memset(p_slot_phy_node,0,sizeof(p_slot_phy_node));
        p_slot_phy_node->repetition_coding_indication = p_ofdma_frame->ele[iloop][islotnum].repetition_coding_indicating;
        
        p_slot_phy_node->coding_type = 0;
        p_slot_phy_node->is_broadcast = p_ofdma_frame->ele[iloop][islotnum].is_broadcast;
        broadcast_flag = p_slot_phy_node->is_broadcast;
        p_slot_phy_node->mimo_mode = p_ofdma_frame->ele[iloop][islotnum].mimo_mode;
        p_slot_phy_node->block_id = p_ofdma_frame->ele[iloop][islotnum].block_id;
        mimo_flag = p_slot_phy_node->mimo_mode;

        if(p_ofdma_frame->ele[iloop][islotnum].len_in_bit != 0)
        {//data burst
            u_int8_t *pdatabuff = ptotalbuff + ioffset;
            int perslotbits = GET_DL_BITS_PERSUBCH_PERSYM(p_ofdma_frame->ele[iloop][islotnum].mc_type) *  CURRENT_DL_OFDMA_SLOT_LENGTH_INSYM;

            bit_to_byte_f(p_ofdma_frame->ele[iloop][islotnum].payload,p_ofdma_frame->ele[iloop][islotnum].offset_for_uchar,p_ofdma_frame->ele[iloop][islotnum].len_in_bit,p_ofdma_frame->ele[iloop][islotnum].len_in_bit,pdatabuff);
            
            p_slot_phy_node->payload = pdatabuff;
            p_slot_phy_node->bytelength = perslotbits;//p_ofdma_frame->ele[iloop][islotnum].len_in_bit;
            p_slot_phy_node->code_id = p_ofdma_frame->ele[iloop][islotnum].mc_type;
            p_slot_phy_node->unused_flag = 0;
            if(p_ofdma_frame->ele[iloop][islotnum].len_in_bit < perslotbits )
            {
                memset (pdatabuff + p_ofdma_frame->ele[iloop][islotnum].len_in_bit, 0, perslotbits - p_ofdma_frame->ele[iloop][islotnum].len_in_bit);
                ioffset += perslotbits;
            }
            else 
                ioffset += p_ofdma_frame->ele[iloop][islotnum].len_in_bit;
        }
        else {//for padding modulation is QPSK_1/2 the data is 0x00
            //u_int8_t *pdatabuff = (u_int8_t*)malloc(sizeof(u_int8_t) * PADDING_LENGTH);
            u_int8_t *pdatabuff = ptotalbuff + ioffset;

            memset(pdatabuff,0,sizeof(u_int8_t) * PADDING_LENGTH);
            p_slot_phy_node->payload = pdatabuff;
            p_slot_phy_node->code_id = -1;
            p_slot_phy_node->unused_flag = 1;
            p_slot_phy_node->bytelength = 0;//PADDING_LENGTH;

            ioffset += PADDING_LENGTH;
        }

        if((*p_phy)->slot_header == NULL)
        {
            (*p_phy)->slot_header = p_slot_phy_node;
            p_pre_slot_node = p_slot_phy_node;
            continue;
        }
        p_pre_slot_node->next = p_slot_phy_node;
        p_pre_slot_node = p_slot_phy_node;
    }
    (*p_phy)->mimo_mode = mimo_flag;
    (*p_phy)->slotlength = p_ofdma_frame->subchannel_num;
    (*p_phy)->symboloffset = CURRENT_DL_OFDMA_SLOT_LENGTH_INSYM * islotnum + 1;
    (*p_phy)->frame_index = frame_index;
    (*p_phy)->dl_subframe_end_flag = 0;
    (*p_phy)->is_broadcast = broadcast_flag;
    (*p_phy)->p_dts_info = p_ofdma_frame->p_dts_info;
    (*p_phy)->p_ulmap = p_ofdma_frame->p_ulmap;
    (*p_phy)->ul_map_len = p_ofdma_frame->ul_map_len;
    (*p_phy)->dl_perscan_flag = p_ofdma_frame->dl_perscan_flag;
    return 0;
}

int adapter_transform_symbolslot_1(struct ofdma_map  *p_ofdma_frame,int islotnum,int frame_index,struct phy_dl_slotsymbol ** p_phy)
{
    return transform_symbolslot_singal(p_ofdma_frame,islotnum,frame_index,p_phy);
}

int set_symslot_zone_info(struct phy_dl_slotsymbol *p_phy,dl_map_msg *dl_map);

int transform_symbolslot_double(struct ofdma_map  *p_ofdma_frame,int islotnum,int frame_index,struct phy_dl_slotsymbol **const p_phy1 ,struct phy_dl_slotsymbol **const p_phy2)
{
    int iloop = 0;
    u_int8_t  mimo_flag = 0;
    u_int8_t  broadcast_flag = 0;
    struct phy_dl_slot *p_slot_phy_node1 = NULL,*p_slot_phy_node2 = NULL,*p_pre_slot_node1 = NULL,*p_pre_slot_node2 = NULL;;
    int countbyte = 0;
    int ioffset = 0;
    if(adapter_malloc_physlotsymbol(p_phy1) == -1)
    {
         FLOG_ERROR("system malloc memory failed\n");
         return -1;
    }
    if(adapter_malloc_physlotsymbol(p_phy2) == -1)
    {
         FLOG_ERROR("system malloc memory failed\n");
         return -1;
    }
    for(iloop = 0; iloop < p_ofdma_frame->subchannel_num; iloop++)
    {
        if (p_ofdma_frame->ele[iloop][islotnum].len_in_bit < 0 )
        {
            FLOG_ERROR("error in process ofdma_transform_phy_slot \n");
            return -1;
        }

        if(p_ofdma_frame->ele[iloop][islotnum].len_in_bit == 0)
            countbyte += PADDING_LENGTH;
        else 
        {
             int perslotbits = GET_DL_BITS_PERSUBCH_PERSYM(p_ofdma_frame->ele[iloop][islotnum].mc_type) *  CURRENT_DL_OFDMA_SLOT_LENGTH_INSYM;

            if(p_ofdma_frame->ele[iloop][islotnum].len_in_bit > perslotbits)
                countbyte += p_ofdma_frame->ele[iloop][islotnum].len_in_bit;
            else 
                countbyte += perslotbits;
        }
        
    }
    u_int8_t *ptotalbuff1 = (u_int8_t*)malloc(sizeof(u_int8_t) * countbyte);
    u_int8_t *ptotalbuff2 = (u_int8_t*)malloc(sizeof(u_int8_t) * countbyte);

 
    for(iloop = 0; iloop < p_ofdma_frame->subchannel_num; iloop++)
    {	  
        if(adapter_malloc_physlotnode(&p_slot_phy_node1) == -1)
            return -1;
        if(adapter_malloc_physlotnode(&p_slot_phy_node2) == -1)
            return -1;

        //memset(p_slot_phy_node1,0,sizeof(p_slot_phy_node1));
        p_slot_phy_node1->repetition_coding_indication = p_ofdma_frame->ele[iloop][islotnum].repetition_coding_indicating;
        p_slot_phy_node1->coding_type = 0;
        p_slot_phy_node1->is_broadcast = p_ofdma_frame->ele[iloop][islotnum].is_broadcast;
        p_slot_phy_node1->mimo_mode = p_ofdma_frame->ele[iloop][islotnum].mimo_mode;
        p_slot_phy_node1->block_id = p_ofdma_frame->ele[iloop][islotnum].block_id;

        //memset(p_slot_phy_node2,0,sizeof(p_slot_phy_node2));
        p_slot_phy_node2->repetition_coding_indication = p_ofdma_frame->ele[iloop][islotnum].repetition_coding_indicating;
        p_slot_phy_node2->coding_type = 0;
        p_slot_phy_node2->is_broadcast = p_ofdma_frame->ele[iloop][islotnum].is_broadcast;
        p_slot_phy_node2->mimo_mode = p_ofdma_frame->ele[iloop][islotnum].mimo_mode;
        p_slot_phy_node2->block_id = p_ofdma_frame->ele[iloop][islotnum].block_id;

        mimo_flag = p_slot_phy_node1->mimo_mode;
        broadcast_flag = p_slot_phy_node1->is_broadcast;
        
        //for  mimo mode, splite the slot with two data buffer. 
            if(p_ofdma_frame->ele[iloop][islotnum].len_in_bit != 0)
            {//data burst
                u_int8_t *pdatabuff1 = ptotalbuff1 + ioffset;
                u_int8_t *pdatabuff2 = ptotalbuff2 + ioffset;
                int perslotbits = GET_DL_BITS_PERSUBCH_PERSYM(p_ofdma_frame->ele[iloop][islotnum].mc_type) *  CURRENT_DL_OFDMA_SLOT_LENGTH_INSYM/2;
             
                bit_to_byte_f_2buf(p_ofdma_frame->ele[iloop][islotnum].payload,p_ofdma_frame->ele[iloop][islotnum].offset_for_uchar,p_ofdma_frame->ele[iloop][islotnum].len_in_bit,p_ofdma_frame->ele[iloop][islotnum].len_in_bit,pdatabuff1,pdatabuff2);
            
                p_slot_phy_node1->payload = pdatabuff1;
                p_slot_phy_node1->bytelength = perslotbits;//p_ofdma_frame->ele[iloop][islotnum].len_in_bit;
                p_slot_phy_node1->code_id = p_ofdma_frame->ele[iloop][islotnum].mc_type;

                p_slot_phy_node2->payload = pdatabuff2;
                p_slot_phy_node2->bytelength = perslotbits;//p_ofdma_frame->ele[iloop][islotnum].len_in_bit;
                p_slot_phy_node2->code_id = p_ofdma_frame->ele[iloop][islotnum].mc_type;
                if(p_ofdma_frame->ele[iloop][islotnum].len_in_bit < perslotbits )
                {
                    memset (pdatabuff1 + p_ofdma_frame->ele[iloop][islotnum].len_in_bit, 0, (perslotbits - p_ofdma_frame->ele[iloop][islotnum].len_in_bit)/2);
                    memset (pdatabuff2 + p_ofdma_frame->ele[iloop][islotnum].len_in_bit, 0, (perslotbits - p_ofdma_frame->ele[iloop][islotnum].len_in_bit)/2);

                    ioffset += perslotbits;
                }
                else 
                    ioffset += p_ofdma_frame->ele[iloop][islotnum].len_in_bit / 2;
            }
            else {
                //for padding modulation is QPSK_1/2 the data is 0x00
                //u_int8_t *pdatabuff = (u_int8_t*)malloc(sizeof(u_int8_t) * PADDING_LENGTH);
                u_int8_t *pdatabuff1 = ptotalbuff1 + ioffset;

                memset(pdatabuff1,0,sizeof(u_int8_t) * PADDING_LENGTH);
                p_slot_phy_node1->payload = pdatabuff1;
                p_slot_phy_node1->code_id = OFDM_QPSK_1_2;
                p_slot_phy_node1->bytelength = PADDING_LENGTH;

                ioffset += PADDING_LENGTH;


                u_int8_t *pdatabuff2 = ptotalbuff2 + ioffset;

                memset(pdatabuff2,0,sizeof(u_int8_t) * PADDING_LENGTH);
                p_slot_phy_node2->payload = pdatabuff2;
                p_slot_phy_node2->code_id = OFDM_QPSK_1_2;
                p_slot_phy_node2->bytelength = PADDING_LENGTH;

                ioffset += PADDING_LENGTH;
            }
            if((*p_phy1)->slot_header == NULL)
            {
                (*p_phy1)->slot_header = p_slot_phy_node1;
                p_pre_slot_node1 = p_slot_phy_node1;
                continue;
            }
            p_pre_slot_node1->next = p_slot_phy_node1;
            p_pre_slot_node1 = p_slot_phy_node1;

            if((*p_phy2)->slot_header == NULL)
            {
                (*p_phy2)->slot_header = p_slot_phy_node2;
                p_pre_slot_node1 = p_slot_phy_node2;
                continue;
            }
            p_pre_slot_node2->next = p_slot_phy_node2;
            p_pre_slot_node2 = p_slot_phy_node2;
    }
    (*p_phy1)->mimo_mode = mimo_flag;
    (*p_phy1)->slotlength = p_ofdma_frame->subchannel_num;
    (*p_phy1)->symboloffset = CURRENT_DL_OFDMA_SLOT_LENGTH_INSYM * islotnum + 1;
    (*p_phy1)->frame_index = frame_index;
    (*p_phy1)->dl_subframe_end_flag = 0;
    (*p_phy1)->is_broadcast = broadcast_flag;

    (*p_phy2)->mimo_mode = mimo_flag;
    (*p_phy2)->slotlength = p_ofdma_frame->subchannel_num;
    (*p_phy2)->symboloffset = CURRENT_DL_OFDMA_SLOT_LENGTH_INSYM * islotnum + 1;
    (*p_phy2)->frame_index = frame_index;
    (*p_phy2)->dl_subframe_end_flag = 0;
    (*p_phy2)->is_broadcast = broadcast_flag;

    
    set_symslot_zone_info((*p_phy1),dl_map);
    set_symslot_zone_info((*p_phy2),dl_map);
    
    return 0;
}

int adapter_transform_symbolslot_2(struct ofdma_map  *p_ofdma_frame,int islotnum,int frame_index,struct phy_dl_slotsymbol **p_phy1 ,struct phy_dl_slotsymbol **p_phy2)
{
    return  transform_symbolslot_double(p_ofdma_frame,islotnum,frame_index, p_phy1 , p_phy2);
}

int set_symslot_zone_info(struct phy_dl_slotsymbol *p_phy,dl_map_msg *dl_map)
{
    stc_dl_zone_ie *stc_ie;
    mimo_dl_basic_ie *mubi;
    struct phy_dl_slot *p_slot_phy_node = NULL;
    dl_map_ie *ie = dl_map->ie_head;
    while (ie != NULL)
    {
        printf("\nIE index: %d, DIUC: %d\n", ie->ie_index, ie->diuc);
        if (ie->normal_ie != NULL)
        {
            printf("N_CID: %d, CID: %d, OFDMA symbol offset: %d, subchannel offset: %d, num OFDMA symbols: %d, num subchannels: %d",\
            ie->normal_ie->n_cid, ie->normal_ie->cid[0], ie->normal_ie->ofdma_symbol_offset, ie->normal_ie->subchannel_offset, ie->normal_ie->ofdma_Symbols_num,\
                ie->normal_ie->subchannels_num);
        }
        else if (ie->extended_ie != NULL)
        {
            printf("Extended IE DIUC code: %d, Length: %d\n", ie->extended_ie->extended_diuc, ie->extended_ie->length);
            if(ie->extended_ie->unspecified_data != NULL)
            {
                if(ie->diuc == 14)
                {
                    switch(ie->extended_ie->extended_diuc)
                    {
                        case MIMO_DL_BASIC_IE:
                            mubi = ie->extended_ie->unspecified_data;
                            printf("MIMO DL BASIC IE: Num region: %d\n", mubi->num_region);
                            region_attri *ra = mubi->region_header;
                            while (ra != NULL)
                            {
                                printf("OFDM sym offset: %d, Subch offset: %d, boost: %d, #symbols: %d, #Subchannels: %d, Matrix: %d, #Layers: %d \n",\
                                  ra->ofdma_symbol_offset, ra->subchannel_offset, ra->boosting, ra->ofdma_symbols_num, ra->subchannels_num, ra->matrix_indicator,\
                                   ra->num_layer);

                                layer_attri *la = ra->layer_header;
                                while(la != NULL)
                                {
                                     printf("CID: %d, Layer Index: %d, DIUC: %d, RCI: %d\n", la->cid, la->layer_index, la->diuc, la->repetition_coding_indication);
                                    la = la->next;
                                }
                                ra = ra->next;
                            }
                        break;
                        default:
                            printf("Can't print extended DIUC 2 IE. Unknown type\n");
                    }
                }
                else if (ie->diuc == 15)
                {
                    switch(ie->extended_ie->extended_diuc)
                    {
                        // Need to add more case blocks for other types of extd DIUC IDs
                        case STC_ZONE_IE:
                            stc_ie = ie->extended_ie->unspecified_data;
#if 0
                            printf("STC Zone IE: OFDMA sym offset: %d, Perm: %d, All SC: %d, STC: %d, Matrix: %d, PermBase: %d, PRBS ID: %d,\
                                    AMC: %d, Midamble present: %d, Midamble boost: %d, #Antenna: %d, Dedicated Pilots: %d\n", stc_ie->ofdma_symbol_offset,\
                                    stc_ie->permutation, stc_ie->use_all_sc_indicator, stc_ie->stc, stc_ie->matrix_indicator ,stc_ie->dl_permbase ,\
                                    stc_ie->prbs_id ,stc_ie->amc_type ,stc_ie->midamble_presence , stc_ie->midamble_boosting, stc_ie->num_antenna_select,\
                                    stc_ie->dedicated_pilots); 
#endif
                            if(p_phy->symboloffset == stc_ie->ofdma_symbol_offset)
                            {
                                p_phy->mimo_mode = param_DL_STC_MATRIX_TYPE  + 1;    //-1: No STC, 0: Matrix A, 1: Matrix B, 2: Matrix C/
                                p_slot_phy_node = p_phy->slot_header;
                                while(p_slot_phy_node != NULL)
                                {
                                    p_slot_phy_node->mimo_mode = param_DL_STC_MATRIX_TYPE  + 1;///-1: No STC, 0: Matrix A, 1: Matrix B, 2: Matrix C
                                }                            
                                return 1;
                            }
                            break;
                        default:
                            printf("Can't print extended DIUC IE. Unknown type\n");
                    } // end swicth
                }
            }
        }
        ie = ie->next;
    }
    return 0;
}


