/**
Copyright 2012 the Generic SIMD Intrinsic Library project authors. All rights reserved.

Copyright IBM Corp. 2013, 2013. All rights reserved.

Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions are
met:

   * Redistributions of source code must retain the above copyright
     notice, this list of conditions and the following disclaimer.
   * Redistributions in binary form must reproduce the above
     copyright notice, this list of conditions and the following
     disclaimer in the documentation and/or other materials provided
     with the distribution.
   * Neither the name of IBM Corp. nor the names of its contributors may be
     used to endorse or promote products derived from this software
     without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
"AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
(INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

/**
 * RGB2YUV.cpp
 *
 *  Created on: Jun 12, 2013
 *  @author: Haichuan Wang (hwang154@illinois.edu)
 *
 * Parameters are from http://en.wikipedia.org/wiki/YUV
 * Storages are based on SoA
 */


/*
 *  IBM Power compiling
 *  g++ -I../../include RGB2YUV.cpp -mvsx -flax-vector-conversions -Wno-int-to-pointer-cast -g -O2 -o RGB2YUV
 * */

#include <getopt.h>
#include <iostream>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <timing.h>
#include <gsimd.h>

//#define N (1048576)
#define N (512*512)

//Doesn't work
//__attribute__((optimize("no-tree-vectorize")))

void
#ifdef __SSE4_2__
__attribute__((target("no-sse")))
#endif
serial_rgb2gray(float* ra, float* ga, float* ba, float* ya, float* ua, float* va) {
    for(int i = 0; i < N; i++) {
        ya[i] = 0.299f * ra[i] + 0.584f * ga[i] + 0.114f * ba[i];
        ua[i] = -0.14713f * ra[i] -0.28886f * ga[i] + 0.436f * ba[i];
        va[i] = 0.615f * ra[i] - 0.51499f * ga[i] - 0.10001f * ba[i];
    }
}

typedef svec<4,float> vfloat;

void svec4_rgb2gray(float* ra, float* ga, float* ba, float* ya, float* ua, float* va) {

    for(int i = 0; i < N; i+=4) {
        vfloat a = vfloat::load((vfloat*)(ra+i));
        vfloat b = vfloat::load((vfloat*)(ga+i));
        vfloat c = vfloat::load((vfloat*)(ba+i));
        vfloat y = 0.299f * a  + 0.584f * b  + 0.114f * c ;
        y.store((vfloat*)(ya+i));
        vfloat u = -0.14713f * a - 0.28886f * b + 0.436f * c;
        u.store((vfloat*)(ua+i));
        vfloat v = 0.615f * a - 0.51499f * b - 0.10001f * c;
        v.store((vfloat*)(va+i));
    }
}

void svec4_rgb2gray_ptr(float* ra, float* ga, float* ba, float* ya, float* ua, float* va) {

    for(int i = 0; i < N; i+=4) {
        vfloat a = *(vfloat*)(ra+i);
        vfloat b = *(vfloat*)(ga+i);
        vfloat c = *(vfloat*)(ba+i);
        vfloat y = 0.299f * a  + 0.584f * b  + 0.114f * c ;
        *(vfloat*)(ya+i) = y;
        vfloat u = -0.14713f * a - 0.28886f * b + 0.436f * c;
        *(vfloat*)(ua+i) = u;
        vfloat v = 0.615f * a - 0.51499f * b - 0.10001f * c;
        *(vfloat*)(va+i) = v;
    }
}

#ifdef __ALTIVEC__
void intrinsics_rgb2gray(float* ra, float* ga, float* ba, float* ya, float* ua, float* va) {
    __vector float c11 = vec_splats(0.299f);
    __vector float c12 = vec_splats(0.584f);
    __vector float c13 = vec_splats(0.114f);
    __vector float c21 = vec_splats(-0.1471f);
    __vector float c22 = vec_splats(-0.28886f);
    __vector float c23 = vec_splats(0.436f);
    __vector float c31 = vec_splats(0.615f);
    __vector float c32 = vec_splats(-0.51499f);
    __vector float c33 = vec_splats(-0.10001f);

    for(int i = 0; i < N; i+=4) {
        __vector float a = vec_vsx_ld(0, ra+i);
        __vector float b = vec_vsx_ld(0, ga+i);
        __vector float c = vec_vsx_ld(0, ba+i);
        __vector float y = c11 * a  + c12 * b  +  c13 * c ;
        vec_vsx_st(y, 0, ya+i);
        __vector float u = c21 * a  + c22 * b  +  c23 * c ;
        vec_vsx_st(u, 0, ua+i);
        __vector float v = c31 * a  + c32 * b  +  c33 * c ;
        vec_vsx_st(v, 0, va+i);
    }
}
#endif

#ifdef __SSE4_2__

void sse_rgb2gray(float* ra, float* ga, float* ba, float* ya, float* ua, float* va) {
  __m128 c11 =  _mm_set1_ps(0.299f);
  __m128 c12 =  _mm_set1_ps(0.584f);
  __m128 c13 =  _mm_set1_ps(0.114f);
  __m128 c21 = _mm_set1_ps(-0.1471f);
  __m128 c22 = _mm_set1_ps(-0.28886f);
  __m128 c23 = _mm_set1_ps(0.436f);
  __m128 c31 = _mm_set1_ps(0.615f);
  __m128 c32 = _mm_set1_ps(-0.51499f);
  __m128 c33 = _mm_set1_ps(-0.10001f);


  for(int i = 0; i < N; i+=4) {
      __m128 a = _mm_loadu_ps(ra+i);
      __m128 b = _mm_loadu_ps(ga+i);
      __m128 c = _mm_loadu_ps(ba+i);
      __m128 y = _mm_add_ps(_mm_add_ps(_mm_mul_ps(c11, a), _mm_mul_ps(c12, b)), _mm_mul_ps(c13, c));
      _mm_storeu_ps(ya+i, y);
      __m128 u = _mm_add_ps(_mm_add_ps(_mm_mul_ps(c21, a), _mm_mul_ps(c22, b)), _mm_mul_ps(c23, c));
      _mm_storeu_ps(ua+i, u);
      __m128 v = _mm_add_ps(_mm_add_ps(_mm_mul_ps(c31, a), _mm_mul_ps(c32, b)), _mm_mul_ps(c33, c));
      _mm_storeu_ps(va+i, v);
  }
}

#endif


//the strange 100,200,300,... offset is used to reduce the effect of "address conflicts"
float r[N+100] POST_ALIGN(16);
float g[N+200] POST_ALIGN(16);
float b[N+300] POST_ALIGN(16);
float y[N+400] POST_ALIGN(16);
float u[N+500] POST_ALIGN(16);
float v[N+600] POST_ALIGN(16);

#define ITERATIONS 1000
int main (int argc, char* argv[])
{
    for(int i = 0; i < N; i++) {
        r[N] = random() % 256;
        g[N] = random() % 256;
        b[N] = random() % 256;
    }
    std::cout<< "Convert " << N << " pixels RGB to YUV." << std::endl;

    reset_and_start_stimer();
    for(int i = 0; i < ITERATIONS; i++) { serial_rgb2gray(r, g, b, y, u, v);}
    double dt = get_elapsed_seconds();
    std::cout<< "serial version: " << dt << " seconds" << std::endl;

    reset_and_start_stimer();
    for(int i = 0; i < ITERATIONS; i++) { svec4_rgb2gray(r, g, b, y, u, v);}
    double dt2 = get_elapsed_seconds();
    std::cout<< "svec4 version: " << dt2 << " seconds" << std::endl;

    reset_and_start_stimer();
    for(int i = 0; i < ITERATIONS; i++) { svec4_rgb2gray_ptr(r, g, b, y, u, v); }
    double dt3 = get_elapsed_seconds();
    std::cout<< "svec4 ptr ld/st version: " << dt3 << " seconds" << std::endl;

#ifdef __ALTIVEC__
    reset_and_start_stimer();
    for(int i = 0; i < ITERATIONS; i++) { intrinsics_rgb2gray(r, g, b, y, u, v);}
    double dt5 = get_elapsed_seconds();
    std::cout<< "Power VSX version: " << dt5 << " seconds" << std::endl;
#endif

#ifdef __SSE4_2__
    reset_and_start_stimer();
    for(int i = 0; i < ITERATIONS; i++) { sse_rgb2gray(r, g, b, y, u, v);}
    double dt6 = get_elapsed_seconds();
    std::cout<< "SSE version: " << dt6 << " seconds" << std::endl;
#endif

    return 0;
}

