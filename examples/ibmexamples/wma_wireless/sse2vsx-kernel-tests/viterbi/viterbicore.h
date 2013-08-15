/* ----------------------------------------------------------------------------
   IBM Confidential

   IBM Wireless M2M Platform

   (C)Copyright IBM Corp. 2009, 2010, 2011

   All Rights Reserved.

   File Name: 

   Function: 

      ----------------------------------------------------------------------------
   PROLOG END TAG zYx                                                         */


#ifndef _VITERBICORE_H
#define _VITERBICORE_H

#include <stdio.h>
#include <stdlib.h>


#define ZERO 			0
#define SAMPLE_RATE 		4
#define SAMPLE_MASK_CHAR 	0x0F
#define SAMPLE_MASK_SHORT 	0xFF

/* Encoder and decoder polynomials */
#define VITERBI_POLYB  		0x6d
#define VITERBI_POLYA  		0x4f

/*
 * Follwoing is the decoding algorithm option
 * ONETBBLOCK decode the block in one TB block, high complexity and better performance
 * If ONETBBLOCK is commented, the multi TBs algorithm is selected,low complexity and accepted performance
*/
//#define ONETBBLOCK

/* Decoder metric data type */
#define CHARTYPE
//#define SHORTTYPE

#ifdef CHARTYPE
#define mtype  char                 /* This define the type of the metric of all the trace back bits */
#endif
#ifdef SHORTTYPE
#define mtype  short                /* This define the type of the metric of all the trace back bits */
#endif

typedef union {
  unsigned mtype  w[64];
} metric_t;     //w[64] save the metrices of  all the posible state

typedef union {
  unsigned mtype m[64];
} decision_t;

/* State info for instance of Viterbi decoder*/
struct va_ctx {
  metric_t metric_buf1;     /* Path metric buffer 1 */
  metric_t metric_buf2;     /* Path metric buffer 2 */
  metric_t *old_metrics,*new_metrics; /* Pointers to path metrics, swapped on every bit */
  decision_t *decisions;    /* Beginning of decisions for block */
  decision_t *curr_decision;    /* Pointer to current decision */
};

/**************************************************************************************************
   Function:    create_viterbi
   Description: Create viterbi decoder instance.
   Parameters:
                len:  The strict length 

   Return Value:
                pointer: pointer to the viterbi decoder instance create.
                NULL:     Error

****************************************************************************************************/
void *create_viterbi(int len);

/**************************************************************************************************
   Function:    init_viterbi
   Description: Initialize the viterbi decoder to the starting state.

   Parameters:
		p:	Viterbi instance pointer
                starting_state:  The starting state to set 

   Return Value:
                0: 	Success
                -EPERM: Viterbi instance pointer is NULL 

****************************************************************************************************/
int init_viterbi(void *p,int starting_state);

/**************************************************************************************************
   Function:    update_viterbi_blk

   Description: Update decoder with a block of demodulated symbols

   Parameters:
		p:	Viterbi instance pointer
                syms:   Demodulated symbols to be decoded.
                nbits:  The number of decoded data bits, not the number of symbols!

   Return Value:
                0: 	Success
                -EPERM: Viterbi instance pointer is NULL 

****************************************************************************************************/
int update_viterbi_blk(void *p, unsigned char *syms,int nbits);

/**************************************************************************************************
   Function:    chainback_viterbi_withtail 

   Description: Viterbi chainback with zero tail padding

   Parameters:
		p:	Viterbi instance pointer
                data:   Decoded output data
                nbits:  The number of decoded data bits.
		endstate: The endstate where the chainback would start

   Return Value:
                0: 	Success
                -EPERM: Viterbi instance pointer is NULL 

****************************************************************************************************/
int chainback_viterbi_withtail(void *p,
                               unsigned char *data, /* */
                               unsigned int nbits,  /* Number of data bits */
                               unsigned int endstate);


/**************************************************************************************************
   Function:    chainback_viterbi_notail 

   Description: Viterbi chainback with tail-bitting

   Parameters:
		p:	Viterbi instance pointer
                data:   Decoded output data
                nbits:  The number of decoded data bits.
		end_state: The end state where the chainback would start

   Return Value:
                0: 	Success
                -EPERM: Viterbi instance pointer is NULL 

****************************************************************************************************/
int chainback_viterbi_notail(void *p,
                             unsigned char *data,  /* Decoded output data */
                             unsigned int  nbits,  /* Number of data bits */
                             unsigned int  tblen,  /* Trace Back block length */
                             unsigned int  *end_state);


/**************************************************************************************************
   Function:    delete_viterbi
   Description: De-initialize the viterbi decoder.

   Parameters:
		p:	Viterbi instance pointer

   Return Value:

****************************************************************************************************/
void delete_viterbi(void *p);

#endif


