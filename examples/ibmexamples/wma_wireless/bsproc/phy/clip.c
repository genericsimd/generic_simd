/* ----------------------------------------------------------------------------
   IBM Confidential

   IBM Wireless M2M Platform

   (C)Copyright IBM Corp. 2009, 2010, 2011

   All Rights Reserved.

   File Name: 

   Function: 

      ----------------------------------------------------------------------------
   PROLOG END TAG zYx                                                         */


#include <math.h>
#include "clip.h"

#define  IClip( Min, Max, Val) (((Val)<(Min))? (Min):(((Val)>(Max))? (Max):(Val)))

float getmax(float* array, u_int32_t size)
{ 
	u_int32_t i;
	float max =0;
	for(i=0;i<size;i++)
	{
	   if(max< (fabs(array[i])))
		max= fabs(array[i]);
	}

      return max;
}

/*
void clip(float *input, u_int8_t *output, u_int32_t len, float a)
{
	u_int32_t i;
	float a_8 = 8.0 /(2* a);
        float temp;
        u_int8_t a_min, b;
        a_min = 0;
        b = 15;
	for(i = 0; i < len; i++)
	{
		temp= IClip(-a, a, input[i]); 
	      //  output[i] = (int32_t)((temp * a_8 + 8.0));//original one
                output[i] = (u_int8_t)((temp * a_8 + 8.0));//added by wq
                   
		output[i]=(u_int8_t) IClip(a_min, b, output[i]);

	}

}
*/

void clip(float *input, u_int8_t *output, u_int32_t len, float a)
{
        u_int32_t i;
        float a_8 = 8.0 /(2.0* a);
        float temp;
        u_int8_t a_min, b;
        a_min = 0;
        b = 15;
        for(i = 0; i < len; i++)
        {
            temp= IClip(-a, a, input[i]);
            output[i] = (u_int8_t)((temp * a_8 + 8.0));//added by wq
            output[i]=(u_int8_t) IClip(a_min, b, output[i]);
        }

}


void clip_alg3(float *input, u_int8_t *output, u_int32_t len, u_int8_t threshold, u_int8_t soft_shift)
{

        u_int32_t i;
        u_int8_t a;
        a = threshold - 1;
        for(i = 0; i < len; i++)
        {
            input[i] = input[i] * pow(2, soft_shift);
            if ( input[i] > (float)a)
            {
                input[i] = a ;
            }
            else if (input[i] <(float)(-a))
            {
                input[i] = -( a + 1);
            }

            output[i] = (int8_t)input[i] + a + 1;
        }

}






