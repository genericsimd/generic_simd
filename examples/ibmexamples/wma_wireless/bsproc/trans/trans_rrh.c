/* ----------------------------------------------------------------------------
 IBM Confidential

 IBM Wireless M2M Platform

 (C)Copyright IBM Corp. 2009, 2010, 2011

 All Rights Reserved.

 File Name: trans_rrh.c

 Change Activity:

 Date             Description of Change                            By
 -----------      ---------------------                            --------
 23-Mar.2011      Created                                          E Wulan

 ----------------------------------------------------------------------------
 PROLOG END TAG zYx                                                         */


#include <sys/types.h>
#include <syslog.h>
#include <flog.h>

#ifndef TRANS_MONITOR_TEST_COMPILE
/*semaphore*/
//#include <sem_util.h>
#endif

//#include "queue_util.h"
#if (defined TRANS_BS_COMPILE) || (defined TRANS_UI_COMPILE)
#include <bs_cfg.h>
#endif
#ifdef TRANS_MS_COMPILE
#include <ms_cfg.h>
#endif

#include <trans.h>
#include <trans_common.h>
#include <trans_list.h>
#include <trans_transaction.h>
#include <trans_device.h>
#include <trans_rrh.h>
#include <trans_agent.h>
#include <trans_action.h>
#include <trans_timer.h>
#include <trans_debug.h>
#include <trans_monitor.h>
#include <trans_wireless.h>

/*TCP*/
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <signal.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <pthread.h>
#include <errno.h>

#ifdef TRANS_RRH_RAW_SOCKET

#include <stdio.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netinet/ip.h>
#include <string.h>
#include <netdb.h>
#include <netinet/tcp.h>
#include <netinet/udp.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include <net/if.h>
#include <sys/ioctl.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <linux/if_ether.h>
#include <netpacket/packet.h>
#endif

/*UDP*/
#include<sys/wait.h>
#include<fcntl.h>
#include <arpa/inet.h>
#include<netdb.h>
/*TIMER*/
#include <sys/time.h>
#include <signal.h> 

/*LIST*/
#include <malloc.h>
#include <pthread.h>
#include <semaphore.h>
#include <sched.h>
//#include <mutex.h>


/*****************************************************************************+
 *Global Variables 
+*****************************************************************************/
int32_t g_trans_rrh_socket = -1;
int32_t g_trans_rrh_udp_raw_socket = -1;

#ifdef TRANS_RRH_COMPILE

//u_int8_t g_trans_rrh_heartbeat_num = 0;
struct trans_rrh_heartbeat  g_trans_rrh_heartbeat;

/*Can't set the g_trans_rrh_serial_number = 0, 0 means nothing*/
u_int16_t g_trans_rrh_serial_number = 1;
pthread_mutex_t  g_trans_rrh_serial_num_mutex;

struct rrh_equipment_config_info g_trans_rrh_eqp_config;

void *g_trans_rrh_hb_timer = NULL;

u_int8_t g_trans_rrh_stop_alert_ack = 0;

#endif

/*****************************************************************************+
 *Code 
+*****************************************************************************/
#ifdef TRANS_RRH_COMPILE

/*****************************************************************************+
* Function: trans_rrh_cal_serial_num()
* Description: calculate and return the serial number
* Parameters:
*           NONE
* Return Values:
*           us_serial_num
*
*  
*  Data:    2011-03-23
* 
+*****************************************************************************/
u_int16_t  trans_rrh_cal_serial_num() 
{
    u_int16_t us_serial_num = 0;
    
    pthread_mutex_lock(&(g_trans_rrh_serial_num_mutex));  
    
    us_serial_num = g_trans_rrh_serial_number;

    g_trans_rrh_serial_number++;

    /*Can't set the g_trans_rrh_serial_number = 0, 0 means nothing*/
    if (0xffff == g_trans_rrh_serial_number)
    {
        g_trans_rrh_serial_number = 1;

    }
    
    pthread_mutex_unlock(&(g_trans_rrh_serial_num_mutex));
    
    //FLOG_DEBUG_TRANS(g_trans_debug_rrh, "Exit us_serial_num = %d\r\n", us_serial_num); 
    
    return(us_serial_num);
}

/*****************************************************************************+
* Function: trans_rrh_get_crc16()
* Description: return the CRC value from the message
* Parameters:
*           *p_buf : message
*            us_len : message length
* Return Values:
*           us_crc
*
*  
*  Data:    2011-03-23
* 
+*****************************************************************************/
u_int16_t  trans_rrh_get_crc16(u_int8_t *p_buf, u_int16_t us_len) 
{
    u_int16_t us_crc = 0;
    
    u_int16_t *p_crc = NULL;

    p_crc = (u_int16_t *)(p_buf +us_len);

    us_crc = TRANS_NTOHS(*p_crc);
    
    //FLOG_DEBUG_TRANS(g_trans_debug_rrh, "Exit us_crc = %d\r\n", us_crc); 
    
    return(us_crc);
}

/*****************************************************************************+
* Function: trans_rrh_cal_crc16()
* Description: 
* Parameters:
*           NONE
* Return Values:
*           NONE
*
*  
*  Data:    2011-03-23
* 
+*****************************************************************************/
u_int16_t  trans_rrh_cal_crc16(u_int8_t *p_buf, u_int16_t us_len) 
{
    u_int16_t us_crc = 0;
    u_int8_t   uc_index = 0;

    while(us_len--!=0) 
    {
        for(uc_index=0x80; uc_index!=0; uc_index/=2) 
        {
            if((us_crc&0x8000)!=0) 
            {
                us_crc*=2; 
    
                us_crc^=0x1021;
            }
            else 
            {
                us_crc*=2;
            }                
            
            if((*p_buf&uc_index)!=0) 
            {
                us_crc^=0x1021;
            }
        }
        
        p_buf++;
    }

    //FLOG_DEBUG_TRANS(g_trans_debug_rrh, "Exit us_crc = %d\r\n", us_crc); 
    
    return(us_crc);
    
}

/*****************************************************************************+
* Function: trans_rrh_type_get_len()
* Description: 
* Parameters:
*           NONE
* Return Values:
*           TRANS_FAILD : 1
*           TRANS_SUCCESS  : 0
*
*  
*  Data:    2011-09-14
* 
+*****************************************************************************/
u_int32_t trans_rrh_type_get_len(u_int16_t us_rrh_param_type, 
                                            u_int8_t *p_rrh_param_len)
{
    switch (us_rrh_param_type)
    {
        /*RRH_MSG_DEV_INFO                               0x0000*/
        /* Equipment Manufacture code  */
        case RRH_MSG_DEV_MANUFACTURE_NO: 
        
            *p_rrh_param_len = RRH_MSG_DEV_MANUFACTURE_NO_LEN;
            break; 

        /* Equipment category */
        case RRH_MSG_DEV_CLASS: 
        
            *p_rrh_param_len = RRH_MSG_DEV_CLASS_LEN;
            break; 

        /* Equipment type */
        case RRH_MSG_DEV_TYPE: 
        
            *p_rrh_param_len = RRH_MSG_DEV_TYPE_LEN;
            break; 

        /* Serial Number of Manufacture */
        case RRH_MSG_DEV_SERIAL_NO: 
        
            *p_rrh_param_len = RRH_MSG_DEV_SERIAL_NO_LEN;
            break; 

        /* Manufacture Date */
        case RRH_MSG_DEV_MADEDATE: 
        
            *p_rrh_param_len = RRH_MSG_DEV_MADEDATE_LEN;
            break; 

        /* Total of Actual Carrier Wave  */
        case RRH_MSG_DEV_ACTUAL_CARRIER_NO: 
        
            *p_rrh_param_len = RRH_MSG_DEV_ACTUAL_CARRIER_NO_LEN;
            break; 

        /* Longitude */
        case RRH_MSG_LONGITUDE: 
        
            *p_rrh_param_len = RRH_MSG_LONGITUDE_LEN;
            break; 

        /* Latitude */
        case RRH_MSG_LATITUDE: 
        
            *p_rrh_param_len = RRH_MSG_LATITUDE_LEN;
            break; 

        /* Current Version of control software */
        case RRH_MSG_SW_VER: 
        
            *p_rrh_param_len = RRH_MSG_SW_VER_LEN;
            break; 

        /* Current Version of hardware */
        case RRH_MSG_HW_VER: 
        
            *p_rrh_param_len = RRH_MSG_HW_VER_LEN;
            break; 

        /* Current Version of  Firmware software */
        case RRH_MSG_FW_VER: 
        
            *p_rrh_param_len = RRH_MSG_FW_VER_LEN;
            break; 

        /* Backup Version for Software */
        case RRH_MSG_SW_BAK_VER: 
        
            *p_rrh_param_len = RRH_MSG_SW_BAK_VER_LEN;
            break; 
            
        /* Backup Version for Firmware*/
        case RRH_MSG_FW_BAK_VER: 
        
            *p_rrh_param_len = RRH_MSG_FW_BAK_VER_LEN;
            break; 
            
        /* Factory Version for Software*/
        case RRH_MSG_SW_DEF_VER: 
        
            *p_rrh_param_len = RRH_MSG_SW_DEF_VER_LEN;
            break; 
            
        /* Factory Version for Firmware*/
        case RRH_MSG_FW_DEF_VER: 
        
            *p_rrh_param_len = RRH_MSG_FW_DEF_VER_LEN;
            break; 

        /*RRH_MSG_NETMGR_INFO                             0x100*/
        /* Serial Number of Server  */
        case RRH_MSG_SERVER_NO:
            
            *p_rrh_param_len = RRH_MSG_SERVER_NO_LEN;
            break; 

        /* Serial Number of RRU */
        case RRH_MSG_RRU_NO: 
            
            *p_rrh_param_len = RRH_MSG_RRU_NO_LEN;
            break; 

        /* Server IP */
        case RRH_MSG_SERVER_IP: 
            
            *p_rrh_param_len = RRH_MSG_SERVER_IP_LEN;
            break; 

        /* RRU MAC */
        case RRH_MSG_MAC: 
            
            *p_rrh_param_len = RRH_MSG_MAC_LEN;
            break; 

        /* RRU IP  */
        case RRH_MSG_RRU_IPV4:
            
            *p_rrh_param_len = RRH_MSG_RRU_IPV4_LEN;
            break; 

        /* The Current Time of Device */
        case RRH_MSG_CUR_TIME: 
            
            *p_rrh_param_len = RRH_MSG_CUR_TIME_LEN;
            break; 

        /* RRU Remote Reset */
        case RRH_MSG_RRU_RESET: 
            
            *p_rrh_param_len = RRH_MSG_RRU_RESET_LEN;
            break; 

        /*IP Address of FTP Server*/
        case RRH_MSG_FTP_ADDRESS: 
            
            *p_rrh_param_len = RRH_MSG_FTP_ADDRESS_LEN;
            break;
        
        /*Port of FTP Server*/
        case RRH_MSG_FTP_PORT: 
            
            *p_rrh_param_len = RRH_MSG_FTP_PORT_LEN;
            break;
        
        /*Download Path for FTP Version*/
        case RRH_MSG_FTP_PATH: 
            
            *p_rrh_param_len =  RRH_MSG_FTP_PATH_LEN;
            break;
        
        /*Download Filename for FTP Version*/
        case RRH_MSG_FTP_FILE: 
            
            *p_rrh_param_len = RRH_MSG_FTP_FILE_LEN;
            break;
        
        /*Download Type for FTP Version*/
        case RRH_MSG_FTP_TYPE: 
            
            *p_rrh_param_len = RRH_MSG_FTP_TYPE_LEN;
            break;
        
        /*FTP Download Control*/
        case RRH_MSG_FTP_CONTROL: 
            
            *p_rrh_param_len = RRH_MSG_FTP_CONTROL_LEN;
            break;
        
        /*FTP Download Result*/
        case RRH_MSG_FTP_RESULT: 
            
            *p_rrh_param_len = RRH_MSG_FTP_RESULT_LEN;
            break;
        
        /*Activate Software*/
        case RRH_MSG_SW_ACTIVATE: 
            
            *p_rrh_param_len = RRH_MSG_SW_ACTIVATE_LEN;
            break;
        
        /*Activate Firmware*/
        case RRH_MSG_FW_ACTIVATE: 
            
            *p_rrh_param_len = RRH_MSG_FW_ACTIVATE_LEN;
            break;    

        /*Activate Firmware*/
        case RRH_MSG_IQ_DATA_PORT: 
            
            *p_rrh_param_len = RRH_MSG_IQ_DATA_PORT_LEN;
            break;   

        /*RRH_MSG_ALM_EN_BASE                             0x200*/
        /* Power failure alarm enabled */
        case RRH_MSG_POWER_FAULT_ENABLE: 
            
            *p_rrh_param_len = RRH_MSG_POWER_FAULT_ENABLE_LEN;
            break; 

        /* Amplifier over-temperature alarm enable */
        case RRH_MSG_PA_OVER_TEMP_ENABLE: 
            
            *p_rrh_param_len = RRH_MSG_PA_OVER_TEMP_ENABLE_LEN;
            break; 

        /* LO loss of lock alarm enabled */
        case RRH_MSG_LO_LOCK_ENABLE: 
            
            *p_rrh_param_len = RRH_MSG_LO_LOCK_ENABLE_LEN;
            break; 

        /* Downlink amplifiers alarm enabled */
        case RRH_MSG_DL_PA_FAULT_ENABLE: 
            
            *p_rrh_param_len = RRH_MSG_DL_PA_FAULT_ENABLE_LEN;
            break; 

        /* GPS alarm enabled */
        case RRH_MSG_GPS_FAULT_ENABLE: 
            
            *p_rrh_param_len = RRH_MSG_GPS_FAULT_ENABLE_LEN;
            break; 

        /* Channel 1 downlink output owe power alarm enabled */
        case RRH_MSG_DL_OUTPUT1_OVER_POWER_ENABLE: 
            
            *p_rrh_param_len = RRH_MSG_DL_OUTPUT1_OVER_POWER_ENABLE_LEN;
            break; 

        /* Channel 1 downlink output over power alarm enabled */
        case RRH_MSG_DL_OUTPUT1_SHORTAGE_POWER_ENABLE: 
            
            *p_rrh_param_len = RRH_MSG_DL_OUTPUT1_SHORTAGE_POWER_ENABLE_LEN;
            break; 

        /* Channel 1 downlink VSWR  alarm enabled */
        case RRH_MSG_DL_VSWR1_ENABLE: 
            
            *p_rrh_param_len = RRH_MSG_DL_VSWR1_ENABLE_LEN;
            break; 

        /* Channel 2 downlink output owe power alarm enabled */
        case RRH_MSG_DL_OUTPUT2_OVER_POWER_ENABLE: 
            
            *p_rrh_param_len = RRH_MSG_DL_OUTPUT2_OVER_POWER_ENABLE_LEN;
            break; 

        /* Channel 2 downlink output over power enabled */
        case RRH_MSG_DL_OUTPUT2_SHORTAGE_POWER_ENABLE: 
            
            *p_rrh_param_len = RRH_MSG_DL_OUTPUT2_SHORTAGE_POWER_ENABLE_LEN;
            break; 

        /* Channel 2 downlink VSWR alarm enabled */
        case RRH_MSG_DL_VSWR2_ENABLE: 
            
            *p_rrh_param_len = RRH_MSG_DL_VSWR2_ENABLE_LEN;
            break; 

        #if 0
        /* Downlink ethernet packet exception alarm enabled */
        case RRH_MSG_DL_ETHPKG_FAULT_ENABLE: 
            
            *p_rrh_param_len = RRH_MSG_DL_ETHPKG_FAULT_ENABLE_LEN;
            break; 
        #endif

        /*RRH_MSG_ALM_BASE                              0x300*/
        /* Power failure alarm */
        case RRH_MSG_POWER_FAULT: 
            
            *p_rrh_param_len = RRH_MSG_POWER_FAULT_LEN;
            break; 
        
        /* Amplifier over-temperature alarm*/
        case RRH_MSG_PA_OVER_TEMP: 
            
            *p_rrh_param_len = RRH_MSG_PA_OVER_TEMP_LEN;
            break; 
        
        /* LO loss of lock alarm */
        case RRH_MSG_LO_LOCK: 
            
            *p_rrh_param_len = RRH_MSG_LO_LOCK_LEN;
            break; 
        
        /* Downlink amplifiers failure alarm */
        case RRH_MSG_DL_PA_FAULT: 
            
            *p_rrh_param_len = RRH_MSG_DL_PA_FAULT_LEN;
            break; 
        
        /* GPS exception alarm */
        case RRH_MSG_GPS_FAULT: 
            
            *p_rrh_param_len = RRH_MSG_GPS_FAULT_LEN;
            break; 
        
        /* Channel 1 downlink output owe power alarm */
        case RRH_MSG_DL_OUTPUT1_OVER_POWER: 
            
            *p_rrh_param_len = RRH_MSG_DL_OUTPUT1_OVER_POWER_LEN;
            break; 
        
        /* Channel 1 downlink output over power alarm */
        case RRH_MSG_DL_OUTPUT1_SHORTAGE_POWER: 
            
            *p_rrh_param_len = RRH_MSG_DL_OUTPUT1_SHORTAGE_POWER_LEN;
            break; 
        
        /* Channel 1 downlink VSWR  alarm */
        case RRH_MSG_DL_VSWR1: 
            
            *p_rrh_param_len = RRH_MSG_DL_VSWR1_LEN;
            break; 
        
        /* Channel 2 downlink output owe power alarm  */
        case RRH_MSG_DL_OUTPUT2_OVER_POWER: 
            
            *p_rrh_param_len = RRH_MSG_DL_OUTPUT2_OVER_POWER_LEN;
            break; 
        
        /* Channel 2 downlink output over power */
        case RRH_MSG_DL_OUTPUT2_SHORTAGE_POWER: 
            
            *p_rrh_param_len = RRH_MSG_DL_OUTPUT2_SHORTAGE_POWER_LEN;
            break; 
        
        /* Channel 2 downlink VSWR alarm enabled */
        case RRH_MSG_DL_VSWR2: 
            
            *p_rrh_param_len = RRH_MSG_DL_VSWR2_LEN;
            break; 
        
        #if 0
        /* Downlink ethernet packet exception alarm */
        case RRH_MSG_DL_ETHPKG_FAULT: 
            
            *p_rrh_param_len = RRH_MSG_DL_ETHPKG_FAULT_LEN;
            break; 
        #endif

        /*RRH_MSG_PARAM_CFG_BASE                        0x400*/
        /* Channel 1 channel enable switch */
        case RRH_MSG_CHAN1_SWITCH: 
            
            *p_rrh_param_len = RRH_MSG_CHAN1_SWITCH_LEN;
            break; 
                
        /* Channel 2 channel enable switch*/
        case RRH_MSG_CHAN2_SWITCH: 
            
            *p_rrh_param_len = RRH_MSG_CHAN2_SWITCH_LEN;
            break; 
                    
        /* Channel 1 working frequency */
        case RRH_MSG_CHAN1_FREQ: 
            
            *p_rrh_param_len = RRH_MSG_CHAN1_FREQ_LEN;
            break; 
                
        /* Channel 2 working frequency */
        case RRH_MSG_CHAN2_FREQ: 
            
            *p_rrh_param_len = RRH_MSG_CHAN2_FREQ_LEN;
            break; 
            
        /* Baseband sampling rate */
        case RRH_MSG_BB_SAMPLE_RATIO: 
            
            *p_rrh_param_len = RRH_MSG_BB_SAMPLE_RATIO_LEN;
            break; 
            
        /* Channel 1 working mode */
        case RRH_MSG_CHAN1_WORKMODE: 
            
            *p_rrh_param_len = RRH_MSG_CHAN1_WORKMODE_LEN;
            break; 
            
        /* Channel 2 working mode */
        case RRH_MSG_CHAN2_WORKMODE: 
            
            *p_rrh_param_len = RRH_MSG_CHAN2_WORKMODE_LEN;
            break; 
        
        /* TX time length  */
        case RRH_MSG_TX_LEN: 
            
            *p_rrh_param_len = RRH_MSG_TX_LEN_LEN;
            break; 
            
        /* RX time length  */
        case RRH_MSG_RX_LEN: 
            
            *p_rrh_param_len = RRH_MSG_RX_LEN_LEN;
            break; 
            
        //TTG
        case RRH_MSG_TTG: 
            
            *p_rrh_param_len = RRH_MSG_TTG_LEN;
            break; 
            
        //RTG
        case RRH_MSG_RTG: 
            
            *p_rrh_param_len = RRH_MSG_RTG_LEN;
            break; 
            
        /* IQ data in the Ethernet packet length */
        case RRH_MSG_IQ_ETHPKG_LEN: 
            
            *p_rrh_param_len = RRH_MSG_IQ_ETHPKG_LEN_LEN;
            break; 
            
        /* Downlink send lead time  */
        case RRH_MSG_DL_PRESEND_TIME: 
            
            *p_rrh_param_len = RRH_MSG_DL_PRESEND_TIME_LEN;
            break; 
            
        /* Baseband data format */
        case RRH_MSG_BB_DATA_FMT: 
            
            *p_rrh_param_len = RRH_MSG_BB_DATA_FMT_LEN;
            break; 
            
        /*threshold  Config*/
        /* Amplifier over-temperature alarm threshold -----Level 1*/
        case RRH_MSG_PA_TEMP_GATE_FIRST: 
            
            *p_rrh_param_len = RRH_MSG_PA_TEMP_GATE_FIRST_LEN;
            break; 
            
        /* Amplifier over-temperature alarm threshold -----Level 2*/
        case RRH_MSG_PA_TEMP_GATE_SECOND: 
            
            *p_rrh_param_len = RRH_MSG_PA_TEMP_GATE_SECOND_LEN;
            break; 
            
        /* Downlink VSWR threshold -----Level 1*/
        case RRH_MSG_DL_VSWR_GATE_FRIST: 
            
            *p_rrh_param_len = RRH_MSG_DL_VSWR_GATE_FRIST_LEN;
            break; 
            
        /* Downlink VSWR threshold -----Level 2*/
        case RRH_MSG_DL_VSWR_GATE_SECOND: 
            
            *p_rrh_param_len = RRH_MSG_DL_VSWR_GATE_SECOND_LEN;
            break; 
            
        /* Downlink output owe power threshold*/
        case RRH_MSG_DL_OUTPUT_SHORTAGE_POWER_GATE: 
            
            *p_rrh_param_len = RRH_MSG_DL_OUTPUT_SHORTAGE_POWER_GATE_LEN;
            break; 
            
        /* Downlink output over power threshold -----Level 1*/
        case RRH_MSG_DL_OUTPUT_OVER_POWER_GATE_FIRST: 
            
            *p_rrh_param_len = RRH_MSG_DL_OUTPUT_OVER_POWER_GATE_FIRST_LEN;
            break; 
            
        /* Downlink output over power threshold -----Level 2*/
        case RRH_MSG_DL_OUTPUT_OVER_POWER_GATE_SECOND: 
            
            *p_rrh_param_len = RRH_MSG_DL_OUTPUT_OVER_POWER_GATE_SECOND_LEN;
            break; 
            
        /* Rated output configuration power  */
        case RRH_MSG_NORM_OUTPUT_POWER: 
            
            *p_rrh_param_len = RRH_MSG_NORM_OUTPUT_POWER_LEN;
            break; 
            
        /*  Set the error packet count of Ethernet packets  0 */
        case RRH_MSG_ETHPKG_ERRNUM_CLEAR: 
            
            *p_rrh_param_len = RRH_MSG_ETHPKG_ERRNUM_CLEAR_LEN;
            break; 
            
        /* Optical port 1 enabled */
        case RRH_MSG_OP1_EN: 
            
            *p_rrh_param_len = RRH_MSG_OP1_EN_LEN;
            break; 
            
        /* Optical port 2 enabled */
        case RRH_MSG_OP2_EN: 
            
            *p_rrh_param_len = RRH_MSG_OP2_EN_LEN;
            break; 
            
        /* Main optical interface */
        case RRH_MSG_MASTER_OP: 
            
            *p_rrh_param_len = RRH_MSG_MASTER_OP_LEN;
            break; 
            
        /* Byte order */
        case RRH_MSG_BYTEORDER: 
            
            *p_rrh_param_len = RRH_MSG_BYTEORDER_LEN;
            break; 
            
        /* Test trigger mode */
        case RRH_MSG_TEST_TRGGER: 
            
            *p_rrh_param_len = RRH_MSG_TEST_TRGGER_LEN;
            break; 
        /* AGC enabled*/
        case RRH_MSG_AGC_ENABLE: 
            
            *p_rrh_param_len = RRH_MSG_AGC_ENABLE_LEN;
            break; 
        /*  Channel 1 RX PGC */
        case RRH_MSG_CHAN1_RX_PGC: 
            
            *p_rrh_param_len = RRH_MSG_CHAN1_RX_PGC_LEN;
            break; 
        /*  Channel 2 RX PGC */
        case RRH_MSG_CHAN2_RX_PGC: 
            
            *p_rrh_param_len = RRH_MSG_CHAN2_RX_PGC_LEN;
            break; 

        /* Intermediate-frequency Bandwidth*/
        case RRH_MSG_INTER_FREQ_BANDWIDTH: 
            
            *p_rrh_param_len = RRH_MSG_INTER_FREQ_BANDWIDTH_LEN;
            break; 
        /* Establish/inquires the carrier info*/
        case RRH_MSG_CARRIER_INFO: 
            
            *p_rrh_param_len = RRH_MSG_CARRIER_INFO_LEN;
            break; 
        /* Delete the carrier info*/
        case RRH_MSG_DELETE_CARRIER_INFO: 
            
            *p_rrh_param_len = RRH_MSG_DELETE_CARRIER_INFO_LEN;
            break; 

        /*RRH_MSG_SAMPLE_INFO                             0x500*/
        /*Power Amplifier Temperature*/ 
        case RRH_MSG_PA_TEMP_VALUE: 

            *p_rrh_param_len = RRH_MSG_PA_TEMP_VALUE_LEN;
            break; 
        
        /*Downlink Output Power for channel 1#*/
        case RRH_MSG_DL_INPUT1_LEVEL:
        
            *p_rrh_param_len = RRH_MSG_DL_INPUT1_LEVEL_LEN;            
            break; 
        
        /*Downlink Output Power for channel 2#*/
        case RRH_MSG_DL_INPUT2_LEVEL: 
          
            *p_rrh_param_len = RRH_MSG_DL_INPUT2_LEVEL_LEN;
            break;      

        /* Channel 1 uplink gain  */
        case RRH_MSG_UL_GAIN1: 
          
            *p_rrh_param_len = RRH_MSG_UL_GAIN1_LEN;
            break;
        
        /* Channel 2 uplink gain  */
        case RRH_MSG_UL_GAIN2: 
          
            *p_rrh_param_len = RRH_MSG_UL_GAIN2_LEN;
            break;
        
        /* Channel 1 downlink gain  */
        case RRH_MSG_DL_GAIN1: 
          
            *p_rrh_param_len = RRH_MSG_DL_GAIN1_LEN;
            break;
        
        /* Channel 2 downlink gain  */
        case RRH_MSG_DL_GAIN2: 
          
            *p_rrh_param_len = RRH_MSG_DL_GAIN2_LEN;
            break;
        
        /*Downlink Voltage Standing Wave Radio (VSWR) for channel 1#*/    
        case RRH_MSG_DL_VSWR1_VALUE: 

            *p_rrh_param_len = RRH_MSG_DL_VSWR1_VALUE_LEN;
            break;    
        
        /*Downlink Voltage Standing Wave Radio (VSWR) for channel 2#*/    
        case RRH_MSG_DL_VSWR2_VALUE: 

            *p_rrh_param_len = RRH_MSG_DL_VSWR2_VALUE_LEN;
            break;  
            
        /* GPS clock if locked */
        case RRH_MSG_GPS_CLK_LOCK_VALUE: 

            *p_rrh_param_len = RRH_MSG_GPS_CLK_LOCK_VALUE_LEN;
            break;
            
        /* Channel 1 power calibration values  */
        case RRH_MSG_CHAN1_POWER_NORM_VALUE: 

            *p_rrh_param_len = RRH_MSG_CHAN1_POWER_NORM_VALUE_LEN;
            break;
            
        /* Channel 2 power calibration values  */
        case RRH_MSG_CHAN2_POWER_NORM_VALUE: 

            *p_rrh_param_len = RRH_MSG_CHAN2_POWER_NORM_VALUE_LEN;
            break;
            
        /* Channel 1 calibration power  */
        case RRH_MSG_CHAN1_NORM_POW_VALUE: 

            *p_rrh_param_len = RRH_MSG_CHAN1_NORM_POW_VALUE_LEN;
            break;
            
        /* Channel 2 calibration power  */
        case RRH_MSG_CHAN2_NORM_POW_VALUE: 

            *p_rrh_param_len = RRH_MSG_CHAN2_NORM_POW_VALUE_LEN;
            break;
            
        /* MAC packet error packets number  */
        case RRH_MSG_MAC_ERRPKG_VALUE: 

            *p_rrh_param_len = RRH_MSG_MAC_ERRPKG_VALUE_LEN;
            break;
            
        /* Ethernet packet error packets number  */
        case RRH_MSG_DATAFRAME_LOST_VALUE: 

            *p_rrh_param_len = RRH_MSG_DATAFRAME_LOST_VALUE_LEN;
            break;
            
        /* Ethernet packet late packets number  */
        case RRH_MSG_DATAFRAME_DELAY_VALUE: 

            *p_rrh_param_len = RRH_MSG_DATAFRAME_DELAY_VALUE_LEN;
            break;
            
        /* Channel 1 RSSI*/
        case RRH_MSG_CHAN1_RSSI_VALUE: 

            *p_rrh_param_len = RRH_MSG_CHAN1_RSSI_VALUE_LEN;
            break;
            
        /* Channel 2 RSSI*/
        case RRH_MSG_CHAN2_RSSI_VALUE: 

            *p_rrh_param_len = RRH_MSG_CHAN2_RSSI_VALUE_LEN;
            break;
        /* Channel 1 RX BB,1.1M RN , AD, 12M RN*/
        case RRH_MSG_CHAN1_RX_BB: 

            *p_rrh_param_len = RRH_MSG_CHAN1_RX_BB_LEN;
            break;
        /* Channel 2 RX BB,1.1M RN , AD, 12M RN*/
        case RRH_MSG_CHAN2_RX_BB: 

            *p_rrh_param_len = RRH_MSG_CHAN2_RX_BB_LEN;
            break;

        /*RRH_MSG_EXP_COM                            0xa00*/        
        /*Calculate PA*/
        case RRH_MSG_CAL_PWR_CFG: 

            *p_rrh_param_len = RRH_MSG_CAL_PWR_CFG_LEN;
            break;
        
        /*PA Switch for Channel 1*/
        case RRH_MSG_PA_SWITCH_A_CFG: 

            *p_rrh_param_len = RRH_MSG_PA_SWITCH_A_CFG_LEN;
            break;
        
        /*PA Switch for Channel 2*/
        case RRH_MSG_PA_SWITCH_B_CFG: 

            *p_rrh_param_len = RRH_MSG_PA_SWITCH_B_CFG_LEN;
            break;
        
        /*Config GPS Enable*/
        case RRH_MSG_GPS_ENABLE_CFG: 

            *p_rrh_param_len = RRH_MSG_GPS_ENABLE_CFG_LEN;
            break;
            
        default:
    
            FLOG_ERROR("Rev unknow param_type! param_type = 0x%x\r\n", us_rrh_param_type);
    
            return TRANS_FAILD;
    }  
    
    
    FLOG_DEBUG_TRANS(g_trans_debug_rrh, "Exit : rrh_param_len = %d. \r\n", *p_rrh_param_len);

    return TRANS_SUCCESS;

}


/*****************************************************************************+
* Function: trans_rrh_check_msg_content()
* Description: Check the header parameters and CRC in the msg
* Parameters:
*           NONE
* Return Values:
*           TRANS_FAILD : 1
*           TRANS_SUCCESS  : 0
*
*  
*  Data:    2011-08-05
* 
+*****************************************************************************/
u_int32_t trans_rrh_check_msg_content(u_int8_t *p_rev_msg, u_int8_t *p_rep_flag)
{
    struct rrh_monitor_header  *p_rrh_rev_header = NULL;
    u_int16_t   us_body_len = 0, us_tot_len = 0;
    u_int16_t   us_crc_val = 0, us_crc_temp = 0;
    
    if ((NULL == p_rev_msg) || (NULL == p_rep_flag))
    {
        FLOG_ERROR("NULL PTR! \r\n");
        return TRANS_FAILD;  
    }    
    
    p_rrh_rev_header = (struct rrh_monitor_header *)p_rev_msg;    

    /*length Check*/
    us_body_len = TRANS_NTOHS(p_rrh_rev_header->us_body_len);
    if (us_body_len >= RRH_MSG_MAX_LEN)
    {
        *p_rep_flag = RRH_MONITOR_REP_FLAG_LEN_ERR;

        FLOG_ERROR("Length error! us_body_len = %d\r\n", us_body_len);         
        return TRANS_SUCCESS;
    }

    /*param Check*/
    if ((g_trans_rrh_eqp_config.uw_rru_id != TRANS_NTOHL(p_rrh_rev_header->uw_rru_id))
        ||(g_trans_rrh_eqp_config.uc_server_id != p_rrh_rev_header->uc_server_id))
    {
        *p_rep_flag = RRH_MONITOR_REP_FLAG_OTHER_ERR;
        
        FLOG_ERROR("ID error! rru_id = %d, server_id = %d\r\n", TRANS_NTOHL(p_rrh_rev_header->uw_rru_id), 
                                                p_rrh_rev_header->uc_server_id);       
        return TRANS_SUCCESS;
    }

    /*CRC  Check->  if CRC error ,discard msg  ->send error to sender*/
    us_tot_len = us_body_len +SIZEOF_RRH_MONITOR_HEADER;

    us_crc_val = trans_rrh_get_crc16((u_int8_t*)p_rev_msg, us_tot_len);    

    us_crc_temp = trans_rrh_cal_crc16((u_int8_t*)p_rev_msg, us_tot_len); 
    if(us_crc_val != us_crc_temp)
    {
        *p_rep_flag = RRH_MONITOR_REP_FLAG_CRC_ERR;

        FLOG_ERROR("CRC error! us_crc_val = %d, us_crc_temp = %d\r\n", us_crc_val, us_crc_temp);
        return TRANS_SUCCESS;
    }
    
    if ((RRH_MONITOR_TYPE_ALARM == p_rrh_rev_header->uc_type)
        &&(RRH_MONITOR_REP_FLAG_ORDER!= p_rrh_rev_header->uc_resp_flag))
    {
        *p_rep_flag = RRH_MONITOR_REP_FLAG_OTHER_ERR;
        
        FLOG_ERROR("Alert Response Flag error! resp_flag = %d.\r\n", p_rrh_rev_header->uc_resp_flag);
        return TRANS_SUCCESS;
    }

    if (((RRH_MONITOR_TYPE_QUERY == p_rrh_rev_header->uc_type)
        ||(RRH_MONITOR_TYPE_CONFIG== p_rrh_rev_header->uc_type))
        &&(RRH_MONITOR_REP_FLAG_OK!= p_rrh_rev_header->uc_resp_flag))
    {
        FLOG_ERROR("Resp_flag error! uc_resp_flag = %d : 0x%x%x\r\n", 
            p_rrh_rev_header->uc_resp_flag, 
            *(p_rev_msg + SIZEOF_RRH_MONITOR_HEADER),
            *(p_rev_msg + SIZEOF_RRH_MONITOR_HEADER + 1));

        *p_rep_flag = p_rrh_rev_header->uc_resp_flag;
        
        return TRANS_SUCCESS;
    }

    *p_rep_flag = RRH_MONITOR_REP_FLAG_OK;
    
    FLOG_DEBUG_TRANS(g_trans_debug_rrh, "*p_rep_flag = %d \r\n", *p_rep_flag);

    return TRANS_SUCCESS;
}


/******************************************************************+
* Function: trans_rrh_rev_forward2_agent()
* Description: Process the quary message from the RRH and send the  result to agent
* Parameters:
*           NONE
* Return Values:
*           TRANS_FAILD : 1
*           TRANS_SUCCESS  : 0
*
*  
*  Data:    2011-09-15
* 
+*****************************************************************/
u_int32_t trans_rrh_rev_forward2_agent(void *p_info, 
                           size_t len,
                           void * p_rev_buf)
{
    u_int32_t   uw_ret;
    int32_t w_result = 0;

    struct trans_send_msg_to_agent *p_agent_msg_info = NULL;
    void * p_user_info = NULL;
    int32_t  w_len = 0;

    if ((NULL == p_info) || (NULL == p_rev_buf))
    {
        FLOG_ERROR("NULL PTR! \r\n");
        return TRANS_FAILD;  
    }

    (void) len;

    w_result = trans_transaction_get_result(p_info);
    
    if (TRANS_ACK_FLAG_OK != w_result)
    {
        return w_result;  
    }
    else
    {
        p_agent_msg_info = (struct trans_send_msg_to_agent *)malloc(SIZEOF_TRANS_SEND_MSG_TO_AGENT);
        if (NULL == p_agent_msg_info)
        {
            FLOG_ERROR("malloc p_agent_msg_info error! \r\n");
            return TRANS_FAILD;   
        }
        
        /*Call this function when timeout or message back*/
        p_agent_msg_info->f_callback = NULL;   
        p_agent_msg_info->p_resp_msg = p_rev_buf;
        p_agent_msg_info->p_info = p_info;
        
        p_user_info = trans_transaction_get_user(p_info, &w_len);
        
        p_agent_msg_info->p_reqs_msg = p_user_info;

        #if 0
        /*********For metric test*********/
        p_agent_msg_info->uc_ack_flag = 0;
        /*********For metric test*********/
        #endif

        uw_ret = trans_send_rrh_msg(TRANS_SEND_TO_AGENT, p_agent_msg_info);
        if (TRANS_SUCCESS != uw_ret) 
        {   
            FLOG_ERROR("Call trans_send_rrh_msg error! uw_ret = %d\r\n", uw_ret);
            //return TRANS_FAILD;     
        }  

        free (p_agent_msg_info);


        #if 0
        us_body_len = len;   
        p_param_temp = (struct rrh_monitor_param *)p_param_msg;
        
        while (us_len < us_body_len)
        {
            //us_param_type = TRANS_HTONS(p_param_temp->us_param_type);   
            us_param_type = TRANS_NTOHS(p_param_temp->us_param_type); 
            uc_param_len = p_param_temp->uc_param_len;
        
            us_param_type = us_param_type&0x0fff;
            
            //FLOG_DEBUG_TRANS(g_trans_debug_rrh, "Rev message! us_param_type = 0x%x, uc_param_len = %d\r\n", us_param_type, uc_param_len);
        
            switch (us_param_type)
            {
                case RRH_MSG_PA_TEMP_VALUE: 
                    
                case RRH_MSG_DL_INPUT1_LEVEL:
                case RRH_MSG_DL_INPUT2_LEVEL:
                    w_value = *((int8_t *)(p_param_temp + 1));
        
                    uw_ret = trans_rrh_forward_rrh_metric(p_send_msg, p_thread_info->w_agent_sockfd,
                                            &(st_msg_info.u_extra_info.st_agent_metric),us_param_type, w_value);                
                    if (TRANS_SUCCESS != uw_ret) 
                    {   
                        FLOG_ERROR("1 Call trans_rrh_forward_rrh_metric error! uw_ret = %d\r\n", uw_ret);
                        return TRANS_FAILD;     
                    }
        
                    break;
        
                case RRH_MSG_DL_VSWR1_VALUE: 
                case RRH_MSG_DL_VSWR2_VALUE: 
        
                    w_value = *((u_int8_t *)(p_param_temp + 1));
        
        
                    break;    
                                   
                default:
                    
                    FLOG_ERROR("Rev unknow message! us_param_type = 0x%x\r\n", us_param_type);
        
                    /*????????????????*/
                    break;
            
            }  
        
        
            us_len = us_len + uc_param_len +SIZEOF_RRH_MONITOR_PARAM;         
            p_param_temp = (struct rrh_monitor_param *)(p_param_msg + us_len);

            /*Length of p_response_msg*/    
            st_agent_msg_info.uw_resp_len = st_agent_msg_info.uw_resp_len + 4;      

            a_value[] = w_value;

        
        }
        #endif
    }  
      
    return uw_ret;     
}


/******************************************************************+
* Function: trans_rrh_rev_forward2_monitor()
* Description: Process the quary message from the RRH and send the  result to monitor
* Parameters:
*           NONE
* Return Values:
*           TRANS_FAILD : 1
*           TRANS_SUCCESS  : 0
*
*  
*  Data:    2011-09-15
* 
+*****************************************************************/
u_int32_t trans_rrh_rev_forward2_monitor(void *p_info, 
                           size_t len,
                           void * p_rev_buf)
{
    u_int32_t   uw_ret = 0;
    int32_t       w_result = 0;
    struct trans_send_msg_to_monitor *p_msg_info = NULL;
    
    if ((NULL == p_info) || (NULL == p_rev_buf))
    {
        FLOG_ERROR("NULL PTR! \r\n");
        return TRANS_FAILD;  
    }

    p_msg_info  = (struct trans_send_msg_to_monitor *)malloc(SIZEOF_TRANS_SEND_MSG_TO_MONITOR);
    if (NULL == p_msg_info )
    {
        FLOG_ERROR("malloc p_msg_info  error! \r\n");
        return TRANS_FAILD;   
    }
    
    w_result = trans_transaction_get_result(p_info);
    
    p_msg_info->p_payload = p_rev_buf;
    p_msg_info->uc_ack_flag = (u_int8_t)w_result;
    //p_msg_info->us_opration = 0;
    p_msg_info->uw_payload_len = len;
    p_msg_info->p_info = p_info;

    uw_ret = trans_send_rrh_msg(TRANS_SEND_TO_MONITOR, p_msg_info);
    if (TRANS_SUCCESS != uw_ret)
    {
        FLOG_ERROR("Call trans_send_rrh_msg error! uw_ret = %d\r\n", uw_ret);
    
        //return uw_ret;    
    }

    free (p_msg_info);
    
    return uw_ret;   
}

/******************************************************************+
* Function: trans_rrh_rev_forward2_action()
* Description: Process the quary message from the RRH and send the  result to action
* Parameters:
*           NONE
* Return Values:
*           TRANS_FAILD : 1
*           TRANS_SUCCESS  : 0
*
*  
*  Data:    2011-09-15
* 
+*****************************************************************/
u_int32_t trans_rrh_rev_forward2_action(void *p_info, 
                           size_t len,
                           void * p_rev_buf)
{
    u_int32_t   uw_ret = 0;
    //u_int32_t   uw_len = 0;
    struct trans_action_info st_action_info;

    fun_callback f_callback = NULL;

    if ((NULL == p_info) || (NULL == p_rev_buf))
    {
        FLOG_ERROR("NULL PTR! \r\n");
        return TRANS_FAILD;  
    }

    FLOG_DEBUG_TRANS(g_trans_debug_rrh, "Enter! \r\n");

    f_callback = trans_transaction_get_rrh(p_info);
    if (NULL == f_callback)
    {
        FLOG_ERROR("NULL PTR! f_callback\r\n");
        return TRANS_FAILD;  
    }
    
    st_action_info.f_callback = f_callback;
    st_action_info.uw_src_moudle = TRANS_MOUDLE_ACTION;
    st_action_info.p_info = p_info;
    st_action_info.p_action_list = &g_trans_action_list;

    uw_ret = trans_action_add(&st_action_info, len, p_rev_buf);
    
    if (TRANS_SUCCESS != uw_ret)
    {
        FLOG_ERROR("Call trans_action_add error! uw_ret = %d\r\n", uw_ret);
    
        return TRANS_FAILD;    
    }   

    /*Delete flag decide by Action function*/
    #if 0
    uw_ret = trans_transaction_set_comn(p_info, 
                                TRANS_TRANSACTION_FLAG_NO_DELETE,
                                TRANS_MOUDLE_ACTION);
    if (TRANS_SUCCESS != uw_ret)
    {
        FLOG_ERROR("Call trans_transaction_set_comn error! uw_ret = %d\r\n", uw_ret);
    
        return TRANS_FAILD;
    }
    #endif

    FLOG_DEBUG_TRANS(g_trans_debug_rrh, "Exit! \r\n");
    
    return TRANS_SUCCESS;   
}

/******************************************************************+
* Function: trans_rrh_rev_forward2_bs()
* Description: Process the quary message from the RRH and send the  result to bs
* Parameters:
*           NONE
* Return Values:
*           TRANS_FAILD : 1
*           TRANS_SUCCESS  : 0
*
*  
*  Data:    2011-09-15
* 
+*****************************************************************/
u_int32_t trans_rrh_rev_forward2_bs(void *p_info, 
                           size_t len,
                           void * p_rev_buf)
{
    u_int32_t   uw_ret;
    u_int16_t   us_len = 0, us_body_len = 0;
    u_int16_t   us_param_type = 0;
    u_int8_t     uc_param_len = 0;    
    
    struct rrh_monitor_param *p_param_temp = NULL;
    u_int8_t  * p_param_msg = NULL;
    char  * p_gps = NULL;
    int32_t w_value = 0;
    //struct trans_en_queue_msg   st_en_quene;

    int32_t w_result = 0;
    fun_callback f_callback = NULL;

    if ((NULL == p_info) || (NULL == p_rev_buf))
    {
        FLOG_ERROR("NULL PTR! \r\n");
        return TRANS_FAILD;  
    }

    p_param_msg = (u_int8_t *)p_rev_buf;
    
    w_result = trans_transaction_get_result(p_info);

    if (TRANS_ACK_FLAG_OK != w_result)
    {
        //st_en_quene.uc_result = w_result;
    }
    else
    {
        w_result = TRANS_SUCCESS;      
        
        us_body_len = len;   
        p_param_temp = (struct rrh_monitor_param *)p_param_msg;
        
        while (us_len < us_body_len)
        {
            //us_param_type = TRANS_HTONS(p_param_temp->us_param_type);   
            us_param_type = TRANS_HTONS(p_param_temp->us_param_type); 
            uc_param_len = p_param_temp->uc_param_len;
            
            //FLOG_DEBUG_TRANS(g_trans_debug_rrh, "Rev message! us_param_type = 0x%x, uc_param_len = %d\r\n", us_param_type, uc_param_len);
        
            switch (us_param_type)
            {
                /*Query response*/
                case RRH_MSG_LONGITUDE: 
                    
                    p_gps = (char  *)(p_param_temp + 1);
                    
                    FLOG_WARNING("Rev LONGITUDE ! %s\r\n", (p_gps));
                    
                    if (0 == strcmp((char *)(p_gps), ""))
                    {
                        #ifdef TRANS_BS_COMPILE
                        uw_ret = set_global_param("RRU_LONGITUDE", (void *)(RRH_LONGITUDE));
                        if (0 != uw_ret)
                        {
                            FLOG_ERROR("Call set_global_param for RRU_LONGITUDE");
                            w_result = TRANS_FAILD;
                        }
                        #endif

                        FLOG_WARNING("1 Set LONGITUDE ! %s\r\n", (RRH_LONGITUDE));
                    }
                    else
                    {
                        #ifdef TRANS_BS_COMPILE
                        uw_ret = set_global_param("RRU_LONGITUDE", (void *)(p_gps + 1));
                        if (0 != uw_ret)
                        {
                            FLOG_ERROR("Call set_global_param for RRU_LONGITUDE");
                            w_result = TRANS_FAILD;
                        }
                        #endif

                        FLOG_WARNING("2 Set LONGITUDE ! %s\r\n", (p_gps + 1));
                    }
                    
                    break; 
                    
                case RRH_MSG_LATITUDE: 
                    
                    p_gps = (char  *)(p_param_temp + 1);
                    
                    FLOG_WARNING("Rev LATITUDE ! %s\r\n", (p_gps));
                    
                    if (0 == strcmp((char *)(p_gps), ""))
                    {
                        #ifdef TRANS_BS_COMPILE
                        uw_ret = set_global_param("RRU_LATITUDE", (void *)(RRH_LATITUDE));
                        if (0 != uw_ret)
                        {
                            FLOG_ERROR("Call set_global_param for RRU_LATITUDE");
                            w_result = TRANS_FAILD;
                        }
                        #endif
                    
                        FLOG_WARNING("1 Set LATITUDE ! %s\r\n", (RRH_LATITUDE));
                    }
                    else
                    {
                        #ifdef TRANS_BS_COMPILE
                        uw_ret = set_global_param("RRU_LATITUDE", (void *)(p_gps + 1));
                        if (0 != uw_ret)
                        {
                            FLOG_ERROR("Call set_global_param for RRU_LATITUDE");
                            w_result = TRANS_FAILD;
                        }
                        #endif
                        
                        FLOG_WARNING("2 Set LATITUDE ! %s\r\n", (p_gps + 1));
                    }
       
                    break; 
                    
                case RRH_MSG_GPS_CLK_LOCK_VALUE: 
                    w_value = *((u_int8_t *)(p_param_temp + 1));
        
                    FLOG_WARNING("Rev GPS_CLK_LOCK! %d\r\n", w_value);

                    w_result = w_value;

                break; 
        
                case RRH_MSG_CHAN1_NORM_POW_VALUE:
                    w_value = TRANS_NTOHL(*((int32_t *)(p_param_temp + 1)));
                    FLOG_WARNING("CALIBRATE_DIGITAL_POWER0 %d\r\n", w_value);
        
                    #if 0
                    uw_ret = set_global_param("CALIBRATE_DIGITAL_POWER0", (void *)&(w_value));
                    if (0 != uw_ret)
                    {
                        FLOG_ERROR("Call set_global_param for DIGITAL_POWER0 error");
                        w_result = TRANS_FAILD;
                    }
                    #endif
        
                    break;  
        
                case RRH_MSG_CHAN2_NORM_POW_VALUE: 
                    
                    w_value = TRANS_NTOHL(*((int32_t *)(p_param_temp + 1)));
                    FLOG_WARNING("CALIBRATE_DIGITAL_POWER1 %d\r\n", w_value);
        
                    #if 0
                    uw_ret = set_global_param("CALIBRATE_DIGITAL_POWER1", (void *)&(w_value));
                    if (0 != uw_ret)
                    {
                        FLOG_ERROR("Call set_global_param for DIGITAL_POWER1 error");
                        w_result = TRANS_FAILD;
                    }
                    #endif
        
                    break;  
                    
                case RRH_MSG_CHAN1_POWER_NORM_VALUE:
                    w_value = *((int8_t *)(p_param_temp + 1));
                    FLOG_WARNING("CHAN1_POWER_VALUE %d\r\n", w_value);
                
                    #if 0
                    uw_ret = set_global_param("CHAN1_POWER_VALUE", (void *)&(w_value));
                    if (0 != uw_ret)
                    {
                        FLOG_ERROR("Call set_global_param for CHAN1_POWER_VALUE error");
                        w_result = TRANS_FAILD;
                    }
                    #endif
                
                    break;  
                
                case RRH_MSG_CHAN2_POWER_NORM_VALUE: 
                    
                    w_value = *((int8_t *)(p_param_temp + 1));
                    FLOG_WARNING("CHAN2_POWER_VALUE %d\r\n", w_value);
                
                    #if 0
                    uw_ret = set_global_param("CHAN2_POWER_VALUE", (void *)&(w_value));
                    if (0 != uw_ret)
                    {
                        FLOG_ERROR("Call set_global_param for CHAN2_POWER_VALUE error");
                        w_result = TRANS_FAILD;
                    }
                    #endif
                
                    break;  
                    
                /*Config response*/
                case RRH_MSG_CHAN1_SWITCH: /* Channel 1 channel enable switch */
                case RRH_MSG_CHAN2_SWITCH: /* Channel 2 channel enable switch*/
                
                case RRH_MSG_CHAN1_FREQ: /* Channel 1 working frequency */
                case RRH_MSG_CHAN2_FREQ: /* Channel 2 working frequency */  
                
                case RRH_MSG_CHAN1_WORKMODE: /* Channel 1 working mode */
                case RRH_MSG_CHAN2_WORKMODE: /* Channel 2 working mode */
                
                case RRH_MSG_DL_PRESEND_TIME: /* Downlink send lead time   */
                case RRH_MSG_GPS_FAULT_ENABLE:        /* GPS alarm enabled */

                case RRH_MSG_PA_SWITCH_A_CFG: /*PA Switch for Channel 1*/
                case RRH_MSG_PA_SWITCH_B_CFG: /*PA Switch for Channel 2*/
                
                case RRH_MSG_GPS_ENABLE_CFG:        /*Config GPS Enable*/

                case RRH_MSG_TX_LEN: /*TX time length*/
                case RRH_MSG_RX_LEN: /*RX time length*/
                
                case RRH_MSG_TTG: /*TTG*/
                case RRH_MSG_RTG: /*RTG*/
                
                case RRH_MSG_NORM_OUTPUT_POWER:        /*Config GPS Enable*/

                case RRH_MSG_AGC_ENABLE:
                
                case RRH_MSG_CHAN1_RX_PGC:
                case RRH_MSG_CHAN2_RX_PGC:

                case RRH_MSG_IQ_DATA_PORT:  

                case RRH_MSG_CARRIER_INFO:  
                    
                case RRH_MSG_BYTEORDER:

                    
                    w_result = TRANS_SUCCESS;

                    FLOG_INFO("Config RRH OK: 0x%x \r\n", us_param_type);
                    break;    
                    
                default:
                    
                    FLOG_ERROR("Rev unknow message! us_param_type = 0x%x\r\n", us_param_type);
        
                    /*????????????????*/
                    return TRANS_FAILD;
            
            }  
        
            if (TRANS_SUCCESS != w_result)
            {
                break;
            }
        
            us_len = us_len + uc_param_len +SIZEOF_RRH_MONITOR_PARAM;         
            p_param_temp = (struct rrh_monitor_param *)(p_param_msg + us_len);
        
        }   

    }
    
    uw_ret = trans_transaction_set_result(p_info, w_result);
    if (TRANS_SUCCESS != uw_ret)
    {
        FLOG_ERROR("Call trans_result_set_transaction error! uw_ret = %d\r\n", uw_ret);
    
        return TRANS_FAILD;    
    } 

    f_callback = trans_transaction_get_rrh(p_info);
    if (NULL == f_callback)
    {
        FLOG_DEBUG_TRANS(g_trans_debug_rrh, "NULL PTR! f_callback\r\n");
    }
    else
    {
        (*(f_callback))(p_info, 0, NULL);
    }
    
    uw_ret = trans_transaction_set_comn(p_info, 
                                TRANS_TRANSACTION_FLAG_DELETE,
                                TRANS_MOUDLE_BS);
    if (TRANS_SUCCESS != uw_ret)
    {
        FLOG_ERROR("Call trans_transaction_set_comn error! uw_ret = %d\r\n", uw_ret);
    
        return TRANS_FAILD;
    }

    //uw_ret = w_result;
    
    FLOG_DEBUG_TRANS(g_trans_debug_rrh, "Exit uw_ret = %d.\r\n", uw_ret);

    return TRANS_SUCCESS;  
}

#ifdef TRANS_UI_COMPILE
/******************************************************************+
* Function: trans_rrh_rev_forward2_ui()
* Description: Process the quary message from the RRH and send the  result to ui
* Parameters:
*           NONE
* Return Values:
*           TRANS_FAILD : 1
*           TRANS_SUCCESS  : 0
*
*  
*  Data:    2012-07-17
* 
+*****************************************************************/
u_int32_t trans_rrh_rev_forward2_ui(void *p_info, 
                           size_t len,
                           void * p_rev_buf)
{
    u_int32_t   uw_ret;
    u_int16_t   us_len = 0, us_body_len = 0;
    u_int16_t   us_param_type = 0;
    u_int8_t     uc_param_len = 0;    
    
    struct rrh_monitor_param *p_param_temp = NULL;
    u_int8_t  * p_param_msg = NULL;

    int32_t w_result = 0;
    fun_callback f_callback = NULL;

    if ((NULL == p_info) || (NULL == p_rev_buf))
    {
        FLOG_ERROR("NULL PTR! \r\n");
        return TRANS_FAILD;  
    }

    p_param_msg = (u_int8_t *)p_rev_buf;
    
    w_result = trans_transaction_get_result(p_info);

    if (TRANS_ACK_FLAG_OK != w_result)
    {
    }
    else
    {
        w_result = TRANS_SUCCESS;      
        
        us_body_len = len;   
        p_param_temp = (struct rrh_monitor_param *)p_param_msg;
        
        while (us_len < us_body_len)
        {
            //us_param_type = TRANS_HTONS(p_param_temp->us_param_type);   
            us_param_type = TRANS_HTONS(p_param_temp->us_param_type); 
            uc_param_len = p_param_temp->uc_param_len;
            
            //FLOG_DEBUG_TRANS(g_trans_debug_rrh, "Rev message! us_param_type = 0x%x, uc_param_len = %d\r\n", us_param_type, uc_param_len);

            uw_ret = trans_rrh_type_get_len(us_param_type, &(uc_param_len));
            if (TRANS_SUCCESS != uw_ret)
            {
                FLOG_ERROR("Call trans_rrh_type_get_len error! uw_ret = %d\r\n", uw_ret);

                w_result = TRANS_FAILD;  
            }
        
            if (TRANS_SUCCESS != w_result)
            {
                break;
            }
        
            us_len = us_len + uc_param_len +SIZEOF_RRH_MONITOR_PARAM;         
            p_param_temp = (struct rrh_monitor_param *)(p_param_msg + us_len);
        
        }   

    }
    
    uw_ret = trans_transaction_set_result(p_info, w_result);
    if (TRANS_SUCCESS != uw_ret)
    {
        FLOG_ERROR("Call trans_result_set_transaction error! uw_ret = %d\r\n", uw_ret);
    
        return TRANS_FAILD;    
    } 

    f_callback = trans_transaction_get_rrh(p_info);
    if (NULL == f_callback)
    {
        FLOG_DEBUG_TRANS(g_trans_debug_rrh, "NULL PTR! f_callback\r\n");
    }
    else
    {
        (*(f_callback))(p_info, len, p_rev_buf);
    }
    
    uw_ret = trans_transaction_set_comn(p_info, 
                                TRANS_TRANSACTION_FLAG_DELETE,
                                TRANS_MOUDLE_BS);
    if (TRANS_SUCCESS != uw_ret)
    {
        FLOG_ERROR("Call trans_transaction_set_comn error! uw_ret = %d\r\n", uw_ret);
    
        return TRANS_FAILD;
    }
    
    FLOG_DEBUG_TRANS(g_trans_debug_rrh, "Exit uw_ret = %d.\r\n", uw_ret);

    return TRANS_SUCCESS;  
}
#endif


/*****************************************************************************+
* Function: trans_rrh_func_alert()
* Description: 
* Parameters:
*           NONE
* Return Values:
*           TRANS_FAILD : 1
*           TRANS_SUCCESS  : 0
*
*  
*  Data:    2011-08-01
* 
+*****************************************************************************/
int trans_rrh_func_alert(void *p_info, 
                           size_t len,
                           void * p_rev_buf)
{
    struct rrh_monitor_header  *p_rrh_rev_header = NULL;
    u_int32_t   uw_ret = 0;

    u_int16_t   us_body_len = 0;
    
    //u_int8_t   uc_rep_flag = 0;
    u_int8_t *p_param_alarm = NULL;

    struct rrh_monitor_param_alarm *p_alarm_temp = NULL;

    u_int16_t   us_alarm_type = 0;
    
    #ifndef TRANS_UI_COMPILE
    u_int16_t us_len =0;

    struct trans_send_msg_to_agent st_msg_info;
    struct trans_agent_alert_info        st_alert_info;
    #endif
    
    u_int8_t *p_rev_msg = NULL;
    
    u_int16_t   *p_new_crc = NULL;
    u_int16_t   us_crc_val = 0;
    int32_t       w_ret = 0;
    int32_t       w_send_len = 0;

    int32_t      w_result = 0;
        
    FLOG_DEBUG_TRANS(g_trans_debug_rrh, "Enter \r\n");
    
    if ((NULL == p_info) || (NULL == p_rev_buf))
    {
        FLOG_ERROR("NULL PTR! \r\n");
        return TRANS_FAILD;  
    }
    
    (void) len;
    
    p_rev_msg = (u_int8_t *)p_rev_buf;
    
    p_rrh_rev_header = (struct rrh_monitor_header *)p_rev_msg;
    p_param_alarm = p_rev_msg + SIZEOF_RRH_MONITOR_HEADER;
    
    us_body_len = TRANS_NTOHS(p_rrh_rev_header->us_body_len);    
    
    w_result = trans_transaction_get_result(p_info);
    /*Error*/
    if (0 > w_result)
    {
        FLOG_ERROR("Call trans_transaction_get_result error! w_result = %d\r\n", w_result);
        return TRANS_FAILD;
    }
    
    #ifndef TRANS_UI_COMPILE
    
    if (RRH_MONITOR_REP_FLAG_OK == w_result)
    {
        /*Analysis the param*/
        while (1)
        {
            p_alarm_temp = (struct rrh_monitor_param_alarm *)p_param_alarm;
    
            us_alarm_type = TRANS_NTOHS(p_alarm_temp->us_param_type); 
            
            p_param_alarm = p_rev_msg + SIZEOF_RRH_MONITOR_HEADER + us_len;

            FLOG_WARNING("Recive RRH alarm. Type:0x%x, Value: %d  \r\n", us_alarm_type, p_alarm_temp->uc_param_value);

            if (us_len >= us_body_len)
            {
                break;
            }
    
            /*Param Length error*/
            if (1 != p_alarm_temp->uc_param_len)
            {
                w_result = RRH_MONITOR_REP_FLAG_OTHER_ERR;
                p_alarm_temp->us_param_type = (p_alarm_temp->us_param_type|RRH_MONITOR_REP_ERR_LEN_ILLEGAL);
                
                FLOG_ERROR("Param length error! uc_param_len = %d\r\n", p_alarm_temp->uc_param_len);
                //FLOG_INFO("us_param_type = %d \r\n", p_alarm_temp->us_param_type);
                break;                     
            }
    
            /*Param Type error*/
            if ((RRH_MSG_POWER_FAULT > us_alarm_type)
                || (RRH_MSG_DL_VSWR2 < us_alarm_type))
            {
                //uc_rep_flag = RRH_MONITOR_REP_FLAG_P_ERR;                
                //p_alarm_temp->us_param_type = (p_alarm_temp->us_param_type|RRH_MONITOR_REP_ERR_IDENTIFY_WRY);
                
                FLOG_ERROR("Param type error!us_param_type = %d\r\n", us_alarm_type);
                //FLOG_INFO("us_param_type = %d \r\n", p_alarm_temp->us_param_type);
                break;    
            }
    
            /*Param Value error*/
            if (RRH_MONITOR_AlARM_CLOSE < p_alarm_temp->uc_param_value)
            {
                w_result = RRH_MONITOR_REP_FLAG_OTHER_ERR;                
                p_alarm_temp->us_param_type = (p_alarm_temp->us_param_type|RRH_MONITOR_REP_INVALID_PARAM);
                
                FLOG_ERROR("Param value error!uc_param_value = %d\r\n", p_alarm_temp->uc_param_value);
                //FLOG_INFO("us_param_type = %d \r\n", p_alarm_temp->us_param_type);
                break;    
            }
       
            FLOG_DEBUG("Send alarm 0x%x to Agent \r\n", us_alarm_type);
    
            /*Send alert to Agent*/
            st_msg_info.f_callback = NULL;
            st_msg_info.p_reqs_msg = "alert";
            
            st_alert_info.us_alarm_id = us_alarm_type;
            st_alert_info.w_alarm_value = p_alarm_temp->uc_param_value;
    
            st_msg_info.p_resp_msg = &st_alert_info;
            st_msg_info.p_info = p_info;
                
            uw_ret = trans_send_rrh_msg(TRANS_SEND_TO_AGENT, &st_msg_info);
            /*Error*/
            if (TRANS_SUCCESS != uw_ret)
            {
                FLOG_ERROR("Call trans_send_rrh_msg error! uw_ret = %d\r\n", uw_ret);
                return TRANS_FAILD;
            }
    
            us_len = us_len + SIZEOF_RRH_MONITOR_PARAM_ALARM;  

            break;    
    
        } 
    }

    #else
    fun_callback f_callback = NULL;

    p_alarm_temp = (struct rrh_monitor_param_alarm *)p_param_alarm;    
    us_alarm_type = TRANS_NTOHS(p_alarm_temp->us_param_type); 

    
    f_callback = g_trans_register_exe_func[TRANS_REGISTER_FUN_WIRELESS_MSG_PRO + 3].f_callback;
    if (NULL == f_callback)
    {
        FLOG_DEBUG_TRANS(g_trans_debug_rrh, "NULL PTR! f_callback\r\n");
    }
    else
    {
        (*(f_callback))(p_info, us_body_len, p_param_alarm);
    }
    
    #endif
    
    /*Send alert ack to RRH*/           
    //#if 0

    if (0 == g_trans_rrh_stop_alert_ack)
    {
        FLOG_WARNING("Send alarm ACK 0x%x to RRH \r\n", us_alarm_type);
        
        p_rrh_rev_header->uc_resp_flag = w_result;
        memset(p_rrh_rev_header->a_time, 0, 7);
            
        /*Build Msg CRC */    /*calculate CRC*/
        us_crc_val = trans_rrh_cal_crc16((u_int8_t*)p_rev_msg, us_body_len + SIZEOF_RRH_MONITOR_HEADER); 
        
        p_new_crc = (u_int16_t  *)(p_rev_msg + us_body_len + SIZEOF_RRH_MONITOR_HEADER);
        *p_new_crc = TRANS_HTONS(us_crc_val);
        
        w_send_len = us_body_len + SIZEOF_RRH_MONITOR_HEADER + 2;
        
        trans_debug_msg_print(p_rev_msg, w_send_len, g_trans_debug_rrh);    
        
        w_ret = send(g_trans_rrh_socket, p_rev_msg, w_send_len, 0);
        
        if(w_ret <= 0)
        {
            //close(sock);
            FLOG_ERROR("send error! w_ret = %d\r\n", w_ret);
            return TRANS_FAILD;
        } 

    }
       
    //#endif

    uw_ret = trans_transaction_set_comn(p_info, 
                                TRANS_TRANSACTION_FLAG_DELETE,
                                TRANS_MOUDLE_RRH);
    if (TRANS_SUCCESS != uw_ret)
    {
        FLOG_ERROR("Call trans_transaction_set_comn error! uw_ret = %d\r\n", uw_ret);
    
        return TRANS_FAILD;
    }
        
    FLOG_DEBUG_TRANS(g_trans_debug_rrh, "Exit \r\n");
    
    return TRANS_SUCCESS;
}

/*****************************************************************************+
* Function: trans_rrh_func_query()
* Description: Process the quary message from the RRH
* Parameters:
*           NONE
* Return Values:
*           TRANS_FAILD : 1
*           TRANS_SUCCESS  : 0
*
*  
*  Data:    2011-03-23
* 
+*****************************************************************************/
int trans_rrh_func_query(void *p_info, 
                           size_t len,
                           void * p_rev_buf)
{
    struct rrh_monitor_header  *p_rrh_rev_header = NULL;
    u_int32_t   uw_ret = 0;
    u_int16_t   us_body_len = 0;
    u_int32_t   uw_src_moudle = TRANS_MOUDLE_BUF;
    
    u_int8_t *p_param_quary = NULL;
    u_int8_t *p_rev_msg = NULL;
   
    FLOG_DEBUG_TRANS(g_trans_debug_rrh, "Enter \r\n");
    
    if ((NULL == p_info) || (NULL == p_rev_buf))
    {
        FLOG_ERROR("NULL PTR! \r\n");
        return TRANS_FAILD;  
    }

    (void)len;
    
    p_rev_msg = (u_int8_t *)p_rev_buf;
    p_rrh_rev_header = (struct rrh_monitor_header *)p_rev_msg;    
    
    us_body_len = TRANS_NTOHS(p_rrh_rev_header->us_body_len);   
    p_param_quary = p_rev_msg + SIZEOF_RRH_MONITOR_HEADER; 

    uw_src_moudle = trans_transaction_get_comn_src(p_info); 

    switch (uw_src_moudle)
    {
        /*Query Message From AGENT */   
        case TRANS_MOUDLE_AGENT: 
            uw_ret = trans_rrh_rev_forward2_agent(p_info, us_body_len, p_param_quary);
            if (TRANS_SUCCESS != uw_ret)
            {
                FLOG_ERROR("Call trans_rrh_rev_forward2_agent error! uw_ret = %d\r\n", uw_ret);
            
                //return uw_ret;    
            }
            
            break;   

        /*Query Message From MONITOR */   
        case TRANS_MOUDLE_MONITOR: 
            uw_ret = trans_rrh_rev_forward2_monitor(p_info, us_body_len, p_param_quary);
            if (TRANS_SUCCESS != uw_ret)
            {
                FLOG_ERROR("Call trans_rrh_rev_forward2_monitor error! uw_ret = %d\r\n", uw_ret);
            
                //return uw_ret;    
            }

            break;   
            
        /*Query Message From ACTION */   
        case TRANS_MOUDLE_ACTION: 
            uw_ret = trans_rrh_rev_forward2_action(p_info, us_body_len, p_param_quary);
            if (TRANS_SUCCESS != uw_ret)
            {
                FLOG_ERROR("Call trans_rrh_rev_forward2_action error! uw_ret = %d\r\n", uw_ret);
            
                //return uw_ret;    
            }

            break;   

        /*Query Message From WIRELESS */   
        case TRANS_MOUDLE_BS: 

            uw_ret = trans_rrh_rev_forward2_bs(p_info, us_body_len, p_param_quary);
            if (TRANS_SUCCESS != uw_ret)
            {
                FLOG_ERROR("Call trans_rrh_rev_forward2_bs error! uw_ret = %d\r\n", uw_ret);
            
                //return uw_ret;    
            }
            
            break;   

        #ifdef TRANS_UI_COMPILE
        /*Query Message From UI testband */   
        case TRANS_MOUDLE_UI: 
        
            uw_ret = trans_rrh_rev_forward2_ui(p_info, us_body_len, p_param_quary);
            if (TRANS_SUCCESS != uw_ret)
            {
                FLOG_ERROR("Call trans_rrh_rev_forward2_ui error! uw_ret = %d\r\n", uw_ret);
            
                //return uw_ret;    
            }
            
            break;   
        #endif
            
        /*Query Message From TRANS */   
        case TRANS_MOUDLE_LOCAL: 
        /*Query Message From RRH */   
        case TRANS_MOUDLE_RRH: 
        /*Query Message From BS */   
        case TRANS_MOUDLE_WIRELESS: 
        /*Query Message From MS */   
        case TRANS_MOUDLE_MS: 

        /*Others*/
        default:
             
            FLOG_ERROR("Source Module error!src_moudle = %d\r\n", uw_src_moudle);
            break;
     
    }  

    FLOG_DEBUG_TRANS(g_trans_debug_rrh, "Exit \r\n");
    
    return uw_ret;  

}

/*****************************************************************************+
* Function: trans_rrh_func_config()
* Description: Process the config message from the RRH
* Parameters:
*           NONE
* Return Values:
*           TRANS_FAILD : 1
*           TRANS_SUCCESS  : 0
*
*  
*  Data:    2011-03-23
* 
+*****************************************************************************/
int trans_rrh_func_config(void *p_info, 
                           size_t len,
                           void * p_rev_buf)
{
    struct rrh_monitor_header  *p_rrh_rev_header = NULL;
    u_int32_t uw_ret;
    u_int16_t   us_body_len = 0;
    u_int32_t   uw_src_moudle = TRANS_MOUDLE_BUF;

    u_int8_t *p_param_config = NULL;
    u_int8_t *p_rev_msg = NULL;

    FLOG_DEBUG_TRANS(g_trans_debug_rrh, "Enter \r\n");
    
    if ((NULL == p_info) || (NULL == p_rev_buf))
    {
        FLOG_ERROR("NULL PTR! \r\n");
        return TRANS_FAILD;  
    }
    
    (void)len;

    p_rev_msg = (u_int8_t *)p_rev_buf;
    p_rrh_rev_header = (struct rrh_monitor_header *)p_rev_msg;    

    us_body_len = TRANS_NTOHS(p_rrh_rev_header->us_body_len);   
    p_param_config = p_rev_msg + SIZEOF_RRH_MONITOR_HEADER; 

    uw_src_moudle = trans_transaction_get_comn_src(p_info); 
    
    switch (uw_src_moudle)
    {
        /*Config message From ACTION */   
        case TRANS_MOUDLE_ACTION: 
            uw_ret = trans_rrh_rev_forward2_action(p_info, us_body_len, p_param_config);
            if (TRANS_SUCCESS != uw_ret)
            {
                FLOG_ERROR("Call trans_rrh_rev_forward2_action error! uw_ret = %d\r\n", uw_ret);
            
                //return uw_ret;    
            }
    
            break;   
    
        /*Config Message From WIRELESS */   
        case TRANS_MOUDLE_BS: 
    
            uw_ret = trans_rrh_rev_forward2_bs(p_info, us_body_len, p_param_config);
            if (TRANS_SUCCESS != uw_ret)
            {
                FLOG_ERROR("Call trans_rrh_rev_forward2_bs error! uw_ret = %d\r\n", uw_ret);
            
                //return uw_ret;    
            }
            
            break;   
            
        /*Config Message From MONITOR */   
        case TRANS_MOUDLE_MONITOR: 
            uw_ret = trans_rrh_rev_forward2_monitor(p_info, us_body_len, p_param_config);
            if (TRANS_SUCCESS != uw_ret)
            {
                FLOG_ERROR("Call trans_rrh_rev_forward2_monitor error! uw_ret = %d\r\n", uw_ret);
            
                //return uw_ret;    
            }
        
            break;   
            
        #ifdef TRANS_UI_COMPILE
        /*Query Message From UI testband */   
        case TRANS_MOUDLE_UI: 
        
            uw_ret = trans_rrh_rev_forward2_ui(p_info, us_body_len, p_param_config);
            if (TRANS_SUCCESS != uw_ret)
            {
                FLOG_ERROR("Call trans_rrh_rev_forward2_ui error! uw_ret = %d\r\n", uw_ret);
            
                //return uw_ret;    
            }
            
            break; 
        #endif
            
        /*Config Message From AGENT */   
        case TRANS_MOUDLE_AGENT: 
        /*Config Message From TRANS */   
        case TRANS_MOUDLE_LOCAL: 
        /*Config Message From RRH */   
        case TRANS_MOUDLE_RRH: 
        /*Config Message From BS */   
        case TRANS_MOUDLE_WIRELESS: 
        /*Config Message From MS */   
        case TRANS_MOUDLE_MS: 
    
        /*Others*/
        default:
             
            FLOG_ERROR("Source Module error!src_moudle = %d\r\n", uw_src_moudle);
            break;
     
    }  
   
    FLOG_DEBUG_TRANS(g_trans_debug_rrh, "Exit \r\n");
    
    return TRANS_SUCCESS;  
}

/*****************************************************************************+
* Function: trans_rrh_func_heartbeat()
* Description: Process the heartbeat message from the RRH
* Parameters:
*           NONE
* Return Values:
*           NONE
*
*  
*  Data:    2011-03-23
* 
+*****************************************************************************/
int trans_rrh_func_heartbeat(void *p_info, 
                           size_t len,
                           void * p_rev_buf )
{
    u_int32_t uw_ret;
    
    (void) p_info;
    (void) len;
    (void) p_rev_buf;
    
    FLOG_DEBUG_TRANS(g_trans_debug_rrh, "Enter \r\n");
    //FLOG_ERROR("Enter \r\n");

    /*g_trans_rrh_heartbeat_num = 0;*/
    g_trans_rrh_heartbeat.uc_count = 0;
    
    uw_ret = trans_transaction_set_comn(p_info, 
                                TRANS_TRANSACTION_FLAG_DELETE,
                                TRANS_MOUDLE_RRH);
    if (TRANS_SUCCESS != uw_ret)
    {
        FLOG_ERROR("Call trans_transaction_set_comn error! uw_ret = %d\r\n", uw_ret);
    
        return TRANS_FAILD;
    }
    
    return TRANS_SUCCESS; 
}

/*****************************************************************************+
* Function: trans_rrh_parse_alert()
* Description: 
* Parameters:
*           NONE
* Return Values:
*           TRANS_FAILD : 1
*           TRANS_SUCCESS  : 0
*
*  
*  Data:    2011-09-05
* 
+*****************************************************************************/
u_int32_t trans_rrh_parse_alert(void *p_info, 
                           size_t len,
                           void * p_rev_buf)
{
    u_int32_t uw_ret = 0;
    u_int8_t     uc_exe_flag;    /**/
    u_int32_t   uw_func_id;     /*Funtion Callback ID*/    
    
    if (NULL == p_rev_buf)
    {
        FLOG_ERROR("NULL PTR! \r\n");
        return TRANS_FAILD;
    }  
    
    FLOG_DEBUG_TRANS(g_trans_debug_rrh, "Enter \r\n");

    void ** p_info_tmp = NULL;
    p_info_tmp = p_info;
    
    uc_exe_flag = TRANS_TRANSACTION_FLAG_EXE_NOW;    /**/
    uw_func_id = TRANS_REGISTER_FUN_RRH_MSG_PRO + 1;     /*Funtion Callback ID*/  
    
    /*Fill in transaction*/
    uw_ret = trans_transaction_creat(p_info);
    /*Error*/
    if (TRANS_SUCCESS != uw_ret)
    {
        FLOG_ERROR("Call trans_transaction_creat for alert error! uw_ret = %d\r\n", uw_ret);
        return TRANS_FAILD;                
    }

    uw_ret = trans_transaction_set_func(*p_info_tmp, 
                                uc_exe_flag,
                                uw_func_id,
                                len,
                                p_rev_buf);
    if (TRANS_SUCCESS != uw_ret)
    {
        FLOG_ERROR("Call trans_transaction_set_func error! uw_ret = %d\r\n", uw_ret);
    
        return TRANS_FAILD;
    }

    #if 0
    uw_ret = trans_transaction_set_rrh(*p_info_tmp, 
                                            NULL, 
                                            len, 
                                            p_rev_buf);
    if (TRANS_SUCCESS != uw_ret)
    {
        FLOG_ERROR("Call trans_transaction_set_rrh error! uw_ret = %d\r\n", uw_ret);
        return TRANS_FAILD;
    }
    #endif

    return TRANS_SUCCESS;
}

/*****************************************************************************+
* Function: trans_rrh_parse_query()
* Description: 
* Parameters:
*           NONE
* Return Values:
*           TRANS_FAILD : 1
*           TRANS_SUCCESS  : 0
*
*  
*  Data:    2011-09-05
* 
+*****************************************************************************/
u_int32_t trans_rrh_parse_query(void *p_info, 
                           size_t len,
                           void * p_rev_buf)
{
    struct rrh_monitor_header  *p_rrh_header = NULL;
    
    u_int32_t uw_ret = 0;
    u_int8_t     uc_exe_flag;    /**/
    u_int32_t   uw_func_id;     /*Funtion Callback ID*/    
    u_int16_t   us_serial_number = 0;
    //void * p_timer_id = NULL;  
    //u_int8_t uc_find_flag = 0;
    
    void ** p_info_tmp = NULL;
    
    if (NULL == p_rev_buf)
    {
        FLOG_ERROR("NULL PTR! \r\n");
        return TRANS_FAILD;
    }  
    
    FLOG_DEBUG_TRANS(g_trans_debug_rrh, "Enter \r\n");
    
    p_rrh_header = (struct rrh_monitor_header *)p_rev_buf;
    
    us_serial_number = TRANS_NTOHS(p_rrh_header->us_serial_number);
    
    /*Get transaction info and Check time out*/
    uw_ret = trans_transaction_get_out_by_mac(us_serial_number,
                                                     g_trans_rrh_eqp_config.a_rrh_mac_addr,
                                                     p_info);    
    p_info_tmp = p_info;
    
    if ((NULL == *p_info_tmp) || (TRANS_SUCCESS != uw_ret))
    {
        FLOG_ERROR("Timeout, can not find the transaction info!uw_ret = %d.\r\n", uw_ret);
        return TRANS_FAILD;   
    }
    
    uc_exe_flag = TRANS_TRANSACTION_FLAG_EXE_NOW;    /**/
    uw_func_id = TRANS_REGISTER_FUN_RRH_MSG_PRO + 2;     /*Funtion Callback ID*/  
    
     /*Fill in transaction*/
    uw_ret = trans_transaction_set_func(*p_info_tmp, 
                                uc_exe_flag,
                                uw_func_id,
                                len,
                                p_rev_buf);
    if (TRANS_SUCCESS != uw_ret)
    {
        FLOG_ERROR("Call trans_transaction_set_func error! uw_ret = %d\r\n", uw_ret);
    
        return TRANS_FAILD;
    }
    
    #if 0
    uw_ret = trans_timer_search(us_serial_number, 
                             g_trans_rrh_eqp_config.a_rrh_mac_addr,
                             &uc_find_flag,
                             p_info,
                             &p_timer_id);
    if (TRANS_SUCCESS != uw_ret)
    {
         FLOG_ERROR("Call trans_timer_search error! uw_ret = %d\r\n", uw_ret);    
         return TRANS_FAILD;    
    }
    
    /*if timeout ,discard message:  do nothing*/
    if (1 != uc_find_flag)
    {
         FLOG_ERROR("Time out error! uc_find_flag = %d\r\n", uc_find_flag);
         return TRANS_FAILD;    
    }
    
    void ** p_info_tmp = NULL;
    p_info_tmp = p_info;

    uc_exe_flag = TRANS_TRANSACTION_FLAG_EXE_NOW;    /**/
    uw_func_id = TRANS_REGISTER_FUN_RRH_MSG_PRO + 2;     /*Funtion Callback ID*/  

     /*Fill in transaction*/
    uw_ret = trans_transaction_set_func(*p_info_tmp, 
                                uc_exe_flag,
                                uw_func_id,
                                len,
                                p_rev_buf);
    if (TRANS_SUCCESS != uw_ret)
    {
        FLOG_ERROR("Call trans_transaction_set_func error! uw_ret = %d\r\n", uw_ret);

        return TRANS_FAILD;
    }
    
    struct trans_timer *p_timer = (struct trans_timer *)p_timer_id;    
    
    uw_ret = trans_transaction_set_rrh(*p_info_tmp, 
                                            p_timer->f_callback, 
                                            len, 
                                            p_rev_buf);
    if (TRANS_SUCCESS != uw_ret)
    {
        FLOG_ERROR("Call trans_transaction_set_rrh error! uw_ret = %d\r\n", uw_ret);
        return TRANS_FAILD;
    }

    /*Delete Timer*/
    uw_ret = trans_timer_delete(p_timer_id);
    if (TRANS_SUCCESS != uw_ret) 
    {   
        FLOG_ERROR("Call trans_timer_delete error! uw_ret = %d\r\n", uw_ret);
        return TRANS_FAILD;     
    } 

    #endif

    return TRANS_SUCCESS;
}

/*****************************************************************************+
* Function: trans_rrh_parse_config()
* Description: 
* Parameters:
*           NONE
* Return Values:
*           TRANS_FAILD : 1
*           TRANS_SUCCESS  : 0
*
*  
*  Data:    2011-09-05
* 
+*****************************************************************************/
u_int32_t trans_rrh_parse_config(void *p_info, 
                           size_t len,
                           void * p_rev_buf)
{
    struct rrh_monitor_header  *p_rrh_header = NULL;
    
    u_int32_t uw_ret = 0;
    u_int8_t     uc_exe_flag;    /**/
    u_int32_t   uw_func_id;     /*Funtion Callback ID*/    
    u_int16_t   us_serial_number = 0;
    //void * p_timer_id = NULL;  
    //u_int8_t uc_find_flag = 0;

    void ** p_info_tmp = NULL;
    
    if (NULL == p_rev_buf)
    {
        FLOG_ERROR("NULL PTR! \r\n");
        return TRANS_FAILD;
    }  
    
    FLOG_DEBUG_TRANS(g_trans_debug_rrh, "Enter \r\n");
    
    p_rrh_header = (struct rrh_monitor_header *)p_rev_buf;
    
    us_serial_number = TRANS_NTOHS(p_rrh_header->us_serial_number);

    /*Get transaction info and Check time out*/
    uw_ret = trans_transaction_get_out_by_mac(us_serial_number,
                                                     g_trans_rrh_eqp_config.a_rrh_mac_addr,
                                                     p_info);    
    p_info_tmp = p_info;
    
    if ((NULL == *p_info_tmp) || (TRANS_SUCCESS != uw_ret))
    {
        FLOG_ERROR("Timeout, can not find the transaction info!uw_ret = %d.\r\n", uw_ret);
        return TRANS_FAILD;   
    }

    uc_exe_flag = TRANS_TRANSACTION_FLAG_EXE_NOW;    /**/
    uw_func_id = TRANS_REGISTER_FUN_RRH_MSG_PRO + 3;     /*Funtion Callback ID*/  
    
     /*Fill in transaction*/
    uw_ret = trans_transaction_set_func(*p_info_tmp, 
                                uc_exe_flag,
                                uw_func_id,
                                len,
                                p_rev_buf);
    if (TRANS_SUCCESS != uw_ret)
    {
        FLOG_ERROR("Call trans_transaction_set_func error! uw_ret = %d\r\n", uw_ret);
    
        return TRANS_FAILD;
    }

    #if 0
    uw_ret = trans_timer_search(us_serial_number, 
                             g_trans_rrh_eqp_config.a_rrh_mac_addr,
                             &uc_find_flag,
                             p_info,
                             &p_timer_id);
    if (TRANS_SUCCESS != uw_ret)
    {
         FLOG_ERROR("Call trans_timer_search error! uw_ret = %d\r\n", uw_ret);    
         return TRANS_FAILD;    
    }
    
    /*if timeout ,discard message:  do nothing*/
    if (1 != uc_find_flag)
    {
         FLOG_ERROR("Time out error! uc_find_flag = %d\r\n", uc_find_flag);
         return TRANS_FAILD;    
    }
   
    void ** p_info_tmp = NULL;
    p_info_tmp = p_info;
    
    uc_exe_flag = TRANS_TRANSACTION_FLAG_EXE_NOW;    /**/
    uw_func_id = TRANS_REGISTER_FUN_RRH_MSG_PRO + 3;     /*Funtion Callback ID*/  

     /*Fill in transaction*/
    uw_ret = trans_transaction_set_func(*p_info_tmp, 
                                uc_exe_flag,
                                uw_func_id,
                                len,
                                p_rev_buf);
    if (TRANS_SUCCESS != uw_ret)
    {
        FLOG_ERROR("Call trans_transaction_set_func error! uw_ret = %d\r\n", uw_ret);

        return TRANS_FAILD;
    }

    struct trans_timer *p_timer = (struct trans_timer *)p_timer_id;    
    
    uw_ret = trans_transaction_set_rrh(*p_info_tmp, 
                                            p_timer->f_callback, 
                                            len, 
                                            p_rev_buf);
    if (TRANS_SUCCESS != uw_ret)
    {
        FLOG_ERROR("Call trans_transaction_set_rrh error! uw_ret = %d\r\n", uw_ret);
        return TRANS_FAILD;
    }
    
    /*Delete Timer*/
    uw_ret = trans_timer_delete(p_timer_id);
    if (TRANS_SUCCESS != uw_ret) 
    {   
        FLOG_ERROR("Call trans_timer_delete error! uw_ret = %d\r\n", uw_ret);
        return TRANS_FAILD;     
    } 
    #endif
    
    return TRANS_SUCCESS;
}

/*****************************************************************************+
* Function: trans_rrh_parse_heartbeat()
* Description: 
* Parameters:
*           NONE
* Return Values:
*           TRANS_FAILD : 1
*           TRANS_SUCCESS  : 0
*
*  
*  Data:    2011-09-05
* 
+*****************************************************************************/
u_int32_t trans_rrh_parse_heartbeat(void *p_info, 
                           size_t len,
                           void * p_rev_buf)
{
    u_int32_t uw_ret = 0;
    u_int8_t     uc_exe_flag;    /**/
    u_int32_t   uw_func_id;     /*Funtion Callback ID*/    
    
    if (NULL == p_rev_buf)
    {
        FLOG_ERROR("NULL PTR! \r\n");
        return TRANS_FAILD;
    }  
    
    FLOG_DEBUG_TRANS(g_trans_debug_rrh, "Enter \r\n");

    uc_exe_flag = TRANS_TRANSACTION_FLAG_EXE_NOW;    /**/
    uw_func_id = TRANS_REGISTER_FUN_RRH_MSG_PRO + 4;     /*Funtion Callback ID*/  
    
    uw_ret = trans_transaction_creat(p_info);
    /*Error*/
    if (TRANS_SUCCESS != uw_ret)
    {
        FLOG_ERROR("Call trans_transaction_creat for heartbeat error! uw_ret = %d\r\n", uw_ret);
        return TRANS_FAILD;                
    }
    
    void ** p_info_tmp = NULL;
    p_info_tmp = p_info;

    uw_ret = trans_transaction_set_func(*p_info_tmp, 
                                uc_exe_flag,
                                uw_func_id,
                                len,
                                p_rev_buf);
    if (TRANS_SUCCESS != uw_ret)
    {
        FLOG_ERROR("Call trans_transaction_set_func error! uw_ret = %d\r\n", uw_ret);
    
        return TRANS_FAILD;
    }

    #if 0
    uw_ret = trans_transaction_set_rrh(*p_info_tmp, 
                                            NULL, 
                                            len, 
                                            p_rev_buf);
    if (TRANS_SUCCESS != uw_ret)
    {
        FLOG_ERROR("Call trans_transaction_set_rrh error! uw_ret = %d\r\n", uw_ret);
        return TRANS_FAILD;
    }
    #endif

    return TRANS_SUCCESS;
}


/*****************************************************************************+
* Function: trans_rrh_parse_msg()
* Description: Parse the message from the RRH
* Parameters:
*           NONE
* Return Values:
*           TRANS_FAILD : 1
*           TRANS_SUCCESS  : 0
*
*  
*  Data:    2011-12-05
* 
+*****************************************************************************/
u_int32_t trans_rrh_parse_msg(void *p_info, 
                           size_t len,
                           void * p_rev_buf)
{
    struct rrh_monitor_header  *p_rrh_header = NULL;
    
    u_int8_t uc_type = 0;
    u_int32_t uw_ret = 0;
    u_int8_t uc_rep_flag = RRH_MONITOR_REP_FLAG_OK; 
    
    
    FLOG_DEBUG_TRANS(g_trans_debug_rrh, "Enter \r\n");
    
    if (NULL == p_rev_buf)
    {
        FLOG_ERROR("NULL PTR! \r\n");
        return TRANS_FAILD;  
    }
    
    FLOG_DEBUG_TRANS(g_trans_debug_rrh, "1 p_rev_buf = %p! \r\n", p_rev_buf);
    
    p_rrh_header = (struct rrh_monitor_header *)p_rev_buf;
    
    uc_type = p_rrh_header->uc_type;

    switch (uc_type)
    {
        /*Request message*/
        case RRH_MONITOR_TYPE_ALARM:
            
            uw_ret = trans_rrh_parse_alert(p_info, len, p_rev_buf);
            if (TRANS_SUCCESS != uw_ret)
            {
                FLOG_ERROR("Call trans_rrh_parse_alert error! uw_ret = %d\r\n", uw_ret);
                return TRANS_FAILD;                
            }

            break;
            
        case RRH_MONITOR_TYPE_HEARTBRAT:
           
            uw_ret = trans_rrh_parse_heartbeat(p_info, len, p_rev_buf);
            if (TRANS_SUCCESS != uw_ret)
            {
               FLOG_ERROR("Call trans_rrh_parse_heartbeat error! uw_ret = %d\r\n", uw_ret);
               return TRANS_FAILD;                
            }
                           
            break;
    
        /*Response message*/
        case RRH_MONITOR_TYPE_QUERY:

            uw_ret = trans_rrh_parse_query(p_info, len, p_rev_buf);
            if (TRANS_SUCCESS != uw_ret)
            {
                FLOG_ERROR("Call trans_rrh_parse_query error! uw_ret = %d\r\n", uw_ret);
                return TRANS_FAILD;                
            }
            
            break;
            
        case RRH_MONITOR_TYPE_CONFIG:

            uw_ret = trans_rrh_parse_config(p_info, len, p_rev_buf);
            if (TRANS_SUCCESS != uw_ret)
            {
                /*???????If  error , Do what?????*/  
                FLOG_ERROR("Call trans_rrh_parse_config error! uw_ret = %d\r\n", uw_ret);
                return TRANS_FAILD;                
            }

            break;

        default:
    
            FLOG_ERROR("Rev message type error! uc_type = %d\r\n", uc_type);
            return TRANS_FAILD;  
    
    }
    
    /*Check message header*/
    uw_ret = trans_rrh_check_msg_content(p_rev_buf, &uc_rep_flag);
    if (TRANS_SUCCESS != uw_ret) 
    {   
        /*If error, do what????????*/
        FLOG_ERROR("Call trans_rrh_check_msg_content error! uw_ret = %d\r\n", uw_ret);
        return TRANS_FAILD;  
    }
    
    void ** p_info_tmp = NULL;
    p_info_tmp = p_info;

    uw_ret = trans_transaction_set_result(*p_info_tmp, uc_rep_flag);
    if (TRANS_SUCCESS != uw_ret)
    {
        FLOG_ERROR("Call trans_transaction_set_result error! uw_ret = %d\r\n", uw_ret);
    
        return TRANS_FAILD;
    }

    FLOG_DEBUG_TRANS(g_trans_debug_rrh, "Exit.\r\n");
    
    return uw_ret;
}


/*****************************************************************************+
* Function: trans_rrh_msg_rev()
* Description: Revice Message From Socket
* Parameters:
*           NONE
* Return Values:
*           TRANS_FAILD : 1
*           TRANS_SUCCESS  : 0
*
*  
*  Data:    2011-04-23
* 
+*****************************************************************************/
u_int32_t trans_rrh_rev_msg(u_int8_t **pp_rev_msg, int32_t w_rrh_socket, int32_t *p_len)
{
    int32_t w_rev_len1 = 0, w_rev_len2 = 0;
    int32_t w_rev_len = 0;    
    int w_len_tmp = 0;
    //int32_t w_param_len = 0;
    int32_t w_msg_len = 0;

    u_int16_t us_len = 0;

    u_int32_t uw_ret = 0;
        
    /*First rev the len : 2 Bytes*/
    w_len_tmp = 2;
    
    while (w_len_tmp)
    {
        /*Then rev a totle message except the len*/
        w_rev_len1 = recv(w_rrh_socket, 
                                &us_len + (2 - w_len_tmp),  
                                w_len_tmp, 
                                0);
        /*Error*/
        if (w_rev_len1 <= 0)
        {
            #if 0
            FLOG_ERROR("Rev complete w_rev_len1 =%d. \r\n", w_rev_len1);
            return TRANS_FAILD;
            #endif

            TRANS_COMMON_SOCKFD_ERROR(w_rrh_socket, errno, uw_ret);
            
            TRANS_COMMON_SOCKFD_PRINT(w_rrh_socket, w_rev_len1, errno);
            
            FLOG_ERROR("Call recv for length error!\r\n");
            
            return uw_ret;

        }
    
        w_len_tmp = w_len_tmp - w_rev_len1;    
        w_rev_len = w_rev_len + w_rev_len1;
    }
    
    if (2 != w_rev_len)
    {
        FLOG_ERROR("Receive RRH Message Header Length error! header_len  = %d, rev_len  = %d\r\n", 2, w_rev_len);
        return TRANS_FAILD;
    }
  
    
    #if 0
    /*Then rev a totle message except the len*/
    w_rev_len1 = recv(w_rrh_socket, 
                            &us_len,  
                            w_len_tmp, 
                            0);
    /*Error*/
    if (w_rev_len1 <= 0)
    {
        FLOG_ERROR("Rev complete w_rev_len1 =%d. \r\n", w_rev_len1);
        return TRANS_FAILD;
    }
    #endif

    //FLOG_INFO("us_len = %d! \r\n", us_len);

    w_rev_len = 0;
    w_len_tmp = 0;
    
    /*-2 : len    ;    +2 :  CRC*/
    w_msg_len = SIZEOF_RRH_MONITOR_HEADER -2 + TRANS_NTOHS(us_len) + 2;

    //FLOG_INFO("1 w_msg_len = %d! \r\n", w_msg_len);
    
    *pp_rev_msg = (u_int8_t *)malloc (w_msg_len);
        
    if (NULL == *pp_rev_msg)
    {
        FLOG_ERROR("Malloc pp_rev_msg error! \r\n");
        return TRANS_FAILD;   
    }
    
    memset((u_int8_t*)*pp_rev_msg, 0, w_msg_len);
    memcpy((u_int8_t*)*pp_rev_msg, &us_len, 2);
   
    /*Then rev a totle message except the len*/
    w_len_tmp = w_msg_len;
    
    while (w_len_tmp)
    {
        /*Then rev a totle message except the len*/
        w_rev_len2 = recv(w_rrh_socket, 
                                *pp_rev_msg + (w_msg_len - w_len_tmp) + 2,  
                                w_len_tmp, 
                                0);
        /*Error*/
        if (w_rev_len2 <= 0)
        {
            #if 0
            FLOG_ERROR("Receivev RRH Message error! w_rev_len2 = %d\r\n", w_rev_len2);
            return TRANS_FAILD;
            #endif

            TRANS_COMMON_SOCKFD_ERROR(w_rrh_socket, errno, uw_ret);
            
            TRANS_COMMON_SOCKFD_PRINT(w_rrh_socket, w_rev_len2, errno);
            
            FLOG_ERROR("Call recv for message error!\r\n");
            
            return uw_ret;

        }
    
        w_len_tmp = w_len_tmp - w_rev_len2;    
        w_rev_len = w_rev_len + w_rev_len2;
    }
    
    if (w_msg_len != w_rev_len)
    {
        FLOG_ERROR("Receivev RRH Message Length error! msg_len  = %d, rev_len  = %d\r\n", w_msg_len, w_rev_len);
        return TRANS_FAILD;
    }
    
    //FLOG_INFO("2 w_rev_len = %d! \r\n", w_rev_len);    
    
    #if 0
    w_rev_len1 = recv(w_rrh_socket, 
                            p_rev_msg, 
                            2, 
                            0);
    
    if (w_rev_len1 <= 0)
    {
        FLOG_ERROR("Rev complete w_rev_len1 =%d. \r\n", w_rev_len1);
        return TRANS_FAILD;
    }
    
    /*Get the len*/
    w_param_len = TRANS_NTOHS(*((u_int16_t*)(p_rev_msg)));

    /*-2 : len    ;    +2 :  CRC*/
    w_msg_len = SIZEOF_RRH_MONITOR_HEADER -2 + w_param_len + 2;
    
    if ((w_msg_len + w_rev_len1) > TRANS_REV_MSG_MAX_LEN - 2)
    {
        FLOG_ERROR("Rev rrh msg length error! msg_len = %d, w_rev_len1 = %d\r\n", w_msg_len, w_rev_len1);
        return TRANS_FAILD;
    }
    
    /*Then rev a totle message except the len*/
    w_rev_len2 = recv(w_rrh_socket, 
                            p_rev_msg + 2,   /*len : 2 Bytes*/
                            w_msg_len, 
                            0);
    /*Error*/
    if (w_rev_len2 <= 0)
    {
        #if 0
        /*如果异常，该进入什么处理流程啊*/
        //close(sock);
    
    
        /*重复关闭有没有问题*/
        close (g_rrh_client_socket.w_sockFd);
        close (g_rrh_server_socket.w_sockFd);
        #endif
        FLOG_ERROR("Rev rrh msg error! w_rev_len2 = %d\r\n", w_rev_len2);
        return TRANS_FAILD;
    }
    #endif
    
    *p_len = w_msg_len + 2;   
    
    FLOG_DEBUG_TRANS(g_trans_debug_rrh, "Rev RRH Msg OK. rev_len = %d. \r\n", *p_len);

    FLOG_DEBUG_TRANS(g_trans_debug_rrh, "1 *pp_rev_msg = %p! \r\n", *pp_rev_msg);

    trans_debug_msg_print(*pp_rev_msg, *p_len, g_trans_debug_rrh);

    return TRANS_SUCCESS;
}


/*****************************************************************************+
* Function: trans_rrh_build_heard()
* Description: Build request message header to send to RRH
* Parameters:
*           NONE
* Return Values:
*           TRANS_FAILD : 1
*           TRANS_SUCCESS  : 0
*
*  
*  Data:    2011-03-23
* 
+*****************************************************************************/
u_int32_t trans_rrh_build_heard(u_int8_t uc_type, 
                                    u_int8_t *p_send_head, 
                                    u_int32_t uw_body_len, 
                                    u_int16_t *p_serial_number)
{
    struct rrh_monitor_header  *p_rrh_header = NULL;
    u_int16_t us_body_len = 0; 

    u_int16_t us_serial_number = 0;

    FLOG_DEBUG_TRANS(g_trans_debug_rrh, "Enter \r\n");
    
    if ((NULL == p_send_head) || (NULL == p_serial_number))
    {
        FLOG_ERROR("NULL PTR! \r\n");
        return TRANS_FAILD;      
    }  
    
    us_body_len = (u_int16_t)uw_body_len;

    p_rrh_header = (struct rrh_monitor_header *)p_send_head;
    
    p_rrh_header->us_body_len = TRANS_HTONS(us_body_len);
    p_rrh_header->uc_server_id = g_trans_rrh_eqp_config.uc_server_id;
    p_rrh_header->uw_rru_id = TRANS_HTONL(g_trans_rrh_eqp_config.uw_rru_id);

    p_rrh_header->uc_type = uc_type;
    p_rrh_header->uc_resp_flag = RRH_MONITOR_REP_FLAG_ORDER;

    /*Get Time; time is zero when Server send msg to RRH*/

    /*Get serial_number*/
    us_serial_number = trans_rrh_cal_serial_num(); 
    p_rrh_header->us_serial_number = TRANS_HTONS(us_serial_number);

    *p_serial_number = us_serial_number;
    
    FLOG_DEBUG_TRANS(g_trans_debug_rrh, "Exit \r\n"); 
    
    return TRANS_SUCCESS;
}


/*****************************************************************************+
* Function: trans_rrh_build_payload()
* Description: 
* Parameters:
*           NONE
* Return Values:
*           TRANS_FAILD : 1
*           TRANS_SUCCESS  : 0
*
*  Data:    2012-04-12
* 
+*****************************************************************************/
u_int32_t trans_rrh_build_payload(struct trans_rrh_build_payload_info *p_build_info, 
                    u_int32_t uw_payload_num,
                    void *p_payload, 
                    u_int32_t *p_payload_len)
{
    u_int32_t uw_ret = 0;
    u_int32_t uw_num = 0;

    u_int16_t us_param_type = 0;
    u_int8_t   uc_param_len = 0;

    u_int16_t *p_tag = NULL;
    u_int8_t    uc_value_len = 0;
    void   *p_value = NULL;

    u_int32_t uw_value32 = 0;
    u_int16_t us_value16 = 0;
    u_int8_t   uc_value8 = 0;
    
    u_int8_t *p_send_msg = NULL;
    struct rrh_monitor_param *p_param = NULL;

    u_int32_t uw_len_temp = 0;

    if ((NULL == p_build_info) ||(NULL == p_payload) || (NULL == p_payload_len))
    {
        FLOG_ERROR("NULL PTR! \r\n");
        return TRANS_FAILD;      
    }  
    
    FLOG_DEBUG_TRANS(g_trans_debug_rrh, "Enter \r\n"); 
    
    p_send_msg = (u_int8_t *)(p_payload);
    
    p_tag = (u_int16_t *)(p_build_info->p_tag);
    p_value = (p_build_info->p_value);

    for (uw_num = 0; uw_num < uw_payload_num; uw_num++)
    {
        us_param_type = *(p_tag);
        
        uw_ret = trans_rrh_type_get_len(us_param_type, &(uc_param_len));
        if (TRANS_SUCCESS != uw_ret)
        {
            FLOG_ERROR("Call trans_rrh_type_get_len error! uw_ret = %d\r\n", uw_ret);
            return TRANS_FAILD;  
        }
        
        p_param = (struct rrh_monitor_param *)(p_send_msg);
       
        p_param->us_param_type = TRANS_HTONS(us_param_type);
        p_param->uc_param_len = uc_param_len;

        if (NULL == p_value)
        {
            memset(p_send_msg + SIZEOF_RRH_MONITOR_PARAM, 0, uc_param_len);
        }
        else
        {
            
            if ((4 == uc_param_len)
                &&(RRH_MSG_SERVER_IP != us_param_type)
                &&(RRH_MSG_RRU_IPV4 != us_param_type)
                &&(RRH_MSG_FTP_ADDRESS != us_param_type))
            {
                uc_value_len = 4;
                
                uw_value32 = *((int *)(p_value));   
                
                FLOG_DEBUG_TRANS(g_trans_debug_rrh, "uw_value32 = %d \n", uw_value32);
                //FLOG_ERROR("uw_value32 = %d \n", uw_value32);
                
                uw_value32 = TRANS_HTONL(uw_value32);           
            
                memcpy(p_param + 1, &(uw_value32), uc_param_len); 
            
            }
            /*Param Length is UInt16*/
            else if (2 == uc_param_len)
            {
                uc_value_len = 4;
                
                //us_value16 = *((u_int16_t *)(p_value));   
                us_value16 = *((int *)(p_value));   
                
                FLOG_DEBUG_TRANS(g_trans_debug_rrh, "us_value16 = %d \n", us_value16);
                //FLOG_ERROR("us_value16 = %d \n", us_value16);

                us_value16 = TRANS_HTONS(us_value16);
                
                memcpy(p_param + 1, &(us_value16), uc_param_len); 
            
            }
            /*Param Length is UInt8*/
            else if (1 == uc_param_len)
            {
                uc_value_len = 4;
                
                //uc_value8 = *((u_int8_t *)(p_value));   
                uc_value8 = *((int *)(p_value));   
                
                FLOG_DEBUG_TRANS(g_trans_debug_rrh, "uc_value8 = %d \n", uc_value8);
                //FLOG_ERROR("uc_value8 = %d \n", uc_value8);

                memcpy(p_param + 1, &(uc_value8), uc_param_len); 
            
            }
            /*Param Length is string or struct*/
            else 
            {
                uc_value_len = uc_param_len;
                
                memcpy(p_param + 1, p_value, uc_param_len);
            
                FLOG_DEBUG_TRANS(g_trans_debug_rrh, " %s \n", p_param + 1);
                //FLOG_ERROR(" %s \n", p_param + 1);
            }   

            p_value = p_value + uc_value_len;
            
        }

        p_tag = p_tag + 1;

        p_send_msg = p_send_msg + (SIZEOF_RRH_MONITOR_PARAM + uc_param_len);

        uw_len_temp = uw_len_temp + (SIZEOF_RRH_MONITOR_PARAM + uc_param_len);

    }

    *p_payload_len = uw_len_temp;

    trans_debug_msg_print(p_payload, uw_len_temp, g_trans_debug_rrh);

    FLOG_DEBUG_TRANS(g_trans_debug_rrh, "Exit \r\n"); 
    
    return TRANS_SUCCESS;

}

/*****************************************************************************+
* Function: trans_rrh_build_msg()
* Description: 
* Parameters:
*           NONE
* Return Values:
*           TRANS_FAILD : 1
*           TRANS_SUCCESS  : 0
*
*  
*  Data:    2011-03-23
* 
+*****************************************************************************/
u_int32_t trans_rrh_build_msg(struct trans_rrh_send_msg_info *p_rrh_send_info,
                                    u_int8_t *p_send_msg, 
                                    u_int32_t *p_msg_len, 
                                    u_int16_t *p_serial_number)
{
    u_int8_t    uc_type = 0;
    u_int32_t  uw_ret = 0;

    u_int32_t  uw_body_len = 0;

    u_int8_t * p_msg_body = NULL;    
    u_int16_t   *p_new_crc = NULL;
    u_int16_t   us_crc_val = 0;
    
    FLOG_DEBUG_TRANS(g_trans_debug_rrh, "Enter \r\n");

    if ((NULL == p_send_msg) || (NULL == p_msg_len) || (NULL == p_serial_number))
    {
        FLOG_ERROR("NULL PTR! \r\n");
        return TRANS_FAILD;      
    }    
    
    uc_type = p_rrh_send_info->uc_type;
    uw_body_len = p_rrh_send_info->uw_payload_len;

    /*Build Msg head */
    uw_ret = trans_rrh_build_heard(uc_type, p_send_msg, uw_body_len, p_serial_number);
    /*Error*/
    if (TRANS_SUCCESS != uw_ret)
    {
        FLOG_ERROR("Call trans_rrh_build_heard error!uw_ret= %d \r\n", uw_ret);
        return TRANS_FAILD;
    }  

    if (0 != uw_body_len)
    {
        p_msg_body = p_send_msg + SIZEOF_RRH_MONITOR_HEADER;
        
        memcpy(p_msg_body, p_rrh_send_info->p_payload, uw_body_len);
    }
        

    /*Build Msg CRC */    /*calculate CRC*/
    us_crc_val = trans_rrh_cal_crc16((u_int8_t*)p_send_msg, uw_body_len + SIZEOF_RRH_MONITOR_HEADER); 
    
    p_new_crc = (u_int16_t  *)(p_send_msg + uw_body_len + SIZEOF_RRH_MONITOR_HEADER);
    *p_new_crc = TRANS_HTONS(us_crc_val);

    //FLOG_DEBUG_TRANS(g_trans_debug_rrh, "*p_new_crc = %d \r\n", *p_new_crc);

    /* 2 for the Bytes length of CRC*/
    *p_msg_len = uw_body_len + SIZEOF_RRH_MONITOR_HEADER + 2;

    FLOG_DEBUG_TRANS(g_trans_debug_rrh, "Exit \r\n"); 

    return TRANS_SUCCESS;  

}

/*****************************************************************************+
* Function: trans_rrh_send()
* Description: 
* Parameters:
*           NONE
* Return Values:
*           TRANS_FAILD : 1
*           TRANS_SUCCESS  : 0
*
*  Data:    2012-05-04
* 
+*****************************************************************************/
u_int32_t trans_rrh_send(void * p_send_buf,
                                u_int32_t uw_send_len)
{
    int32_t w_ret = 0;
    int32_t     w_send_len_temp = 0;
    int32_t w_total_send_len = 0;

    u_int32_t uw_congestion_num = 0;
    
    time_t   now;
    struct tm   *timenow;
    
    /*Send New Message to Monitor*/
    while (w_total_send_len < ((int)uw_send_len))
    {
        if (uw_send_len - w_total_send_len > 2*1024)
        {
            w_send_len_temp = 2*1024 ;
        }
        else
        {
            w_send_len_temp = uw_send_len - w_total_send_len;
        }

        uw_congestion_num = trans_device_get_congestion_num(
                g_trans_rrh_eqp_config.a_rrh_mac_addr, 0); 
        
        /*Not Congestion*/
        if (0 == uw_congestion_num)
        {
            /*Send New Message to Monitor*/
            w_ret = send(g_trans_rrh_socket, (p_send_buf + w_total_send_len), w_send_len_temp, 0);
            
            if(w_ret <= 0)
            {
                //close(w_sockfd);
                if (EAGAIN == errno)
                {
                    FLOG_ERROR("Congestion start! \r\n");
                
                    uw_congestion_num = trans_device_get_congestion_num(
                            g_trans_rrh_eqp_config.a_rrh_mac_addr, 1); 
                }  

                TRANS_COMMON_SOCKFD_PRINT(g_trans_rrh_socket, w_ret, errno);

                return TRANS_FAILD;
            }  
    
            if (w_ret != w_send_len_temp)
            {
                time(&now);
                timenow = localtime(&now);
                
                FLOG_ERROR("send length error! %d, %d. Time : %s.\r\n", w_ret, w_send_len_temp, asctime(timenow));
                
                w_send_len_temp = w_ret;
            }
    
            w_total_send_len = w_total_send_len + w_send_len_temp;
            
        }
        /*Congestion-----discard or disconnect?*/
        else
        {
            FLOG_ERROR("Congestion : discard the message! \r\n");
            
            return TRANS_FAILD;
        }

    }
    
    return TRANS_SUCCESS;
}


/*****************************************************************************+
* Function: trans_rrh_send_msg_process()
* Description: 
* Parameters:
*           NONE
* Return Values:
*           TRANS_FAILD : 1
*           TRANS_SUCCESS  : 0
*
*  
*  Data:    2011-03-23
* 
+*****************************************************************************/
u_int32_t trans_rrh_send_msg_process(struct trans_rrh_send_msg_info *p_rrh_send_info)
{

    struct trans_rrh_send_msg_info *p_send_info = NULL;

    u_int8_t    uc_type = 0;
    u_int8_t  *p_send_buf = NULL;
    u_int32_t uw_ret = 0;
    u_int32_t uw_send_len = 0;
    u_int16_t   us_serial_number = 0;  
    
    struct trans_timer_info st_timer_info;    
    void* p_timer = NULL;

    //int32_t w_ret = 0;
    
    FLOG_DEBUG_TRANS(g_trans_debug_rrh, "Enter \r\n"); 

    if (NULL == p_rrh_send_info)
    {
        FLOG_ERROR("NULL PTR! \r\n");
        return TRANS_FAILD;      
    }  

    p_send_info = (struct trans_rrh_send_msg_info *)p_rrh_send_info;   

    if (RRH_MSG_MAX_LEN < p_send_info->uw_payload_len)
    {
        FLOG_ERROR("Length error! \r\n");
        return TRANS_FAILD;      
    }

    uc_type = p_send_info->uc_type;
    
    if (uc_type >= RRH_MONITOR_TYPE_BUF)
    {
        FLOG_ERROR("Type error! \r\n");
        return TRANS_FAILD; 
    }
   
    /* Allocate a memory.  */
    uw_send_len = (p_send_info->uw_payload_len) + SIZEOF_RRH_MONITOR_HEADER + 2;
    
    p_send_buf = (u_int8_t *)malloc(uw_send_len);
    if (NULL == p_send_buf)
    {
        FLOG_ERROR("malloc p_send_buf error! \r\n");
        return TRANS_FAILD;   
    }  

    memset((u_int8_t*)p_send_buf, 0, uw_send_len);

    /*Build Msg*/
    uw_ret =  trans_rrh_build_msg(p_send_info, p_send_buf, &uw_send_len, &us_serial_number);
    if (TRANS_SUCCESS != uw_ret) 
    {   
        free(p_send_buf);

        FLOG_ERROR("Call trans_rrh_build_msg error! uw_ret = %d\r\n", uw_ret);
        return TRANS_FAILD;     
    }   

    trans_debug_msg_print(p_send_buf, uw_send_len + 4, g_trans_debug_rrh);
    
    /*Send New Msg to RRH*/
    #if 0
    w_ret = send(p_send_info->w_rrh_sockfd, p_send_buf, uw_send_len, 0);
    
    if(w_ret <= 0)
    {
        free(p_send_buf);
        //close(sock);
        FLOG_ERROR("send error! w_ret = %d\r\n", w_ret);
        return TRANS_FAILD;
    }   
    #endif
    
    uw_ret = trans_rrh_send(p_send_buf, uw_send_len);
    if (TRANS_SUCCESS != uw_ret) 
    {   
        free(p_send_buf);
    
        FLOG_ERROR("Call trans_rrh_send error! uw_ret = %d\r\n", uw_ret);
        return TRANS_FAILD;     
    }  

    free(p_send_buf);
    p_send_buf = NULL;
   
    if ((RRH_MONITOR_TYPE_HEARTBRAT != uc_type)
        && (NULL != p_send_info->p_info))
    {
        uw_ret = trans_transaction_set_dst(p_send_info->p_info, 
                                    us_serial_number,
                                    p_send_info->w_rrh_sockfd,
                                    g_trans_rrh_eqp_config.a_rrh_mac_addr);
        if (TRANS_SUCCESS != uw_ret)
        {
            FLOG_ERROR("Call trans_transaction_set_dst error! uw_ret = %d\r\n", uw_ret);
        
            return TRANS_FAILD;
        }

        /*For record NO Delete*/
        uw_ret = trans_transaction_set_comn(p_send_info->p_info, 
                                    TRANS_TRANSACTION_FLAG_NO_DELETE,
                                    TRANS_MOUDLE_RRH);
        if (TRANS_SUCCESS != uw_ret)
        {
            FLOG_ERROR("Call trans_transaction_set_comn error! uw_ret = %d\r\n", uw_ret);
            return TRANS_FAILD;
        }

        /*For record Function Callback*/
        /*p_send_buf now is NULL , It is useless, can not free if need*/
        uw_ret = trans_transaction_set_rrh(p_send_info->p_info, 
                                                p_send_info->f_rrh_callback, 
                                                uw_send_len, 
                                                p_send_buf);
        if (TRANS_SUCCESS != uw_ret)
        {
            FLOG_ERROR("Call trans_transaction_set_rrh error! uw_ret = %d\r\n", uw_ret);
            return TRANS_FAILD;
        }

        st_timer_info.f_callback = p_send_info->f_rrh_callback;
        st_timer_info.p_data = p_send_info->p_info;
        st_timer_info.p_timer_list = &g_trans_timer_list;
        st_timer_info.uc_type = TRANS_TIMER_TYPE_ONCE;
        st_timer_info.uw_interval = TRANS_SEND_RRH_MSG_TIMEOUT;
        
        uw_ret = trans_timer_add(&st_timer_info, &p_timer);    
        if (TRANS_SUCCESS != uw_ret)
        {
            FLOG_ERROR("Call trans_timer_add error! uw_ret = %d\r\n", uw_ret);
            return TRANS_FAILD;
        }

        uw_ret = trans_transaction_set_timer(p_send_info->p_info, p_timer);
        if (TRANS_SUCCESS != uw_ret)
        {
            FLOG_ERROR("Call trans_transaction_set_timer error! uw_ret = %d\r\n", uw_ret);
            return TRANS_FAILD;
        }        
    }

    FLOG_DEBUG_TRANS(g_trans_debug_rrh, "Exit \r\n");   
    
    return TRANS_SUCCESS;    
}

/*****************************************************************************+
* Function: trans_rrh_heartbeat_flag()
* Description: 
* Parameters:
*           NONE
* Return Values:
*           NONE
*
*  
*  Data:    2012-07-19
* 
+*****************************************************************************/
void trans_rrh_heartbeat_flag()
{
    static u_int8_t  uc_send_count = 0;
    static u_int8_t  uc_stop_count = 0;

    if ((0 != g_trans_rrh_heartbeat.uc_send_num)
        && (0 != g_trans_rrh_heartbeat.uc_stop_num))
    {
        if (1 == g_trans_rrh_heartbeat.uc_send_flag)
        {
            if (uc_send_count < g_trans_rrh_heartbeat.uc_send_num)
            {
                uc_send_count++;
            }
            else
            {
                uc_send_count = 0;                
                g_trans_rrh_heartbeat.uc_send_flag = 0;

                uc_stop_count++;
            }
        }
        else
        {
            if (uc_stop_count < g_trans_rrh_heartbeat.uc_stop_num)
            {
                uc_stop_count++;
            }
            else
            {
                uc_stop_count = 0;                
                g_trans_rrh_heartbeat.uc_send_flag = 1;

                uc_send_count++;
            }
        }
    }

    return;  
}

/*****************************************************************************+
* Function: trans_rrh_heartbeat_timer_func()
* Description: 
* Parameters:
*           NONE
* Return Values:
*           TRANS_FAILD : 1
*           TRANS_SUCCESS  : 0
*
*  
*  Data:    2011-03-23
* 
+*****************************************************************************/
int trans_rrh_heartbeat_timer_func(void *p_msg, size_t len, void *p_msg_info)
{
    u_int32_t uw_ret = 0;
    struct trans_rrh_send_msg_info st_send_info;
    //struct trans_timer_msg_info *p_src_msg_info = NULL;
   
    FLOG_DEBUG_TRANS(g_trans_debug_rrh, "Enter\r\n"); 
    
    (void) p_msg;
    (void) len;
    (void) p_msg_info;

    /*Not revice HeartBeat Msg for 3 times->report to C&M*/
    if (g_trans_rrh_heartbeat.uc_count >= RRH_HEARTBEAT_TIMEOUT_NUM)
    {
        /*report to C&M----future*/
        //close (g_rrh_client_socket.w_sockFd);
        FLOG_ERROR("9 times! g_trans_rrh_heartbeat_num = %d\r\n", g_trans_rrh_heartbeat.uc_count);
        
        return TRANS_SUCCESS;
    }

    trans_rrh_heartbeat_flag();

    if (1 != g_trans_rrh_heartbeat.uc_send_flag)
    {
        return TRANS_SUCCESS;
    }
    
    memset((u_int8_t*)&st_send_info, 0, sizeof(struct trans_rrh_send_msg_info));

    /*Send Heartbeat Msg Again  per 3 second*/
    st_send_info.w_rrh_sockfd = g_trans_rrh_socket;
    
    st_send_info.uw_src_moudle = TRANS_MOUDLE_LOCAL;
    st_send_info.uc_type = RRH_MONITOR_TYPE_HEARTBRAT;
    
    st_send_info.f_rrh_callback = trans_rrh_heartbeat_timer_func;
    st_send_info.p_info = NULL;
    st_send_info.p_payload = NULL;
    st_send_info.uw_payload_len = 0;

    //FLOG_DEBUG_TRANS(g_trans_debug_rrh, "st_send_info.st_build_info.uw_param_num = %d\r\n", st_send_info.st_build_info.uw_param_num);
    
    uw_ret = trans_rrh_send_msg_process(&st_send_info);
    if (TRANS_SUCCESS != uw_ret) 
    {   
     
        FLOG_ERROR("Call trans_rrh_send_msg_process error! uw_ret = %d\r\n", uw_ret);
        return TRANS_FAILD;     
    }    

    g_trans_rrh_heartbeat.uc_count++;

    FLOG_INFO("Send Heartbeat to RRH\r\n"); 
    
    FLOG_DEBUG_TRANS(g_trans_debug_rrh, "Exit\r\n");    

    return TRANS_SUCCESS;
}

/*****************************************************************************+
* Function: trans_rrh_heartbeat_timer()
* Description: Send HeartBeat message to RRH and start a timer
* Parameters:
*           NONE
* Return Values:
*           TRANS_FAILD : 1
*           TRANS_SUCCESS  : 0
*
*  
*  Data:    2011-06-30
* 
+*****************************************************************************/
u_int32_t trans_rrh_heartbeat_timer(void)
{
    u_int32_t uw_ret = 0;
    struct trans_timer_info st_timer_info;
    
    void* p_timer = NULL;

    trans_rrh_heartbeat_timer_func(NULL, 0, NULL);
    
    /*ADD TIMER*/
    /*Send Heartbeat Msg Again  per 3 second*/
   
    st_timer_info.f_callback = trans_rrh_heartbeat_timer_func;
    st_timer_info.p_data = NULL;
    st_timer_info.p_timer_list = &g_trans_timer_list;
    st_timer_info.uc_type = TRANS_TIMER_TYPE_CIRCLE;
    st_timer_info.uw_interval = g_trans_rrh_heartbeat.uc_duration;
    
    uw_ret = trans_timer_add(&st_timer_info, &p_timer);    
    if (TRANS_SUCCESS != uw_ret)
    {
        FLOG_ERROR("Call trans_timer_add error! uw_ret = %d\r\n", uw_ret);
        return TRANS_FAILD;
    }

    if (NULL != p_timer)
    {
        g_trans_rrh_heartbeat.p_timer = p_timer;
        g_trans_rrh_hb_timer = p_timer;
    }
    else
    {
        FLOG_ERROR("NULL PTR p_timer \r\n");
        return TRANS_FAILD;
    }

    return TRANS_SUCCESS;
}


/*****************************************************************************+
* Function: trans_rrh_send_monitor()
* Description: 
* Parameters:
*           NONE
* Return Values:
*           TRANS_FAILD : 1
*           TRANS_SUCCESS  : 0
*
*  Data:    2012-04-18
* 
+*****************************************************************************/
u_int32_t trans_rrh_send_monitor(
                            struct trans_send_msg_to_rrh *p_rrh,
                            size_t len,
                            u_int8_t * p_send_buf)
{
    u_int32_t uw_ret = 0;
    struct trans_rrh_send_msg_info st_send_info;

    (void) len;
    (void) p_send_buf;
    
    if (NULL == p_rrh)
    {
        FLOG_ERROR("NULL PTR! \r\n");
        return TRANS_FAILD;      
    }  
    
    FLOG_DEBUG_TRANS(g_trans_debug_rrh, "Enter! \r\n");    
        
    memset((u_int8_t*)&st_send_info, 0, sizeof(struct trans_rrh_send_msg_info));
   
    st_send_info.w_rrh_sockfd = g_trans_rrh_socket;
    
    st_send_info.uw_src_moudle = TRANS_MOUDLE_MONITOR;
    st_send_info.uc_type = p_rrh->uc_type;

    st_send_info.f_rrh_callback = p_rrh->f_callback;
    st_send_info.p_info = p_rrh->p_info;
    st_send_info.p_payload = p_rrh->p_payload;
    st_send_info.uw_payload_len = p_rrh->uw_payload_len;
        
    uw_ret = trans_rrh_send_msg_process(&st_send_info);
    if (TRANS_SUCCESS != uw_ret) 
    {   
        FLOG_ERROR("Call trans_rrh_send_msg_process error! uw_ret = %d\r\n", uw_ret);

        return TRANS_FAILD;     
    }   

    FLOG_DEBUG_TRANS(g_trans_debug_rrh, "Exit! \r\n");   
    
    return TRANS_SUCCESS;

}

/*****************************************************************************+
* Function: trans_rrh_send_action()
* Description: 
* Parameters:
*           NONE
* Return Values:
*           TRANS_FAILD : 1
*           TRANS_SUCCESS  : 0
*
*  Data:    2012-04-18
* 
+*****************************************************************************/
u_int32_t trans_rrh_send_action(
                            struct trans_send_msg_to_rrh *p_rrh,
                            size_t len,
                            u_int8_t * p_send_buf)
{
    u_int32_t uw_ret = 0;
    struct trans_rrh_send_msg_info st_send_info;

    (void) len;
    (void) p_send_buf;
    
    if (NULL == p_rrh)
    {
        FLOG_ERROR("NULL PTR! \r\n");
        return TRANS_FAILD;      
    }  
    
    FLOG_DEBUG_TRANS(g_trans_debug_rrh, "Enter! \r\n");    
        
    memset((u_int8_t*)&st_send_info, 0, sizeof(struct trans_rrh_send_msg_info));
   
    st_send_info.w_rrh_sockfd = g_trans_rrh_socket;
    
    st_send_info.uw_src_moudle = TRANS_MOUDLE_ACTION;
    st_send_info.uc_type = p_rrh->uc_type;

    st_send_info.f_rrh_callback = p_rrh->f_callback;
    st_send_info.p_info = p_rrh->p_info;
    st_send_info.p_payload = p_rrh->p_payload;
    st_send_info.uw_payload_len = p_rrh->uw_payload_len;
        
    uw_ret = trans_rrh_send_msg_process(&st_send_info);
    if (TRANS_SUCCESS != uw_ret) 
    {   
        FLOG_ERROR("Call trans_rrh_send_msg_process error! uw_ret = %d\r\n", uw_ret);

        return TRANS_FAILD;     
    }   

    FLOG_DEBUG_TRANS(g_trans_debug_rrh, "Exit! \r\n");   
    
    return TRANS_SUCCESS;

}

/*****************************************************************************+
* Function: trans_rrh_send_agent()
* Description: 
* Parameters:
*           NONE
* Return Values:
*           TRANS_FAILD : 1
*           TRANS_SUCCESS  : 0
*
*  Data:    2012-04-18
* 
+*****************************************************************************/
u_int32_t trans_rrh_send_agent(
                            struct trans_send_msg_to_rrh *p_rrh,
                            size_t len,
                            u_int8_t * p_send_buf)
{
    u_int32_t uw_ret = 0;
    struct trans_rrh_send_msg_info st_send_info;

    (void) len;
    (void) p_send_buf;
    
    if (NULL == p_rrh)
    {
        FLOG_ERROR("NULL PTR! \r\n");
        return TRANS_FAILD;      
    }  
    
    FLOG_DEBUG_TRANS(g_trans_debug_rrh, "Enter! \r\n");    
        
    memset((u_int8_t*)&st_send_info, 0, sizeof(struct trans_rrh_send_msg_info));
   
    st_send_info.w_rrh_sockfd = g_trans_rrh_socket;
    
    st_send_info.uw_src_moudle = TRANS_MOUDLE_AGENT;
    st_send_info.uc_type = p_rrh->uc_type;

    st_send_info.f_rrh_callback = p_rrh->f_callback;
    st_send_info.p_info = p_rrh->p_info;
    st_send_info.p_payload = p_rrh->p_payload;
    st_send_info.uw_payload_len = p_rrh->uw_payload_len;
        
    uw_ret = trans_rrh_send_msg_process(&st_send_info);
    if (TRANS_SUCCESS != uw_ret) 
    {   
        FLOG_ERROR("Call trans_rrh_send_msg_process error! uw_ret = %d\r\n", uw_ret);

        return TRANS_FAILD;     
    }   

    FLOG_DEBUG_TRANS(g_trans_debug_rrh, "Exit! \r\n");   
    
    return TRANS_SUCCESS;

}

/*****************************************************************************+
* Function: trans_rrh_send_agent()
* Description: 
* Parameters:
*           NONE
* Return Values:
*           TRANS_FAILD : 1
*           TRANS_SUCCESS  : 0
*
*  Data:    2012-04-18
* 
+*****************************************************************************/
u_int32_t trans_rrh_send_bs(
                            struct trans_send_msg_to_rrh *p_rrh,
                            size_t len,
                            u_int8_t * p_send_buf)
{
    u_int32_t uw_ret = 0;
    struct trans_rrh_send_msg_info st_send_info;

    (void) len;
    (void) p_send_buf;
    
    if (NULL == p_rrh)
    {
        FLOG_ERROR("NULL PTR! \r\n");
        return TRANS_FAILD;      
    }  
    
    FLOG_DEBUG_TRANS(g_trans_debug_rrh, "Enter! \r\n");    
        
    memset((u_int8_t*)&st_send_info, 0, sizeof(struct trans_rrh_send_msg_info));
   
    st_send_info.w_rrh_sockfd = g_trans_rrh_socket;
    
    st_send_info.uw_src_moudle = TRANS_MOUDLE_BS;
    st_send_info.uc_type = p_rrh->uc_type;

    st_send_info.f_rrh_callback = p_rrh->f_callback;
    st_send_info.p_info = p_rrh->p_info;
    st_send_info.p_payload = p_rrh->p_payload;
    st_send_info.uw_payload_len = p_rrh->uw_payload_len;
        
    uw_ret = trans_rrh_send_msg_process(&st_send_info);
    if (TRANS_SUCCESS != uw_ret) 
    {   
        FLOG_ERROR("Call trans_rrh_send_msg_process error! uw_ret = %d\r\n", uw_ret);

        return TRANS_FAILD;     
    }   

    FLOG_DEBUG_TRANS(g_trans_debug_rrh, "Exit! \r\n");   
    
    return TRANS_SUCCESS;

}

#ifdef TRANS_RRH_NEW_CONNECT
/*****************************************************************************+
* Function: trans_wireless_send_rru_id()
* Description: 
* Parameters:
*           NONE
* Return Values:
*           ADAPT_FAILD : 1
*           ADAPT_SUCCESS  : 0
*
*  Data:    2012-06-26
* 
+*****************************************************************************/
u_int32_t trans_wireless_send_rru_id(int32_t w_rrh_socket )
{
    u_int32_t uw_ret = 0;
    int32_t w_ret = 0;
    
    u_int8_t *  p_rrh_payload = NULL;
    struct trans_rrh_build_payload_info st_build_rrh;
    u_int8_t   uc_num = 0;
    u_int32_t uw_rrh_len = 0;
    u_int16_t  a_param_type[1] = {0};

    struct trans_rrh_send_msg_info st_send_info;

    u_int8_t  *p_send_buf = NULL;
    u_int32_t uw_send_len = 0;
    u_int16_t   us_serial_number = 0;  
   
    a_param_type[0] = RRH_MSG_RRU_NO;
    
    st_build_rrh.p_tag = a_param_type;
    st_build_rrh.p_value = NULL;

    uc_num = 1;
    
    /* Allocate a memory.  */
    p_rrh_payload = (u_int8_t *)malloc(RRH_MSG_PAYLOAD_MAX_LEN);
    if (NULL == p_rrh_payload)
    {
        FLOG_ERROR("malloc p_rrh_payload error! \r\n");
        return TRANS_FAILD;   
    }
    
    uw_ret = trans_rrh_build_payload(&st_build_rrh, 
                        uc_num,
                        p_rrh_payload, 
                        &uw_rrh_len);
    /*Error*/
    if (TRANS_SUCCESS != uw_ret)
    {
        FLOG_ERROR("Call trans_rrh_build_payload error! uw_ret = %d\r\n", uw_ret);
    
        free(p_rrh_payload);
        return TRANS_FAILD;   
    }
    
    memset((u_int8_t*)&st_send_info, 0, sizeof(struct trans_rrh_send_msg_info));
    
    st_send_info.w_rrh_sockfd = g_trans_rrh_socket;
    
    st_send_info.uw_src_moudle = TRANS_MOUDLE_BS;
    st_send_info.uc_type = RRH_MONITOR_TYPE_QUERY;
    
    st_send_info.f_rrh_callback = NULL;
    st_send_info.p_info = NULL;
    st_send_info.p_payload = p_rrh_payload;
    st_send_info.uw_payload_len = uw_rrh_len;
        
    #if 0
    uw_ret = trans_rrh_send_msg_process(&st_send_info);
    if (TRANS_SUCCESS != uw_ret) 
    {   
        FLOG_ERROR("Call trans_rrh_send_msg_process error! uw_ret = %d\r\n", uw_ret);
    
        //return TRANS_FAILD;     
    }   
    #endif

    uw_send_len = (uw_rrh_len) + SIZEOF_RRH_MONITOR_HEADER + 2;
    
    p_send_buf = (u_int8_t *)malloc(uw_send_len);
    if (NULL == p_send_buf)
    {
        FLOG_ERROR("malloc p_send_buf error! \r\n");
        free(p_rrh_payload);
        
        return TRANS_FAILD;   
    }  
    
    memset((u_int8_t*)p_send_buf, 0, uw_send_len);

    /*Build Msg*/
    uw_ret =  trans_rrh_build_msg(&st_send_info, p_send_buf, &uw_send_len, &us_serial_number);
    if (TRANS_SUCCESS != uw_ret) 
    {   
        free(p_send_buf);
        free(p_rrh_payload);
        
        FLOG_ERROR("Call trans_rrh_build_msg error! uw_ret = %d\r\n", uw_ret);
        return TRANS_FAILD;     
    }   
    
    trans_debug_msg_print(p_send_buf, uw_send_len + 4, 1);

    w_ret = send(w_rrh_socket, p_send_buf, uw_send_len, 0);
    
    if(w_ret <= 0)
    {
        free(p_rrh_payload);
        free(p_send_buf);
        //close(sock);
        FLOG_ERROR("send error! w_ret = %d\r\n", w_ret);
        return TRANS_FAILD;
    }     
    
    
    if (NULL != p_rrh_payload)
    {
        free(p_rrh_payload);
    }

    if (NULL != p_send_buf)
    {
        free(p_send_buf);
    }

    return uw_ret;
    
}


/*****************************************************************************+
* Function: trans_rrh_tcp_connect_msg()
* Description: 
* Parameters:
*           NONE
* Return Values:
*           ADAPT_FAILD : 1
*           ADAPT_SUCCESS  : 0
*
*  Data:    2012-04-20
* 
+*****************************************************************************/
u_int32_t trans_rrh_tcp_connect(int32_t w_rrh_socket)
{
    int32_t w_ret = 0;
    u_int32_t uw_ret = 0;

    u_int8_t uc_rep_flag = RRH_MONITOR_REP_FLAG_OK; 
    
    u_int8_t * p_rev_buf = NULL;
    u_int32_t   uw_buf_len = 512;
    int32_t   w_rev_len = 0;

    u_int32_t uw_rru_id = 0xffffffff;

    struct rrh_monitor_param *p_param = NULL;

    fd_set readfds;
    struct timeval st_time_val;

    FLOG_DEBUG_TRANS(g_trans_debug_rrh, "Enter \r\n");

    /**Send message for querying RRUID */
    //uw_ret = trans_wireless_send2_rrh_q_rru_id(NULL, 1, NULL);
    uw_ret = trans_wireless_send_rru_id(w_rrh_socket);
    if (TRANS_SUCCESS != uw_ret) 
    {   
        FLOG_ERROR("Call trans_wireless_send_rru_id error! uw_ret = %d\r\n", uw_ret);
        return TRANS_FAILD;  
    }

    st_time_val.tv_sec = TRANS_RRH_GET_RRUID_TIMEOUT;   
    st_time_val.tv_usec = 0;

    FD_ZERO(&readfds);
    FD_SET(w_rrh_socket, &readfds);

    w_ret = select(w_rrh_socket + 1, &readfds, NULL, NULL, &st_time_val);

    if(w_ret <0)
    {
        /*If error, do what ?????*/
        FLOG_ERROR("select error! w_ret = %d\r\n", w_ret);
        //close(w_socket);
        return TRANS_FAILD;
    }
    else if (0 == w_ret)
    {
        FLOG_ERROR("Timeout! w_ret = %d\r\n", w_ret);
        FLOG_ERROR("Get RRU ID from RRU timeout\r\n");
        return TRANS_FAILD;
    }
    
    p_rev_buf = (u_int8_t *)malloc (uw_buf_len);
        
    if (NULL == p_rev_buf)
    {
        FLOG_ERROR("Malloc p_rev_buf error! \r\n");
        return TRANS_FAILD;   
    }

    if((FD_ISSET(w_rrh_socket, &readfds)))
    {
        w_rev_len = recv(w_rrh_socket, 
                                p_rev_buf,  
                                uw_buf_len, 
                                0);
        /*Error*/
        if (w_rev_len <= 0)
        {
            FLOG_ERROR("Rev complete w_rev_len1 =%d. \r\n", w_rev_len);
            return TRANS_FAILD;
        }        
    }

    /*Check message header*/
    uw_ret = trans_rrh_check_msg_content(p_rev_buf, &uc_rep_flag);
    if (TRANS_SUCCESS != uw_ret) 
    {   
        /*If error, do what????????*/
        FLOG_ERROR("Call trans_rrh_check_msg_content error! uw_ret = %d\r\n", uw_ret);
        return TRANS_FAILD;  
    }

    /*Get RRU ID*/
    p_param = (struct rrh_monitor_param *)(p_rev_buf + SIZEOF_RRH_MONITOR_HEADER);

    if (RRH_MSG_RRU_NO != TRANS_NTOHS(p_param->us_param_type))
    {
        FLOG_ERROR("Rev message type error type =0x%x. \r\n", TRANS_NTOHS(p_param->us_param_type));
        return TRANS_FAILD;  
    }

    uw_rru_id = *((u_int32_t *)(p_param + 1));
    g_trans_rrh_eqp_config.uw_rru_id = TRANS_NTOHL(uw_rru_id);

    #if (defined TRANS_BS_COMPILE) || (defined TRANS_UI_COMPILE)
    uw_ret = set_global_param("RRU_ID", (void *)&(uw_rru_id));
    if (0 != uw_ret)
    {
        FLOG_ERROR("Call set_global_param for RRU_ID");
    }
    #endif

    
    FLOG_DEBUG_TRANS(g_trans_debug_rrh, "Exit RRU_ID = %d.\r\n", g_trans_rrh_eqp_config.uw_rru_id);

    return TRANS_SUCCESS;
    
}


/*****************************************************************************+
* Function: trans_rrh_tcp_socket()
* Description: 
* Parameters:
*           NONE
* Return Values:
*           ADAPT_FAILD : 1
*           ADAPT_SUCCESS  : 0
*
*  Data:    2012-04-20
* 
+*****************************************************************************/
u_int32_t trans_rrh_tcp_socket()
{
    int32_t w_rrh_socket = 1;

    int32_t w_ret = 0;
    struct sockaddr_in st_peer_addr; 
    socklen_t sin_size = sizeof(struct sockaddr_in);    
    int32_t w_sendbuflen = 0, w_recvbuflen = 0, w_reuseORnot = 0;
    
    struct trans_device_info st_device_info;
    void * p_device = NULL;

    u_int8_t  uc_mac_flag = TRANS_USE_RRH_DEFAULT_MAC;
    
    struct timeval st_time_val;
    
    FLOG_DEBUG_TRANS(g_trans_debug_rrh, "Enter \r\n");
   
    /* Create an IPv4 Internet Socket */
    w_ret = w_rrh_socket = socket (AF_INET, SOCK_STREAM, 0);
    
    if (w_ret < 0)
    {    
        FLOG_ERROR("Creat socket error! w_ret = %d\r\n", w_ret);
        return TRANS_FAILD;    
    }
    
    /*Set if reuse the adderss*/
    w_reuseORnot = 1;
    w_ret = setsockopt (w_rrh_socket, SOL_SOCKET, SO_REUSEADDR, &w_reuseORnot,
            sizeof(int32_t));
    
    if (w_ret < 0)
    {
        FLOG_ERROR("Call setsockopt SO_REUSEADDR error! w_ret = %d\r\n", w_ret);

        close (w_rrh_socket);
        return TRANS_FAILD;
     }
    
    
    /*Set the length of the REV buffer*/
    w_recvbuflen = RRH_REV_BUF_MAX_LEN;
    w_ret = setsockopt (w_rrh_socket, SOL_SOCKET, SO_RCVBUF, &w_recvbuflen, sizeof(int32_t));
    
    if (w_ret < 0)
    {
        FLOG_ERROR("Call setsockopt SO_RCVBUF error! w_ret = %d\r\n", w_ret);

        close (w_rrh_socket);
        return TRANS_FAILD;
    }
    
    /*Set the length of the revice buffer*/
    w_sendbuflen = RRH_SEND_BUF_MAX_LEN;
    w_ret = setsockopt (w_rrh_socket, SOL_SOCKET, SO_SNDBUF, &w_sendbuflen, sizeof(int32_t));
    
    if (w_ret < 0)
    {
        FLOG_ERROR("Call setsockopt SO_SNDBUF error! w_ret = %d\r\n", w_ret);

        close (w_rrh_socket);
        return TRANS_FAILD;    
    }
    
    /* Zero out structure */
    memset(&st_peer_addr, 0, sizeof(st_peer_addr));    
    /* Create an AF_INET address */
    st_peer_addr.sin_family = AF_INET; 
    st_peer_addr.sin_port = TRANS_HTONS(g_trans_rrh_eqp_config.us_rrh_tcp_port);     
    st_peer_addr.sin_addr.s_addr = g_trans_rrh_eqp_config.uw_rrh_ip_addr; 

    /*Connect  persistent */
    w_ret = connect(w_rrh_socket, (struct sockaddr * )&st_peer_addr, sin_size); 
    if (w_ret < 0)
    {
        FLOG_ERROR("Call connect error! w_ret = %d, w_rrh_socket = %d.\r\n", w_ret, w_rrh_socket);

        close(w_rrh_socket);
        return TRANS_FAILD;
    }

    #if 0
    st_time_val.tv_sec = 0;   
    st_time_val.tv_usec = 60000;  /*60ms*/
    
    w_ret = setsockopt (w_rrh_socket, SOL_SOCKET, SO_RCVTIMEO, &st_time_val, sizeof(st_time_val));
    
    if (w_ret < 0)
    {
        FLOG_ERROR("Call setsockopt SO_RCVTIMEO error! w_ret = %d\r\n", w_ret);
    
        close (w_rrh_socket);
        return TRANS_FAILD;
    }
    #endif
    
    st_time_val.tv_sec = 0;   
    st_time_val.tv_usec = 6000; /*6ms*/
    
    w_ret = setsockopt (w_rrh_socket, SOL_SOCKET, SO_SNDTIMEO, &st_time_val, sizeof(st_time_val));
    
    if (w_ret < 0)
    {
        FLOG_ERROR("Call setsockopt SO_SNDTIMEO error! w_ret = %d\r\n", w_ret);
    
        close (w_rrh_socket);
        return TRANS_FAILD;
    }


    /*Send msg to query RRUID */
    w_ret = trans_rrh_tcp_connect(w_rrh_socket);
    if(TRANS_SUCCESS != w_ret) 
    {
        FLOG_ERROR("Call trans_rrh_tcp_connect error!w_ret = %d. \r\n", w_ret);
        return w_ret;
    }

    g_trans_rrh_socket = w_rrh_socket;
    
    /*Add device List-----but not active*/
    st_device_info.uc_module_type = TRANS_MOUDLE_RRH;
    st_device_info.w_sockfd = w_rrh_socket;
    st_device_info.uc_states = TRANS_DEVICE_ACTIVE;
    
    if (1 == uc_mac_flag)
    {
        memset(st_device_info.a_mac, 0xff, TRANS_MAC_ADDR_LEN);
    
    }
    else
    {
        memcpy(st_device_info.a_mac, g_trans_rrh_eqp_config.a_rrh_mac_addr, TRANS_MAC_ADDR_LEN);
    }
    
    w_ret = trans_device_add(&st_device_info, &p_device);
    if(TRANS_SUCCESS != w_ret) 
    {
        FLOG_ERROR("Call trans_device_add error!w_ret = %d. \r\n", w_ret);
        return w_ret;
    }
    
    FLOG_DEBUG_TRANS(g_trans_debug_rrh, "Exit \r\n");
    
    return w_ret;
}

#else


/*****************************************************************************+
* Function: trans_rrh_tcp_socket()
* Description: 
* Parameters:
*           NONE
* Return Values:
*           TRANS_FAILD : 1
*           TRANS_SUCCESS  : 0
*
*  
*  Data:    2011-03-23
* 
+*****************************************************************************/
u_int32_t trans_rrh_tcp_socket()
{

    int32_t w_server_socket = 0, w_client_socket = 0, w_ret = 0;
    struct sockaddr_in st_server_addr; 
    struct sockaddr_in st_client_addr; 
    socklen_t sin_size = sizeof(st_client_addr);

    int32_t w_total = 0; 
    int32_t w_sendbuflen = 0, w_recvbuflen = 0, w_reuseORnot = 0;

    int32_t w_if_listen_timeout = TRANS_HEARTBEAT_TIMEOUT;
    u_int8_t  uc_mac_flag = TRANS_USE_RRH_DEFAULT_MAC;

    struct timeval st_time_val;

    FLOG_DEBUG_TRANS(g_trans_debug_rrh, "Enter \r\n");
   
    /* Create an IPv4 Internet Socket */
    w_ret = w_server_socket = socket (AF_INET, SOCK_STREAM, 0);

    if (w_ret < 0)
    {
        FLOG_ERROR("Creat socket error! w_ret = %d\r\n", w_ret);
        return TRANS_FAILD;
    }

    /*Set if reuse the adderss*/
    w_reuseORnot = 1;
    w_ret = setsockopt (w_server_socket, SOL_SOCKET, SO_REUSEADDR, &w_reuseORnot,
            sizeof(int32_t));

    if (w_ret < 0)
    {
        FLOG_ERROR("Call setsockopt SO_REUSEADDR error! w_ret = %d\r\n", w_ret);

        close (w_server_socket);
        return TRANS_FAILD;

    }


    /*Set the length of the revice buffer*/
    w_recvbuflen = RRH_REV_BUF_MAX_LEN;
    w_ret = setsockopt (w_server_socket, SOL_SOCKET, SO_RCVBUF, &w_recvbuflen, sizeof(int32_t));

    if (w_ret < 0)
    {
        FLOG_ERROR("Call setsockopt SO_RCVBUF error! w_ret = %d\r\n", w_ret);

        close (w_server_socket);
        return TRANS_FAILD;

    }

    /*Set the length of the Send buffer*/
    w_sendbuflen = RRH_SEND_BUF_MAX_LEN;
    w_ret = setsockopt (w_server_socket, SOL_SOCKET, SO_SNDBUF, &w_sendbuflen, sizeof(int32_t));

    if (w_ret < 0)
    {

        FLOG_ERROR("Call setsockopt SO_SNDBUF error! w_ret = %d\r\n", w_ret);

        close (w_server_socket);
        return TRANS_FAILD;

    }

    /* Create an AF_INET address */
    st_server_addr.sin_family = AF_INET;         
    st_server_addr.sin_port = TRANS_HTONS(g_trans_rrh_eqp_config.us_ser_tcp_port); 
    st_server_addr.sin_addr.s_addr = g_trans_rrh_eqp_config.uw_ser_ip_addr; 

    bzero (& ( st_server_addr.sin_zero ), 8);

    /* Now bind the address to the socket */
    w_ret = bind (w_server_socket, (struct sockaddr *) &st_server_addr, sizeof(struct sockaddr));

    if (w_ret < 0)
    {

        FLOG_ERROR("Bind error! w_ret = %d\r\n", w_ret);
        close (w_server_socket);
        
        return TRANS_FAILD;
    }
   
    /*listen*/
    w_total = 5; 
    w_ret = listen (w_server_socket, w_total);
    if (w_ret < 0)
    {
        FLOG_ERROR("Listen error! w_ret = %d\r\n", w_ret);
    
        //close (w_server_socket);        
        return TRANS_FAILD;
    }
    
    if (w_if_listen_timeout > 0)
    {

        fd_set readfds;
        struct timeval st_time_val;
    
        st_time_val.tv_sec = TRANS_RRH_LISTEN_TIMEOUT;   
        st_time_val.tv_usec = 100;
    
        FD_ZERO(&readfds);
        FD_SET(w_server_socket, &readfds);
    
        w_ret = select(w_server_socket + 1, &readfds, NULL, NULL, &st_time_val);
    
        if(w_ret <0)
        {
            /*If error, do what ?????*/
            FLOG_ERROR("select error! w_ret = %d\r\n", w_ret);
            //close(w_socket);
            return TRANS_FAILD;
        }
        else if (0 == w_ret)
        {
            FLOG_ERROR("Select time out! w_ret = %d\r\n", w_ret);
            FLOG_ERROR("Listen failed: No one connect to BS\r\n");
            return TRANS_FAILD;
        }
    
        if((FD_ISSET(w_server_socket, &readfds)))
        {
            /*Accept the connection->Connect establish*/
            w_client_socket = accept(w_server_socket, (struct sockaddr *)&st_client_addr, &sin_size);
                
            if (w_client_socket <= 0)
            {
                FLOG_ERROR("Accept error! w_ret = %d\r\n", w_ret);
                close (w_server_socket);
                return TRANS_FAILD;
            }  
        
            //printf("new client[%d] %s:%d\n", conn_num, inet_ntoa(client_addr.sin_addr), TRANS_NTOHS(client_addr.sin_port));
            //g_trans_moudle_socket_fd[TRANS_MOUDLE_RRH] = w_client_socket;
            
        }
    }
    else
    {
        /*Accept the connection->Connect establish*/
        w_client_socket = accept(w_server_socket, (struct sockaddr *)&st_client_addr, &sin_size);
            
        if (w_client_socket <= 0)
        {
        
            FLOG_ERROR("Accept error! w_ret = %d\r\n", w_ret);
            close (w_server_socket);
            return TRANS_FAILD;
        }  

        st_time_val.tv_sec = 0;   
        st_time_val.tv_usec = 60000;  /*60ms*/
        
        w_ret = setsockopt (w_client_socket, SOL_SOCKET, SO_RCVTIMEO, &st_time_val, sizeof(st_time_val));
        
        if (w_ret < 0)
        {
            FLOG_ERROR("Call setsockopt SO_RCVTIMEO error! w_ret = %d\r\n", w_ret);
        
            close (w_client_socket);
            return TRANS_FAILD;
        }
        
        st_time_val.tv_sec = 0;   
        st_time_val.tv_usec = 6000; /*6ms*/
        
        w_ret = setsockopt (w_client_socket, SOL_SOCKET, SO_SNDTIMEO, &st_time_val, sizeof(st_time_val));
        
        if (w_ret < 0)
        {
            FLOG_ERROR("Call setsockopt SO_SNDTIMEO error! w_ret = %d\r\n", w_ret);
        
            close (w_client_socket);
            return TRANS_FAILD;
        }
    
        //printf("new client[%d] %s:%d\n", conn_num, inet_ntoa(client_addr.sin_addr), TRANS_NTOHS(client_addr.sin_port));
        //g_trans_moudle_socket_fd[TRANS_MOUDLE_RRH] = w_client_socket;
    }

    g_trans_rrh_socket = w_client_socket;

    /*Add device List-----but not active*/
    struct trans_device_info st_device_info;
    void * p_device = NULL;
    
    st_device_info.uc_module_type = TRANS_MOUDLE_RRH;
    st_device_info.w_sockfd = w_client_socket;
    st_device_info.uc_states = TRANS_DEVICE_ACTIVE;
    
    if (1 == uc_mac_flag)
    {
        memset(st_device_info.a_mac, 0xff, TRANS_MAC_ADDR_LEN);

    }
    else
    {
        memcpy(st_device_info.a_mac, g_trans_rrh_eqp_config.a_rrh_mac_addr, TRANS_MAC_ADDR_LEN);
    }

    
    w_ret = trans_device_add(&st_device_info, &p_device);
    if(TRANS_SUCCESS != w_ret) 
    {
        FLOG_ERROR("Call trans_device_add error!w_ret = %d. \r\n", w_ret);
        return w_ret;
    }

    FLOG_INFO("Exit client_socket = %d\r\n", w_client_socket);

    return TRANS_SUCCESS;
}


/*****************************************************************************+
* Function: trans_rrh_udp_brocast_process()
* Description: 
* Parameters:
*           NONE
* Return Values:
*           TRANS_FAILD : 1
*           TRANS_SUCCESS  : 0
*  
*  Data:    2011-03-23
* 
+*****************************************************************************/
u_int32_t trans_rrh_udp_brocast_process(struct rrh_conn_req_msg *p_req_msg, 
            struct rrh_conn_rep_msg *p_rep_msg)
{
    struct rrh_conn_req_msg *p_request_msg;
    struct rrh_conn_rep_msg *p_response_msg;

    u_int32_t   uw_rru_id;  /*RRU ID*/
    u_int8_t     a_mac_addr[RRH_MAC_ADDER_LEN];    /*RRU MAC  adderss*/
    
    FLOG_DEBUG_TRANS(g_trans_debug_rrh, "Enter \r\n");

    if ((NULL == p_req_msg) || (NULL == p_rep_msg) )
    {
        FLOG_ERROR("NULL PTR! \r\n");
        return TRANS_FAILD;  
    }
    
    /*analysis the msg from RRH */
    p_request_msg = (struct rrh_conn_req_msg *)p_req_msg;

    uw_rru_id = TRANS_NTOHL(p_request_msg->uw_rru_id);

    FLOG_DEBUG_TRANS(g_trans_debug_rrh, "uw_rru_id = %d \r\n", uw_rru_id);
    
    memset((u_int8_t*)a_mac_addr, 0, RRH_MAC_ADDER_LEN);    
    /*Destinate MAC*/
    memcpy(a_mac_addr, p_request_msg->a_mac_addr, RRH_MAC_ADDER_LEN);
    
    memcpy(g_trans_rrh_eqp_config.a_rrh_mac_addr, p_request_msg->a_mac_addr, RRH_MAC_ADDER_LEN);

    /*check RRUID--Not the aimb. Do not accept*/
    if (uw_rru_id != g_trans_rrh_eqp_config.uw_rru_id)
    {

        FLOG_WARNING("new_rru_id = %d, aim_id = %d\r\n", uw_rru_id, g_trans_rrh_eqp_config.uw_rru_id);
        return TRANS_FAILD;
    }    
    
    /*????? a_mac_addr*/

    /*Build a new message sent to RRH*/
    p_response_msg = (struct rrh_conn_rep_msg *)p_rep_msg;

    //memset((u_int8_t*)g_rrh_send_msg, 0, RRH_SEND_MSG_MAX_LEN);

    p_response_msg->uw_rru_id = TRANS_HTONL(uw_rru_id);    /*RRU ID*/
    p_response_msg->uc_server_id = g_trans_rrh_eqp_config.uc_server_id;   /*SERVER ID*/

    /*RRU MAC */
    memcpy(p_response_msg->a_rrh_mac_addr, a_mac_addr, RRH_MAC_ADDER_LEN);

    /*RRU  IP*/
    memcpy(p_response_msg->a_rrh_ip_addr, &(g_trans_rrh_eqp_config.uw_rrh_ip_addr), RRH_IP_ADDER_LEN);

    /*RRU  mask*/
    memcpy(p_response_msg->a_rrh_mask_addr, &(g_trans_rrh_eqp_config.uw_rrh_mask_addr), RRH_SUBNET_MASK_LEN);

    /*SERVER  IP*/
    memcpy(p_response_msg->a_ser_ip_addr, &(g_trans_rrh_eqp_config.uw_ser_ip_addr), RRH_IP_ADDER_LEN);
    
    p_response_msg->us_ser_tcp_port = TRANS_HTONS(g_trans_rrh_eqp_config.us_ser_tcp_port); /*SERVE  TCP  PORT*/
    p_response_msg->us_ser_data_port = TRANS_HTONS(g_trans_rrh_eqp_config.us_ser_data_udp_port); /*SERVER  I/Q Data  PORT*/

    //*p_new_msg_len = SIZEOF_RRH_CONN_REP_MSG;
    
   FLOG_DEBUG_TRANS(g_trans_debug_rrh, "Exit \r\n"); 
    
    return TRANS_SUCCESS; 

}

#ifdef TRANS_RRH_RAW_SOCKET

u_int32_t trans_rrh_do_promisc(char *nif, int sock) 
{
    struct ifreq ifr;
    struct sockaddr_ll sll;

    FLOG_DEBUG("nif = %s\r\n", nif);

    memset (&ifr, 0, sizeof(struct ifreq));    
    strncpy(ifr.ifr_name, nif,strlen(nif) + 1);

    if((ioctl(sock, SIOCGIFFLAGS, &ifr) == -1))
    {
        FLOG_ERROR("Open Ethernet failed");
        return TRANS_FAILD;
    }

    ifr.ifr_flags |= IFF_PROMISC;

    if(ioctl(sock, SIOCSIFFLAGS, &ifr) == -1 )
    {
        FLOG_ERROR("Set Ethernet card promisc failed");
        return TRANS_FAILD;
    }

    memset (&ifr, 0, sizeof(struct ifreq));
    strncpy(ifr.ifr_name, nif, strlen(nif) + 1);

    if (ioctl(sock,SIOCGIFINDEX,&ifr))
    {
        FLOG_ERROR("Get Ethernet card infomation failed");
        return TRANS_FAILD;
    }

    memset(&sll, 0, sizeof(struct sockaddr_ll));
    sll.sll_family = AF_PACKET;
    sll.sll_ifindex = ifr.ifr_ifindex;
    sll.sll_protocol = TRANS_HTONS(ETH_P_ALL);

    if( bind(sock, (struct sockaddr *)&sll, sizeof(struct sockaddr_ll)) == -1 )
    {
        FLOG_ERROR("Bind Ethernet card failed");
        return TRANS_FAILD;
    } 

    return TRANS_SUCCESS;
}

/*****************************************************************************+
* Function: trans_rrh_udp_socket_send()
* Description: 
* Parameters:
*           NONE
* Return Values:
*           TRANS_FAILD : 1
*           TRANS_SUCCESS  : 0

*  
*  Data:    2011-03-23
* 
+*****************************************************************************/
u_int32_t trans_rrh_udp_socket_send(struct rrh_conn_req_msg *p_request_msg)
{
    u_int32_t uw_ret = 0;
    u_int32_t uw_msg_len = SIZEOF_RRH_CONN_REQ_MSG;

    int32_t w_serversocket = 0; 
    struct sockaddr_in st_serveraddress, st_clientaddress; 

    int32_t w_so_broadcast = 1; 
    int32_t w_size = 0; 
    //int32_t w_ret = 0; 
    
    socklen_t uw_sin_size = sizeof(struct sockaddr_in);    
    int32_t w_reuseORnot = 0;

    #if 0
    struct timeval start_tv;      
    fd_set readfds;
    struct timeval end_tv;
    struct timeval st_time_val; 
    #endif
    
    //static struct rrh_conn_req_msg st_request_msg;
    static struct rrh_conn_rep_msg st_response_msg;

    /*analysis the msg from RRH */

    FLOG_DEBUG_TRANS(g_trans_debug_rrh, "Enter \r\n");
    
    if((w_serversocket = (socket(AF_INET, SOCK_DGRAM, 0))) < 0) 
    {
        FLOG_ERROR("Creat socket error! w_serversocket = %d\r\n", w_serversocket);
        return TRANS_FAILD;
    }

    if(setsockopt(w_serversocket, SOL_SOCKET, SO_BROADCAST, &w_so_broadcast, sizeof(w_so_broadcast)) < 0) 
    {
        FLOG_ERROR("Call setsockopt SO_BROADCAST error!\r\n");

        return TRANS_FAILD;
    } 
    
    /*Set if reuse the adderss*/
    w_reuseORnot = 1;
    if(setsockopt(w_serversocket, SOL_SOCKET, SO_REUSEADDR, &w_reuseORnot, sizeof(w_reuseORnot)) < 0) 
    {
        FLOG_ERROR("Call setsockopt SO_REUSEADDR error!\r\n");

        return TRANS_FAILD;
    }     

    st_serveraddress.sin_family = AF_INET; 
    st_serveraddress.sin_port = TRANS_HTONS(g_trans_rrh_eqp_config.us_ser_udp_port); 
    //st_serveraddress.sin_addr.s_addr = g_trans_rrh_eqp_config.uw_ser_ip_addr; 
    //st_serveraddress.sin_addr.s_addr = g_trans_rrh_server_socket.uw_ipAddr;
    //st_serveraddress.sin_addr.s_addr = inet_addr("9.186.57.90");
    st_serveraddress.sin_addr.s_addr = TRANS_HTONL(INADDR_ANY);
    //st_serveraddress.sin_addr.s_addr = g_trans_rrh_eqp_config.uw_ser_ip_addr;

    if((bind(w_serversocket, (struct sockaddr *)&st_serveraddress, sizeof(st_serveraddress))) < 0) 
    {
        FLOG_ERROR("Bind error!\r\n");
        
        close(w_serversocket);
        return TRANS_FAILD;
    }

    st_clientaddress.sin_family = AF_INET; 
    st_clientaddress.sin_port = TRANS_HTONS(g_trans_rrh_eqp_config.us_rrh_udp_port); 
    //st_clientaddress.sin_addr.s_addr = TRANS_HTONL(INADDR_BROADCAST); 
    //st_clientaddress.sin_addr.s_addr = g_trans_rrh_client_socket.uw_ipAddr; 
    st_clientaddress.sin_addr.s_addr = g_trans_rrh_eqp_config.uw_ser_broc_addr;

    //FLOG_ERROR("%d!\r\n", g_trans_rrh_eqp_config.uw_ser_broc_addr);
    
    trans_debug_msg_print(p_request_msg, uw_msg_len, g_trans_debug_rrh);
    
    /*process the revMsg, then build a new msg broadcasting to rrh*/
    memset((u_int8_t*)&st_response_msg, 0, SIZEOF_RRH_CONN_REP_MSG);
    
    uw_ret = trans_rrh_udp_brocast_process(p_request_msg, &st_response_msg);
    
    /*no need for sending msg*/
    if (TRANS_SUCCESS != uw_ret)
    {
        return TRANS_FAILD;
    }
    
    /*Send the Msg to RRH*/
    uw_msg_len = SIZEOF_RRH_CONN_REP_MSG;
    
    w_size = sendto(w_serversocket, &st_response_msg, uw_msg_len, 0,
                        (struct sockaddr *)&st_clientaddress, uw_sin_size);
    
    if(w_size < 0) 
    {
    
        FLOG_ERROR("Sendto error!w_size = %d\r\n", w_size);
        close(w_serversocket);
        return TRANS_FAILD;
    }
    else 
    {
    
        trans_debug_msg_print(&st_response_msg, uw_msg_len, g_trans_debug_rrh);
        
        FLOG_INFO("Exit :UDP socket finished.Close UDP socket. \r\n");  
        close(w_serversocket);
        
        return TRANS_SUCCESS; 
    
    }

    FLOG_DEBUG("Exit \r\n");  
    
    return TRANS_SUCCESS; 
}

/*****************************************************************************+
* Function: trans_rrh_udp_raw_socket_timer_func()
* Description: 
* Parameters:
*           NONE
* Return Values:
*           TRANS_FAILD : 1
*           TRANS_SUCCESS  : 0

*  
*  Data:    2011-03-23
* 
+*****************************************************************************/
int trans_rrh_udp_raw_socket_timer_func(void *p_msg, size_t len, void *p_msg_info)
{
    FLOG_WARNING("Time out \r\n");  
    
    (void)p_msg;
    (void)len;
    (void)p_msg_info;
    
    close(g_trans_rrh_udp_raw_socket);

    g_trans_rrh_udp_raw_socket = -1;

    return TRANS_SUCCESS; 

}


/*****************************************************************************+
* Function: trans_rrh_udp_raw_socket()
* Description: 
* Parameters:
*           NONE
* Return Values:
*           TRANS_FAILD : 1
*           TRANS_SUCCESS  : 0

*  
*  Data:    2011-03-23
* 
+*****************************************************************************/
u_int32_t trans_rrh_udp_raw_socket(void)
{
    u_int32_t uw_ret = 0;
    u_int32_t uw_msg_len = 0;

    //int32_t w_serversocket = 0; 
    struct sockaddr_in st_serveraddress; 
    //struct sockaddr_in st_clientaddress; 

   // int32_t w_so_broadcast = 1; 
    int32_t w_size = 0; 
    //int32_t w_ret = 0; 

    socklen_t uw_sin_size = sizeof(struct sockaddr_in);
//    int32_t w_reuseORnot = 0;
    
    static struct rrh_conn_req_msg st_request_msg;
    //static struct rrh_conn_rep_msg st_response_msg;

    struct ethhdr *p_eth = NULL;
    struct iphdr *p_ip = NULL;
    struct udphdr *p_udp = NULL;
    /*add more protocol head here....*/

    u_int8_t *p_temp = NULL;
    //char ss[32], dd[32];
    int w_src_port = 0; 
    int w_dest_port = 0;
    char a_rev_buf[2*32767] = {0};

    struct trans_timer_info st_timer_info;    
    void* p_timer = NULL;
    
    /*analysis the msg from RRH */

    FLOG_DEBUG_TRANS(g_trans_debug_rrh, "Enter \r\n");

    if((g_trans_rrh_udp_raw_socket = (socket(AF_PACKET, SOCK_RAW, TRANS_HTONS(ETH_P_ALL)))) < 0) 
    {
        FLOG_ERROR("Creat socket error! w_serversocket = %d\r\n", g_trans_rrh_udp_raw_socket);
        return TRANS_FAILD;
    }

    if ( trans_rrh_do_promisc((char *)g_trans_rrh_eqp_config.a_rrh_nic_if, g_trans_rrh_udp_raw_socket) != TRANS_SUCCESS)
    {
        FLOG_ERROR("Configure RAW socket error! %d\r\n", g_trans_rrh_udp_raw_socket);
        close(g_trans_rrh_udp_raw_socket);
        return TRANS_FAILD;
    }
   
    st_timer_info.f_callback = trans_rrh_udp_raw_socket_timer_func;
    st_timer_info.p_data = NULL;
    st_timer_info.p_timer_list = &g_trans_timer_list;
    st_timer_info.uc_type = TRANS_TIMER_TYPE_ONCE;
    st_timer_info.uw_interval = TRANS_RRH_RAW_SOCKFD_TIMEOUT;
    
    uw_ret = trans_timer_add(&st_timer_info, &p_timer);    
    if (TRANS_SUCCESS != uw_ret)
    {
        FLOG_ERROR("Call trans_timer_add error! uw_ret = %d\r\n", uw_ret);
        return TRANS_FAILD;
    }
    
    //FLOG_ERROR("g_trans_rrh_udp_raw_socket = %d\r\n", g_trans_rrh_udp_raw_socket);
        
    while (1) 
    { 
     
        //r = recvfrom(sock, (char *)buf, sizeof(buf), 0, (struct sockaddr *)&addr, &uw_len);
        uw_sin_size = sizeof(struct sockaddr_in);
        
        w_size = recvfrom(g_trans_rrh_udp_raw_socket, (char *)a_rev_buf, sizeof(a_rev_buf), 0, 
                            (struct sockaddr *)&st_serveraddress, &uw_sin_size);
    
        if(w_size < 0) 
        {
    
            FLOG_ERROR("Recvfrom error!w_size = %d\r\n", w_size);
            close(g_trans_rrh_udp_raw_socket);
            return TRANS_FAILD;
        }

        a_rev_buf[w_size] = 0;
        p_temp = (u_int8_t *)a_rev_buf;
        /*which can get source mac address and destnation address, and which network packet, here is OSI-2, link layer*/
        p_eth = (struct ethhdr *)p_temp;

        p_temp += sizeof(struct ethhdr);
        /*which get IP layer informations, includes which transport protocol, source and destnation IP address...*/     
        p_ip = (struct iphdr *)p_temp;
        /*
         * which can get transport layer informations, such as: transport socket port, transport layer includes
         * TCP, UDP, ICMP, IGMP......, can get which transport protocol from IP header
         */
        p_temp += sizeof(struct iphdr);

        if (IPPROTO_UDP == p_ip->protocol)
        {
            p_udp = (struct udphdr *)p_temp;
            
            //strcpy(ss, inet_ntoa(*(struct in_addr*)&(pip->saddr)));
            //strcpy(dd, inet_ntoa(*(struct in_addr*)&(pip->daddr)));
            
            w_src_port = TRANS_NTOHS(p_udp->source);
            w_dest_port = TRANS_NTOHS(p_udp->dest);

            if ((g_trans_rrh_eqp_config.us_rrh_udp_port == w_src_port)
                && (g_trans_rrh_eqp_config.us_ser_udp_port == w_dest_port))
            {
                p_temp += sizeof(struct udphdr);

                /*Rev Msg from the RRH */
                memset((u_int8_t*)&st_request_msg, 0, SIZEOF_RRH_CONN_REQ_MSG);
                uw_msg_len = SIZEOF_RRH_CONN_REQ_MSG;
                
                memcpy(&st_request_msg, p_temp, uw_msg_len);

                uw_ret = trans_rrh_udp_socket_send(&st_request_msg);
                if (TRANS_SUCCESS != uw_ret)
                {
                    FLOG_ERROR("Call trans_rrh_udp_socket error!uw_ret = %d\r\n", uw_ret);
                    continue;
                }
                else
                {
                    trans_timer_delete(&g_trans_timer_list, p_timer);
                    
                    FLOG_INFO("Exit :RAW UDP socket finished.Close UDP socket. \r\n");  
                    close(g_trans_rrh_udp_raw_socket);
                    g_trans_rrh_udp_raw_socket = -1;
                    
                    return TRANS_SUCCESS; 
                }
            }
            else
            {
                 FLOG_DEBUG_TRANS(g_trans_debug_rrh, "Port is diff:%d, %d, %d, %d \r\n", 
                                    g_trans_rrh_eqp_config.us_rrh_udp_port, w_src_port,
                                    g_trans_rrh_eqp_config.us_ser_udp_port, w_dest_port);            
                continue;
            }

        }
        else
        {
            FLOG_DEBUG_TRANS(g_trans_debug_rrh, "Other pkt, protocl:%d\n", p_ip->protocol);            
            continue;
        }
    
    }

    
    FLOG_DEBUG("Exit \r\n");  
    
    return TRANS_SUCCESS; 
}

#else


/*****************************************************************************+
* Function: trans_rrh_udp_socket()
* Description: 
* Parameters:
*           NONE
* Return Values:
*           TRANS_FAILD : 1
*           TRANS_SUCCESS  : 0

*  
*  Data:    2011-03-23
* 
+*****************************************************************************/
u_int32_t trans_rrh_udp_socket(void)
{
    u_int32_t uw_ret = 0;
    u_int32_t uw_msg_len = 0;

    int32_t w_serversocket = 0; 
    struct sockaddr_in st_serveraddress, st_clientaddress; 

    int32_t w_so_broadcast = 1; 
    int32_t w_size = 0; 
    int32_t w_ret = 0; 

    socklen_t uw_sin_size = sizeof(struct sockaddr_in);
    int32_t w_reuseORnot = 0;

    struct timeval start_tv;      
    fd_set readfds;
    struct timeval end_tv;
    struct timeval st_time_val;  
    
    static struct rrh_conn_req_msg st_request_msg;
    static struct rrh_conn_rep_msg st_response_msg;

    /*analysis the msg from RRH */

    FLOG_DEBUG_TRANS(g_trans_debug_rrh, "Enter \r\n");
    
    if((w_serversocket = (socket(AF_INET, SOCK_DGRAM, 0))) < 0) 
    {
        FLOG_ERROR("Creat socket error! w_serversocket = %d\r\n", w_serversocket);
        return TRANS_FAILD;
    }

    if(setsockopt(w_serversocket, SOL_SOCKET, SO_BROADCAST, &w_so_broadcast, sizeof(w_so_broadcast)) < 0) 
    {
        FLOG_ERROR("Call setsockopt SO_BROADCAST error!\r\n");

        return TRANS_FAILD;
    } 
    
    /*Set if reuse the adderss*/
    w_reuseORnot = 1;
    if(setsockopt(w_serversocket, SOL_SOCKET, SO_REUSEADDR, &w_reuseORnot, sizeof(w_reuseORnot)) < 0) 
    {
        FLOG_ERROR("Call setsockopt SO_REUSEADDR error!\r\n");

        return TRANS_FAILD;
    }     

    st_serveraddress.sin_family = AF_INET; 
    st_serveraddress.sin_port = TRANS_HTONS(g_trans_rrh_eqp_config.us_ser_udp_port); 
    //st_serveraddress.sin_addr.s_addr = g_trans_rrh_eqp_config.uw_ser_ip_addr; 
    //st_serveraddress.sin_addr.s_addr = g_trans_rrh_server_socket.uw_ipAddr;
    //st_serveraddress.sin_addr.s_addr = inet_addr("9.186.57.90");
    st_serveraddress.sin_addr.s_addr = TRANS_HTONL(INADDR_ANY);

    if((bind(w_serversocket, (struct sockaddr *)&st_serveraddress, sizeof(st_serveraddress))) < 0) 
    {
        FLOG_ERROR("Bind error!\r\n");
        
        close(w_serversocket);
        return TRANS_FAILD;
    }

    st_clientaddress.sin_family = AF_INET; 
    st_clientaddress.sin_port = TRANS_HTONS(g_trans_rrh_eqp_config.us_rrh_udp_port); 
    //st_clientaddress.sin_addr.s_addr = TRANS_HTONL(INADDR_BROADCAST); 
    //st_clientaddress.sin_addr.s_addr = g_trans_rrh_client_socket.uw_ipAddr; 
    st_clientaddress.sin_addr.s_addr = g_trans_rrh_eqp_config.uw_ser_broc_addr;
   
    gettimeofday(&start_tv, NULL);
    
    while (1) 
    { 
        /*Rev Msg from the RRH */
        memset((u_int8_t*)&st_request_msg, 0, SIZEOF_RRH_CONN_REQ_MSG);
        uw_msg_len = SIZEOF_RRH_CONN_REQ_MSG;
       
        gettimeofday(&end_tv, NULL);

        if ((end_tv.tv_sec -start_tv.tv_sec) >= 15)
        {
            FLOG_ERROR("Time out, Connect to RRH error\r\n");
            close(w_serversocket);
            return TRANS_FAILD;
        }
        
        /*Time out 15s*/
        st_time_val.tv_sec = (15 - (end_tv.tv_sec -start_tv.tv_sec));   
        st_time_val.tv_usec = 0;
        
        FD_ZERO(&readfds);
        FD_SET(w_serversocket, &readfds);
        
        w_ret = select(w_serversocket + 1, &readfds, NULL, NULL, &st_time_val);
        
        if(w_ret <0)
        {
            FLOG_ERROR("select error! w_ret = %d\r\n", w_ret);
            close(w_serversocket);
            return TRANS_FAILD;
        }
        else if (0 == w_ret)
        {
            FLOG_ERROR("Select time out, Connect to RRH error! w_ret = %d\r\n", w_ret);
            close(w_serversocket);
            return TRANS_FAILD;
        }

        if ((FD_ISSET(w_serversocket, &readfds)) <= 0)
        {
            
            continue;
        }        
        
        uw_sin_size = sizeof(struct sockaddr_in);
        
        w_size = recvfrom(w_serversocket, &st_request_msg, uw_msg_len, 0, 
                            (struct sockaddr *)&st_serveraddress, &uw_sin_size);

        if(w_size < 0) 
        {

            FLOG_ERROR("Recvfrom error!w_size = %d\r\n", w_size);
            close(w_serversocket);
            return TRANS_FAILD;
        }

        trans_debug_msg_print(&st_request_msg, uw_msg_len, g_trans_debug_rrh);
        
        /*process the revMsg, then build a new msg broadcasting to rrh*/
        memset((u_int8_t*)&st_response_msg, 0, SIZEOF_RRH_CONN_REP_MSG);
        
        uw_ret = trans_rrh_udp_brocast_process(&st_request_msg, &st_response_msg);

        /*no need for sending msg*/
        if (TRANS_SUCCESS != uw_ret)
        {
            
            continue;
        }

        /*Send the Msg to RRH*/
        uw_msg_len = SIZEOF_RRH_CONN_REP_MSG;
        
        w_size = sendto(w_serversocket, &st_response_msg, uw_msg_len, 0,
                            (struct sockaddr *)&st_clientaddress, uw_sin_size);
        
        if(w_size < 0) 
        {

            FLOG_ERROR("Sendto error!w_size = %d\r\n", w_size);
            close(w_serversocket);
            return TRANS_FAILD;
        }
        else 
        {

            FLOG_INFO("Exit :UDP socket finished.Close UDP socket. \r\n");  
            close(w_serversocket);
            
            return TRANS_SUCCESS; 

        }

    }
    FLOG_DEBUG_TRANS(g_trans_debug_rrh, "Exit \r\n");  
    
    return TRANS_SUCCESS; 
}

#endif

#endif

/*****************************************************************************+
* Function: trans_rrh_register_func()
* Description: 
* Parameters:
*           NONE
* Return Values:
*           TRANS_FAILD : 1
*           TRANS_SUCCESS  : 0
*
*  
*  Data:    2011-09-05
* 
+*****************************************************************************/
u_int32_t trans_rrh_register_func()
{    
    u_int16_t us_op = 0;
    u_int32_t uw_ret = 0;

    us_op = 1;
    uw_ret = trans_register_func_callback(TRANS_REGISTER_FUN_RRH_MSG_PRO,
                                    &us_op,  
                                    trans_rrh_func_alert,
                                    NULL);
    if (TRANS_SUCCESS != uw_ret)
    {
        FLOG_ERROR("Call trans_register_func_callback for OP%d error! uw_ret = %d\r\n", us_op, uw_ret);
    
        return TRANS_FAILD;    
    }  

    us_op = 2;
    uw_ret = trans_register_func_callback(TRANS_REGISTER_FUN_RRH_MSG_PRO,
                                    &us_op,  
                                    trans_rrh_func_query,
                                    NULL);
    if (TRANS_SUCCESS != uw_ret)
    {
        FLOG_ERROR("Call trans_register_func_callback OP%d error! uw_ret = %d\r\n", us_op, uw_ret);
    
        return TRANS_FAILD;    
    }  

    us_op = 3;
    uw_ret = trans_register_func_callback(TRANS_REGISTER_FUN_RRH_MSG_PRO,
                                    &us_op,  
                                    trans_rrh_func_config,
                                    NULL);
    if (TRANS_SUCCESS != uw_ret)
    {
        FLOG_ERROR("Call trans_register_func_callback OP%d error! uw_ret = %d\r\n", us_op, uw_ret);
    
        return TRANS_FAILD;    
    }  
    
    us_op = 4;
    uw_ret = trans_register_func_callback(TRANS_REGISTER_FUN_RRH_MSG_PRO,
                                    &us_op,  
                                    trans_rrh_func_heartbeat,
                                    NULL);
    if (TRANS_SUCCESS != uw_ret)
    {
        FLOG_ERROR("Call trans_register_func_callback OP%d error! uw_ret = %d\r\n", us_op, uw_ret);
    
        return TRANS_FAILD;    
    }  

    return TRANS_SUCCESS;
}

/*****************************************************************************+
* Function: trans_rrh_init()
* Description: init 
* Parameters:
*           p_init_info: init info
* Return Values:
*           TRANS_FAILD : 1
*           TRANS_SUCCESS  : 0

*  
*  Data:    2011-03-23
* 
+*****************************************************************************/

u_int32_t trans_rrh_init(struct trans_init_info *p_init_info)
{
    u_int32_t uw_ret = 0;
    /*Init the Global variables */

    if (NULL == p_init_info)
    {
        FLOG_ERROR("NULL PTR! \r\n");
        return TRANS_FAILD;
    }  

    memset((u_int8_t*)&g_trans_rrh_eqp_config, 0, sizeof(struct rrh_equipment_config_info));

    //g_ad_rrh_eqp_config.uw_rru_id;    /*RRU ID*/

    /*SERVER ID*/
    g_trans_rrh_eqp_config.uc_server_id = p_init_info->uc_server_id;    
    
    #ifdef TRANS_RRH_NEW_CONNECT
    /*RRU ID*/
    g_trans_rrh_eqp_config.uw_rru_id = 0xffffffff;    

    memcpy(g_trans_rrh_eqp_config.a_rrh_mac_addr, p_init_info->a_rrh_mac, RRH_MAC_ADDER_LEN);
    
    FLOG_DEBUG_TRANS(g_trans_debug_device, "MAC :0x%x:0x%x:0x%x:0x%x:0x%x:0x%x..\r\n", 
                        g_trans_rrh_eqp_config.a_rrh_mac_addr[0], 
                        g_trans_rrh_eqp_config.a_rrh_mac_addr[1], 
                        g_trans_rrh_eqp_config.a_rrh_mac_addr[2], 
                        g_trans_rrh_eqp_config.a_rrh_mac_addr[3], 
                        g_trans_rrh_eqp_config.a_rrh_mac_addr[4], 
                        g_trans_rrh_eqp_config.a_rrh_mac_addr[5]); 

    /*RRU  IP*/
    g_trans_rrh_eqp_config.uw_rrh_ip_addr = p_init_info->uw_rrh_ip_addr;

    #else
    /*RRU ID*/
    g_trans_rrh_eqp_config.uw_rru_id = p_init_info->uw_rru_id;  
    /*RRU  IP*/
    g_trans_rrh_eqp_config.uw_rrh_ip_addr = p_init_info->uw_rrh_ip_addr;
    /*RRU  mask*/
    g_trans_rrh_eqp_config.uw_rrh_mask_addr = p_init_info->uw_rrh_mask_addr;
    /*SERVER  IP*/
    g_trans_rrh_eqp_config.uw_ser_ip_addr = p_init_info->uw_ser_ip_addr;
    /*SERVER BROADCAST IP */
    g_trans_rrh_eqp_config.uw_ser_broc_addr = p_init_info->uw_ser_broc_addr;
    /*SERVER  TCP Port*/
    g_trans_rrh_eqp_config.us_ser_tcp_port = p_init_info->us_ser_tcp_port; 
    #endif

    g_trans_rrh_eqp_config.us_ser_udp_port = RRH_SERVER_UDP_PORT; /*SERVER  UDP Port*/
    g_trans_rrh_eqp_config.us_rrh_tcp_port = RRH_RRU_TCP_PORT; /*RRH  TCP Port*/
    g_trans_rrh_eqp_config.us_rrh_udp_port = RRH_RRU_UDP_PORT; /*RRH  UDP Port*/
    g_trans_rrh_eqp_config.us_rrh_data_udp_port = RRH_RRU_DATA_UDP_PORT; /*RRH  data UDP Port*/
    g_trans_rrh_eqp_config.us_ser_data_udp_port = p_init_info->us_ser_data_port; /*SERVER  I/Q  Data Port*/ 
    
    strcpy((char *)(g_trans_rrh_eqp_config.a_rrh_nic_if), (char *)(p_init_info->a_rrh_nic_if));

    g_trans_rrh_socket = -1;
    g_trans_rrh_udp_raw_socket = -1;

    g_trans_rrh_heartbeat.uc_count = 0;
    g_trans_rrh_heartbeat.uc_duration = TRANS_HEARTBEAT_TIMEOUT;
    g_trans_rrh_heartbeat.uc_send_flag = 1;
    g_trans_rrh_heartbeat.uc_send_num = 0;
    g_trans_rrh_heartbeat.uc_stop_num = 0;
    g_trans_rrh_heartbeat.p_timer = NULL;
    
    #if 0
    if (0 != create_sem (&g_trans_rrh_msg_sem))
    {
        FLOG_ERROR ("Enable msg sem error");
        return TRANS_FAILD;
    }
    #endif

    uw_ret = trans_rrh_register_func();
    if (TRANS_SUCCESS != uw_ret)
    {
        FLOG_ERROR("Call trans_rrh_register_func error! uw_ret = %d\r\n", uw_ret);
    
        return uw_ret;    
    } 

    pthread_mutex_init (&g_trans_rrh_serial_num_mutex, NULL);

    return TRANS_SUCCESS;

}

/*****************************************************************************+
* Function: trans_rrh_release()
* Description: init 
* Parameters:
*           NONE
* Return Values:
*           TRANS_FAILD : 1
*           TRANS_SUCCESS  : 0

*  
*  Data:    2011-03-23
* 
+*****************************************************************************/
u_int32_t trans_rrh_release()
{
    memset((u_int8_t*)&g_trans_rrh_eqp_config, 0, sizeof(struct rrh_equipment_config_info));

    #if 0
    if (0 != delete_sem(g_trans_rrh_msg_sem))
    {
        FLOG_ERROR ("Disable msg sem error");
        return TRANS_FAILD;
    }
    #endif
    
    /*Close socket*/
    close(g_trans_rrh_socket);
    g_trans_rrh_socket = -1;

    return TRANS_SUCCESS;
}

#endif


