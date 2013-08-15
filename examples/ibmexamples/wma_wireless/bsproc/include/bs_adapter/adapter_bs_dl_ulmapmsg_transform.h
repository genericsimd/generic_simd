#ifndef _MAC_ULMAPMSG_TRANSFORM_H_
#define _MAC_ULMAPMSG_TRANSFORM_H_

#include "adapter_config.h"

#include "ulmap.h"


struct ul_map_frame
{
    struct ulmapheader{
        u_int16_t        packetlength;        //packet length in current packet
        u_int8_t         packnum;
        u_int16_t        packtype;            //for 0 is dataframe,1 is control frame, 2 is ulmapframe
    }ulmapheader;
    u_int8_t  manage_msg_type;
    u_int8_t  rsv;
    u_int8_t  ucd_count;
    u_int32_t alloc_start_time;
    u_int8_t  ulmap_ie_num;
  //ulmapieframe *ie;
    ul_map_ie *ie;
};


int transform_ulmapie_to_ulmapieframe(ul_map_msg *p_ulmap, u_int8_t *p_netbuffer,int *ibuflen);

int transfrom_ulmapieframe_to_ulmapie(u_int8_t *p_netbuffer,  int *pgetlength,ul_map_msg * p_ulmap);
#endif
