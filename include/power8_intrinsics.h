/**
 * IBM Confidential
 */

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

//
//// POWER8 intrinsics
//

#ifdef __POWER8

////////////////////////////////////////////////
//   int 64 math/logic operations
static FORCEINLINE __vector signed long long vec_add_p8(__vector signed long long a,
					                  __vector signed long long b){
  __vector signed long long register r;
  asm ("vaddudm %[xt], %[xa], %[xb]" : [xt] "=v"(r) : [xa] "v"(a), [xb] "v"(b) );
  return r;
}

static FORCEINLINE __vector signed long long vec_sub_p8(__vector signed long long a,
                                                          __vector signed long long b){
  __vector signed long long register r;
  asm ("vsubudm %[xt], %[xa], %[xb]" : [xt] "=v"(r) : [xa] "v"(a), [xb] "v"(b) );
  return r;
}


static FORCEINLINE __vector signed long long vec_sld_p8(__vector signed long long a,
                                                          __vector signed long long b){
  __vector signed long long register r;
  asm ("vsld %[xt], %[xa], %[xb]" : [xt] "=v"(r) : [xa] "v"(a), [xb] "v"(b) );
  return r;
}


static FORCEINLINE __vector unsigned int       vec_pack_p8(__vector signed long long a,
					                   __vector signed long long b){
  __vector unsigned int  register r;
  asm ("vpkuhum %[xt], %[xa], %[xb]" : [xt] "=v"(r) : [xa] "v"(a), [xb] "v"(b) );
  return r;
}

static FORCEINLINE __vector unsigned long long vec_unpackh_p8(__vector unsigned int a){
  __vector unsigned long long register r;
  asm ("vupkhsw %[xt], %[xa]"        : [xt] "=v"(r)  : [xa] "v"(a)    );
  return r;
}

static FORCEINLINE __vector unsigned long long vec_unpackl_p8(__vector unsigned int a){
  __vector unsigned long long register r;
  asm ("vupklsw %[xt], %[xa]"        : [xt] "=v"(r)  : [xa] "v"(a)    );
  return r;
}

static FORCEINLINE __vector signed long long vec_cmpeq_p8(__vector signed long long a,
						            __vector signed long long b){
  __vector signed long long register r;
  asm ("vcmpequd %[xt], %[xa], %[xb]": [xt] "=v"(r) : [xa] "v"(a), [xb] "v"(b) );
  return r;
}

static FORCEINLINE __vector unsigned long long vec_sel_p8(__vector unsigned long long a,
					                  __vector unsigned long long b,
					                  __vector unsigned long long m /*mask*/){
  __vector unsigned long long register r;
  asm ("vsel %[xt], %[xa], %[xb], %[xc]": [xt] "=v"(r) : [xa] "v"(a), [xb] "v"(b), [xc] "v"(m));
  return r;
}

static FORCEINLINE __vector double vec_sel_p8(__vector double a,__vector double b, __vector double m){
  __vector double register r;
  asm ("vsel %[xt], %[xa], %[xb], %[xc]": [xt] "=v"(r) : [xa] "v"(a), [xb] "v"(b), [xc] "v"(m));
  return r;
}

static FORCEINLINE __vector signed int   vec_smear_p8(signed int a){
  __vector signed int register r, t;
  asm ("mtvsrwz %x[xt], %[xa]"      : [xt] VSXW(t) : [xa] "r"(a)   );
  asm ("xxspltw %x[xt], %x[xa], 1"  : [xt] VSXW(r) : [xa] VSXR(t)   );
  return r;
}

static FORCEINLINE __vector unsigned int vec_smear_p8(unsigned int a){
  __vector unsigned int register r, t;
  asm ("mtvsrwz %x[xt], %[xa]"      : [xt] VSXW(t) : [xa] "r"(a)   );
  asm ("xxspltw %x[xt], %x[xa], 1"  : [xt] VSXW(r) : [xa] VSXR(t)   );
  return r;
}

static FORCEINLINE __vector float        vec_smear_p8(float a){
  __vector float register r, t;
  asm ("xscvdpspn %x[xt], %[xa]"    : [xt] VSXW(t) : [xa] "f"(a)   );
  asm ("xxspltw %x[xt], %x[xa], 0"  : [xt] VSXW(r) : [xa] VSXR(t)   );
  return r;
}

static FORCEINLINE __vector unsigned long long  vec_smear_i64_p8(long long a){
  __vector unsigned long long register r, t;
  asm ("mtvsrd  %x[xt], %[xa]"      : [xt] VSXW(t) : [xa] "r"(a)   );
  asm ("xxspltd %x[xt], %x[xa], 0"  : [xt] VSXW(r) : [xa] VSXR(t)   );
  return r;
}

static FORCEINLINE __vector float               vec_smear_float_p8(float *ptr){
  __vector float register r, t;
  //asm ("lxsiwzx %x[xt], 0, %[xb]"   : [xt] VSXW(t) : [xb] "b"(ptr): "memory");
  asm ("lxsiwzx %x[xt],%y1"         : [xt] VSXW(t) : "Z"(*ptr));
  asm ("xxspltw %x[xt], %x[xa], 1"  : [xt] VSXW(r) : [xa] VSXR(t)   );
  return r;
}

static FORCEINLINE __vector unsigned int        vec_smear_i32_p8(unsigned int *ptr){
  __vector unsigned int register r, t;
  //asm ("lxsiwzx %x[xt], 0, %[xb]"   : [xt] VSXW(t) : [xb] "b"(ptr): "memory");
  asm ("lxsiwzx %x[xt],%y1"         : [xt] VSXW(t) : "Z"(*ptr));
  asm ("xxspltw %x[xt], %x[xa], 1"  : [xt] VSXW(r) : [xa] VSXR(t)   );
  return r;
}


static FORCEINLINE __vector unsigned int vec_mul_p8(__vector unsigned int a,__vector unsigned int b) {
  __vector unsigned int register r;
  asm ("vmuluwm %[xt], %[xa], %[xb]": [xt] "=v"(r)  : [xa] "v"(a), [xb] "v"(b) );
  return r;
}

//P8 specific extract 
static FORCEINLINE uint64_t vec_extract_l(__vector int a) {
  uint64_t register r;
  asm ("mfvsrd %[ra], %x[xs]": [ra] "=r"(r)  : [xs] VSXR(a) );
  return r;
}

static FORCEINLINE uint64_t vec_extract_r(__vector int a) {
  uint64_t register r;
  __vector int register tmp;
  asm ("vsldoi %[xt], %[xa], %[xb], %[im]": [xt] "=v"(tmp)  : [xa] "v"(a), [xb] "v"(a),[im] "i"(8) );
  asm ("mfvsrd %[ra], %x[xs]": [ra] "=r"(r)  : [xs] VSXR(tmp) );
  return r;
}

static FORCEINLINE uint64_t vec_extract_l(__vector float a) {
  uint64_t register r;
  asm ("mfvsrd %[ra], %x[xs]": [ra] "=r"(r)  : [xs] VSXR(a) );
  return r;
}

static FORCEINLINE uint64_t vec_extract_r(__vector float a) {
  uint64_t register r;
  __vector int register tmp;
  asm ("vsldoi %[xt], %[xa], %[xb], %[im]": [xt] "=v"(tmp)  : [xa] "v"(a), [xb] "v"(a),[im] "i"(8) );
  asm ("mfvsrd %[ra], %x[xs]": [ra] "=r"(r)  : [xs] VSXR(tmp) );
  return r;
}

#define GATHER_WORD_OFF32_P8(TYPE) \
static FORCEINLINE __vector TYPE vec_gather_p8(TYPE *ptr0, \
					       TYPE *ptr1, \
					       TYPE *ptr2, \
					       TYPE *ptr3){ \
  __vector TYPE register r0,r1,r2,r3;			    \
  __vector TYPE register t0,t1;				    \
  __vector TYPE register r;						\
  asm ("lxsiwzx %x[xt],%y1"         : [xt] VSXW(r0) : "Z"(*ptr0));	\
  asm ("lxsiwzx %x[xt],%y1"         : [xt] VSXW(r1) : "Z"(*ptr1));	\
  asm ("xxmrghd %x[xt], %x[xa], %x[xb]"         : [xt] VSXW(t0) : [xa] VSXR(r0), [xb] VSXR(r1) ); \
  asm ("lxsiwzx %x[xt],%y1"         : [xt] VSXW(r2) : "Z"(*ptr2));	\
  asm ("lxsiwzx %x[xt],%y1"         : [xt] VSXW(r3) : "Z"(*ptr3));	\
  asm ("xxmrghd %x[xt], %x[xa], %x[xb]"         : [xt] VSXW(t1) : [xa] VSXR(r2), [xb] VSXR(r3) ); \
  asm ("vpkudum %[xt], %[xa], %[xb]"         : [xt] "=v"(r) : [xa] "v"(t0), [xb] "v"(t1) ); \
  return r;								\
}									

GATHER_WORD_OFF32_P8(float)
GATHER_WORD_OFF32_P8(signed int)
GATHER_WORD_OFF32_P8(unsigned int)



#define GATHER_D_WORD_OFF32_P8(TYPE) \
static FORCEINLINE __vector TYPE vec_gather_p8(TYPE *ptr0, \
					       TYPE *ptr1){	    \
  __vector TYPE register r0,r1;			    \
  __vector TYPE register r;						\
  asm ("lxvdsx %x[xt],%y1"         : [xt] VSXW(r0) : "Z"(*ptr0));	\
  asm ("lxvdsx %x[xt],%y1"         : [xt] VSXW(r1) : "Z"(*ptr1));	\
  asm ("xxmrghd %x[xt], %x[xa], %x[xb]"         : [xt] VSXW(r) : [xa] VSXR(r0), [xb] VSXR(r1) ); \
  return r;								\
}									

GATHER_D_WORD_OFF32_P8(double)
GATHER_D_WORD_OFF32_P8(signed long)

//POWER 8 Scatter Intrinsics

#define SCATTER_WORD_OFF32_P8(TYPE,IMM)					\
void vec_scatter_step_##IMM(TYPE* ptr0, __vector TYPE val){ \
 __vector TYPE register tmp;							\
 asm ("vsldoi %[xt], %[xa], %[xb], %[im]": [xt] "=v"(tmp)  : [xa] "v"(val), [xb] "v"(val),[im] "i"(IMM) ); \
 asm ("stxsiwx %x[xt],%y1"         : : [xt] VSXR(tmp), "Z"(*ptr0));	\
}

#define SCATTER_WORD_OFF32_Z_P8(TYPE)					\
void vec_scatter_step_0(TYPE* ptr0, __vector TYPE val){ \
 asm ("stxsiwx %x[xt],%y1"         : : [xt] VSXR(val), "Z"(*ptr0));	\
}

SCATTER_WORD_OFF32_Z_P8(float)
SCATTER_WORD_OFF32_P8(float,4)
SCATTER_WORD_OFF32_P8(float,8)
SCATTER_WORD_OFF32_P8(float,12)

SCATTER_WORD_OFF32_Z_P8(signed int)
SCATTER_WORD_OFF32_P8(signed int,4)
SCATTER_WORD_OFF32_P8(signed int,8)
SCATTER_WORD_OFF32_P8(signed int,12)

SCATTER_WORD_OFF32_Z_P8(unsigned int)
SCATTER_WORD_OFF32_P8(unsigned int,4)
SCATTER_WORD_OFF32_P8(unsigned int,8)
SCATTER_WORD_OFF32_P8(unsigned int,12)

#define SCATTER_D_WORD_OFF32_P8(TYPE) \
void vec_scatter_step_8(TYPE* ptr0, __vector TYPE val){ \
 __vector TYPE tmp;							\
 asm ("vsldoi %[xt], %[xa], %[xb], %[im]": [xt] "=v"(tmp)  : [xa] "v"(val), [xb] "v"(val),[im] "i"(8) ); \
 asm ("stxsdx %x[xt],%y1"         : : [xt] VSXR(tmp), "Z"(*ptr0));	\
}

#define SCATTER_D_WORD_OFF32_Z_P8(TYPE) \
void vec_scatter_step_0(TYPE* ptr0, __vector TYPE val){ \
 asm ("stxsdx %x[xt],%y1"         : : [xt] VSXR(val), "Z"(*ptr0));	\
}

SCATTER_D_WORD_OFF32_P8(double)
SCATTER_D_WORD_OFF32_P8(signed long)
SCATTER_D_WORD_OFF32_Z_P8(double)
SCATTER_D_WORD_OFF32_Z_P8(signed long)
#endif
