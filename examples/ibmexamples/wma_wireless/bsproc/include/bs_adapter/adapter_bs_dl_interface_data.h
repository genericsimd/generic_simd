/* ----------------------------------------------------------------------------
 * IBM Confidential

 IBM Wireless M2M Platform

 (C)Copyright IBM Corp. 2009, 2010, 2011

 All Rights Reserved.

 File Name: adapter_bs_dl_interface_data.h

 Change Activity:

 Date             Description of Change                            By
 -----------      ---------------------                            --------
 13-Feb 2011       Created                                         jian jun chang 

 ----------------------------------------------------------------------------
 PROLOG END TAG zYx                                                         */

#ifndef _ADAPTER_BS_DL_INTERFACE_DATA_H_
#define _ADAPTER_BS_DL_INTERFACE_DATA_H_

#include "adapter_config.h"

#define PACKLENGTHSTARTPOS         0
#define PACKLENGTHSIZE             sizeof(u_int32_t)
#define PACKNUMSTARTPOS            (sizeof(u_int32_t) + 0)
#define PACKNUMSIZE                sizeof(u_int8_t)
#define PACKTYPESTARTPOS           (sizeof(u_int8_t)+sizeof(u_int32_t)) 
#define PACKTYPESIZE               sizeof(u_int8_t)   
#define PARAMETERNUMSTARTPOS       (sizeof(u_int8_t)+sizeof(u_int32_t) + sizeof(u_int8_t) ) 
#define PARAMETERNUMSIZE           sizeof(u_int32_t)   

#define SYSTEMPARANUM              12
#define TRANMISSBLEPARANUM         6       

// calcuated according to the 5ms ofdma frame, the dl subframe has 26~35 symbol, overall frame has 47 symbol
#define MAX_SUBFRAME_SLOT_NUM 60
// calculated with 5ms ofdma frame, the dl subframe may be 
#define MAX_SUBFRAME_SYMBOL_NUM 60

#define MAX_SUBCHANNEL_NUM 60

#define NET_FRAME_BUFFER_SIZE           2048

#define SLOTSYMBOL_NUM 3


struct ofdma_ele
{
    u_int8_t* payload;
    int len_in_bit;
    int offset_for_uchar;                 //define for how many bits will be move from payload; 
    int count_in_bytes;
    u_int8_t  mc_type;
    u_int8_t  mimo_mode;
    u_int8_t  is_broadcast;
    u_int8_t  repetition_coding_indicating;
    u_int8_t  block_id;
};

struct ofdma_map
{
    int slot_num;
    int subchannel_num;
    void              *p_dts_info;          
    void              *p_ulmap;
    int                ul_map_len;
    struct ofdma_ele ele[MAX_SUBFRAME_SLOT_NUM][MAX_SUBFRAME_SYMBOL_NUM];
    int dl_perscan_flag;
};


int initialize_ofdma_map(struct ofdma_map* p_ofdma_frame);

int release_ofdma_map(struct ofdma_map* p_ofdma_frame);


struct slot_node{
    //int flag;                            //flot for current state,10:start  00:middle 01:finish;
    u_int8_t                   modulation;
    
    u_int8_t                   bitoffset;
    u_int16_t                   bitlength;
    u_int16_t                   bytelength; 

    int                        zone_type;
    int                        symbol_num_in_zone;
    int                        symbol_offset_in_zone;  
    struct slot_node                  *next;
  //int                        next;
    u_int8_t*                  payload;             
};

struct    packet
{
    struct header{
        u_int16_t        packetlength;        //packet length in current packet
        u_int8_t         packnum;
        u_int16_t        packtype;            //for 0 is dataframe,1 is control frame, 2 is ulmapframe
    }packheader;
    
    int                  parameternum;
    struct parameter                          //notes   the type  may contain one or more than one
    {
         u_int8_t          length;
         u_int8_t          type;
         u_int8_t          value;
    }packparameter;

  //struct _slotcow slotcow;
    struct slotcol
    {
        u_int8_t           symboloffset;          //symbol offset in physical 
        u_int8_t           slotlength;
        struct slot_node          *slot_header;          //first slot node
      
    }slotdata;
};



/** MAC - PHY */
struct phy_dl_slot
{
    u_int8_t*         payload;             /* Bit in byte */
    u_int16_t         bytelength;
    u_int8_t          repetition_coding_indication;
    u_int8_t          is_broadcast;        //1  is broadcast  other 0
    u_int8_t          coding_type;         //CC or CTC
    u_int8_t          code_id;             //modulation and rate  combine
    u_int8_t          mimo_mode;           //single     = 0,stcmatrixa = 1,stcmatrixb = 2
    u_int8_t          block_id;
    u_int8_t          unused_flag;    // 1--subchannel unused, 0--used
    struct phy_dl_slot       *next;

};

struct phy_dl_slotsymbol
{
    u_int8_t          is_broadcast;
    u_int32_t         frame_index;         //
    u_int8_t          dl_subframe_end_flag; 

    u_int8_t          symboloffset;
    u_int8_t          slotlength;          /* length of slot in bytes */
    u_int8_t          mimo_mode;
    void              *p_dts_info;          
    void              *p_ulmap;
    u_int8_t          ul_map_len; 
    void              *p_payload_buf;
    struct phy_dl_slot       *slot_header;        /* First slot node */
    int dl_perscan_flag;
};

//implemented by adapter and open to phy only
struct phy_dl_slotsymbol *  adapter_dl_init_physlotsymbol(const char *const p_queue_name);
int  adapter_dl_deinit_physlotsymbol(struct phy_dl_slotsymbol **slotsymbol);



//private APIs for adapter
//int  phy_ofdma_dl_adapter_malloc_physlotsymbol(phy_dl_slotsymbol **pp_slotsymbol);
//int  phy_ofdma_dl_adapter_malloc_physlotnode(phy_dl_slot** pp_slot);
//int  phy_ofdma_dl_adapter_release_physlotnode(phy_dl_slot** pp_slot);

int  adapter_malloc_physlotsymbol(struct phy_dl_slotsymbol **pp_slotsymbol);
int  adapter_malloc_physlotnode(struct phy_dl_slot** pp_slot);
int  adapter_release_physlotnode(struct phy_dl_slot** pp_slot);


/*
slot_phynode* getslotphynode();
int releaseslotphynode();

phypacket *  getnewphypacket();
int  releasephypacket(phypacket **phy);*/

#endif
