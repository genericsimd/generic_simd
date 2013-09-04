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

#define VSXW    "=wa"
#define VSXR    "wa"
#define VSXWC   "=&wa"

/// already implemented in POWER7
//

static FORCEINLINE __vector float               vec_splat_p7(__vector float a, const int v){
  if(__builtin_constant_p(v) && v >= 0 && v < 4) {
    __vector float register r;
    asm ("xxspltw %x[xt], %x[xa],%[im] "  : [xt] VSXW(r) : [xa] VSXR(a), [im] "i"(v)    );
    return r;
  } else {
    float f = vec_extract(a, v);
    __vector float r = {f,f,f,f};
    return r;
  }
}

static FORCEINLINE __vector signed int          vec_splat_p7(__vector signed int a, const int v){
  return (__vector signed int)vec_splat_p7((__vector float)a, v);
}

static FORCEINLINE __vector unsigned int          vec_splat_p7(__vector unsigned int a, const int v){
  return (__vector unsigned int)vec_splat_p7((__vector float)a, v);
}

/**
 * @brief use xxpermdi
 */
static FORCEINLINE __vector double               vec_splat_p7(__vector double a, const int v){
  if(__builtin_constant_p(v) && v >= 0 && v < 2) {
      __vector double register r;
      const int perm_v = (v == 0 ? 0 : 3);
      asm ("xxpermdi %x[xt], %x[xa], %x[xb], %[im] "  : [xt] VSXW(r) : [xa] VSXR(a), [xb] VSXR(a), [im] "i"(perm_v)    );
      return r;
  } else {
    double d = vec_extract(a, v);
    __vector double r = {d,d};
    return r;
  }
}

/**
 * @brief use xxpermdi
 */
static FORCEINLINE __vector long long            vec_splat_p7(__vector long long a, const int v){
  return (__vector long long)vec_splat_p7((__vector double )a, v);
}

/**
 * @brief use xxpermdi
 */
static FORCEINLINE __vector unsigned long long   vec_splat_p7(__vector unsigned long long a, const int v){
  return (__vector unsigned long long)vec_splat_p7((__vector double )a, v);
}



static FORCEINLINE __vector double              vec_smear_p7(double a){
  __vector double register r;
  asm ("xxspltd %x[xt], %x[xa], 0"  : [xt] VSXW(r) : [xa] "f"(a)   );
  return r;
}

static FORCEINLINE __vector float vec_zero_p7(){
  __vector float register r;
  asm   ("vspltisw %[xt], 0": [xt] "=v"(r) );
  return r;
}

static FORCEINLINE __vector unsigned long long   vec_smear_i64_p7(unsigned long long *ptr) {
  __vector unsigned long long r;
  asm ("lxvdsx %x[xt],%y1" : [xt] VSXW(r) : "Z"(*ptr) );
  return r;
}

static FORCEINLINE __vector long long   vec_smear_i64_p7(long long *ptr) {
  __vector long long r;
  asm ("lxvdsx %x[xt],%y1" : [xt] VSXW(r) : "Z"(*ptr) );
  return r;
}

static FORCEINLINE __vector double               vec_smear_double_p7(double *ptr) {
  __vector double r;
  asm ("lxvdsx %x[xt],%y1" : [xt] VSXW(r) : "Z"(*ptr) );
  return r;
}

static FORCEINLINE __vector double               vec_smear_const_double_p7(const double *ptr) {
  __vector double r;
  //asm ("lxvdsx %x[xt],%y1" : [xt] VSXW(r) : "Z"(*ptr) );
  asm ("lxvdsx %x[xt], 0, %[xb]"   : [xt] VSXW(r) : [xb] "b"(ptr) );
  return r;
}

static FORCEINLINE __vector unsigned long long   vec_smear_const_i64_p7(const long long *ptr) {
  __vector unsigned long long r;
  //asm ("lxvdsx %x[xt],%y1" : [xt] VSXW(r) : "Z"(*ptr) );
  asm ("lxvdsx %x[xt], 0, %[xb]"   : [xt] VSXW(r) : [xb] "b"(ptr) );
  return r;
}

/**
 *\brief This one is not really a smear constant. Need fix it.
 */
static FORCEINLINE __vector float               vec_smear_const_float_p7(const __vector float *ptr) {
  __vector float r, r1;
  asm ("lxvw4x %x[xt], 0, %[xb]"   : [xt] VSXW(r) : [xb] "b"(ptr) );
  asm ("vspltw %x[xt], %x[xa], %[im]"   : [xt] VSXW(r1) : [xa] VSXR(r) , [im] "i"(0));;
  return r1;
}

static FORCEINLINE __vector float vec_neg_p7(__vector float a) {
  __vector float register r;
  asm ("xvnegsp %x[xt], %x[xa]"    : [xt] VSXW(r) : [xa] VSXR(a));
  return r;
}


