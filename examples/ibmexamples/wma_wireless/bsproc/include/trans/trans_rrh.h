/* ----------------------------------------------------------------------------
 IBM Confidential

 IBM Wireless M2M Platform

 (C)Copyright IBM Corp. 2009, 2010, 2011

 All Rights Reserved.

 File Name: trans_rrh.h

 Change Activity:

 Date             Description of Change                            By
 -----------      ---------------------                            --------
 23-Mar.2011      Created                                          E Wulan

 ----------------------------------------------------------------------------
 PROLOG END TAG zYx                                                         */


#ifndef TRANS_RRH_H_
#define TRANS_RRH_H_


#include <sys/types.h>
#include <sys/time.h>
#include <trans_timer.h>
#include <trans_agent.h>
#include <trans_common.h>

/*****************************************************************************+
*Macro
+*****************************************************************************/


#define RRH_MSG_MAX_LEN 40960

#define RRH_MSG_PAYLOAD_MAX_LEN 1024

#define RRH_SERVER_UDP_PORT     33333   /*SERVER C&M UDP PORT：33333  */
//#define RRH_SERVER_TCP_PORT     33335   /*SERVER C&M TCP PORT：*/

#define RRH_RRU_UDP_PORT           33334/*RRU C&M UDP PORT ：33334；*/
#define RRH_RRU_TCP_PORT           30000/*RRU C&M TCP PORT：30000；*/
#define RRH_RRU_DATA_UDP_PORT           33336/*RRU I/Q UDP端口号：33336。*/

#define RRH_MONITOR_HRADER_LEN        22


#define RRH_MAC_ADDER_LEN        6

#define RRH_IP_ADDER_LEN        4

#define RRH_SUBNET_MASK_LEN        4

#define RRH_SEND_BUF_MAX_LEN        (40960)
#define RRH_REV_BUF_MAX_LEN          (40960)


#define RRH_HEARTBEAT_TIMEOUT_NUM        9

/**/
#define RRH_MAX_PROTOCOL_BODY_LEN        4096

/**/
#define RRH_LONGITUDE                    "116.280678"
#define RRH_LATITUDE                    "40.048943"


/*********************Equipment Information*************************/
#define RRH_MSG_DEV_INFO                               0x0000

/* Equipment Manufacture code  */
#define RRH_MSG_DEV_MANUFACTURE_NO                     (0x01)
#define RRH_MSG_DEV_MANUFACTURE_NO_LEN                  1

/* Equipment category */
#define RRH_MSG_DEV_CLASS                              (0x02)
#define RRH_MSG_DEV_CLASS_LEN                           1

/* Equipment type */
#define RRH_MSG_DEV_TYPE                              (0x03)
#define RRH_MSG_DEV_TYPE_LEN                           20

/* Serial Number of Manufacture */
#define RRH_MSG_DEV_SERIAL_NO                         (0x04)
#define RRH_MSG_DEV_SERIAL_NO_LEN                      20

/* Manufacture Date */
#define RRH_MSG_DEV_MADEDATE                         (0x05)
#define RRH_MSG_DEV_MADEDATE_LEN                      16

/* Total of Actual Carrier Wave  */
#define RRH_MSG_DEV_ACTUAL_CARRIER_NO                 (0x06)
#define RRH_MSG_DEV_ACTUAL_CARRIER_NO_LEN              1

/* Longitude */
#define RRH_MSG_LONGITUDE                             (0x07)
#define RRH_MSG_LONGITUDE_LEN                           20

/* Latitude */
#define RRH_MSG_LATITUDE                              (0x08)
#define RRH_MSG_LATITUDE_LEN                           20

/* Current Version of control software */
#define RRH_MSG_SW_VER                                (0x09)
#define RRH_MSG_SW_VER_LEN                              20

/* Current Version of hardware */
#define RRH_MSG_HW_VER                                (0x0A)
#define RRH_MSG_HW_VER_LEN                              20

/* Current Version of  Firmware software */
#define RRH_MSG_FW_VER                                (0x0B)
#define RRH_MSG_FW_VER_LEN                              20

/* Backup Version for Software */
#define RRH_MSG_SW_BAK_VER                      (0x0C)
#define RRH_MSG_SW_BAK_VER_LEN                  20

/* Backup Version for Firmware*/
#define RRH_MSG_FW_BAK_VER                      (0x0D)
#define RRH_MSG_FW_BAK_VER_LEN                  20

/* Factory Version for Software*/
#define RRH_MSG_SW_DEF_VER                      (0x0E)
#define RRH_MSG_SW_DEF_VER_LEN                  20

/* Factory Version for Firmware*/
#define RRH_MSG_FW_DEF_VER                      (0x0F)
#define RRH_MSG_FW_DEF_VER_LEN                  20

/*********************Network management information*************************/

#define RRH_MSG_NETMGR_INFO                             0x100
/* Serial Number of Server  */
#define RRH_MSG_SERVER_NO                             (0x101)
#define RRH_MSG_SERVER_NO_LEN                           1

/* Serial Number of RRU */
#define RRH_MSG_RRU_NO                                (0x102)
#define RRH_MSG_RRU_NO_LEN                              4

/*Server IP*/
#define RRH_MSG_SERVER_IP                             (0x103)
#define RRH_MSG_SERVER_IP_LEN                           4

/*RRU MAC*/
#define RRH_MSG_MAC                                   (0x104)
#define RRH_MSG_MAC_LEN                                 6

/*RRU IP */
#define RRH_MSG_RRU_IPV4                              (0x105)
#define RRH_MSG_RRU_IPV4_LEN                            4

/* The Current Time of Device */
#define RRH_MSG_CUR_TIME                              (0x106)
#define RRH_MSG_CUR_TIME_LEN                            7

/* RRU Remote Reset */
#define RRH_MSG_RRU_RESET                             (0x107)
#define RRH_MSG_RRU_RESET_LEN                           1

/*IP Address of FTP Server*/
#define RRH_MSG_FTP_ADDRESS                     (0x108)
#define RRH_MSG_FTP_ADDRESS_LEN                 4

/*Port of FTP Server*/
#define RRH_MSG_FTP_PORT                        (0x109)
#define RRH_MSG_FTP_PORT_LEN                    2

/*Download Path for FTP Version*/
#define RRH_MSG_FTP_PATH                        (0x10A)
#define RRH_MSG_FTP_PATH_LEN                    100

/*Download Filename for FTP Version*/
#define RRH_MSG_FTP_FILE                        (0x10B)
#define RRH_MSG_FTP_FILE_LEN                    100

/*Download Type for FTP Version*/
#define RRH_MSG_FTP_TYPE                        (0x10C)
#define RRH_MSG_FTP_TYPE_LEN                    1

/*FTP Download Control*/
#define RRH_MSG_FTP_CONTROL                     (0x10D)
#define RRH_MSG_FTP_CONTROL_LEN                 1

/*FTP Download Result*/
#define RRH_MSG_FTP_RESULT                      (0x10E)
#define RRH_MSG_FTP_RESULT_LEN                  1

/*Activate Software*/
#define RRH_MSG_SW_ACTIVATE                     (0x10F)
#define RRH_MSG_SW_ACTIVATE_LEN                 20

/*Activate Firmware*/
#define RRH_MSG_FW_ACTIVATE                     (0x110)
#define RRH_MSG_FW_ACTIVATE_LEN                 20

/*I/Q Data Port*/
#define RRH_MSG_IQ_DATA_PORT                     (0x111)
#define RRH_MSG_IQ_DATA_PORT_LEN                 2


/*************************Alarm Enable************************/
#define RRH_MSG_ALM_EN_BASE                             0x200

/* Power failure alarm enabled */
#define RRH_MSG_POWER_FAULT_ENABLE                    (0x201)
#define RRH_MSG_POWER_FAULT_ENABLE_LEN                  1

/* Amplifier over-temperature alarm enable */
#define RRH_MSG_PA_OVER_TEMP_ENABLE                   (0x202)
#define RRH_MSG_PA_OVER_TEMP_ENABLE_LEN                 1


/* LO loss of lock alarm enabled */
#define RRH_MSG_LO_LOCK_ENABLE                        (0x203)
#define RRH_MSG_LO_LOCK_ENABLE_LEN                      1

/* Downlink amplifiers alarm enabled */
#define RRH_MSG_DL_PA_FAULT_ENABLE                    (0x204)
#define RRH_MSG_DL_PA_FAULT_ENABLE_LEN                  1

/* GPS alarm enabled */
#define RRH_MSG_GPS_FAULT_ENABLE                      (0x205)
#define RRH_MSG_GPS_FAULT_ENABLE_LEN                   1

/* Channel 1 downlink output owe power alarm enabled */
#define RRH_MSG_DL_OUTPUT1_OVER_POWER_ENABLE          (0x206)
#define RRH_MSG_DL_OUTPUT1_OVER_POWER_ENABLE_LEN        1

/* Channel 1 downlink output over power alarm enabled */
#define RRH_MSG_DL_OUTPUT1_SHORTAGE_POWER_ENABLE      (0x207)
#define RRH_MSG_DL_OUTPUT1_SHORTAGE_POWER_ENABLE_LEN    1

/* Channel 1 downlink VSWR  alarm enabled */
#define RRH_MSG_DL_VSWR1_ENABLE                      (0x208)
#define RRH_MSG_DL_VSWR1_ENABLE_LEN                    1

/* Channel 2 downlink output owe power alarm enabled */
#define RRH_MSG_DL_OUTPUT2_OVER_POWER_ENABLE          (0x209)
#define RRH_MSG_DL_OUTPUT2_OVER_POWER_ENABLE_LEN        1

/* Channel 2 downlink output over power enabled */
#define RRH_MSG_DL_OUTPUT2_SHORTAGE_POWER_ENABLE      (0x20A)
#define RRH_MSG_DL_OUTPUT2_SHORTAGE_POWER_ENABLE_LEN    1

/* Channel 2 downlink VSWR alarm enabled */
#define RRH_MSG_DL_VSWR2_ENABLE                       (0x20B)
#define RRH_MSG_DL_VSWR2_ENABLE_LEN                     1

#if 0
/* Downlink ethernet packet exception alarm enabled */
#define RRH_MSG_DL_ETHPKG_FAULT_ENABLE                (RRH_MSG_ALM_EN_BASE+12)
#define RRH_MSG_DL_ETHPKG_FAULT_ENABLE_LEN              1
#endif
/*************************Equipment alarm data***************************/
#define RRH_MSG_ALM_BASE                              0x300

/* Power failure alarm */
#define RRH_MSG_POWER_FAULT                           (0x301)
#define RRH_MSG_POWER_FAULT_LEN                         1

/* Amplifier over-temperature alarm*/
#define RRH_MSG_PA_OVER_TEMP                          (0x302)
#define RRH_MSG_PA_OVER_TEMP_LEN                        1

/* LO loss of lock alarm */
#define RRH_MSG_LO_LOCK                               (0x303)
#define RRH_MSG_LO_LOCK_LEN                             1

/* Downlink amplifiers failure alarm */
#define RRH_MSG_DL_PA_FAULT                           (0x304)
#define RRH_MSG_DL_PA_FAULT_LEN                         1

/* GPS exception alarm */
#define RRH_MSG_GPS_FAULT                             (0x305)
#define RRH_MSG_GPS_FAULT_LEN                           1

/* Channel 1 downlink output owe power alarm */
#define RRH_MSG_DL_OUTPUT1_SHORTAGE_POWER       (0x306)
#define RRH_MSG_DL_OUTPUT1_SHORTAGE_POWER_LEN   1

/* Channel 1 downlink output over power alarm */
#define RRH_MSG_DL_OUTPUT1_OVER_POWER           (0x307)
#define RRH_MSG_DL_OUTPUT1_OVER_POWER_LEN       1

/* Channel 1 downlink VSWR  alarm */
#define RRH_MSG_DL_VSWR1                             (0x308)
#define RRH_MSG_DL_VSWR1_LEN                           1

/* Channel 2 downlink output over power */
#define RRH_MSG_DL_OUTPUT2_SHORTAGE_POWER             (0x309)
#define RRH_MSG_DL_OUTPUT2_SHORTAGE_POWER_LEN           1

/* Channel 2 downlink output owe power alarm  */
#define RRH_MSG_DL_OUTPUT2_OVER_POWER                 (0x30A)
#define RRH_MSG_DL_OUTPUT2_OVER_POWER_LEN               1

/* Channel 2 downlink VSWR alarm enabled */
#define RRH_MSG_DL_VSWR2                              (0x30B)
#define RRH_MSG_DL_VSWR2_LEN                            1

#if 0
/* Downlink ethernet packet exception alarm */
#define RRH_MSG_DL_ETHPKG_FAULT                       (RRH_MSG_ALM_BASE+12)
#define RRH_MSG_DL_ETHPKG_FAULT_LEN                     1
#endif
/**************************Equipment Config data****************************/

#define RRH_MSG_PARAM_CFG_BASE                        0x400

/* Channel 1 channel enable switch */
#define RRH_MSG_CHAN1_SWITCH                0x401
#define RRH_MSG_CHAN1_SWITCH_LEN            1
        
/* Channel 2 channel enable switch*/
#define RRH_MSG_CHAN2_SWITCH                0x402
#define RRH_MSG_CHAN2_SWITCH_LEN            1
            
/* Channel 1 working frequency */
#define RRH_MSG_CHAN1_FREQ                  0x405
#define RRH_MSG_CHAN1_FREQ_LEN              4
        
/* Channel 2 working frequency */
#define RRH_MSG_CHAN2_FREQ                  0x406
#define RRH_MSG_CHAN2_FREQ_LEN              4
    
/* Baseband sampling rate */
#define RRH_MSG_BB_SAMPLE_RATIO             0x407
#define RRH_MSG_BB_SAMPLE_RATIO_LEN         2
    
/* Channel 1 working mode */
#define RRH_MSG_CHAN1_WORKMODE              0x408
#define RRH_MSG_CHAN1_WORKMODE_LEN          1
    
/* Channel 2 working mode */
#define RRH_MSG_CHAN2_WORKMODE              0x409
#define RRH_MSG_CHAN2_WORKMODE_LEN          1

/* TX time length  */
#define RRH_MSG_TX_LEN                      0x40A
#define RRH_MSG_TX_LEN_LEN                  2
    
/* RX time length  */
#define RRH_MSG_RX_LEN                      0x40B
#define RRH_MSG_RX_LEN_LEN                  2
    
//TTG
#define RRH_MSG_TTG                         0x40C
#define RRH_MSG_TTG_LEN                     2
    
//RTG
#define RRH_MSG_RTG                         0x40D
#define RRH_MSG_RTG_LEN                     2
    
/* IQ data in the Ethernet packet length */
#define RRH_MSG_IQ_ETHPKG_LEN               0x40E
#define RRH_MSG_IQ_ETHPKG_LEN_LEN           2
    
/* Downlink send lead time  */
#define RRH_MSG_DL_PRESEND_TIME             0x40F
#define RRH_MSG_DL_PRESEND_TIME_LEN         2
    
/* Baseband data format */
#define RRH_MSG_BB_DATA_FMT                 0x410
#define RRH_MSG_BB_DATA_FMT_LEN             1
    
/*threshold  Config*/
/* Amplifier over-temperature alarm threshold -----Level 1*/
#define RRH_MSG_PA_TEMP_GATE_FIRST          0x411
#define RRH_MSG_PA_TEMP_GATE_FIRST_LEN      1
    
/* Amplifier over-temperature alarm threshold -----Level 2*/
#define RRH_MSG_PA_TEMP_GATE_SECOND         0x412
#define RRH_MSG_PA_TEMP_GATE_SECOND_LEN     1
    
/* Downlink VSWR threshold -----Level 1*/
#define RRH_MSG_DL_VSWR_GATE_FRIST          0x413
#define RRH_MSG_DL_VSWR_GATE_FRIST_LEN      1
    
/* Downlink VSWR threshold -----Level 2*/
#define RRH_MSG_DL_VSWR_GATE_SECOND         0x414
#define RRH_MSG_DL_VSWR_GATE_SECOND_LEN     1
    
/* Downlink output owe power threshold*/
#define RRH_MSG_DL_OUTPUT_SHORTAGE_POWER_GATE           0x415
#define RRH_MSG_DL_OUTPUT_SHORTAGE_POWER_GATE_LEN       1
    
/* Downlink output over power threshold -----Level 1*/
#define RRH_MSG_DL_OUTPUT_OVER_POWER_GATE_FIRST         0x416
#define RRH_MSG_DL_OUTPUT_OVER_POWER_GATE_FIRST_LEN     1
    
/* Downlink output over power threshold -----Level 2*/
#define RRH_MSG_DL_OUTPUT_OVER_POWER_GATE_SECOND        0x417
#define RRH_MSG_DL_OUTPUT_OVER_POWER_GATE_SECOND_LEN    1
    
/* Rated output configuration power  */
#define RRH_MSG_NORM_OUTPUT_POWER            0x418
#define RRH_MSG_NORM_OUTPUT_POWER_LEN        1
    
/*  Set the error packet count of Ethernet packets  0 */
#define RRH_MSG_ETHPKG_ERRNUM_CLEAR          0x419
#define RRH_MSG_ETHPKG_ERRNUM_CLEAR_LEN      1
    
/* Optical port 1 enabled */
#define RRH_MSG_OP1_EN                       0x41A
#define RRH_MSG_OP1_EN_LEN                   1
    
/* Optical port 2 enabled */
#define RRH_MSG_OP2_EN                       0x41B
#define RRH_MSG_OP2_EN_LEN                   1
    
/* Main optical interface */
#define RRH_MSG_MASTER_OP                    0x41C
#define RRH_MSG_MASTER_OP_LEN                1
    
/* Byte order */
#define RRH_MSG_BYTEORDER                    0x41D  
#define RRH_MSG_BYTEORDER_LEN                1
    
/* Test trigger mode */
#define RRH_MSG_TEST_TRGGER                  0x41E
#define RRH_MSG_TEST_TRGGER_LEN              1

/* AGC enabled*/
#define RRH_MSG_AGC_ENABLE                   0x41F
#define RRH_MSG_AGC_ENABLE_LEN              1

/*  Channel 1 RX PGC */
#define RRH_MSG_CHAN1_RX_PGC                  0x420
#define RRH_MSG_CHAN1_RX_PGC_LEN              1

/*  Channel 2 RX PGC */
#define RRH_MSG_CHAN2_RX_PGC                  0x421
#define RRH_MSG_CHAN2_RX_PGC_LEN              1

/* Intermediate-frequency Bandwidth*//*0：1.25M   1：2.5M    2：5M      3：12M*/
#define RRH_MSG_INTER_FREQ_BANDWIDTH                  0x422         
#define RRH_MSG_INTER_FREQ_BANDWIDTH_LEN              1

/* Establish/inquires the carrier info*/  /**/
#define RRH_MSG_CARRIER_INFO                  0x423
#define RRH_MSG_CARRIER_INFO_LEN          SIZEOF_TRANS_RRH_CARRIER_INFO  

/* Delete the carrier info*/  /**/
#define RRH_MSG_DELETE_CARRIER_INFO                  0x424
#define RRH_MSG_DELETE_CARRIER_INFO_LEN              1


/**************************Equipment sample data**************************/
#define RRH_MSG_SAMPLE_INFO                             0x500

/* Amplifier Temperature*/
#define RRH_MSG_PA_TEMP_VALUE                       (0x501)
#define RRH_MSG_PA_TEMP_VALUE_LEN                      1

#ifdef TRANS_RRH_NEW_CONNECT
/* Channel 1 downlink output power level*/
#define RRH_MSG_DL_INPUT1_LEVEL                     (0x502)
#define RRH_MSG_DL_INPUT1_LEVEL_LEN                    1

/* Channel 2 downlink output power level */
#define RRH_MSG_DL_INPUT2_LEVEL                     (0x503)
#define RRH_MSG_DL_INPUT2_LEVEL_LEN                    1

/* Channel 1 uplink gain  */
#define RRH_MSG_UL_GAIN1                            (0x504)
#define RRH_MSG_UL_GAIN1_LEN                           1

/* Channel 2 uplink gain  */
#define RRH_MSG_UL_GAIN2                            (0x505)
#define RRH_MSG_UL_GAIN2_LEN                           1

/* Channel 1 downlink gain  */
#define RRH_MSG_DL_GAIN1                            (0x506)
#define RRH_MSG_DL_GAIN1_LEN                           1

/* Channel 2 downlink gain  */
#define RRH_MSG_DL_GAIN2                            (0x507)
#define RRH_MSG_DL_GAIN2_LEN                           1

#else
/* Channel 1 downlink output power level*/
#define RRH_MSG_DL_INPUT1_LEVEL                     (0x502)
#define RRH_MSG_DL_INPUT1_LEVEL_LEN                    4

/* Channel 2 downlink output power level */
#define RRH_MSG_DL_INPUT2_LEVEL                     (0x503)
#define RRH_MSG_DL_INPUT2_LEVEL_LEN                    4


/* Channel 1 uplink gain  */
#define RRH_MSG_UL_GAIN1                            (0x504)
#define RRH_MSG_UL_GAIN1_LEN                           4

/* Channel 2 uplink gain  */
#define RRH_MSG_UL_GAIN2                            (0x505)
#define RRH_MSG_UL_GAIN2_LEN                           4

/* Channel 1 downlink gain  */
#define RRH_MSG_DL_GAIN1                            (0x506)
#define RRH_MSG_DL_GAIN1_LEN                           4

/* Channel 2 downlink gain  */
#define RRH_MSG_DL_GAIN2                            (0x507)
#define RRH_MSG_DL_GAIN2_LEN                           4

#endif


/* Channel 1 downlink VSWR(Standing wave ratio) */
#define RRH_MSG_DL_VSWR1_VALUE                     (0x508)
#define RRH_MSG_DL_VSWR1_VALUE_LEN                    1

/* Channel 2 downlink VSWR(Standing wave ratio) */
#define RRH_MSG_DL_VSWR2_VALUE                     (0x509)
#define RRH_MSG_DL_VSWR2_VALUE_LEN                    1

/* GPS clock if locked */
#define RRH_MSG_GPS_CLK_LOCK_VALUE                 (0x50A)
#define RRH_MSG_GPS_CLK_LOCK_VALUE_LEN                 1

/* Channel 1 power calibration values  */
#define RRH_MSG_CHAN1_POWER_NORM_VALUE             (0x50B)
#define RRH_MSG_CHAN1_POWER_NORM_VALUE_LEN             1

/* Channel 2 power calibration values  */
#define RRH_MSG_CHAN2_POWER_NORM_VALUE             (0x50C)
#define RRH_MSG_CHAN2_POWER_NORM_VALUE_LEN             1


/* Channel 1 calibration power  */
#define RRH_MSG_CHAN1_NORM_POW_VALUE               (0x50D)
#define RRH_MSG_CHAN1_NORM_POW_VALUE_LEN               4

/* Channel 2 calibration power  */
#define RRH_MSG_CHAN2_NORM_POW_VALUE               (0x50E)
#define RRH_MSG_CHAN2_NORM_POW_VALUE_LEN               4

/* MAC packet error packets number  */
#define RRH_MSG_MAC_ERRPKG_VALUE                   (0x50F)
#define RRH_MSG_MAC_ERRPKG_VALUE_LEN                   4

/* Ethernet packet error packets number  */
#define RRH_MSG_DATAFRAME_LOST_VALUE               (0x510)
#define RRH_MSG_DATAFRAME_LOST_VALUE_LEN               4

/* Ethernet packet late packets number  */
#define RRH_MSG_DATAFRAME_DELAY_VALUE              (0x511)
#define RRH_MSG_DATAFRAME_DELAY_VALUE_LEN              4

/* Channel 1 RSSI*/
#define RRH_MSG_CHAN1_RSSI_VALUE                   (0x512)
#define RRH_MSG_CHAN1_RSSI_VALUE_LEN                   4

/* Channel 2 RSSI*/
#define RRH_MSG_CHAN2_RSSI_VALUE                   (0x513)
#define RRH_MSG_CHAN2_RSSI_VALUE_LEN                   4

/* Channel 1 RX BB,1.1M RN , AD, 12M RN*/
#define RRH_MSG_CHAN1_RX_BB                   (0x514)
#define RRH_MSG_CHAN1_RX_BB_LEN                   40

/* Channel 2 RX BB,1.1M RN , AD, 12M RN*/
#define RRH_MSG_CHAN2_RX_BB                    (0x515)
#define RRH_MSG_CHAN2_RX_BB_LEN                   40


/************************** Expanded command **************************/
#define RRH_MSG_EXP_COM                            0xa00

/*Calculate PA*/
#define RRH_MSG_CAL_PWR_CFG                     (0xA01)
#define RRH_MSG_CAL_PWR_CFG_LEN                 1

/*PA Switch for Channel 1*/
#define RRH_MSG_PA_SWITCH_A_CFG                 (0xA02)
#define RRH_MSG_PA_SWITCH_A_CFG_LEN             1

/*PA Switch for Channel 2*/
#define RRH_MSG_PA_SWITCH_B_CFG                 (0xA03)
#define RRH_MSG_PA_SWITCH_B_CFG_LEN             1

/*Config GPS Enable*/
#define RRH_MSG_GPS_ENABLE_CFG                         (0xA04)
#define RRH_MSG_GPS_ENABLE_CFG_LEN              1

/*****************************************************************************+
*Enum
+*****************************************************************************/
/*RRH State*/
enum rrh_addr_reuse_enum 
{   
   RRH_ADDR_NO_REUSE = 0,       /*not reuse */
   RRH_ADDR_REUSE = 1       /*reuse */

};

/*Monitor Type*/
enum rrh_monitor_type_enum
{   
   RRH_MONITOR_TYPE_DEFAULT = 0x00,              /*Forbid*/
   RRH_MONITOR_TYPE_ALARM = 0x01,       /*Alarm */
   RRH_MONITOR_TYPE_QUERY = 0x02,       /*Query */   
   RRH_MONITOR_TYPE_CONFIG = 0x03,       /*CONFIG */   
   RRH_MONITOR_TYPE_HEARTBRAT = 0x04,       /*HEARTBRAT */
   RRH_MONITOR_TYPE_RESERVED = 0x80,                /*Custom*/     
   RRH_MONITOR_TYPE_BUF = 0xff       /* */

};

/*Monitor Response Flag*/
enum rrh_monitor_rep_flag_enum
{   
    RRH_MONITOR_REP_FLAG_OK = 0x00,      /*sucessful*/ 
    RRH_MONITOR_REP_FLAG_P_ERR = 0x01,       /*Part error*/
    RRH_MONITOR_REP_FLAG_TYPE_ERR = 0x02,      /*Type error*/ 
    RRH_MONITOR_REP_FLAG_CRC_ERR = 0x03,       /*CRC error*/ 
    RRH_MONITOR_REP_FLAG_LEN_ERR = 0x04,        /*len error*/ 
    RRH_MONITOR_REP_FLAG_EXT_ERR = 0xC0,        /*Exe error*/
    RRH_MONITOR_REP_FLAG_OTHER_ERR = 0xFE,          /*Other error*/       
    RRH_MONITOR_REP_FLAG_ORDER = 0xff       /*Order*/
};


/* Error Flag When Response Flag is RRH_MONITOR_REP_FLAG_P_ERR = 0x01*/
/* */
enum rrh_monitor_rep_err_enum
{
    RRH_MONITOR_REP_ERR_NONE          = 0x0000,           /*sucessful*/ 
    RRH_MONITOR_REP_ERR_IDENTIFY_WRY  = 0x1000,  /*unknow param type*/
    RRH_MONITOR_REP_ERR_NO_WRITE  = 0x2000,          /*param not write,just read*/
    RRH_MONITOR_REP_ERR_LEN_ILLEGAL   = 0x3000,     /*param len error*/    
    RRH_MONITOR_REP_INVALID_PARAM = 0x4000,          /*param error*/    
    RRH_MONITOR_REP_INVALID_RESULT= 0x5000,	        /*param value for query error*/
    RRH_MONITOR_REP_QRY_ILLEGAL   = 0x6000  	     	/*query param value error*/
};

/*RRH State*/
enum rrh_monitor_alarm_enum 
{   
   RRH_MONITOR_AlARM_RECOVER  = 0,       /*Alarm report */
   RRH_MONITOR_AlARM_REPORT = 1,       /*Alarm recover */
   RRH_MONITOR_AlARM_CLOSE = 2       /*Alarm close */

};


/*****************************************************************************+
*Data structure
+*****************************************************************************/
/*  Equipment Config Info  */  
struct rrh_equipment_config_info
{   
    u_int32_t   uw_rru_id;    /*RRU ID*/
    u_int8_t     uc_server_id;   /*SERVER ID*/
    u_int8_t     a_rrh_mac_addr[RRH_MAC_ADDER_LEN];   /*RRU MAC */
    u_int32_t   uw_rrh_ip_addr;/*RRU IP */
    u_int32_t   uw_rrh_mask_addr;/*RRU  mask*/
    u_int32_t   uw_ser_ip_addr;/*SERVER IP */
    u_int32_t   uw_ser_broc_addr;/*SERVER BROADCAST IP */
    u_int16_t   us_ser_tcp_port; /*SERVER  TCP Port*/
    u_int16_t   us_ser_udp_port; /*SERVER  UDP Port*/
    u_int16_t   us_rrh_tcp_port; /*RRH  TCP Port*/
    u_int16_t   us_rrh_udp_port; /*RRH  UDP Port*/
    u_int16_t   us_rrh_data_udp_port; /*RRH  data UDP Port*/
    u_int16_t   us_ser_data_udp_port; /*SERVER  I/Q  Data Port*/ 
    u_int8_t     a_rrh_nic_if[TRANS_RRH_DEVICE_ID_SIZE];   /*Device ID : ETH1, Eth2....*/
};  


/*RRh Request Msg revice from RRH*/
struct rrh_conn_req_msg
{
    u_int32_t   uw_rru_id;  /*RRU ID号*/
    u_int8_t     a_mac_addr[RRH_MAC_ADDER_LEN];    /*RRU MAC*/

}__attribute__ ((packed));

#define SIZEOF_RRH_CONN_REQ_MSG   sizeof(struct rrh_conn_req_msg)


/*RRh Response Msg send to RRH*/
struct rrh_conn_rep_msg
{
    u_int32_t   uw_rru_id;    /*RRU ID*/
    u_int8_t     uc_server_id;   /*SERVER ID*/
    u_int8_t     a_rrh_mac_addr[RRH_MAC_ADDER_LEN];   /*RRU MAC */
    u_int8_t     a_rrh_ip_addr[RRH_IP_ADDER_LEN];/*RRU IP */
    u_int8_t     a_rrh_mask_addr[RRH_SUBNET_MASK_LEN];/*RRU  mask*/
    u_int8_t     a_ser_ip_addr[RRH_IP_ADDER_LEN];/*SERVER IP */
    u_int16_t   us_ser_tcp_port; /*SERVER  TCP*/
    u_int16_t   us_ser_data_port; /*SERVER  I/Q  Data Port*/

}__attribute__ ((packed));

#define SIZEOF_RRH_CONN_REP_MSG   sizeof(struct rrh_conn_rep_msg)


/*=====================Monitoring Protocol Data====================*/

/*
   |rrh_monitor_header (22Bytes)  |  rrh_monitor_param  (变长)  |CRC    校验单元（2Bytes)  |
*/

/*Monitoring Protocol header*/
struct rrh_monitor_header
{
    u_int16_t   us_body_len;    
    u_int8_t     uc_server_id;    /*SERVERID   1Byte*/
    u_int32_t   uw_rru_id;        /*RRUID：4byte*/
    u_int16_t   us_serial_number;  
    u_int8_t     a_time[7];       
                                            
    u_int8_t    uc_type;            
    u_int8_t    uc_resp_flag;   
                                           
    u_int32_t   uw_extend;    

}__attribute__ ((packed));

#define SIZEOF_RRH_MONITOR_HEADER     sizeof(struct rrh_monitor_header)


/*Monitoring Protocol param*/
struct rrh_monitor_param
{
    u_int16_t   us_param_type;  /*Param type：2Bytes*/
    u_int8_t     uc_param_len;    /*Param Length：1Byte*/
    //u_int8_t     *p_param;         /*Param */

}__attribute__ ((packed));

#define SIZEOF_RRH_MONITOR_PARAM   sizeof(struct rrh_monitor_param)



/*Monitoring param values*/
struct rrh_monitor_param_alarm
{
    u_int16_t   us_param_type;  /*Param type：2Bytes*/
    u_int8_t     uc_param_len;    /*Param Length：1Byte*/
    u_int8_t     uc_param_value;

}__attribute__ ((packed));

#define SIZEOF_RRH_MONITOR_PARAM_ALARM   sizeof(struct rrh_monitor_param_alarm)

#if 0

struct rrh_monitor_param_value16
{
    struct rrh_monitor_param   st_param;
    u_int16_t     us_param_value;

}__attribute__ ((packed));

#define SIZEOF_RRH_MONITOR_PARAM_VALUE16   sizeof(struct rrh_monitor_param_value16)


struct rrh_monitor_param_value32
{
    struct rrh_monitor_param   st_param;
    u_int32_t     uw_param_value;

}__attribute__ ((packed));

#define SIZEOF_RRH_MONITOR_PARAM_VALUE32   sizeof(struct rrh_monitor_param_value32)

/*socket addr*/
struct rrh_socket_info
{
    u_int16_t us_port;
    u_int16_t us_padding;
    u_int32_t uw_ipAddr;
    int32_t w_sockFd;
};
#endif

/*=====================Change Agent Msg To RRH Msg ====================*/

/*Query msg for metric*/
struct trans_rrh_metric_info
{
    u_int16_t us_metric_id;
    u_int8_t   uc_metric_len;
    //u_int32_t uw_source_id;
    struct trans_agent_metric_info *p_agent_metric;
};

/*=====================Build and Send RRH Msg Info====================*/
#if 0
/*Build Messsage content for struct rrh_monitor_param*/
struct trans_rrh_build_msg_param
{
    u_int16_t   us_param_type;  /*Param type：2Bytes*/
    u_int8_t     uc_param_len;    /*Param Length：1Byte*/
    void    *p_param;           /*Param content*/
};


/*Build Messsage Info*/
struct trans_rrh_build_msg_info
{
    u_int8_t     uc_type;                /*Monitor Type :   enum rrh_monitor_type_enum*/
    u_int32_t   uw_param_num;  /*Param number：4Bytes*/
    u_int32_t   uw_msg_len;  /*Param Length：4Bytes*/
    void    *p_msg;    /*Msg content */
};

/*Send Messsage Info including Build and Send Info*/
struct trans_rrh_send_msg_info
{
    int32_t      w_rrh_sockfd;       /*socket for sending msg to RRH  : 4Bytes*/
    u_int32_t   uw_src_moudle;   /*Sender*/
    trans_timeout_action f_action;          /*Funtion Callback*/
    struct trans_rrh_build_msg_info  st_build_info;  /*Build Messsage Info*/
    
    void  *        p_info;/*Don't care its memory, it is controled by user in f_callback*/
};
#endif

/*Send Messsage Info including Build and Send Info*/
struct trans_rrh_send_msg_info
{
    int32_t      w_rrh_sockfd;       /*socket for sending msg to RRH  : 4Bytes*/
    u_int32_t   uw_src_moudle;   /*Sender*/
    u_int8_t     uc_type;                /*Monitor Type :   enum rrh_monitor_type_enum*/
    u_int32_t    uw_payload_len;   /*length of Payload*/
    void *          p_payload;     /*Payload*/     
    fun_callback f_rrh_callback;          /*Funtion Callback*/
    void  *        p_info;
};

/*=====================Build RRH payload====================*/

/*Build Messsage content for struct rrh_monitor_param*/
struct trans_rrh_build_payload_info
{
    u_int16_t   * p_tag;        /*Tag*/                                                
    void *         p_value;           /*Value */   

};

#define SIZEOF_TRANS_RRH_BUILD_PAYLOAD_INFO    sizeof(struct trans_rrh_build_payload_info)


/*=====================RRH heartbeat control====================*/

/*Send Messsage Info including Build and Send Info*/
struct trans_rrh_heartbeat
{
    u_int8_t     uc_send_flag;       
    u_int8_t     uc_count;       
    u_int8_t     uc_duration;                
    u_int8_t     uc_send_num;  
    u_int8_t     uc_stop_num; 
    void*         p_timer;
};

#define SIZEOF_TRANS_RRH_HEARTBEAT    sizeof(struct trans_rrh_heartbeat)


/*****************************************************************************+
*extern
+*****************************************************************************/

extern u_int32_t trans_rrh_init(struct trans_init_info *p_init_info);
extern u_int32_t trans_rrh_udp_socket(void);
extern u_int32_t trans_rrh_udp_brocast_process(struct rrh_conn_req_msg *p_req_msg, 
                    struct rrh_conn_rep_msg *p_rep_msg);

extern u_int32_t trans_rrh_udp_raw_socket(void);

extern u_int32_t trans_rrh_tcp_socket();

extern u_int32_t trans_rrh_type_get_len(u_int16_t us_rrh_param_type, 
                                            u_int8_t *p_rrh_param_len);

extern u_int32_t trans_rrh_check_msg_content(u_int8_t *p_rev_msg, u_int8_t *p_rep_flag);

extern u_int32_t trans_rrh_check_msg(void *p_info, 
                           size_t len,
                           void * p_rev_buf);

extern u_int32_t trans_rrh_parse_msg(void *p_info, 
                           size_t len,
                           void * p_rev_buf);

extern u_int32_t trans_rrh_rev_msg(u_int8_t **pp_rev_msg, int32_t w_rrh_socket, int32_t *p_len);

extern u_int32_t trans_rrh_send_msg_process(struct trans_rrh_send_msg_info *p_rrh_send_info);

extern u_int32_t trans_rrh_build_msg(struct trans_rrh_send_msg_info *p_rrh_send_info,
                                    u_int8_t *p_send_msg, 
                                    u_int32_t *p_msg_len, 
                                    u_int16_t *p_serial_number);
                                    
extern u_int32_t trans_rrh_build_payload(struct trans_rrh_build_payload_info *p_build_info, 
                    u_int32_t uw_payload_num,
                    void *p_payload, 
                    u_int32_t *p_payload_len);

extern u_int32_t trans_rrh_build_heard(u_int8_t uc_type, 
                                    u_int8_t *p_send_head, 
                                    u_int32_t uw_body_len, 
                                    u_int16_t *p_serial_number);


extern u_int32_t trans_rrh_heartbeat_timer(void);

extern u_int32_t trans_rrh_send_monitor(
                            struct trans_send_msg_to_rrh *p_rrh,
                            size_t len,
                            u_int8_t * p_send_buf);

extern u_int32_t trans_rrh_send_action(
                            struct trans_send_msg_to_rrh *p_rrh,
                            size_t len,
                            u_int8_t * p_send_buf);

extern u_int32_t trans_rrh_send_agent(
                            struct trans_send_msg_to_rrh *p_rrh,
                            size_t len,
                            u_int8_t * p_send_buf);

extern u_int32_t trans_rrh_send_bs(
                            struct trans_send_msg_to_rrh *p_rrh,
                            size_t len,
                            u_int8_t * p_send_buf);



extern u_int16_t  trans_rrh_cal_serial_num(); 

extern u_int32_t trans_rrh_release();

extern u_int32_t trans_rrh_type_get_len(u_int16_t us_rrh_param_type, 
                                            u_int8_t *p_rrh_param_len);


extern int32_t g_trans_rrh_socket;

extern struct rrh_equipment_config_info g_trans_rrh_eqp_config;

extern void *g_trans_rrh_hb_timer;

#endif /* TRANS_RRH_H_ */


