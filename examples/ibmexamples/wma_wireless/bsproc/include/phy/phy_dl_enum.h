/* ----------------------------------------------------------------------------
   IBM Confidential

   IBM Wireless M2M Platform

   (C)Copyright IBM Corp. 2009, 2010, 2011

   All Rights Reserved.
   

   File Name: phy_initial_sensing.h

   Function: Declare the functions to form DL frame in transmitter.

   Change Activity:

   Date             Description of Change                            By
   -----------      ---------------------                            --------


   ----------------------------------------------------------------------------
   PROLOG END TAG zYx                                                         */

#ifndef _PHY_ENUM_H_
#define _PHY_ENUM_H_

/** Enum */

enum
{
    CDD  = 0,
    STCA = 1,
    STCB = 2
}enum_mimo_mode;

enum
{
    CC_QPSK12    = 0,
    CC_QPSK34    = 1,
    CC_QAM16_12  = 2,
    CC_QAM16_34  = 3,
    CC_QAM64_12  = 4,
    CC_QAM64_23  = 5,
    CC_QAM64_34  = 6,

    CTC_QPSK12   = 7,
    CTC_QPSK34   = 8,
    CTC_QAM16_12 = 9,
    CTC_QAM16_34 = 10,
    CTC_QAM16_56 = 11,
    CTC_QAM64_12 = 12,
    CTC_QAM64_23 = 13,
    CTC_QAM64_34 = 14,
    CTC_QAM64_56 = 15,

    CODEIDRESERVED = 16
}enum_code_id;

enum
{
    PUSC = 0,
    FUSC = 1,
    OFUSC = 2,
    AMC = 3,
    OPUSC = 4
}enum_permutation_mode;


#endif

