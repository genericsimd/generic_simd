/* ----------------------------------------------------------------------------
 IBM Confidential

 IBM Wireless M2M Platform

 (C)Copyright IBM Corp. 2009, 2010, 2011

 All Rights Reserved.

 File Name: gpp_utils.c

 Change Activity:

 Date             Description of Change                            By
 -----------      ---------------------                            --------
 16-Apr.2011      Created                                          Zhu Zhenbo

 ----------------------------------------------------------------------------
 PROLOG END TAG zYx                                                         */


/** OS */
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <semaphore.h>
#include <stdlib.h>

/** Application */
#include <sem_util.h>

/** Create semaphore */
int32_t create_sem(struct sem_handle ** sem_handle)
{
    int32_t ret = 0;

    *sem_handle = malloc( sizeof(struct sem_handle) );

    if (*sem_handle == NULL)
    {
        printf("semaphore malloc error!\n");
        return 1;
    }

    ret = sem_init(&( (*sem_handle)->sem), 0, 0);

    if (ret < 0)
    {
        printf("semaphore initial error!\n");
        free (*sem_handle);
        *sem_handle = NULL;
        return 1;
    }

    return 0;
}

/** Delete semaphore */

int32_t delete_sem(struct sem_handle *sem_handle)
{
    int32_t ret = 0;
    int32_t status = 0;

    if (sem_handle != NULL)
    {
        ret = sem_destroy (&(sem_handle->sem)) ;

        if (ret < 0) {
            printf("free semaphore error!\n");
            status = 1;
        }
        free (sem_handle);
    }

    return status;
}

/** Wait semaphore */
int32_t wait_sem(struct sem_handle *sem_handle)
{
    int32_t ret = 0;

    if (sem_handle != NULL)
    {
        ret = sem_wait(&(sem_handle->sem));

        if (ret < 0)
        {
            printf("Wait semaphore error!\n");
            return 1;
        }
    }

    return 0;
}

/** Post semaphore */
int32_t post_sem(struct sem_handle *sem_handle)
{
    int32_t ret = 0;

    if (sem_handle != NULL)
    {
        ret = sem_post(&(sem_handle->sem));

        if (ret < 0)
        {
            printf("Post semaphore error!\n");
            return 1;
        }
    }

    return 0;
}


