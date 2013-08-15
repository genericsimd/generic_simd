/* ----------------------------------------------------------------------------
 IBM Confidential

 IBM Wireless M2M Platform

 (C)Copyright IBM Corp. 2009, 2010, 2011

 All Rights Reserved.

 File Name: gpp_utils.h

 Change Activity:

 Date             Description of Change                            By
 -----------      ---------------------                            --------
 16-Apr.2011      Created                                          Zhu Zhenbo

 ----------------------------------------------------------------------------
 PROLOG END TAG zYx                                                         */

#ifndef __GPP_UTILS_H_
#define __GPP_UTILS_H_

#include <semaphore.h>

/*
 * Semaphore handle
 */

struct sem_handle
{
    sem_t sem;
};


/* ----------------------------------------------------------------------------
 Function:    create_sem()

 Description: Initialize a semaphore.

 Parameters:
     struct sem_handle ** sem_handle [Output] Struct which keeps the semaphore.

 Return Value:
     0       Success
     Other   Error

 ---------------------------------------------------------------------------
 PROLOG END TAG zYx                                                         */

int32_t create_sem(struct sem_handle ** sem_handle);

/* ----------------------------------------------------------------------------
 Function:    delete_sem()

 Description: delete a semaphore.

 Parameters:
     struct sem_handle ** sem_handle [IN] Struct which keeps the semaphore.

 Return Value:
     0       Success
     Other   Error

 ---------------------------------------------------------------------------
 PROLOG END TAG zYx                                                         */

int32_t delete_sem(struct sem_handle * sem_handle);

/* ----------------------------------------------------------------------------
 Function:    wait_sem()

 Description: blocking wait for a semaphore.

 Parameters:
     sem_t sem_handle [IN] The semaphore.

 Return Value:
     0       Success
     Other   Error

 ---------------------------------------------------------------------------
 PROLOG END TAG zYx                                                         */

int32_t wait_sem (struct sem_handle * sem_handle);

/* ----------------------------------------------------------------------------
 Function:    post_sem()

 Description: post to the semaphore.

 Parameters:
     sem_t sem_handle [IN] The semaphore handle.

 Return Value:
     0       Success
     Other   Error

 ---------------------------------------------------------------------------
 PROLOG END TAG zYx                                                         */

int32_t post_sem (struct sem_handle * sem_handle);


#endif /* __GPP_UTILS_H_ */

