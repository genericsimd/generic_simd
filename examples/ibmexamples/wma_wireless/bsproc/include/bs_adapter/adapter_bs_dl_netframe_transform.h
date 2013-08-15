#ifndef _MAC_NETFRAME_TRANSFORM_H_
#define _MAC_NETFRAME_TRANSFORM_H_

#include  "adapter_config.h"
#include "adapter_bs_dl_interface_data.h"
#include "mac_map_phy.h"
#include "mac_assistfunc.h"


//after call this function,remember release pnetbuffer please;  if u don't release pnetbuffer, then memory feak
int ofdma_transform_netframe(struct ofdma_map *ofdma_frame,int *islotnum, u_int8_t *pnetbuffer,int *netlength,int *packnum);

//continue; 
int tranform_system_parameter(u_int8_t *p_netbuffer,int *p_netlength,u_int8_t *packnum);
#endif
