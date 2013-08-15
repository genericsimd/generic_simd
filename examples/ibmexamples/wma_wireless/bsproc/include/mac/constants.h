/* ----------------------------------------------------------------------------
   IBM Confidential

   IBM Wireless M2M Platform   

   (C)Copyright IBM Corp. 2010, 2011

   All Rights Reserved.

   File Name: constants.h

   Change Activity:

   Date             Description of Change                   By
   -----------      ---------------------					--------
   1-Oct.2010       Created                                 Parul Gupta

   ----------------------------------------------------------------------------
   PROLOG END TAG zYx                                                         */

#ifndef _CONSTANTS_H_
#define _CONSTANTS_H_

// define system constants: Ref Table 525 (Rev2D2), Table 548 (Rev2D5)
#define CONTENTION_RANGING_RETRIES 16 //At the SS
#define NUM_DSX_REQ_RETRIES 3
#define NUM_DSX_RSP_RETRIES 3

#define T3_RNG 600000 // In usec
#define T3_RNG_HO_NEGOTIATED_BS 50000 // In usec
#define T3_RNG_HO_NON_NEGOTIATED_BS 200000 // In usec
#define T3_LOC_UPDATE 200000 // In usec
#define T4_RNG 1000000

#define T7_DURATION (1000000 * 4) // in usec (=1s)
#define T8_DURATION (300000 * 4) // in usec (=300ms)
#define T10_DURATION (1000000 * 4) // in usec (=1sec), maximum 3sec
#define T14_DURATION (200000 * 4) // in usec (=200msec)


#endif

