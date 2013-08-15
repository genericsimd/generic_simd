#ifndef ADAPTER_BS_UL_TRANSFORM_H_
#define ADAPTER_BS_UL_TRANSFORM_H_

#include "phy_ul_rx_interface.h"

#include "mac_frame.h"
#include "adapter_bs_ul_interface_data.h"
#include "mac_shift_bits.h"
#include "adapter_bs_ulmap_queue.h"


#include "ulmap.h"

//the following all function is for phy->mac in BS
int mac_ul_transform_netframe(struct ul_slot_node *pburstframe,struct ul_net_frame *pnetframe);

//old
//int ul_ofmda_transform_mac_received(ofdma_ulmap* p_ulofdma_frame, physical_subframe* p_phy_subframe);

int ul_slotnode_transform_mac_unit_data(struct ul_slot_node *p_burstdata,struct ul_net_frame** pp_netframe);

int ul_transform_mac_received_phy(struct ul_slot_node  *p_burstframe,physical_subframe *p_phy_subframe);

int adapter_block_transform_subframe_1(struct phy_ul_procblk *p_blocklink,physical_subframe** p_phy_subframe);
int adapter_block_transform_subframe_2(struct phy_ul_procblk *p_blocklink1,struct phy_ul_procblk *p_blocklink2,physical_subframe** p_phy_subframe);
int adapter_block_transform_subframe(struct phy_ul_procblk *p_blocklink1,struct phy_ul_procblk *p_blocklink2,physical_subframe** p_phy_subframe);

#endif
