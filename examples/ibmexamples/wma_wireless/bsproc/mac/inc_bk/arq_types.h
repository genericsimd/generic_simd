/* ----------------------------------------------------------------------------
   IBM Confidential

   IBM Wireless M2M Platform   

   (C)Copyright IBM Corp. 2008, 2009, 2010, 2011

   All Rights Reserved.

   File Name: arq_types.h

   Change Activity:

   Date             Description of Change                            By
   -----------      ---------------------                            --------
   1-Oct.2008       Created                                          Uma Devi

   ----------------------------------------------------------------------------
   PROLOG END TAG zYx                                                         */

#ifndef _ARQ_TYPES_H
#define _ARQ_TYPES_H


typedef int boolean;
typedef unsigned char byte;

extern const int true;
extern const int false;

typedef enum {
	SUCCESS = 0,
	E_FAILED  = -1,
	E_Q_FULL	= -2,
	E_Q_EMPTY	= -3,
	E_LL_NOT_FOUND = -4,
	E_COND_NOT_MET = -5,
	E_DUPLICATE_CONN = -6,
	E_START_BSN_OUT_OF_RANGE = -7,
	E_INVALID_CONN	= -8,
	E_NULL = -9,
} op_status_t;

#endif //_ARQ_TYPES_H
