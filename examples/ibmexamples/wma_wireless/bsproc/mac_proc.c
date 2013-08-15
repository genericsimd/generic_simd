/* ----------------------------------------------------------------------------
 * IBM Confidential

   IBM Wireless M2M Platform

   (C)Copyright IBM Corp. 2009, 2010, 2011

   All Rights Reserved.

   File Name: mac_proc.c

   Change Activity:

   Date             Description of Change                            By
   -----------      ---------------------                            --------
   12-May 2011       Created                                         Zhu, Zhenbo

   ----------------------------------------------------------------------------
   PROLOG END TAG zYx                                                         */

#include <unistd.h>

#include "mac_proc.h"
#include "global.h"
#include "flog.h"

#include "mac.h"
#include "sdu_cid_queue.h"

/** BS adapter */
#include "adapter_test_stube.h"

ul_map_msg *ul_map_stube = NULL;

dcd_msg *dcdmsg = NULL;

static int init_bs_adapter ();

static int free_bs_adapter ();

int resync_mac(void)
{
    return (set_current_frame_number(0));
}

int mac_bs_process (void)
{
    int ret = 0;

    ret = init_bs_adapter();

    if (ret != 0)
    {
        FLOG_FATAL("initial MAC variables error");

        return RET_ERROR;
    }

    ret = init_mac_simulation_variables(1, NULL);

    if (ret != 0)
    {
        FLOG_FATAL("initial MAC variables error");

        return RET_ERROR;
    }

    ret = init_mac_core_variables();

    if (ret != 0)
    {
        FLOG_FATAL("initial MAC variables error");

        return RET_ERROR;
    }

    ret = start_mac_threads();

    if (ret != 0)
    {
        FLOG_FATAL("initial MAC variables error");

        return RET_ERROR;
    }

    return 0;
}

int mac_bs_release (void)
{
    int ret = 0;

//    sleep(1);

//    ret = free_mac_core_variables();

    if (ret != 0)
    {
        FLOG_FATAL("release MAC core variables error");

        return RET_ERROR;
    }

//    sleep(1);
#ifdef ARQ_ENABLED
   ARQ_shutdown();
#endif
    ret = free_mac_simulation_variables();

    if (ret != 0)
    {
        FLOG_FATAL("release MAC simulation variables error");

        return RET_ERROR;
    }

//    sleep(1);

    ret = free_bs_adapter();

    if (ret != 0)
    {
        FLOG_FATAL("release adapter error");

        return RET_ERROR;
    }

    FLOG_INFO("MAC layer released");

    return 0;
}


/** BS adapter */

static int init_bs_adapter ()
{
/*
    ul_map_msg *ul_map_stube = (ul_map_msg*) malloc (sizeof(ul_map_msg));
    adapter_build_ul_map (ul_map_stube);

    set_ul_map_msg_stube (ul_map_stube);
*/
    dcd_msg *dcdmsg = (dcd_msg*) malloc (sizeof(dcd_msg));
    setdcdmsgdata (dcdmsg);
    set_dcd_msg_stube (dcdmsg);

    return 0;
}

static int free_bs_adapter ()
{
    if (ul_map_stube != NULL)
    {
        free(ul_map_stube);
    }

    if (dcdmsg != NULL)
    {
        free(dcdmsg);
    }

    return 0;
}
