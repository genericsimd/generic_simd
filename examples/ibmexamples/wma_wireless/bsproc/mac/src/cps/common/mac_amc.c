/* ----------------------------------------------------------------------------
   IBM Confidential

   IBM Wireless M2M Platform   

   (C)Copyright IBM Corp. 2008, 2009, 2010, 2011

   All Rights Reserved.

   File Name: mac_amc.c

   Change Activity:

   Date             Description of Change                   By
   -----------      ---------------------					--------
   1-Oct.2008       Created                                 Partha Dutta
   1-Sep.2012	    Modified				    Mukundan Madhavan
	//it initializes the shared CINR table between BS MAC and PHY

   ----------------------------------------------------------------------------
   PROLOG END TAG zYx                                                         */

#include "memmgmt.h"
#include "mac.h"
#include "phy_params.h"
#include "mac_amc.h"
#include "dl_exp_params.h"
#include "flog.h"

//dump file
#include "dump_util.h"

int init_shared_tables()
{
	int i;
	for (i=0;i< TABLE_SIZE;i++)
	{
		shared_cinr_table[i][0] = 0;
		shared_rssi_table[i][0] = 0;
		shared_crc_table[i][0] = 0.0;
		average_cinr_table[i][0]=(float)(-5);//-5 is out of bound. means it ws never updated before
                prior_average_cinr_table[i][0]=(float)(-5);
		average_rssi_table[i][0]=0.0;
		crc_error_count_table[i][0]=0;
		total_crc_cinr_adjust[i][0] = 0;
                rep_cinr_adjust[i][0] = 0;
                current_mcs_table[i][0] = QPSK_12;


#ifndef SS_TX
	//There is index 1 only if this is BS
		shared_cinr_table[i][1] = 0;
		shared_rssi_table[i][1] = 0;
		shared_crc_table[i][1] = 0.0;
		average_cinr_table[i][1]=(float)(-5);
                prior_average_cinr_table[i][1]=(float)(-5);
		average_rssi_table[i][1]=0.0;
		crc_error_count_table[i][1]=0;
		total_crc_cinr_adjust[i][1] = 0;
                rep_cinr_adjust[i][1] = 0;
		cpe_packet_count[i] = 0;
                cpe_crc_error_count[i] = 0;
		num_rep_retries_left[i] = NUM_REP_REQ_RETRIES;
                current_mcs_table[i][1] = QPSK_12;
#endif

	}
	pthread_mutex_init(&cinr_table_lock, NULL);
	pthread_mutex_init(&crc_table_lock, NULL);

	return 0;
}

int reset_rep_cinr_adjust(int basic_cid)
{
   int index = basic_cid - BASIC_CID_MIN_VALUE;

   if ((index >= TABLE_SIZE) || (index < 0))
   {
       return -1;
   }

   rep_cinr_adjust[index][0] = 0;
   FLOG_DEBUG("Rep Cinr Reset.........\n");

}

int update_rep_cinr_adjust(int basic_cid)
{
   int index = basic_cid - BASIC_CID_MIN_VALUE;

   if ((index >= TABLE_SIZE) || (index < 0))
   {
       return -1;
   }
   
   rep_cinr_adjust[index][0] += 3;
   if(rep_cinr_adjust[index][0] >= 20)
   {
   	rep_cinr_adjust[index][0] = 20;
   }
   FLOG_INFO("update cid: %d with rep cinr adjust: %d\n", basic_cid, rep_cinr_adjust[index][0]);
}

int update_ss_ul_link_quality(int basic_cid, int cinr, int rssi)
{
	int index = basic_cid - BASIC_CID_MIN_VALUE;

	if ((index >= TABLE_SIZE) || (index < 0))
	{
		return -1;
	}
	FLOG_DEBUG("update cid: %d with cinr: %d\n", basic_cid, cinr);
	//int temp;
	pthread_mutex_lock(&cinr_table_lock);
	//First, update current cinr/rssi values to shared table.
	shared_cinr_table[index][1] = cinr;
	shared_rssi_table[index][1] = rssi;

	//Now, take care of updating average ul cinr.
	if (average_cinr_table[index][1] == (float)(-5))
	{
		average_cinr_table[index][1] = (float)shared_cinr_table[index][1];
	}
	else
	{
		//Do averaging according to scheme 2.
                prior_average_cinr_table[index][1] =  average_cinr_table[index][1];
		//average_cinr_table[index][1] = ALPHA_CINR*(float)cinr + (1 - ALPHA_CINR)*average_cinr_table[index][1];
                average_cinr_table[index][1] = 10*log10( ALPHA_CINR* pow(10,(float)cinr /10.0)+ (1 - ALPHA_CINR)*pow(10,(float)average_cinr_table[index][1]/10.0));
	}
	FLOG_DEBUG("Update SS UL Link Quality for CID %d CINR inputted %d Average CINR %f\n",basic_cid, cinr, average_cinr_table[index][1]);
	pthread_mutex_unlock(&cinr_table_lock);
	return 0;
}

int update_ss_dl_link_quality_cpe(int basic_cid, int cinr, int rssi)
{
#ifdef SS_TX
	int index = 0;
#else
	int index = basic_cid - BASIC_CID_MIN_VALUE;
	if ((index >= TABLE_SIZE) || (index < 0))
        {
                return -1;
        }
#endif

	pthread_mutex_lock(&cinr_table_lock);

	//First, update current cinr/rssi values to shared table.
	shared_cinr_table[index][0] = cinr;
	shared_rssi_table[index][0] = rssi;

	//Now, take care of updating average dl cinr.
	if (average_cinr_table[index][0] == (float)(-5))
	{
		average_cinr_table[index][0] = (float)shared_cinr_table[index][0];
	}
	else
	{
		//Do averaging according to scheme 2.
                prior_average_cinr_table[index][0] = average_cinr_table[index][0];
		//average_cinr_table[index][0] = ALPHA_CINR*(float)cinr + (1 - ALPHA_CINR)*average_cinr_table[index][0];
                average_cinr_table[index][0] = (float)cinr; 
	}
	FLOG_DEBUG("Update SS DL Link Quality for CID %d CINR inputted %d Average CINR %f\n",basic_cid, cinr, average_cinr_table[index][0]);

	pthread_mutex_unlock(&cinr_table_lock);
	return 0;
}

int update_average_crc_table_rx(int basic_cid, int  crc_errors, int packets)
{
	//remember to reset on  AMC change
#ifdef SS_TX
	int index = 0;
#else
	int index = basic_cid - BASIC_CID_MIN_VALUE;
        if ((index >= TABLE_SIZE) || (index < 0))
        {
                return -1;
        }
#endif
	pthread_mutex_lock(&crc_table_lock);

		shared_crc_table[index][ASIZE-1] = (float) crc_errors/packets;
		FLOG_DEBUG("Update CRC TABLE RX: CID %d CRC Errors %d Packets %d Error rate %f\n",basic_cid, crc_errors, packets, shared_crc_table[index][ASIZE-1]);
		if (shared_crc_table[index][ASIZE-1] > 0.1)
		{
			total_crc_cinr_adjust[index][ASIZE-1] += CRC_GREATER_THAN_90_PRC;
		}
		else if (shared_crc_table[index][ASIZE-1] >= 0.02)
		{
			total_crc_cinr_adjust[index][ASIZE-1] += CRC_GREATER_THAN_10_PRC;
		}
                else if(shared_crc_table[index][ASIZE-1] < 0.01)
                {
			total_crc_cinr_adjust[index][ASIZE-1] += CRC_LESS_THAN_POINT_1_PRC;
                }
/*
		else 
		{
			if (shared_crc_table[index][ASIZE-1] < 0.001)
			{
				total_crc_cinr_adjust[index][ASIZE-1] +=CRC_LESS_THAN_POINT_1_PRC;
			}
			else if (shared_crc_table[index][ASIZE-1] < 0.01)
			{
				total_crc_cinr_adjust[index][ASIZE-1] +=CRC_LESS_THAN_1_PRC;
			}
			
		}
*/
		if (total_crc_cinr_adjust[index][ASIZE-1] > TOTAL_CRC_ADJUST_UPPER_LIMIT)
		total_crc_cinr_adjust[index][ASIZE-1] = TOTAL_CRC_ADJUST_UPPER_LIMIT;
		
		if (total_crc_cinr_adjust[index][ASIZE-1] < TOTAL_CRC_ADJUST_LOWER_LIMIT)
		total_crc_cinr_adjust[index][ASIZE-1] = TOTAL_CRC_ADJUST_LOWER_LIMIT;
        
        FLOG_DEBUG("Update CRC TABLE RX: Adjuest:%ddB, CID %d CRC Errors %d Packets %d Error rate %f\n",total_crc_cinr_adjust[index][ASIZE-1],basic_cid, crc_errors, packets, shared_crc_table[index][ASIZE-1]);
	

	pthread_mutex_unlock(&crc_table_lock);
	return 0;

}

static char* get_amc_string(int type)
{
        char    *string;

        switch (type)
        {
                case QPSK_12:
                        string = "QPSK_12";
                        break;
                case QPSK_34:
                        string = "QPSK_34";
                        break;
                case QAM16_12:
                        string = "16QAM_12";
                        break;
                case QAM16_34:
                        string = "16QAM_34";
                        break;
                case QAM64_12:
                        string = "64QAM_12";
                        break;
                case QAM64_23:
                        string = "64QAM_23";
                        break;
                case QAM64_34:
                        string = "64QAM_34";
                        break;
                default:
                        string = "unknown";
                        break;
        }
        return string;
}

int update_dl_amcs(amc_info* amc_info_header)
{
	ss_amc_info* temp = amc_info_header->ss_amc_head;	
	int index = 0;
	float net_cinr_table[NUM_SS];
        float prior_net_cinr_table[NUM_SS];
	int amc_change_flag = 0;int i;

        int mod_sel;

	pthread_mutex_lock(&cinr_table_lock);
	pthread_mutex_lock(&crc_table_lock);
		
	for (i=0;i<NUM_SS;i++)
	{
#ifdef CRC_ADJUST_ENABLE
		net_cinr_table[i] = average_cinr_table[i][index] + total_crc_cinr_adjust[i][index] - rep_cinr_adjust[i][index];
                prior_net_cinr_table[i] = prior_average_cinr_table[i][index] + total_crc_cinr_adjust[i][index] - rep_cinr_adjust[i][index];
               
		FLOG_DEBUG("Update DL AMCS: CRC adjust : SS NUM %d Average CINR %f total CINR Adjust %d net cinr %f\n",i,average_cinr_table[i][index], total_crc_cinr_adjust[i][index], net_cinr_table[i]);

#else
		net_cinr_table[i] = average_cinr_table[i][index];
                prior_net_cinr_table[i] = prior_average_cinr_table[i][index];
#endif
	}

	pthread_mutex_unlock(&crc_table_lock);
	pthread_mutex_unlock(&cinr_table_lock);


	pthread_mutex_lock(&(amc_info_header->amc_lock));
	for (i=0;i<NUM_SS;i++)
	{
		if (average_cinr_table[i][index] == (float)(-5))
		{
			//No CINR feedback is in yet. Keep using initial AMC until you get CINR estimates
			continue;
		}
		amc_change_flag = 0;
		if (temp == NULL) 
		{
			FLOG_ERROR("AMC Entry is NULL. Unexpected. SS num is %d\n", i);
			continue;
		}
#if 0
		if (net_cinr_table[i] < DL_QPSK_12_LEAVE)
                {
                        if (temp->dl_fec_code_modulation_type != QPSK_12)
                        {
                                FLOG_INFO("%dth ss dl: %s --> %s\n", i, get_amc_string(temp->dl_fec_code_modulation_type), \
                                                                        get_amc_string(QPSK_12));
                                temp->dl_fec_code_modulation_type = QPSK_12;
                                amc_change_flag = 1;
                        }
                }
                else if (net_cinr_table[i] < DL_QAM16_12_LEAVE)
                {
                        if (temp->dl_fec_code_modulation_type != QAM16_12)
                        {
                                FLOG_INFO("%dth ss dl: %s --> %s\n", i, get_amc_string(temp->dl_fec_code_modulation_type), \
                                                                        get_amc_string(QAM16_12));
                                temp->dl_fec_code_modulation_type = QAM16_12;
                                amc_change_flag = 1;
                        }
                }
                else if (net_cinr_table[i] < DL_QAM64_12_LEAVE)
                {
                        if (temp->dl_fec_code_modulation_type != QAM64_12)
                        {
                                FLOG_INFO("%dth ss dl: %s --> %s\n", i, get_amc_string(temp->dl_fec_code_modulation_type), \
                                                                        get_amc_string(QAM64_12));
                                temp->dl_fec_code_modulation_type = QAM64_12;
                                amc_change_flag = 1;
                        }
                }
                else if (net_cinr_table[i] < DL_QAM64_23_LEAVE)
                {
                        if (temp->dl_fec_code_modulation_type != QAM64_23)
                        {
                                FLOG_INFO("%dth ss dl: %s --> %s\n", i, get_amc_string(temp->dl_fec_code_modulation_type), \
                                                                        get_amc_string(QAM64_23));
                                temp->dl_fec_code_modulation_type = QAM64_23;
                                amc_change_flag = 1;
                        }
                }
                else
                {
                        if (temp->dl_fec_code_modulation_type != QAM64_34)
			{
                                FLOG_INFO("%dth ss dl: %s --> %s\n", i, get_amc_string(temp->dl_fec_code_modulation_type), \
                                                                        get_amc_string(QAM64_34));
                                temp->dl_fec_code_modulation_type = QAM64_34;
                                amc_change_flag = 1;
                        }
		}
#endif

#if 0
                mod_sel = temp->dl_fec_code_modulation_type;
		if(net_cinr_table[i] >= prior_net_cinr_table[i]) //Could to improve the modulation method
                {
                    if(net_cinr_table[i] >= DL_QPSK_12_ENTER)  //Enter level for QPSK_12
                    {
                        mod_sel = QPSK_12;
                    }

                    if(net_cinr_table[i] >= DL_QPSK_34_ENTER)  //Enter level for QPSK_34
                    {
                        mod_sel = QPSK_34;
                    }
                    if(net_cinr_table[i] >= DL_QAM16_12_ENTER)  //Enter level for QAM16_12
                    {
                        mod_sel = QAM16_12;
                    }
                    if(net_cinr_table[i] >= DL_QAM16_34_ENTER)  //Enter level for QAM16_34
                    {
                        mod_sel = QAM16_34;
                    }
                    if(net_cinr_table[i] >= DL_QAM64_12_ENTER)  //Enter level for QAM64_12
                    {
                        mod_sel = QAM64_12;
                    }
                    if(net_cinr_table[i] >= DL_QAM64_23_ENTER)  //Enter level for QAM64_23
                    {
                        mod_sel = QAM64_23;
                    }
                    if(net_cinr_table[i] >= DL_QAM64_34_ENTER)  //Enter level for QAM64_34
                    {
                        mod_sel  = QAM64_34;
                    }
                }
                else //Need to decrease the modulation methods
                {
                    if(net_cinr_table[i] <  DL_QAM64_34_LEAVE)  //Leave for QAM64_34
                    {
                            mod_sel = QAM64_23;
                    }
                    if(net_cinr_table[i] <  DL_QAM64_23_LEAVE)  //Leave for QAM64_23
                    {
                            mod_sel = QAM64_12;
                    }
                    if(net_cinr_table[i] <  DL_QAM64_12_LEAVE)  //Leave for QAM64_12
                    {
                            mod_sel = QAM16_12;
                    }
                    if(net_cinr_table[i] <  DL_QAM16_12_LEAVE)  //Leave for QAM16_12
                    {
                            mod_sel = QPSK_12;
                    }
                }

                if(temp->dl_fec_code_modulation_type != mod_sel) //Modulation Mode change, needs update
                {
                    FLOG_INFO("%dth SS DL:%s-->%s,NetCinrchange %5.1fdB-->%5.1fdB,AvgCinrchange %5.1f dB-->%5.1f dB\n", i, get_amc_string(temp->dl_fec_code_modulation_type), \
                              get_amc_string(mod_sel), prior_net_cinr_table[i], net_cinr_table[i], prior_average_cinr_table[i][0], average_cinr_table[i][0]);
                    temp->dl_fec_code_modulation_type = mod_sel;
                    amc_change_flag = 1;
                }                		
#endif

#if 1
		switch(temp->dl_fec_code_modulation_type)
                {
                    case QPSK_12:
                        if(net_cinr_table[i] >= DL_QAM16_12_ENTER)
                        {
                            mod_sel = QAM16_12;
                        }
                        else
                        {
                            mod_sel = QPSK_12;
                        }
                        break;
                    case QAM16_12:
                        if(net_cinr_table[i] >= DL_QAM64_12_ENTER)
                        {
			    mod_sel = QAM64_12;
                        }
                        else if(net_cinr_table[i] < DL_QAM16_12_LEAVE)
                        {
                            mod_sel = QPSK_12;
                        }
                        else
                        {
                            mod_sel = QAM16_12;								
                        }
                        break;
                    case QAM64_12:
                        if(net_cinr_table[i] >= DL_QAM64_23_ENTER)
                        {
                            mod_sel = QAM64_23;
                        }
                        else if(net_cinr_table[i] < DL_QAM64_12_LEAVE)
                        {
                            mod_sel = QAM16_12;
                        }
                        else
                        {
                            mod_sel = QAM64_12;
                        }
                        break;
                    case QAM64_23:
                        if(net_cinr_table[i] >= DL_QAM64_34_ENTER)
                        {
                            mod_sel = QAM64_34;
                        }
                        else if(net_cinr_table[i] < DL_QAM64_23_LEAVE)
                        {
                            mod_sel = QAM64_12;
                        }
                        else
                        {
                            mod_sel = QAM64_23;
                        }
                        break;
                    case QAM64_34:
                        if(net_cinr_table[i] < DL_QAM64_23_LEAVE)
                        {
                            mod_sel = QAM64_23;
                        }
                        else
                        {
                            mod_sel = QAM64_34;
                        }
                        break;

 		   default:
			break;

                }

                if(temp->dl_fec_code_modulation_type != mod_sel) //Modulation Mode change, needs update
                {
                    FLOG_INFO("%dth SS DL:%s-->%s,NetCinrchange %5.1fdB-->%5.1fdB,AvgCinrchange %5.1f dB-->%5.1f dB\n", i, get_amc_string(temp->dl_fec_code_modulation_type), \
                              get_amc_string(mod_sel), prior_net_cinr_table[i], net_cinr_table[i], prior_average_cinr_table[i][0], average_cinr_table[i][0]);
                    temp->dl_fec_code_modulation_type = mod_sel;
                    amc_change_flag = 1;
                }

               current_mcs_table[i][0] = temp->dl_fec_code_modulation_type;
#endif
		if (amc_change_flag == 1)
		{
			pthread_mutex_lock(&crc_table_lock);
		//	total_crc_cinr_adjust[i][index] = 0;
			cpe_packet_count[i]=0; 
                        cpe_crc_error_count[i] = 0;
			FLOG_DEBUG("SS NUM %d DL AMC Changed. CRC Error rate Reset \n",i);
			pthread_mutex_unlock(&crc_table_lock);
		}
		FLOG_DEBUG("DL AMC code for SS %d is %d\n",i, temp->dl_fec_code_modulation_type);
		temp = temp->next;
	}
	pthread_mutex_unlock(&(amc_info_header->amc_lock));
		

	return 0;
}

int update_ul_amcs(amc_info* amc_info_header)
{
	ss_amc_info* temp = amc_info_header->ss_amc_head;	
	int index = 1;
	float net_cinr_table[NUM_SS];
        float prior_net_cinr_table[NUM_SS];
        int mod_sel;

	int amc_change_flag = 0;int i;
	pthread_mutex_lock(&cinr_table_lock);
	pthread_mutex_lock(&crc_table_lock);
		
	for (i=0;i<NUM_SS;i++)
	{
#ifdef CRC_ADJUST_ENABLE
		net_cinr_table[i] = average_cinr_table[i][index] + total_crc_cinr_adjust[i][index];
                prior_net_cinr_table[i] = average_cinr_table[i][index] + total_crc_cinr_adjust[i][index];  

		FLOG_DEBUG("Update UL AMCS: CRC adjust : SS NUM %d Average CINR %f total CINR Adjust %d net cinr %f\n",i,average_cinr_table[i][index], total_crc_cinr_adjust[i][index], net_cinr_table[i]);

#else
		net_cinr_table[i] = average_cinr_table[i][index];
                prior_net_cinr_table[i] = average_cinr_table[i][index];
#endif
	}

	pthread_mutex_unlock(&crc_table_lock);
	pthread_mutex_unlock(&cinr_table_lock);


	pthread_mutex_lock(&(amc_info_header->amc_lock));
	for (i=0;i<NUM_SS;i++)
	{
		if (average_cinr_table[i][index] == (float)(-5))
		{
			//No CINR feedback is in yet. Keep using initial AMC until you get CINR estimates
			continue;
		}
		amc_change_flag = 0;
		if (temp == NULL) 
		{
			FLOG_ERROR("AMC Entry is NULL. Unexpected. SS num is %d\n", i);
			continue;
		}
#if 0
		if (net_cinr_table[i] < UL_QPSK_12_LEAVE)
		{
			if (temp->ul_fec_code_modulation_type != QPSK_12)
			{
				FLOG_INFO("%dth ss ul: %s --> %s\n", i, get_amc_string(temp->ul_fec_code_modulation_type), \
									get_amc_string(QPSK_12));
				temp->ul_fec_code_modulation_type = QPSK_12;
				amc_change_flag = 1;
			}
		}
		else if (net_cinr_table[i] < UL_QAM16_12_LEAVE)
		{
			if (temp->ul_fec_code_modulation_type != QAM16_12)
                        {
                                FLOG_INFO("%dth ss ul: %s --> %s\n", i, get_amc_string(temp->ul_fec_code_modulation_type), \
                                                                        get_amc_string(QAM16_12));
                                temp->ul_fec_code_modulation_type = QAM16_12;
                                amc_change_flag = 1;
                        }
		}
		else if (net_cinr_table[i] < UL_QAM64_12_LEAVE)
		{
			if (temp->ul_fec_code_modulation_type != QAM64_12)
                        {
                                FLOG_INFO("%dth ss ul: %s --> %s\n", i, get_amc_string(temp->ul_fec_code_modulation_type), \
                                                                        get_amc_string(QAM64_12));
                                temp->ul_fec_code_modulation_type = QAM64_12;
                                amc_change_flag = 1;
                        }
		}
		else if (net_cinr_table[i] < UL_QAM64_23_LEAVE)
		{
                        if (temp->ul_fec_code_modulation_type != QAM64_23)
                        {
                                FLOG_INFO("%dth ss ul: %s --> %s\n", i, get_amc_string(temp->ul_fec_code_modulation_type), \
                                                                        get_amc_string(QAM64_23));
                                temp->ul_fec_code_modulation_type = QAM64_23;
                                amc_change_flag = 1;
                        }
                }
		else
                {
                        if (temp->ul_fec_code_modulation_type != QAM64_34)
                        {
                                FLOG_INFO("%dth ss ul: %s --> %s\n", i, get_amc_string(temp->ul_fec_code_modulation_type), \
                                                                        get_amc_string(QAM64_34));
                                temp->ul_fec_code_modulation_type = QAM64_34;
                                amc_change_flag = 1;
                        }
                }
#endif	

#if 0

		if(net_cinr_table[i] >= prior_net_cinr_table[i]) //Could to improve the modulation method
                {
                    if(net_cinr_table[i] >= UL_QPSK_12_ENTER)  //Enter level for QPSK_12
                    {
                        if(temp->ul_fec_code_modulation_type != QPSK_12)
                        {
                            FLOG_INFO("%dth ss ul: %s --> %s\n", i, get_amc_string(temp->ul_fec_code_modulation_type), \
                                                                        get_amc_string(QPSK_12));
                            temp->ul_fec_code_modulation_type = QPSK_12;
                            amc_change_flag = 1;
                        }
                    }

                   if(net_cinr_table[i] >= UL_QPSK_34_ENTER)  //Enter level for QPSK_34
                    {
                        if(temp->ul_fec_code_modulation_type != QPSK_34)
                        {
                            FLOG_INFO("%dth ss ul: %s --> %s\n", i, get_amc_string(temp->ul_fec_code_modulation_type), \
                                                                        get_amc_string(QPSK_34));
                            temp->ul_fec_code_modulation_type = QPSK_34;
                            amc_change_flag = 1;
                        }
                    }
                    if(net_cinr_table[i] >= UL_QAM16_12_ENTER)  //Enter level for QAM16_12
                    {
                        if(temp->ul_fec_code_modulation_type != QAM16_12)
                        {
                            FLOG_INFO("%dth ss ul: %s --> %s\n", i, get_amc_string(temp->ul_fec_code_modulation_type), \
                                                                        get_amc_string(QAM16_12));
                            temp->ul_fec_code_modulation_type = QAM16_12;
                            amc_change_flag = 1;
                        }
                    }
                    if(net_cinr_table[i] >= UL_QAM16_34_ENTER)  //Enter level for QAM16_34
                    {
                        if(temp->ul_fec_code_modulation_type != QAM16_34)
                        {
                            FLOG_INFO("%dth ss ul: %s --> %s\n", i, get_amc_string(temp->ul_fec_code_modulation_type), \
                                                                        get_amc_string(QAM16_34));
                            temp->ul_fec_code_modulation_type = QAM16_34;
                            amc_change_flag = 1;
                        }
                    }
                    if(net_cinr_table[i] >= UL_QAM64_12_ENTER)  //Enter level for QAM64_12
                    {
                        if(temp->ul_fec_code_modulation_type != QAM64_12)
                        {
                            FLOG_INFO("%dth ss ul: %s --> %s\n", i, get_amc_string(temp->ul_fec_code_modulation_type), \
                                                                        get_amc_string(QAM64_12));
                            temp->ul_fec_code_modulation_type = QAM64_12;
                            amc_change_flag = 1;
                        }
                    }
                    if(net_cinr_table[i] >= UL_QAM64_23_ENTER)  //Enter level for QAM64_23
                    {
                        if(temp->ul_fec_code_modulation_type != QAM64_23)
                        {
                            FLOG_INFO("%dth ss ul: %s --> %s\n", i, get_amc_string(temp->ul_fec_code_modulation_type), \
                                                                        get_amc_string(QAM64_23));
                            temp->ul_fec_code_modulation_type = QAM64_23;
                            amc_change_flag = 1;
                        }
                    }
                    if(net_cinr_table[i] >= UL_QAM64_34_ENTER)  //Enter level for QAM64_34
                    {
                        if(temp->ul_fec_code_modulation_type != QAM64_34)
                        {
                            FLOG_INFO("%dth ss ul: %s --> %s\n", i, get_amc_string(temp->ul_fec_code_modulation_type), \
                                                                        get_amc_string(QAM64_34));
                            temp->ul_fec_code_modulation_type = QAM64_34;
                            amc_change_flag = 1;
                        }
                    }
                }
                else //Need to decrease the modulation methods
                {
                    if(net_cinr_table[i] <  UL_QAM64_34_LEAVE)  //Leave for QAM64_34
                    {
                        if(temp->ul_fec_code_modulation_type != QAM64_23)
                        {
                            FLOG_INFO("%dth ss ul: %s --> %s\n", i, get_amc_string(temp->ul_fec_code_modulation_type), \
                                                                        get_amc_string(QAM64_23));
                            temp->ul_fec_code_modulation_type = QAM64_23;
                            amc_change_flag = 1;
                        }
                    }
                    if(net_cinr_table[i] <  UL_QAM64_23_LEAVE)  //Leave for QAM64_23
                    {
                        if(temp->ul_fec_code_modulation_type != QAM64_12)
                        {
                            FLOG_INFO("%dth ss ul: %s --> %s\n", i, get_amc_string(temp->ul_fec_code_modulation_type), \
                                                                        get_amc_string(QAM64_12));
                            temp->ul_fec_code_modulation_type = QAM64_12;
                            amc_change_flag = 1;
                        }
                    }
                    if(net_cinr_table[i] <  UL_QAM64_12_LEAVE)  //Leave for QAM64_12
                    {
                        if(temp->ul_fec_code_modulation_type != QAM16_12)
                        {
                            FLOG_INFO("%dth ss ul: %s --> %s\n", i, get_amc_string(temp->ul_fec_code_modulation_type), \
                                                                        get_amc_string(QAM16_12));
                            temp->ul_fec_code_modulation_type = QAM16_12;
                            amc_change_flag = 1;
                        }
                    }
                    if(net_cinr_table[i] <  UL_QAM16_12_LEAVE)  //Leave for QAM16_12
                    {
                        if(temp->ul_fec_code_modulation_type != QPSK_12)
                        {
                            FLOG_INFO("%dth ss ul: %s --> %s\n", i, get_amc_string(temp->ul_fec_code_modulation_type), \
                                                                        get_amc_string(QPSK_12));
                            temp->ul_fec_code_modulation_type = QPSK_12;
                            amc_change_flag = 1;
                        }
                    }
                }		

#endif

#if 1 
		switch (temp->ul_fec_code_modulation_type)
		{       
                   case QPSK_12:
                        if(net_cinr_table[i] >= UL_QAM16_12_ENTER)
                        {
                            mod_sel = QAM16_12;
                        }
                        else
                        {
                            mod_sel = QPSK_12;
                        }
                        break;
	            case QAM16_12:
                        if(net_cinr_table[i] >= UL_QAM64_12_ENTER)
                        {
                            mod_sel = QAM64_12;
                        }
                        else if(net_cinr_table[i] < UL_QAM16_12_LEAVE)
                        {
                            mod_sel = QPSK_12;
                        }
                        else
                        {
                            mod_sel = QAM16_12;
                        }
                        break;
                    case QAM64_12:
                        if(net_cinr_table[i] >= UL_QAM64_23_ENTER)
                        {
                            mod_sel = QAM64_23;
                        }
                        else if(net_cinr_table[i] < UL_QAM64_12_LEAVE)
                        {
                            mod_sel = QAM16_12;
                        }
                        else
                        {
                            mod_sel = QAM64_12;
                        }
			break;
		    case QAM64_23:
                        if(net_cinr_table[i] >= UL_QAM64_34_ENTER)
                        {
                            mod_sel = QAM64_34;
                        }
                        else if(net_cinr_table[i] < UL_QAM64_23_LEAVE)
                        {
                            mod_sel = QAM64_12;
                        }
                        else
                        {
                            mod_sel = QAM64_23;
                        }
                        break;
		    case QAM64_34:
                        if(net_cinr_table[i] < UL_QAM64_34_LEAVE)
                        {
                            mod_sel = QAM64_23;
                        }
                        else
                        {
                            mod_sel = QAM64_34;
                        }
                        break;
                   default:
                        break;

                }

                if(temp->ul_fec_code_modulation_type != mod_sel) //Modulation Mode change, needs update
                {
                    FLOG_INFO("%dth SS UL:%s-->%s,NetCinrchange %5.1fdB-->%5.1fdB,AvgCinrchange %5.1f dB-->%5.1f dB\n", i, get_amc_string(temp->ul_fec_code_modulation_type), \
                              get_amc_string(mod_sel), prior_net_cinr_table[i], net_cinr_table[i], prior_average_cinr_table[i][1], average_cinr_table[i][1]);
                    temp->ul_fec_code_modulation_type = mod_sel;
                    amc_change_flag = 1;
                }
                current_mcs_table[i][1] = temp->ul_fec_code_modulation_type;

#endif

		if (amc_change_flag == 1)
		{
			pthread_mutex_lock(&crc_table_lock);
			//total_crc_cinr_adjust[i][index] = 0;

//UL CRC Error is measured in BS. So resetting crc error num etc too. For DL CRC error, this resetting should happen in CPE since crc_error_num is obtained from CPE's REP RSP
			crc_error_count_table[i][index] = 0;
			packet_count[i] = 0; 
			FLOG_DEBUG("SS NUM %d UL AMC Changed. CRC Error rate Reset. Index is %d \n",i,index);
			pthread_mutex_unlock(&crc_table_lock);
		}
		FLOG_DEBUG("UL AMC code for SS %d is %d\n",i, temp->ul_fec_code_modulation_type);
		temp = temp->next;
	}
	pthread_mutex_unlock(&(amc_info_header->amc_lock));
		

	return 0;
}
int update_amc_before_bs_scheduling(amc_info* amc_info_header)
{
#ifdef AMC_ENABLE
	//DL Info Print
        int i;
        int static print_loop = 0;
        int current_frame_num;
        int dump_data;
        
#if 1
    if((print_loop++ % 150) == 0)
    { 
        if(NUM_SS != 0)
        {
            FLOG_INFO("------------------------%d------------------------\n", print_loop);
        }
        for(i = 0; i < NUM_SS; i++)
        {
            FLOG_INFO("DL Cid:%d->Cinr:%5.1fdB,CrcErrCnt:%d,PkgCnt:%d,ErrRate:%5.4f,CinrAdj:%d,Mcs:%d\n", i+BASIC_CID_MIN_VALUE, average_cinr_table[i][0], \
                            cpe_crc_error_count[i], cpe_packet_count[i], shared_crc_table[i][0], total_crc_cinr_adjust[i][0], current_mcs_table[i][0]);
        
            FLOG_INFO("UL Cid:%d->Cinr:%5.1fdB,CrcErrCnt:%d,PkgCnt:%d,ErrRate:%5.4f,CinrAdj:%d,Mcs:%d\n", i+BASIC_CID_MIN_VALUE, average_cinr_table[i][1], \
                            crc_error_count_table[i][1], packet_count[i], shared_crc_table[i][1], total_crc_cinr_adjust[i][1], current_mcs_table[i][1]);

        }
    }
#endif

        //Update DL AMC
 	update_dl_amcs(amc_info_header); 
	//Update UL AMC
	update_ul_amcs(amc_info_header); 

        current_frame_num = get_current_frame_number();
   
        for(i = 0; i < NUM_SS; i++)
        { 
            DO_DUMP(DUMP_MAC_AMC_PARAM_ID, 0, &current_frame_num, 1); //dump the frame number;
            dump_data = i + BASIC_CID_MIN_VALUE;
            DO_DUMP(DUMP_MAC_AMC_PARAM_ID, 0, &dump_data, 1); //Number the number of active SS

            //AMC dl infor dump
	    dump_data = (int)(average_cinr_table[i][0] * 10);
            DO_DUMP(DUMP_MAC_AMC_PARAM_ID, 0, &dump_data, 1); //cinr
            dump_data = (int)(average_rssi_table[i][0] * 10);
            DO_DUMP(DUMP_MAC_AMC_PARAM_ID, 0, &dump_data, 1); //rssi
            DO_DUMP(DUMP_MAC_AMC_PARAM_ID, 0, &cpe_crc_error_count[i], 1); //crc error count
            DO_DUMP(DUMP_MAC_AMC_PARAM_ID, 0, &cpe_packet_count[i], 1);  //total packet count
            dump_data = (int) (shared_crc_table[i][0] * 10000);
            DO_DUMP(DUMP_MAC_AMC_PARAM_ID, 0, &dump_data, 1);  //crc error rate  
            DO_DUMP(DUMP_MAC_AMC_PARAM_ID, 0, &total_crc_cinr_adjust[i][0], 1); //cinr adjut 
            DO_DUMP(DUMP_MAC_AMC_PARAM_ID, 0, &rep_cinr_adjust[i][0], 1);  //cinr burst
            DO_DUMP(DUMP_MAC_AMC_PARAM_ID, 0, &current_mcs_table[i][0], 1); //mcs

            //AMC dl infor dump
            dump_data = (int)(average_cinr_table[i][1] * 10);
            DO_DUMP(DUMP_MAC_AMC_PARAM_ID, 0, &dump_data, 1); //cinr
            dump_data = (int)(average_rssi_table[i][1] * 10);
            DO_DUMP(DUMP_MAC_AMC_PARAM_ID, 0, &dump_data, 1); //rssi
            DO_DUMP(DUMP_MAC_AMC_PARAM_ID, 0, &crc_error_count_table[i][1], 1); //crc error count
            DO_DUMP(DUMP_MAC_AMC_PARAM_ID, 0, &packet_count[i], 1);  //total packet count
            dump_data = (int) (shared_crc_table[i][1] * 10000);
            DO_DUMP(DUMP_MAC_AMC_PARAM_ID, 0, &dump_data, 1);  //crc error rate
            DO_DUMP(DUMP_MAC_AMC_PARAM_ID, 0, &total_crc_cinr_adjust[i][1], 1); //cinr adjut
            DO_DUMP(DUMP_MAC_AMC_PARAM_ID, 0, &rep_cinr_adjust[i][1], 1);  //cinr burst
            DO_DUMP(DUMP_MAC_AMC_PARAM_ID, 0, &current_mcs_table[i][1], 1); //mcs            
        }
        
#endif
	
	return 0;
}
int update_dl_link_quality_from_rep_rsp(int cid, int cinr, int rssi)
{
#ifndef SS_TX
	int index = cid - BASIC_CID_MIN_VALUE;
	if ((index >= TABLE_SIZE) || (index < 0))
        {
                return -1;
        }
//	pthread_mutex_lock(&cinr_table_lock);
//	average_cinr_table[index][0] = cinr;
//	average_rssi_table[index][0] = rssi;
//	pthread_mutex_unlock(&cinr_table_lock);
        reset_rep_cinr_adjust(cid);

        update_ss_dl_link_quality_cpe(cid, cinr, rssi);
#endif
	return 0;
}
int update_dl_crc_rate_from_rep_rsp(int cid, int crc_error_num, int packet_num)
{
#ifndef SS_TX
	int index = cid - BASIC_CID_MIN_VALUE;
	if ((index >= TABLE_SIZE) || (index < 0))
        {
                return -1;
        }
	pthread_mutex_lock(&crc_table_lock);
	FLOG_DEBUG("CRC Update from REP RSP: crc error num %d packet count %d cpe packet count %d\n",crc_error_num, packet_num, cpe_packet_count[index]);
	if (/*(cpe_packet_count[index] < PACKET_UPDATE_MODULUS) ||*/ (packet_num - cpe_packet_count[index] > PACKET_UPDATE_MODULUS))
	{

		shared_crc_table[index][0] = (float)((crc_error_num - cpe_crc_error_count[index]) / (packet_num - cpe_packet_count[index]));
		//update total crc cinr adjust
		FLOG_DEBUG("CRC Update from REP RSP: Updating Table %f\n",shared_crc_table[index][0]);
		
		if (shared_crc_table[index][0] > 0.1)
		{
                    total_crc_cinr_adjust[index][0] += CRC_GREATER_THAN_90_PRC;
		}
		else if (shared_crc_table[index][0] >= 0.02)
		{
                    total_crc_cinr_adjust[index][0] += CRC_GREATER_THAN_10_PRC;
		}
                else if(shared_crc_table[index][0] < 0.01)
                {
		    total_crc_cinr_adjust[index][0] +=CRC_LESS_THAN_POINT_1_PRC;
                }
/* 
		else 
		{
			if (shared_crc_table[index][0] < 0.001)
			{
				total_crc_cinr_adjust[index][0] +=CRC_LESS_THAN_POINT_1_PRC;
			}
			else if (shared_crc_table[index][0] < 0.01)
			{
				total_crc_cinr_adjust[index][0] +=CRC_LESS_THAN_1_PRC;
			}
			
		}
*/
		if (total_crc_cinr_adjust[index][0] > TOTAL_CRC_ADJUST_UPPER_LIMIT)
		total_crc_cinr_adjust[index][0] = TOTAL_CRC_ADJUST_UPPER_LIMIT;
		
		if (total_crc_cinr_adjust[index][0] < TOTAL_CRC_ADJUST_LOWER_LIMIT)
		total_crc_cinr_adjust[index][0] = TOTAL_CRC_ADJUST_LOWER_LIMIT;
	
		cpe_packet_count[index] = packet_num;
                cpe_crc_error_count[index] = crc_error_num;
 
                FLOG_DEBUG("CRC Update from REP RSP: Adjust:%ddB, crc error num %d packet count %d cpe packet count %d\n",total_crc_cinr_adjust[index][0], crc_error_num, packet_num, cpe_packet_count[index]);


	}
	pthread_mutex_unlock(&crc_table_lock);
#endif
	return 0;
}
ss_amc_info* get_ss_amc_info(amc_info* amc, int ss_num)
{
	ss_amc_info* temp = amc->ss_amc_head;
	int i=0;
	while (i < ss_num)
	{
		temp = temp->next;
		if (temp == NULL) 
		{
			FLOG_ERROR("DL Scheduler trying to get non-existent SS's AMC info\n");
			return NULL;
		}
		i++;
	}
	return temp;
}
int initialize_amc_info(){
#ifdef SS_TX
	#define MY_NUM_SS 1
#else
	#define MY_NUM_SS MAX_NUM_SS
#endif
  int ii=0;
  amc_list=(amc_info*)mac_malloc(sizeof(amc_info));
  if(!amc_list)
    {
      FLOG_FATAL("Error allocating memory for amc_info");
      return -1;
    }
  amc_list->ss_num=MY_NUM_SS;
  amc_list->ss_amc_head=NULL;
  pthread_mutex_init(&amc_list->amc_lock,NULL);
  ss_amc_info *current=NULL, *tail=NULL;

  do{
    current=(ss_amc_info*)mac_malloc(sizeof(ss_amc_info));
    if (!current) 
      {
	FLOG_FATAL("get_amc_info: Error allocating memory.\n");
	return -1;
      }

    // If the list is yet empty, assign the head.
    if (amc_list->ss_amc_head==NULL)
      {
	amc_list->ss_amc_head=current;
      }
    else
      {
	tail->next=current;
      }
    // MAC addr is not set or used right now. It might
    // even be removed from the structure as long as the SS
    // mapping index is correct
    current->ss_mac=0;
    current->subchannel_num=0;
    current->ul_fec_code_modulation_type=param_UL_MCS;
    current->dl_fec_code_modulation_type=param_DL_MCS;
    current->next=NULL;
    tail=current;
 //   printf("Initialize amc info . ss num %d ul %d dl %d num_ss %d\n",ii, current->ul_fec_code_modulation_type, current->dl_fec_code_modulation_type,NUM_SS);
    ii++;
  } while(ii<MY_NUM_SS);

#ifdef SS_TX
   prev_dl_amc = param_DL_MCS;
#endif
  return 0;
}

int get_amc_info(amc_info** amc_info_header)
{
  (*amc_info_header) = amc_list;
  return 0;
}

int free_amc_info()
{
  amc_info *amc_info_header = amc_list;
  ss_amc_info* temp;

  while(amc_info_header->ss_amc_head!=NULL)
    {
      temp=amc_info_header->ss_amc_head;
      amc_info_header->ss_amc_head=amc_info_header->ss_amc_head->next;
      mac_free(sizeof(ss_amc_info), temp);
    }
  mac_free(sizeof(amc_info), amc_info_header);
  amc_list = NULL;

  return 0;
}

int get_ss_mcs(int ss_index, ModulCodingType *dl_mcs, ModulCodingType *ul_mcs)
{
	ss_amc_info	*head;
    	int 		i;

	head = amc_list->ss_amc_head;
	if ((head == NULL) || (dl_mcs == NULL) || (ul_mcs == NULL))
	{
		return -1;
	}

	i = 0;
    	while (i < ss_index)
    	{
        	head = head->next;
        	if (head == NULL)
        	{
            		FLOG_ERROR("DL Scheduler trying to get non-existent SS's AMC info\n");
            		return -1;
        	}
        	i++;
    	}
    
	*dl_mcs = head->dl_fec_code_modulation_type;
	*ul_mcs = head->ul_fec_code_modulation_type;
	
	return 0;
}

int set_ss_dl_mcs(int ss_index, ModulCodingType dl_mcs)
{
    ss_amc_info	*head;
    int     	i;

    head = amc_list->ss_amc_head;
    if (head == NULL)
    {
        return -1;
    }

    i = 0;
    while (i < ss_index)
    {
        head = head->next;
        if (head == NULL)
        {
            FLOG_ERROR("DL Scheduler trying to get non-existent SS's AMC info\n");
            return -1;
        }
        i++;
    }

    head->dl_fec_code_modulation_type = dl_mcs;

#ifdef SS_TX
    prev_dl_amc = dl_mcs;
#endif

    return 0;
}

int set_ss_ul_mcs(int ss_index, ModulCodingType ul_mcs)
{
    ss_amc_info *head;
    int         i;

    head = amc_list->ss_amc_head;
    if (head == NULL)
    {
        return -1;
    }

    i = 0;
    while (i < ss_index)
    {
        head = head->next;
        if (head == NULL)
        {
            FLOG_ERROR("DL Scheduler trying to get non-existent SS's AMC info\n");
            return -1;
        }
        i++;
    }

    head->ul_fec_code_modulation_type = ul_mcs;

#ifdef SS_TX
    prev_dl_amc = dl_mcs;
#endif

    return 0;
}


