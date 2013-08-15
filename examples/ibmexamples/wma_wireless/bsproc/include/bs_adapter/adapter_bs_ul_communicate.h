#ifndef  ADAPTER_BS_UL_COMMUNICATE_H_
#define  ADAPTER_BS_UL_COMMUNICATE_H_

#include "adapter_bs_ul_transform.h"
#include "adapter_bs_ul_interface_data.h"
#include "selectserver.h"
#include "types.h"
#include "mac_frame.h"

void* send_ul_frame_handle(void *param);

int send_ul_phy_frame(struct ul_net_frame *pframe);

int receive_ul_phy_frame(struct buffer_obj *buffobj,int *pflag,struct ul_net_frame **p_burstframe);

int  set_control_envirement();
void* process_phy_get_data_and_transmite(void *param);
int  pushnewframe(struct phy_ul_procblk  *p_netframe);
//int adapter_get_phy_subframe(void *p_buf, physical_subframe **pp_phy_subframe);
int adapter_get_phy_subframe(void *p_buf, physical_subframe **pp_phy_subframe);
#endif
