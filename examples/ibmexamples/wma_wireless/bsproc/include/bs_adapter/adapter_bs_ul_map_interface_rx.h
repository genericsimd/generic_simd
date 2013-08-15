#ifndef ADAPTER_BS_UL_MAP_INTERFACE_RX_H_
#define ADAPTER_BS_UL_MAP_INTERFACE_RX_H_

//////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////MAC->PHY Interface /////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////
//UL_BS 
//PHY -> MAC Interface for PHY Receiver output
////////////////////////////////////////////

#include "adapter_config.h"
#define UL_MAP_IE_NUM            8
enum ul_ie_type
{
    adapter_data_uiuc_ie_indic  =  1,    //this ie is generate from adapter
    uiuc12_ie_indic,                    //this ie is for ranging ie
    extended_uiuc_ie_indic,             //
    extended_uiuc_ie_15_indic,             //
    cdma_alloc_ie_indic,
    paprreduc_safezone_alloc_ie_indic,
    fastfeedback_alloc_ie_indic,
    other_uiuc_ie_indic
};

struct adapter_data_ie
{
    u_int32_t      uiuc;
    u_int16_t      *cid;
    u_int8_t       n_cid;
    u_int8_t       ofdma_symbol_offset;
    u_int8_t       subchannel_offset;
    u_int8_t       ofdma_symbol_num;	
    u_int8_t       subchannel_num;
    u_int8_t       repetition_coding_indication;
    u_int16_t      slot_num;
    u_int8_t       coding_type;          //0 CC 1 CTC 
    u_int8_t       mimo_mode;                 //0 siso 1 stcmatrixa2x1  2 stcmatrixb2x2
    u_int8_t       burst_id;          //this id is for identify burst from one with another
    u_int16_t      len;               //for data length for bit_in_byte
};
struct adapter_ul_map_ie_node
{
    u_int8_t       map_ie_nums;
    u_int8_t       ie_type_arr[UL_MAP_IE_NUM];  //the para is to identify uiuc_ie type that *p_ie_unit pointer
    void           *p_ie_unit;   //notice the param is a pointer array,  it will push more than one pointer into this array, and each pointer's ie type is determined by the ie_type_arr. The first ie must be data_uiuc_ie,if there is no  data_uiuc_ie, the first ie will be set NULL and ie_type will be set to zero '0'.    
    struct         adapter_ul_map_ie_node *next;      
};

struct adapter_ul_map_msg {
    u_int8_t  manage_msg_type;
    u_int8_t  rsv;
    u_int8_t  ucd_count;
    u_int32_t alloc_start_time;
    u_int8_t  ulmap_ie_num;
    struct adapter_ul_map_ie_node*  ie_header;
};

//function:phy_ofdma_ul_get_ulmap
//description:get ulmap by frame_num,the frame_num will be supplied by framework,return the function from mac
//int adapter_get_mac_ulmap(int frame_num,ul_map_msg **ulmap_msg); 

//function:phy_ofdma_ul_process_ulmap
//description:transfrom regular ulmap to adapter_ulmap
int adapter_init_adapter_ulmap(int frame_num, struct adapter_ul_map_msg **adapter_ulmap);
//int adapter_ofdma_ul_process_ulmap(ul_map_msg *ulmap_msg, adapter_ul_map_msg **adapter_ulmap);

int adapter_malloc_ulmap_node(struct adapter_ul_map_ie_node ** pp_adapter_ulmap_node);
int adapter_release_ulmap_node(struct adapter_ul_map_ie_node ** pp_adapter_ulmap_node);

int adapter_deinit_adapter_ulmap(struct adapter_ul_map_msg ** adapter_ulmap);
//int phy_ofdma_ul_release_ulmap_msg(ul_map_msg **ul_map);
//int adapter_free_mac_ulmap(ul_map_msg **ul_map);

#endif
