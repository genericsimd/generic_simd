/* ----------------------------------------------------------------------------
 * IBM Confidential

 IBM Wireless M2M Platform

 (C)Copyright IBM Corp. 2009, 2010, 2011

 All Rights Reserved.

 File Name: bs_cfg.c

 Change Activity:

 Date             Description of Change                            By
 -----------      ---------------------                            --------
 13-May 2011       Created                                         Zhu, Zhenbo

 ----------------------------------------------------------------------------
 PROLOG END TAG zYx                                                         */

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <sys/types.h>

#include <pthread.h>

#include "global.h"
#include "bs_cfg.h"
#include "flog.h"

#include "prephy_proc.h"
#include "initial_sensing_proc.h"
#include "phy_proc.h"

struct param_unit * gp_global_param = NULL;
struct config_unit * gp_config_st = NULL;

//pthread_mutex_t g_param_mutex;

int32_t init_global_param (char * file_name)
{
    FILE * config_fp = NULL;

    char * key_name = (char *) malloc (sizeof(char) * MAX_KEY_LEN);
    int i;
    int ret = 1;
    int ret_val = 0;
    int idx_count = 0;

//    pthread_mutex_init (&g_param_mutex, NULL);

//    pthread_mutex_lock (&g_param_mutex);

    gp_global_param = (struct param_unit *) malloc (MAX_PARAM_NUM
            * MAX_COMPONENT_NUM * sizeof(struct param_unit));

    gp_config_st = (struct config_unit *) malloc (sizeof(struct config_unit));

    /* Initial the key name and component name first */

#define CONVERT_KEY(NAME, COMPONENT, KEYIDX, TYPE, MIN, MAX, FUN)    \
        strcpy ( gp_global_param[idx_count].key_name, #NAME );    \
        strcpy ( gp_global_param[idx_count].component_name, #COMPONENT );\
        gp_global_param[idx_count].type = TYPE;    \
        gp_global_param[idx_count].component_id = COMPONENT##_PARAM_ID;    \
        gp_global_param[idx_count].index = KEYIDX;    \
        gp_global_param[idx_count].active = 0;    \
        gp_global_param[idx_count].f_cb = FUN;    \
        pthread_mutex_init (&(gp_global_param[idx_count].param_mutex), NULL);    \
        idx_count++;
    PARAM_KEYS;
#undef CONVERT_KEY

    gp_config_st->total_param_num = idx_count;

    config_fp = fopen ((char *) file_name, "r");

    if (config_fp == NULL)
    {
        FLOG_FATAL ("open configuration file %s failed\n", file_name);
        return RET_ERROR;
    }

    while (ret == 1)
    {
        if (fscanf (config_fp, "%s", key_name) == EOF)
        {
            ret = 0;
            break;
        }

        for (i = 0; i < gp_config_st->total_param_num; i++)
        {
            if (strcmp (gp_global_param[i].key_name, key_name) == 0)
            {

                switch (gp_global_param[i].type)
                {
                    case VAL_INTEGER:
                        pthread_mutex_lock ( &(gp_global_param[i].param_mutex) );
                        ret_val = fscanf (config_fp, "%d",
                                (int *) gp_global_param[i].value);

                        gp_global_param[i].active = 1;
                        pthread_mutex_unlock ( &(gp_global_param[i].param_mutex) );

                        FLOG_INFO ("read param %s\t value %d",
                                gp_global_param[i].key_name,
                                * ( (int *) ( gp_global_param[i].value ) ));

                        break;

                    case VAL_STRING:
                        pthread_mutex_lock ( &(gp_global_param[i].param_mutex) );
                        ret_val = fscanf (config_fp, "%s",
                                gp_global_param[i].value);

                        gp_global_param[i].active = 1;
                        pthread_mutex_unlock ( &(gp_global_param[i].param_mutex) );
                        FLOG_INFO ("read param %s\t value %s",
                                gp_global_param[i].key_name,
                                gp_global_param[i].value);

                        break;

                    case VAL_FLOAT:
                        pthread_mutex_lock ( &(gp_global_param[i].param_mutex) );
                        ret_val = fscanf (config_fp, "%f",
                                (float *) gp_global_param[i].value);

                        gp_global_param[i].active = 1;
                        pthread_mutex_unlock ( &(gp_global_param[i].param_mutex) );

                        FLOG_INFO ("read param %s\t value %f",
                                gp_global_param[i].key_name,
                                * ( (float *) ( gp_global_param[i].value ) ));

                        break;

                    default:
                        FLOG_WARNING ("error configure file line\n");
                        break;
                }

                if (ret_val == EOF)
                {
                    ret = 0;
                    break;
                }
            }
        }
    }

//    pthread_mutex_unlock (&g_param_mutex);

    fclose (config_fp);
    free (key_name);

    return 0;
}

int32_t deinit_global_param (void)
{
    int i = 0;
/*
    if (pthread_mutex_destroy (&g_param_mutex) != 0)
    {
        FLOG_ERROR ("destory lock error\n");
    }
*/

    if (gp_global_param != NULL)
    {
        for (i = 0; i < gp_config_st->total_param_num; i++)
        {
            pthread_mutex_destroy( &(gp_global_param[i].param_mutex) );
        }

        free (gp_global_param);
        gp_global_param = NULL;
    }

    if (gp_config_st != NULL)
    {
        free (gp_config_st);
        gp_config_st = NULL;
    }

    return 0;

}

int32_t get_global_param (char * key_name, void * value)
{
    int i;
    int ret_val = 0;

    if ( ( gp_global_param == NULL ) || ( gp_config_st == NULL ))
    {
        FLOG_ERROR ("global parameters haven't been initialized yet\n");
        return RET_ERROR;
    }

    if ((key_name == NULL) || (value == NULL))
    {
        FLOG_ERROR ("key name or value NULL pointer\n");
        return RET_ERROR;
    }

//    pthread_mutex_lock (&g_param_mutex);

    for (i = 0; i < gp_config_st->total_param_num; i++)
    {
        if (strcmp (gp_global_param[i].key_name, key_name) == 0)
        {
            switch (gp_global_param[i].type)
            {
                case VAL_INTEGER:
//                    pthread_mutex_lock ( &(gp_global_param[i].param_mutex) );
                    if (gp_global_param[i].active == 1)
                    {
                        * ( (int *) value )
                                = * ( (int *) ( gp_global_param[i].value ) );

                        ret_val = 1;
                    }
                    else
                    {
                        FLOG_WARNING ("Parameters %s is not initialized\n",
                                key_name);
                    }
//                    pthread_mutex_unlock ( &(gp_global_param[i].param_mutex) );

                    break;

                case VAL_STRING:
                    pthread_mutex_lock ( &(gp_global_param[i].param_mutex) );
                    if (gp_global_param[i].active == 1)
                    {
                        strcpy ((char *) value, gp_global_param[i].value);

                        ret_val = 1;
                    }
                    else
                    {
                        FLOG_WARNING ("Parameters %s is not initialized\n",
                                key_name);
                    }
                    pthread_mutex_unlock ( &(gp_global_param[i].param_mutex) );
                    break;

                case VAL_FLOAT:
//                    pthread_mutex_lock ( &(gp_global_param[i].param_mutex) );
                    if (gp_global_param[i].active == 1)
                    {
                        * ( (float *) value )
                                = * ( (float *) ( gp_global_param[i].value ) );
                        ret_val = 1;
                    }
                    else
                    {
                        FLOG_WARNING ("Parameters %s is not initialized\n",
                                key_name);
                    }
//                    pthread_mutex_unlock ( &(gp_global_param[i].param_mutex) );
                    break;

                default:
                    FLOG_WARNING ("error configure file line\n");
                    break;
            }
            if (ret_val == 1)
            {
//                pthread_mutex_unlock (&g_param_mutex);
                return 0;
            }
        }
    }

//    pthread_mutex_unlock (&g_param_mutex);

    return RET_NOTMATCH;
}

int32_t set_global_param (char * key_name, void * value)
{
    int i;
    int ret_val = 0;

    if ( ( gp_global_param == NULL ) || ( gp_config_st == NULL ))
    {
        FLOG_ERROR ("global parameters haven't been initialized yet\n");
        return RET_ERROR;
    }

    if ((key_name == NULL) || (value == NULL))
    {
        FLOG_ERROR ("key name or value NULL pointer\n");
        return RET_ERROR;
    }

//    pthread_mutex_lock (&g_param_mutex);

    for (i = 0; i < gp_config_st->total_param_num; i++)
    {
        if (strcmp (gp_global_param[i].key_name, key_name) == 0)
        {
            switch (gp_global_param[i].type)
            {
                case VAL_INTEGER:
//                    pthread_mutex_lock ( &(gp_global_param[i].param_mutex) );
                    * ( (int *) ( gp_global_param[i].value ) )
                            = * ( (int *) value );

                    gp_global_param[i].active = 1;
//                    pthread_mutex_unlock ( &(gp_global_param[i].param_mutex) );

                    ret_val = 1;

                    break;

                case VAL_STRING:
                    pthread_mutex_lock ( &(gp_global_param[i].param_mutex) );
                    strcpy (gp_global_param[i].value, (char *) value);
                    gp_global_param[i].active = 1;
                    pthread_mutex_unlock ( &(gp_global_param[i].param_mutex) );

                    ret_val = 1;

                    break;

                case VAL_FLOAT:
//                    pthread_mutex_lock ( &(gp_global_param[i].param_mutex) );
                    * ( (float *) ( gp_global_param[i].value ) )
                            = * ( (float *) value );

                    gp_global_param[i].active = 1;
//                    pthread_mutex_unlock ( &(gp_global_param[i].param_mutex) );

                    ret_val = 1;

                    break;

                default:
                    FLOG_WARNING ("error configure file line\n");
                    break;
            }
        }

        if (ret_val == 1)
        {
//            pthread_mutex_unlock (&g_param_mutex);
            return 0;
        }
    }

//    pthread_mutex_unlock (&g_param_mutex);

    FLOG_WARNING ("Key not exist in system line\n");

    return RET_NOTMATCH;
}

int32_t update_global_param (char * key_name, void * value)
{
    int i;
    int ret_val = 0;

    if ( ( gp_global_param == NULL ) || ( gp_config_st == NULL ))
    {
        FLOG_ERROR ("global parameters haven't been initialized yet\n");
        return RET_ERROR;
    }

    if ((key_name == NULL) || (value == NULL))
    {
        FLOG_ERROR ("key name or value NULL pointer\n");
        return RET_ERROR;
    }

//    pthread_mutex_lock (&g_param_mutex);

    for (i = 0; i < gp_config_st->total_param_num; i++)
    {
        if (strcmp (gp_global_param[i].key_name, key_name) == 0)
        {
            switch (gp_global_param[i].type)
            {
                case VAL_INTEGER:
//                    pthread_mutex_lock ( &(gp_global_param[i].param_mutex) );
                    * ( (int *) ( gp_global_param[i].value ) )
                            = * ( (int *) value );

                    gp_global_param[i].active = 1;
//                    pthread_mutex_unlock ( &(gp_global_param[i].param_mutex) );

                    if (gp_global_param[i].f_cb != NULL)
                    {
                        (*(gp_global_param[i].f_cb))(VAL_INTEGER, 4, value);
                    }

                    ret_val = 1;

                    break;

                case VAL_STRING:
                    pthread_mutex_lock ( &(gp_global_param[i].param_mutex) );
                    strcpy (gp_global_param[i].value, (char *) value);

                    gp_global_param[i].active = 1;
                    pthread_mutex_unlock ( &(gp_global_param[i].param_mutex) );

                    if (gp_global_param[i].f_cb != NULL)
                    {
                        (*(gp_global_param[i].f_cb))(VAL_STRING, strlen((char *)value), value);
                    }

                    ret_val = 1;

                    break;

                case VAL_FLOAT:
//                    pthread_mutex_lock ( &(gp_global_param[i].param_mutex) );
                    * ( (float *) ( gp_global_param[i].value ) )
                            = * ( (float *) value );

                    gp_global_param[i].active = 1;
//                    pthread_mutex_unlock ( &(gp_global_param[i].param_mutex) );

                    if (gp_global_param[i].f_cb != NULL)
                    {
                        (*(gp_global_param[i].f_cb))(VAL_FLOAT, 4, value);
                    }

                    ret_val = 1;

                    break;

                default:
                    FLOG_WARNING ("error configure file line\n");
                    break;
            }
        }

        if (ret_val == 1)
        {
//            pthread_mutex_unlock (&g_param_mutex);
            return 0;
        }
    }

//    pthread_mutex_unlock (&g_param_mutex);

    FLOG_WARNING ("Key not exist in system line\n");

    return RET_NOTMATCH;
}

