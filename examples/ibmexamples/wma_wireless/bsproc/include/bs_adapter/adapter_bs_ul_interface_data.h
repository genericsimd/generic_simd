/* ----------------------------------------------------------------------------
 * IBM Confidential

 IBM Wireless M2M Platform

 (C)Copyright IBM Corp. 2009, 2010, 2011

 All Rights Reserved.

 File Name: adapter_bs_ul_interface_data.h

 Change Activity:

 Date             Description of Change                            By
 -----------      ---------------------                            --------
 3-Mar 2011       Created                                         jian jun chang 

 ----------------------------------------------------------------------------
 PROLOG END TAG zYx                                                         */
#ifndef ADAPTER_BS_UL_INTERFACE_DATA_H_
#define ADAPTER_BS_UL_INTERFACE_DATA_H_
#include "adapter_config.h"

#define    MAX_UL_SUBFRAME_SLOT_NUM         40
#define    MAX_UL_SUBCHANNEL_NUM              40

#define    UL_NETFRAME_BUFFER_SIZE             2048
#define    UL_DATASTARTPOS    4


struct ss_ul_ofdma_sym_subchannel
{
    u_int8_t* payload;
    int len_in_bit;
    int offset_for_uchar;                 //define for how many bits will be move from payload; 
    int count_in_bytes;
    u_int8_t    mc_type;
/************for temp record***********/
    u_int8_t *p_srcpayload;
    int   burst_id;
    int   data_length;
/************************************************************/
};

struct ss_ul_ofdma_sym_frame
{
    int sym_num;
    int subchannel_num;
    struct ss_ul_ofdma_sym_subchannel ele_sub_slot[MAX_UL_SUBCHANNEL_NUM][MAX_UL_SUBFRAME_SLOT_NUM];
};

/*
 *
 *
 */
struct ul_assist_data
{
/*
    u_int32_t       xoffset;
    u_int32_t       yoffset;
    u_int32_t       burst_id;
    u_int32_t       is_used;                        //0 for not use, 1 is used.
    u_int32_t       mode_id;
    u_int32_t       data_length;
    u_int32_t       af_dem_offset;
    u_int32_t       be_dem_offset;
    u_int32_t       uiuc;
    u_int32_t       slots_num;
    u_int32_t       repetition_coding_indication;
    u_int32_t       coding_type;          //0 CC 1 CTC
    u_int32_t       mimo_mode;                 //0 siso 1 stcmatrixa2x1  2 stcmatrixb2x2
    u_int32_t       coding_rate;
*/
    int32_t       xoffset;
    int32_t       yoffset;
    int32_t       burst_id;
    int32_t       cid;
    int32_t       is_used;                        //0 for not use, 1 is used.
    int32_t       mode_id;
    int32_t       data_length;
    int32_t       af_dem_offset;
    int32_t       be_dem_offset;
    int32_t       uiuc;
    int32_t       slots_num;
    int32_t       repetition_coding_indication;
    int32_t       coding_type;          //0 CC 1 CTC
    int32_t       mimo_mode;                 //0 siso 1 stcmatrixa2x1  2 stcmatrixb2x2
    int32_t       coding_rate;
};
//this data struct is  used for  phy-layer transmite data to mac-layer. 
//like this
//    |-----------|        |-------|        |------|       |-------|        |----------|
//    |  SS's mac | -----> |adapter| ------>| phy  | ----->|adapter| -----> | BS's mac |
//    |-----------|        |-------|        |------|       |-------|        |----------|
//where is use in 'phy->adapter'
struct ul_slot_node                         
{
    int            framenum;
    //------------------------------
    u_int8_t       *p_payload;
    int            ibytelength;
    int            ibitlength;
    int            ibitoffset;

    u_int8_t       uflag;                   //marked for identify the node is last in whole frame;
    struct ul_slot_node   *next;
};


struct ul_slot_node * get_ul_slot_node();
int    free_ul_slot_link(struct ul_slot_node *p_slot_node);

struct   net_node
{
    int             ibytelength;  
    int             ibitlength;
    int             ibitoffset;
    u_int8_t        *p_payload;
    struct net_node         *next;
};


struct ul_net_frame
{
    long            framenum;
    struct net_node         *p_net_node_data;
};

struct ul_net_frame*  get_new_ul_net_frame();
int free_ul_net_frame(struct ul_net_frame *p_ul_net_frame);

#endif
