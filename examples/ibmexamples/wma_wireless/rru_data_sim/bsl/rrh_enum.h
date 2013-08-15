#ifndef __RRH_ENUM_H__
#define __RRH_ENUM_H__


typedef enum _RRH_TYPE_T
{
    USRP_RRH  =  0,
    THU_RRH   =  1,
} RRH_TYPE_T;


typedef enum _RRH_PARAM_T
{
    RX_BITRATE  = 0,
    RX_CENTER_FREQ  = 1,
    TX_BITRATE  = 2, 
    TX_CENTER_FREQ  = 3,
    SEND_CHANNEL_NUM      = 4,
    REC_CHANNEL_NUM      = 5,
    RRH_ID                  = 6,
    RX_GAIN               = 7,
    TX_AMPL               = 8
} RRH_PARAM_T;


#endif //__RRH_ENUM_H__
