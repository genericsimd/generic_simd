#include "mac_headermsg_builder.h"
#include "dlmap.h"
enum  data_attri{
  SYM_OFFSET  = 0,
  SUBCH_OFFSET,
  SYM_NUM,
  SUBCH_NUM,
  CID,
  IE_INDEX,
  DIUC
};
const int subch_sym[][7]=
  {{4,0,3,32,102,0,0},{7,0,3,5,103,1,2},{7,6,3,15,104,2,0},{7,21,3,22,105,3,4}};

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
          
int main()
{
    crc_init();
    dl_map_msg  *p_dlmap;
    generate_dlmap(&p_dlmap);
    u_char  payload[1024];
    memset(payload,0,sizeof(u_char) * 1024);
    dl_subframe_prefix   dlp;
    dlp.used_subchannel_bitmap = 0;
    dlp.rsv1 = 0;
    dlp.repetition_coding_indication = 0;
    dlp.coding_indication = 1;
    dlp.dl_map_length = 13; // define the length in slots of the burst which contains only DL-MAP message or compressed DL-MAP messge and compressed UL-MAP.
    dlp.rsv2 = 0;
    dlp.p_dts_info = NULL;
 
    int     length = 0;
    int len = 0;
    
    build_dlframeprefix(&dlp, payload, 0, &length);
    
    build_dlmap(p_dlmap, &payload[length], &len);
    length += len;  
    for(len = 0; len < length; len++)
    {
       printf("0x%x,",payload[len]);
    }
    printf("\n");
    printf("build payload length %d\n",length);
}
