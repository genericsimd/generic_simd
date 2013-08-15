/* ----------------------------------------------------------------------------
   IBM Confidential

   IBM Wireless M2M Platform

   (C)Copyright IBM Corp. 2009, 2010, 2011

   All Rights Reserved.

   File Name: clip.h

   Function: Interface claim for clip()

      ----------------------------------------------------------------------------
   PROLOG END TAG zYx                                                         */



#ifndef _CLIP_H_
#define _CLIP_H_

#include <sys/types.h>

float getmax(float* array, u_int32_t size);

void clip(float *input, u_int8_t *output, u_int32_t len, float a);

void clip_alg3(float *input, u_int8_t *output, u_int32_t len, u_int8_t threshold, u_int8_t soft_shift);


#endif
