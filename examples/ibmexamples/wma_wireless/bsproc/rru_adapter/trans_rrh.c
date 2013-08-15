/*****************************************************************************+
*
*  File Name: trans_rrh.c
*
*  Function: RRH Message Process
*
*  
*  Data:    2011-03-23
*  Modify:
*
+*****************************************************************************/


#include <sys/types.h>
#include <syslog.h>
#include <flog.h>
/*semaphore*/
#include <sem_util.h>
//#include "queue_util.h"
#ifdef TRANS_BS_COMPILE
#include <bs_cfg.h>
#endif
#ifdef TRANS_MS_COMPILE
#include <ms_cfg.h>
#endif

#include <trans.h>
#include <trans_rrh.h>
#include <trans_agent.h>
#include <trans_action.h>
#include <trans_list.h>
#include <trans_timer.h>
#include <trans_debug.h>

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
/*UDP*/
#include<sys/wait.h>
#include<fcntl.h>
#include <arpa/inet.h>
#include<netdb.h>

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

u_int8_t g_trans_rrh_heartbeat_num = 0;

/*Can't set the g_trans_rrh_serial_number = 0, 0 means nothing*/
u_int16_t g_trans_rrh_serial_number = 1;
pthread_mutex_t  g_trans_rrh_serial_num_mutex;

struct rrh_equipment_config_info g_trans_rrh_eqp_config;

//struct sem_handle * g_trans_rrh_msg_sem;

/*****************************************************************************+
 *Code 
+*****************************************************************************/
extern u_int32_t trans_msg_en_quene(void *p_msg, struct trans_en_queue_msg *p_en_quene);
extern u_int32_t trans_msg_de_quene(u_int8_t *p_result);
#endif

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
    
    //FLOG_DEBUG("Exit us_serial_num = %d\r\n", us_serial_num); 
    
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

    us_crc = ntohs(*p_crc);
    
    //FLOG_DEBUG("Exit us_crc = %d\r\n", us_crc); 
    
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

    //FLOG_DEBUG("Exit us_crc = %d\r\n", us_crc); 
    
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
    
    
    FLOG_DEBUG("Exit : rrh_param_len = %d. \r\n", *p_rrh_param_len);

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
    us_body_len = ntohs(p_rrh_rev_header->us_body_len);
    if (us_body_len >= RRH_MSG_MAX_LEN)
    {
        *p_rep_flag = RRH_MONITOR_REP_FLAG_LEN_ERR;

        FLOG_ERROR("Length error! us_body_len = %d\r\n", us_body_len);         
        return TRANS_SUCCESS;
    }

    /*param Check*/
    if ((g_trans_rrh_eqp_config.uw_rru_id != ntohl(p_rrh_rev_header->uw_rru_id))
        ||(g_trans_rrh_eqp_config.uc_server_id != p_rrh_rev_header->uc_server_id))
    {
        *p_rep_flag = RRH_MONITOR_REP_FLAG_OTHER_ERR;
        
        FLOG_ERROR("ID error! rru_id = %d, server_id = %d\r\n", p_rrh_rev_header->uw_rru_id, 
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
        
        FLOG_ERROR("CRC error! us_crc_val = %d, us_crc_temp = %d\r\n", us_crc_val, us_crc_temp);
        return TRANS_SUCCESS;
    }

    *p_rep_flag = RRH_MONITOR_REP_FLAG_OK;
    
    FLOG_DEBUG("*p_rep_flag = %d \r\n", *p_rep_flag);

    return TRANS_SUCCESS;
}

/*****************************************************************************+
* Function: trans_rrh_check_msg()
* Description: Check the message 
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
u_int32_t trans_rrh_check_msg(u_int8_t *p_rev_msg, 
                            struct trans_rrh_resp_msg_result   *p_resp_result,
                            u_int32_t   *p_src_moudle)
{
    struct rrh_monitor_header  *p_rrh_rev_header = NULL;
    u_int32_t uw_ret;
    u_int16_t   us_serial_number = 0;
    
    u_int8_t uc_find_flag = 0;
    u_int8_t uc_rep_flag = RRH_MONITOR_REP_FLAG_OK; 
    struct trans_timer_msg_info *p_timer_info;
    //struct trans_en_queue_msg   st_en_quene;
        
    FLOG_DEBUG("Enter \r\n");
    
    if (NULL == p_rev_msg) 
    {
        FLOG_ERROR("NULL PTR! \r\n");
        return TRANS_FAILD;  
    }
    
    p_rrh_rev_header = (struct rrh_monitor_header *)p_rev_msg;    
    us_serial_number = ntohs(p_rrh_rev_header->us_serial_number);
    
    /*find the msg, if it is not exist ,mean timeout*/
    //uw_ret = trans_timer_find_by_serial_num(us_serial_number, &st_msg_info, &uc_find_flag);
    uw_ret = trans_timer_find_by_serial_num(us_serial_number, 
                p_resp_result->p_result_msg, 
                &uc_find_flag);
    if (TRANS_SUCCESS != uw_ret)
    {
        FLOG_ERROR("Call trans_timer_find_by_serial_num error! uw_ret = %d\r\n", uw_ret);    

        /*enum trans_ack_flag_enum*/
        p_resp_result->uc_result = TRANS_ACK_FLAG_OTHER_ERR;

        return TRANS_FAILD;    
    }
    
    /*if timeout ,discard message:  do nothing*/
    if (1 != uc_find_flag)
    {
        FLOG_ERROR("Time out error! uc_find_flag = %d\r\n", uc_find_flag);

        /*enum trans_ack_flag_enum*/
        p_resp_result->uc_result = TRANS_ACK_FLAG_RRH_TIMEOUT;

        return TRANS_FAILD;    
    }

    p_timer_info = (struct trans_timer_msg_info *)p_resp_result->p_result_msg;

    /*Length of p_result_msg*/
    p_resp_result->uw_len = SIZEOF_TRANS_TIMER_MSG_INFO;

    p_resp_result->f_callback = p_timer_info->f_callback;

    *p_src_moudle = p_timer_info->uw_src_moudle;

    /*If time out ---have send the response message to src for bolck message ,
        Don't need send response message again*/
    
    /*
    FLOG_DEBUG("us_serial_number = %d, uw_src_moudle = %d, uc_block_flag = %d.\r\n", 
        p_msg_info->us_serial_number, p_msg_info->uw_src_moudle, p_msg_info->uc_block_flag);
    */
    uw_ret = trans_rrh_check_msg_content(p_rev_msg, &uc_rep_flag);
    if (TRANS_SUCCESS != uw_ret) 
    {   
        /*If error, do what????????*/
        FLOG_ERROR("Call trans_rrh_check_msg_content error! uw_ret = %d\r\n", uw_ret);

        /*enum trans_ack_flag_enum*/
        p_resp_result->uc_result = TRANS_ACK_FLAG_OTHER_ERR;
    }
    else
    {
        if (RRH_MONITOR_REP_FLAG_OK != uc_rep_flag) 
        {   
            FLOG_ERROR("Call trans_rrh_check_msg_content error! uc_rep_flag = %d\r\n", uc_rep_flag);

            /*enum trans_ack_flag_enum*/
            p_resp_result->uc_result = uc_rep_flag;

            //uw_ret = TRANS_FAILD;     
        }
        else
        {
            /*if Msg is not OK ,ERROR*/
            if (RRH_MONITOR_REP_FLAG_OK != p_rrh_rev_header->uc_resp_flag)
            {
                /*???????---- analysis the details*/
                /*enum trans_ack_flag_enum*/
                p_resp_result->uc_result = p_rrh_rev_header->uc_resp_flag;
                
                FLOG_ERROR("Resp_flag error! uc_resp_flag = %d : 0x%x%x\r\n", 
                    p_rrh_rev_header->uc_resp_flag, 
                    *(p_rev_msg + SIZEOF_RRH_MONITOR_HEADER),
                    *(p_rev_msg + SIZEOF_RRH_MONITOR_HEADER + 1));
                
                //uw_ret = TRANS_FAILD;   
            } 
            else
            {
                /*nothing*/
                /*enum trans_ack_flag_enum*/
                p_resp_result->uc_result = TRANS_ACK_FLAG_OK;
            }
        }
    }

    #if 0
    /*Rev message error & blocking message----need send response message*/
    if ((TRANS_SUCCESS != uw_ret) 
        && (TRANS_QUENE_BLOCK == p_msg_info->uc_block_flag))
    {
        FLOG_ERROR("Call trans_rrh_rev_config_process error! uw_ret = %d\r\n", uw_ret);
        
        //st_en_quene.uw_src_moudle = p_thread_info->w_rrh_sockfd;
        st_en_quene.uc_result = uw_ret;
        /*2 --CRC*/
        st_en_quene.uw_len = (p_rrh_rev_header->us_body_len) + SIZEOF_RRH_MONITOR_HEADER + 2;
        
        uw_ret = trans_msg_en_quene(p_rev_msg, &st_en_quene);
        
        if (TRANS_SUCCESS != uw_ret)
        {
            FLOG_ERROR("Call trans_msg_en_quene error! uw_ret = %d\r\n", uw_ret);
        }
        
        return TRANS_FAILD;
    }
    #endif

    FLOG_DEBUG("Exit uw_ret = %d, uc_result = %d.\r\n", uw_ret, p_resp_result->uc_result);
    
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
u_int32_t trans_rrh_rev_forward2_agent(u_int8_t *p_param_msg,
                           size_t len,
                           struct trans_rrh_resp_msg_result *p_rrh_result)
{
    u_int32_t   uw_ret;
    //u_int16_t   us_len = 0, us_body_len = 0;
    //u_int16_t   us_param_type = 0;
    //u_int8_t     uc_param_len = 0;    
    
    //struct rrh_monitor_param *p_param_temp = NULL;
    //int32_t w_value = 0;
    
    struct trans_timer_msg_info *p_timer_info = NULL;

    struct trans_send_msg_to_agent st_agent_msg_info;

    if ((NULL == p_param_msg) || (NULL == p_rrh_result) || (NULL == p_rrh_result->p_result_msg))
    {
        FLOG_ERROR("NULL PTR! \r\n");
        return TRANS_FAILD;  
    }

    p_timer_info = (struct trans_timer_msg_info *)p_rrh_result->p_result_msg;

    /*result---0--OK or 1--error  or--enum trans_ack_flag_enum*/
    st_agent_msg_info.uc_ack_flag = p_rrh_result->uc_result;  

    st_agent_msg_info.uc_block_flag = TRANS_QUENE_NO_BLOCK;
    /*Call this function when timeout or message back*/
    st_agent_msg_info.f_callback = p_rrh_result->f_callback;   
    
    /*********For metric test*********/
    st_agent_msg_info.uc_ack_flag = 0;
    /*********For metric test*********/
    
    if (TRANS_ACK_FLAG_OK != st_agent_msg_info.uc_ack_flag)
    {
        return st_agent_msg_info.uc_ack_flag;  
    }
    else
    {
        st_agent_msg_info.uw_resp_len = len;
        st_agent_msg_info.p_resp_msg = p_param_msg;
        
        st_agent_msg_info.p_reqs_msg = p_timer_info->p_user_info;

        uw_ret = trans_send_rrh_msg(TRANS_SEND_TO_AGENT, &st_agent_msg_info);
        if (TRANS_SUCCESS != uw_ret) 
        {   
            FLOG_ERROR("Call trans_send_rrh_msg error! uw_ret = %d\r\n", uw_ret);
            return TRANS_FAILD;     
        }  


        #if 0
        us_body_len = len;   
        p_param_temp = (struct rrh_monitor_param *)p_param_msg;
        
        while (us_len < us_body_len)
        {
            //us_param_type = htons(p_param_temp->us_param_type);   
            us_param_type = ntohs(p_param_temp->us_param_type); 
            uc_param_len = p_param_temp->uc_param_len;
        
            us_param_type = us_param_type&0x0fff;
            
            //FLOG_DEBUG("Rev message! us_param_type = 0x%x, uc_param_len = %d\r\n", us_param_type, uc_param_len);
        
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
      
    return TRANS_SUCCESS;     
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
u_int32_t trans_rrh_rev_forward2_monitor(u_int8_t *p_param_msg, 
                           size_t len,
                           struct trans_rrh_resp_msg_result *p_rrh_result)
{
    u_int32_t   uw_ret;
    struct trans_send_msg_to_monitor st_msg_info;
    
    if ((NULL == p_param_msg) || (NULL == p_rrh_result))
    {
        FLOG_ERROR("NULL PTR! \r\n");
        return TRANS_FAILD;  
    }

    st_msg_info.p_payload = p_param_msg;
    st_msg_info.uc_ack_flag = p_rrh_result->uc_result;
    st_msg_info.us_opration = 0;
    st_msg_info.uw_payload_len = len;

    uw_ret = trans_send_rrh_msg(TRANS_SEND_TO_MONITOR, &st_msg_info);
    if (TRANS_SUCCESS != uw_ret)
    {
        FLOG_ERROR("Call trans_send_rrh_msg error! uw_ret = %d\r\n", uw_ret);
    
        return uw_ret;    
    }
    
    return TRANS_SUCCESS;   
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
u_int32_t trans_rrh_rev_forward2_action(u_int8_t *p_param_msg, 
                           size_t len,
                           struct trans_rrh_resp_msg_result *p_rrh_result)
{
    u_int32_t   uw_ret = 0;
    u_int32_t   uw_len = 0;
    struct trans_action_info st_action_info;
    struct trans_timer_msg_info *p_timer_info = NULL;

    struct trans_resp_msg_header *p_resp_header = NULL;
    u_int8_t *p_action_msg = NULL;

    if ((NULL == p_param_msg) || (NULL == p_rrh_result) || (NULL == p_rrh_result->p_result_msg))
    {
        FLOG_ERROR("NULL PTR! \r\n");
        return TRANS_FAILD;  
    }

    FLOG_DEBUG("Enter! \r\n");

    p_timer_info = (struct trans_timer_msg_info *)p_rrh_result->p_result_msg;
    
    gettimeofday(&(st_action_info.st_tv), NULL);
    
    st_action_info.f_callback = p_rrh_result->f_callback;
    st_action_info.uw_src_moudle = TRANS_MOUDLE_ACTION;

    /*Its memory is controled by user*/
    st_action_info.p_user_info = p_timer_info->p_user_info;

    uw_len = len + SIZEOF_TRANS_RESP_MSG_HEADER;
    
    p_action_msg = (u_int8_t *)malloc(uw_len);
    if (NULL == p_action_msg)
    {
        FLOG_ERROR("Malloc p_action_msg error! \r\n");
        return TRANS_FAILD;   
    }
    
    memset((u_int8_t*)p_action_msg, 0, uw_len);
    
    p_resp_header = (struct trans_resp_msg_header *)p_action_msg;

    p_resp_header->uc_result = p_rrh_result->uc_result;
    //p_resp_header->uc_tpye = TRANS_RESP_MSG_QUERY_RRH;
    p_resp_header->uw_len = len;

    p_resp_header->p_buf = p_action_msg + SIZEOF_TRANS_RESP_MSG_HEADER;
        
    memcpy((p_action_msg + SIZEOF_TRANS_RESP_MSG_HEADER), p_param_msg, len);

    uw_ret = trans_action_add(&st_action_info, uw_len, p_action_msg);

    if (TRANS_SUCCESS != uw_ret)
    {
        FLOG_ERROR("Call trans_action_add error! uw_ret = %d\r\n", uw_ret);

        //return TRANS_FAILD;    
    }   

    free(p_action_msg);

    FLOG_DEBUG("Exit! \r\n");
    
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
u_int32_t trans_rrh_rev_forward2_bs(u_int8_t *p_param_msg, 
                           size_t len,
                           struct trans_rrh_resp_msg_result *p_rrh_result)
{
    u_int32_t   uw_ret;
    u_int16_t   us_len = 0, us_body_len = 0;
    u_int16_t   us_param_type = 0;
    u_int8_t     uc_param_len = 0;    
    
    struct rrh_monitor_param *p_param_temp = NULL;
    int32_t w_value = 0;
    char  * p_gps = NULL;
    //char a_wireless_cfg[128] = {0};

    struct trans_timer_msg_info *p_timer_info = NULL;
    struct trans_en_queue_msg   st_en_quene;

    if ((NULL == p_param_msg) || (NULL == p_rrh_result) || (NULL == p_rrh_result->p_result_msg))
    {
        FLOG_ERROR("NULL PTR! \r\n");
        return TRANS_FAILD;  
    }

    p_timer_info = (struct trans_timer_msg_info *)p_rrh_result->p_result_msg;

    if (TRANS_ACK_FLAG_OK != p_rrh_result->uc_result)
    {
        st_en_quene.uc_result = p_rrh_result->uc_result;
    }
    else
    {
        st_en_quene.uc_result = TRANS_SUCCESS;      
        
        us_body_len = len;   
        p_param_temp = (struct rrh_monitor_param *)p_param_msg;
        
        while (us_len < us_body_len)
        {
            //us_param_type = htons(p_param_temp->us_param_type);   
            us_param_type = ntohs(p_param_temp->us_param_type); 
            uc_param_len = p_param_temp->uc_param_len;
            
            //FLOG_DEBUG("Rev message! us_param_type = 0x%x, uc_param_len = %d\r\n", us_param_type, uc_param_len);
        
            switch (us_param_type)
            {
                /*Query response*/
                case RRH_MSG_LONGITUDE: 
                    
                    p_gps = (char  *)(p_param_temp + 1);
                    
                    FLOG_WARNING("Rev LONGITUDE ! %s\r\n", (p_gps));
                    
                    if (0 == strcmp((char *)(p_gps), ""))
                    {
                        uw_ret = set_global_param("RRU_LONGITUDE", (void *)(RRH_LONGITUDE));
                        if (0 != uw_ret)
                        {
                            FLOG_ERROR("Call set_global_param for RRU_LONGITUDE");
                            st_en_quene.uc_result = TRANS_FAILD;
                        }
                
                        FLOG_WARNING("1 Set LONGITUDE ! %s\r\n", (RRH_LONGITUDE));
                    }
                    else
                    {
                        uw_ret = set_global_param("RRU_LONGITUDE", (void *)(p_gps + 1));
                        if (0 != uw_ret)
                        {
                            FLOG_ERROR("Call set_global_param for RRU_LONGITUDE");
                            st_en_quene.uc_result = TRANS_FAILD;
                        }
                
                        FLOG_WARNING("2 Set LONGITUDE ! %s\r\n", (p_gps + 1));
                    }
                    
                    break; 
                    
                case RRH_MSG_LATITUDE: 
                    
                    p_gps = (char  *)(p_param_temp + 1);
                    
                    FLOG_WARNING("Rev LATITUDE ! %s\r\n", (p_gps));
                    
                    if (0 == strcmp((char *)(p_gps), ""))
                    {
                        uw_ret = set_global_param("RRU_LATITUDE", (void *)(RRH_LATITUDE));
                        if (0 != uw_ret)
                        {
                            FLOG_ERROR("Call set_global_param for RRU_LATITUDE");
                            st_en_quene.uc_result = TRANS_FAILD;
                        }
                    
                        FLOG_WARNING("1 Set LATITUDE ! %s\r\n", (RRH_LATITUDE));
                    }
                    else
                    {
                        uw_ret = set_global_param("RRU_LATITUDE", (void *)(p_gps + 1));
                        if (0 != uw_ret)
                        {
                            FLOG_ERROR("Call set_global_param for RRU_LATITUDE");
                            st_en_quene.uc_result = TRANS_FAILD;
                        }
                        FLOG_WARNING("2 Set LATITUDE ! %s\r\n", (p_gps + 1));
                    }
                
                    break; 

                    
                case RRH_MSG_GPS_CLK_LOCK_VALUE: 
                    w_value = *((u_int8_t *)(p_param_temp + 1));
                    
                    st_en_quene.uc_result = w_value;
                    
                    FLOG_WARNING("Rev GPS_CLK_LOCK! %d\r\n", w_value);
                break; 
        
                case RRH_MSG_CHAN1_NORM_POW_VALUE:
                    w_value = ntohl(*((int32_t *)(p_param_temp + 1)));
                    FLOG_WARNING("CALIBRATE_DIGITAL_POWER0 %d\r\n", w_value);
        
                    #if 0
                    uw_ret = set_global_param("CALIBRATE_DIGITAL_POWER0", (void *)&(w_value));
                    if (0 != uw_ret)
                    {
                        FLOG_ERROR("Call set_global_param for DIGITAL_POWER0 error");
                        st_en_quene.uc_result = TRANS_FAILD;
                    }
                    #endif
        
                    break;  
        
                case RRH_MSG_CHAN2_NORM_POW_VALUE: 
                    
                    w_value = ntohl(*((int32_t *)(p_param_temp + 1)));
                    FLOG_WARNING("CALIBRATE_DIGITAL_POWER1 %d\r\n", w_value);
        
                    #if 0
                    uw_ret = set_global_param("CALIBRATE_DIGITAL_POWER1", (void *)&(w_value));
                    if (0 != uw_ret)
                    {
                        FLOG_ERROR("Call set_global_param for DIGITAL_POWER1 error");
                        st_en_quene.uc_result = TRANS_FAILD;
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
                        st_en_quene.uc_result = TRANS_FAILD;
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
                        st_en_quene.uc_result = TRANS_FAILD;
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

                case RRH_MSG_TX_LEN: /*PA Switch for Channel 1*/
                case RRH_MSG_RX_LEN: /*PA Switch for Channel 2*/
                
                case RRH_MSG_NORM_OUTPUT_POWER:        /*Config GPS Enable*/

                case RRH_MSG_AGC_ENABLE:

                case RRH_MSG_CHAN1_RX_PGC:
                case RRH_MSG_CHAN2_RX_PGC:

                    
                    st_en_quene.uc_result = TRANS_SUCCESS;

                    FLOG_INFO("Config RRH OK: 0x%x \r\n", us_param_type);
                    break;    
                    
                default:
                    
                    FLOG_ERROR("Rev unknow message! us_param_type = 0x%x\r\n", us_param_type);
        
                    /*????????????????*/
                    return TRANS_FAILD;
            
            }  
        
            if ((TRANS_SUCCESS != st_en_quene.uc_result) 
                && (TRANS_QUENE_BLOCK == p_timer_info->uc_block_flag))
            {
                break;
            }
        
            us_len = us_len + uc_param_len +SIZEOF_RRH_MONITOR_PARAM;         
            p_param_temp = (struct rrh_monitor_param *)(p_param_msg + us_len);
        
        }   

    }

    /*Block message -- send response message*/
    if (TRANS_QUENE_BLOCK == p_timer_info->uc_block_flag)
    {
        /*2 --CRC*/
        st_en_quene.uw_src_moudle = TRANS_MOUDLE_RRH;
        //st_en_quene.uc_result = TRANS_SUCCESS;
        st_en_quene.uw_len = us_body_len;
        
        uw_ret = trans_msg_en_quene(p_param_msg, &st_en_quene);
        
        if (TRANS_SUCCESS != uw_ret)
        {
            FLOG_ERROR("Call trans_msg_en_quene error! uw_ret = %d\r\n", uw_ret);
            return TRANS_FAILD;
        }
    }

//    uw_ret = st_en_quene.uc_result;

    FLOG_DEBUG("Exit uw_ret = %d.\r\n", uw_ret);

    return TRANS_SUCCESS;
}

/*****************************************************************************+
* Function: trans_rrh_rev_alarm_process()
* Description: Process the alarm message from the RRH
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
u_int32_t trans_rrh_rev_alarm_process(u_int8_t *p_rev_msg, u_int8_t *p_send_msg, 
                           struct trans_thread_info *p_thread_info)
{
    //struct rrh_monitor_header  *p_rrh_send_header = NULL;
    struct rrh_monitor_header  *p_rrh_rev_header = NULL;
    u_int32_t   uw_ret = 0;
    u_int16_t   us_body_len = 0, us_len =0;

    u_int8_t   uc_rep_flag = 0;
    u_int8_t *p_param_alarm = NULL;
    struct rrh_monitor_param_alarm *p_alarm_temp = NULL;
    u_int16_t   us_alarm_type = 0;
    
    struct trans_send_msg_to_agent st_msg_info;
    struct trans_agent_alert_info        st_alert_info;
        
    FLOG_DEBUG("Enter \r\n");
    
    if ((NULL == p_rev_msg) || (NULL == p_send_msg) || (NULL == p_thread_info))
    {
        FLOG_ERROR("NULL PTR! \r\n");
        return TRANS_FAILD;  
    }
    
    p_rrh_rev_header = (struct rrh_monitor_header *)p_rev_msg;
    p_param_alarm = p_rev_msg + SIZEOF_RRH_MONITOR_HEADER;

    us_body_len = ntohs(p_rrh_rev_header->us_body_len);    

    uw_ret = trans_rrh_check_msg_content(p_rev_msg, &uc_rep_flag);
    if (TRANS_SUCCESS != uw_ret) 
    {   
        FLOG_ERROR("Call trans_rrh_check_msg_content error! uw_ret = %d\r\n", uw_ret);
        return TRANS_FAILD;     
    }  

    if (RRH_MONITOR_REP_FLAG_OK == uc_rep_flag)
    {
        /*Analysis the param*/
        while (1)
        {
            p_alarm_temp = (struct rrh_monitor_param_alarm *)p_param_alarm;
    
            us_alarm_type = ntohs(p_alarm_temp->us_param_type); 
            
            p_param_alarm = p_rev_msg + SIZEOF_RRH_MONITOR_HEADER + us_len;

            FLOG_WARNING("Recive RRH alarm. Type:0x%x, Value: %d  \r\n", us_alarm_type, p_alarm_temp->uc_param_value);
            

            if (us_len >= us_body_len)
            {
                break;
            }
    
            /*Param Length error*/
            if (1 != p_alarm_temp->uc_param_len)
            {
                uc_rep_flag = RRH_MONITOR_REP_FLAG_OTHER_ERR;
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
                uc_rep_flag = RRH_MONITOR_REP_FLAG_OTHER_ERR;                
                p_alarm_temp->us_param_type = (p_alarm_temp->us_param_type|RRH_MONITOR_REP_INVALID_PARAM);
                
                FLOG_ERROR("Param value error!uc_param_value = %d\r\n", p_alarm_temp->uc_param_value);
                //FLOG_INFO("us_param_type = %d \r\n", p_alarm_temp->us_param_type);
                break;    
            }
       
            FLOG_DEBUG("Send alarm 0x%x to Agent \r\n", us_alarm_type);
    
            /*Send alert to Agent*/
            st_msg_info.f_callback = NULL;
            st_msg_info.uc_block_flag = TRANS_QUENE_NO_BLOCK;
            st_msg_info.uc_ack_flag = TRANS_ACK_FLAG_OK;
            st_msg_info.p_reqs_msg = "alert";
    
            st_alert_info.us_alarm_id = us_alarm_type;
            st_alert_info.w_alarm_value = p_alarm_temp->uc_param_value;
    
            st_msg_info.uw_resp_len = SIZEOF_TRANS_AGENT_ALERT_INFO;
            st_msg_info.p_resp_msg = &st_alert_info;
                
            uw_ret = trans_send_rrh_msg(TRANS_SEND_TO_AGENT, &st_msg_info);
            /*Error*/
            if (TRANS_SUCCESS != uw_ret)
            {
                FLOG_ERROR("Call trans_send_rrh_msg error! uw_ret = %d\r\n", uw_ret);
                return TRANS_FAILD;
            }

            us_len = us_len + SIZEOF_RRH_MONITOR_PARAM_ALARM;  
    
        } 
    }

    /*Send alert ack to RRH*/           
    //#if 0

    FLOG_WARNING("Send alarm ACK 0x%x to RRH \r\n", us_alarm_type);
   
    u_int16_t   *p_new_crc = NULL;
    u_int16_t   us_crc_val = 0;
    int32_t       w_ret = 0;
    int32_t       w_send_len = 0;
    
    p_rrh_rev_header->uc_resp_flag = uc_rep_flag;
    memset(p_rrh_rev_header->a_time, 0, 7);
        
    /*Build Msg CRC */    /*calculate CRC*/
    us_crc_val = trans_rrh_cal_crc16((u_int8_t*)p_rev_msg, us_body_len + SIZEOF_RRH_MONITOR_HEADER); 
    
    p_new_crc = (u_int16_t  *)(p_rev_msg + us_body_len + SIZEOF_RRH_MONITOR_HEADER);
    *p_new_crc = htons(us_crc_val);

    w_send_len = us_body_len + SIZEOF_RRH_MONITOR_HEADER + 2;
    
    trans_debug_msg_print(p_rev_msg, w_send_len, g_trans_debug_rrh);    
    
    w_ret = send(p_thread_info->w_rrh_sockfd, p_rev_msg, w_send_len, 0);
    
    if(w_ret <= 0)
    {
        //close(sock);
        FLOG_ERROR("send error! w_ret = %d\r\n", w_ret);
        return TRANS_FAILD;
    }     
    //#endif

  
    #if 0
    if (RRH_MONITOR_REP_FLAG_OK == uc_rep_flag)
    {
        /*Analysis the param*/
        while (us_len < us_body_len)
        {
            p_alarm_temp = (struct rrh_monitor_param_alarm *)p_param_alarm;

            us_alarm_type = ntohs(p_alarm_temp->us_param_type); 
            
            us_len = us_len + SIZEOF_RRH_MONITOR_PARAM_ALARM;  
            p_param_alarm = p_rev_msg + SIZEOF_RRH_MONITOR_HEADER + us_len;

            /*Param Length error*/
            if (1 != p_alarm_temp->uc_param_len)
            {
                //uc_rep_flag = RRH_MONITOR_REP_FLAG_P_ERR;
                //p_alarm_temp->us_param_type = (p_alarm_temp->us_param_type|RRH_MONITOR_REP_ERR_LEN_ILLEGAL);
                
                FLOG_ERROR("Param length error! uc_param_len = %d\r\n", p_alarm_temp->uc_param_len);
                //FLOG_INFO("us_param_type = %d \r\n", p_alarm_temp->us_param_type);
                continue;                     
            }

            /*Param Type error*/
            if ((RRH_MSG_POWER_FAULT > us_alarm_type)
                || (RRH_MSG_DL_VSWR2 < us_alarm_type))
            {
                //uc_rep_flag = RRH_MONITOR_REP_FLAG_P_ERR;                
                //p_alarm_temp->us_param_type = (p_alarm_temp->us_param_type|RRH_MONITOR_REP_ERR_IDENTIFY_WRY);
                
                FLOG_ERROR("Param type error!us_param_type = %d\r\n", us_alarm_type);
                //FLOG_INFO("us_param_type = %d \r\n", p_alarm_temp->us_param_type);
                continue;    
            }

            /*Param Value error*/
            if (RRH_MONITOR_AlARM_CLOSE < p_alarm_temp->uc_param_value)
            {
                //uc_rep_flag = RRH_MONITOR_REP_FLAG_P_ERR;                
                //p_alarm_temp->us_param_type = (p_alarm_temp->us_param_type|RRH_MONITOR_REP_INVALID_PARAM);
                
                FLOG_ERROR("Param value error!uc_param_value = %d\r\n", p_alarm_temp->uc_param_value);
                //FLOG_INFO("us_param_type = %d \r\n", p_alarm_temp->us_param_type);
                continue;    
            }
       
            FLOG_WARNING("Send alarm 0x%x to Agent \r\n", us_alarm_type);

            /*Send alert to Agent*/
            st_msg_info.f_callback = NULL;
            st_msg_info.uc_block_flag = TRANS_QUENE_NO_BLOCK;
            st_msg_info.uc_ack_flag = TRANS_ACK_FLAG_OK;
            st_msg_info.p_reqs_msg = "alert";

            st_alert_info.us_alarm_id = us_alarm_type;
            st_alert_info.w_alarm_value = p_alarm_temp->uc_param_value;

            st_msg_info.uw_resp_len = SIZEOF_TRANS_AGENT_ALERT_INFO;
            st_msg_info.p_resp_msg = &st_alert_info;
                
            uw_ret = trans_send_rrh_msg(TRANS_SEND_TO_AGENT, &st_msg_info);
            /*Error*/
            if (TRANS_SUCCESS != uw_ret)
            {
                FLOG_ERROR("Call trans_send_rrh_msg error! uw_ret = %d\r\n", uw_ret);
                return TRANS_FAILD;
            }

        } 
    }
    #endif

    FLOG_DEBUG("Exit \r\n");

    return TRANS_SUCCESS;
}


/*****************************************************************************+
* Function: trans_rrh_rev_query_process()
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
u_int32_t trans_rrh_rev_query_process(u_int8_t *p_rev_msg, u_int8_t *p_send_msg, 
                           struct trans_thread_info *p_thread_info)
{
    struct rrh_monitor_header  *p_rrh_rev_header = NULL;
    u_int32_t   uw_ret;
    u_int16_t   us_body_len = 0;
    u_int32_t   uw_src_moudle = TRANS_MOUDLE_BUF;
    
    u_int8_t *p_param_quary = NULL;
    struct trans_rrh_resp_msg_result   st_rrh_result;
    
    FLOG_DEBUG("Enter \r\n");
    
    if ((NULL == p_rev_msg) || (NULL == p_send_msg) || (NULL == p_thread_info))
    {
        FLOG_ERROR("NULL PTR! \r\n");
        return TRANS_FAILD;  
    }
    
    /*enum trans_ack_flag_enum*/
    st_rrh_result.uc_result = TRANS_ACK_FLAG_OK;
    st_rrh_result.f_callback = NULL;
    st_rrh_result.uw_len = 0;
    st_rrh_result.p_result_msg = p_send_msg;
    
    p_rrh_rev_header = (struct rrh_monitor_header *)p_rev_msg;    
    
    uw_ret = trans_rrh_check_msg(p_rev_msg, &st_rrh_result, &uw_src_moudle);
    if (TRANS_SUCCESS != uw_ret)
    {
        FLOG_ERROR("Call trans_rrh_check_msg error! uw_ret = %d\r\n", uw_ret);
    
        return uw_ret;    
    }
        
    us_body_len = ntohs(p_rrh_rev_header->us_body_len);   
    p_param_quary = p_rev_msg + SIZEOF_RRH_MONITOR_HEADER; 

    switch (uw_src_moudle)
    {
        /*Query Message From AGENT */   
        case TRANS_MOUDLE_AGENT: 
            uw_ret = trans_rrh_rev_forward2_agent(p_param_quary, us_body_len, &(st_rrh_result));
            if (TRANS_SUCCESS != uw_ret)
            {
                FLOG_ERROR("Call trans_rrh_rev_forward2_agent error! uw_ret = %d\r\n", uw_ret);
            
                //return uw_ret;    
            }
            
            break;   

        /*Query Message From MONITOR */   
        case TRANS_MOUDLE_MONITOR: 
            uw_ret = trans_rrh_rev_forward2_monitor(p_param_quary, us_body_len, &(st_rrh_result));
            if (TRANS_SUCCESS != uw_ret)
            {
                FLOG_ERROR("Call trans_rrh_rev_forward2_monitor error! uw_ret = %d\r\n", uw_ret);
            
                //return uw_ret;    
            }

            break;   
            
        /*Query Message From ACTION */   
        case TRANS_MOUDLE_ACTION: 
            uw_ret = trans_rrh_rev_forward2_action(p_param_quary, us_body_len, &(st_rrh_result));
            if (TRANS_SUCCESS != uw_ret)
            {
                FLOG_ERROR("Call trans_rrh_rev_forward2_action error! uw_ret = %d\r\n", uw_ret);
            
                //return uw_ret;    
            }

            break;   

        /*Query Message From WIRELESS */   
        case TRANS_MOUDLE_BS: 

            uw_ret = trans_rrh_rev_forward2_bs(p_param_quary, us_body_len, &(st_rrh_result));
            if (TRANS_SUCCESS != uw_ret)
            {
                FLOG_ERROR("Call trans_rrh_rev_forward2_bs error! uw_ret = %d\r\n", uw_ret);
            
                //return uw_ret;    
            }
            
            break;   
            
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


    #if 0
    p_param_temp = (struct rrh_monitor_param *)p_param_quary;
    
    while (us_len < us_body_len)
    {
        //us_param_type = htons(p_param_temp->us_param_type);   
        us_param_type = ntohs(p_param_temp->us_param_type); 
        uc_param_len = p_param_temp->uc_param_len;
    
        us_param_type = us_param_type&0x0fff;
        
        //FLOG_DEBUG("Rev message! us_param_type = 0x%x, uc_param_len = %d\r\n", us_param_type, uc_param_len);
    
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
 
                uw_ret = trans_rrh_forward_rrh_metric(p_send_msg, p_thread_info->w_agent_sockfd,
                                        &(st_msg_info.u_extra_info.st_agent_metric),us_param_type, w_value);  
                if (TRANS_SUCCESS != uw_ret) 
                {   
                    FLOG_ERROR("2 Call trans_rrh_forward_rrh_metric error! uw_ret = %d\r\n", uw_ret);
                    return TRANS_FAILD;     
                }  

                break;    
                
            case RRH_MSG_LONGITUDE: 
            case RRH_MSG_LATITUDE: 
                
                FLOG_INFO("Rev LONGITUDE/LATITUDE message! %s\r\n", (p_param_temp + 1));
                #if 0
                w_ret = set_global_param("DEFAULT_WORKING_FREQ", (void *)&(w_value));
                if (0 != w_ret)
                {
                    FLOG_ERROR("Call set_global_param for FREQ error");
                    st_en_quene.uc_result = TRANS_FAILD;
                    uw_ret = TRANS_FAILD;
                }
                #endif

                break; 
                
            case RRH_MSG_GPS_CLK_LOCK_VALUE: 
                w_value = *((u_int8_t *)(p_param_temp + 1));

                FLOG_INFO("Rev GPS_CLK_LOCK! %d\r\n", w_value);
                #if 0
                w_ret = set_global_param("DEFAULT_WORKING_FREQ", (void *)&(w_value));
                if (0 != w_ret)
                {
                    FLOG_ERROR("Call set_global_param for FREQ error");
                    st_en_quene.uc_result = TRANS_FAILD;
                    uw_ret = TRANS_FAILD;
                }
                #endif
            break; 

            case RRH_MSG_CHAN1_NORM_POW_VALUE:
                w_value = ntohl(*((int32_t *)(p_param_temp + 1)));
                FLOG_INFO("CALIBRATE_DIGITAL_POWER0 %d\r\n", w_value);

                #if 0
                uw_ret = set_global_param("CALIBRATE_DIGITAL_POWER0", (void *)&(w_value));
                if (0 != uw_ret)
                {
                    FLOG_ERROR("Call set_global_param for DIGITAL_POWER0 error");
                    st_en_quene.uc_result = TRANS_FAILD;
                }
                #endif

                break;  

            case RRH_MSG_CHAN2_NORM_POW_VALUE: 
                
                w_value = ntohl(*((int32_t *)(p_param_temp + 1)));
                FLOG_INFO("CALIBRATE_DIGITAL_POWER1 %d\r\n", w_value);

                #if 0
                uw_ret = set_global_param("CALIBRATE_DIGITAL_POWER1", (void *)&(w_value));
                if (0 != uw_ret)
                {
                    FLOG_ERROR("Call set_global_param for DIGITAL_POWER1 error");
                    st_en_quene.uc_result = TRANS_FAILD;
                }
                #endif

                break;  
                
            default:
                
                FLOG_ERROR("Rev unknow message! us_param_type = 0x%x\r\n", us_param_type);
    
                /*????????????????*/
                break;
        
        }  

        if ((TRANS_SUCCESS != st_en_quene.uc_result) 
            && (TRANS_QUENE_BLOCK == st_msg_info.uc_block_flag))
        {
            break;
        }
    
        us_len = us_len + uc_param_len +SIZEOF_RRH_MONITOR_PARAM;         
        p_param_temp = (struct rrh_monitor_param *)(p_rev_msg + SIZEOF_RRH_MONITOR_HEADER + us_len);
    
    }

    /*Block message -- send response message*/
    if (TRANS_QUENE_BLOCK == st_msg_info.uc_block_flag)
    {
        /*2 --CRC*/
        st_en_quene.uw_src_moudle = p_thread_info->w_rrh_sockfd;
        //st_en_quene.uc_result = TRANS_SUCCESS;
        st_en_quene.uw_len = us_body_len + SIZEOF_RRH_MONITOR_HEADER + 2;
        
        uw_ret = trans_msg_en_quene(p_rev_msg, &st_en_quene);
        
        if (TRANS_SUCCESS != uw_ret)
        {
            FLOG_ERROR("Call trans_msg_en_quene error! uw_ret = %d\r\n", uw_ret);
            return TRANS_FAILD;
        }
    }

    uw_ret = st_en_quene.uc_result;
    #endif

    FLOG_DEBUG("Exit \r\n");
    
    return uw_ret;  

}

/*****************************************************************************+
* Function: trans_rrh_rev_config_process()
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
u_int32_t trans_rrh_rev_config_process(u_int8_t *p_rev_msg, u_int8_t *p_send_msg, 
                           struct trans_thread_info *p_thread_info)
{
    struct rrh_monitor_header  *p_rrh_rev_header = NULL;
    u_int32_t uw_ret;
    u_int16_t   us_body_len = 0;
    u_int32_t   uw_src_moudle = TRANS_MOUDLE_BUF;

    u_int8_t *p_param_config = NULL;
    struct trans_rrh_resp_msg_result   st_rrh_result;
    
    FLOG_DEBUG("Enter \r\n");
    
    if ((NULL == p_rev_msg) || (NULL == p_send_msg) || (NULL == p_thread_info))
    {
        FLOG_ERROR("NULL PTR! \r\n");
        return TRANS_FAILD;  
    }
    
    p_rrh_rev_header = (struct rrh_monitor_header *)p_rev_msg;    

    /*enum trans_ack_flag_enum*/
    st_rrh_result.uc_result = TRANS_ACK_FLAG_OK;
    st_rrh_result.f_callback = NULL;
    st_rrh_result.uw_len = 0;
    st_rrh_result.p_result_msg = p_send_msg;

    uw_ret = trans_rrh_check_msg(p_rev_msg, &st_rrh_result, &uw_src_moudle);
    if (TRANS_SUCCESS != uw_ret)
    {
        FLOG_ERROR("Call trans_rrh_check_msg error! uw_ret = %d\r\n", uw_ret);
    
        return uw_ret;    
    }

    us_body_len = ntohs(p_rrh_rev_header->us_body_len);   
    p_param_config = p_rev_msg + SIZEOF_RRH_MONITOR_HEADER; 
    
    switch (uw_src_moudle)
    {
        /*Config message From ACTION */   
        case TRANS_MOUDLE_ACTION: 
            uw_ret = trans_rrh_rev_forward2_action(p_param_config, us_body_len, &(st_rrh_result));
            if (TRANS_SUCCESS != uw_ret)
            {
                FLOG_ERROR("Call trans_rrh_rev_forward2_action error! uw_ret = %d\r\n", uw_ret);
            
                //return uw_ret;    
            }
    
            break;   
    
        /*Config Message From WIRELESS */   
        case TRANS_MOUDLE_BS: 
    
            uw_ret = trans_rrh_rev_forward2_bs(p_param_config, us_body_len, &(st_rrh_result));
            if (TRANS_SUCCESS != uw_ret)
            {
                FLOG_ERROR("Call trans_rrh_rev_forward2_bs error! uw_ret = %d\r\n", uw_ret);
            
                //return uw_ret;    
            }
            
            break;   
            
        /*Config Message From MONITOR */   
        case TRANS_MOUDLE_MONITOR: 
            uw_ret = trans_rrh_rev_forward2_monitor(p_param_config, us_body_len, &(st_rrh_result));
            if (TRANS_SUCCESS != uw_ret)
            {
                FLOG_ERROR("Call trans_rrh_rev_forward2_monitor error! uw_ret = %d\r\n", uw_ret);
            
                //return uw_ret;    
            }
        
            break;   

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
    
    #if 0
    us_body_len = ntohs(p_rrh_rev_header->us_body_len);   
    p_param_temp = (struct rrh_monitor_param *)p_param_config;
    
    while (us_len < us_body_len)
    {
        us_param_type = ntohs(p_param_temp->us_param_type);        
        uc_param_len = p_param_temp->uc_param_len;

        //FLOG_DEBUG("Rev message! us_param_type = 0x%x, uc_param_len = %d\r\n", us_param_type, uc_param_len);

        switch (us_param_type)
        {
            case RRH_MSG_PA_SWITCH1_CFG: /* Channel 1 channel enable switch */
            case RRH_MSG_PA_SWITCH2_CFG: /* Channel 2 channel enable switch*/

            case RRH_MSG_CHAN1_FREQ_CFG: /* Channel 1 working frequency */
            case RRH_MSG_CHAN2_FREQ_CFG: /* Channel 2 working frequency */  

            case RRH_MSG_CHAN1_WORKMODE_CFG: /* Channel 1 working mode */
            case RRH_MSG_CHAN2_WORKMODE_CFG: /* Channel 2 working mode */

            case RRH_MSG_DL_PRESEND_TIME_CFG: /* Downlink send lead time   */
              
                break;      
        
            default:
                
                FLOG_ERROR("Rev no-blocking message! us_param_type = 0x%x\r\n", us_param_type);

                /*????????????????*/
                break;
        
        }  

        us_len = us_len + uc_param_len +SIZEOF_RRH_MONITOR_PARAM;         
        p_param_temp = (struct rrh_monitor_param *)(p_rev_msg + SIZEOF_RRH_MONITOR_HEADER + us_len);

    }
    
    /*Block message -- send response message*/
    if (TRANS_QUENE_BLOCK == st_msg_info.uc_block_flag)
    {
        /*2 --CRC*/
        st_en_quene.uw_src_moudle = p_thread_info->w_rrh_sockfd;
        st_en_quene.uc_result = TRANS_SUCCESS;
        st_en_quene.uw_len = us_body_len + SIZEOF_RRH_MONITOR_HEADER + 2;
        
        uw_ret = trans_msg_en_quene(p_rev_msg, &st_en_quene);
        
        if (TRANS_SUCCESS != uw_ret)
        {
            FLOG_ERROR("Call trans_msg_en_quene error! uw_ret = %d\r\n", uw_ret);
            return TRANS_FAILD;
        }
    }
    #endif
    
    FLOG_DEBUG("Exit \r\n");
    
    return TRANS_SUCCESS;  
}

/*****************************************************************************+
* Function: trans_rrh_rev_heartbeat_process()
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
void trans_rrh_rev_heartbeat_process(void )
{

    g_trans_rrh_heartbeat_num = 0;

    return; 
}

/*****************************************************************************+
* Function: trans_rrh_rev_msg_process()
* Description: Process the message from the RRH
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
u_int32_t trans_rrh_rev_msg_process(u_int8_t *p_rev_msg, u_int8_t *p_send_msg, 
                           struct trans_thread_info *p_thread_info)
{
    struct rrh_monitor_header  *p_rrh_header = NULL;

    u_int8_t uc_type = 0;
    u_int32_t uw_ret = 0;
    
    //struct trans_en_queue_msg   st_en_quene;
    //u_int16_t   us_body_len = 0;
    //u_int32_t   uw_tot_len = 0;
    //u_int16_t   us_crc_val = 0, us_crc_temp = 0;

    FLOG_DEBUG("Enter \r\n");
    
    if ((NULL == p_rev_msg) || (NULL == p_send_msg) || (NULL == p_thread_info))
    {
        FLOG_ERROR("NULL PTR! \r\n");
        return TRANS_FAILD;  
    }
    
    p_rrh_header = (struct rrh_monitor_header *)p_rev_msg;

    uc_type = p_rrh_header->uc_type;

    switch (uc_type)
    {
        case RRH_MONITOR_TYPE_ALARM:
    
            /*Alarm process：replay or send msg to： RRH和C&M Server */
            uw_ret = trans_rrh_rev_alarm_process(p_rev_msg, p_send_msg, p_thread_info);
    
            if (TRANS_SUCCESS != uw_ret)
            {
                FLOG_ERROR("Call trans_rrh_rev_alarm_process error! uw_ret = %d\r\n", uw_ret);

            }
                            
            break;
    
        case RRH_MONITOR_TYPE_QUERY:
         
            /*Query Process：send msg to： .....*/    
            uw_ret = trans_rrh_rev_query_process(p_rev_msg, p_send_msg, p_thread_info);
    
            if (TRANS_SUCCESS != uw_ret)
            {
                FLOG_ERROR("Call trans_rrh_rev_query_process error! uw_ret = %d\r\n", uw_ret);

            }
            break;
    
        case RRH_MONITOR_TYPE_CONFIG:
    
            /*Config process：send msg to： .....*/
            uw_ret = trans_rrh_rev_config_process(p_rev_msg, p_send_msg, p_thread_info);
            if (TRANS_SUCCESS != uw_ret)
            {
                #if 0
                FLOG_ERROR("Call trans_rrh_rev_config_process error! uw_ret = %d\r\n", uw_ret);

                /*2 --CRC*/
                uw_tot_len = (p_rrh_header->us_body_len) + SIZEOF_RRH_MONITOR_HEADER + 2;
                st_en_quene.uw_src_moudle = p_thread_info->w_rrh_sockfd;
                st_en_quene.uc_result = uw_ret;
                st_en_quene.uw_len = uw_tot_len;

                uw_ret = trans_msg_en_quene(p_rev_msg, &st_en_quene);

                if (TRANS_SUCCESS != uw_ret)
                {
                    FLOG_ERROR("Call trans_msg_en_quene error! uw_ret = %d\r\n", uw_ret);
                }

                uw_ret = TRANS_FAILD;
                #endif
            }
            
            break;
    
        case RRH_MONITOR_TYPE_HEARTBRAT:
    
            /*Heart beat process： No replay*/
            trans_rrh_rev_heartbeat_process();
            
            break;    
    
        default:
    
            FLOG_ERROR("Rev message type error! uc_type = %d\r\n", uc_type);
            uw_ret = TRANS_FAILD;
    
    }

    FLOG_DEBUG("Exit uw_ret =%d.\r\n", uw_ret);

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
u_int32_t trans_rrh_rev_msg(u_int8_t *p_rev_msg, int32_t w_rrh_socket)
{
    int32_t w_rev_len1 = 0, w_rev_len2 = 0;
    int32_t w_rev_len = 0;    
    int w_len_tmp = 0;
    int32_t w_param_len = 0;
    int32_t w_msg_len = 0;
        
    /*First rev the len : 2 Bytes*/
    w_len_tmp = 2;

    while (w_len_tmp)
    {
        /*Then rev a totle message except the len*/
        w_rev_len1 = recv(w_rrh_socket, 
                                p_rev_msg + (2 - w_len_tmp),  
                                w_len_tmp, 
                                0);
        /*Error*/
        if (w_rev_len1 <= 0)
        {
            FLOG_ERROR("Rev complete w_rev_len1 =%d. \r\n", w_rev_len1);
            return TRANS_FAILD;
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
    w_rev_len1 = recv(w_rrh_socket, 
                            p_rev_msg, 
                            2, 
                            0);
    
    if (w_rev_len1 <= 0)
    {
        FLOG_ERROR("Rev complete w_rev_len1 =%d. \r\n", w_rev_len1);
        return TRANS_FAILD;
    }
    #endif
    
    w_rev_len = 0;
    w_len_tmp = 0;
    
    /*Get the len*/
    w_param_len = ntohs(*((u_int16_t*)(p_rev_msg)));

    /*-2 : len    ;    +2 :  CRC*/
    w_msg_len = SIZEOF_RRH_MONITOR_HEADER -2 + w_param_len + 2;
    
    if ((w_msg_len + w_rev_len1) > TRANS_REV_MSG_MAX_LEN - 2)
    {
        FLOG_ERROR("Rev rrh msg length error! msg_len = %d, w_rev_len1 = %d\r\n", w_msg_len, w_rev_len1);
        return TRANS_FAILD;
    }
    
    /*Then rev a totle message except the len*/
    w_len_tmp = w_msg_len;
    
    while (w_len_tmp)
    {
        /*Then rev a totle message except the len*/
        w_rev_len2 = recv(w_rrh_socket, 
                                p_rev_msg + (w_msg_len - w_len_tmp) + 2,  
                                w_len_tmp, 
                                0);
        /*Error*/
        if (w_rev_len2 <= 0)
        {
            FLOG_ERROR("Receivev RRH message error! w_rev_len2 = %d\r\n", w_rev_len2);
            return TRANS_FAILD;
        }
    
        w_len_tmp = w_len_tmp - w_rev_len2;    
        w_rev_len = w_rev_len + w_rev_len2;
    }
    
    if (w_msg_len != w_rev_len)
    {
        FLOG_ERROR("Receivev RRH Message Length error! msg_len  = %d, rev_len  = %d\r\n", w_msg_len, w_rev_len);
        return TRANS_FAILD;
    }
    
    #if 0
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
    
    FLOG_DEBUG("Rev RRH Msg OK len = %d. \r\n", w_rev_len+w_rev_len1);

    trans_debug_msg_print(p_rev_msg, 40, g_trans_debug_rrh);

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
u_int32_t trans_rrh_build_heard(struct trans_rrh_build_msg_info *p_build_info, 
                                    u_int8_t *p_send_head, u_int32_t uw_body_len, u_int16_t *p_serial_number)
{
    struct rrh_monitor_header  *p_rrh_header = NULL;
    u_int16_t us_body_len = 0; 

    u_int16_t us_serial_number = 0;

    FLOG_DEBUG("Enter \r\n");
    
    if ((NULL == p_send_head) || (NULL == p_serial_number))
    {
        FLOG_ERROR("NULL PTR! \r\n");
        return TRANS_FAILD;      
    }  
    
    us_body_len = (u_int16_t)uw_body_len;

    p_rrh_header = (struct rrh_monitor_header *)p_send_head;
    
    p_rrh_header->us_body_len = htons(us_body_len);
    p_rrh_header->uc_server_id = g_trans_rrh_eqp_config.uc_server_id;
    p_rrh_header->uw_rru_id = htonl(g_trans_rrh_eqp_config.uw_rru_id);

    p_rrh_header->uc_type = p_build_info->uc_type;
    p_rrh_header->uc_resp_flag = RRH_MONITOR_REP_FLAG_ORDER;

    /*Get Time; time is zero when Server send msg to RRH*/

    /*Get serial_number*/
    us_serial_number = trans_rrh_cal_serial_num(); 
    p_rrh_header->us_serial_number = htons(us_serial_number);

    *p_serial_number = us_serial_number;
    
    FLOG_DEBUG("Exit \r\n"); 
    
    return TRANS_SUCCESS;
}


/*****************************************************************************+
* Function: trans_rrh_build_body()
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
u_int32_t trans_rrh_build_body(struct trans_rrh_build_msg_info *p_build_info, u_int32_t uw_param_num,
                                    u_int8_t *p_send_body, u_int32_t *p_body_len)
{
    u_int32_t uw_num = uw_param_num;
    struct trans_rrh_build_msg_param *p_build = NULL;
    u_int32_t uw_len_temp = 0;

    u_int32_t uw_value32 = 0;
    u_int16_t us_value16 = 0;

    u_int8_t *p_temp1 = NULL;
    u_int8_t *p_temp2 = NULL;

    FLOG_DEBUG("Enter \r\n");

    struct rrh_monitor_param *p_send_temp = NULL;

    if ((NULL == p_build_info) ||(NULL == p_send_body) || (NULL == p_body_len))
    {
        FLOG_ERROR("NULL PTR! \r\n");
        return TRANS_FAILD;      
    }  

    *p_body_len = 0;

    p_send_temp = (struct rrh_monitor_param *)p_send_body;
    
    p_build = (struct trans_rrh_build_msg_param *)(p_build_info->p_msg);

    #if 0
    trans_debug_msg_print(p_build, 20, g_trans_debug_rrh);
    
    FLOG_INFO("2 uw_num =%d \r\n", uw_num);
    printf("%p\n",p_build); 
    printf("us_param_type %d\n",p_build->us_param_type); 

    //p_send_temp->us_param_type = htons(p_build->us_param_type);   
    printf("%d\n",htons(p_build->us_param_type));   
    
    p_send_temp->uc_param_len = p_build->uc_param_len;


    p_temp1 = p_send_body+3;

    p_temp2 = ((u_int8_t *)p_build_info->p_msg)+3;
    
    printf("1 %p %p\n",p_send_body+3, p_build->p_param);

    printf("2 %p %p\n",p_temp1, p_temp2);
    //*(p_send_temp->p_param) = *((u_int8_t *)(p_build->p_param));
    
    memcpy(p_temp1, ((u_int8_t *)p_build)+3, p_build->uc_param_len);
    
    uw_len_temp = uw_len_temp + 3 + p_build->uc_param_len;
      
    #endif
    while (uw_num)
    {
        /*Offset 3 for the Bytes of us_param_type  and uc_param_len*/
        p_send_temp = (struct rrh_monitor_param *)(p_send_body + uw_len_temp);
        
        p_build = (struct trans_rrh_build_msg_param *)(((u_int8_t *)(p_build_info->p_msg)) + uw_len_temp);
    
        p_send_temp->us_param_type = htons(p_build->us_param_type);
        //p_send_temp->us_param_type = p_build->us_param_type;
        p_send_temp->uc_param_len = p_build->uc_param_len;

        //FLOG_INFO("2 us_param_type =%d \r\n", p_build->us_param_type); 
        //FLOG_INFO("2 p_build->uc_param_len =%d \r\n", p_build->uc_param_len); 

        //FLOG_INFO("2 p_send_temp->uc_param_len =%d \r\n", p_send_temp->uc_param_len); 

        //trans_debug_msg_print(p_send_temp, 30, g_trans_debug_rrh);

        p_temp1 = ((u_int8_t *)p_send_temp)+SIZEOF_RRH_MONITOR_PARAM;        
        p_temp2 = ((u_int8_t *)p_build)+SIZEOF_RRH_MONITOR_PARAM;

        //FLOG_DEBUG("2 %p %p\n",p_temp1, p_temp2);

        //#if 0
        /*Param Length is UInt32*/
        if (4 == p_build->uc_param_len)
        {
            memcpy(&(uw_value32), p_temp2, 4); 

            uw_value32 = htonl(uw_value32);

            FLOG_DEBUG("uw_value32 = %d \n", uw_value32);

            memcpy(p_temp1, &(uw_value32), 4); 

        }
        /*Param Length is UInt16*/
        else if (2 == p_build->uc_param_len)
        {
            memcpy(&(us_value16), p_temp2, 2); 
            
            us_value16 = htons(us_value16);

            FLOG_DEBUG("us_value16 = %d \n", us_value16);
            
            memcpy(p_temp1, &(us_value16), 2); 

        }
        /*Param Length is UInt8 or str*/
        else 
        {
            memcpy(p_temp1, p_temp2, p_build->uc_param_len);

            FLOG_DEBUG(" %d \n", p_build->uc_param_len);
        }   
        //#endif
        
        //memcpy(p_temp1, p_temp2, p_build->uc_param_len); 

        /* 3 for the Bytes length of us_param_type  and uc_param_len*/
        uw_len_temp = uw_len_temp + SIZEOF_RRH_MONITOR_PARAM + p_build->uc_param_len;

        uw_num--;
    }
   
    *p_body_len = uw_len_temp;

    trans_debug_msg_print(p_send_body, 20, g_trans_debug_rrh);

    FLOG_DEBUG("Exit \r\n"); 
     
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
u_int32_t trans_rrh_build_msg(struct trans_rrh_build_msg_info *p_build_info, 
                                    u_int8_t *p_send_msg, u_int32_t *p_msg_len, u_int16_t *p_serial_number)
{
    u_int8_t    uc_type = 0;
    u_int32_t  uw_ret = 0;
    u_int32_t   uw_param_num = 0;  /*Param number：4Bytes*/
    u_int32_t  uw_body_len = 0;

    u_int8_t * p_msg_body = NULL;    
    u_int16_t   *p_new_crc = NULL;
    u_int16_t   us_crc_val = 0;
    
    FLOG_DEBUG("Enter \r\n");

    if ((NULL == p_send_msg) || (NULL == p_msg_len) || (NULL == p_serial_number))
    {
        FLOG_ERROR("NULL PTR! \r\n");
        return TRANS_FAILD;      
    }      

    //trans_debug_msg_print(p_build_info, 20, g_trans_debug_rrh);
    
    uw_param_num = p_build_info->uw_param_num;
   
    p_msg_body = p_send_msg + SIZEOF_RRH_MONITOR_HEADER;
 
    /*Build Msg Body */
   
    /*HEARTBRAT has no body*/
    uc_type = p_build_info->uc_type;
    if (RRH_MONITOR_TYPE_HEARTBRAT != p_build_info->uc_type)
    {
        uw_ret = trans_rrh_build_body(p_build_info, uw_param_num, p_msg_body, &uw_body_len);
        /*Error*/
        if (TRANS_SUCCESS != uw_ret)
        {
            FLOG_ERROR("Call trans_rrh_build_body error!uw_ret= %d \r\n", uw_ret);
            return TRANS_FAILD;
        } 
    }

    if (uw_body_len != p_build_info->uw_msg_len)
    {
        FLOG_ERROR("Length error!uw_body_len= %d, uw_msg_len= %d \r\n", uw_body_len, p_build_info->uw_msg_len);
        return TRANS_FAILD;
    }

    /*Build Msg head */
    uw_ret = trans_rrh_build_heard(p_build_info, p_send_msg, uw_body_len, p_serial_number);
    /*Error*/
    if (TRANS_SUCCESS != uw_ret)
    {
        FLOG_ERROR("Call trans_rrh_build_heard error!uw_ret= %d \r\n", uw_ret);
        return TRANS_FAILD;
    }  

    /*Build Msg CRC */    /*calculate CRC*/
    us_crc_val = trans_rrh_cal_crc16((u_int8_t*)p_send_msg, uw_body_len + SIZEOF_RRH_MONITOR_HEADER); 
    
    p_new_crc = (u_int16_t  *)(p_send_msg + uw_body_len + SIZEOF_RRH_MONITOR_HEADER);
    *p_new_crc = htons(us_crc_val);

    //FLOG_DEBUG("*p_new_crc = %d \r\n", *p_new_crc);

    /* 2 for the Bytes length of CRC*/
    *p_msg_len = uw_body_len + SIZEOF_RRH_MONITOR_HEADER +2;

    FLOG_DEBUG("Exit \r\n"); 

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
u_int32_t trans_rrh_send_msg_process(struct trans_rrh_send_msg_info *p_info)
{

    struct trans_rrh_build_msg_info *p_build_info = NULL;
    struct trans_rrh_send_msg_info *p_send_info = NULL;
    u_int32_t   uw_param_num = 0;  /*Param number：4Bytes*/
    u_int8_t    uc_type = 0;
    u_int8_t  *p_send_buf = NULL;
    u_int32_t uw_ret = 0;
    u_int32_t uw_send_len = 0;
    u_int16_t   us_serial_number = 0;

    int32_t w_ret = 0;
    
    FLOG_DEBUG("Enter \r\n"); 

    if (NULL == p_info)
    {
    
        FLOG_ERROR("NULL PTR! \r\n");
        return TRANS_FAILD;      
    }  

    p_send_info = (struct trans_rrh_send_msg_info *)p_info;
   
    p_build_info = (struct trans_rrh_build_msg_info *)(&(p_send_info->st_build_info));

    if (RRH_MSG_MAX_LEN < p_build_info->uw_msg_len)
    {
        FLOG_ERROR("Length error! \r\n");
        return TRANS_FAILD;      
    }

    uw_param_num = p_build_info->uw_param_num;
    uc_type = p_build_info->uc_type;
    
    if ((0 == uw_param_num) && (RRH_MONITOR_TYPE_HEARTBRAT != uc_type))
    {
        FLOG_ERROR("Param num error! \r\n");
        return TRANS_FAILD;  
    }

    if (uc_type >= RRH_MONITOR_TYPE_BUF)
    {
        FLOG_ERROR("Type error! \r\n");
        return TRANS_FAILD; 
    }
   
    /* Allocate a memory.  */
    p_send_buf = (u_int8_t *)malloc(RRH_MSG_MAX_LEN);
    if (NULL == p_send_buf)
    {
        FLOG_ERROR("malloc p_send_buf error! \r\n");
        return TRANS_FAILD;   
    }  

    memset((u_int8_t*)p_send_buf, 0, RRH_MSG_MAX_LEN);

    /*Build Msg*/
    uw_ret =  trans_rrh_build_msg(p_build_info, p_send_buf, &uw_send_len, &us_serial_number);
    if (TRANS_SUCCESS != uw_ret) 
    {   
        free(p_send_buf);

        FLOG_ERROR("Call trans_rrh_build_msg error! uw_ret = %d\r\n", uw_ret);
        return TRANS_FAILD;     
    }   

    trans_debug_msg_print(p_send_buf, 30, g_trans_debug_rrh);
    
    /*Send New Msg to RRH*/
    w_ret = send(p_send_info->w_rrh_sockfd, p_send_buf, uw_send_len, 0);
    
    if(w_ret <= 0)
    {
        free(p_send_buf);
        //close(sock);
        FLOG_ERROR("send error! w_ret = %d\r\n", w_ret);
        return TRANS_FAILD;
    }   

    /*ADD TIMER*/
    void* p_timer_id = NULL;
    struct trans_timer_msg_info st_msg_info;
    //u_int32_t uw_len = 0;
    
    memset((u_int8_t*)&st_msg_info, 0, SIZEOF_TRANS_TIMER_MSG_INFO);
    
    st_msg_info.us_serial_number = us_serial_number;
    st_msg_info.uw_src_moudle = p_send_info->uw_src_moudle;
    st_msg_info.uc_block_flag = p_send_info->uc_block_flag;
    st_msg_info.f_callback = p_send_info->f_action;

    #if 0
    if (NULL != p_send_info->u_extra_info)
    {
        uw_len = sizeof(p_send_info->u_extra_info);   
        memcpy((u_int8_t *)(&(st_msg_info.u_extra_info)), (u_int8_t *)(&(p_send_info->u_extra_info)), uw_len);
    }
    #endif

    /*p_action->u_action_info : It could be NULL*/
    if (NULL == p_send_info->p_user_info)
    {
        FLOG_DEBUG("No user info! \r\n");
    
        st_msg_info.p_user_info = NULL;
    }
    else
    {
        st_msg_info.p_user_info = p_send_info->p_user_info;
      
        FLOG_INFO("Copy user info %p! \r\n", st_msg_info.p_user_info);
    }
    
    uw_ret = trans_timer_add(&p_send_info->expired_tv, 
                            p_send_info->f_action, 
                            p_send_buf, 
                            uw_send_len, 
                            &st_msg_info,
                            &p_timer_id);
    
    if (TRANS_SUCCESS != uw_ret) 
    {   
        free(p_send_buf);
        
        FLOG_ERROR("Call trans_timer_add error! uw_ret = %d\r\n", uw_ret);
        return TRANS_FAILD;     
    }  

    free(p_send_buf);
    
    FLOG_DEBUG("Exit \r\n");   
    
    return TRANS_SUCCESS;    
}

/*****************************************************************************+
* Function: trans_rrh_block_msg_timer_func()
* Description: 
* Parameters:
*           NONE
* Return Values:
*           TRANS_FAILD : 1
*           TRANS_SUCCESS  : 0
*
*  
*  Data:    2011-08-04
* 
+*****************************************************************************/
int trans_rrh_block_msg_timer_func(void *p_msg, size_t len, void *p_msg_info)
{
    u_int32_t uw_ret = 0;    
    
    len = len;
    
    FLOG_ERROR("Time out : callback function\r\n"); 
    
    if ((NULL == p_msg) || (NULL == p_msg_info) )
    {
        FLOG_ERROR("NULL PTR! \r\n");
        return TRANS_FAILD;  
    }
    
    struct trans_en_queue_msg   st_en_quene;
    
    st_en_quene.uc_result = 2;
    st_en_quene.uw_len = len;
    st_en_quene.uw_src_moudle = TRANS_MOUDLE_RRH;
    
    uw_ret = trans_msg_en_quene(p_msg, &st_en_quene);
    
    if (TRANS_SUCCESS != uw_ret)
    {
        FLOG_ERROR("Call trans_msg_en_quene error! uw_ret = %d\r\n", uw_ret);
    
        return TRANS_FAILD;    
    }
    
    return TRANS_SUCCESS;    

}

/*****************************************************************************+
* Function: trans_rrh_noblock_msg_timer_func()
* Description: 
* Parameters:
*           NONE
* Return Values:
*           TRANS_FAILD : 1
*           TRANS_SUCCESS  : 0
*
*  
*  Data:    2011-08-04
* 
+*****************************************************************************/
int trans_rrh_noblock_msg_timer_func(void *p_msg, size_t len, void *p_msg_info)
{
    len = len;
    
    FLOG_ERROR("Time out : callback function\r\n"); 
    
    if ((NULL == p_msg) || (NULL == p_msg_info) )
    {
        FLOG_ERROR("NULL PTR! \r\n");
        return TRANS_FAILD;  
    }
    
     /*Do not handle exceptions -----need do it in future*/  
    
  
    return TRANS_SUCCESS;

}

#if 0
/*****************************************************************************+
* Function: trans_rrh_send_msg_timer_func()
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
int trans_rrh_send_msg_timer_func(void *p_msg, size_t len, void *p_msg_info)
{
    //#if 0
    u_int16_t   us_serial_number = 0;
    u_int8_t uc_find_flag = 0;
    struct adapt_timer *p_timer = NULL;
    //#endif
    u_int32_t uw_ret = 0;    

    len = len;
    
    FLOG_INFO("Time out : callback function\r\n"); 

    if ((NULL == p_msg) || (NULL == p_msg_info) )
    {
        FLOG_ERROR("NULL PTR! \r\n");
        return TRANS_FAILD;  
    }

    struct trans_en_queue_msg   st_en_quene;

    st_en_quene.uc_result = TRANS_FAILD;
    st_en_quene.uw_len = len;
    st_en_quene.uw_src_moudle = TRANS_MOUDLE_BUF;
    
    uw_ret = trans_msg_en_quene(p_msg, &st_en_quene);
    
    if (TRANS_SUCCESS != uw_ret)
    {
        FLOG_ERROR("Call trans_msg_en_quene error! uw_ret = %d\r\n", uw_ret);
    
        return TRANS_FAILD;    
    }
    
    //#if 0
    struct rrh_monitor_header  *p_rrh_header = NULL;

    p_rrh_header = (struct rrh_monitor_header *)p_msg;

    //memcpy(&us_serial_number, p_msg, len);  
    us_serial_number = ntohs(p_rrh_header->us_serial_number);
    FLOG_INFO("us_serial_number = %d\r\n", us_serial_number); 
    
    uw_ret = adapt_timer_find_by_serial_num(us_serial_number, (void**)&p_timer, &uc_find_flag);
    
    if (TRANS_SUCCESS != uw_ret)
    {
        FLOG_ERROR("Call adapt_timer_find_by_serial_num error! uw_ret = %d\r\n", uw_ret);
    
        return TRANS_FAILD;    
    }
    
    /*if timeout ,discard message*/
    if (1 != uc_find_flag)
    {
        FLOG_ERROR("Can't find the timer! us_serial_number = %d\r\n", us_serial_number);
    
        /*???????Report to C&M*/
        socketfd = socketfd;
    
        return TRANS_SUCCESS;    
    }
    /*if find , delete timer*/
    else
    {
        FLOG_INFO("Get the timer\r\n"); 
        /*Delete Timer*/
        uw_ret = adapt_timer_delete(p_timer);
        if (TRANS_SUCCESS != uw_ret) 
        {   
            FLOG_ERROR("Call adapt_timer_delete error! uw_ret = %d\r\n", uw_ret);
            return TRANS_FAILD;     
        } 
        FLOG_INFO("Timer print :uc_deleted = %d\r\n", p_timer->uc_deleted); 
        
        trans_debug_msg_print(p_timer->p_msg, 30, g_trans_debug_rrh);
        
    }
    //#endif
    
    FLOG_INFO("Exit\r\n");    
    return TRANS_SUCCESS;
}
#endif

/*****************************************************************************+
* Function: trans_rrh_metric_timer_func()
* Description: 
* Parameters:
*           NONE
* Return Values:
*           TRANS_FAILD : 1
*           TRANS_SUCCESS  : 0
*
*  
*  Data:    2011-07-12
* 
+*****************************************************************************/
int trans_rrh_metric_timer_func(void *p_msg, size_t len, void *p_msg_info)
{
    len = len;
    
    FLOG_INFO("Time out : callback function\r\n"); 
    
    if ((NULL == p_msg) || (NULL == p_msg_info) )
    {
        FLOG_ERROR("NULL PTR! \r\n");
        return TRANS_FAILD;  
    }

     /*Do not handle exceptions -----need do it in future*/  
   
   
    return TRANS_SUCCESS;

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

    //u_int16_t   us_crc_val = 0;
    struct timeval tv;
    
    FLOG_DEBUG("Enter\r\n"); 
    p_msg = p_msg;
    p_msg_info = p_msg_info;
    #if 0
    if ((NULL == p_msg) || (NULL == p_msg_info) )
    {
        FLOG_ERROR("NULL PTR! \r\n");
        return TRANS_FAILD;  
    }
    #endif

    len = len;

    //p_src_msg_info = (struct trans_timer_msg_info *)p_msg_info;

    /*Not revice HeartBeat Msg for 3 times->report to C&M*/
    if (g_trans_rrh_heartbeat_num >= RRH_HEARTBEAT_TIMEOUT_NUM)
    {
        /*report to C&M----future*/

        /*怎么处理 */
        //close (g_rrh_client_socket.w_sockFd);
        //close (g_rrh_server_socket.w_sockFd);
        FLOG_ERROR("9 times! g_trans_rrh_heartbeat_num = %d\r\n", g_trans_rrh_heartbeat_num);
        
        return TRANS_SUCCESS;
    }
    
    memset((u_int8_t*)&st_send_info, 0, sizeof(struct trans_rrh_send_msg_info));

    /*Send Heartbeat Msg Again  per 3 second*/
    gettimeofday(&tv, NULL);
    
    st_send_info.expired_tv.tv_sec = tv.tv_sec + TRANS_HEARTBEAT_TIMEOUT;
    st_send_info.expired_tv.tv_usec = tv.tv_usec;
    
    //st_send_info.w_rrh_sockfd = (*p_msg);
    //memcpy(&(st_send_info.w_rrh_sockfd), p_msg, len);  
    //st_send_info.w_rrh_sockfd = g_trans_moudle_socket_fd[p_src_msg_info->uw_src_moudle];
    st_send_info.w_rrh_sockfd = g_trans_rrh_socket;
    
    /*Send msg to RRH again and again*/
    st_send_info.uw_src_moudle = TRANS_MOUDLE_LOCAL;
    st_send_info.uc_block_flag = TRANS_QUENE_NO_BLOCK;
    st_send_info.f_action = trans_rrh_heartbeat_timer_func;
    
    st_send_info.st_build_info.uc_type = RRH_MONITOR_TYPE_HEARTBRAT;
    st_send_info.st_build_info.uw_param_num = 0;
    st_send_info.st_build_info.uw_msg_len = 0;
    st_send_info.st_build_info.p_msg = NULL;

    FLOG_DEBUG("st_send_info.st_build_info.uw_param_num = %d\r\n", st_send_info.st_build_info.uw_param_num);
    
    uw_ret = trans_rrh_send_msg_process(&st_send_info);
    if (TRANS_SUCCESS != uw_ret) 
    {   
     
        FLOG_ERROR("Call trans_rrh_send_msg_process error! uw_ret = %d\r\n", uw_ret);
        return TRANS_FAILD;     
    }    

    g_trans_rrh_heartbeat_num++;

    FLOG_INFO("Send Heartbeat to RRH\r\n"); 
    
    FLOG_DEBUG("Exit\r\n");    

    return TRANS_SUCCESS;
}

/*****************************************************************************+
* Function: trans_rrh_module_query_info()
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
u_int32_t trans_rrh_module_query_info(struct trans_send_query_to_rrh *p_query_rrh,
                            struct trans_rrh_send_msg_info *p_send_info,
                            u_int8_t * p_send_msg)
{
    u_int32_t uw_ret = 0;
    u_int32_t us_num = 0;
    u_int32_t uw_len = 0;

    struct timeval tv;
    struct rrh_monitor_param *p_param = NULL;
    u_int16_t  *p_type = NULL;
    
    if ((NULL == p_query_rrh) || (NULL == p_send_info) || (NULL == p_send_msg))
    {
        FLOG_ERROR("NULL PTR! \r\n");
        return TRANS_FAILD;      
    }  
    
    p_param = (struct rrh_monitor_param *)p_send_msg;
    
    gettimeofday(&tv, NULL);
    
    p_send_info->expired_tv.tv_sec = tv.tv_sec + p_query_rrh->uw_timeout;
    p_send_info->expired_tv.tv_usec = tv.tv_usec;
    
    p_send_info->f_action = p_query_rrh->f_callback; 
    
    p_send_info->st_build_info.uw_param_num = p_query_rrh->us_param_num;
    p_send_info->p_user_info = p_query_rrh->p_info;
    
    us_num = p_query_rrh->us_param_num;

    p_type = (u_int16_t *)(p_query_rrh->p_param_type);
    
    while (us_num)
    {

        if (NULL == p_type)
        {
            FLOG_ERROR("NULL PTR!p_type \r\n");
            return TRANS_FAILD;  
        }
        
        p_param->us_param_type = ntohs(*(p_type));
    
        uw_ret = trans_rrh_type_get_len(p_param->us_param_type, 
                                                    &(p_param->uc_param_len));
        if (TRANS_SUCCESS != uw_ret)
        {
            FLOG_ERROR("Call trans_rrh_type_get_len error! uw_ret = %d\r\n", uw_ret);
            return TRANS_FAILD;  
        }
        
        uw_len = uw_len + SIZEOF_RRH_MONITOR_PARAM + p_param->uc_param_len;
        
        p_param = (struct rrh_monitor_param *)(p_send_msg + uw_len);
    
        us_num--;
        p_type++;
    
    }
    
    p_send_info->st_build_info.uw_msg_len = uw_len;
    
    p_send_info->st_build_info.p_msg = p_send_msg;

    //trans_debug_msg_print(p_send_msg, 10, g_trans_debug_rrh);

    return TRANS_SUCCESS;

}


/*****************************************************************************+
* Function: trans_rrh_module_cfg_info()
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
u_int32_t trans_rrh_module_cfg_info(struct trans_send_cfg_to_rrh *p_cfg_rrh,
                            struct trans_rrh_send_msg_info *p_send_info,
                            u_int8_t * p_send_msg)
{
    u_int32_t uw_ret = 0;
    u_int32_t us_num = 0;
    u_int32_t uw_len = 0;

    struct timeval tv;
    struct rrh_monitor_param *p_param = NULL;
    u_int16_t  *p_type = NULL;
    int32_t  *p_value = NULL;
    void  *p_value_tmp = NULL;
    int32_t    w_value_temp = 0;

    if ((NULL == p_cfg_rrh) || (NULL == p_send_info) || (NULL == p_send_msg))
    {
        FLOG_ERROR("NULL PTR! \r\n");
        return TRANS_FAILD;      
    }  
    
    p_param = (struct rrh_monitor_param *)p_send_msg;
    
    gettimeofday(&tv, NULL);
    
    p_send_info->expired_tv.tv_sec = tv.tv_sec + p_cfg_rrh->uw_timeout;
    p_send_info->expired_tv.tv_usec = tv.tv_usec;
    
    p_send_info->f_action = p_cfg_rrh->f_callback; 
    
    p_send_info->st_build_info.uw_param_num = p_cfg_rrh->us_param_num;
    p_send_info->p_user_info = p_cfg_rrh->p_info;

    //FLOG_INFO("%p, \r\n", p_send_info->p_user_info);
    
    us_num = p_cfg_rrh->us_param_num;
    p_type = (u_int16_t *)(p_cfg_rrh->p_param_type);
    p_value = (int32_t *)(p_cfg_rrh->p_param_value);
    
    while (us_num)
    {
        
        if (NULL == p_type)
        {
            FLOG_ERROR("NULL PTR!p_type \r\n");
            return TRANS_FAILD;  
        }
        
        //p_param->us_param_type = ntohs(*(p_type));
        /*Config message just can from LOCAL DEVICE (bs or action) now, not from monitor, so don't change*/
        p_param->us_param_type = *(p_type);
        
        uw_ret = trans_rrh_type_get_len(p_param->us_param_type, 
                                                    &(p_param->uc_param_len));
        if (TRANS_SUCCESS != uw_ret)
        {
            FLOG_ERROR("Call trans_rrh_type_get_len error! uw_ret = %d\r\n", uw_ret);
            return TRANS_FAILD;  
        }

        if (NULL == p_value)
        {
            FLOG_ERROR("NULL PTR!p_value \r\n");
            return TRANS_FAILD;  
        }

        p_value_tmp = (void*)(((u_int8_t *)p_param) + SIZEOF_RRH_MONITOR_PARAM);
        
        w_value_temp = *p_value;
        
        /*Config message just can from LOCAL DEVICE (bs or action) now, not from monitor, so don't change*/
        //w_value_temp = ntohl(w_value_temp);
        
        memcpy(p_value_tmp, &(w_value_temp), p_param->uc_param_len);
        
        uw_len = uw_len + SIZEOF_RRH_MONITOR_PARAM + p_param->uc_param_len;        
        p_param = (struct rrh_monitor_param *)(p_send_msg + uw_len);
        
        p_type++;
        /*RRU Parameters value : 4Bytes for one parameters*/
        p_value = p_value + 1;

        us_num--;
        
    }
    
    p_send_info->st_build_info.uw_msg_len = uw_len;
    
    p_send_info->st_build_info.p_msg = p_send_msg;

    //trans_debug_msg_print(p_send_msg, 10, g_trans_debug_rrh);

    return TRANS_SUCCESS;

}


/*****************************************************************************+
* Function: trans_rrh_send_msg_rx_mode()
* Description: 
* Parameters:
*           NONE
* Return Values:
*           TRANS_FAILD : 1
*           TRANS_SUCCESS  : 0
*
*  
*  Data:    2011-06-22
* 
+*****************************************************************************/
u_int32_t trans_rrh_send_msg_ch_mode(u_int8_t *p_msg, u_int8_t uc_num, void *p_value)
{
    u_int32_t uw_ret = 0;
    struct trans_rrh_send_msg_info st_send_info;
    struct timeval tv;
    struct rrh_monitor_param *p_param = NULL;

    u_int32_t uw_len = 0;
    int w_value = 0;
    if ((NULL == p_msg) || (NULL == p_value))
    {
    
        FLOG_ERROR("1 NULL PTR! \r\n");
        return TRANS_FAILD;      
    }  

    /*set ch1 and ch2 at the same time*/
    if (2 != uc_num)
    {
        FLOG_ERROR("Set param num error uc_num = %d! \r\n", uc_num);        
        return TRANS_FAILD;   
    }    

    p_param = (struct rrh_monitor_param *)p_msg;
   
    memset((u_int8_t*)&st_send_info, 0, sizeof(struct trans_rrh_send_msg_info));
    
    gettimeofday(&tv, NULL);
    
    st_send_info.expired_tv.tv_sec = tv.tv_sec + 30;
    st_send_info.expired_tv.tv_usec = tv.tv_usec;
    
    st_send_info.w_rrh_sockfd = g_trans_rrh_socket;
    
    st_send_info.uw_src_moudle = TRANS_MOUDLE_BS;
    st_send_info.uc_block_flag = TRANS_QUENE_BLOCK;
    st_send_info.f_action = trans_rrh_block_msg_timer_func;
    
    st_send_info.st_build_info.uc_type = RRH_MONITOR_TYPE_CONFIG;

    st_send_info.st_build_info.uw_param_num = uc_num;
    st_send_info.st_build_info.uw_msg_len = SIZEOF_RRH_MONITOR_PARAM
        +RRH_MSG_CHAN1_WORKMODE_LEN
        +SIZEOF_RRH_MONITOR_PARAM
        +RRH_MSG_CHAN2_WORKMODE_LEN;

    p_param->us_param_type = RRH_MSG_CHAN1_WORKMODE;
    p_param->uc_param_len = RRH_MSG_CHAN1_WORKMODE_LEN;

    w_value = * ( (int *) p_value );
    *( p_msg +  SIZEOF_RRH_MONITOR_PARAM) = w_value;

    uw_len = SIZEOF_RRH_MONITOR_PARAM + p_param->uc_param_len;

    p_param = (struct rrh_monitor_param *)(p_msg + uw_len);
    
    p_param->us_param_type = RRH_MSG_CHAN2_WORKMODE;
    p_param->uc_param_len = RRH_MSG_CHAN2_WORKMODE_LEN;
    
    w_value = * ( ((int *) p_value) + 1 );
    *( p_msg +  SIZEOF_RRH_MONITOR_PARAM + uw_len) = w_value;
    
    //trans_debug_msg_print(p_msg, 10, g_trans_debug_rrh);
    
    st_send_info.st_build_info.p_msg = p_msg;
    
    uw_ret = trans_rrh_send_msg_process(&st_send_info);
    if (TRANS_SUCCESS != uw_ret) 
    {   
     
        FLOG_ERROR("Call trans_rrh_send_msg_process error! uw_ret = %d\r\n", uw_ret);
        return TRANS_FAILD;     
    } 

    u_int8_t uc_result = 0;
    uw_ret = trans_msg_de_quene(&uc_result);
    
    if ((TRANS_SUCCESS != uw_ret) || (TRANS_SUCCESS != uc_result))
    {
        FLOG_ERROR("The response message from RRH error!result = %d, uw_ret. \r\n", uc_result, uw_ret);
        return TRANS_FAILD; 
    }

    return TRANS_SUCCESS;

}

/*****************************************************************************+
* Function: trans_rrh_send_msg_ch_flag()
* Description: 
* Parameters:
*           NONE
* Return Values:
*           TRANS_FAILD : 1
*           TRANS_SUCCESS  : 0
*
*  
*  Data:    2011-06-22
* 
+*****************************************************************************/
u_int32_t trans_rrh_send_msg_ch_flag(u_int8_t *p_msg, u_int8_t uc_num, void *p_value)
{
    u_int32_t uw_ret = 0;
    struct trans_rrh_send_msg_info st_send_info;
    struct timeval tv;
    struct rrh_monitor_param *p_param = NULL;
    
    u_int32_t uw_len = 0;
    int w_value = 0;
    
    if ((NULL == p_msg) || (NULL == p_value))
    {
    
        FLOG_ERROR("NULL PTR! \r\n");
        return TRANS_FAILD;      
    }  
    
    /*set ch1 and ch2 at the same time*/
    if (2 != uc_num)
    {
        FLOG_ERROR("Set param num error uc_num = %d! \r\n", uc_num);        
        return TRANS_FAILD;   
    }    
    
    p_param = (struct rrh_monitor_param *)p_msg;
    
    memset((u_int8_t*)&st_send_info, 0, sizeof(struct trans_rrh_send_msg_info));
    
    gettimeofday(&tv, NULL);
    
    st_send_info.expired_tv.tv_sec = tv.tv_sec + 30;
    st_send_info.expired_tv.tv_usec = tv.tv_usec;
    
    st_send_info.w_rrh_sockfd = g_trans_rrh_socket;
    
    st_send_info.uw_src_moudle = TRANS_MOUDLE_BS;
    st_send_info.uc_block_flag = TRANS_QUENE_BLOCK;
    st_send_info.f_action = trans_rrh_block_msg_timer_func;
    
    st_send_info.st_build_info.uc_type = RRH_MONITOR_TYPE_CONFIG;
    
    st_send_info.st_build_info.uw_param_num = uc_num;
    st_send_info.st_build_info.uw_msg_len = SIZEOF_RRH_MONITOR_PARAM
        + RRH_MSG_CHAN1_SWITCH_LEN
        + SIZEOF_RRH_MONITOR_PARAM
        + RRH_MSG_CHAN2_SWITCH_LEN;
    
    p_param->us_param_type = RRH_MSG_CHAN1_SWITCH;
    p_param->uc_param_len = RRH_MSG_CHAN1_SWITCH_LEN;
    
    w_value = * ( (int *) p_value );
    *( p_msg +  SIZEOF_RRH_MONITOR_PARAM) = w_value;
    
    uw_len = SIZEOF_RRH_MONITOR_PARAM + p_param->uc_param_len;
    
    p_param = (struct rrh_monitor_param *)(p_msg + uw_len);
    
    p_param->us_param_type = RRH_MSG_CHAN2_SWITCH;
    p_param->uc_param_len = RRH_MSG_CHAN2_SWITCH_LEN;
    
    w_value = * ( ((int *) p_value) + 1 );
    *( p_msg +  SIZEOF_RRH_MONITOR_PARAM + uw_len) = w_value;

    //trans_debug_msg_print(p_msg, 10, g_trans_debug_rrh);
    
    st_send_info.st_build_info.p_msg = p_msg;
    
    uw_ret = trans_rrh_send_msg_process(&st_send_info);
    if (TRANS_SUCCESS != uw_ret) 
    {   
     
        FLOG_ERROR("Call trans_rrh_send_msg_process error! uw_ret = %d\r\n", uw_ret);
        return TRANS_FAILD;     
    }   
    
    u_int8_t uc_result = 0;
    uw_ret = trans_msg_de_quene(&uc_result);
    
    if ((TRANS_SUCCESS != uw_ret) || (TRANS_SUCCESS != uc_result))
    {
        FLOG_ERROR("The response message from RRH error!result = %d, uw_ret. \r\n", uc_result, uw_ret);
        return TRANS_FAILD; 
    }

    return TRANS_SUCCESS;
}

/*****************************************************************************+
* Function: trans_rrh_send_msg_ch_freq()
* Description: 
* Parameters:
*           NONE
* Return Values:
*           TRANS_FAILD : 1
*           TRANS_SUCCESS  : 0
*
*  
*  Data:    2011-06-22
* 
+*****************************************************************************/
u_int32_t trans_rrh_send_msg_ch_freq(u_int8_t *p_msg, u_int8_t uc_num, void *p_value)
{
    u_int32_t uw_ret = 0;
    struct trans_rrh_send_msg_info st_send_info;
    struct timeval tv;
    struct rrh_monitor_param *p_param = NULL;
    
    u_int32_t uw_len = 0;
    int w_value = 0;

    if ((NULL == p_msg) || (NULL == p_value))
    {
    
        FLOG_ERROR("NULL PTR! \r\n");
        return TRANS_FAILD;      
    }  
    
    /*set ch1 and ch2 at the same time*/
    if (2 != uc_num)
    {
        FLOG_ERROR("Set param num error uc_num = %d! \r\n", uc_num);        
        return TRANS_FAILD;   
    }    
    
    p_param = (struct rrh_monitor_param *)p_msg;
    
    memset((u_int8_t*)&st_send_info, 0, sizeof(struct trans_rrh_send_msg_info));
    
    gettimeofday(&tv, NULL);
    
    st_send_info.expired_tv.tv_sec = tv.tv_sec + 30;
    st_send_info.expired_tv.tv_usec = tv.tv_usec;
    
    st_send_info.w_rrh_sockfd = g_trans_rrh_socket;
    
    st_send_info.uw_src_moudle = TRANS_MOUDLE_BS;
    st_send_info.uc_block_flag = TRANS_QUENE_BLOCK;
    st_send_info.f_action = trans_rrh_block_msg_timer_func;
    
    st_send_info.st_build_info.uc_type = RRH_MONITOR_TYPE_CONFIG;
    
    st_send_info.st_build_info.uw_param_num = uc_num;
    st_send_info.st_build_info.uw_msg_len = SIZEOF_RRH_MONITOR_PARAM
        + RRH_MSG_CHAN1_FREQ_LEN
        + SIZEOF_RRH_MONITOR_PARAM
        + RRH_MSG_CHAN2_FREQ_LEN;
    
    p_param->us_param_type = RRH_MSG_CHAN1_FREQ;
    p_param->uc_param_len = RRH_MSG_CHAN1_FREQ_LEN;
    
    w_value = * ( (int *) p_value );
    *((u_int32_t *)( p_msg +  SIZEOF_RRH_MONITOR_PARAM)) = w_value;
    
    uw_len = SIZEOF_RRH_MONITOR_PARAM + p_param->uc_param_len;
    
    p_param = (struct rrh_monitor_param *)(p_msg + uw_len);
    
    p_param->us_param_type = RRH_MSG_CHAN2_FREQ;
    p_param->uc_param_len = RRH_MSG_CHAN2_FREQ_LEN;
    
    w_value = * ( ((int *) p_value) + 1 );
    *((u_int32_t *)( p_msg +  SIZEOF_RRH_MONITOR_PARAM + uw_len)) = w_value;
    
    //trans_debug_msg_print(p_msg, 10, g_trans_debug_rrh);

    st_send_info.st_build_info.p_msg = p_msg;
    
    uw_ret = trans_rrh_send_msg_process(&st_send_info);
    if (TRANS_SUCCESS != uw_ret) 
    {   
     
        FLOG_ERROR("Call trans_rrh_send_msg_process error! uw_ret = %d\r\n", uw_ret);
        return TRANS_FAILD;     
    }   
    
    u_int8_t uc_result = 0;
    uw_ret = trans_msg_de_quene(&uc_result);
    
    if ((TRANS_SUCCESS != uw_ret) || (TRANS_SUCCESS != uc_result))
    {
        FLOG_ERROR("The response message from RRH error!result = %d, uw_ret. \r\n", uc_result, uw_ret);
        return TRANS_FAILD; 
    }

    return TRANS_SUCCESS;
}


/*****************************************************************************+
* Function: trans_rrh_send_msg_dl_pre_time()
* Description: 
* Parameters:
*           NONE
* Return Values:
*           TRANS_FAILD : 1
*           TRANS_SUCCESS  : 0
*
*  
*  Data:    2011-06-22
* 
+*****************************************************************************/
u_int32_t trans_rrh_send_msg_dl_pre_time(u_int8_t *p_msg, u_int8_t uc_num, void *p_value)
{
    u_int32_t uw_ret = 0;
    struct trans_rrh_send_msg_info st_send_info;
    struct timeval tv;
    struct rrh_monitor_param *p_param = NULL;
    
    //u_int32_t uw_len = 0;
    int w_value = 0;

    if ((NULL == p_msg) || (NULL == p_value))
    {
    
        FLOG_ERROR("NULL PTR! \r\n");
        return TRANS_FAILD;      
    }  
    
    /*set ch1 and ch2 at the same time*/
    if (1 != uc_num)
    {
        FLOG_ERROR("Set param num error uc_num = %d! \r\n", uc_num);        
        return TRANS_FAILD;   
    }    
    
    p_param = (struct rrh_monitor_param *)p_msg;
    
    memset((u_int8_t*)&st_send_info, 0, sizeof(struct trans_rrh_send_msg_info));
    
    gettimeofday(&tv, NULL);
    
    st_send_info.expired_tv.tv_sec = tv.tv_sec + 30;
    st_send_info.expired_tv.tv_usec = tv.tv_usec;
    
    st_send_info.w_rrh_sockfd = g_trans_rrh_socket;
    
    st_send_info.uw_src_moudle = TRANS_MOUDLE_BS;
    st_send_info.uc_block_flag = TRANS_QUENE_BLOCK;
    st_send_info.f_action = trans_rrh_block_msg_timer_func;
    
    st_send_info.st_build_info.uc_type = RRH_MONITOR_TYPE_CONFIG;
    
    st_send_info.st_build_info.uw_param_num = uc_num;
    st_send_info.st_build_info.uw_msg_len = SIZEOF_RRH_MONITOR_PARAM
        + RRH_MSG_DL_PRESEND_TIME_LEN;
    
    p_param->us_param_type = RRH_MSG_DL_PRESEND_TIME;
    p_param->uc_param_len = RRH_MSG_DL_PRESEND_TIME_LEN;
    
    w_value = * ( (int *) p_value );
    *((u_int16_t *)( p_msg +  SIZEOF_RRH_MONITOR_PARAM)) = w_value;

    //trans_debug_msg_print(p_msg, 10, g_trans_debug_rrh);

    st_send_info.st_build_info.p_msg = p_msg;
    
    uw_ret = trans_rrh_send_msg_process(&st_send_info);
    if (TRANS_SUCCESS != uw_ret) 
    {   
     
        FLOG_ERROR("Call trans_rrh_send_msg_process error! uw_ret = %d\r\n", uw_ret);
        return TRANS_FAILD;     
    }   
    
    u_int8_t uc_result = 0;
    uw_ret = trans_msg_de_quene(&uc_result);
    
    if ((TRANS_SUCCESS != uw_ret) || (TRANS_SUCCESS != uc_result))
    {
        FLOG_ERROR("The response message from RRH error!result = %d, uw_ret. \r\n", uc_result, uw_ret);
        return TRANS_FAILD; 
    }

    return TRANS_SUCCESS;
}

/*****************************************************************************+
* Function: trans_rrh_send_msg_tx_len()
* Description: 
* Parameters:
*           NONE
* Return Values:
*           TRANS_FAILD : 1
*           TRANS_SUCCESS  : 0
*
*  
*  Data:    2011-06-22
* 
+*****************************************************************************/
u_int32_t trans_rrh_send_msg_tx_len(u_int8_t *p_msg, u_int8_t uc_num, void *p_value)
{
    u_int32_t uw_ret = 0;
    struct trans_rrh_send_msg_info st_send_info;
    struct timeval tv;
    struct rrh_monitor_param *p_param = NULL;
    
    //u_int32_t uw_len = 0;
    int w_value = 0;

    if ((NULL == p_msg) || (NULL == p_value))
    {
    
        FLOG_ERROR("NULL PTR! \r\n");
        return TRANS_FAILD;      
    }  
    
    /*set ch1 and ch2 at the same time*/
    if (1 != uc_num)
    {
        FLOG_ERROR("Set param num error uc_num = %d! \r\n", uc_num);        
        return TRANS_FAILD;   
    }    
    
    p_param = (struct rrh_monitor_param *)p_msg;
    
    memset((u_int8_t*)&st_send_info, 0, sizeof(struct trans_rrh_send_msg_info));
    
    gettimeofday(&tv, NULL);
    
    st_send_info.expired_tv.tv_sec = tv.tv_sec + 30;
    st_send_info.expired_tv.tv_usec = tv.tv_usec;
    
    st_send_info.w_rrh_sockfd = g_trans_rrh_socket;
    
    st_send_info.uw_src_moudle = TRANS_MOUDLE_BS;
    st_send_info.uc_block_flag = TRANS_QUENE_BLOCK;
    st_send_info.f_action = trans_rrh_block_msg_timer_func;
    
    st_send_info.st_build_info.uc_type = RRH_MONITOR_TYPE_CONFIG;
    
    st_send_info.st_build_info.uw_param_num = uc_num;
    st_send_info.st_build_info.uw_msg_len = SIZEOF_RRH_MONITOR_PARAM
        + RRH_MSG_TX_LEN_LEN;
    
    p_param->us_param_type = RRH_MSG_TX_LEN;
    p_param->uc_param_len = RRH_MSG_TX_LEN_LEN;
    
    w_value = * ( (int *) p_value );
    *((u_int16_t *)( p_msg +  SIZEOF_RRH_MONITOR_PARAM)) = w_value;

    //trans_debug_msg_print(p_msg, 10, g_trans_debug_rrh);

    st_send_info.st_build_info.p_msg = p_msg;
    
    uw_ret = trans_rrh_send_msg_process(&st_send_info);
    if (TRANS_SUCCESS != uw_ret) 
    {   
     
        FLOG_ERROR("Call trans_rrh_send_msg_process error! uw_ret = %d\r\n", uw_ret);
        return TRANS_FAILD;     
    }   
    
    u_int8_t uc_result = 0;
    uw_ret = trans_msg_de_quene(&uc_result);
    
    if ((TRANS_SUCCESS != uw_ret) || (TRANS_SUCCESS != uc_result))
    {
        FLOG_ERROR("The response message from RRH error!result = %d, uw_ret. \r\n", uc_result, uw_ret);
        return TRANS_FAILD; 
    }

    return TRANS_SUCCESS;
}

/*****************************************************************************+
* Function: trans_rrh_send_msg_rx_len()
* Description: 
* Parameters:
*           NONE
* Return Values:
*           TRANS_FAILD : 1
*           TRANS_SUCCESS  : 0
*
*  
*  Data:    2011-06-22
* 
+*****************************************************************************/
u_int32_t trans_rrh_send_msg_rx_len(u_int8_t *p_msg, u_int8_t uc_num, void *p_value)
{
    u_int32_t uw_ret = 0;
    struct trans_rrh_send_msg_info st_send_info;
    struct timeval tv;
    struct rrh_monitor_param *p_param = NULL;
    
    //u_int32_t uw_len = 0;
    int w_value = 0;

    if ((NULL == p_msg) || (NULL == p_value))
    {
    
        FLOG_ERROR("NULL PTR! \r\n");
        return TRANS_FAILD;      
    }  
    
    /*set ch1 and ch2 at the same time*/
    if (1 != uc_num)
    {
        FLOG_ERROR("Set param num error uc_num = %d! \r\n", uc_num);        
        return TRANS_FAILD;   
    }    
    
    p_param = (struct rrh_monitor_param *)p_msg;
    
    memset((u_int8_t*)&st_send_info, 0, sizeof(struct trans_rrh_send_msg_info));
    
    gettimeofday(&tv, NULL);
    
    st_send_info.expired_tv.tv_sec = tv.tv_sec + 30;
    st_send_info.expired_tv.tv_usec = tv.tv_usec;
    
    st_send_info.w_rrh_sockfd = g_trans_rrh_socket;
    
    st_send_info.uw_src_moudle = TRANS_MOUDLE_BS;
    st_send_info.uc_block_flag = TRANS_QUENE_BLOCK;
    st_send_info.f_action = trans_rrh_block_msg_timer_func;
    
    st_send_info.st_build_info.uc_type = RRH_MONITOR_TYPE_CONFIG;
    
    st_send_info.st_build_info.uw_param_num = uc_num;
    st_send_info.st_build_info.uw_msg_len = SIZEOF_RRH_MONITOR_PARAM
        + RRH_MSG_RX_LEN_LEN;
    
    p_param->us_param_type = RRH_MSG_RX_LEN;
    p_param->uc_param_len = RRH_MSG_RX_LEN_LEN;
    
    w_value = * ( (int *) p_value );
    *((u_int16_t *)( p_msg +  SIZEOF_RRH_MONITOR_PARAM)) = w_value;

    //trans_debug_msg_print(p_msg, 10, g_trans_debug_rrh);

    st_send_info.st_build_info.p_msg = p_msg;
    
    uw_ret = trans_rrh_send_msg_process(&st_send_info);
    if (TRANS_SUCCESS != uw_ret) 
    {   
     
        FLOG_ERROR("Call trans_rrh_send_msg_process error! uw_ret = %d\r\n", uw_ret);
        return TRANS_FAILD;     
    }   
    
    u_int8_t uc_result = 0;
    uw_ret = trans_msg_de_quene(&uc_result);
    
    if ((TRANS_SUCCESS != uw_ret) || (TRANS_SUCCESS != uc_result))
    {
        FLOG_ERROR("The response message from RRH error!result = %d, uw_ret. \r\n", uc_result, uw_ret);
        return TRANS_FAILD; 
    }

    return TRANS_SUCCESS;
}

/*****************************************************************************+
* Function: trans_rrh_send_msg_pa_switch()
* Description: 
* Parameters:
*           NONE
* Return Values:
*           TRANS_FAILD : 1
*           TRANS_SUCCESS  : 0
*
*  
*  Data:    2011-10-21
* 
+*****************************************************************************/
u_int32_t trans_rrh_send_msg_pa_switch(u_int8_t *p_msg, u_int8_t uc_num, void *p_value)
{
    u_int32_t uw_ret = 0;
    struct trans_rrh_send_msg_info st_send_info;
    struct timeval tv;
    struct rrh_monitor_param *p_param = NULL;
    
    u_int32_t uw_len = 0;
    int w_value = 0;
    
    if ((NULL == p_msg) || (NULL == p_value))
    {
    
        FLOG_ERROR("NULL PTR! \r\n");
        return TRANS_FAILD;      
    }  
    
    /*set ch1 and ch2 at the same time*/
    if (2 != uc_num)
    {
        FLOG_ERROR("Set param num error uc_num = %d! \r\n", uc_num);        
        return TRANS_FAILD;   
    }    
    
    p_param = (struct rrh_monitor_param *)p_msg;
    
    memset((u_int8_t*)&st_send_info, 0, sizeof(struct trans_rrh_send_msg_info));
    
    gettimeofday(&tv, NULL);
    
    st_send_info.expired_tv.tv_sec = tv.tv_sec + 30;
    st_send_info.expired_tv.tv_usec = tv.tv_usec;
    
    st_send_info.w_rrh_sockfd = g_trans_rrh_socket;
    
    st_send_info.uw_src_moudle = TRANS_MOUDLE_BS;
    st_send_info.uc_block_flag = TRANS_QUENE_BLOCK;
    st_send_info.f_action = trans_rrh_block_msg_timer_func;
    
    st_send_info.st_build_info.uc_type = RRH_MONITOR_TYPE_CONFIG;
    
    st_send_info.st_build_info.uw_param_num = uc_num;
    st_send_info.st_build_info.uw_msg_len = SIZEOF_RRH_MONITOR_PARAM
        + RRH_MSG_PA_SWITCH_A_CFG_LEN
        + SIZEOF_RRH_MONITOR_PARAM
        + RRH_MSG_PA_SWITCH_B_CFG_LEN;
    
    p_param->us_param_type = RRH_MSG_PA_SWITCH_A_CFG;
    p_param->uc_param_len = RRH_MSG_PA_SWITCH_A_CFG_LEN;
    
    w_value = * ( (int *) p_value );
    *( p_msg +  SIZEOF_RRH_MONITOR_PARAM) = w_value;
    
    uw_len = SIZEOF_RRH_MONITOR_PARAM + p_param->uc_param_len;
    
    p_param = (struct rrh_monitor_param *)(p_msg + uw_len);
    
    p_param->us_param_type = RRH_MSG_PA_SWITCH_B_CFG;
    p_param->uc_param_len = RRH_MSG_PA_SWITCH_B_CFG_LEN;
    
    w_value = * ( ((int *) p_value) + 1 );
    *( p_msg +  SIZEOF_RRH_MONITOR_PARAM + uw_len) = w_value;

    //trans_debug_msg_print(p_msg, 10, g_trans_debug_rrh);
    
    st_send_info.st_build_info.p_msg = p_msg;
    
    uw_ret = trans_rrh_send_msg_process(&st_send_info);
    if (TRANS_SUCCESS != uw_ret) 
    {   
     
        FLOG_ERROR("Call trans_rrh_send_msg_process error! uw_ret = %d\r\n", uw_ret);
        return TRANS_FAILD;     
    }   
    
    u_int8_t uc_result = 0;
    uw_ret = trans_msg_de_quene(&uc_result);
    
    if ((TRANS_SUCCESS != uw_ret) || (TRANS_SUCCESS != uc_result))
    {
        FLOG_ERROR("The response message from RRH error!result = %d, uw_ret. \r\n", uc_result, uw_ret);
        return TRANS_FAILD; 
    }

    return TRANS_SUCCESS;
}

/*****************************************************************************+
* Function: trans_rrh_send_msg_agc()
* Description: 
* Parameters:
*           NONE
* Return Values:
*           TRANS_FAILD : 1
*           TRANS_SUCCESS  : 0
*
*  
*  Data:    2011-06-22
* 
+*****************************************************************************/
u_int32_t trans_rrh_send_msg_agc(u_int8_t *p_msg, u_int8_t uc_num, void *p_value)
{
    u_int32_t uw_ret = 0;
    struct trans_rrh_send_msg_info st_send_info;
    struct timeval tv;
    struct rrh_monitor_param *p_param = NULL;
    
    //u_int32_t uw_len = 0;
    int w_value = 0;

    if ((NULL == p_msg) || (NULL == p_value))
    {
    
        FLOG_ERROR("NULL PTR! \r\n");
        return TRANS_FAILD;      
    }  
    
    /*set ch1 and ch2 at the same time*/
    if (1 != uc_num)
    {
        FLOG_ERROR("Set param num error uc_num = %d! \r\n", uc_num);        
        return TRANS_FAILD;   
    }    
    
    p_param = (struct rrh_monitor_param *)p_msg;
    
    memset((u_int8_t*)&st_send_info, 0, sizeof(struct trans_rrh_send_msg_info));
    
    gettimeofday(&tv, NULL);
    
    st_send_info.expired_tv.tv_sec = tv.tv_sec + 30;
    st_send_info.expired_tv.tv_usec = tv.tv_usec;
    
    st_send_info.w_rrh_sockfd = g_trans_rrh_socket;
    
    st_send_info.uw_src_moudle = TRANS_MOUDLE_BS;
    st_send_info.uc_block_flag = TRANS_QUENE_BLOCK;
    st_send_info.f_action = trans_rrh_block_msg_timer_func;
    
    st_send_info.st_build_info.uc_type = RRH_MONITOR_TYPE_CONFIG;
    
    st_send_info.st_build_info.uw_param_num = uc_num;
    st_send_info.st_build_info.uw_msg_len = SIZEOF_RRH_MONITOR_PARAM
        + RRH_MSG_AGC_ENABLE_LEN;
    
    p_param->us_param_type = RRH_MSG_AGC_ENABLE;
    p_param->uc_param_len = RRH_MSG_AGC_ENABLE_LEN;
    
    w_value = * ( (int *) p_value );
    *((u_int16_t *)( p_msg +  SIZEOF_RRH_MONITOR_PARAM)) = w_value;

    //trans_debug_msg_print(p_msg, 10, g_trans_debug_rrh);

    st_send_info.st_build_info.p_msg = p_msg;
    
    uw_ret = trans_rrh_send_msg_process(&st_send_info);
    if (TRANS_SUCCESS != uw_ret) 
    {   
     
        FLOG_ERROR("Call trans_rrh_send_msg_process error! uw_ret = %d\r\n", uw_ret);
        return TRANS_FAILD;     
    }   
    
    u_int8_t uc_result = 0;
    uw_ret = trans_msg_de_quene(&uc_result);
    
    if ((TRANS_SUCCESS != uw_ret) || (TRANS_SUCCESS != uc_result))
    {
        FLOG_ERROR("The response message from RRH error!result = %d, uw_ret. \r\n", uc_result, uw_ret);
        return TRANS_FAILD; 
    }

    return TRANS_SUCCESS;
}

/*****************************************************************************+
* Function: trans_rrh_send_msg_ch_rx_pgc()
* Description: 
* Parameters:
*           NONE
* Return Values:
*           TRANS_FAILD : 1
*           TRANS_SUCCESS  : 0
*
*  
*  Data:    2011-10-21
* 
+*****************************************************************************/
u_int32_t trans_rrh_send_msg_ch_rx_pgc(u_int8_t *p_msg, u_int8_t uc_num, void *p_value)
{
    u_int32_t uw_ret = 0;
    struct trans_rrh_send_msg_info st_send_info;
    struct timeval tv;
    struct rrh_monitor_param *p_param = NULL;
    
    u_int32_t uw_len = 0;
    int w_value = 0;
    
    if ((NULL == p_msg) || (NULL == p_value))
    {
    
        FLOG_ERROR("NULL PTR! \r\n");
        return TRANS_FAILD;      
    }  
    
    /*set ch1 and ch2 at the same time*/
    if (2 != uc_num)
    {
        FLOG_ERROR("Set param num error uc_num = %d! \r\n", uc_num);        
        return TRANS_FAILD;   
    }    
    
    p_param = (struct rrh_monitor_param *)p_msg;
    
    memset((u_int8_t*)&st_send_info, 0, sizeof(struct trans_rrh_send_msg_info));
    
    gettimeofday(&tv, NULL);
    
    st_send_info.expired_tv.tv_sec = tv.tv_sec + 30;
    st_send_info.expired_tv.tv_usec = tv.tv_usec;
    
    st_send_info.w_rrh_sockfd = g_trans_rrh_socket;
    
    st_send_info.uw_src_moudle = TRANS_MOUDLE_BS;
    st_send_info.uc_block_flag = TRANS_QUENE_BLOCK;
    st_send_info.f_action = trans_rrh_block_msg_timer_func;
    
    st_send_info.st_build_info.uc_type = RRH_MONITOR_TYPE_CONFIG;
    
    st_send_info.st_build_info.uw_param_num = uc_num;
    st_send_info.st_build_info.uw_msg_len = SIZEOF_RRH_MONITOR_PARAM
        + RRH_MSG_CHAN1_RX_PGC_LEN
        + SIZEOF_RRH_MONITOR_PARAM
        + RRH_MSG_CHAN2_RX_PGC_LEN;
    
    p_param->us_param_type = RRH_MSG_CHAN1_RX_PGC;
    p_param->uc_param_len = RRH_MSG_CHAN1_RX_PGC_LEN;
    
    w_value = * ( (int *) p_value );
    *( p_msg +  SIZEOF_RRH_MONITOR_PARAM) = w_value;
    
    uw_len = SIZEOF_RRH_MONITOR_PARAM + p_param->uc_param_len;
    
    p_param = (struct rrh_monitor_param *)(p_msg + uw_len);
    
    p_param->us_param_type = RRH_MSG_CHAN2_RX_PGC;
    p_param->uc_param_len = RRH_MSG_CHAN2_RX_PGC_LEN;
    
    w_value = * ( ((int *) p_value) + 1 );
    *( p_msg +  SIZEOF_RRH_MONITOR_PARAM + uw_len) = w_value;

    //trans_debug_msg_print(p_msg, 10, g_trans_debug_rrh);
    
    st_send_info.st_build_info.p_msg = p_msg;
    
    uw_ret = trans_rrh_send_msg_process(&st_send_info);
    if (TRANS_SUCCESS != uw_ret) 
    {   
     
        FLOG_ERROR("Call trans_rrh_send_msg_process error! uw_ret = %d\r\n", uw_ret);
        return TRANS_FAILD;     
    }   
    
    u_int8_t uc_result = 0;
    uw_ret = trans_msg_de_quene(&uc_result);
    
    if ((TRANS_SUCCESS != uw_ret) || (TRANS_SUCCESS != uc_result))
    {
        FLOG_ERROR("The response message from RRH error!result = %d, uw_ret. \r\n", uc_result, uw_ret);
        return TRANS_FAILD; 
    }

    return TRANS_SUCCESS;
}

/*****************************************************************************+
* Function: trans_rrh_send_msg_gps_enable()
* Description: 
* Parameters:
*           NONE
* Return Values:
*           TRANS_FAILD : 1
*           TRANS_SUCCESS  : 0
*
*  
*  Data:    2011-06-22
* 
+*****************************************************************************/
u_int32_t trans_rrh_send_msg_gps_enable(u_int8_t *p_msg, u_int8_t uc_num, void *p_value)
{
    u_int32_t uw_ret = 0;
    struct trans_rrh_send_msg_info st_send_info;
    struct timeval tv;
    struct rrh_monitor_param *p_param = NULL;
    
    //u_int32_t uw_len = 0;
    int w_value = 0;

    if ((NULL == p_msg) || (NULL == p_value))
    {
    
        FLOG_ERROR("NULL PTR! \r\n");
        return TRANS_FAILD;      
    }  
    
    /*set ch1 and ch2 at the same time*/
    if (1 != uc_num)
    {
        FLOG_ERROR("Set param num error uc_num = %d! \r\n", uc_num);        
        return TRANS_FAILD;   
    }    
    
    p_param = (struct rrh_monitor_param *)p_msg;
    
    memset((u_int8_t*)&st_send_info, 0, sizeof(struct trans_rrh_send_msg_info));
    
    gettimeofday(&tv, NULL);
    
    st_send_info.expired_tv.tv_sec = tv.tv_sec + 30;
    st_send_info.expired_tv.tv_usec = tv.tv_usec;
    
    st_send_info.w_rrh_sockfd = g_trans_rrh_socket;
    
    st_send_info.uw_src_moudle = TRANS_MOUDLE_BS;
    st_send_info.uc_block_flag = TRANS_QUENE_BLOCK;
    st_send_info.f_action = trans_rrh_block_msg_timer_func;
    
    st_send_info.st_build_info.uc_type = RRH_MONITOR_TYPE_CONFIG;
    
    st_send_info.st_build_info.uw_param_num = uc_num;
    st_send_info.st_build_info.uw_msg_len = SIZEOF_RRH_MONITOR_PARAM
        + RRH_MSG_GPS_ENABLE_CFG_LEN;
    
    p_param->us_param_type = RRH_MSG_GPS_ENABLE_CFG;
    p_param->uc_param_len = RRH_MSG_GPS_ENABLE_CFG_LEN;
    
    w_value = * ( (int *) p_value );
    *((u_int16_t *)( p_msg +  SIZEOF_RRH_MONITOR_PARAM)) = w_value;

    //trans_debug_msg_print(p_msg, 10, g_trans_debug_rrh);

    st_send_info.st_build_info.p_msg = p_msg;
    
    uw_ret = trans_rrh_send_msg_process(&st_send_info);
    if (TRANS_SUCCESS != uw_ret) 
    {   
     
        FLOG_ERROR("Call trans_rrh_send_msg_process error! uw_ret = %d\r\n", uw_ret);
        return TRANS_FAILD;     
    }   
    
    u_int8_t uc_result = 0;
    uw_ret = trans_msg_de_quene(&uc_result);
    
    if ((TRANS_SUCCESS != uw_ret) || (TRANS_SUCCESS != uc_result))
    {
        FLOG_ERROR("The response message from RRH error!result = %d, uw_ret. \r\n", uc_result, uw_ret);
        return TRANS_FAILD; 
    }

    return TRANS_SUCCESS;
}

/*****************************************************************************+
* Function: trans_rrh_send_msg_output_pow()
* Description: 
* Parameters:
*           NONE
* Return Values:
*           TRANS_FAILD : 1
*           TRANS_SUCCESS  : 0
*
*  
*  Data:    2011-06-22
* 
+*****************************************************************************/
u_int32_t trans_rrh_send_msg_output_pow(u_int8_t *p_msg, u_int8_t uc_num, void *p_value)
{
    u_int32_t uw_ret = 0;
    struct trans_rrh_send_msg_info st_send_info;
    struct timeval tv;
    struct rrh_monitor_param *p_param = NULL;
    
    //u_int32_t uw_len = 0;
    int w_value = 0;

    if ((NULL == p_msg) || (NULL == p_value))
    {
    
        FLOG_ERROR("NULL PTR! \r\n");
        return TRANS_FAILD;      
    }  
    
    /*set ch1 and ch2 at the same time*/
    if (1 != uc_num)
    {
        FLOG_ERROR("Set param num error uc_num = %d! \r\n", uc_num);        
        return TRANS_FAILD;   
    }    
    
    p_param = (struct rrh_monitor_param *)p_msg;
    
    memset((u_int8_t*)&st_send_info, 0, sizeof(struct trans_rrh_send_msg_info));
    
    gettimeofday(&tv, NULL);
    
    st_send_info.expired_tv.tv_sec = tv.tv_sec + 30;
    st_send_info.expired_tv.tv_usec = tv.tv_usec;
    
    st_send_info.w_rrh_sockfd = g_trans_rrh_socket;
    
    st_send_info.uw_src_moudle = TRANS_MOUDLE_BS;
    st_send_info.uc_block_flag = TRANS_QUENE_BLOCK;
    st_send_info.f_action = trans_rrh_block_msg_timer_func;
    
    st_send_info.st_build_info.uc_type = RRH_MONITOR_TYPE_CONFIG;
    
    st_send_info.st_build_info.uw_param_num = uc_num;
    st_send_info.st_build_info.uw_msg_len = SIZEOF_RRH_MONITOR_PARAM
        + RRH_MSG_NORM_OUTPUT_POWER_LEN;
    
    p_param->us_param_type = RRH_MSG_NORM_OUTPUT_POWER;
    p_param->uc_param_len = RRH_MSG_NORM_OUTPUT_POWER_LEN;
    
    w_value = * ( (int *) p_value );
    *((u_int16_t *)( p_msg +  SIZEOF_RRH_MONITOR_PARAM)) = w_value;

    //trans_debug_msg_print(p_msg, 10, g_trans_debug_rrh);

    st_send_info.st_build_info.p_msg = p_msg;
    
    uw_ret = trans_rrh_send_msg_process(&st_send_info);
    if (TRANS_SUCCESS != uw_ret) 
    {   
     
        FLOG_ERROR("Call trans_rrh_send_msg_process error! uw_ret = %d\r\n", uw_ret);
        return TRANS_FAILD;     
    }   
    
    u_int8_t uc_result = 0;
    uw_ret = trans_msg_de_quene(&uc_result);
    
    if ((TRANS_SUCCESS != uw_ret) || (TRANS_SUCCESS != uc_result))
    {
        FLOG_ERROR("The response message from RRH error!result = %d, uw_ret. \r\n", uc_result, uw_ret);
        return TRANS_FAILD; 
    }

    return TRANS_SUCCESS;
}


/*****************************************************************************+
* Function: trans_rrh_send_msg_query_gps()
* Description: 
* Parameters:
*           NONE
* Return Values:
*           TRANS_FAILD : 1
*           TRANS_SUCCESS  : 0
*
*  
*  Data:    2011-08-03
* 
+*****************************************************************************/
u_int32_t trans_rrh_send_msg_q_gps(u_int8_t *p_msg)
{
    u_int32_t uw_ret = 0;
    struct trans_rrh_send_msg_info st_send_info;
    struct timeval tv;
    struct rrh_monitor_param *p_param = NULL;
    
    u_int32_t uw_len = 0;
    //int w_value = 0;
    
    if (NULL == p_msg)
    {
    
        FLOG_ERROR("NULL PTR! \r\n");
        return TRANS_FAILD;      
    }  
    
    p_param = (struct rrh_monitor_param *)p_msg;
    
    memset((u_int8_t*)&st_send_info, 0, sizeof(struct trans_rrh_send_msg_info));
    
    gettimeofday(&tv, NULL);
    
    st_send_info.expired_tv.tv_sec = tv.tv_sec + 30;
    st_send_info.expired_tv.tv_usec = tv.tv_usec;
    
    st_send_info.w_rrh_sockfd = g_trans_rrh_socket;
    
    st_send_info.uw_src_moudle = TRANS_MOUDLE_BS;
    st_send_info.uc_block_flag = TRANS_QUENE_BLOCK;
    st_send_info.f_action = trans_rrh_block_msg_timer_func;
    
    st_send_info.st_build_info.uc_type = RRH_MONITOR_TYPE_QUERY;
    
    st_send_info.st_build_info.uw_param_num = 2;
    st_send_info.st_build_info.uw_msg_len = SIZEOF_RRH_MONITOR_PARAM
        + RRH_MSG_LONGITUDE_LEN
        + SIZEOF_RRH_MONITOR_PARAM
        + RRH_MSG_LATITUDE_LEN;
    
    p_param->us_param_type = RRH_MSG_LONGITUDE;
    p_param->uc_param_len = RRH_MSG_LONGITUDE_LEN;
    
    uw_len = SIZEOF_RRH_MONITOR_PARAM + p_param->uc_param_len;
    
    p_param = (struct rrh_monitor_param *)(p_msg + uw_len);
    
    p_param->us_param_type = RRH_MSG_LATITUDE;
    p_param->uc_param_len = RRH_MSG_LATITUDE_LEN;
    
    //trans_debug_msg_print(p_msg, 10, g_trans_debug_rrh);
    
    st_send_info.st_build_info.p_msg = p_msg;
    
    uw_ret = trans_rrh_send_msg_process(&st_send_info);
    if (TRANS_SUCCESS != uw_ret) 
    {   
     
        FLOG_ERROR("Call trans_rrh_send_msg_process error! uw_ret = %d\r\n", uw_ret);
        return TRANS_FAILD;     
    }   
    
    u_int8_t uc_result = 0;
    uw_ret = trans_msg_de_quene(&uc_result);
    
    if ((TRANS_SUCCESS != uw_ret) || (TRANS_SUCCESS != uc_result))
    {
        FLOG_ERROR("The response message from RRH error!result = %d, uw_ret. \r\n", uc_result, uw_ret);
        return TRANS_FAILD; 
    }

    return TRANS_SUCCESS;
}

/*****************************************************************************+
* Function: trans_rrh_send_msg_q_power()
* Description: 
* Parameters:
*           NONE
* Return Values:
*           TRANS_FAILD : 1
*           TRANS_SUCCESS  : 0
*
*  
*  Data:    2011-08-03
* 
+*****************************************************************************/
u_int32_t trans_rrh_send_msg_q_power(u_int8_t *p_msg)
{
    u_int32_t uw_ret = 0;
    struct trans_rrh_send_msg_info st_send_info;
    struct timeval tv;
    struct rrh_monitor_param *p_param = NULL;
    
    u_int32_t uw_len = 0;
    //int w_value = 0;
    
    if (NULL == p_msg)
    {
    
        FLOG_ERROR("NULL PTR! \r\n");
        return TRANS_FAILD;      
    }  
    
    p_param = (struct rrh_monitor_param *)p_msg;
    
    memset((u_int8_t*)&st_send_info, 0, sizeof(struct trans_rrh_send_msg_info));
    
    gettimeofday(&tv, NULL);
    
    st_send_info.expired_tv.tv_sec = tv.tv_sec + 30;
    st_send_info.expired_tv.tv_usec = tv.tv_usec;
    
    st_send_info.w_rrh_sockfd = g_trans_rrh_socket;
    
    st_send_info.uw_src_moudle = TRANS_MOUDLE_BS;
    st_send_info.uc_block_flag = TRANS_QUENE_BLOCK;
    st_send_info.f_action = trans_rrh_block_msg_timer_func;
    
    st_send_info.st_build_info.uc_type = RRH_MONITOR_TYPE_QUERY;
    
    st_send_info.st_build_info.uw_param_num = 4;
    
    st_send_info.st_build_info.uw_msg_len = SIZEOF_RRH_MONITOR_PARAM
        + RRH_MSG_CHAN1_NORM_POW_VALUE_LEN
        + SIZEOF_RRH_MONITOR_PARAM
        + RRH_MSG_CHAN2_NORM_POW_VALUE_LEN;
    
    p_param->us_param_type = RRH_MSG_CHAN1_NORM_POW_VALUE;
    p_param->uc_param_len = RRH_MSG_CHAN1_NORM_POW_VALUE_LEN;
    
    uw_len = uw_len + SIZEOF_RRH_MONITOR_PARAM + p_param->uc_param_len;
    
    p_param = (struct rrh_monitor_param *)(p_msg + uw_len);
    
    p_param->us_param_type = RRH_MSG_CHAN2_NORM_POW_VALUE;
    p_param->uc_param_len = RRH_MSG_CHAN2_NORM_POW_VALUE_LEN;

    uw_len = uw_len + SIZEOF_RRH_MONITOR_PARAM + p_param->uc_param_len;

    p_param = (struct rrh_monitor_param *)(p_msg + uw_len);
    
    st_send_info.st_build_info.uw_msg_len = st_send_info.st_build_info.uw_msg_len
        + SIZEOF_RRH_MONITOR_PARAM
        + RRH_MSG_CHAN1_POWER_NORM_VALUE_LEN
        + SIZEOF_RRH_MONITOR_PARAM
        + RRH_MSG_CHAN2_POWER_NORM_VALUE_LEN;
    
    p_param->us_param_type = RRH_MSG_CHAN1_POWER_NORM_VALUE;
    p_param->uc_param_len = RRH_MSG_CHAN1_POWER_NORM_VALUE_LEN;
    
    uw_len = uw_len + SIZEOF_RRH_MONITOR_PARAM + p_param->uc_param_len;
    
    p_param = (struct rrh_monitor_param *)(p_msg + uw_len);
    
    p_param->us_param_type = RRH_MSG_CHAN2_POWER_NORM_VALUE;
    p_param->uc_param_len = RRH_MSG_CHAN2_POWER_NORM_VALUE_LEN;
    
    st_send_info.st_build_info.p_msg = p_msg;
    
    uw_ret = trans_rrh_send_msg_process(&st_send_info);
    if (TRANS_SUCCESS != uw_ret) 
    {   
     
        FLOG_ERROR("Call trans_rrh_send_msg_process error! uw_ret = %d\r\n", uw_ret);
        return TRANS_FAILD;     
    }   
   
    u_int8_t uc_result = 0;
    uw_ret = trans_msg_de_quene(&uc_result);
    
    if ((TRANS_SUCCESS != uw_ret) || (TRANS_SUCCESS != uc_result))
    {
        FLOG_ERROR("The response message from RRH error!result = %d, uw_ret. \r\n", uc_result, uw_ret);
        return TRANS_FAILD; 
    }
    
    return TRANS_SUCCESS;
}


/*****************************************************************************+
* Function: trans_rrh_send_msg_q_gps_lock()
* Description: 
* Parameters:
*           NONE
* Return Values:
*           TRANS_FAILD : 1
*           TRANS_SUCCESS  : 0
*
*  
*  Data:    2011-08-18
* 
+*****************************************************************************/
u_int32_t trans_rrh_send_msg_q_gps_lock(u_int8_t *p_msg, void *p_value)
{
    u_int32_t uw_ret = 0;
    struct trans_rrh_send_msg_info st_send_info;
    struct timeval tv;
    struct rrh_monitor_param *p_param = NULL;
    
    //u_int32_t uw_len = 0;
    //int w_value = 0;
    
    if (NULL == p_msg)
    {
        FLOG_ERROR("NULL PTR! \r\n");
        return TRANS_FAILD;      
    }  
    
    p_param = (struct rrh_monitor_param *)p_msg;
    
    memset((u_int8_t*)&st_send_info, 0, sizeof(struct trans_rrh_send_msg_info));
    
    gettimeofday(&tv, NULL);
    
    st_send_info.expired_tv.tv_sec = tv.tv_sec + 30;
    st_send_info.expired_tv.tv_usec = tv.tv_usec;
    
    st_send_info.w_rrh_sockfd = g_trans_rrh_socket;
    
    st_send_info.uw_src_moudle = TRANS_MOUDLE_BS;
    st_send_info.uc_block_flag = TRANS_QUENE_BLOCK;
    st_send_info.f_action = trans_rrh_block_msg_timer_func;
    
    st_send_info.st_build_info.uc_type = RRH_MONITOR_TYPE_QUERY;
    
    st_send_info.st_build_info.uw_param_num = 1;
    st_send_info.st_build_info.uw_msg_len = SIZEOF_RRH_MONITOR_PARAM
        + RRH_MSG_GPS_CLK_LOCK_VALUE_LEN;
    
    p_param->us_param_type = RRH_MSG_GPS_CLK_LOCK_VALUE;
    p_param->uc_param_len = RRH_MSG_GPS_CLK_LOCK_VALUE_LEN;
    
    //trans_debug_msg_print(p_msg, 10, g_trans_debug_rrh);
    
    st_send_info.st_build_info.p_msg = p_msg;
    
    uw_ret = trans_rrh_send_msg_process(&st_send_info);
    if (TRANS_SUCCESS != uw_ret) 
    {   
        FLOG_ERROR("Call trans_rrh_send_msg_process error! uw_ret = %d\r\n", uw_ret);
        return TRANS_FAILD;     
    }   
    
    u_int8_t uc_result = 0;
    uw_ret = trans_msg_de_quene(&uc_result);

    (*((int *)(p_value))) = uc_result;
    
    if (TRANS_SUCCESS != uw_ret)
    {
        FLOG_ERROR("The response message from RRH error!result = %d, uw_ret. \r\n", uc_result, uw_ret);
        return TRANS_FAILD; 
    }

    return TRANS_SUCCESS;

}

/*****************************************************************************+
* Function: trans_rrh_send_action_query()
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
u_int32_t trans_rrh_send_action_query(
                            struct trans_send_query_to_rrh *p_query_rrh,
                            size_t len,
                            u_int8_t * p_send_msg)
{
    u_int32_t uw_ret = 0;
    struct trans_rrh_send_msg_info st_send_info;

    len = len;
   
    if ((NULL == p_query_rrh) || (NULL == p_send_msg))
    {
        FLOG_ERROR("NULL PTR! \r\n");
        return TRANS_FAILD;      
    }  
    
    memset((u_int8_t*)&st_send_info, 0, sizeof(struct trans_rrh_send_msg_info));
   
    st_send_info.w_rrh_sockfd = g_trans_rrh_socket;
    
    st_send_info.uw_src_moudle = TRANS_MOUDLE_ACTION;
    st_send_info.uc_block_flag = TRANS_QUENE_NO_BLOCK;
    st_send_info.st_build_info.uc_type = RRH_MONITOR_TYPE_QUERY;

    uw_ret = trans_rrh_module_query_info(p_query_rrh, &st_send_info, p_send_msg);
    if (TRANS_SUCCESS != uw_ret) 
    {   
        FLOG_ERROR("Call trans_rrh_module_query_info error! uw_ret = %d\r\n", uw_ret);
        return TRANS_FAILD;     
    }   
    
    uw_ret = trans_rrh_send_msg_process(&st_send_info);
    if (TRANS_SUCCESS != uw_ret) 
    {   
        FLOG_ERROR("Call trans_rrh_send_msg_process error! uw_ret = %d\r\n", uw_ret);
        return TRANS_FAILD;     
    }   

    return TRANS_SUCCESS;

}


/*****************************************************************************+
* Function: trans_rrh_send_monitor_query()
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
u_int32_t trans_rrh_send_monitor_query(
                            struct trans_send_query_to_rrh *p_query_rrh,
                            size_t len,
                            u_int8_t * p_send_msg)
{
    u_int32_t uw_ret = 0;
    struct trans_rrh_send_msg_info st_send_info;
    
    len = len;  
    
    if ((NULL == p_query_rrh) || (NULL == p_send_msg))
    {
        FLOG_ERROR("NULL PTR! \r\n");
        return TRANS_FAILD;      
    }  
    
    FLOG_DEBUG("Enter! \r\n");    
        
    memset((u_int8_t*)&st_send_info, 0, sizeof(struct trans_rrh_send_msg_info));
   
    st_send_info.w_rrh_sockfd = g_trans_rrh_socket;
    
    st_send_info.uw_src_moudle = TRANS_MOUDLE_MONITOR;
    st_send_info.uc_block_flag = TRANS_QUENE_NO_BLOCK;
    st_send_info.st_build_info.uc_type = RRH_MONITOR_TYPE_QUERY;

    uw_ret = trans_rrh_module_query_info(p_query_rrh, &st_send_info, p_send_msg);
    if (TRANS_SUCCESS != uw_ret) 
    {   
        FLOG_ERROR("Call trans_rrh_module_query_info error! uw_ret = %d\r\n", uw_ret);
        return TRANS_FAILD;     
    }   
    
    uw_ret = trans_rrh_send_msg_process(&st_send_info);
    if (TRANS_SUCCESS != uw_ret) 
    {   
        FLOG_ERROR("Call trans_rrh_send_msg_process error! uw_ret = %d\r\n", uw_ret);
        return TRANS_FAILD;     
    }   
     
    FLOG_DEBUG("Exit! \r\n");   
    
    return TRANS_SUCCESS;

}

/*****************************************************************************+
* Function: trans_rrh_send_agent_query()
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
u_int32_t trans_rrh_send_agent_query(
                            struct trans_send_query_to_rrh *p_query_rrh,
                            size_t len,
                            u_int8_t * p_send_msg)
{
    u_int32_t uw_ret = 0;
    struct trans_rrh_send_msg_info st_send_info;
 
    len = len;
      
    if ((NULL == p_query_rrh) || (NULL == p_send_msg))
    {
        FLOG_ERROR("NULL PTR! \r\n");
        return TRANS_FAILD;      
    }  
    
    FLOG_DEBUG("Enter! \r\n");   
    
    memset((u_int8_t*)&st_send_info, 0, sizeof(struct trans_rrh_send_msg_info));
   
    st_send_info.w_rrh_sockfd = g_trans_rrh_socket;
    
    st_send_info.uw_src_moudle = TRANS_MOUDLE_AGENT;
    st_send_info.uc_block_flag = TRANS_QUENE_NO_BLOCK;
    st_send_info.st_build_info.uc_type = RRH_MONITOR_TYPE_QUERY;

    uw_ret = trans_rrh_module_query_info(p_query_rrh, &st_send_info, p_send_msg);
    if (TRANS_SUCCESS != uw_ret) 
    {   
        FLOG_ERROR("Call trans_rrh_module_query_info error! uw_ret = %d\r\n", uw_ret);
        return TRANS_FAILD;     
    }   
    
    uw_ret = trans_rrh_send_msg_process(&st_send_info);
    if (TRANS_SUCCESS != uw_ret) 
    {   
        FLOG_ERROR("Call trans_rrh_send_msg_process error! uw_ret = %d\r\n", uw_ret);
        return TRANS_FAILD;     
    }   
       
    FLOG_DEBUG("Exit! \r\n");   
    
    return TRANS_SUCCESS;

}


/*****************************************************************************+
* Function: trans_rrh_send_wireless_query()
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
u_int32_t trans_rrh_send_wireless_query(
                            struct trans_send_query_to_rrh *p_query_rrh,
                            size_t len,
                            u_int8_t * p_send_msg)
{
    u_int32_t uw_ret = 0;
    struct trans_rrh_send_msg_info st_send_info;
    
    len = len;
    
    if ((NULL == p_query_rrh) || (NULL == p_send_msg))
    {
        FLOG_ERROR("NULL PTR! \r\n");
        return TRANS_FAILD;      
    }  
    
    memset((u_int8_t*)&st_send_info, 0, sizeof(struct trans_rrh_send_msg_info));
   
    st_send_info.w_rrh_sockfd = g_trans_rrh_socket;
    
    st_send_info.uw_src_moudle = TRANS_MOUDLE_BS;
    st_send_info.uc_block_flag = TRANS_QUENE_NO_BLOCK;
    st_send_info.st_build_info.uc_type = RRH_MONITOR_TYPE_QUERY;

    uw_ret = trans_rrh_module_query_info(p_query_rrh, &st_send_info, p_send_msg);
    if (TRANS_SUCCESS != uw_ret) 
    {   
        FLOG_ERROR("Call trans_rrh_module_query_info error! uw_ret = %d\r\n", uw_ret);
        return TRANS_FAILD;     
    }   
    
    uw_ret = trans_rrh_send_msg_process(&st_send_info);
    if (TRANS_SUCCESS != uw_ret) 
    {   
        FLOG_ERROR("Call trans_rrh_send_msg_process error! uw_ret = %d\r\n", uw_ret);
        return TRANS_FAILD;     
    }   
    
    return TRANS_SUCCESS;

}

/*****************************************************************************+
* Function: trans_rrh_send_action_cfg()
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
u_int32_t trans_rrh_send_action_cfg(
                            struct trans_send_cfg_to_rrh *p_cfg_rrh,
                            size_t len,
                            u_int8_t * p_send_msg)
{
    u_int32_t uw_ret = 0;
    struct trans_rrh_send_msg_info st_send_info;
 
    len = len;
    
    FLOG_DEBUG("Enter! \r\n");

    if ((NULL == p_cfg_rrh) || (NULL == p_send_msg))
    {
        FLOG_ERROR("NULL PTR! \r\n");
        return TRANS_FAILD;      
    }  
    
    memset((u_int8_t*)&st_send_info, 0, sizeof(struct trans_rrh_send_msg_info));
   
    st_send_info.w_rrh_sockfd = g_trans_rrh_socket;
    
    st_send_info.uw_src_moudle = TRANS_MOUDLE_ACTION;
    st_send_info.uc_block_flag = TRANS_QUENE_NO_BLOCK;
    st_send_info.st_build_info.uc_type = RRH_MONITOR_TYPE_CONFIG;

    uw_ret = trans_rrh_module_cfg_info(p_cfg_rrh, &st_send_info, p_send_msg);
    if (TRANS_SUCCESS != uw_ret) 
    {   
        FLOG_ERROR("Call trans_rrh_module_cfg_info error! uw_ret = %d\r\n", uw_ret);
        return TRANS_FAILD;     
    }   
    
    uw_ret = trans_rrh_send_msg_process(&st_send_info);
    if (TRANS_SUCCESS != uw_ret) 
    {   
        FLOG_ERROR("Call trans_rrh_send_msg_process error! uw_ret = %d\r\n", uw_ret);
        return TRANS_FAILD;     
    }   
    
    FLOG_DEBUG("Exit ! \r\n");
    
    return TRANS_SUCCESS;

}


/*****************************************************************************+
* Function: trans_rrh_send_monitor_cfg()
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
u_int32_t trans_rrh_send_monitor_cfg(
                            struct trans_send_cfg_to_rrh *p_cfg_rrh,
                            size_t len,
                            u_int8_t * p_send_msg)
{
    u_int32_t uw_ret = 0;
    struct trans_rrh_send_msg_info st_send_info;
 
    len = len;
    
    FLOG_DEBUG("Enter! \r\n");
        
    if ((NULL == p_cfg_rrh) || (NULL == p_send_msg))
    {
        FLOG_ERROR("NULL PTR! \r\n");
        return TRANS_FAILD;      
    }  
    
    memset((u_int8_t*)&st_send_info, 0, sizeof(struct trans_rrh_send_msg_info));
   
    st_send_info.w_rrh_sockfd = g_trans_rrh_socket;
    
    st_send_info.uw_src_moudle = TRANS_MOUDLE_MONITOR;
    st_send_info.uc_block_flag = TRANS_QUENE_NO_BLOCK;
    st_send_info.st_build_info.uc_type = RRH_MONITOR_TYPE_CONFIG;

    uw_ret = trans_rrh_module_cfg_info(p_cfg_rrh, &st_send_info, p_send_msg);
    if (TRANS_SUCCESS != uw_ret) 
    {   
        FLOG_ERROR("Call trans_rrh_module_cfg_info error! uw_ret = %d\r\n", uw_ret);
        return TRANS_FAILD;     
    }   
    
    uw_ret = trans_rrh_send_msg_process(&st_send_info);
    if (TRANS_SUCCESS != uw_ret) 
    {   
        FLOG_ERROR("Call trans_rrh_send_msg_process error! uw_ret = %d\r\n", uw_ret);
        return TRANS_FAILD;     
    }   

    FLOG_DEBUG("Exit ! \r\n");
    
    return TRANS_SUCCESS;

}

/*****************************************************************************+
* Function: trans_rrh_send_agent_cfg()
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
u_int32_t trans_rrh_send_agent_cfg(
                            struct trans_send_cfg_to_rrh *p_cfg_rrh,
                            size_t len,
                            u_int8_t * p_send_msg)
{
    u_int32_t uw_ret = 0;
    struct trans_rrh_send_msg_info st_send_info;;

    len = len;
    
    FLOG_DEBUG("Enter! \r\n");

    if ((NULL == p_cfg_rrh) || (NULL == p_send_msg))
    {
        FLOG_ERROR("NULL PTR! \r\n");
        return TRANS_FAILD;      
    }  
    
    memset((u_int8_t*)&st_send_info, 0, sizeof(struct trans_rrh_send_msg_info));
   
    st_send_info.w_rrh_sockfd = g_trans_rrh_socket;
    
    st_send_info.uw_src_moudle = TRANS_MOUDLE_AGENT;
    st_send_info.uc_block_flag = TRANS_QUENE_NO_BLOCK;
    st_send_info.st_build_info.uc_type = RRH_MONITOR_TYPE_CONFIG;

    uw_ret = trans_rrh_module_cfg_info(p_cfg_rrh, &st_send_info, p_send_msg);
    if (TRANS_SUCCESS != uw_ret) 
    {   
        FLOG_ERROR("Call trans_rrh_module_cfg_info error! uw_ret = %d\r\n", uw_ret);
        return TRANS_FAILD;     
    }   
    
    uw_ret = trans_rrh_send_msg_process(&st_send_info);
    if (TRANS_SUCCESS != uw_ret) 
    {   
        FLOG_ERROR("Call trans_rrh_send_msg_process error! uw_ret = %d\r\n", uw_ret);
        return TRANS_FAILD;     
    }   
    
    FLOG_DEBUG("Exit ! \r\n");
    
    return TRANS_SUCCESS;

}

/*****************************************************************************+
* Function: trans_rrh_send_wireless_cfg()
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
u_int32_t trans_rrh_send_wireless_cfg(
                            struct trans_send_cfg_to_rrh *p_cfg_rrh,
                            size_t len,
                            u_int8_t * p_send_msg)
{
    u_int32_t uw_ret = 0;
    struct trans_rrh_send_msg_info st_send_info;
    
    len = len;
    
    FLOG_DEBUG("Enter! \r\n"); 
    
    if ((NULL == p_cfg_rrh) || (NULL == p_send_msg))
    {
        FLOG_ERROR("NULL PTR! \r\n");
        return TRANS_FAILD;      
    }  
    
    memset((u_int8_t*)&st_send_info, 0, sizeof(struct trans_rrh_send_msg_info));
   
    st_send_info.w_rrh_sockfd = g_trans_rrh_socket;
    
    st_send_info.uw_src_moudle = TRANS_MOUDLE_BS;
    st_send_info.uc_block_flag = TRANS_QUENE_NO_BLOCK;
    st_send_info.st_build_info.uc_type = RRH_MONITOR_TYPE_CONFIG;

    uw_ret = trans_rrh_module_cfg_info(p_cfg_rrh, &st_send_info, p_send_msg);
    if (TRANS_SUCCESS != uw_ret) 
    {   
        FLOG_ERROR("Call trans_rrh_module_cfg_info error! uw_ret = %d\r\n", uw_ret);
        return TRANS_FAILD;     
    }   
    
    uw_ret = trans_rrh_send_msg_process(&st_send_info);
    if (TRANS_SUCCESS != uw_ret) 
    {   
        FLOG_ERROR("Call trans_rrh_send_msg_process error! uw_ret = %d\r\n", uw_ret);
        return TRANS_FAILD;     
    }   


    FLOG_DEBUG("Exit ! \r\n");
    
    return TRANS_SUCCESS;

}


#if 0

/*****************************************************************************+
* Function: trans_rrh_send_msg_metric()
* Description: 
* Parameters:
*           NONE
* Return Values:
*           TRANS_FAILD : 1
*           TRANS_SUCCESS  : 0
*
*  
*  Data:    2011-07-12
* 
+*****************************************************************************/
u_int32_t trans_rrh_send_msg_metric(u_int8_t  *p_send_msg, 
                                             struct trans_rrh_metric_info *p_metric_info)
{
    u_int32_t uw_ret = 0;
    struct trans_rrh_send_msg_info st_send_info;
    struct timeval tv;
    struct rrh_monitor_param *p_param = NULL;
    
    if (NULL == p_send_msg)
    {
    
        FLOG_ERROR("NULL PTR! \r\n");
        return TRANS_FAILD;      
    }  
    
    p_param = (struct rrh_monitor_param *)p_send_msg;
    
    memset((u_int8_t*)&st_send_info, 0, sizeof(struct trans_rrh_send_msg_info));
    
    gettimeofday(&tv, NULL);
    
    st_send_info.expired_tv.tv_sec = tv.tv_sec + 10;
    st_send_info.expired_tv.tv_usec = tv.tv_usec;
    
    st_send_info.w_rrh_sockfd = g_trans_rrh_socket;
    
    st_send_info.uw_src_moudle = TRANS_MOUDLE_AGENT;
    st_send_info.uc_block_flag = TRANS_QUENE_NO_BLOCK;
    st_send_info.f_action = trans_rrh_metric_timer_func;
    
    st_send_info.st_build_info.uc_type = RRH_MONITOR_TYPE_QUERY;
    
    st_send_info.st_build_info.uw_param_num = 1;
    st_send_info.st_build_info.uw_msg_len = p_metric_info->uc_metric_len + SIZEOF_RRH_MONITOR_PARAM;

    //st_send_info.u_extra_info.w_source_id = p_metric_info->uw_source_id;
    memcpy(&(st_send_info.u_extra_info.st_agent_metric), p_metric_info->p_agent_metric, 
                                                            SIZEOF_TRANS_AGENT_METRIC_INFO);

    p_param->us_param_type = p_metric_info->us_metric_id;
    p_param->uc_param_len = p_metric_info->uc_metric_len;

    //trans_debug_msg_print(p_msg, 10, g_trans_debug_rrh);
    
    st_send_info.st_build_info.p_msg = p_send_msg;
    
    uw_ret = trans_rrh_send_msg_process(&st_send_info);
    if (TRANS_SUCCESS != uw_ret) 
    {   
     
        FLOG_ERROR("Call trans_rrh_send_msg_process error! uw_ret = %d\r\n", uw_ret);
        return TRANS_FAILD;     
    }   
    
      
    return TRANS_SUCCESS;

}



/*****************************************************************************+
* Function: trans_rrh_get_agent_metric_id()
* Description: 
* Parameters:
*           NONE
* Return Values:
*           TRANS_FAILD : 1
*           TRANS_SUCCESS  : 0
*
*  
*  Data:    2011-07-12
* 
+*****************************************************************************/
u_int32_t trans_rrh_get_agent_metric_id(u_int32_t uw_rrh_metric_id,
                                                                int32_t *p_agent_metric_id)

{
    switch (uw_rrh_metric_id)
    {
        /*Power Amplifier Temperature*/ 
        case RRH_MSG_PA_TEMP_VALUE: 
    
            *p_agent_metric_id = TRANS_AGENT_RRH_METRIC_ID_PAT;
            break; 
    
        /*Downlink Output Power for channel 1#*/
        case RRH_MSG_DL_INPUT1_LEVEL: 
    
            *p_agent_metric_id = TRANS_AGENT_RRH_METRIC_ID_CH1_PWR;
       
            break; 
    
        /*Downlink Output Power for channel 2#*/
        case RRH_MSG_DL_INPUT2_LEVEL: 
          
            *p_agent_metric_id = TRANS_AGENT_RRH_METRIC_ID_CH2_PWR;

            break;      
    
        /*Downlink Voltage Standing Wave Radio (VSWR) for channel 1#*/    
        case RRH_MSG_DL_VSWR1_VALUE: 
          
            *p_agent_metric_id = TRANS_AGENT_RRH_METRIC_ID_CH1_VSWR;

            break;    
    
        /*Downlink Voltage Standing Wave Radio (VSWR) for channel 2#*/    
        case RRH_MSG_DL_VSWR2_VALUE: 
    
            *p_agent_metric_id = TRANS_AGENT_RRH_METRIC_ID_CH2_VSWR;

            break;    
            
        default:
    
            FLOG_ERROR("Rev unknow metric_id! w_metric_id = %d\r\n", uw_rrh_metric_id);
    
            return TRANS_FAILD;
    }  
    
    
    FLOG_DEBUG("Exit : p_agent_metric_id = %d. \r\n", 
                            *p_agent_metric_id);
    
    return TRANS_SUCCESS;

}

/*****************************************************************************+
* Function: trans_rrh_get_metric_param()
* Description: 
* Parameters:
*           NONE
* Return Values:
*           TRANS_FAILD : 1
*           TRANS_SUCCESS  : 0
*
*  
*  Data:    2011-07-12
* 
+*****************************************************************************/
u_int32_t trans_rrh_get_metric_param(int w_agent_metric_id,
                                                                u_int16_t *p_rrh_metric_id, 
                                                                u_int8_t *p_rrh_metric_len)
{
    switch (w_agent_metric_id)
    {
        /*Power Amplifier Temperature*/ 
        case TRANS_AGENT_RRH_METRIC_ID_PAT: 

            *p_rrh_metric_id = RRH_MSG_PA_TEMP_VALUE;
            *p_rrh_metric_len = RRH_MSG_PA_TEMP_VALUE_LEN;
            break; 
    
        /*Downlink Output Power for channel 1#*/
        case TRANS_AGENT_RRH_METRIC_ID_CH1_PWR:

            *p_rrh_metric_id = RRH_MSG_DL_INPUT1_LEVEL;
            *p_rrh_metric_len = RRH_MSG_DL_INPUT1_LEVEL_LEN;            
            break; 
    
        /*Downlink Output Power for channel 2#*/
        case TRANS_AGENT_RRH_METRIC_ID_CH2_PWR: 
          
            *p_rrh_metric_id = RRH_MSG_DL_INPUT2_LEVEL;
            *p_rrh_metric_len = RRH_MSG_DL_INPUT2_LEVEL_LEN;
            break;      

        /*Downlink Voltage Standing Wave Radio (VSWR) for channel 1#*/    
        case TRANS_AGENT_RRH_METRIC_ID_CH1_VSWR: 
          
            *p_rrh_metric_id = RRH_MSG_DL_VSWR1_VALUE;
            *p_rrh_metric_len = RRH_MSG_DL_VSWR1_VALUE_LEN;
            break;    

        /*Downlink Voltage Standing Wave Radio (VSWR) for channel 2#*/    
        case TRANS_AGENT_RRH_METRIC_ID_CH2_VSWR: 

            *p_rrh_metric_id = RRH_MSG_DL_VSWR2_VALUE;
            *p_rrh_metric_len = RRH_MSG_DL_VSWR2_VALUE_LEN;
            break;    
            
        default:
    
            FLOG_ERROR("Rev unknow metric_id! w_metric_id = %d\r\n", w_agent_metric_id);
    
            return TRANS_FAILD;
    }  
    
    
    FLOG_DEBUG("Exit : uw_rrh_metric_id = %d, uw_rrh_metric_len = %d. \r\n", 
                            *p_rrh_metric_id, *p_rrh_metric_len);

    return TRANS_SUCCESS;

}

/*****************************************************************************+
* Function: trans_rrh_forward_agent_metric()
* Description: 
* Parameters:
*           NONE
* Return Values:
*           TRANS_FAILD : 1
*           TRANS_SUCCESS  : 0
*
*  
*  Data:    2011-07-12
* 
+*****************************************************************************/
u_int32_t trans_rrh_forward_agent_metric(u_int8_t  *p_send_msg, 
                            struct trans_agent_metric_info *p_agent_metric_info)
{
    u_int32_t uw_ret = 0;
    //u_int32_t uw_metric_id = 0;
    //u_int32_t uw_metric_len = 0;
    struct trans_rrh_metric_info st_metric_info;
    
    if ((NULL == p_send_msg) || (NULL == p_agent_metric_info))
    {
        FLOG_ERROR("NULL PTR! \r\n");
        return TRANS_FAILD;      
    }    

    /*Get Info*/
    uw_ret = trans_rrh_get_metric_param(p_agent_metric_info->w_metric_id, 
                                        &st_metric_info.us_metric_id, &st_metric_info.uc_metric_len);
    if (TRANS_SUCCESS != uw_ret) 
    {   
     
        FLOG_ERROR("Call trans_rrh_get_metric_param error! uw_ret = %d\r\n", uw_ret);
        return TRANS_FAILD;     
    } 

    st_metric_info.p_agent_metric = p_agent_metric_info;
    
    /*Snd message*/
    uw_ret = trans_rrh_send_msg_metric(p_send_msg, &st_metric_info);
    if (TRANS_SUCCESS != uw_ret) 
    {   
     
        FLOG_ERROR("Call trans_rrh_send_msg_metric error! uw_ret = %d\r\n", uw_ret);
        return TRANS_FAILD;     
    } 

    return TRANS_SUCCESS;   
    
}

/*****************************************************************************+
* Function: trans_rrh_forward_rrh_metric()
* Description: 
* Parameters:
*           NONE
* Return Values:
*           TRANS_FAILD : 1
*           TRANS_SUCCESS  : 0
*
*  
*  Data:    2011-07-12
* 
+*****************************************************************************/
u_int32_t trans_rrh_forward_rrh_metric(u_int8_t  *p_send_msg, int32_t w_sockfd,
    struct trans_agent_metric_info *p_agent_metric, u_int32_t uw_metric_id, int32_t w_metric_value)
{
    u_int32_t uw_ret = 0;
    //u_int32_t uw_agent_metric_id = 0;

    if (NULL == p_send_msg)
    {
        FLOG_ERROR("NULL PTR! \r\n");
        return TRANS_FAILD;      
    }    
    
    /*Convert the Metric ID*/
    uw_ret = trans_rrh_get_agent_metric_id(uw_metric_id, &(p_agent_metric->w_metric_id));
    if (TRANS_SUCCESS != uw_ret) 
    {   
        FLOG_ERROR("Call trans_rrh_get_agent_metric_id error! uw_ret = %d\r\n", uw_ret);
        return TRANS_FAILD;     
    } 

    #ifdef TRANS_AGENT_COMPILE
    struct trans_agent_metric_msg st_metric_msg;
    
    //memcpy(&(st_metric_msg.st_metric), p_agent_metric, SIZEOF_TRANS_AGENT_METRIC_INFO);
    st_metric_msg.p_agent_metric = p_agent_metric;
    st_metric_msg.w_metric_val = w_metric_value;
    
    pthread_mutex_lock(&(g_trans_agent_metric_mutex));  
    /*Build and Send The Message To Agent (C&M)*/
    uw_ret = trans_agent_send_metric_msg(&st_metric_msg, p_send_msg, w_sockfd);
    if (TRANS_SUCCESS != uw_ret) 
    {   
        FLOG_ERROR("Call trans_agent_send_metric_msg error! uw_ret = %d\r\n", uw_ret);
        return TRANS_FAILD;     
    }

    pthread_mutex_unlock(&(g_trans_agent_metric_mutex));
    
    #else
    
    w_sockfd = w_sockfd;
    p_send_msg = p_send_msg;
    w_metric_value = w_metric_value;
    p_agent_metric = p_agent_metric;

    #endif

    return TRANS_SUCCESS;   

}
#endif

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

    //struct timeval tv;

    FLOG_DEBUG("Enter \r\n");
   
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
    st_server_addr.sin_port = htons(g_trans_rrh_eqp_config.us_ser_tcp_port);     
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
        
            //printf("new client[%d] %s:%d\n", conn_num, inet_ntoa(client_addr.sin_addr), ntohs(client_addr.sin_port));
            g_trans_moudle_socket_fd[TRANS_MOUDLE_RRH] = w_client_socket;
            g_trans_rrh_socket = w_client_socket;
        }
    }else
    {
        /*Accept the connection->Connect establish*/
        w_client_socket = accept(w_server_socket, (struct sockaddr *)&st_client_addr, &sin_size);
            
        if (w_client_socket <= 0)
        {
        
            FLOG_ERROR("Accept error! w_ret = %d\r\n", w_ret);
            close (w_server_socket);
            return TRANS_FAILD;
        }  
    
        //printf("new client[%d] %s:%d\n", conn_num, inet_ntoa(client_addr.sin_addr), ntohs(client_addr.sin_port));
        g_trans_moudle_socket_fd[TRANS_MOUDLE_RRH] = w_client_socket;
        g_trans_rrh_socket = w_client_socket;
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
    
    FLOG_INFO("Enter \r\n");

    if ((NULL == p_req_msg) || (NULL == p_rep_msg) )
    {
        FLOG_ERROR("NULL PTR! \r\n");
        return TRANS_FAILD;  
    }
    
    /*analysis the msg from RRH */
    p_request_msg = (struct rrh_conn_req_msg *)p_req_msg;

    uw_rru_id = ntohl(p_request_msg->uw_rru_id);

    FLOG_DEBUG("uw_rru_id = %d \r\n", uw_rru_id);
    
    memset((u_int8_t*)a_mac_addr, 0, RRH_MAC_ADDER_LEN);    
    /*Destinate MAC*/
    memcpy(a_mac_addr, p_request_msg->a_mac_addr, RRH_MAC_ADDER_LEN);

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

    p_response_msg->uw_rru_id = htonl(uw_rru_id);    /*RRU ID*/
    p_response_msg->uc_server_id = g_trans_rrh_eqp_config.uc_server_id;   /*SERVER ID*/

    /*RRU MAC */
    memcpy(p_response_msg->a_rrh_mac_addr, a_mac_addr, RRH_MAC_ADDER_LEN);

    /*RRU  IP*/
    memcpy(p_response_msg->a_rrh_ip_addr, &(g_trans_rrh_eqp_config.uw_rrh_ip_addr), RRH_IP_ADDER_LEN);

    /*RRU  mask*/
    memcpy(p_response_msg->a_rrh_mask_addr, &(g_trans_rrh_eqp_config.uw_rrh_mask_addr), RRH_SUBNET_MASK_LEN);

    /*SERVER  IP*/
    memcpy(p_response_msg->a_ser_ip_addr, &(g_trans_rrh_eqp_config.uw_ser_ip_addr), RRH_IP_ADDER_LEN);
    
    p_response_msg->us_ser_tcp_port = htons(g_trans_rrh_eqp_config.us_ser_tcp_port); /*SERVE  TCP  PORT*/
    p_response_msg->us_ser_data_port = htons(g_trans_rrh_eqp_config.us_ser_data_udp_port); /*SERVER  I/Q Data  PORT*/

    //*p_new_msg_len = SIZEOF_RRH_CONN_REP_MSG;
    
   FLOG_DEBUG("Exit \r\n"); 
    
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
    sll.sll_protocol = htons(ETH_P_ALL);

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

    FLOG_DEBUG("Enter \r\n");
    
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
    st_serveraddress.sin_port = htons(g_trans_rrh_eqp_config.us_ser_udp_port); 
    //st_serveraddress.sin_addr.s_addr = g_trans_rrh_eqp_config.uw_ser_ip_addr; 
    //st_serveraddress.sin_addr.s_addr = g_trans_rrh_server_socket.uw_ipAddr;
    //st_serveraddress.sin_addr.s_addr = inet_addr("9.186.57.90");
    st_serveraddress.sin_addr.s_addr = htonl(INADDR_ANY);
    //st_serveraddress.sin_addr.s_addr = g_trans_rrh_eqp_config.uw_ser_ip_addr;

    if((bind(w_serversocket, (struct sockaddr *)&st_serveraddress, sizeof(st_serveraddress))) < 0) 
    {
        FLOG_ERROR("Bind error!\r\n");
        
        close(w_serversocket);
        return TRANS_FAILD;
    }

    st_clientaddress.sin_family = AF_INET; 
    st_clientaddress.sin_port = htons(g_trans_rrh_eqp_config.us_rrh_udp_port); 
    //st_clientaddress.sin_addr.s_addr = htonl(INADDR_BROADCAST); 
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

    
    struct timeval st_time_val;      
    #if 0
    fd_set readfds;
    struct timeval end_tv;
    struct timeval st_time_val;  
    #endif
    
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

    void* p_timer_id = NULL;
    struct trans_timer_msg_info st_timer_info;
    
    /*analysis the msg from RRH */

    FLOG_DEBUG("Enter \r\n");

    if((g_trans_rrh_udp_raw_socket = (socket(AF_PACKET, SOCK_RAW, htons(ETH_P_ALL)))) < 0) 
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

    #if 0
    st_serveraddress.sin_family = AF_INET; 
    st_serveraddress.sin_port = htons(g_trans_rrh_eqp_config.us_ser_udp_port); 
    //st_serveraddress.sin_addr.s_addr = g_trans_rrh_eqp_config.uw_ser_ip_addr; 
    //st_serveraddress.sin_addr.s_addr = g_trans_rrh_server_socket.uw_ipAddr;
    //st_serveraddress.sin_addr.s_addr = inet_addr("9.186.57.90");
    st_serveraddress.sin_addr.s_addr = htonl(INADDR_ANY);
    
    st_clientaddress.sin_family = AF_INET; 
    st_clientaddress.sin_port = htons(g_trans_rrh_eqp_config.us_rrh_udp_port); 
    //st_clientaddress.sin_addr.s_addr = htonl(INADDR_BROADCAST); 
    //st_clientaddress.sin_addr.s_addr = g_trans_rrh_client_socket.uw_ipAddr; 
    st_clientaddress.sin_addr.s_addr = g_trans_rrh_eqp_config.uw_ser_broc_addr;
    #endif

    gettimeofday(&st_time_val, NULL);
    /*Timeout -- 10s*/
    st_time_val.tv_sec = st_time_val.tv_sec + 15;
    //st_time_val.tv_usec = st_time_val.tv_usec;
    
    st_timer_info.us_serial_number = 0;
    st_timer_info.uw_src_moudle = TRANS_MOUDLE_LOCAL;
    st_timer_info.uc_block_flag = TRANS_QUENE_NO_BLOCK;
    st_timer_info.f_callback = trans_rrh_udp_raw_socket_timer_func;
    st_timer_info.p_user_info = NULL;
    
    uw_ret = trans_timer_add(&st_time_val,
                             trans_rrh_udp_raw_socket_timer_func,
                             &st_timer_info,
                             sizeof (struct trans_timer_msg_info),
                             &st_timer_info,
                             &p_timer_id);
    
    if (TRANS_SUCCESS != uw_ret)
    {
        FLOG_ERROR("Call trans_timer_add error! uw_ret = %d\r\n", uw_ret);
        return TRANS_FAILD;
    }
    
    //FLOG_ERROR("g_trans_rrh_udp_raw_socket = %d\r\n", g_trans_rrh_udp_raw_socket);

    
    while (1) 
    { 
     
        //r = recvfrom(sock, (char *)buf, sizeof(buf), 0, (struct sockaddr *)&addr, &uw_len);
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
            
            w_src_port = ntohs(p_udp->source);
            w_dest_port = ntohs(p_udp->dest);

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
                    trans_timer_delete(p_timer_id);
                    
                    FLOG_INFO("Exit :RAW UDP socket finished.Close UDP socket. \r\n");  
                    close(g_trans_rrh_udp_raw_socket);
                    g_trans_rrh_udp_raw_socket = -1;
                    
                    return TRANS_SUCCESS; 
                }
            }
            else
            {
                 FLOG_DEBUG("Port is diff:%d, %d, %d, %d \r\n", 
                                    g_trans_rrh_eqp_config.us_rrh_udp_port, w_src_port,
                                    g_trans_rrh_eqp_config.us_ser_udp_port, w_dest_port);            
                continue;
            }

        }
        else
        {
            FLOG_DEBUG("Other pkt, protocl:%d\n", p_ip->protocol);            
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

    FLOG_DEBUG("Enter \r\n");
    
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
    st_serveraddress.sin_port = htons(g_trans_rrh_eqp_config.us_ser_udp_port); 
    //st_serveraddress.sin_addr.s_addr = g_trans_rrh_eqp_config.uw_ser_ip_addr; 
    //st_serveraddress.sin_addr.s_addr = g_trans_rrh_server_socket.uw_ipAddr;
    //st_serveraddress.sin_addr.s_addr = inet_addr("9.186.57.90");
    st_serveraddress.sin_addr.s_addr = htonl(INADDR_ANY);
    //st_serveraddress.sin_addr.s_addr = g_trans_rrh_eqp_config.uw_ser_ip_addr;

    if((bind(w_serversocket, (struct sockaddr *)&st_serveraddress, sizeof(st_serveraddress))) < 0) 
    {
        FLOG_ERROR("Bind error!\r\n");
        
        close(w_serversocket);
        return TRANS_FAILD;
    }

    st_clientaddress.sin_family = AF_INET; 
    st_clientaddress.sin_port = htons(g_trans_rrh_eqp_config.us_rrh_udp_port); 
    //st_clientaddress.sin_addr.s_addr = htonl(INADDR_BROADCAST); 
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
    FLOG_DEBUG("Exit \r\n");  
    
    return TRANS_SUCCESS; 
}
#endif

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

    g_trans_rrh_eqp_config.us_ser_tcp_port = p_init_info->us_ser_tcp_port; /*SERVER  TCP Port*/
    g_trans_rrh_eqp_config.us_ser_udp_port = RRH_SERVER_UDP_PORT; /*SERVER  UDP Port*/
    g_trans_rrh_eqp_config.us_rrh_tcp_port = RRH_RRU_TCP_PORT; /*RRH  TCP Port*/
    g_trans_rrh_eqp_config.us_rrh_udp_port = RRH_RRU_UDP_PORT; /*RRH  UDP Port*/
    g_trans_rrh_eqp_config.us_rrh_data_udp_port = RRH_RRU_DATA_UDP_PORT; /*RRH  data UDP Port*/
    g_trans_rrh_eqp_config.us_ser_data_udp_port = p_init_info->us_ser_data_port; /*SERVER  I/Q  Data Port*/ 

    strcpy((char *)(g_trans_rrh_eqp_config.a_rrh_nic_if), (char *)(p_init_info->a_rrh_nic_if));

    g_trans_rrh_socket = -1;

    #if 0
    if (0 != create_sem (&g_trans_rrh_msg_sem))
    {
        FLOG_ERROR ("Enable msg sem error");
        return TRANS_FAILD;
    }
    #endif
    
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

    return TRANS_SUCCESS;
}

#endif

