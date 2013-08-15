#ifndef _MAC_PHYPACKET_RECEIVED_H_
#define _MAC_PHYPACKET_RECEIVED_H_


#include "adapter_bs_dl_interface_data.h"
/*
phypacket *  getnewphypacket();
int  releasephypacket(phypacket **phy);*/

int netframe_transform_phypacket(u_int8_t  *pbuffer,int netlength,struct phy_dl_slotsymbol *pphybuffer);

int phy_get_systemparameter(u_int8_t  *pbuffer,int netlength);

#endif
