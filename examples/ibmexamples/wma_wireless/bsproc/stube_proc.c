/* ----------------------------------------------------------------------------
 * IBM Confidential

 IBM Wireless M2M Platform

 (C)Copyright IBM Corp. 2009, 2010, 2011

 All Rights Reserved.

 File Name: stube_proc.c

 Change Activity:

 Date             Description of Change                            By
 -----------      ---------------------                            --------
 25-JUL 2011       Created                                         jian jun chang 

 ----------------------------------------------------------------------------
 PROLOG END TAG zYx                                                         */
#include "stube_proc.h"

#include "mac_thread.h"

//changed by changjj
int stube_thread_start()
{ 
    int ret = 0;

    ret = start_stube_thread();

    return ret;
}

int stube_thread_release()
{
    int ret = 0;

    ret = stop_stube_thread();

    return ret;
}
