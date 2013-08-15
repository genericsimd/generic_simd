/* ----------------------------------------------------------------------------
 * IBM Confidential

 IBM Wireless M2M Platform

 (C)Copyright IBM Corp. 2009, 2010, 2011

 All Rights Reserved.

 File Name: mac_thread.c

 Change Activity:

 Date             Description of Change                            By
 -----------      ---------------------                            --------
 23-JUL 2011       Created                                         jian jun chang 

 ----------------------------------------------------------------------------
 PROLOG END TAG zYx                                                         */
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <pthread.h>

#include "mac_thread.h"
#include "stube_mac.h"
#include "flog.h"
#include "adapter_bs_dl_transformer_phy.h"
#include "queue_util.h"
#include "mac_frame.h"
#include "dlmap.h"
#include "adapter_bs_ul_transform_mac.h"
#include "mac_subframe_queue.h"
#include "initial_sensing_proc.h"

#include "sem_util.h"

#include "phy_proc.h"
#include "rru_proc.h"
#include "prephy_proc.h"

pthread_t enq_thread= 0;
pthread_t deq_thread= 0;
extern int spectrum_measure_flag;

void * process_enq_frame(void * arg)
{
    struct ofdma_map ofdma_frame;
    struct phy_dl_slotsymbol *p_phy = NULL;
    struct queue_msg queue_obj;
    int iresult = 0;
    unsigned int frame_num = 0;;
    int islotnum = 0;
    physical_subframe *mac_frame;
    dl_map_msg  *p_dlmap;

    generate_physubframe(&mac_frame);
    generate_dlmap(&p_dlmap);
    queue_obj.my_type = mac_en_id[0];
    int sensing_idx = 0;
/*
    if (wait_sem (tx_s_handle) != 0 )
    {
        FLOG_ERROR ("get SEM error\n");
    }
*/
    int  ret = get_global_param ("PERIODIC_SENSING_ENABLE", & ( g_periodic_sensing_enable ));

    if (ret != 0)
    {
        FLOG_WARNING ("get PERIODIC_SENSING_ENABLE error\n");
    }


    while(1)
    {

        if (wait_sem (tx_s_handle) != 0 )
        {
            FLOG_ERROR ("get SEM error\n");
        }
        if (g_periodic_sensing_enable == 0)
        {
            sensing_idx = 0;
        }else
        {
            sensing_idx = frame_num ;
        }

        if( ( (sensing_idx != 0) && ( (sensing_idx % (g_spectrum.periodic_duration * 50)) == 0 )) ||
            (g_periodic_sensing_reset == 1) )
        {
            spectrum_measure_flag = 1;

            if (g_periodic_sensing_reset == 1)
            {
                g_periodic_sensing_drop = 1;
                g_periodic_sensing_reset = 0;
            }
        }

        generate_physubframe(&mac_frame);
        if(spectrum_measure_flag == 1)
        {
             mac_frame->sense_flag = 1;
             spectrum_measure_flag = 0;
        }
        p_dlmap->frame_number = frame_num;

        iresult = phy_subframe_send_transform_dl (mac_frame, &ofdma_frame,p_dlmap);

        if (iresult == -1)
        {
            FLOG_ERROR ("Transform error\n");
        }

        for (islotnum = 0; islotnum < ofdma_frame.slot_num; islotnum++)
        {
            adapter_transform_symbolslot_1 (&ofdma_frame, islotnum, frame_num, &p_phy);
            if (islotnum + 1 == ofdma_frame.slot_num)
                 p_phy->dl_subframe_end_flag = 1;
            queue_obj.p_buf = p_phy;
            p_phy = NULL;

            if ( wmrt_enqueue( mac_en_id[0], &queue_obj,sizeof(struct queue_msg)) == -1)
            {
                FLOG_ERROR("enqueue error\n");
            }
        }

        frame_num++;
        release_subframe(mac_frame);
        
        //free(mac_frame);
        //printf("frame num %d\n",frame_num);

    }
}

void * process_deq_frame(void *arg)
{
    physical_subframe *phy_subframe;
    struct queue_msg *p_msg = (struct queue_msg *)malloc(sizeof(struct queue_msg));
    int ranging_count = 0;
    int ranging_idx;
    void * pr_result = NULL;

    (void) arg;

    while (1){
        phy_subframe = NULL;


        p_msg->my_type= mac_ul_de_id[0];

        if (wmrt_dequeue (mac_ul_de_id[0], p_msg, sizeof(struct queue_msg)) == -1)
        {
            FLOG_WARNING ("DEQUEUE ERROR in MAC layer\n");
        }

        //adapter_get_phy_subframe(p_msg->p_buf,&phy_subframe);
        adapter_transform_physical_frame(p_msg->p_buf, &phy_subframe, &pr_result);

        free_adapter_frame((void **)&(p_msg->p_buf));

        pthread_mutex_lock(&mutex_tx_phy_en_flag);
        ranging_count = ranging_en_flag;
        ranging_en_flag = 0;
        pthread_mutex_unlock(&mutex_tx_phy_en_flag);

        if (ranging_count != 0)
        {
//            FLOG_INFO("%d ranging result found!\n", ranging_count);

            p_msg->my_type= mac_ul_de_id[2];

            for (ranging_idx = 0; ranging_idx < ranging_count; ranging_idx ++)
            {
                if (wmrt_dequeue (mac_ul_de_id[2], p_msg, sizeof(struct queue_msg)) == -1)
                {
                    FLOG_WARNING ("DEQUEUE ERROR in MAC layer\n");
                }

//                phy_dump_ranging_result(p_msg->p_buf);

                free(p_msg->p_buf);
            }
        }

        if (phy_subframe->interference_info != NULL)
        {
//            FLOG_INFO("got dts_info from PHY %p\n", phy_subframe->interference_info);
            free(phy_subframe->interference_info);
        }
        release_subframe(phy_subframe);
/** for testing only */

        phy_subframe = NULL;
/** for testing currently */
    }
}

int start_stube_thread()
{
    pthread_attr_t tattr;
    //    cpu_set_t cpuset;

    pthread_attr_init (&tattr);
    /*
     __CPU_ZERO_S(sizeof (cpu_set_t), &cpuset);
     __CPU_SET_S(0, sizeof (cpu_set_t), &cpuset);
     pthread_attr_setaffinity_np(&tattr, sizeof(cpuset), &cpuset);
     */
    pthread_create (&enq_thread, NULL, process_enq_frame, NULL);
    pthread_create (&deq_thread, NULL, process_deq_frame, NULL);
    pthread_attr_destroy (&tattr);
 
    return 0;   
}

int stop_stube_thread()
{
    if (enq_thread != 0)
    {
        pthread_cancel (enq_thread);
        pthread_join (enq_thread, NULL);
    }
    if(deq_thread != 0)
    {
        pthread_cancel (deq_thread);
        pthread_join (deq_thread, NULL);
    }

    return 0;
}
