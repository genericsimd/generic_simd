/* ----------------------------------------------------------------------------
   IBM Confidential

   IBM Wireless M2M Platform   

   (C)Copyright IBM Corp. 2009, 2010, 2011

   All Rights Reserved.

   File Name: phy_dl_tx.c

   Function: Form a DL frame at transmitter side.

   Change Activity:

   Date             Description of Change                            By
   -----------      -------------------------------                  --------
   ----------------------------------------------------------------------------
   PROLOG END TAG zYx                                                         */

#include <memory.h>
#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include <time.h>
#include <sys/time.h>
#include <sys/times.h>

#include "flog.h"
#include "queue_util.h"
#include "adapter_bs_dl_interface_data.h"

#include "phy_dl_tx.h"
#include "phy_dl_preamble.h"
#include "phy_dl_ofdmamodul.h"
#include "phy_dl_ofdmamodul_cdd.h"
#include "phy_dl_zonepermutation.h"
#include "phy_dl_muxofdmsym.h"
#include "phy_dl_subcarrandom.h"
#include "phy_dl_wk_generator.h"
#include "phy_dl_zonepermutation.h"
#include "phy_dl_fec_encoding.h"
#include "phy_dl_utility.h"
#include "phy_dl_tx_common.h"

#include "prephy_proc.h"

/* turn on/off dump according to DUMP_PHY_DL_TX setting */
#ifndef DUMP_PHY_DL_TX

#undef LOCAL_DUMP_ENABLE

#ifdef _DUMP_UTIL_ENABLE_
#define LOCAL_DUMP_ENABLE
#endif

#undef _DUMP_UTIL_ENABLE_

#endif

#include "dump_util.h"
#include "prof_util.h"

int phy_dl_ofdmamodul_cdd(const struct phy_dl_tx_syspara *para,
                          const float *input_r,
                          const float *input_i,
                          float *output_r,
                          float *output_i);

/**----------------------------------------------------------------------------
   Function:    phy_dl_init_rrusymbol()

   Description: Memory malloc and initilize for rru symbol.

   Parameters:
                Input-   [const struct phy_dl_tx_syspara *const para]  The pointer 
                          refer to the struct of system parameters.
                         [phy_dl_slotsymbol *p_slot_symbol] The pointer refer to
                         the slot symbol.
              
                      


   Return Value:
                0       Success
                150     Error

   Notes:
                None

   ----------------------------------------------------------------------------
   LOG END TAG zYx                                                            */

struct phy_dl_rru_symbol *phy_dl_init_rrusymbol(
                              const struct phy_dl_tx_syspara *const para,
                              struct phy_dl_slotsymbol *p_slot_symbol)
{
    struct phy_dl_rru_symbol *p_rru_symbol;
    p_rru_symbol =(struct phy_dl_rru_symbol *)calloc(1, sizeof(struct phy_dl_rru_symbol));

    if (p_rru_symbol == NULL) 
    {
        FLOG_FATAL ("phy_dl_init_rrusymbol: Error in calloc memory for rru_symbol!\n");
        return NULL;
    }
    p_rru_symbol->frame_index = p_slot_symbol->frame_index;
    p_rru_symbol->symbol_len = para->ofdma_symlen_with_guard;
    if (p_slot_symbol->symboloffset == 1) 
    {
        p_rru_symbol->symbol_num = 4;
        p_rru_symbol->symbol_offset = 0;
    }
    else 
    {
        p_rru_symbol->symbol_num = 3;
        p_rru_symbol->symbol_offset = p_slot_symbol->symboloffset;
    }
#ifdef VSXOPT
    posix_memalign((void **)&p_rru_symbol->symbol_i, 16, p_rru_symbol->symbol_num*p_rru_symbol->symbol_len * sizeof(float));
    posix_memalign((void **)&p_rru_symbol->symbol_q, 16, p_rru_symbol->symbol_num*p_rru_symbol->symbol_len * sizeof(float));
#else
    p_rru_symbol->symbol_i =(float *)calloc(p_rru_symbol->symbol_num*p_rru_symbol->symbol_len, sizeof(float));
    p_rru_symbol->symbol_q =(float *)calloc(p_rru_symbol->symbol_num*p_rru_symbol->symbol_len, sizeof(float));
#endif

    if (p_rru_symbol->symbol_i == NULL || p_rru_symbol->symbol_q == NULL) 
    {
        if (p_rru_symbol != NULL)
        {
           free(p_rru_symbol);
           p_rru_symbol = NULL;
        }
        FLOG_FATAL ("phy_dl_init_rrusymbol: Error in calloc memory for rru_symbol!\n");
        return NULL;
    }
   
    /* update the dts information and ULMAP for RRU symbol */

    p_rru_symbol->p_ulmap = (void *)p_slot_symbol->p_ulmap;
    p_rru_symbol->ul_map_len = p_slot_symbol->ul_map_len;

     
    return p_rru_symbol;
}

/**----------------------------------------------------------------------------
   Function:    phy_dl_tx_div1()

   Description: To genarate dl sub frame.

   Parameters:
                Input-  [struct phy_dl_tx_syspara *para]  The pointer refers
                        to the truct of system parameters.
                        [const u_int32_t in_que_id] The id of input queue.

                Output- [const u_int32_t out_que_id] The id of output queue.

   Return Value:
                0       Success
                150     Error

   ----------------------------------------------------------------------------
   LOG END TAG zYx                                                            */

int32_t phy_dl_tx_div1(struct phy_dl_tx_syspara *para,
                       const u_int32_t in_que_id,
                       const u_int32_t out_que_id)
{
    int32_t err_code;
    struct phy_dl_slotsymbol *p_slotsymbol_1;
    struct phy_dl_rru_symbol *p_rru_symbol_ant1;
    struct phy_dts_info *phydts_info;

    struct queue_msg * p_msg_in1 = my_malloc (sizeof(struct queue_msg));
    if (p_msg_in1 == NULL)
    {
        FLOG_ERROR("NULL PTR of malloc in function phy_dl_tx_div1");
        return 1;
    }
    struct queue_msg * p_msg_out1 = my_malloc (sizeof(struct queue_msg));
    if (p_msg_out1 == NULL)
    {
        FLOG_ERROR("NULL PTR of malloc in function phy_dl_tx_div1");
        return 1;
    }
    
    if (para == NULL) 
    {
        FLOG_FATAL ("phy_dl_tx_div1: The pointer refer to sys para is null!\n");
        return ERROR_CODE;
    }

    p_slotsymbol_1 = NULL;
    p_msg_in1->my_type = in_que_id;
    p_msg_out1->my_type = out_que_id;
    if (para->frame_index == 1){
        FLOG_INFO("begain phy_dl_tx_div1...\n");
    }
    while (1) 
    {
        /** Receive one slot_symbol from MAC layer */
        if (wmrt_dequeue (in_que_id, p_msg_in1, sizeof(struct queue_msg)) == -1) 
        {
            FLOG_FATAL("phy_dl_tx_div1: In PHY layer DEQUEUE ERROR\n");
            return ERROR_CODE;
        }

        p_slotsymbol_1 = (struct phy_dl_slotsymbol *) p_msg_in1->p_buf;
        if (p_slotsymbol_1 == NULL
            || p_slotsymbol_1->slot_header == NULL
            || (p_slotsymbol_1->is_broadcast == 1 
                && p_slotsymbol_1->mimo_mode != 0)) 
        {
            FLOG_FATAL("phy_dl_tx_div1: Error in received MAC slot symbol!\n");
            return ERROR_CODE;
        }

		DO_DUMP(DUMP_PHY_TX_SLOTSYMBOL_ANT1_IN_ID, 0, p_slotsymbol_1, 1);
		DO_DUMP(DUMP_PHY_TX_SLOTSDATA_ANT1_IN_ID, 0, p_slotsymbol_1, 1);

        /* update Interference information */
        phydts_info = (struct phy_dts_info *)p_slotsymbol_1->p_dts_info;
        if (phydts_info != NULL)
        {
            memcpy(para->active_band, phydts_info->active_band, 21*sizeof(int8_t));
            para->dl_unused_subch = phydts_info->dl_unused_subch;

            if (g_periodic_sensing_drop == 1)
            {
                g_periodic_sensing_enable = 0;
                g_periodic_sensing_reset = 0;
                g_periodic_sensing_drop = 0;
            }
        }


        /* Update system parameters */
        para->frame_index = p_slotsymbol_1->frame_index;
        para->symbol_offset = p_slotsymbol_1->symboloffset;
        para->mimo_mode = p_slotsymbol_1->mimo_mode;
/* in M2M FOAK, there are only one zone for DL on current stage, as First Zone */
/*
        if (para->symbol_offset == 1)
            para->first_zone = 1;
        else
            para->first_zone = 0;
*/

        /** Generate preamble or/and one slot_symbol */
        /* Malloc and initialize for rru_symbols - antenna 1 */
        p_rru_symbol_ant1 = phy_dl_init_rrusymbol(para,
                                                  p_slotsymbol_1);

        /* Single channel ofdma symbols generation */
        err_code = phy_dl_tx_single(para,
                                    p_slotsymbol_1,
                                    p_rru_symbol_ant1);
        if (err_code)
        {
            FLOG_ERROR("phy_dl_tx_div1: Error in phy_dl_tx_single!\n");
            continue;
        }

        /* Push rru symbol to the queue - antenna 1 */
        p_msg_out1->p_buf = (void *)p_rru_symbol_ant1;

		DO_DUMP(DUMP_PHY_TX_RRUSYMBOL_ANT1_ID, 0, p_rru_symbol_ant1, 1);

		DO_DUMP(DUMP_PHY_TX_I_ANT1_ID, 0, p_rru_symbol_ant1->symbol_i, 
			     p_rru_symbol_ant1->symbol_num*p_rru_symbol_ant1->symbol_len);


		DO_DUMP(DUMP_PHY_TX_Q_ANT1_ID, 0, p_rru_symbol_ant1->symbol_q, 
			     p_rru_symbol_ant1->symbol_num*p_rru_symbol_ant1->symbol_len);



        if ( wmrt_enqueue( out_que_id, p_msg_out1,
             sizeof(struct queue_msg) ) == -1) 
        {
            FLOG_ERROR("phy_dl_tx_div1: Error in write dl sub_frame!\n");
            continue;
        }



        /* Release the slot_symbol */
        err_code = adapter_dl_deinit_physlotsymbol(&p_slotsymbol_1);
        if (err_code == -1) 
        {
            FLOG_ERROR("phy_dl_tx_div1: Error in release slotsymbol!\n");
            continue;
        }
    } /* while (1)  */

    /* Releae message sturcture */
    if (p_msg_in1 != NULL) 
    {
        free(p_msg_in1);
        p_msg_in1 = NULL;
    }
    if (p_msg_out1 != NULL) 
    {
        free(p_msg_out1);
        p_msg_out1 = NULL;
    }

    return SUCCESS_CODE;
}


/**----------------------------------------------------------------------------
   Function:    phy_dl_tx_div2()

   Description: To genarate dl sub frame.

   Parameters:
                Input-  [struct phy_dl_tx_syspara *para]  The pointer refers
                        to the struct of system parameters.
                        [const u_int32_t in_que_id1] Antenna 1 input queue id.
                        [const u_int32_t in_que_id2] Antenna 2 input queue id.

                Output- [const u_int32_t out_que_id1] Antenna 1 output queue id.
                        [const u_int32_t out_que_id2] Antenna 2 output queue id.

   Return Value:
                0       Success
                150     Error

   ----------------------------------------------------------------------------
   LOG END TAG zYx                                                            */
int32_t phy_dl_tx_div2(struct phy_dl_tx_syspara *para,
                       const u_int32_t in_que_id1,
                       const u_int32_t in_que_id2,
                       const u_int32_t out_que_id1,
                       const u_int32_t out_que_id2)
{
    int32_t err_code;
    struct phy_dl_slotsymbol *p_slotsymbol_1, *p_slotsymbol_2;
    struct phy_dl_rru_symbol *p_rru_symbol_ant1, *p_rru_symbol_ant2;
    struct phy_dts_info *phydts_info;

    struct queue_msg * p_msg_in1 = my_malloc (sizeof(struct queue_msg));
    struct queue_msg * p_msg_in2 = my_malloc (sizeof(struct queue_msg));
    struct queue_msg * p_msg_out1 = my_malloc (sizeof(struct queue_msg));
    struct queue_msg * p_msg_out2 = my_malloc (sizeof(struct queue_msg));
    
    if (p_msg_in1 == NULL || p_msg_in2 == NULL || p_msg_out1 == NULL || p_msg_out2 == NULL )
    {
        FLOG_ERROR("NULL PTR of malloc in function phy_dl_tx_div1");
        return 1;
    }
    if (para == NULL) 
    {
        FLOG_FATAL("phy_dl_tx_div2: The pointer refer to sys para is null!\n");
        return ERROR_CODE;
    }
    if (para->frame_index == 1){
        FLOG_INFO("begain phy_dl_tx_div2...\n");
    }

    p_slotsymbol_1 = NULL;
    p_slotsymbol_2 = NULL;

    p_msg_in1->my_type = in_que_id1;
    p_msg_in2->my_type = in_que_id2;
    p_msg_out1->my_type = out_que_id1;
    p_msg_out2->my_type = out_que_id2;

    while (1) 
    {
        /** Receive one slot_symbol from MAC layer */
        if (wmrt_dequeue (in_que_id1, p_msg_in1, sizeof(struct queue_msg)) == -1) 
        {
            FLOG_FATAL("phy_dl_tx_div2: In PHY layer DEQUEUE ERROR\n");
            return ERROR_CODE;
        }

	PROF_START_TIMER( PROF_TIMER_TOKEN( PHY_DL ));

        p_slotsymbol_1 = (struct phy_dl_slotsymbol *) p_msg_in1->p_buf;

        if (p_slotsymbol_1 == NULL
            || p_slotsymbol_1->slot_header == NULL
            || (p_slotsymbol_1->is_broadcast == 1 
                && p_slotsymbol_1->mimo_mode != 0)) 
        {
            FLOG_ERROR("phy_dl_tx_div2: p_slotsymbol_1=%p, p_slotsymbol_1->slot_header=%p, p_slotsymbol_1->is_broadcast=%d, p_slotsymbol_1->mimo_mode=%d!\n", p_slotsymbol_1, p_slotsymbol_1->slot_header, p_slotsymbol_1->is_broadcast, p_slotsymbol_1->mimo_mode);
            FLOG_FATAL("phy_dl_tx_div2: Error in received MAC slot symbol!\n");
            return ERROR_CODE;
        }

		DO_DUMP(DUMP_PHY_TX_SLOTSYMBOL_ANT1_IN_ID, 0, p_slotsymbol_1, 1);
		DO_DUMP(DUMP_PHY_TX_SLOTSDATA_ANT1_IN_ID, 0, p_slotsymbol_1, 1);

         /* update Interference information */
         phydts_info = (struct phy_dts_info *)p_slotsymbol_1->p_dts_info;
        if (phydts_info != NULL)
        {
            memcpy(para->active_band, phydts_info->active_band, 21*sizeof(int8_t));
            para->dl_unused_subch = phydts_info->dl_unused_subch;

            if (g_periodic_sensing_drop == 1)
            {
                g_periodic_sensing_enable = 0;
                g_periodic_sensing_reset = 0;
                g_periodic_sensing_drop = 0;
            }

        }
        if (para->frame_index==1){        
            FLOG_INFO("para->dl_unused_subch= %d\n", para->dl_unused_subch);    
        }
        if (p_slotsymbol_1->mimo_mode == 2) 
        {
            if (wmrt_dequeue (in_que_id2, p_msg_in2, sizeof(struct queue_msg)) == -1) 
            {
                FLOG_FATAL ("phy_dl_tx_div2: In PHY layer DEQUEUE ERROR\n");
                return ERROR_CODE;
            }
            p_slotsymbol_2 = (struct phy_dl_slotsymbol *) p_msg_in2->p_buf;
            if ( p_slotsymbol_2 == NULL
                || p_slotsymbol_2->slot_header == NULL
                || p_slotsymbol_1->symboloffset != p_slotsymbol_2->symboloffset
                || p_slotsymbol_1->slotlength != p_slotsymbol_2->slotlength
                || p_slotsymbol_1->is_broadcast != p_slotsymbol_2->is_broadcast
                || p_slotsymbol_1->mimo_mode != p_slotsymbol_2->mimo_mode
                || p_slotsymbol_1->frame_index != p_slotsymbol_2->frame_index
                || p_slotsymbol_1->dl_subframe_end_flag 
                   != p_slotsymbol_2->dl_subframe_end_flag ) 
            {
                FLOG_FATAL("phy_dl_tx_div2: Error in received MAC slot symbol!\n");
                return ERROR_CODE;
            }

			DO_DUMP(DUMP_PHY_TX_SLOTSYMBOL_ANT2_IN_ID, 0, p_slotsymbol_2, 1);
			DO_DUMP(DUMP_PHY_TX_SLOTSDATA_ANT2_IN_ID, 0, p_slotsymbol_2, 1);
			
        }

        /* Update system parameters */
        para->frame_index = p_slotsymbol_1->frame_index;
        para->symbol_offset = p_slotsymbol_1->symboloffset;
        para->mimo_mode = p_slotsymbol_1->mimo_mode;
        /* in the current stage, only one First Zone */
        /*
        if (para->symbol_offset == 1)
            para->first_zone = 1;
        else
            para->first_zone = 0;
        */

        /** Generate preamble or/and one slot_symbol */
        switch(para->mimo_mode) 
        {
            case CDD: /* CDD mode */
                /* Malloc and initialize for rru_symbols - antenna 1 */
                p_rru_symbol_ant1 = phy_dl_init_rrusymbol(para,
                                                          p_slotsymbol_1);
                /* Malloc and initialize for rru_symbols - antenna 2 */
                p_rru_symbol_ant2 = phy_dl_init_rrusymbol(para,
                                                          p_slotsymbol_1);

                p_rru_symbol_ant1->dl_perscan_flag = p_slotsymbol_1->dl_perscan_flag;
                p_rru_symbol_ant2->dl_perscan_flag = p_slotsymbol_1->dl_perscan_flag;
                
                /* Single channel ofdma symbols generation */
                err_code = phy_dl_tx_cdd(para,
                                         p_slotsymbol_1,
                                         p_rru_symbol_ant1,
                                         p_rru_symbol_ant2);
                if (err_code) 
                {
                    FLOG_ERROR("phy_dl_tx_div2: Error in phy_dl_tx_cdd!\n");
                    return err_code;
                }

				
				DO_DUMP(DUMP_PHY_TX_RRUSYMBOL_ANT1_ID, 0, p_rru_symbol_ant1, 1);
				
				DO_DUMP(DUMP_PHY_TX_I_ANT1_ID, 0, p_rru_symbol_ant1->symbol_i, 
						 p_rru_symbol_ant1->symbol_num*p_rru_symbol_ant1->symbol_len);
				
				
				DO_DUMP(DUMP_PHY_TX_Q_ANT1_ID, 0, p_rru_symbol_ant1->symbol_q, 
						 p_rru_symbol_ant1->symbol_num*p_rru_symbol_ant1->symbol_len);
				
			
				DO_DUMP(DUMP_PHY_TX_RRUSYMBOL_ANT2_ID, 0, p_rru_symbol_ant2, 1);
				
				DO_DUMP(DUMP_PHY_TX_I_ANT2_ID, 0, p_rru_symbol_ant2->symbol_i, 
						 p_rru_symbol_ant2->symbol_num*p_rru_symbol_ant1->symbol_len);
				
				
				DO_DUMP(DUMP_PHY_TX_Q_ANT2_ID, 0, p_rru_symbol_ant2->symbol_q, 
						 p_rru_symbol_ant2->symbol_num*p_rru_symbol_ant1->symbol_len);
				

                /* Push rru symbol to the queue - antenna 1 */
                p_msg_out1->p_buf = (void *)p_rru_symbol_ant1;
                if ( wmrt_enqueue( out_que_id1, p_msg_out1,
                     sizeof(struct queue_msg) ) == -1) 
                {
                    FLOG_FATAL("phy_dl_tx_cdd: Error in write dl sub_frame for anttena0!\n");
                    return err_code;
                }

                 /* Push rru symbol to the queue - antenna 2 */
                p_msg_out2->p_buf = (void *)p_rru_symbol_ant2;

                if ( wmrt_enqueue( out_que_id2, p_msg_out2,
                     sizeof(struct queue_msg) ) == -1)
                {
                    FLOG_FATAL("phy_dl_tx_cdd: Error in write dl sub_frame for antenna1!\n");
                    return err_code;
                }

                pthread_mutex_lock(&mutex_tx_phy_en_flag);
                tx_phy_en_flag ++;
                pthread_mutex_unlock(&mutex_tx_phy_en_flag);

                /* Release the slot_symbol */
                err_code = adapter_dl_deinit_physlotsymbol(&p_slotsymbol_1);
                if (err_code == -1) 
                {
                    FLOG_ERROR("phy_dl_tx_cdd: Error in release slotsymbol!\n");
                    return err_code;
                }
                break;
            case STCA: /* STC matrix A */
                if (para->frame_index==1){  
                    FLOG_INFO("STCA: symbol offset=%d...\n", para->symbol_offset);
                }
                /* Malloc and initialize for rru_symbols - antenna 1 */
                p_rru_symbol_ant1 = phy_dl_init_rrusymbol(para,
                                                          p_slotsymbol_1);

                /* Malloc and initialize for rru_symbols - antenna 2 */
                p_rru_symbol_ant2 = phy_dl_init_rrusymbol(para,
                                                          p_slotsymbol_1);

                /* STC matrix A ofdma symbols generation */
                err_code = phy_dl_tx_stca(para,
                                          p_slotsymbol_1,
                                          p_rru_symbol_ant1,
                                          p_rru_symbol_ant2);
                if (err_code) 
                {
                    FLOG_ERROR("phy_dl_tx_div2: Error in phy_dl_tx_stca!\n");
                    return err_code;
                }

                /* Push rru symbole to the queue - antenna 1 */
                p_msg_out1->p_buf = (void *)p_rru_symbol_ant1;

                if ( wmrt_enqueue( out_que_id1, p_msg_out1,
                     sizeof(struct queue_msg) ) == -1) 
                {
                    FLOG_FATAL("phy_dl_tx_stca: Error in write dl sub_frame for antenna 0!\n");
                    return err_code;
                }

                p_msg_out2->p_buf = (void *)p_rru_symbol_ant2;

                if ( wmrt_enqueue( out_que_id2, p_msg_out2,
                     sizeof(struct queue_msg) ) == -1) 
                {
                    FLOG_FATAL("phy_dl_tx_stca: Error in write dl sub_frame for antenna 1!\n");
                    return err_code;
                }

                pthread_mutex_lock(&mutex_tx_phy_en_flag);
                tx_phy_en_flag ++;
                pthread_mutex_unlock(&mutex_tx_phy_en_flag);


                /* Release the phy slot_symbol */
                err_code = adapter_dl_deinit_physlotsymbol(&p_slotsymbol_1);
                if (err_code == -1) 
                {
                    FLOG_ERROR("phy_dl_tx_stca: Error in release slotsymbol!\n");
                    return err_code;
                }
                break;
            case STCB: /* STC matrix B */
                if (para->frame_index==1){
                    FLOG_INFO("STCB: symbol offset=%d...\n", para->symbol_offset);}
                /* Malloc and initialize for rru_symbols - antenna 1 */
                p_rru_symbol_ant1 = phy_dl_init_rrusymbol(para,
                                                          p_slotsymbol_1);

                /* Malloc and initialize for rru_symbols - antenna 2 */
                p_rru_symbol_ant2 = phy_dl_init_rrusymbol(para,
                                                          p_slotsymbol_2);

                /* STC matrix B ofdma symbols generation */
                err_code = phy_dl_tx_stcb(para,
                                          p_slotsymbol_1,
                                          p_slotsymbol_2,
                                          p_rru_symbol_ant1,
                                          p_rru_symbol_ant2);
                if (err_code) 
                {
                    FLOG_ERROR("phy_dl_tx_div2: Error in phy_dl_tx_stcb!\n");
                    return err_code;
                }

                p_msg_out1->p_buf = (void *)p_rru_symbol_ant1;
                if ( wmrt_enqueue( out_que_id1, p_msg_out1,
                     sizeof(struct queue_msg) ) == -1) 
                {
                    FLOG_FATAL("phy_dl_tx_stcb: Error in write dl sub_frame for antenna0!\n");
                    return err_code;
                }

                /* Push rru symbole to the queue - antenna 2 */
                p_msg_out2->p_buf = (void *)p_rru_symbol_ant2;

                if ( wmrt_enqueue( out_que_id2, p_msg_out2,
                     sizeof(struct queue_msg) ) == -1) 
                {
                    FLOG_FATAL("phy_dl_tx_stcb: Error in write dl sub_frame for antenna1!\n");
                    return err_code;
                }

                pthread_mutex_lock(&mutex_tx_phy_en_flag);
                tx_phy_en_flag ++;
                pthread_mutex_unlock(&mutex_tx_phy_en_flag);

                /* Release the phy slot_symbol - 1 */
                err_code = adapter_dl_deinit_physlotsymbol(&p_slotsymbol_1);
                if (err_code == -1) 
                {
                    FLOG_ERROR("phy_dl_tx_div2: Error in release slotsymbol for antenna0!\n");
                    return err_code;
                }
                /* Release the phy slot_symbol - 2 */
                err_code = adapter_dl_deinit_physlotsymbol(&p_slotsymbol_2);
                if (err_code == -1) 
                {
                    FLOG_ERROR("phy_dl_tx_div2: Error in release slotsymbol for antenna1!\n");
                    return err_code;
                }
                break;
            default:
                FLOG_WARNING( "phy_dl_tx_div2: Unsupported mimo mode!\n");
                return ERROR_CODE;
        } /* switch(sys_para.mimo_mode) */
	PROF_STOP_TIMER( PROF_TIMER_TOKEN( PHY_DL ));
    } /* while (1)  */

    /* Releae message sturcture */
    if (p_msg_in1 != NULL) 
    {
        free(p_msg_in1);
        p_msg_in1 = NULL;
    }
    if (p_msg_in2 != NULL) 
    {
        free(p_msg_in2);
        p_msg_in2 = NULL;
    }
    if (p_msg_out1 != NULL) 
    {
        free(p_msg_out1);
        p_msg_out1 = NULL;
    }
    if (p_msg_out2 != NULL) 
    {
        free(p_msg_out2);
        p_msg_out2 = NULL;
    }

/*
#ifdef _DEBUG_
#ifdef _DEBUG_LEVEL_2_
    close_file_dl(fp1);
#endif
#ifdef _DEBUG_LEVEL_1_
    close_file_dl(fp2);
#endif
#endif
*/
    return SUCCESS_CODE;
}


/**----------------------------------------------------------------------------
   Function:    phy_dl_tx_single()

   Description: To genarate DL sub-frame for single channel.

   Parameters:
                Input-  [struct phy_dl_tx_syspara *para]  The pointer refers
                        to the struct of system parameters.
                        [const phy_dl_slotsymbol *p_slotsymbol]  The
                        pointer refer to one input slot symbol.

                Output- [struct phy_dl_rru_symbol *p_dl_rru_symbol_ant1]  The 
                        pointer refer to the output rru symbol buffer of 
                        antenna 1.
   Return Value:
                0       Success
                150     Error

   ----------------------------------------------------------------------------
   LOG END TAG zYx                                                            */
int phy_dl_tx_single(struct phy_dl_tx_syspara *para,
                     const struct phy_dl_slotsymbol *p_slotsymbol,
                     struct phy_dl_rru_symbol *p_dl_rru_symbol_ant1)
{
    struct phy_dl_slot *first_slot, *current_slot, *next_slot;
    unsigned int subchannel_count;
    unsigned int num_of_slots, fec_len, nused_subch;
    u_int32_t i;
    float zone_boost;
    int err_code;
#ifdef VSXOPT
    float a_r[3*840] __attribute__ ((aligned (128))), a_i[3*840] __attribute__ ((aligned (128))), b_r[3*840] __attribute__ ((aligned (128))), b_i[3*840] __attribute__ ((aligned (128)));
#else
    float a_r[3*840], a_i[3*840], b_r[3*840], b_i[3*840];
#endif
    u_int32_t pilot_allocation[3*96], data_allocation[768*3];
    
    /* clean memory for slot_symbol use */
    memset(a_r,0,sizeof(float)*3 * 840);
    memset(a_i,0,sizeof(float)*3 * 840);
    memset(b_r,0,sizeof(float)*3 * 840);
    memset(b_i,0,sizeof(float)*3 * 840);

    float  *dl_preamble_r, *dl_preamble_i;
    float  *dl_burst_r, *dl_burst_i;

    float *fec_r, *fec_i, *muxofdm_r, *muxofdm_i;
    float *subcarrandom_r, *subcarrandom_i;

    if (para == NULL || p_slotsymbol == NULL) {
        FLOG_FATAL("phy_dl_tx_single: The pointer refer to input buf is null!\n");
        return ERROR_CODE;
    }

    if (p_dl_rru_symbol_ant1 ==  NULL) {
        FLOG_FATAL("phy_dl_tx_single: The pointer refer to output buf is null!\n");
        return ERROR_CODE;
    }

    /**********************************************************************/
    /** Update output buffer address */
    if (para->symbol_offset == 1) 
    {
        dl_preamble_r = p_dl_rru_symbol_ant1->symbol_i;
        dl_preamble_i = p_dl_rru_symbol_ant1->symbol_q;
        dl_burst_r = dl_preamble_r + para->ofdma_symlen_with_guard;
        dl_burst_i = dl_preamble_i + para->ofdma_symlen_with_guard;
    }
    else {
        dl_burst_r = p_dl_rru_symbol_ant1->symbol_i;
        dl_burst_i = p_dl_rru_symbol_ant1->symbol_q;
    }
    if (para->frame_index==1){
        FLOG_INFO("enter into single processing!\n");
        FLOG_DEBUG("para->symbol_offset = %d\n", para->symbol_offset);
    }
    /**********************************************************************/
    /** If symbol_offset == 1
            preamble generation, FCH, DL_MAP, data burst fec encoding
        Else
            data burst fec encoding */

     if (para->symbol_offset == 1)
    {

        /** Preamble generation */
        err_code = phy_dl_preamble_gen(para, 
                                       para->active_band,
                                       dl_preamble_r,
                                       dl_preamble_i);
        if (err_code)
        {
            FLOG_ERROR("phy_dl_tx_single: Error in preamble generator!\n");
            return err_code;
        }
#ifdef _ZONE_BOOST_

    zone_boost = sqrt( (float)para->ofdma_nfft );
    for (i=0; i< para->ofdma_symlen_with_guard;i++)
    {
        dl_preamble_r[i] = dl_preamble_r[i]*zone_boost;
        dl_preamble_i[i] = dl_preamble_i[i]*zone_boost;
    }
#endif
                                    

		DO_DUMP(DUMP_PHY_TX_PREAMBLE_ANT1_I_ID, 0, dl_preamble_r, para->ofdma_symlen_with_guard);
		DO_DUMP(DUMP_PHY_TX_PREAMBLE_ANT1_Q_ID, 0, dl_preamble_i, para->ofdma_symlen_with_guard);

                                                                                                                                                /** FCH fec encoding */
      /* Find the pointer refers to DL_MAP */
        first_slot = current_slot = p_slotsymbol->slot_header;
        next_slot  = current_slot->next;
        num_of_slots = 1;

		DO_DUMP(DUMP_PHY_TX_FEC_ANT1_IN_ID, 0, current_slot, 1);
           
        for (subchannel_count=0; subchannel_count<3; subchannel_count++)
        {
            /* Compose the processing block */
            current_slot = next_slot;
            next_slot = current_slot->next;

			DO_DUMP(DUMP_PHY_TX_FEC_ANT1_IN_ID, 0, current_slot, 1);

        }
        /* FEC encoding - Ramdomizer
                          CC encoder and puncture
                          Interleaver
                          Repetition
                          Modulation */
        fec_r = a_r;
        fec_i = a_i;
        fec_len = 0;
        err_code = phy_dl_fch_fecencoding(para,
                                          first_slot,
                                          4,
                                          fec_r,
                                          fec_i,
                                          &fec_len);
        if (err_code)
        {
            FLOG_ERROR("phy_dl_tx_single: Error in fch encoding!\n");
            return err_code;
        }
        fec_r += fec_len;
        fec_i += fec_len;
        fec_len = 0;

        first_slot = current_slot = next_slot;
        next_slot = current_slot->next;
        num_of_slots = 1;

		DO_DUMP(DUMP_PHY_TX_FEC_ANT1_IN_ID, 0, current_slot, 1);


         /** DL_MAP and DL burst fec encoding */
        for (subchannel_count=4;
             subchannel_count<para->num_subch-1; subchannel_count++)
        {

            /* Compose the processing block */
            if (current_slot->block_id == next_slot->block_id)
            {
                current_slot = next_slot;
                next_slot = current_slot->next;
                num_of_slots ++;
				DO_DUMP(DUMP_PHY_TX_FEC_ANT1_IN_ID, 0, current_slot, 1);

                continue;
            }

           /* judge the unused subchannel, for unused subchan, not processing */

            if (current_slot->unused_flag == 0)
            {
            /* FEC encoding - Ramdomizer
                              CC encoder and puncture
                              Interleaver
                              Repetition
                              Modulation */
                err_code = phy_dl_fec_encoding(para,
                                               first_slot,
                                               num_of_slots,
                                               fec_r,
                                               fec_i,
                                               &fec_len);

                if (err_code)
                {
                    FLOG_ERROR("phy_dl_tx_single: Error in fec encoding!\n");
                    return err_code;
                }
                fec_r += fec_len;
                fec_i += fec_len;
                fec_len = 0;
            }
            else
            {
                fec_r += para->num_subcar_of_subch * num_of_slots;
                fec_i += para->num_subcar_of_subch * num_of_slots;
                fec_len = 0;
            }

            first_slot = current_slot = next_slot;
            next_slot = current_slot->next;
            num_of_slots = 1;
			DO_DUMP(DUMP_PHY_TX_FEC_ANT1_IN_ID, 0, current_slot, 1);

        } /* End of for (subchannel_count=4...) */
    } /* End of if (para->symbol_offset == 1) */
    else /* Not the first slot symbol */
    {
        /** FCH, DL_MAP and DL burst generation */
        fec_r = a_r;
        fec_i = a_i;
        fec_len = 0;

        first_slot = current_slot = p_slotsymbol->slot_header;
        next_slot  = current_slot->next;
        num_of_slots = 1;
		DO_DUMP(DUMP_PHY_TX_FEC_ANT1_IN_ID, 0, current_slot, 1);

        for (subchannel_count=0;
             subchannel_count<para->num_subch-1; subchannel_count++)
        {
             /* Compose the processing block */
            if (current_slot->block_id == next_slot->block_id)
            {
                current_slot = next_slot;
                next_slot = current_slot->next;
                num_of_slots ++;
				DO_DUMP(DUMP_PHY_TX_FEC_ANT1_IN_ID, 0, current_slot, 1);

                continue;
            }

            if (current_slot->unused_flag == 0)
            {

            /* FEC encoding - Ramdomizer
                              CC encoder and puncture
                              Interleaver
                              Repetition
                              Modulation */
                err_code = phy_dl_fec_encoding(para,
                                               first_slot,
                                               num_of_slots,
                                               fec_r,
                                               fec_i,
                                               &fec_len);
                if (err_code)
                {
                    FLOG_ERROR("phy_dl_tx_single: Error in fec encoding!\n");
                    return err_code;
                }
                fec_r += fec_len;
                fec_i += fec_len;
                fec_len = 0;
            }
            else
            {
                 fec_r += para->num_subcar_of_subch * num_of_slots;
                 fec_i += para->num_subcar_of_subch * num_of_slots;
                 fec_len = 0;
            }


            first_slot = current_slot = next_slot;
            next_slot = current_slot->next;
            num_of_slots = 1;
			DO_DUMP(DUMP_PHY_TX_FEC_ANT1_IN_ID, 0, current_slot, 1);

        } /* End of for (subchannel_count=0...) */
    } /* End of if (para->symbol_offset==1) else */

    /* FEC encoding of last processing block */

    if (current_slot->unused_flag == 0)
    {
        err_code = phy_dl_fec_encoding(para,
                                       first_slot,
                                       num_of_slots,
                                       fec_r,
                                       fec_i,
                                       &fec_len);

        if (err_code)
        {
            FLOG_ERROR("phy_dl_tx_single: Error in fec encoding!\n");
            return err_code;
        }
    }
    else
    {
        fec_r += para->num_subcar_of_subch * num_of_slots;
        fec_i += para->num_subcar_of_subch * num_of_slots;

    }

    fec_r = a_r;
    fec_i = a_i;


	
	DO_DUMP(DUMP_PHY_TX5_FEC_ANT1_I_ID, 0, fec_r, (para->ofdma_nused_no_dc - para->num_pilot)*para->symbol_per_slot);

	DO_DUMP(DUMP_PHY_TX5_FEC_ANT1_Q_ID, 0, fec_i, (para->ofdma_nused_no_dc - para->num_pilot)*para->symbol_per_slot);
                                                        


   /* DL Zone Permutation */
   err_code = phy_dl_zonepermutation(para,
                                     para->active_band,
                                     pilot_allocation,
                                     data_allocation); 
    if (err_code)
    {
        FLOG_ERROR("E009_tx: Error in DL zonepermutation!\n");
        return err_code;
    }

	DO_DUMP(DUMP_PHY_TX_PILOT_ANT1_ID, 0, pilot_allocation, (42-para->dl_unused_subch)*6);

	DO_DUMP(DUMP_PHY_TX_DATA_ALLOC_ANT1_ID, 0, data_allocation, 48*(42-para->dl_unused_subch));


    /* OFDM symbol multiplex (map constellation) */
    

    muxofdm_r = b_r;
    muxofdm_i = b_i;

    err_code = phy_dl_muxofdmsym(para,
                                 para->dl_unused_subch,
                                 fec_r,
                                 fec_i,
                                 data_allocation,
                                 pilot_allocation,
                                 muxofdm_r,
                                 muxofdm_i);

    if (err_code)
    {
        FLOG_ERROR("E0010_tx: Error in muxofdmsym!\n");
        return err_code;
    }


	DO_DUMP(DUMP_PHY_TX4_MULTIPLEX_ANT1_I_ID, 0, muxofdm_r, para->ofdma_nused_no_dc*para->symbol_per_slot);
	DO_DUMP(DUMP_PHY_TX4_MULTIPLEX_ANT1_Q_ID, 0, muxofdm_i, para->ofdma_nused_no_dc*para->symbol_per_slot);

    /* Subcarrier randomize */

    subcarrandom_r = a_r;
    subcarrandom_i = a_i;

    err_code = phy_dl_subcarrandom(para,
                                   para->wk,
                                   muxofdm_r,
                                   muxofdm_i,
                                   subcarrandom_r,
                                   subcarrandom_i);
#ifdef _ZONE_BOOST_
    FLOG_DEBUG("execute zone boost\n");
    nused_subch = para->num_subch - para->dl_unused_subch;
    zone_boost = sqrt( (float)para->num_subch/(float)nused_subch );
    FLOG_DEBUG("zone_boost = %f\n", zone_boost);
    for (i=0; i< (para->ofdma_nused-1)* para->symbol_per_slot;i++)
    {
        subcarrandom_r[i] = subcarrandom_r[i]*zone_boost;
        subcarrandom_i[i] = subcarrandom_i[i]*zone_boost;
    }
#endif

    if (err_code)
    {
        FLOG_ERROR("E008_tx: Error in subcarrandom!\n");
        return err_code;
    }


	DO_DUMP(DUMP_PHY_TX2_SUBCARAND_ANT1_I_ID, 0, subcarrandom_r, para->ofdma_nused_no_dc*para->symbol_per_slot);
	DO_DUMP(DUMP_PHY_TX2_SUBCARAND_ANT1_Q_ID, 0, subcarrandom_i, para->ofdma_nused_no_dc*para->symbol_per_slot);



    /* OFDMA modulation */
    err_code = phy_dl_ofdmamodul(para,
                                 subcarrandom_r,
                                 subcarrandom_i,
                                 dl_burst_r,
                                 dl_burst_i);
#ifdef _ZONE_BOOST_
    nused_subch = para->num_subch - para->dl_unused_subch;
    zone_boost = sqrt( (float)para->ofdma_nfft );
    for (i=0; i< para->ofdma_symlen_with_guard * para->symbol_per_slot;i++)
    {
        dl_burst_r[i] = dl_burst_r[i]*zone_boost;
        dl_burst_i[i] = dl_burst_i[i]*zone_boost;
    }
#endif



    if (err_code) 
    {
        FLOG_ERROR("phy_dl_tx_single: Error in ofdma modulation!\n");
        return err_code;
    }


	DO_DUMP(DUMP_PHY_TX1_OFDMAMOD_ANT1_I_ID, 0, dl_burst_r, para->ofdma_symlen_with_guard*para->symbol_per_slot);
	DO_DUMP(DUMP_PHY_TX1_OFDMAMOD_ANT1_Q_ID, 0, dl_burst_i, para->ofdma_symlen_with_guard*para->symbol_per_slot);


    return SUCCESS_CODE;
}

/*  CDD MOde */

int phy_dl_tx_cdd(struct phy_dl_tx_syspara *para,
                  const struct phy_dl_slotsymbol *p_slotsymbol,
                  struct phy_dl_rru_symbol *p_dl_rru_symbol_ant1,
                  struct phy_dl_rru_symbol *p_dl_rru_symbol_ant2)
{
    struct phy_dl_slot *first_slot, *current_slot, *next_slot;
    unsigned int subchannel_count;
    unsigned int num_of_slots, fec_len, nused_subch;
    u_int32_t i;
    float zone_boost;
    int err_code;
#ifdef VSXOPT
    float a_r[3*840] __attribute__ ((aligned (128))), a_i[3*840] __attribute__ ((aligned (128))), b_r[3*840] __attribute__ ((aligned (128))), b_i[3*840] __attribute__ ((aligned (128)));
#else
    float a_r[3*840], a_i[3*840], b_r[3*840], b_i[3*840];
#endif
    u_int32_t pilot_allocation[3*96], data_allocation[768*3];
    
 
    /* clean memory for slot_symbol use */
    memset(a_r,0,sizeof(float)*3 * 840);
    memset(a_i,0,sizeof(float)*3 * 840);
    memset(b_r,0,sizeof(float)*3 * 840);
    memset(b_i,0,sizeof(float)*3 * 840);

    float  *dl_preamble_r_ant1, *dl_preamble_i_ant1;
    float  *dl_preamble_r_ant2, *dl_preamble_i_ant2;
    float  *dl_burst_r_ant1, *dl_burst_i_ant1;
    float  *dl_burst_r_ant2, *dl_burst_i_ant2;

    float *fec_r, *fec_i, *muxofdm_r, *muxofdm_i;
    float *subcarrandom_r, *subcarrandom_i;

    if (para == NULL || p_slotsymbol == NULL) {
        FLOG_FATAL("phy_dl_tx_cdd: The pointer refer to input buf is null!\n");
        return ERROR_CODE;
    }

    if (p_dl_rru_symbol_ant1 ==  NULL || p_dl_rru_symbol_ant2 == NULL) {
        FLOG_FATAL("phy_dl_tx_cdd: The pointer refer to output buf is null!\n");
        return ERROR_CODE;
    }

    /**********************************************************************/
    /** Update output buffer address */
    if (para->symbol_offset == 1) 
    {
        dl_preamble_r_ant1 = p_dl_rru_symbol_ant1->symbol_i;
        dl_preamble_i_ant1 = p_dl_rru_symbol_ant1->symbol_q;
        dl_burst_r_ant1 = dl_preamble_r_ant1 + para->ofdma_symlen_with_guard;
        dl_burst_i_ant1 = dl_preamble_i_ant1 + para->ofdma_symlen_with_guard;
		
        dl_preamble_r_ant2 = p_dl_rru_symbol_ant2->symbol_i;
        dl_preamble_i_ant2 = p_dl_rru_symbol_ant2->symbol_q;
	dl_burst_r_ant2 = dl_preamble_r_ant2 + para->ofdma_symlen_with_guard;
        dl_burst_i_ant2 = dl_preamble_i_ant2 + para->ofdma_symlen_with_guard;
    }
    else
    {
        dl_burst_r_ant1 = p_dl_rru_symbol_ant1->symbol_i;
        dl_burst_i_ant1 = p_dl_rru_symbol_ant1->symbol_q;
		
	dl_burst_r_ant2 = p_dl_rru_symbol_ant2->symbol_i;
        dl_burst_i_ant2 = p_dl_rru_symbol_ant2->symbol_q;
    }
#ifdef DUMP_PHY_DL_TX
    FLOG_DEBUG("enter into cdd processing!\n");
    FLOG_DEBUG("para->symbol_offset = %d\n", para->symbol_offset);
#endif
    /**********************************************************************/
    /** If symbol_offset == 1
            preamble generation, FCH, DL_MAP, data burst fec encoding
        Else
            data burst fec encoding */

     if (para->symbol_offset == 1)
    {

    		PROF_START_TIMER( PROF_TIMER_TOKEN( PHY_DL_SYM1 ));
        /** Preamble generation */
#ifndef _MULTIBAND_
        err_code = phy_dl_preamble_gen(para, 
                                       para->active_band,
                                       dl_preamble_r_ant1,
                                       dl_preamble_i_ant1);
        if (err_code)
        {
            FLOG_ERROR("phy_dl_tx_cdd: Error in preamble generator for antenna0!\n");
            return err_code;
        }
#else
        err_code = phy_dl_preamble_gen_multi(para,
                                             para->active_band,
                                             para->dlusesc,
                                             dl_preamble_r_ant1,
                                             dl_preamble_i_ant1);
        if (err_code)
        {
            FLOG_ERROR("phy_dl_tx_cdd: Error in preamble generator for antenna0 under Multiband mode!\n");
            return err_code;
        }

#endif

#ifdef _ZONE_BOOST_
        zone_boost = sqrt( (float)para->ofdma_nfft );
        for (i=0; i< para->ofdma_symlen_with_guard;i++)
        {
            dl_preamble_r_ant1[i] = dl_preamble_r_ant1[i]*zone_boost;
            dl_preamble_i_ant1[i] = dl_preamble_i_ant1[i]*zone_boost;
        }
#endif

#ifndef _MULTIBAND_
        err_code = phy_dl_preamble_gen_cdd(para,
                                           para->active_band,
                                           dl_preamble_r_ant2,
                                           dl_preamble_i_ant2);
        if (err_code)
        {
            FLOG_ERROR("phy_dl_tx_cdd: Error in preamble generator for antenna1!\n");
            return err_code;
        }
#else
        err_code = phy_dl_preamble_gen_cdd_multi(para,
                                                 para->active_band,
                                                 para->dlusesc,
                                                 dl_preamble_r_ant2,
                                                 dl_preamble_i_ant2);
        if (err_code)
        {
            FLOG_ERROR("phy_dl_tx_cdd: Error in preamble generator for antenna1 under multiband mode!\n");
            return err_code;
        }
#endif

#ifdef _ZONE_BOOST_
        zone_boost = sqrt( (float)para->ofdma_nfft );
        for (i=0; i< para->ofdma_symlen_with_guard;i++)
        {
            dl_preamble_r_ant2[i] = dl_preamble_r_ant2[i]*zone_boost;
            dl_preamble_i_ant2[i] = dl_preamble_i_ant2[i]*zone_boost;
        }
#endif


		DO_DUMP(DUMP_PHY_TX_PREAMBLE_ANT1_I_ID, 0, dl_preamble_r_ant1, para->ofdma_symlen_with_guard);
		DO_DUMP(DUMP_PHY_TX_PREAMBLE_ANT1_Q_ID, 0, dl_preamble_i_ant1, para->ofdma_symlen_with_guard);
		DO_DUMP(DUMP_PHY_TX_PREAMBLE_ANT2_I_ID, 0, dl_preamble_r_ant2, para->ofdma_symlen_with_guard);
		DO_DUMP(DUMP_PHY_TX_PREAMBLE_ANT2_Q_ID, 0, dl_preamble_i_ant2, para->ofdma_symlen_with_guard);

#if 0
    FILE *fp1_i, *fp1_q;
    FILE *fp2_i, *fp2_q;
    fp1_i = fopen("./data_dst/1_preamble_i.out","w+t");
    fp1_q = fopen("./data_dst/1_preamble_q.out","w+t");
    fp2_i = fopen("./data_dst/1_preamble_cdd_i.out","w+t");
    fp2_q = fopen("./data_dst/1_preamble_cdd_q.out","w+t");
    dl_dump_ffloat(fp1_i, dl_preamble_r_ant1, 1088);
    dl_dump_ffloat(fp1_q, dl_preamble_i_ant1, 1088);
    dl_dump_ffloat(fp2_i, dl_preamble_r_ant2, 1088);
    dl_dump_ffloat(fp2_q, dl_preamble_i_ant2, 1088);
    fclose(fp1_i);
    fclose(fp1_q);
    fclose(fp2_i);
    fclose(fp2_q);
#endif
		
                                                                                                                                                /** FCH fec encoding */
    		PROF_STOP_TIMER( PROF_TIMER_TOKEN( PHY_DL_SYM1 ));
    		PROF_START_TIMER( PROF_TIMER_TOKEN( PHY_DL_FEC_ENCODE ));
      /* Find the pointer refers to DL_MAP */
        first_slot = current_slot = p_slotsymbol->slot_header;
        next_slot  = current_slot->next;
        num_of_slots = 1;

		DO_DUMP(DUMP_PHY_TX_FEC_ANT1_IN_ID, 0, current_slot, 1);
           
        for (subchannel_count=0; subchannel_count<3; subchannel_count++)
        {
            /* Compose the processing block */
            current_slot = next_slot;
            next_slot = current_slot->next;

			DO_DUMP(DUMP_PHY_TX_FEC_ANT1_IN_ID, 0, current_slot, 1);
        }
        /* FEC encoding - Ramdomizer
                          CC encoder and puncture
                          Interleaver
                          Repetition
                          Modulation */
        fec_r = a_r;
        fec_i = a_i;
        fec_len = 0;
        err_code = phy_dl_fch_fecencoding(para,
                                          first_slot,
                                          4,
                                          fec_r,
                                          fec_i,
                                          &fec_len);
        if (err_code)
        {
            FLOG_ERROR("phy_dl_tx_cdd: Error in fch encoding!\n");
            return err_code;
        }
        fec_r += fec_len;
        fec_i += fec_len;
        fec_len = 0;

        first_slot = current_slot = next_slot;
        next_slot = current_slot->next;
        num_of_slots = 1;

		DO_DUMP(DUMP_PHY_TX_FEC_ANT1_IN_ID, 0, current_slot, 1);

         /** DL_MAP and DL burst fec encoding */
        for (subchannel_count=4;
             subchannel_count<para->num_subch-1; subchannel_count++)
        {

            /* Compose the processing block */
            if (current_slot->block_id == next_slot->block_id)
            {
                current_slot = next_slot;
                next_slot = current_slot->next;
                num_of_slots ++;
				DO_DUMP(DUMP_PHY_TX_FEC_ANT1_IN_ID, 0, current_slot, 1);
                continue;
            }

           /* judge the unused subchannel, for unused subchan, not processing */

            if (current_slot->unused_flag == 0)
            {
            /* FEC encoding - Ramdomizer
                              CC encoder and puncture
                              Interleaver
                              Repetition
                              Modulation */
                err_code = phy_dl_fec_encoding(para,
                                               first_slot,
                                               num_of_slots,
                                               fec_r,
                                               fec_i,
                                               &fec_len);

                if (err_code)
                {
                    FLOG_ERROR("phy_dl_tx_cdd: Error in fec encoding!\n");
                    return err_code;
                }
                fec_r += fec_len;
                fec_i += fec_len;
                fec_len = 0;
            }
            else
            {
                fec_r += para->num_subcar_of_subch * num_of_slots;
                fec_i += para->num_subcar_of_subch * num_of_slots;
                fec_len = 0;
            }

            first_slot = current_slot = next_slot;
            next_slot = current_slot->next;
            num_of_slots = 1;
			DO_DUMP(DUMP_PHY_TX_FEC_ANT1_IN_ID, 0, current_slot, 1);
        } /* End of for (subchannel_count=4...) */
    } /* End of if (para->symbol_offset == 1) */
    else /* Not the first slot symbol */
    {
        /** FCH, DL_MAP and DL burst generation */
        fec_r = a_r;
        fec_i = a_i;
        fec_len = 0;

        first_slot = current_slot = p_slotsymbol->slot_header;
        next_slot  = current_slot->next;
        num_of_slots = 1;
		DO_DUMP(DUMP_PHY_TX_FEC_ANT1_IN_ID, 0, current_slot, 1);
    		PROF_START_TIMER( PROF_TIMER_TOKEN( PHY_DL_FEC_ENCODE ));
        for (subchannel_count=0;
             subchannel_count<para->num_subch-1; subchannel_count++)
        {
             /* Compose the processing block */
            if (current_slot->block_id == next_slot->block_id)
            {
                current_slot = next_slot;
                next_slot = current_slot->next;
                num_of_slots ++;
				DO_DUMP(DUMP_PHY_TX_FEC_ANT1_IN_ID, 0, current_slot, 1);
                continue;
            }

            if (current_slot->unused_flag == 0)
            {

            /* FEC encoding - Ramdomizer
                              CC encoder and puncture
                              Interleaver
                              Repetition
                              Modulation */
                err_code = phy_dl_fec_encoding(para,
                                               first_slot,
                                               num_of_slots,
                                               fec_r,
                                               fec_i,
                                               &fec_len);
                if (err_code)
                {
                    FLOG_ERROR("phy_dl_tx_cdd: Error in fec encoding!\n");
                    return err_code;
                }
                fec_r += fec_len;
                fec_i += fec_len;
                fec_len = 0;
            }
            else
            {
                 fec_r += para->num_subcar_of_subch * num_of_slots;
                 fec_i += para->num_subcar_of_subch * num_of_slots;
                 fec_len = 0;
            }


            first_slot = current_slot = next_slot;
            next_slot = current_slot->next;
             num_of_slots = 1;
			 DO_DUMP(DUMP_PHY_TX_FEC_ANT1_IN_ID, 0, current_slot, 1);
        } /* End of for (subchannel_count=0...) */
    		PROF_STOP_TIMER( PROF_TIMER_TOKEN( PHY_DL_FEC_ENCODE ));
    } /* End of if (para->symbol_offset==1) else */

    /* FEC encoding of last processing block */

   	PROF_START_TIMER( PROF_TIMER_TOKEN( PHY_DL_FEC_ENCODE ));
    if (current_slot->unused_flag == 0)
    {
        err_code = phy_dl_fec_encoding(para,
                                       first_slot,
                                       num_of_slots,
                                       fec_r,
                                       fec_i,
                                       &fec_len);

        if (err_code)
        {
            FLOG_ERROR("phy_dl_tx_cdd: Error in fec encoding!\n");
            return err_code;
        }
    }
    else
    {
        fec_r += para->num_subcar_of_subch * num_of_slots;
        fec_i += para->num_subcar_of_subch * num_of_slots;

    }

    fec_r = a_r;
    fec_i = a_i;

	DO_DUMP(DUMP_PHY_TX5_FEC_ANT1_I_ID, 0, fec_r, (para->ofdma_nused_no_dc - para->num_pilot)*para->symbol_per_slot);

	DO_DUMP(DUMP_PHY_TX5_FEC_ANT1_Q_ID, 0, fec_i, (para->ofdma_nused_no_dc - para->num_pilot)*para->symbol_per_slot);
   	PROF_STOP_TIMER( PROF_TIMER_TOKEN( PHY_DL_FEC_ENCODE ));
#if 0

    FILE *fp1, *fp2;
    fp1 = fopen("tx5_fec_ant1_i.dat","a+t");
    fp2 = fopen("tx5_fec_ant1_q.dat","a+t");
    dl_dump_ffloat(fp1, fec_r, (para->ofdma_nused_no_dc - para->num_pilot)*para->symbol_per_slot);
    dl_dump_ffloat(fp2, fec_i, (para->ofdma_nused_no_dc - para->num_pilot)*para->symbol_per_slot);
    fclose(fp1);
    fclose(fp2);

#endif
	
                                                        
     /* DL Zone Permutation */
    PROF_START_TIMER( PROF_TIMER_TOKEN( PHY_DL_ZONEPERM ));
#ifndef _MULTIBAND_
    err_code = phy_dl_zonepermutation(para,
                                     para->active_band,
                                     pilot_allocation,
                                     data_allocation);
#else
    err_code = phy_dl_zonepermutation_multi(para,
                                     para->active_band,
                                     pilot_allocation,
                                     data_allocation);
#endif
    PROF_STOP_TIMER( PROF_TIMER_TOKEN( PHY_DL_ZONEPERM ));
    if (err_code)
    {
        FLOG_ERROR("E009_tx: Error in dl zonepermutation!\n");
        return err_code;
    }

	DO_DUMP(DUMP_PHY_TX_PILOT_ANT1_ID, 0, pilot_allocation, (para->num_subch-para->dl_unused_subch)*6);

	DO_DUMP(DUMP_PHY_TX_DATA_ALLOC_ANT1_ID, 0, data_allocation, 48*(para->num_subch-para->dl_unused_subch));
#if 0
    FILE *fp3, *fp4;
    fp3 = fopen("./data_dst/2_pilot_allocation.out","w+t");
    fp4 = fopen("./data_dst/2_data_allocation.out","a+t");
    dl_dump_uinteger(fp3, pilot_allocation, (para->num_subch-para->dl_unused_subch)*6);
    dl_dump_uinteger(fp4, data_allocation, 48*(para->num_subch-para->dl_unused_subch));  		
    fclose(fp3);
    fclose(fp4);
#endif

    /* OFDM symbol multiplex (map constellation) */
    
    PROF_START_TIMER( PROF_TIMER_TOKEN( PHY_DL_OTHER ));

    muxofdm_r = b_r;
    muxofdm_i = b_i;

    err_code = phy_dl_muxofdmsym(para,
                                 para->dl_unused_subch,
                                 fec_r,
                                 fec_i,
                                 data_allocation,
                                 pilot_allocation,
                                 muxofdm_r,
                                 muxofdm_i);

    if (err_code)
    {
        FLOG_ERROR("E010_tx: Error in muxofdmsym!\n");
        return err_code;
    }



	DO_DUMP(DUMP_PHY_TX4_MULTIPLEX_ANT1_I_ID, 0, muxofdm_r, para->ofdma_nused_no_dc*para->symbol_per_slot);
	DO_DUMP(DUMP_PHY_TX4_MULTIPLEX_ANT1_Q_ID, 0, muxofdm_i, para->ofdma_nused_no_dc*para->symbol_per_slot);



    /* Subcarrier randomize */

    subcarrandom_r = a_r;
    subcarrandom_i = a_i;

    err_code = phy_dl_subcarrandom(para,
                                   para->wk,
                                   muxofdm_r,
                                   muxofdm_i,
                                   subcarrandom_r,
                                   subcarrandom_i);
#ifdef _ZONE_BOOST_
    nused_subch = para->num_subch - para->dl_unused_subch;
    zone_boost = sqrt( (float)para->num_subch/(float)nused_subch );
    for (i=0; i< (para->ofdma_nused-1)* para->symbol_per_slot;i++)
    {
        subcarrandom_r[i] = subcarrandom_r[i]*zone_boost;
        subcarrandom_i[i] = subcarrandom_i[i]*zone_boost;
    }
#endif

    if (err_code)
    {
        FLOG_ERROR("E008_tx: Error in subcarrandom!\n");
        return err_code;
    }


	DO_DUMP(DUMP_PHY_TX2_SUBCARAND_ANT1_I_ID, 0, subcarrandom_r, para->ofdma_nused_no_dc*para->symbol_per_slot);
	DO_DUMP(DUMP_PHY_TX2_SUBCARAND_ANT1_Q_ID, 0, subcarrandom_i, para->ofdma_nused_no_dc*para->symbol_per_slot);


    /* OFDMA modulation */
    err_code = phy_dl_ofdmamodul(para,
                                 subcarrandom_r,
                                 subcarrandom_i,
                                 dl_burst_r_ant1,
                                 dl_burst_i_ant1);
#ifdef _ZONE_BOOST_
    nused_subch = para->num_subch - para->dl_unused_subch;
    zone_boost = sqrt( (float)para->ofdma_nfft );
    for (i=0; i< para->ofdma_symlen_with_guard * para->symbol_per_slot;i++)
    {
        dl_burst_r_ant1[i] = dl_burst_r_ant1[i]*zone_boost;
        dl_burst_i_ant1[i] = dl_burst_i_ant1[i]*zone_boost;
    }
#endif

    if (err_code) 
    {
        FLOG_ERROR("phy_dl_tx_cdd: Error in ofdma modulation for antenna0!\n");
        return err_code;
    }


	DO_DUMP(DUMP_PHY_TX1_OFDMAMOD_ANT1_I_ID, 0, dl_burst_r_ant1, para->ofdma_symlen_with_guard*para->symbol_per_slot);
	DO_DUMP(DUMP_PHY_TX1_OFDMAMOD_ANT1_Q_ID, 0, dl_burst_i_ant1, para->ofdma_symlen_with_guard*para->symbol_per_slot);


    /* OFDMA modulation--CDD */
    err_code = phy_dl_ofdmamodul_cdd(para,
                                     subcarrandom_r,
                                     subcarrandom_i,
                                     dl_burst_r_ant2,
                                     dl_burst_i_ant2);
#ifdef _ZONE_BOOST_
    nused_subch = para->num_subch - para->dl_unused_subch;
    zone_boost = sqrt( (float)para->ofdma_nfft );
    for (i=0; i< para->ofdma_symlen_with_guard * para->symbol_per_slot;i++)
    {
        dl_burst_r_ant2[i] = dl_burst_r_ant2[i]*zone_boost;
        dl_burst_i_ant2[i] = dl_burst_i_ant2[i]*zone_boost;
    }
#endif

    if (err_code) 
    {
        FLOG_ERROR("phy_dl_tx_cdd: Error in ofdma modulation for antenna 1!\n");
        return err_code;
    }


	DO_DUMP(DUMP_PHY_TX1_OFDMAMOD_ANT2_I_ID, 0, dl_burst_r_ant2, para->ofdma_symlen_with_guard*para->symbol_per_slot);
	DO_DUMP(DUMP_PHY_TX1_OFDMAMOD_ANT2_Q_ID, 0, dl_burst_i_ant2, para->ofdma_symlen_with_guard*para->symbol_per_slot);
    PROF_STOP_TIMER( PROF_TIMER_TOKEN( PHY_DL_OTHER ));

    return SUCCESS_CODE;
}



/**----------------------------------------------------------------------------
   Function:    phy_dl_tx_stca()

   Description: To genarate DL sub-frame for STC matrix A.

   Parameters:
                Input-  [struct phy_dl_tx_syspara *para]  The pointer refers
                        to the struct of system parameters.
                        [const phy_dl_slotsymbol *p_slotsymbol]  The
                        pointer refer to one input slot symbol.

                Output- [struct phy_dl_rru_symbol *p_dl_rru_symbol_ant1]  The 
                        pointer refer to the output rru symbol buffer of 
                        antenna 1.
                        [struct phy_dl_rru_symbol *p_dl_rru_symbol_ant2]  The 
                        pointer refer to the output rru symbol buffer of
                        antenna 2.

   Return Value:
                0       Success
                150     Error

   ----------------------------------------------------------------------------
   LOG END TAG zYx                                                            */
int phy_dl_tx_stca(struct phy_dl_tx_syspara *para,
                   const struct phy_dl_slotsymbol *p_slotsymbol,
                   struct phy_dl_rru_symbol *p_dl_rru_symbol_ant1,
                   struct phy_dl_rru_symbol *p_dl_rru_symbol_ant2)
{
    (void) para;
    (void) p_slotsymbol;
    (void) p_dl_rru_symbol_ant1;
    (void) p_dl_rru_symbol_ant2;

    FLOG_WARNING("unsupported MIMO mode-STC MatrixA in current stage!\n");    
    return SUCCESS_CODE;
}

/**----------------------------------------------------------------------------
   Function:    phy_dl_tx_stcb()

   Description: To genarate DL sub-frame for STC matrix B.

   Parameters:
                Input-  [struct phy_dl_tx_syspara *para]  The pointer refers
                        to the struct of system parameters.
                        [const phy_dl_slotsymbol *p_slotsymbol_1]  The
                        pointer refers to the first input slot symbol.
                        [const phy_dl_slotsymbol *p_slotsymbol_2]  The
                        pointer refers to the second input slot symbol.

                Output- [struct phy_dl_rru_symbol *p_dl_rru_symbol_ant1]  The 
                        pointer refers to the output rru symbol buffer of 
                        antenna 1.
                        [struct phy_dl_rru_symbol *p_dl_rru_symbol_ant2]  The 
                        pointer refers to the output rru symbol buffer of 
                        antenna 2.

   Return Value:
                0       Success
                150     Error

   ----------------------------------------------------------------------------
   LOG END TAG zYx                                                            */

int phy_dl_tx_stcb(struct phy_dl_tx_syspara *para,
                   const struct phy_dl_slotsymbol *p_slotsymbol_1,
                   const struct phy_dl_slotsymbol *p_slotsymbol_2,
                   struct phy_dl_rru_symbol *p_dl_rru_symbol_ant1,
                   struct phy_dl_rru_symbol *p_dl_rru_symbol_ant2)
{
    (void) para;
    (void) p_slotsymbol_1;
    (void) p_slotsymbol_2;
    (void) p_dl_rru_symbol_ant1;
    (void) p_dl_rru_symbol_ant2;

    FLOG_WARNING("unsupported MIMO mode--STC MatrixB at current stage!\n");    
    return SUCCESS_CODE;
}


/* restore _DUMP_UTIL_ENABLE_ setting */
#ifndef DUMP_PHY_DL_TX

#ifdef LOCAL_DUMP_ENABLE
#define _DUMP_UTIL_ENABLE_
#endif

#undef LOCAL_DUMP_ENABLE
#endif

