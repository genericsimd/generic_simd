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

/*
 * RGB2Gray_tune.cpp
 *
 *  Created on: Jun 12, 2013
 *      Author: haichuan
 */


/*
 *  g++ -I../../include RGB2Gray.cpp -mvsx -flax-vector-conversions -Wno-int-to-pointer-cast -g -O2 -o RGB2Gray
 * */

#include <getopt.h>
#include <iostream>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <timing.h>
#include <perfmeasure.h>
#ifdef __ALTIVEC__
#include <power_vsx4.h>
using namespace vsx;
#else
#ifdef __SSE4_2__
#include <sse4.h>
using namespace sse;
#else
#include <generic4.h>
using namespace generic;
#endif //__SSE4_2__
#endif //__ALTIVEC__



//#define N (16000)
//#define N 1000000
#define N (1048576)
#ifdef __SSE4_2__
__attribute__((target("no-sse")))
#endif
void serial_rgb2gray(float* ra, float* ga, float* ba, float* gray) {
    for(int i = 0; i < N; i++) {
        gray[i] = 0.3f * ra[i] + 0.59f * ga[i] + 0.11f * ba[i];
    }
}

void
__attribute__((optimize("no-tree-vectorize")))
svec4_rgb2gray(float* ra, float* ga, float* ba, float* gray ) {

    for(int i = 0; i < N; i+=4) {
        svec4_f a = svec4_f::load((svec4_f*)(ra+i));
        svec4_f b = svec4_f::load((svec4_f*)(ga+i));
        svec4_f c = svec4_f::load((svec4_f*)(ba+i));
        svec4_f out = 0.3f * a  + 0.59f * b  + 0.11f * c ;
        out.store((svec4_f*)(gray+i));
    }
}

void
__attribute__((optimize("no-tree-vectorize")))
svec4_rgb2gray_ptr(float* ra, float* ga, float* ba, float* gray ) {

    for(int i = 0; i < N; i+=4) {
        svec4_f a = *(svec4_f*)(ra+i);
        svec4_f b = *(svec4_f*)(ga+i);
        svec4_f c = *(svec4_f*)(ba+i);
        svec4_f out = 0.3f * a  + 0.59f * b  + 0.11f * c ;
        *(svec4_f*)(gray+i) = out;
    }
}


void
__attribute__((optimize("no-tree-vectorize")))
svec4_rgb2gray_fma(float* ra, float* ga, float* ba, float* gray) {
    for(int i = 0; i < N; i+=4) {
        svec4_f a = svec4_f::load((svec4_f*)(ra+i));
        svec4_f b = svec4_f::load((svec4_f*)(ga+i));
        svec4_f c = svec4_f::load((svec4_f*)(ba+i));
        svec4_f out = 0.3 * a;
        out = svec_madd(svec4_f(0.59), b, out);
        out = svec_madd(svec4_f(0.11), c, out);
        out.store((svec4_f*)(gray+i));
    }
}

#ifdef __ALTIVEC__
void intrinsics_rgb2gray(float* ra, float* ga, float* ba, float* gray ) {
    __vector float c1 = vec_splats(0.3f);
    __vector float c2 = vec_splats(0.59f);
    __vector float c3 = vec_splats(0.11f);

    for(int i = 0; i < N; i+=4) {
        __vector float a = vec_vsx_ld(0, ra+i);
        __vector float b = vec_vsx_ld(0, ga+i);
        __vector float c = vec_vsx_ld(0, ba+i);
        __vector float out = c1 * a  + c2 * b  +  c3 * c ;
        vec_vsx_st(out, 0, gray+i);
    }
}
#endif


#ifdef __SSE4_2__

void sse_rgb2gray(float* ra, float* ga, float* ba, float* gray) {
  __m128 c1 =  _mm_set1_ps(0.3f);
  __m128 c2 =  _mm_set1_ps(0.59f);
  __m128 c3 =  _mm_set1_ps(0.11f);

  for(int i = 0; i < N; i+=4) {
      __m128 a = _mm_loadu_ps(ra+i);
      __m128 b = _mm_loadu_ps(ga+i);
      __m128 c = _mm_loadu_ps(ba+i);
      __m128 ab = _mm_add_ps(_mm_mul_ps(c1, a), _mm_mul_ps(c2, b));
      __m128 out = _mm_add_ps(ab, _mm_mul_ps(c3, c));
      _mm_storeu_ps(gray+i, out);
  }
}
#endif


#ifdef __AVX__
#include "immintrin.h"
void avx_rgb2gray(float* ra, float* ga, float* ba, float* gray) {
  __m256 c1 =  _mm256_set1_ps(0.3f);
  __m256 c2 =  _mm256_set1_ps(0.59f);
  __m256 c3 =  _mm256_set1_ps(0.11f);

  for(int i = 0; i < N; i+=8) {
      __m256 a = _mm256_loadu_ps(ra+i);
      __m256 b = _mm256_loadu_ps(ga+i);
      __m256 c = _mm256_loadu_ps(ba+i);
      __m256 ab = _mm256_add_ps(_mm256_mul_ps(c1, a), _mm256_mul_ps(c2, b));
      __m256 out = _mm256_add_ps(ab, _mm256_mul_ps(c3, c));
      _mm256_storeu_ps(gray+i, out);
  }
}

#endif

float r[N+10000] POST_ALIGN(16);
float g[N+20000] POST_ALIGN(16);
float b[N+30000] POST_ALIGN(16);
float gray[N+40000] POST_ALIGN(16);

#define ITERATIONS 1000
int main (int argc, char* argv[])
{

    for(int i = 0; i < N; i++) {
        r[N] = random() % 256;
        g[N] = random() % 256;
        b[N] = random() % 256;
    }
    std::cout<< "Convert " << N << " pixels RGB to gray." << std::endl;

    HPM_PERF_CREATE;

    HPM_PERF_START;
    reset_and_start_stimer();
    for(int i = 0; i < ITERATIONS; i++) { serial_rgb2gray(r, g, b, gray);}
    double dt = get_elapsed_seconds();
    HPM_PERF_STOP;
    std::cout<< "serial version: " << dt << " seconds" << std::endl;

    HPM_PERF_START;
    reset_and_start_stimer();
    for(int i = 0; i < ITERATIONS; i++) { svec4_rgb2gray(r, g, b, gray);}
    double dt2 = get_elapsed_seconds();
    HPM_PERF_STOP;
    std::cout<< "svec4 version: " << dt2 << " seconds" << std::endl;

    HPM_PERF_START;
    reset_and_start_stimer();
    for(int i = 0; i < ITERATIONS; i++) { svec4_rgb2gray_ptr(r, g, b, gray); }
    double dt3 = get_elapsed_seconds();
    HPM_PERF_STOP;
    std::cout<< "svec4 ptr ld/st version: " << dt3 << " seconds" << std::endl;

    HPM_PERF_START;
    reset_and_start_stimer();
    for(int i = 0; i < ITERATIONS; i++) { svec4_rgb2gray_fma(r, g, b, gray); }
    double dt4 = get_elapsed_seconds();
    HPM_PERF_STOP;
    std::cout<< "svec4 fma version: " << dt4 << " seconds" << std::endl;

#ifdef __ALTIVEC__
    HPM_PERF_START;
    reset_and_start_stimer();
    for(int i = 0; i < ITERATIONS; i++) { intrinsics_rgb2gray(r, g, b, gray);}
    double dt5 = get_elapsed_seconds();
    HPM_PERF_STOP;
    std::cout<< "Intrinsics version: " << dt5 << " seconds" << std::endl;
#endif

#ifdef __SSE4_2__
    HPM_PERF_START;
    reset_and_start_stimer();
    for(int i = 0; i < ITERATIONS; i++) { sse_rgb2gray(r, g, b, gray);}
    double dt6 = get_elapsed_seconds();
    HPM_PERF_STOP;
    std::cout<< "SSE version: " << dt6 << " seconds" << std::endl;
#endif

#ifdef __AVX__
    HPM_PERF_START;
    reset_and_start_stimer();
    for(int i = 0; i < ITERATIONS; i++) { avx_rgb2gray(r, g, b, gray);}
    double dt7 = get_elapsed_seconds();
    HPM_PERF_STOP;
    std::cout<< "AVX version: " << dt7 << " seconds" << std::endl;
#endif

    HPM_PERF_CLOSE;
    return 0;
}

