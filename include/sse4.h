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

The original source code covered by the above license above has been
modified significantly by IBM Corp.
Copyright 2013 the Generic SIMD Intrinsic Library project authors. All rights reserved.

Copyright (c) 2010-2012, Intel Corporation
  All rights reserved.

  Redistribution and use in source and binary forms, with or without
  modification, are permitted provided that the following conditions are
  met:

    * Redistributions of source code must retain the above copyright
      notice, this list of conditions and the following disclaimer.

    * Redistributions in binary form must reproduce the above copyright
      notice, this list of conditions and the following disclaimer in the
      documentation and/or other materials provided with the distribution.

    * Neither the name of Intel Corporation nor the names of its
      contributors may be used to endorse or promote products derived from
      this software without specific prior written permission.


   THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS
   IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED
   TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A
   PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER
   OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
   EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
   PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
   PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF
   LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
   NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
   SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

/**
 * @file sse4.h
 * @date  July 20, 2013
 * @author Haichuan Wang (haichuan@us.ibm.com, hwang154@illinois.edu)
 * @brief SIMD LANES=4 interfaces implemented by scalar
 *
 * The source file is organized as follows
 * I. all types definition, with class function incorporated
 * II. data operation interfaces
 *   1. load/store
 *   2. select
 *   3. broadcast/rotate/shuffle/smear/setzero
 *   4. gather/scatter
 *   5. load const, smear const, load and splat
 *   6. masked load/masked store
 * III. Mask type (i1) interfaces
 *   1. mask construction
 *   2. bit operations
 * IV. General data operation interfaces
 *   1. Unary
 *   2. Math unary
 *   3. Binary
 *   4. Ternary
 *   5. Compare
 *   6. Max/Min
 *   7. Reduce
 *   8. Cast
 *
 *  The current implementation is based on Intel ISPC SIMD intrinsics.
 *  ISPC: http://ispc.github.io/
 */




#ifndef SSE4_H_
#define SSE4_H_

#include <stdint.h>
#include <math.h>
#include <assert.h>
#include <iostream>

#include <smmintrin.h>
#include <nmmintrin.h>
#include "gsimd_utility.h"


#ifndef __SSE4_2__
# error "SSE 4.2 must be enabled in the C++ compiler to use this header."
#endif // !__SSE4_2__

namespace sse {

#define LANES 4

//////////////////////////////////////////////////////////////
//
// Constructor Section
//
//////////////////////////////////////////////////////////////
template <int Lanes, class T>
struct svec : public invalid_template_arguments<Lanes,T>::type {
  //here we need to add the static assert
};

template <>
struct svec<4,bool>;
template <>
  struct svec<4,int8_t>;
template <>
  struct svec<4,uint8_t>;
template <>
  struct svec<4,int16_t>;
template <>
  struct svec<4,uint16_t>;
template <>
  struct svec<4,int32_t>;
template <>
  struct svec<4,uint32_t>;
template <>
  struct svec<4,int64_t>;
template <>
  struct svec<4,uint64_t>;
template <>
  struct svec<4,float>;
template <>
  struct svec<4,double>;
template <>
  struct svec<4,void*>;

//required because macros are confused by the , in the template declaration
//typedef svec<4,bool> _svec4_i1;
//typedef svec<4,int8_t> svec<4,int8_t>;
//typedef svec<4,uint8_t> svec<4,uint8_t>;
//typedef svec<4,int16_t> svec<4,int16_t>;
//typedef svec<4,uint16_t> svec<4,uint16_t>;
//typedef svec<4,int32_t> svec<4,int32_t>;
//typedef svec<4,uint32_t> svec<4,uint32_t>;
//typedef svec<4,int64_t> _svec4_i64;
//typedef svec<4,uint64_t> _svec4_u64;
//typedef svec<4,float> _svec4_f;
//typedef svec<4,double> _svec4_d;
//typedef svec<4,void*> _svec4_ptr;

/**
 * @brief Data representation and operations on a vector of 4 boolean values.
 * This is used in predicated vector operations. Specifically the ith value of 
 * svec<4,bool> indicates whether the ith lane of a predicated vector operation is 
 * enabled or not.
 * 
 * See also gather, scatter, load, store, and compare operations.
 */
template<>
struct svec<4,bool> {

  __m128 v; //only use 4 bits

    /**
     * @brief Default constructor. 
     * @return a vector of 4 undefined values.
     */
    FORCEINLINE svec() { }
    /**
     * @brief For internal use only.
     * @param[in] vv a __m128 valye.
     * @return a mask vector whose value is from the vv.
     */
    FORCEINLINE svec(__m128 vv) : v(vv) { }
    /**
     * @brief For internal use only.
     * @param[in] vv a __m128i valye.
     * @return a mask vector whose value is from the vv.
     */
    FORCEINLINE svec(__m128i vv) : v(_mm_castsi128_ps(vv)) { }
    /** 
     * @brief Constructor.
     * @param[in] a,b,c,d boolean values
     * \note a,b,c,d must be either 0 or -1
     * @return a vector of 4 mask/booleans: {a,b,c,d}.
     */
    FORCEINLINE svec(uint32_t a, uint32_t b, uint32_t c, uint32_t d) {
      v = _mm_castsi128_ps(_mm_set_epi32(d ? -1 : 0, c ? -1 : 0,
                                         b ? -1 : 0, a ? -1 : 0));
    }
    /**
     * @brief Constructor.
     * @param[in] a a boolean value
     * \note a must be either 0 or -1
     * @return a vector of 4 mask/booleans: {a,a,a,a}.
     */
    FORCEINLINE svec(uint32_t a){
      v = (a != 0) ? _mm_castsi128_ps(_mm_set1_epi32(-1)) : _mm_setzero_ps();
    }

    SUBSCRIPT_FUNC_BOOL_DECL(uint32_t);
    COUT_FUNC_BOOL_DECL();
    SVEC_BOOL_CLASS_METHOD_DECL();
};


/**
 * @brief data representation and operations on a vector of 4 signed chars.
 */
template <>
struct svec<4,int8_t> {
  __m128i v;

    /**
     * @brief Default constructor
     * @return a vector of 4 undefined signed chars.
     */
    FORCEINLINE svec() { }
    /**
     * @brief For internal use only.
     * @param vv a __m128i value.
     * @return a signed char vector, whose value is from the vv.
     */
    FORCEINLINE svec(__m128i vv) : v(vv) {  }
    /**
     * @brief Constructor
     * @return a vector of 4 signed chars: {a,b,c,d}.
     */
    FORCEINLINE svec(int8_t a, int8_t b, int8_t c, int8_t d) {
      v = _mm_set_epi8(0, 0, 0, 0, 0, 0, 0, 0,
                       0, 0, 0, 0, d, c, b, a);
    }
    /**
     * @brief Constructor.
     * @return a vector of 4 signed chars: {a,a,a,a}.
     */
    FORCEINLINE svec( int8_t a) {
      if(__builtin_constant_p(a) && a == 0) {
        v = _mm_setzero_si128 ();
      } else {
        v = _mm_set1_epi8(a);
      }
    }
    /**
     * @brief operator [] to set or get the vector element specified by index.
     * @param index specifies the index of the element in the vector.
     */
    SUBSCRIPT_FUNC_DECL(int8_t);
    COUT_FUNC_CHAR_DECL(int8_t);

    VEC_CLASS_METHOD_DECL(int8_t);
    VEC_INT_CLASS_METHOD_DECL(int8_t, uint8_t);
};

/**
 * @brief data representation and operations on a vector of 4 unsigned chars.
 */
template<>
struct svec<4,uint8_t> {
  __m128i v;
    /**
     * @brief Default constructor
     * @return a vector of 4 undefined unsigned chars.
     */
    FORCEINLINE svec() { }
    /**
     * @brief For internal use only.
     * @param vv a __m128i value.
     * @return a signed char vector, whose value is from the vv.
     */
    FORCEINLINE svec(__m128i vv) : v(vv) {  }
    /**
     * @brief Constructor
     * @return a vector of 4 unsigned chars: {a,b,c,d}.
     */
    FORCEINLINE svec(uint8_t a, uint8_t b, uint8_t c, uint8_t d) {
      v = _mm_set_epi8(0, 0, 0, 0, 0, 0, 0, 0,
                       0, 0, 0, 0, d, c, b, a);
    }
    /**
     * @brief Constructor.
     * @param a an unsigned char value
     * @return a vector of 4 unsigned chars: {a,a,a,a}.
     */
    FORCEINLINE svec(uint8_t a){
      if(__builtin_constant_p(a) && a == 0) {
        v = _mm_setzero_si128 ();
      } else {
        v = _mm_set1_epi8(a);
      }
    }
    /**
     * @brief operator [] to set or get the vector element specified by index.
     * @param index specifies the index of the element in the vector.
     */
    SUBSCRIPT_FUNC_DECL(uint8_t);
    COUT_FUNC_CHAR_DECL(uint8_t);

    VEC_CLASS_METHOD_DECL(uint8_t);
    VEC_INT_CLASS_METHOD_DECL(uint8_t, uint8_t);
};

/**
 * @brief data representation and operations on a vector of 4 signed short.
 */
template <>
struct svec<4,int16_t> {
  __m128i v;
    /**
     * @brief Default constructor
     * @return a vector of 4 undefined signed short.
     */
    FORCEINLINE svec() { }
    /**
     * @brief For internal use only.
     * @param vv a __m128i.
     * @return a signed short vector, whose value is from the vv.
     */
    FORCEINLINE svec(__m128i vv) : v(vv) {  }
    /**
     * @brief Constructor.
     * @return a vector of 4 signed short: {a,b,c,d}.
     */
    FORCEINLINE svec(int16_t a, int16_t b, int16_t c, int16_t d) {
      v = _mm_set_epi16(0, 0, 0, 0, d, c, b, a);
    }
    /**
     * @brief Constructor.
     * @param a a signed short
     * @return a vector of 4 signed short: {a,a,a,a}.
     */
    FORCEINLINE svec( int16_t a) {
      if(__builtin_constant_p(a) && a == 0) {
        v = _mm_setzero_si128 ();
      } else {
        v = _mm_set1_epi16(a);
      }
    }
    /**
     * @brief operator [] to set or get the vector element specified by index.
     * @param index specifies the index of the element in the vector.
     */
    SUBSCRIPT_FUNC_DECL(int16_t);
    COUT_FUNC_DECL(int16_t);

    VEC_CLASS_METHOD_DECL(int16_t);
    VEC_INT_CLASS_METHOD_DECL(int16_t, uint16_t);

};

/**
 * @brief data representation and operations on a vector of 4 unsigned short.
 */
template <>
struct svec<4,uint16_t> {
  __m128i v;
    /**
     * @brief Default constructor
     * @return a vector of 4 undefined unsigned short.
     */
    FORCEINLINE svec() { }
    /**
     * @brief For internal use only.
     * @param vv a __m128i.
     * @return a signed short vector, whose value is from the vv.
     */
    FORCEINLINE svec(__m128i vv) : v(vv) {  }
    /** 
     * @brief Constructor.
     * @return a vector of 4 unsigned short: {a,b,c,d}.
     */
    FORCEINLINE svec(uint16_t a, uint16_t b, uint16_t c, uint16_t d) {
      v = _mm_set_epi16(0, 0, 0, 0, d, c, b, a);
    }
    /**
     * @brief Constructor.
     * @param a an unsigned short
     * @return a vector of 4 unsigned short: {a,a,a,a}.
     */
    FORCEINLINE svec(uint16_t a) {
      if(__builtin_constant_p(a) && a == 0) {
        v = _mm_setzero_si128 ();
      } else {
        v = _mm_set1_epi16(a);
      }
    }
    /**
     * @brief operator [] to set or get the vector element specified by index.
     * @param index specifies the index of the element in the vector.
     */
    SUBSCRIPT_FUNC_DECL(uint16_t);
    COUT_FUNC_DECL(uint16_t);

    VEC_CLASS_METHOD_DECL(uint16_t);
    VEC_INT_CLASS_METHOD_DECL(uint16_t, uint16_t);

};

/**
 * @brief data representation and operations on a vector of 4 signed int.
 */
template <>
struct svec<4,int32_t> {
  __m128i v;
    /**
     * @brief Default constructor
     * @return a vector of 4 undefined signed int.
     */
    FORCEINLINE svec() { }
    /**
     * @brief For internal use only.
     * @param vv a __m128i.
     * @return a signed int vector, whose value is from the vv.
     */
    FORCEINLINE svec(__m128i vv) : v(vv) {  }
    /** 
     * @brief Constructor.
     * @return a vector of 4 signed int: {a,b,c,d}.
     */
    FORCEINLINE svec(int a, int b, int c, int d) {
      v = _mm_set_epi32(d, c, b, a);
    }
    /**
     * @brief Constructor.
     * @param a a signed int
     * @return a vector of 4 signed int: {a,a,a,a}.
     */
    FORCEINLINE svec(int32_t a) {
      if(__builtin_constant_p(a) && a == 0) {
        v = _mm_setzero_si128 ();
      } else {
        v = _mm_set1_epi32(a);
      }
    }
    /**
     * @brief transform svec<4,int32_t> into _m128 float vector
     * @return _m128
     */
    FORCEINLINE operator __m128() const { return _mm_castsi128_ps(v); }
    /**
     * @brief operator [] to set or get the vector element specified by index.
     * @param index specifies the index of the element in the vector.
     */
    SUBSCRIPT_FUNC_DECL(int32_t);
    COUT_FUNC_DECL(int32_t);

    VEC_CLASS_METHOD_DECL(int32_t);
    VEC_INT_CLASS_METHOD_DECL(int32_t, uint32_t);

};

/**
 * @brief data representation and operations on a vector of 4 unsigned int.
 */
template <>
struct svec<4,uint32_t> {
  __m128i v;
    /**
     * @brief Default constructor
     * @return a vector of 4 undefined unsigned int.
     */
    FORCEINLINE svec() { }
    /**
     * @brief For internal use only.
     * @param vv a __m128i.
     * @return a signed int vector, whose value is from the vv.
     */
    FORCEINLINE svec(__m128i vv) : v(vv) {  }
    /** 
     * @brief Constructor.
     * @return a vector of 4 unsigned int: {a,b,c,d}.
     */
    FORCEINLINE svec(uint32_t a, uint32_t b, uint32_t c, uint32_t d) {
      v = _mm_set_epi32(d, c, b, a);
    }
    /**
     * @brief Constructor.
     * @param a an unsigned int
     * @return a vector of 4 unsigned int: {a,a,a,a}.
     */
    FORCEINLINE svec(uint32_t a) {
      if(__builtin_constant_p(a) && a == 0) {
        v = _mm_setzero_si128 ();
      } else {
        v = _mm_set1_epi32(a);
      }
    }
    /**
     * @brief transform svec<4,int32_t> into _m128 float vector
     * @return _m128
     */
    FORCEINLINE operator __m128() const { return _mm_castsi128_ps(v); }
    /**
     * @brief operator [] to set or get the vector element specified by index.
     * @param index specifies the index of the element in the vector.
     */
    SUBSCRIPT_FUNC_DECL(uint32_t);
    COUT_FUNC_DECL(uint32_t);

    VEC_CLASS_METHOD_DECL(uint32_t);
    VEC_INT_CLASS_METHOD_DECL(uint32_t, uint32_t);
};

/**
 * @brief data representation and operations on a vector of 4 signed long long.
 */
template <>
struct svec<4,int64_t> { 
  __m128i v[2];
    /**
     * @brief Default constructor,
     * @return a vector of 4 undefined signed long long.
     */
    FORCEINLINE svec() { }
    /**
     * @brief For internal use only. Construct svec<4,int64_t> with two _m128i objects
     * @return a signed long long vector, whose value is from the vv.
     */
    FORCEINLINE svec(__m128i a, __m128i b){
        v[0] = a;
        v[1] = b;
    }
    /** 
     * @brief Constructor.
     * @return a vector of 4 signed long long: {a,b,c,d}.
     */
    FORCEINLINE svec(int64_t a, int64_t b, int64_t c, int64_t d) {
      v[0] = _mm_set_epi32((b >> 32) & 0xffffffff, b & 0xffffffff,
                           (a >> 32) & 0xffffffff, a & 0xffffffff);
      v[1] = _mm_set_epi32((d >> 32) & 0xffffffff, d & 0xffffffff,
                           (c >> 32) & 0xffffffff, c & 0xffffffff);
    }
    /**
     * @brief Constructor.
     * @param a a signed long long
     * @return a vector of 4 signed long long: {a,a,a,a}.
     */
    FORCEINLINE svec( int64_t a) {
      if(__builtin_constant_p(a) && a == 0) {
        v[0] = v[1] = _mm_setzero_si128 ();
      } else {
        int a1 = (a >> 32) & 0xffffffff;
        int a0 = a & 0xffffffff;
        v[0] = v[1] = _mm_set_epi32(a1, a0, a1, a0);
      }
    }
    /**
     * @brief operator [] to set or get the vector element specified by index.
     * @param index specifies the index of the element in the vector.
     */
    SUBSCRIPT_FUNC_DECL(int64_t);
    COUT_FUNC_DECL(int64_t);

    VEC_CLASS_METHOD_DECL(int64_t);
    VEC_INT_CLASS_METHOD_DECL(int64_t, uint64_t);
};

/**
 * @brief data representation and operations on a vector of 4 unsigned long long.
 */
template <>
struct svec<4,uint64_t> {
  __m128i v[2];
    /**
     * @brief Default constructor
     * @return a vector of 4 undefined unsigned long long.
     */
    FORCEINLINE svec() { }
    /**
     * @brief For internal use only. Construct svec<4,int64_t> with two _m128i objects
     * @return a signed long long vector, whose value is from the vv.
     */
    FORCEINLINE svec(__m128i a, __m128i b){
        v[0] = a;
        v[1] = b;
    }
    /** 
     * @brief Constructor.
     * @return a vector of 4 unsigned long long: {a,b,c,d}.
     */
    FORCEINLINE svec(uint64_t a, uint64_t b, uint64_t c, uint64_t d) {
      v[0] = _mm_set_epi32((b >> 32) & 0xffffffff, b & 0xffffffff,
                           (a >> 32) & 0xffffffff, a & 0xffffffff);
      v[1] = _mm_set_epi32((d >> 32) & 0xffffffff, d & 0xffffffff,
                           (c >> 32) & 0xffffffff, c & 0xffffffff);
    }
    /**
     * @brief Constructor.
     * @param a an unsigned long long.
     * @return a vector of 4 unsigned long long: {a,a,a,a}.
     */
    FORCEINLINE svec( uint64_t a) {
      if(__builtin_constant_p(a) && a == 0) {
         v[0] = v[1] = _mm_setzero_si128 ();
      } else {
        int a1 = (a >> 32) & 0xffffffff;
        int a0 = a & 0xffffffff;
        v[0] = v[1] = _mm_set_epi32(a1, a0, a1, a0);
      }
    }
    /**
     * @brief operator [] to set or get the vector element specified by index.
     * @param index specifies the index of the element in the vector.
     */
    SUBSCRIPT_FUNC_DECL(uint64_t);
    COUT_FUNC_DECL(uint64_t);

    VEC_CLASS_METHOD_DECL(uint64_t);
    VEC_INT_CLASS_METHOD_DECL(uint64_t, uint64_t);
};

/**
 * @brief data representation and operations on a vector of 4 float.
 */
template<>
struct svec<4,float> {
  __m128 v;
    /**
     * @brief Default constructor
     * @return a vector of 4 undefined float.
     */
    FORCEINLINE svec() { }
    /**
     * @brief For internal use only.
     * @param vv a __m128.
     * @return a float vector, whose value is from the vv.
     */
    FORCEINLINE svec(__m128 vv) : v(vv) {  }
    /** 
     * @brief Constructor.
     * @return a vector of 4 float: {a,b,c,d}.
     */
    FORCEINLINE svec(float a, float b, float c, float d) {
      v = _mm_set_ps(d, c, b, a);
    }
    /**
     * @brief Constructor.
     * @param a a float
     * @return a vector of 4 floats: {a,a,a,a}.
     */
    FORCEINLINE svec( float a) {
      if(__builtin_constant_p(a) && a == 0) {
        v = _mm_setzero_ps();
      } else {
        v = _mm_set1_ps(a);
      }
    }
    /**
     * @brief operator [] to set or get the vector element specified by index.
     * @param index specifies the index of the element in the vector.
     */
    SUBSCRIPT_FUNC_DECL(float);
    COUT_FUNC_DECL(float);

    VEC_CLASS_METHOD_DECL(float);
    VEC_FLOAT_CLASS_METHOD_DECL(float);
};

/**
 * @brief data representation and operations on a vector of 4 double.
 */
template<>
struct svec<4,double> {
  __m128d v[2];
    /**
     * @brief Default constructor
     * @return a vector of 4 undefined double.
     */
    FORCEINLINE svec() { }
    /**
     * @brief For internal use only. Construct svec<4,double> with two __vector double values
     * @return a double vector, whose value is from a and b.
     */
    FORCEINLINE svec(__m128d a, __m128d b){
        v[0] = a;
        v[1] = b;
    }
    /** 
     * @brief Constructor.
     * @return a vector of 4 double: {a,b,c,d}.
     */
    FORCEINLINE svec(double a, double b, double c, double d) {
      v[0] = _mm_set_pd(b, a);
      v[1] = _mm_set_pd(d, c);
    }
    /**
     * @brief Constructor.
     * @param a a double
     * @return a vector of 4 doubles: {a,a,a,a}.
     */
    FORCEINLINE svec( double a) {
    if (__builtin_constant_p(a) && a == 0) {
      v[0] = v[1] = _mm_setzero_pd();
    } else {
      v[0] = v[1] = _mm_set1_pd(a);
    }
    }
    /**
     * @brief operator [] to set or get the vector element specified by index.
     * @param index specifies the index of the element in the vector.
     */
    SUBSCRIPT_FUNC_DECL(double);
    COUT_FUNC_DECL(double);

    VEC_CLASS_METHOD_DECL(double);
    VEC_FLOAT_CLASS_METHOD_DECL(double);
};


//////////////////////////////////////////////////////////////
//
// Data operation interfaces
//
//////////////////////////////////////////////////////////////

//
//// 1. Extract / Insert
//

/**
 * @brief macros for svec's insert extract method implementation
 * The implementation is based on vector type's subscript operator
 */
#define INSERT_EXTRACT_SSE(STYPE)                               \
  static FORCEINLINE STYPE svec_extract(svec<LANES,STYPE> v, int index) {    \
    return ((STYPE*)&v)[index];                      \
  }                                          \
  static FORCEINLINE void svec_insert(svec<LANES,STYPE> *v, int index, STYPE val) { \
    ((STYPE*)v)[index] = val;                      \
  }

#define INSERT_EXTRACT_SSEOPT(STYPE, FUNC)                               \
  static FORCEINLINE STYPE svec_extract(svec<LANES,STYPE> v, int index) {    \
    if(__builtin_constant_p(index) && index >=0 && index < 4) { \
      return (STYPE)_mm_extract_##FUNC(v.v, index);                \
    } else { \
      return ((STYPE*)&v)[index];  \
    } \
  }                                     \
  static FORCEINLINE void svec_insert(svec<LANES,STYPE> *v, int index, STYPE val) { \
    if(__builtin_constant_p(index) && index >=0 && index < 4) { \
      v->v = _mm_insert_##FUNC(v->v, val, index);                      \
    } else {\
      ((STYPE*)v)[index] = val;               \
    } \
  }

#define INSERT_EXTRACT_SSEOPT64(STYPE, FUNC)                               \
  static FORCEINLINE STYPE svec_extract(svec<LANES,STYPE> v, int index) {    \
    if(__builtin_constant_p(index) && index >=0 && index < 4) { \
      return (STYPE)_mm_extract_##FUNC(v.v[index>>1], index%2);    \
    } else { \
      return ((STYPE*)&v)[index];  \
    } \
  }                                          \
  static FORCEINLINE void svec_insert(svec<LANES,STYPE> *v, int index, STYPE val) { \
    if(__builtin_constant_p(index) && index >=0 && index < 4) { \
      v->v[index>>1] = _mm_insert_##FUNC(v->v[index>>1], val, index%2);      \
    } else { \
      ((STYPE*)v)[index] = val;               \
    } \
  }

//i1 use different approach
static FORCEINLINE uint32_t svec_extract(svec<4,bool> v, int index) {
  if(__builtin_constant_p(index) && index >=0 && index < 4) {
    return _mm_extract_epi32(_mm_castps_si128(v.v), index);
  } else
  {
    return ((uint32_t*)&v)[index];
  }

}
static FORCEINLINE void svec_insert(svec<4,bool> *v, int index, uint32_t val) {
  if(__builtin_constant_p(index) && index >=0 && index < 4) {
    v->v = _mm_castsi128_ps(_mm_insert_epi32(_mm_castps_si128(v->v), val ? -1 : 0, index));
  } else {
    ((uint32_t *)v)[index] = val ? -1 : 0;
  }

}
INSERT_EXTRACT_SSEOPT(int8_t, epi8);
INSERT_EXTRACT_SSEOPT(uint8_t, epi8);
INSERT_EXTRACT_SSEOPT(int16_t, epi16);
INSERT_EXTRACT_SSEOPT(uint16_t, epi16);
INSERT_EXTRACT_SSEOPT(int32_t, epi32);
INSERT_EXTRACT_SSEOPT(uint32_t, epi32);
#ifdef __x86_64__
INSERT_EXTRACT_SSEOPT64(int64_t, epi64);
INSERT_EXTRACT_SSEOPT64(uint64_t, epi64);
#else
INSERT_EXTRACT_SSE(int64_t);
INSERT_EXTRACT_SSE(uint64_t);
#endif
INSERT_EXTRACT_SSE(float); //no intrinsics to insert/extract
INSERT_EXTRACT_SSE(double); //no intrinsics to insert/extract

// 1. Load / Store
/**
 * @brief load a new vector from a pointer
 * @param[in] p load address
 * \note p does not have to be aligned
 * @return a new vector loaded from p
 */
static FORCEINLINE svec<4,bool> svec_load(const svec<4,bool> *p) {
  return svec<4,bool>(_mm_loadu_ps((float *)(&p->v)));

}

/**
 * @brief store a vector to an address
 * @param[in] p store address
 * \note p does not have to be aligned
 * @param[in] v vector to be stored
 */
static FORCEINLINE void svec_store(svec<4,bool> *p, svec<4,bool> v) {
  _mm_storeu_ps((float *)(&p->v), v.v);
}
/**
 * @brief load a new vector from a pointer
 * @param[in] p load address
 * \note p does not have to be aligned
 * @return a new vector loaded from p
 */
static FORCEINLINE svec<4,int8_t> svec_load(const svec<4,int8_t> *p) {
  int8_t *ptr = (int8_t *)(&p->v);
  return svec<4,int8_t>(ptr[0], ptr[1], ptr[2], ptr[3]);
}

/**
 * @brief store a vector to an address
 * @param[in] p store address
 * \note p does not have to be aligned
 * @param[in] v vector to be stored
 */
static FORCEINLINE void svec_store(svec<4,int8_t> *p, svec<4,int8_t> v) {
  int8_t *ptr = (int8_t *)(&p->v);
  ptr[0] = _mm_extract_epi8(v.v, 0);
  ptr[1] = _mm_extract_epi8(v.v, 1);
  ptr[2] = _mm_extract_epi8(v.v, 2);
  ptr[3] = _mm_extract_epi8(v.v, 3);
}
/**
 * @brief load a new vector from a pointer
 * @param[in] p load address
 * \note p does not have to be aligned
 * @return a new vector loaded from p
 */
static FORCEINLINE svec<4,uint8_t> svec_load(const svec<4,uint8_t> *p) {
  uint8_t *ptr = (uint8_t *)(&p->v);
  return svec<4,uint8_t>(ptr[0], ptr[1], ptr[2], ptr[3]);
}

/**
 * @brief store a vector to an address
 * @param[in] p store address
 * \note p does not have to be aligned
 * @param[in] v vector to be stored
 */
static FORCEINLINE void svec_store(svec<4,uint8_t> *p, svec<4,uint8_t> v) {
  uint8_t *ptr = (uint8_t *)(&p->v);
  ptr[0] = _mm_extract_epi8(v.v, 0);
  ptr[1] = _mm_extract_epi8(v.v, 1);
  ptr[2] = _mm_extract_epi8(v.v, 2);
  ptr[3] = _mm_extract_epi8(v.v, 3);
}
/**
 * @brief load a new vector from a pointer
 * @param[in] p load address
 * \note p does not have to be aligned
 * @return a new vector loaded from p
 */
static FORCEINLINE svec<4,int16_t> svec_load(const svec<4,int16_t> *p) {
  int16_t *ptr = (int16_t *)(&p->v);
  return svec<4,int16_t>(ptr[0], ptr[1], ptr[2], ptr[3]);
}

/**
 * @brief store a vector to an address
 * @param[in] p store address
 * \note p does not have to be aligned
 * @param[in] v vector to be stored
 */
static FORCEINLINE void svec_store(svec<4,int16_t> *p, svec<4,int16_t> v) {
  int16_t *ptr = (int16_t *)(&p->v);
  ptr[0] = _mm_extract_epi16(v.v, 0);
  ptr[1] = _mm_extract_epi16(v.v, 1);
  ptr[2] = _mm_extract_epi16(v.v, 2);
  ptr[3] = _mm_extract_epi16(v.v, 3);
}
/**
 * @brief load a new vector from a pointer
 * @param[in] p load address
 * \note p does not have to be aligned
 * @return a new vector loaded from p
 */
static FORCEINLINE svec<4,uint16_t> svec_load(const svec<4,uint16_t> *p) {
  uint16_t *ptr = (uint16_t *)(&p->v);
  return svec<4,uint16_t>(ptr[0], ptr[1], ptr[2], ptr[3]);
}

/**
 * @brief store a vector to an address
 * @param[in] p store address
 * \note p does not have to be aligned
 * @param[in] v vector to be stored
 */
static FORCEINLINE void svec_store(svec<4,uint16_t> *p, svec<4,uint16_t> v) {
  uint16_t *ptr = (uint16_t *)(&p->v);
  ptr[0] = _mm_extract_epi16(v.v, 0);
  ptr[1] = _mm_extract_epi16(v.v, 1);
  ptr[2] = _mm_extract_epi16(v.v, 2);
  ptr[3] = _mm_extract_epi16(v.v, 3);
}
/**
 * @brief load a new vector from a pointer
 * @param[in] p load address
 * \note p does not have to be aligned
 * @return a new vector loaded from p
 */
static FORCEINLINE svec<4,int32_t> svec_load(const svec<4,int32_t> *p) {
  return svec<4,int32_t>(_mm_loadu_si128((__m128i *)(&p->v)));
}

/**
 * @brief store a vector to an address
 * @param[in] p store address
 * \note p does not have to be aligned
 * @param[in] v vector to be stored
 */
static FORCEINLINE void svec_store(svec<4,int32_t> *p, svec<4,int32_t> v) {
  _mm_storeu_si128((__m128i *)(&p->v), v.v);
}

/**
 * @brief load a new vector from a pointer
 * @param[in] p load address
 * \note p does not have to be aligned
 * @return a new vector loaded from p
 */
static FORCEINLINE svec<4,uint32_t> svec_load(const svec<4,uint32_t> *p) {
  return svec<4,uint32_t>(_mm_loadu_si128((__m128i *)(&p->v)));
}

/**
 * @brief store a vector to an address
 * @param[in] p store address
 * \note p does not have to be aligned
 * @param[in] v vector to be stored
 */
static FORCEINLINE void svec_store(svec<4,uint32_t> *p, svec<4,uint32_t> v) {
  _mm_storeu_si128((__m128i *)(&p->v), v.v);
}
/**
 * @brief load a new vector from a pointer
 * @param[in] p load address
 * \note p does not have to be aligned
 * @return a new vector loaded from p
 */
static FORCEINLINE svec<4,int64_t> svec_load(const svec<4,int64_t> *p) {
  return svec<4,int64_t>(_mm_loadu_si128((__m128i *)(&p->v[0])),
                   _mm_loadu_si128((__m128i *)(&p->v[1])));
}

/**
 * @brief store a vector to an address
 * @param[in] p store address
 * \note p does not have to be aligned
 * @param[in] v vector to be stored
 */
static FORCEINLINE void svec_store(svec<4,int64_t> *p, svec<4,int64_t> v) {
  _mm_storeu_si128((__m128i *)(&p->v[0]), v.v[0]);
  _mm_storeu_si128((__m128i *)(&p->v[1]), v.v[1]);
}

/**
 * @brief load a new vector from a pointer
 * @param[in] p load address
 * \note p does not have to be aligned
 * @return a new vector loaded from p
 */
static FORCEINLINE svec<4,uint64_t> svec_load(const svec<4,uint64_t> *p) {
  return svec<4,uint64_t>(_mm_loadu_si128((__m128i *)(&p->v[0])),
                   _mm_loadu_si128((__m128i *)(&p->v[1])));
}
/**
 * @brief store a vector to an address
 * @param[in] p store address
 * \note p does not have to be aligned
 * @param[in] v vector to be stored
 */
static FORCEINLINE void svec_store(svec<4,uint64_t> *p, svec<4,uint64_t> v) {
  _mm_storeu_si128((__m128i *)(&p->v[0]), v.v[0]);
  _mm_storeu_si128((__m128i *)(&p->v[1]), v.v[1]);
}
/**
 * @brief load a new vector from a pointer
 * @param[in] p load address
 * \note p does not have to be aligned
 * @return a new vector loaded from p
 */
static FORCEINLINE svec<4,float> svec_load(const svec<4,float> *p) {
  return svec<4,float>(_mm_loadu_ps((float *)(&p->v)));
}

/**
 * @brief store a vector to an address
 * @param[in] p store address
 * \note p does not have to be aligned
 * @param[in] v vector to be stored
 */
static FORCEINLINE void svec_store(svec<4,float> *p, svec<4,float> v) {
  _mm_storeu_ps((float *)(&p->v), v.v);
}

/**
 * @brief load a new vector from a pointer
 * @param[in] p load address
 * \note p does not have to be aligned
 * @return a new vector loaded from p
 */
static FORCEINLINE svec<4,double> svec_load(const svec<4,double> *p) {
  return svec<4,double>(_mm_loadu_pd((double *)(&p->v[0])),
                 _mm_loadu_pd((double *)(&p->v[1])));
}

/**
 * @brief store a vector to an address
 * @param[in] p store address
 * \note p does not have to be aligned
 * @param[in] v vector to be stored
 */
static FORCEINLINE void svec_store(svec<4,double> *p, svec<4,double> v) {
  _mm_storeu_pd((double *)(&p->v[0]), v.v[0]);
  _mm_storeu_pd((double *)(&p->v[1]), v.v[1]);
}

// 3. Select


/**
 * @brief construct c by selecting elements from two input vectors according to the mask
 * @param mask the mask vector. True: select elements from a; False: select elements from b.
 * @param a the 1st vector. The elements will be selected if the corresponding mask element is true.
 * @param b the 2nd vector. The elements will be selected if the corresponding mask element is true.
 * @return A new vector whose elements is selected from a or b
 */
FORCEINLINE svec<4,bool> svec_select(svec<4,bool> mask, svec<4,bool> a, svec<4,bool> b) {
  return _mm_blendv_ps(b.v, a.v, mask.v);
}

/**
 * @brief select of svec<4,int8_t> vectors by a mask vector
 * see svec_select(svec<4,bool> mask, svec<4,bool> a, svec<4,bool> b)
 */
FORCEINLINE svec<4,int8_t> svec_select(svec<4,bool> mask, svec<4,int8_t> a, svec<4,int8_t> b) {
  INC_STATS_NAME(STATS_OTHER_SLOW, 1, "select i8");
  return svec<4,int8_t>((_mm_extract_ps(mask.v, 0) != 0) ? _mm_extract_epi8(a.v, 0) :
                                                      _mm_extract_epi8(b.v, 0),
                   (_mm_extract_ps(mask.v, 1) != 0) ? _mm_extract_epi8(a.v, 1) :
                                                      _mm_extract_epi8(b.v, 1),
                   (_mm_extract_ps(mask.v, 2) != 0) ? _mm_extract_epi8(a.v, 2) :
                                                      _mm_extract_epi8(b.v, 2),
                   (_mm_extract_ps(mask.v, 3) != 0) ? _mm_extract_epi8(a.v, 3) :
                                                      _mm_extract_epi8(b.v, 3));
}

/**
 * @brief select of svec<4,uint8_t> vectors by a mask vector
 * see svec_select(svec<4,bool> mask, svec<4,bool> a, svec<4,bool> b)
 */
FORCEINLINE svec<4,uint8_t> svec_select(svec<4,bool> mask, svec<4,uint8_t> a, svec<4,uint8_t> b) {
  INC_STATS_NAME(STATS_OTHER_SLOW, 1, "select u8");
  return svec<4,uint8_t>((_mm_extract_ps(mask.v, 0) != 0) ? _mm_extract_epi8(a.v, 0) :
                                                      _mm_extract_epi8(b.v, 0),
                   (_mm_extract_ps(mask.v, 1) != 0) ? _mm_extract_epi8(a.v, 1) :
                                                      _mm_extract_epi8(b.v, 1),
                   (_mm_extract_ps(mask.v, 2) != 0) ? _mm_extract_epi8(a.v, 2) :
                                                      _mm_extract_epi8(b.v, 2),
                   (_mm_extract_ps(mask.v, 3) != 0) ? _mm_extract_epi8(a.v, 3) :
                                                      _mm_extract_epi8(b.v, 3));
}

/**
 * @brief select of svec<4,int16_t> vectors by a mask vector
 * see svec_select(svec<4,bool> mask, svec<4,bool> a, svec<4,bool> b)
 */
FORCEINLINE svec<4,int16_t> svec_select(svec<4,bool> mask, svec<4,int16_t> a, svec<4,int16_t> b) {
    INC_STATS_NAME(STATS_OTHER_SLOW, 1, "select i16");
    return svec<4,int16_t>((_mm_extract_ps(mask.v, 0) != 0) ? _mm_extract_epi16(a.v, 0) :
                                                         _mm_extract_epi16(b.v, 0),
                      (_mm_extract_ps(mask.v, 1) != 0) ? _mm_extract_epi16(a.v, 1) :
                                                         _mm_extract_epi16(b.v, 1),
                      (_mm_extract_ps(mask.v, 2) != 0) ? _mm_extract_epi16(a.v, 2) :
                                                         _mm_extract_epi16(b.v, 2),
                      (_mm_extract_ps(mask.v, 3) != 0) ? _mm_extract_epi16(a.v, 3) :
                                                         _mm_extract_epi16(b.v, 3));
}

/**
 * @brief select of svec<4,uint16_t> vectors by a mask vector
 * see svec_select(svec<4,bool> mask, svec<4,bool> a, svec<4,bool> b)
 */
FORCEINLINE svec<4,uint16_t> svec_select(svec<4,bool> mask, svec<4,uint16_t> a, svec<4,uint16_t> b) {
    INC_STATS_NAME(STATS_OTHER_SLOW, 1, "select u16");
    return svec<4,uint16_t>((_mm_extract_ps(mask.v, 0) != 0) ? _mm_extract_epi16(a.v, 0) :
                                                         _mm_extract_epi16(b.v, 0),
                      (_mm_extract_ps(mask.v, 1) != 0) ? _mm_extract_epi16(a.v, 1) :
                                                         _mm_extract_epi16(b.v, 1),
                      (_mm_extract_ps(mask.v, 2) != 0) ? _mm_extract_epi16(a.v, 2) :
                                                         _mm_extract_epi16(b.v, 2),
                      (_mm_extract_ps(mask.v, 3) != 0) ? _mm_extract_epi16(a.v, 3) :
                                                         _mm_extract_epi16(b.v, 3));
}

/**
 * @brief select of svec<4,int32_t> vectors by a mask vector
 * see svec_select(svec<4,bool> mask, svec<4,bool> a, svec<4,bool> b)
 */
FORCEINLINE svec<4,int32_t> svec_select(svec<4,bool> mask, svec<4,int32_t> a, svec<4,int32_t> b) {
  return _mm_castps_si128(_mm_blendv_ps(_mm_castsi128_ps(b.v),
                                        _mm_castsi128_ps(a.v), mask.v));
}

/**
 * @brief select of svec<4,uint32_t> vectors by a mask vector
 * see svec_select(svec<4,bool> mask, svec<4,bool> a, svec<4,bool> b)
 */
FORCEINLINE svec<4,uint32_t> svec_select(svec<4,bool> mask, svec<4,uint32_t> a, svec<4,uint32_t> b) {
  return _mm_castps_si128(_mm_blendv_ps(_mm_castsi128_ps(b.v),
                                        _mm_castsi128_ps(a.v), mask.v));
}

/**
 * @brief select of svec<4,int64_t> vectors by a mask vector
 * see svec_select(svec<4,bool> mask, svec<4,bool> a, svec<4,bool> b)
 */
FORCEINLINE svec<4,int64_t> svec_select(svec<4,bool> mask, svec<4,int64_t> a, svec<4,int64_t> b) {
  __m128 m0 = _mm_shuffle_ps(mask.v, mask.v, _MM_SHUFFLE(1, 1, 0, 0));
  __m128 m1 = _mm_shuffle_ps(mask.v, mask.v, _MM_SHUFFLE(3, 3, 2, 2));
  __m128d m0d = _mm_castps_pd(m0);
  __m128d m1d = _mm_castps_pd(m1);
  __m128d r0 = _mm_blendv_pd(_mm_castsi128_pd(b.v[0]), _mm_castsi128_pd(a.v[0]), m0d);
  __m128d r1 = _mm_blendv_pd(_mm_castsi128_pd(b.v[1]), _mm_castsi128_pd(a.v[1]), m1d);
  return svec<4,int64_t>(_mm_castpd_si128(r0), _mm_castpd_si128(r1));
}

/**
 * @brief select of svec<4,uint64_t> vectors by a mask vector
 * see svec_select(svec<4,bool> mask, svec<4,bool> a, svec<4,bool> b)
 */
FORCEINLINE svec<4,uint64_t> svec_select(svec<4,bool> mask, svec<4,uint64_t> a, svec<4,uint64_t> b) {
  __m128 m0 = _mm_shuffle_ps(mask.v, mask.v, _MM_SHUFFLE(1, 1, 0, 0));
  __m128 m1 = _mm_shuffle_ps(mask.v, mask.v, _MM_SHUFFLE(3, 3, 2, 2));
  __m128d m0d = _mm_castps_pd(m0);
  __m128d m1d = _mm_castps_pd(m1);
  __m128d r0 = _mm_blendv_pd(_mm_castsi128_pd(b.v[0]), _mm_castsi128_pd(a.v[0]), m0d);
  __m128d r1 = _mm_blendv_pd(_mm_castsi128_pd(b.v[1]), _mm_castsi128_pd(a.v[1]), m1d);
  return svec<4,uint64_t>(_mm_castpd_si128(r0), _mm_castpd_si128(r1));
}

/**
 * @brief select of svec<4,float> vectors by a mask vector
 * see svec_select(svec<4,bool> mask, svec<4,bool> a, svec<4,bool> b)
 */
FORCEINLINE svec<4,float> svec_select(svec<4,bool> mask, svec<4,float> a, svec<4,float> b) {
  return _mm_blendv_ps(b.v, a.v, mask.v);
}

/**
 * @brief select of svec<4,double> vectors by a mask vector
 * see svec_select(svec<4,bool> mask, svec<4,bool> a, svec<4,bool> b)
 */
FORCEINLINE svec<4,double> svec_select(svec<4,bool> mask, svec<4,double> a, svec<4,double> b) {
  __m128 m0 = _mm_shuffle_ps(mask.v, mask.v, _MM_SHUFFLE(1, 1, 0, 0));
  __m128 m1 = _mm_shuffle_ps(mask.v, mask.v, _MM_SHUFFLE(3, 3, 2, 2));
  __m128d m0d = _mm_castps_pd(m0);
  __m128d m1d = _mm_castps_pd(m1);
  __m128d r0 = _mm_blendv_pd(b.v[0], a.v[0], m0d);
  __m128d r1 = _mm_blendv_pd(b.v[1], a.v[1], m1d);
  return svec<4,double>(r0, r1);
}

SELECT_BOOLCOND(bool);
SELECT_BOOLCOND(int8_t);
SELECT_BOOLCOND(uint8_t);
SELECT_BOOLCOND(int16_t);
SELECT_BOOLCOND(uint16_t);
SELECT_BOOLCOND(int32_t);
SELECT_BOOLCOND(uint32_t);
SELECT_BOOLCOND(int64_t);
SELECT_BOOLCOND(uint64_t);
SELECT_BOOLCOND(float);
SELECT_BOOLCOND(double);

// 4. broadcast/rotate/shuffle/smear/setzero
static FORCEINLINE svec<4,int8_t> svec_broadcast(svec<4,int8_t> v, int index) {
  return _mm_set1_epi8(v[index]);
}
static FORCEINLINE svec<4,uint8_t> svec_broadcast(svec<4,uint8_t> v, int index) {
  return _mm_set1_epi8(v[index]);
}
static FORCEINLINE svec<4,int16_t> svec_broadcast(svec<4,int16_t> v, int index) {
  return _mm_set1_epi16(v[index]);
}
static FORCEINLINE svec<4,uint16_t> svec_broadcast(svec<4,uint16_t> v, int index) {
  return _mm_set1_epi16(v[index]);
}
static FORCEINLINE svec<4,int32_t> svec_broadcast(svec<4,int32_t> v, int index) {
  return _mm_set1_epi32(v[index]);
}
static FORCEINLINE svec<4,uint32_t> svec_broadcast(svec<4,uint32_t> v, int index) {
  return _mm_set1_epi32(v[index]);
}

static FORCEINLINE svec<4,int64_t> svec_broadcast(svec<4,int64_t> v, int index) {
  int64_t val = v[index];
  return svec<4,int64_t>(val);
}
static FORCEINLINE svec<4,uint64_t> svec_broadcast(svec<4,uint64_t> v, int index) {
  uint64_t val = v[index];
  return svec<4,uint64_t>(val);
}

static FORCEINLINE svec<4,float> svec_broadcast(svec<4,float> v, int index) {
  return _mm_set1_ps(v[index]);
}
static FORCEINLINE svec<4,double> svec_broadcast(svec<4,double> v, int index) {
  return svec<4,double>(_mm_set1_pd(v[index]),
                  _mm_set1_pd(v[index]));
}


ROTATE_L4(int8_t);
ROTATE_L4(uint8_t);
ROTATE_L4(int16_t);
ROTATE_L4(uint16_t);
ROTATE_L4(int32_t);
ROTATE_L4(uint32_t);
ROTATE_L4(int64_t);
ROTATE_L4(uint64_t);
ROTATE_L4(float);
ROTATE_L4(double);

SHUFFLES_L4(int8_t);
SHUFFLES_L4(uint8_t);
SHUFFLES_L4(int16_t);
SHUFFLES_L4(uint16_t);
SHUFFLES_L4(int32_t);
SHUFFLES_L4(uint32_t);
SHUFFLES_L4(int64_t);
SHUFFLES_L4(uint64_t);
SHUFFLES_L4(float);
SHUFFLES_L4(double);


//load const
#define LOAD_CONST_SSE(STYPE) \
template <class RetVecType> static RetVecType svec_load_const(const STYPE* p); \
template<> \
  FORCEINLINE svec<LANES,STYPE> svec_load_const<svec<LANES,STYPE> >(const STYPE* p) { \
    return svec<LANES,STYPE>(*p); \
} \
template <class RetVecType> static RetVecType svec_load_and_splat(STYPE* p); \
template<> \
FORCEINLINE svec<LANES,STYPE> svec_load_and_splat<svec<LANES,STYPE> >(STYPE* p) { \
  return svec<LANES,STYPE>(*p);\
}

LOAD_CONST_SSE(int8_t);
LOAD_CONST_SSE(uint8_t);
LOAD_CONST_SSE(int16_t);
LOAD_CONST_SSE(uint16_t);
LOAD_CONST_SSE(int32_t);
LOAD_CONST_SSE(uint32_t);
LOAD_CONST_SSE(int64_t);
LOAD_CONST_SSE(uint64_t);
LOAD_CONST_SSE(float);
LOAD_CONST_SSE(double);


// 5. Gather / Scatter
/**
 * including gather general/gather base offsets, scatter general, scatter base offsets
 *
 * Here, we will define a special compile time dependent type for ptrs vector
 *
 */
/**
 * @brief data representation and operations on a vector of 4 pointers.
 * This is only used in gather and scatter.
 * @note In 32bit platform, svec<4,void*> extends svec<4,uint32_t>, while in 64bit platform, svec<4,void*> extends svec<4,uint64_t>.
 * @see gather and scatter
 */
#if defined(__x86_64__) || defined(__PPC64__)
template <>
  struct svec<4,void*> : public svec<4,uint64_t> {
    /**
     * @brief Constructor.
     * @return a vector of 4 pointers: {p10, p1, p2, p3}.
     */
    FORCEINLINE svec(void* p0, void* p1, void* p2, void* p3):
        svec<4,uint64_t>((uint64_t)(p0),(uint64_t)(p1),(uint64_t)(p2),(uint64_t)(p3)){}
};
#else // 32-bit
template <>
  struct svec<4,void*>: public svec<4,uint32_t>{
    /**
     * @brief Constructor.
     * @return a vector of 4 pointers: {p0, p1, p2, p3}.
     */
    FORCEINLINE svec(void* p0, void* p1, void* p2, void* p3):
        svec<4,uint32_t>((uint32_t)(p0),(uint32_t)(p1),(uint32_t)(p2),(uint32_t)(p3)){}
};
#endif // __PPC64__

#ifndef DOXYGEN_SHOULD_SKIP_THIS //not want generate svec_gather*/svec_scatter methods

template <class RetVecType> static RetVecType svec_gather(svec<4,uint32_t> ptrs, svec<4,bool> mask);

GATHER_GENERAL_L4(int8_t, uint32_t);
GATHER_GENERAL_L4(uint8_t, uint32_t);
GATHER_GENERAL_L4(int16_t, uint32_t);
GATHER_GENERAL_L4(uint16_t, uint32_t);
//GATHER_GENERAL_L4(int32_t, uint32_t);
template<>
FORCEINLINE svec<4,int32_t> svec_gather<svec<4,int32_t> >(svec<4,uint32_t> ptrs, svec<4,bool> mask) {
  svec<4,int32_t> ret;
  if(svec_extract(mask,0)) { svec_insert(&ret, 0, *((int32_t*)svec_extract(ptrs,0)));}
  if(svec_extract(mask,1)) { svec_insert(&ret, 1, *((int32_t*)svec_extract(ptrs,1)));}
  if(svec_extract(mask,2)) { svec_insert(&ret, 2, *((int32_t*)svec_extract(ptrs,2)));}
  if(svec_extract(mask,3)) { svec_insert(&ret, 3, *((int32_t*)svec_extract(ptrs,3)));}
  INC_STATS_NAME(STATS_GATHER_SLOW,1, "Gather general");
  return ret;
}
//GATHER_GENERAL_L4(uint32_t, uint32_t);
template<>
FORCEINLINE svec<4,uint32_t> svec_gather<svec<4,uint32_t> >(svec<4,uint32_t> ptrs, svec<4,bool> mask) {
  return svec<4,uint32_t>(svec_gather<svec<4,int32_t> >(ptrs, mask).v);
}

GATHER_GENERAL_L4(int64_t, uint32_t);
GATHER_GENERAL_L4(uint64_t, uint32_t);
//GATHER_GENERAL_L4(float, uint32_t);
template<>
FORCEINLINE svec<4,float> svec_gather<svec<4,float> >(svec<4,uint32_t> ptrs, svec<4,bool> mask) {
  return svec<4,float>(_mm_castsi128_ps(svec_gather<svec<4,int32_t> >(ptrs, mask).v));
}
GATHER_GENERAL_L4(double, uint32_t);

template <class RetVecType> static RetVecType svec_gather(svec<4,uint64_t> ptrs, svec<4,bool> mask);
GATHER_GENERAL_L4(int8_t, uint64_t);
GATHER_GENERAL_L4(int16_t, uint64_t);
GATHER_GENERAL_L4(uint16_t, uint64_t);
GATHER_GENERAL_L4(uint8_t, uint64_t);
//GATHER_GENERAL_L4(int32_t, uint64_t);
template<>
FORCEINLINE svec<4,int32_t> svec_gather<svec<4,int32_t> >(svec<4,uint64_t> ptrs, svec<4,bool> mask) {
  svec<4,int32_t> ret;
  if(svec_extract(mask,0)) { svec_insert(&ret, 0, *((int32_t*)svec_extract(ptrs,0)));}
  if(svec_extract(mask,1)) { svec_insert(&ret, 1, *((int32_t*)svec_extract(ptrs,1)));}
  if(svec_extract(mask,2)) { svec_insert(&ret, 2, *((int32_t*)svec_extract(ptrs,2)));}
  if(svec_extract(mask,3)) { svec_insert(&ret, 3, *((int32_t*)svec_extract(ptrs,3)));}
  INC_STATS_NAME(STATS_GATHER_SLOW,1, "Gather general");
  return ret;
}
//GATHER_GENERAL_L4(svec<4,uint32_t>, uint32_t, svec<4,uint64_t>, svec<4,bool>);
template<>
FORCEINLINE svec<4,uint32_t> svec_gather<svec<4,uint32_t> >(svec<4,uint64_t> ptrs, svec<4,bool> mask) {
  return svec<4,uint32_t>(svec_gather<svec<4,int32_t> >(ptrs, mask).v);
}
GATHER_GENERAL_L4(int64_t, uint64_t);
GATHER_GENERAL_L4(uint64_t, uint64_t);
//GATHER_GENERAL_L4(float, uint64_t);
template<>
FORCEINLINE svec<4,float> svec_gather<svec<4,float> >(svec<4,uint64_t> ptrs, svec<4,bool> mask) {
  return svec<4,float>(_mm_castsi128_ps(svec_gather<svec<4,int32_t> >(ptrs, mask).v));
}
GATHER_GENERAL_L4(double, uint64_t);



GATHER_BASE_OFFSETS_L4(int8_t, int32_t);
GATHER_BASE_OFFSETS_L4(uint8_t, int32_t);
GATHER_BASE_OFFSETS_L4(int16_t, int32_t);
GATHER_BASE_OFFSETS_L4(uint16_t, int32_t);
GATHER_BASE_OFFSETS_L4(int32_t, int32_t);
GATHER_BASE_OFFSETS_L4(uint32_t, int32_t);
GATHER_BASE_OFFSETS_L4(int64_t, int32_t);
GATHER_BASE_OFFSETS_L4(uint64_t, int32_t);
GATHER_BASE_OFFSETS_L4(float, int32_t);
GATHER_BASE_OFFSETS_L4(double, int32_t);


GATHER_BASE_OFFSETS_L4(int8_t, int64_t);
GATHER_BASE_OFFSETS_L4(uint8_t, int64_t);
GATHER_BASE_OFFSETS_L4(int16_t, int64_t);
GATHER_BASE_OFFSETS_L4(uint16_t, int64_t);
GATHER_BASE_OFFSETS_L4(int32_t, int64_t);
GATHER_BASE_OFFSETS_L4(uint32_t, int64_t);
GATHER_BASE_OFFSETS_L4(int64_t, int64_t);
GATHER_BASE_OFFSETS_L4(uint64_t, int64_t);
GATHER_BASE_OFFSETS_L4(float, int64_t);
GATHER_BASE_OFFSETS_L4(double, int64_t);

GATHER_STRIDE_L4(int8_t, int32_t);
GATHER_STRIDE_L4(int8_t, int64_t);
GATHER_STRIDE_L4(uint8_t, int32_t);
GATHER_STRIDE_L4(uint8_t, int64_t);
GATHER_STRIDE_L4(int16_t, int32_t);
GATHER_STRIDE_L4(int16_t, int64_t);
GATHER_STRIDE_L4(uint16_t, int32_t);
GATHER_STRIDE_L4(uint16_t, int64_t);
GATHER_STRIDE_L4(int32_t, int32_t);
GATHER_STRIDE_L4(int32_t, int64_t);
GATHER_STRIDE_L4(uint32_t, int32_t);
GATHER_STRIDE_L4(uint32_t, int64_t);
GATHER_STRIDE_L4(int64_t, int32_t);
GATHER_STRIDE_L4(int64_t, int64_t);
GATHER_STRIDE_L4(uint64_t, int32_t);
GATHER_STRIDE_L4(uint64_t, int64_t);
GATHER_STRIDE_L4(float, int32_t);
GATHER_STRIDE_L4(float, int64_t);
GATHER_STRIDE_L4(double, int32_t);
GATHER_STRIDE_L4(double, int64_t);


SCATTER_GENERAL_L4(int8_t, uint32_t);
SCATTER_GENERAL_L4(int8_t, uint64_t);
SCATTER_GENERAL_L4(uint8_t, uint32_t);
SCATTER_GENERAL_L4(uint8_t, uint64_t);
SCATTER_GENERAL_L4(int16_t, uint32_t);
SCATTER_GENERAL_L4(int16_t, uint64_t);
SCATTER_GENERAL_L4(uint16_t, uint32_t);
SCATTER_GENERAL_L4(uint16_t, uint64_t);
SCATTER_GENERAL_L4(int32_t, uint32_t);
SCATTER_GENERAL_L4(int32_t, uint64_t);
SCATTER_GENERAL_L4(uint32_t, uint32_t);
SCATTER_GENERAL_L4(uint32_t, uint64_t);
SCATTER_GENERAL_L4(int64_t, uint32_t);
SCATTER_GENERAL_L4(int64_t, uint64_t);
SCATTER_GENERAL_L4(uint64_t, uint32_t);
SCATTER_GENERAL_L4(uint64_t, uint64_t);
SCATTER_GENERAL_L4(float, uint32_t);
SCATTER_GENERAL_L4(float, uint64_t);
SCATTER_GENERAL_L4(double, uint32_t);
SCATTER_GENERAL_L4(double, uint64_t);


SCATTER_BASE_OFFSETS_L4(int8_t, int32_t);
SCATTER_BASE_OFFSETS_L4(int8_t, int64_t);
SCATTER_BASE_OFFSETS_L4(uint8_t, int32_t);
SCATTER_BASE_OFFSETS_L4(uint8_t, int64_t);
SCATTER_BASE_OFFSETS_L4(int16_t, int32_t);
SCATTER_BASE_OFFSETS_L4(int16_t, int64_t);
SCATTER_BASE_OFFSETS_L4(uint16_t, int32_t);
SCATTER_BASE_OFFSETS_L4(uint16_t, int64_t);
SCATTER_BASE_OFFSETS_L4(int32_t, int32_t);
SCATTER_BASE_OFFSETS_L4(int32_t, int64_t);
SCATTER_BASE_OFFSETS_L4(uint32_t, int32_t);
SCATTER_BASE_OFFSETS_L4(uint32_t, int64_t);
SCATTER_BASE_OFFSETS_L4(int64_t, int32_t);
SCATTER_BASE_OFFSETS_L4(int64_t, int64_t);
SCATTER_BASE_OFFSETS_L4(uint64_t, int32_t);
SCATTER_BASE_OFFSETS_L4(uint64_t, int64_t);
SCATTER_BASE_OFFSETS_L4(float, int32_t);
SCATTER_BASE_OFFSETS_L4(float, int64_t);
SCATTER_BASE_OFFSETS_L4(double, int32_t);
SCATTER_BASE_OFFSETS_L4(double, int64_t);

SCATTER_STRIDE_L4(int8_t, int32_t);
SCATTER_STRIDE_L4(int8_t, int64_t);
SCATTER_STRIDE_L4(uint8_t, int32_t);
SCATTER_STRIDE_L4(uint8_t, int64_t);
SCATTER_STRIDE_L4(int16_t, int32_t);
SCATTER_STRIDE_L4(int16_t, int64_t);
SCATTER_STRIDE_L4(uint16_t, int32_t);
SCATTER_STRIDE_L4(uint16_t, int64_t);
SCATTER_STRIDE_L4(int32_t, int32_t);
SCATTER_STRIDE_L4(int32_t, int64_t);
SCATTER_STRIDE_L4(uint32_t, int32_t);
SCATTER_STRIDE_L4(uint32_t, int64_t);
SCATTER_STRIDE_L4(int64_t, int32_t);
SCATTER_STRIDE_L4(int64_t, int64_t);
SCATTER_STRIDE_L4(uint64_t, int32_t);
SCATTER_STRIDE_L4(uint64_t, int64_t);
SCATTER_STRIDE_L4(float, int32_t);
SCATTER_STRIDE_L4(float, int64_t);
SCATTER_STRIDE_L4(double, int32_t);
SCATTER_STRIDE_L4(double, int64_t);


#endif //DOXYGEN_SHOULD_SKIP_THIS


//  5. masked load/masked store

//Masked load/store is implemented based on gather_base_offsets/scatter_base_offsets
//Here we only use offsets with 32bit

MASKED_LOAD_STORE_L4(int8_t);
MASKED_LOAD_STORE_L4(uint8_t);
MASKED_LOAD_STORE_L4(int16_t);
MASKED_LOAD_STORE_L4(uint16_t);
MASKED_LOAD_STORE_L4(int32_t);
MASKED_LOAD_STORE_L4(uint32_t);
MASKED_LOAD_STORE_L4(int64_t);
MASKED_LOAD_STORE_L4(uint64_t);
MASKED_LOAD_STORE_L4(float);
MASKED_LOAD_STORE_L4(double);

//////////////////////////////////////////////////////////////
//
// Mask type (i1) interfaces
//
//////////////////////////////////////////////////////////////

// 1. mask construction
/**
 * @brief Check any element of the mask is non-zero
 * @param mask the svec<4,bool> type vector
 * @return true is at least one element in the mask is true
 */
static FORCEINLINE bool svec_any_true(const svec<4,bool>& mask) {
  return (_mm_movemask_ps(mask.v)!=0);
}

/**
 * @brief Check all elements of the mask are non-zero
 * @param mask the svec<4,bool> type vector
 * @return true is all elements in the mask are true
 */
static FORCEINLINE bool svec_all_true(const svec<4,bool>& mask) {
  return (_mm_movemask_ps(mask.v)==0xF);
}


/**
 * @brief Check none elements of the mask are zero
 * @param mask the svec<4,bool> type vector
 * @return true is all elements in the mask are false
 */
static FORCEINLINE bool svec_none_true(const svec<4,bool>& mask) {
  return (_mm_movemask_ps(mask.v)==0);
}

// 2. bit operations

/**
 * @brief return a & b
 */
static FORCEINLINE svec<4,bool> svec_and(svec<4,bool> a, svec<4,bool> b) {
  return _mm_and_ps(a.v, b.v);
}


/**
 * @brief return a | b
 */
static FORCEINLINE svec<4,bool> svec_or(svec<4,bool> a, svec<4,bool> b) {
  return _mm_or_ps(a.v, b.v);
}

/**
 * @brief return a ^ b
 */
static FORCEINLINE svec<4,bool> svec_xor(svec<4,bool> a, svec<4,bool> b) {
  return _mm_xor_ps(a.v, b.v);
}

/**
 * @brief return ~a
 */
static FORCEINLINE svec<4,bool> svec_not(svec<4,bool> a) {
  __m128 allon = _mm_castsi128_ps(_mm_set1_epi32(-1));
  return _mm_xor_ps(a.v, allon);
}

/**
 * @brief Change a mask type (i1 vector) to a uint64_t integer
 * The method is only used for compatibility of ISPC
 * @param mask the svec<4,bool> type vector
 * @return a uint64_t integer to represent the mask
 */
static FORCEINLINE uint64_t svec_movmsk(svec<4,bool> mask) {
  return (uint64_t)_mm_movemask_ps(mask.v);
}


//////////////////////////////////////////////////////////////
//
// General data operation interfaces
//
//////////////////////////////////////////////////////////////
// 1. Unary

#define UNARY_OP_OPT(STYPE, NAME, OP)\
static FORCEINLINE svec<LANES,STYPE> NAME(svec<LANES,STYPE> a) { \
  return OP(a.v); \
}

/**
 * @brief macros for 64bit object, i64/u64/double
 */
#define UNARY_OP_OPT64(STYPE, NAME, OP)\
static FORCEINLINE svec<LANES,STYPE> NAME(svec<LANES,STYPE> a) { \
  return  svec<LANES,STYPE>(OP(a.v[0]), OP(a.v[1]));  \
}

// neg operation
static FORCEINLINE svec<4,int8_t>  svec_neg(svec<4,int8_t> a) {
  return  _mm_sub_epi8(_mm_setzero_si128(), (a.v));
}
static FORCEINLINE svec<4,uint8_t>  svec_neg(svec<4,uint8_t> a) {
  return  _mm_sub_epi8(_mm_setzero_si128(), (a.v));
}
static FORCEINLINE svec<4,int16_t>  svec_neg(svec<4,int16_t> a) {
  return  _mm_sub_epi16(_mm_setzero_si128(), (a.v));
}
static FORCEINLINE svec<4,uint16_t>  svec_neg(svec<4,uint16_t> a) {
  return  _mm_sub_epi16(_mm_setzero_si128(), (a.v));
}
static FORCEINLINE svec<4,int32_t>  svec_neg(svec<4,int32_t> a) {
  return  _mm_sub_epi32(_mm_setzero_si128(), (a.v));
}
static FORCEINLINE svec<4,uint32_t>  svec_neg(svec<4,uint32_t> a) {
  return  _mm_sub_epi32(_mm_setzero_si128(), (a.v));
}
//it seems i64/f/d sse overload's "-" operator.
UNARY_OP_OPT64(int64_t, svec_neg, -);
UNARY_OP_OPT64(uint64_t, svec_neg, -);
UNARY_OP_OPT(float, svec_neg, -);
UNARY_OP_OPT64(double, svec_neg, -);

//  2. Math unary
//round
static FORCEINLINE svec<4,float> svec_round(svec<4,float> a) {
  return _mm_round_ps(a.v, _MM_FROUND_TO_NEAREST_INT | _MM_FROUND_NO_EXC);
}
static FORCEINLINE svec<4,double> svec_round(svec<4,double> a) {
  return svec<4,double>(
      _mm_round_pd(a.v[0], _MM_FROUND_TO_NEAREST_INT | _MM_FROUND_NO_EXC),
      _mm_round_pd(a.v[1], _MM_FROUND_TO_NEAREST_INT | _MM_FROUND_NO_EXC));
}
//floor
UNARY_OP_OPT(float, svec_floor, _mm_floor_ps);
UNARY_OP_OPT64(double, svec_floor, _mm_floor_pd);
//ceil
UNARY_OP_OPT(float, svec_ceil, _mm_ceil_ps);
UNARY_OP_OPT64(double, svec_ceil, _mm_ceil_pd);
//reverse 1/
static FORCEINLINE svec<4,float> svec_rcp(svec<4,float> v) {
  __m128 rcp = _mm_rcp_ps(v.v);
  // N-R iteration:
  __m128 m = _mm_mul_ps(v.v, rcp);
  __m128 twominus = _mm_sub_ps(_mm_set1_ps(2.f), m);
  __m128 r = _mm_mul_ps(rcp, twominus);
  return r;
}
UNARY_OP_L4(double, svec_rcp, 1.0/);
//reverse sqrt
static FORCEINLINE svec<4,float> svec_rsqrt(svec<4,float> v) {
  __m128 rsqrt = _mm_rsqrt_ps(v.v);
  // Newton-Raphson iteration to improve precision
  // return 0.5 * rsqrt * (3. - (v * rsqrt) * rsqrt);
  __m128 v_rsqrt = _mm_mul_ps(rsqrt, v.v);
  __m128 v_r_r = _mm_mul_ps(v_rsqrt, rsqrt);
  __m128 three_sub = _mm_sub_ps(_mm_set1_ps(3.f), v_r_r);
  __m128 rs_mul = _mm_mul_ps(rsqrt, three_sub);
  __m128 half_scale = _mm_mul_ps(_mm_set1_ps(0.5), rs_mul);
  return half_scale;
}
UNARY_OP_L4(double, svec_rsqrt, 1.0/sqrt);
//sqrt
UNARY_OP_OPT(float, svec_sqrt, _mm_sqrt_ps);
UNARY_OP_OPT64(double, svec_sqrt, _mm_sqrt_pd);
//exp - _mm_exp_ps/_mm_exp_pd not in gcc but in ICC
UNARY_OP_L4(float, svec_exp, expf);
UNARY_OP_L4(double, svec_exp, exp);
//log - _mm_log_ps / _mm_log_pd not in gcc but in ICC
UNARY_OP_L4(float, svec_log, logf);
UNARY_OP_L4(double, svec_log, log);
//abs - for all types
UNARY_OP_L4(int8_t, svec_abs, abs<int8_t>);
static FORCEINLINE svec<4,uint8_t>  svec_abs(svec<4,uint8_t> v) { return v;}
UNARY_OP_L4(int16_t, svec_abs, abs<int16_t>);
static FORCEINLINE svec<4,uint16_t>  svec_abs(svec<4,uint16_t> v) { return v;}
UNARY_OP_L4(int32_t, svec_abs, abs<int32_t>);
static FORCEINLINE svec<4,uint32_t>  svec_abs(svec<4,uint32_t> v) { return v;}
UNARY_OP_L4(int64_t, svec_abs, abs<int64_t>);
static FORCEINLINE svec<4,uint64_t>  svec_abs(svec<4,uint64_t> v) { return v;}
//UNARY_OP(float, svec_abs, abs);
static FORCEINLINE svec<4,float> svec_abs(svec<4,float> v) {
  unsigned int x = 0x7fffffff;
  float &f = * (float *)( &x );
  __m128 tmp = _mm_set1_ps(f);
  return _mm_and_ps(v.v, tmp);
}
UNARY_OP_L4(double, svec_abs, abs);

//  3. Binary

#define BINARY_OP_OPT_FUNC(STYPE, STYPE2, NAME, FUNC) \
static FORCEINLINE svec<LANES,STYPE> NAME(svec<LANES,STYPE> a, svec<LANES,STYPE2> b) { \
  return svec<LANES,STYPE>(FUNC(a.v, b.v)); \
}

#define BINARY_OP_OPT_FUNC64(STYPE, STYPE2, NAME, FUNC) \
static FORCEINLINE svec<LANES,STYPE> NAME(svec<LANES,STYPE> a, svec<LANES,STYPE> b) { \
  return svec<LANES,STYPE>(FUNC(a.v[0], b.v[0]), FUNC(a.v[1], b.v[1])); \
}

//add, sub, div, mul.

//add
BINARY_OP_OPT_FUNC(int8_t, int8_t, svec_add, _mm_add_epi8);
BINARY_OP_OPT_FUNC(uint8_t, uint8_t, svec_add, _mm_add_epi8);
BINARY_OP_OPT_FUNC(int16_t, int16_t, svec_add, _mm_add_epi16);
BINARY_OP_OPT_FUNC(uint16_t, uint16_t, svec_add, _mm_add_epi16);
BINARY_OP_OPT_FUNC(int32_t, int32_t, svec_add, _mm_add_epi32);
BINARY_OP_OPT_FUNC(uint32_t, uint32_t, svec_add, _mm_add_epi32);
BINARY_OP_OPT_FUNC64(int64_t, int64_t, svec_add, _mm_add_epi64);
BINARY_OP_OPT_FUNC64(uint64_t, uint64_t, svec_add, _mm_add_epi64);
BINARY_OP_OPT_FUNC(float, float, svec_add, _mm_add_ps);
BINARY_OP_OPT_FUNC64(double, double, svec_add, _mm_add_pd);

//sub
BINARY_OP_OPT_FUNC(int8_t, int8_t, svec_sub, _mm_sub_epi8);
BINARY_OP_OPT_FUNC(uint8_t, uint8_t, svec_sub, _mm_sub_epi8);
BINARY_OP_OPT_FUNC(int16_t, int16_t, svec_sub, _mm_sub_epi16);
BINARY_OP_OPT_FUNC(uint16_t, uint16_t, svec_sub, _mm_sub_epi16);
BINARY_OP_OPT_FUNC(int32_t, int32_t, svec_sub, _mm_sub_epi32);
BINARY_OP_OPT_FUNC(uint32_t, uint32_t, svec_sub, _mm_sub_epi32);
BINARY_OP_OPT_FUNC64(int64_t, int64_t, svec_sub, _mm_sub_epi64);
BINARY_OP_OPT_FUNC64(uint64_t, uint64_t, svec_sub, _mm_sub_epi64);
BINARY_OP_OPT_FUNC(float, float, svec_sub, _mm_sub_ps);
BINARY_OP_OPT_FUNC64(double, double, svec_sub, _mm_sub_pd);

//mul
BINARY_OP_L4(int8_t, svec_mul, *);
BINARY_OP_L4(uint8_t, svec_mul, *);
BINARY_OP_L4(int16_t, svec_mul, *);
BINARY_OP_L4(uint16_t, svec_mul, *);
BINARY_OP_OPT_FUNC(int32_t, int32_t, svec_mul, _mm_mullo_epi32);
BINARY_OP_OPT_FUNC(uint32_t, uint32_t, svec_mul, _mm_mullo_epi32);
BINARY_OP_L4(int64_t, svec_mul, *);
BINARY_OP_L4(uint64_t, svec_mul, *);
BINARY_OP_OPT_FUNC(float, float, svec_mul, _mm_mul_ps);
BINARY_OP_OPT_FUNC64(double, double, svec_mul, _mm_mul_pd);

//div - no _mm_idiv_epi32 and _mm_udiv_epi32
BINARY_OP_L4(int8_t, svec_div, /);
BINARY_OP_L4(uint8_t, svec_div, /);
BINARY_OP_L4(int16_t, svec_div, /);
BINARY_OP_L4(uint16_t, svec_div, /);
BINARY_OP_L4(int32_t, svec_div, /);
BINARY_OP_L4(uint32_t, svec_div, /);
BINARY_OP_L4(int64_t, svec_div, /);
BINARY_OP_L4(uint64_t, svec_div, /);
BINARY_OP_OPT_FUNC(float, float, svec_div, _mm_div_ps);
BINARY_OP_OPT_FUNC64(double, double, svec_div, _mm_div_pd);

#define BIN_VEC_SCAL(STYPE) \
static FORCEINLINE svec<LANES,STYPE> svec_add_scalar(svec<LANES,STYPE> a, STYPE s) { \
  return svec_add(a, svec<LANES,STYPE>(s)); \
} \
static FORCEINLINE svec<LANES,STYPE> svec_scalar_add(STYPE s, svec<LANES,STYPE> a) { \
  return svec_add(svec<LANES,STYPE>(s), a); \
} \
static FORCEINLINE svec<LANES,STYPE> svec_sub_scalar(svec<LANES,STYPE> a, STYPE s) { \
  return svec_sub(a, svec<LANES,STYPE>(s)); \
} \
static FORCEINLINE svec<LANES,STYPE> svec_scalar_sub(STYPE s, svec<LANES,STYPE> a) { \
  return svec_sub(svec<LANES,STYPE>(s), a); \
} \
static FORCEINLINE svec<LANES,STYPE> svec_mul_scalar(svec<LANES,STYPE> a, STYPE s) { \
  return svec_mul(a, svec<LANES,STYPE>(s)); \
} \
static FORCEINLINE svec<LANES,STYPE> svec_scalar_mul(STYPE s, svec<LANES,STYPE> a) { \
  return svec_mul(svec<LANES,STYPE>(s), a); \
} \
static FORCEINLINE svec<LANES,STYPE> svec_div_scalar(svec<LANES,STYPE> a, STYPE s) { \
  return svec_div(a, svec<LANES,STYPE>(s)); \
} \
static FORCEINLINE svec<LANES,STYPE> svec_scalar_div(STYPE s, svec<LANES,STYPE> a) { \
  return svec_div(svec<LANES,STYPE>(s), a); \
} \

BIN_VEC_SCAL(int8_t);
BIN_VEC_SCAL(uint8_t);
BIN_VEC_SCAL(int16_t);
BIN_VEC_SCAL(uint16_t);
BIN_VEC_SCAL(int32_t);
BIN_VEC_SCAL(uint32_t);
BIN_VEC_SCAL(int64_t);
BIN_VEC_SCAL(uint64_t);
BIN_VEC_SCAL(float);
BIN_VEC_SCAL(double);


#define INT_BINARY_OP_METHODS(STYPE) \
BINARY_OP_OPT_FUNC(STYPE, STYPE, svec_or, _mm_or_si128); \
BINARY_OP_OPT_FUNC(STYPE, STYPE, svec_and, _mm_and_si128); \
BINARY_OP_OPT_FUNC(STYPE, STYPE, svec_xor, _mm_xor_si128); \
BINARY_OP_L4(STYPE, svec_rem, %); \
BINARY_OP_SCALAR_L4(STYPE, STYPE, svec_rem, %);

#define INT_BINARY_OP_METHODS64(STYPE) \
BINARY_OP_OPT_FUNC64(STYPE, STYPE, svec_or, _mm_or_si128); \
BINARY_OP_OPT_FUNC64(STYPE, STYPE, svec_and, _mm_and_si128); \
BINARY_OP_OPT_FUNC64(STYPE, STYPE, svec_xor, _mm_xor_si128); \
BINARY_OP_L4(STYPE, svec_rem, %); \
BINARY_OP_SCALAR_L4(STYPE, STYPE, svec_rem, %);




INT_BINARY_OP_METHODS(int8_t);
INT_BINARY_OP_METHODS(uint8_t);
INT_BINARY_OP_METHODS(int16_t);
INT_BINARY_OP_METHODS(uint16_t);
INT_BINARY_OP_METHODS(int32_t);
INT_BINARY_OP_METHODS(uint32_t);
INT_BINARY_OP_METHODS64(int64_t);
INT_BINARY_OP_METHODS64(uint64_t);


//power only for float - cannot find _mm_pow_ps/pd in gcc
BINARY_OP_FUNC(float, svec_pow, powf);
BINARY_OP_FUNC(double, svec_pow, pow);

//shift left
BINARY_OP2_L4(int8_t, uint8_t, svec_shl, <<);
BINARY_OP2_L4(uint8_t, uint8_t, svec_shl, <<);
BINARY_OP2_L4(int16_t, uint16_t, svec_shl, <<);
BINARY_OP2_L4(uint16_t, uint16_t, svec_shl, <<);
BINARY_OP2_L4(int32_t, uint32_t, svec_shl, <<);
BINARY_OP2_L4(uint32_t, uint32_t, svec_shl, <<);
BINARY_OP2_L4(int64_t, uint64_t, svec_shl, <<);
BINARY_OP2_L4(uint64_t, uint64_t, svec_shl, <<);

//shift right
BINARY_OP2_L4(int8_t, uint8_t, svec_shr, >>);
BINARY_OP2_L4(uint8_t, uint8_t, svec_shr, >>);
BINARY_OP2_L4(int16_t, uint16_t, svec_shr, >>);
BINARY_OP2_L4(uint16_t, uint16_t, svec_shr, >>);
BINARY_OP2_L4(int32_t, uint32_t, svec_shr, >>);
BINARY_OP2_L4(uint32_t, uint32_t, svec_shr, >>);
BINARY_OP2_L4(int64_t, uint64_t, svec_shr, >>);
BINARY_OP2_L4(uint64_t, uint64_t, svec_shr, >>);

// shift scalar left
BINARY_OP_SCALAR_L4(int8_t, int32_t, svec_shl, <<);
BINARY_OP_SCALAR_L4(uint8_t, int32_t, svec_shl, <<);
//BINARY_OP_SCALAR_L4(int16_t, int32_t, svec_shl, <<);
static FORCEINLINE svec<4,int16_t>  svec_shl(svec<4,int16_t> a, int32_t s) {
  return svec<4,int16_t>(_mm_sll_epi16(a.v, _mm_set_epi32(0, 0, 0, s)));                            \
}

//BINARY_OP_SCALAR_L4(uint16_t, int32_t, svec_shl, <<);
static FORCEINLINE svec<4,uint16_t>  svec_shl(svec<4,uint16_t> a, int32_t s) {
  return svec<4,uint16_t>(_mm_sll_epi16(a.v, _mm_set_epi32(0, 0, 0, s)));                            \
}
//BINARY_OP_SCALAR_L4(int32_t, int32_t, svec_shl, <<);
static FORCEINLINE svec<4,int32_t>  svec_shl(svec<4,int32_t> a, int32_t s) {
  return svec<4,int32_t>(_mm_sll_epi32(a.v, _mm_set_epi32(0, 0, 0, s)));                            \
}
//BINARY_OP_SCALAR_L4(uint32_t, int32_t, svec_shl, <<);
static FORCEINLINE svec<4,uint32_t>  svec_shl(svec<4,uint32_t> a, int32_t s) {
  return svec<4,uint32_t>(_mm_sll_epi32(a.v, _mm_set_epi32(0, 0, 0, s)));                            \
}
//BINARY_OP_SCALAR_L4(int64_t, int32_t, svec_shl, <<);
static FORCEINLINE svec<4,int64_t>  svec_shl(svec<4,int64_t> a, int32_t s) {
  __m128i amt = _mm_set_epi32(0, 0, 0, s);
  return svec<4,int64_t>(_mm_sll_epi64(a.v[0], amt),
                    _mm_sll_epi64(a.v[1], amt));
}
//BINARY_OP_SCALAR_L4(uint64_t, int32_t, svec_shl, <<);
static FORCEINLINE svec<4,uint64_t>  svec_shl(svec<4,uint64_t> a, int32_t s) {
  __m128i amt = _mm_set_epi32(0, 0, 0, s);
  return svec<4,uint64_t>(_mm_sll_epi64(a.v[0], amt),
                    _mm_sll_epi64(a.v[1], amt));
}

//shift sclar right
BINARY_OP_SCALAR_L4(int8_t, int32_t, svec_shr, >>);
BINARY_OP_SCALAR_L4(uint8_t, int32_t, svec_shr, >>);
//BINARY_OP_SCALAR_L4(int16_t, int32_t, svec_shr, >>);
static FORCEINLINE svec<4,int16_t>  svec_shr(svec<4,int16_t> a, int32_t s) {
  return svec<4,int16_t>(_mm_sra_epi16(a.v, _mm_set_epi32(0, 0, 0, s)));                            \
}
//BINARY_OP_SCALAR_L4(uint16_t, int32_t, svec_shr, >>);
static FORCEINLINE svec<4,uint16_t>  svec_shr(svec<4,uint16_t> a, int32_t s) {
  return svec<4,uint16_t>(_mm_srl_epi16(a.v, _mm_set_epi32(0, 0, 0, s)));                            \
}
//BINARY_OP_SCALAR_L4(int32_t, int32_t, svec_shr, >>);
static FORCEINLINE svec<4,int32_t>  svec_shr(svec<4,int32_t> a, int32_t s) {
  return svec<4,int32_t>(_mm_sra_epi32(a.v, _mm_set_epi32(0, 0, 0, s)));                            \
}
//BINARY_OP_SCALAR_L4(uint32_t, int32_t, svec_shr, >>);
static FORCEINLINE svec<4,uint32_t>  svec_shr(svec<4,uint32_t> a, int32_t s) {
  return svec<4,uint32_t>(_mm_srl_epi32(a.v, _mm_set_epi32(0, 0, 0, s)));                            \
}
BINARY_OP_SCALAR_L4(int64_t, int32_t, svec_shr, >>);
//BINARY_OP_SCALAR_L4(uint64_t, int32_t, svec_shr, >>);
static FORCEINLINE svec<4,uint64_t>  svec_shr(svec<4,uint64_t> a, int32_t s) {
  __m128i amt = _mm_set_epi32(0, 0, 0, s);
  return svec<4,uint64_t>(_mm_srl_epi64(a.v[0], amt),
                    _mm_srl_epi64(a.v[1], amt));
}

//  4. Ternary

//madd / msub for only int32/u32/float/double
#define TERNERY_OPT(STYPE) \
/**
 * @brief vector multiply and add operation. return a * b + c.
 */ \
FORCEINLINE svec<LANES,STYPE> svec_madd(svec<LANES,STYPE> a, svec<LANES,STYPE> b, svec<LANES,STYPE> c) { \
  return a * b + c;\
} \
/**
 * @brief vector multiply and add operation. return a * b - c.
 */ \
FORCEINLINE svec<LANES,STYPE> svec_msub(svec<LANES,STYPE> a, svec<LANES,STYPE> b, svec<LANES,STYPE> c) { \
  return a * b - c;\
} \
/**
 * @brief vector multiply and add operation. return -(a * b - c).
 */ \
FORCEINLINE svec<LANES,STYPE> svec_nmsub(svec<LANES,STYPE> a, svec<LANES,STYPE> b, svec<LANES,STYPE> c) { \
  return c - a * b ;\
}


TERNERY_OPT(int32_t);
TERNERY_OPT(uint32_t);
TERNERY_OPT(int64_t);
TERNERY_OPT(uint64_t);
TERNERY_OPT(float);
TERNERY_OPT(double);


//  5. Max/Min
BINARY_OP_FUNC_L4(int8_t, svec_max, max<int8_t>);
BINARY_OP_FUNC_L4(uint8_t, svec_max, max<uint8_t>);
BINARY_OP_FUNC_L4(int16_t, svec_max, max<int16_t>);
BINARY_OP_FUNC_L4(uint16_t, svec_max, max<uint16_t>);
BINARY_OP_OPT_FUNC(int32_t, int32_t, svec_max, _mm_max_epi32);
BINARY_OP_OPT_FUNC(uint32_t, uint32_t, svec_max, _mm_max_epu32);
BINARY_OP_FUNC_L4(int64_t, svec_max, max<int64_t>);
BINARY_OP_FUNC_L4(uint64_t, svec_max, max<uint64_t>);
BINARY_OP_OPT_FUNC(float, float, svec_max, _mm_max_ps);
BINARY_OP_OPT_FUNC64(double, double, svec_max, _mm_max_pd);

BINARY_OP_FUNC_L4(int8_t, svec_min, min<int8_t>);
BINARY_OP_FUNC_L4(uint8_t, svec_min, min<uint8_t>);
BINARY_OP_FUNC_L4(int16_t, svec_min, min<int16_t>);
BINARY_OP_FUNC_L4(uint16_t, svec_min, min<uint16_t>);
BINARY_OP_OPT_FUNC(int32_t, int32_t, svec_min, _mm_min_epi32);
BINARY_OP_OPT_FUNC(uint32_t, uint32_t, svec_min, _mm_min_epu32);
BINARY_OP_FUNC_L4(int64_t, svec_min, min<int64_t>);
BINARY_OP_FUNC_L4(uint64_t, svec_min, min<uint64_t>);
BINARY_OP_OPT_FUNC(float, float, svec_min, _mm_min_ps);
BINARY_OP_OPT_FUNC64(double, double, svec_min, _mm_min_pd);



//6. Reduce

#define MAX_MIN_REDUCE_METHODS(STYPE) \
BINARY_OP_REDUCE_FUNC(STYPE, svec_reduce_add, add<STYPE>); \
BINARY_OP_REDUCE_FUNC(STYPE, svec_reduce_max, max<STYPE>); \
BINARY_OP_REDUCE_FUNC(STYPE, svec_reduce_min, min<STYPE>); \


MAX_MIN_REDUCE_METHODS(int8_t);
MAX_MIN_REDUCE_METHODS(uint8_t);
MAX_MIN_REDUCE_METHODS(int16_t);
MAX_MIN_REDUCE_METHODS(uint16_t);
MAX_MIN_REDUCE_METHODS(int32_t);
MAX_MIN_REDUCE_METHODS(uint32_t);
MAX_MIN_REDUCE_METHODS(int64_t);
MAX_MIN_REDUCE_METHODS(uint64_t);
MAX_MIN_REDUCE_METHODS(float);
MAX_MIN_REDUCE_METHODS(double);

FORCEINLINE svec<LANES,float> svec_preduce_add(svec<LANES,float> v0, svec<LANES,float> v1, svec<LANES,float> v2, svec<LANES,float> v3) {
  __m128 s0 = _mm_hadd_ps(v0.v,v1.v);
  __m128 s1 = _mm_hadd_ps(v2.v,v3.v);
  __m128 s = _mm_hadd_ps(s0, s1);
  return svec<LANES,float>(s);
}

FORCEINLINE svec<4,double> svec_preduce_add(svec<4,double> v0, svec<4,double> v1, svec<4,double> v2, svec<4,double> v3) {
  __m128d s00 = _mm_add_pd(v0.v[0], v0.v[1]);
  __m128d s01 = _mm_add_pd(v1.v[0], v1.v[1]);
  __m128d s02 = _mm_add_pd(v2.v[0], v2.v[1]);
  __m128d s03 = _mm_add_pd(v3.v[0], v3.v[1]);

  __m128d s0 = _mm_hadd_pd(s00, s01);
  __m128d s1 = _mm_hadd_pd(s02, s03);

  return svec<4,double>(s0, s1);
}

//  7. Compare
/**
 * @brief element by element comparison of two svec_vec4_i1 type object
 * @param a
 * @param b
 * @return a svec_vec4_i1 object
 */
static FORCEINLINE svec<4,bool> svec_equal(svec<4,bool> a, svec<4,bool> b) {
  return _mm_castsi128_ps(_mm_cmpeq_epi32(_mm_castps_si128(a.v), _mm_castps_si128(b.v)));
}

static FORCEINLINE svec<4,bool> svec_not_equal(svec<4,bool> a, svec<4,bool> b) {
  return svec_not(svec_equal(a, b));
}

static FORCEINLINE svec<4,bool> svec_equal(svec<4,int8_t> a, svec<4,int8_t> b) {
  __m128i cmp = _mm_cmpeq_epi8(a.v, b.v);
  return svec<4,bool>(_mm_extract_epi8(cmp, 0),
                   _mm_extract_epi8(cmp, 1),
                   _mm_extract_epi8(cmp, 2),
                   _mm_extract_epi8(cmp, 3));
}
static FORCEINLINE svec<4,bool> svec_not_equal(svec<4,int8_t> a, svec<4,int8_t> b) {
  return ~(a == b);
}

static FORCEINLINE svec<4,bool> svec_less_than(svec<4,int8_t> a, svec<4,int8_t> b) {
  __m128i cmp = _mm_cmplt_epi8(a.v, b.v);
  return svec<4,bool>(_mm_extract_epi8(cmp, 0),
                   _mm_extract_epi8(cmp, 1),
                   _mm_extract_epi8(cmp, 2),
                   _mm_extract_epi8(cmp, 3));
}

static FORCEINLINE svec<4,bool> svec_less_equal(svec<4,int8_t> a, svec<4,int8_t> b) {
  return (a < b) | (a == b);
}

static FORCEINLINE svec<4,bool> svec_greater_than(svec<4,int8_t> a, svec<4,int8_t> b) {
  __m128i cmp = _mm_cmpgt_epi8(a.v, b.v);
  return svec<4,bool>(_mm_extract_epi8(cmp, 0),
                   _mm_extract_epi8(cmp, 1),
                   _mm_extract_epi8(cmp, 2),
                   _mm_extract_epi8(cmp, 3));
}

static FORCEINLINE svec<4,bool> svec_greater_equal(svec<4,int8_t> a, svec<4,int8_t> b) {
  return (a > b) | (a == b);
}
CMP_ALL_MASKED_OP(int8_t);

static FORCEINLINE svec<4,bool> svec_equal(svec<4,uint8_t> a, svec<4,uint8_t> b) {
  __m128i cmp = _mm_cmpeq_epi8(a.v, b.v);
  return svec<4,bool>(_mm_extract_epi8(cmp, 0),
                   _mm_extract_epi8(cmp, 1),
                   _mm_extract_epi8(cmp, 2),
                   _mm_extract_epi8(cmp, 3));
}

static FORCEINLINE svec<4,bool> svec_not_equal(svec<4,uint8_t> a, svec<4,uint8_t> b) {
  return ~(a == b);
}

CMP_OP_L4(uint8_t, less_than, <);
CMP_OP_L4(uint8_t, less_equal, <=);
CMP_OP_L4(uint8_t, greater_than, >);
CMP_OP_L4(uint8_t, greater_equal, >=);
CMP_ALL_MASKED_OP(uint8_t);


static FORCEINLINE svec<4,bool> svec_equal(svec<4,int16_t> a, svec<4,int16_t> b) {
  __m128i cmp = _mm_cmpeq_epi16(a.v, b.v);
  return svec<4,bool>(_mm_extract_epi16(cmp, 0),
                   _mm_extract_epi16(cmp, 1),
                   _mm_extract_epi16(cmp, 2),
                   _mm_extract_epi16(cmp, 3));
}
static FORCEINLINE svec<4,bool> svec_not_equal(svec<4,int16_t> a, svec<4,int16_t> b) {
  return ~(a == b);
}

static FORCEINLINE svec<4,bool> svec_less_than(svec<4,int16_t> a, svec<4,int16_t> b) {
  __m128i cmp = _mm_cmplt_epi16(a.v, b.v);
  return svec<4,bool>(_mm_extract_epi16(cmp, 0),
                   _mm_extract_epi16(cmp, 1),
                   _mm_extract_epi16(cmp, 2),
                   _mm_extract_epi16(cmp, 3));
}

static FORCEINLINE svec<4,bool> svec_less_equal(svec<4,int16_t> a, svec<4,int16_t> b) {
  return (a < b) | (a == b);
}

static FORCEINLINE svec<4,bool> svec_greater_than(svec<4,int16_t> a, svec<4,int16_t> b) {
  __m128i cmp = _mm_cmpgt_epi16(a.v, b.v);
  return svec<4,bool>(_mm_extract_epi16(cmp, 0),
                   _mm_extract_epi16(cmp, 1),
                   _mm_extract_epi16(cmp, 2),
                   _mm_extract_epi16(cmp, 3));
}

static FORCEINLINE svec<4,bool> svec_greater_equal(svec<4,int16_t> a, svec<4,int16_t> b) {
  return (a > b) | (a == b);
}
CMP_ALL_MASKED_OP(int16_t);

static FORCEINLINE svec<4,bool> svec_equal(svec<4,uint16_t> a, svec<4,uint16_t> b) {
  __m128i cmp = _mm_cmpeq_epi16(a.v, b.v);
  return svec<4,bool>(_mm_extract_epi16(cmp, 0),
                   _mm_extract_epi16(cmp, 1),
                   _mm_extract_epi16(cmp, 2),
                   _mm_extract_epi16(cmp, 3));
}

static FORCEINLINE svec<4,bool> svec_not_equal(svec<4,uint16_t> a, svec<4,uint16_t> b) {
  return ~(a == b);
}

CMP_OP_L4(uint16_t, less_than, <);
CMP_OP_L4(uint16_t, less_equal, <=);
CMP_OP_L4(uint16_t, greater_than, >);
CMP_OP_L4(uint16_t, greater_equal, >=);
CMP_ALL_MASKED_OP(uint16_t);


static FORCEINLINE svec<4,bool> svec_equal(svec<4,int32_t> a, svec<4,int32_t> b) {
  __m128i cmp = _mm_cmpeq_epi32(a.v, b.v);
  return svec<4,bool>(_mm_extract_epi32(cmp, 0),
                   _mm_extract_epi32(cmp, 1),
                   _mm_extract_epi32(cmp, 2),
                   _mm_extract_epi32(cmp, 3));
}
static FORCEINLINE svec<4,bool> svec_not_equal(svec<4,int32_t> a, svec<4,int32_t> b) {
  return ~(a == b);
}

static FORCEINLINE svec<4,bool> svec_less_than(svec<4,int32_t> a, svec<4,int32_t> b) {
  __m128i cmp = _mm_cmplt_epi32(a.v, b.v);
  return svec<4,bool>(_mm_extract_epi32(cmp, 0),
                   _mm_extract_epi32(cmp, 1),
                   _mm_extract_epi32(cmp, 2),
                   _mm_extract_epi32(cmp, 3));
}

static FORCEINLINE svec<4,bool> svec_less_equal(svec<4,int32_t> a, svec<4,int32_t> b) {
  return (a < b) | (a == b);
}

static FORCEINLINE svec<4,bool> svec_greater_than(svec<4,int32_t> a, svec<4,int32_t> b) {
  __m128i cmp = _mm_cmpgt_epi32(a.v, b.v);
  return svec<4,bool>(_mm_extract_epi32(cmp, 0),
                   _mm_extract_epi32(cmp, 1),
                   _mm_extract_epi32(cmp, 2),
                   _mm_extract_epi32(cmp, 3));
}

static FORCEINLINE svec<4,bool> svec_greater_equal(svec<4,int32_t> a, svec<4,int32_t> b) {
  return (a > b) | (a == b);
}
CMP_ALL_MASKED_OP(int32_t);

static FORCEINLINE svec<4,bool> svec_equal(svec<4,uint32_t> a, svec<4,uint32_t> b) {
  __m128i cmp = _mm_cmpeq_epi32(a.v, b.v);
  return svec<4,bool>(_mm_extract_epi32(cmp, 0),
                   _mm_extract_epi32(cmp, 1),
                   _mm_extract_epi32(cmp, 2),
                   _mm_extract_epi32(cmp, 3));
}

static FORCEINLINE svec<4,bool> svec_not_equal(svec<4,uint32_t> a, svec<4,uint32_t> b) {
  return ~(a == b);
}

CMP_OP_L4(uint32_t, less_than, <);
CMP_OP_L4(uint32_t, less_equal, <=);
CMP_OP_L4(uint32_t, greater_than, >);
CMP_OP_L4(uint32_t, greater_equal, >=);
CMP_ALL_MASKED_OP(uint32_t);


static FORCEINLINE svec<4,bool> svec_equal(svec<4,int64_t> a, svec<4,int64_t> b) {
  __m128i cmp0 = _mm_cmpeq_epi64(a.v[0], b.v[0]);
  __m128i cmp1 = _mm_cmpeq_epi64(a.v[1], b.v[1]);
  return svec<4,bool>(_mm_shuffle_ps(_mm_castsi128_ps(cmp0), _mm_castsi128_ps(cmp1),
                        _MM_SHUFFLE(2, 0, 2, 0)));
}
static FORCEINLINE svec<4,bool> svec_not_equal(svec<4,int64_t> a, svec<4,int64_t> b) {
  return ~(a == b);
}

static FORCEINLINE svec<4,bool> svec_less_than(svec<4,int64_t> a, svec<4,int64_t> b) {
  return ~(a >= b);
}

static FORCEINLINE svec<4,bool> svec_less_equal(svec<4,int64_t> a, svec<4,int64_t> b) {
  return (a < b) | (a == b);
}

static FORCEINLINE svec<4,bool> svec_greater_than(svec<4,int64_t> a, svec<4,int64_t> b) {
  __m128i cmp0 = _mm_cmpgt_epi64(a.v[0], b.v[0]);
  __m128i cmp1 = _mm_cmpgt_epi64(a.v[1], b.v[1]);
  return svec<4,bool>(_mm_shuffle_ps(_mm_castsi128_ps(cmp0), _mm_castsi128_ps(cmp1),
                        _MM_SHUFFLE(2, 0, 2, 0)));
}

static FORCEINLINE svec<4,bool> svec_greater_equal(svec<4,int64_t> a, svec<4,int64_t> b) {
  return (a > b) | (a == b);
}
CMP_ALL_MASKED_OP(int64_t);

static FORCEINLINE svec<4,bool> svec_equal(svec<4,uint64_t> a, svec<4,uint64_t> b) {
  __m128i cmp0 = _mm_cmpeq_epi64(a.v[0], b.v[0]);
  __m128i cmp1 = _mm_cmpeq_epi64(a.v[1], b.v[1]);
  return svec<4,bool>(_mm_shuffle_ps(_mm_castsi128_ps(cmp0), _mm_castsi128_ps(cmp1),
                        _MM_SHUFFLE(2, 0, 2, 0)));
}

static FORCEINLINE svec<4,bool> svec_not_equal(svec<4,uint64_t> a, svec<4,uint64_t> b) {
  return ~(a == b);
}

CMP_OP_L4(uint64_t, less_than, <);
CMP_OP_L4(uint64_t, less_equal, <=);
CMP_OP_L4(uint64_t, greater_than, >);
CMP_OP_L4(uint64_t, greater_equal, >=);
CMP_ALL_MASKED_OP(uint64_t);


static FORCEINLINE svec<4,bool> svec_equal(svec<4,float> a, svec<4,float> b) {
  return _mm_cmpeq_ps(a.v, b.v);
}

static FORCEINLINE svec<4,bool> svec_not_equal(svec<4,float> a, svec<4,float> b) {
  return _mm_cmpneq_ps(a.v, b.v);
}

static FORCEINLINE svec<4,bool> svec_less_than(svec<4,float> a, svec<4,float> b) {
  return _mm_cmplt_ps(a.v, b.v);
}

static FORCEINLINE svec<4,bool> svec_less_equal(svec<4,float> a, svec<4,float> b) {
  return _mm_cmple_ps(a.v, b.v);
}

static FORCEINLINE svec<4,bool> svec_greater_than(svec<4,float> a, svec<4,float> b) {
  return _mm_cmpgt_ps(a.v, b.v);
}

static FORCEINLINE svec<4,bool> svec_greater_equal(svec<4,float> a, svec<4,float> b) {
  return _mm_cmpge_ps(a.v, b.v);
}

CMP_ALL_MASKED_OP(float);

static FORCEINLINE svec<4,bool> svec_equal(svec<4,double> a, svec<4,double> b) {
  __m128d cmp0 = _mm_cmpeq_pd(a.v[0], b.v[0]);
  __m128d cmp1 = _mm_cmpeq_pd(a.v[1], b.v[1]);
  return _mm_shuffle_ps(_mm_castpd_ps(cmp0), _mm_castpd_ps(cmp1),
                        _MM_SHUFFLE(2, 0, 2, 0));
}

static FORCEINLINE svec<4,bool> svec_not_equal(svec<4,double> a, svec<4,double> b) {
  __m128d cmp0 = _mm_cmpneq_pd(a.v[0], b.v[0]);
  __m128d cmp1 = _mm_cmpneq_pd(a.v[1], b.v[1]);
  return _mm_shuffle_ps(_mm_castpd_ps(cmp0), _mm_castpd_ps(cmp1),
                        _MM_SHUFFLE(2, 0, 2, 0));
}

static FORCEINLINE svec<4,bool> svec_less_than(svec<4,double> a, svec<4,double> b) {
  __m128d cmp0 = _mm_cmplt_pd(a.v[0], b.v[0]);
  __m128d cmp1 = _mm_cmplt_pd(a.v[1], b.v[1]);
  return _mm_shuffle_ps(_mm_castpd_ps(cmp0), _mm_castpd_ps(cmp1),
                        _MM_SHUFFLE(2, 0, 2, 0));
}

static FORCEINLINE svec<4,bool> svec_less_equal(svec<4,double> a, svec<4,double> b) {
  __m128d cmp0 = _mm_cmple_pd(a.v[0], b.v[0]);
  __m128d cmp1 = _mm_cmple_pd(a.v[1], b.v[1]);
  return _mm_shuffle_ps(_mm_castpd_ps(cmp0), _mm_castpd_ps(cmp1),
                        _MM_SHUFFLE(2, 0, 2, 0));
}

static FORCEINLINE svec<4,bool> svec_greater_than(svec<4,double> a, svec<4,double> b) {
  __m128d cmp0 = _mm_cmpgt_pd(a.v[0], b.v[0]);
  __m128d cmp1 = _mm_cmpgt_pd(a.v[1], b.v[1]);
  return _mm_shuffle_ps(_mm_castpd_ps(cmp0), _mm_castpd_ps(cmp1),
                        _MM_SHUFFLE(2, 0, 2 ,0));
}

static FORCEINLINE svec<4,bool> svec_greater_equal(svec<4,double> a, svec<4,double> b) {
  __m128d cmp0 = _mm_cmpge_pd(a.v[0], b.v[0]);
  __m128d cmp1 = _mm_cmpge_pd(a.v[1], b.v[1]);
  return _mm_shuffle_ps(_mm_castpd_ps(cmp0), _mm_castpd_ps(cmp1),
                        _MM_SHUFFLE(2, 0, 2, 0));
}

CMP_ALL_MASKED_OP(double);



//  8. Cast

/**
 * Here we provide the full cast combinations.
 * Some may have fast impl
 */


/**
 * @brief cast based on directly change the __mm object type type
 */
#define CAST_OPT(SFROM, STO)        \
template <class T> static T svec_cast(svec<LANES,SFROM> val);     \
/**
 * @brief cast val from FROM type to TO type.
 */ \
template <> FORCEINLINE svec<LANES,STO> svec_cast<svec<LANES,STO> >(svec<LANES,SFROM> val) {      \
    return svec<LANES,STO>((val.v)); \
}

/**
 * @brief cast based on directly change the __vector type
 */
#define CAST_OPT64(SFROM, STO)        \
template <class T> static T svec_cast(svec<LANES,SFROM> val);     \
/**
 * @brief cast val from FROM type to TO type.
 */ \
template <> FORCEINLINE svec<LANES,STO> svec_cast<svec<LANES,STO> >(svec<LANES,SFROM> val) {      \
    return svec<LANES,STO>((val.v[0]),(val.v[1])); \
}


//i1 -> all
//CAST_L4(svec<4,bool>, svec<4,bool>, bool);
//CAST_L4(svec<4,bool>, svec<4,int8_t>, int8_t);  //better way: packing
template <class T> static T svec_cast(svec<4,bool> val);
/**
 * @brief cast val from svec<4,bool> type to svec<4,int8_t> type.
 */
template <> FORCEINLINE svec<4,int8_t> svec_cast<svec<4,int8_t> >(svec<4,bool> val) {
  return svec_select(val, svec<4,int8_t>(0xff), svec<4,int8_t>(0));
}
//CAST_L4(svec<4,bool>, svec<4,uint8_t>, uint8_t);  //better way: packing
template <class T> static T svec_cast(svec<4,bool> val);
/**
 * @brief cast val from svec<4,bool> type to svec<4,uint8_t> type.
 */
template <> FORCEINLINE svec<4,uint8_t> svec_cast<svec<4,uint8_t> >(svec<4,bool> val) {
  return svec_select(val, svec<4,uint8_t>(0xff), svec<4,uint8_t>(0));
}
//CAST_L4(svec<4,bool>, svec<4,int16_t>, int16_t);  //better way: packing
template <class T> static T svec_cast(svec<4,bool> val);
/**
 * @brief cast val from svec<4,bool> type to svec<4,int16_t> type.
 */
template <> FORCEINLINE svec<4,int16_t> svec_cast<svec<4,int16_t> >(svec<4,bool> val) {
  return svec_select(val, svec<4,int16_t>(0xffff), svec<4,int16_t>(0));
}
//CAST_L4(svec<4,bool>, svec<4,uint16_t>, uint16_t); //better way: packing
template <class T> static T svec_cast(svec<4,bool> val);
/**
 * @brief cast val from svec<4,bool> type to svec<4,uint16_t> type.
 */
template <> FORCEINLINE svec<4,uint16_t> svec_cast<svec<4,uint16_t> >(svec<4,bool> val) {
  return svec_select(val, svec<4,uint16_t>(0xffff), svec<4,uint16_t>(0));
}
//CAST_L4(svec<4,bool>, svec<4,int32_t>, int32_t);
template <class T> static T svec_cast(svec<4,bool> val);
/**
 * @brief cast val from svec<4,bool> type to svec<4,int32_t> type.
 */
template <> FORCEINLINE svec<4,int32_t> svec_cast<svec<4,int32_t> >(svec<4,bool> val) {
  return _mm_castps_si128(val.v);
}
//CAST_L4(svec<4,bool>, svec<4,uint32_t>, uint32_t);
template <class T> static T svec_cast(svec<4,bool> val);
/**
 * @brief cast val from svec<4,bool> type to svec<4,uint32_t> type.
 */
template <> FORCEINLINE svec<4,uint32_t> svec_cast<svec<4,uint32_t> >(svec<4,bool> val) {
  return _mm_and_si128(_mm_castps_si128(val.v), _mm_set1_epi32(-1));
}
CAST_L4(bool, int64_t); //better way: unpack, singed ext
CAST_L4(bool, uint64_t);//better way: unpack, singed ext
//CAST_L4(bool, float); //si to fp call
template <class T> static T svec_cast(svec<4,bool> val);
/**
 * @brief cast val from svec<4,bool> type to svec<4,float> type.
 */
template <> FORCEINLINE svec<4,float> svec_cast<svec<4,float> >(svec<4,bool> val) {
  return svec_select(val, svec<4,float>(4294967295.), svec<4,float>(0));
}
//CAST_L4(svec<4,bool>, svec<4,double>, double);
template <class T> static T svec_cast(svec<4,bool> val);
/**
 * @brief cast val from svec<4,bool> type to svec<4,float> type.
 */
template <> FORCEINLINE svec<4,double> svec_cast<svec<4,double> >(svec<4,bool> val) {
  return svec_select(val, svec<4,double>(4294967295.), svec<4,double>(0));
}

//i8 -> all
CAST_L4(int8_t, bool);
//CAST_L4(int8_t, int8_t);
CAST_OPT(int8_t, uint8_t);
CAST_L4(int8_t, int16_t); //better way, use vec_unpackh
CAST_L4(int8_t, uint16_t); //better way, sext + zero mask and
CAST_L4(int8_t, int32_t); //better way, use twice vec_unpack
CAST_L4(int8_t, uint32_t); //better way, use unpack + zero mask
CAST_L4(int8_t, int64_t);
CAST_L4(int8_t, uint64_t);
CAST_L4(int8_t, float);
CAST_L4(int8_t, double);

//u8 -> all
CAST_L4(uint8_t, bool);
CAST_OPT(uint8_t, int8_t);
//CAST_L4(uint8_t, uint8_t);
CAST_L4(uint8_t, int16_t); //better way, use unpack + zero mask
CAST_L4(uint8_t, uint16_t); //better way use unpack + zero mask
CAST_L4(uint8_t, int32_t);
CAST_L4(uint8_t, uint32_t);
CAST_L4(uint8_t, int64_t);
CAST_L4(uint8_t, uint64_t);
CAST_L4(uint8_t, float);
CAST_L4(uint8_t, double);

//i16 -> all
CAST_L4(int16_t, bool);
CAST_L4(int16_t, int8_t); //could use pack
CAST_L4(int16_t, uint8_t); //could use pack
//CAST_L4(int16_t, int16_t);
CAST_OPT(int16_t, uint16_t);
CAST_L4(int16_t, int32_t); //use unpack
CAST_L4(int16_t, uint32_t); //use unpack and zeromaskout
CAST_L4(int16_t, int64_t);
CAST_L4(int16_t, uint64_t);
CAST_L4(int16_t, float);
CAST_L4(int16_t, double);

//u16 -> all
CAST_L4(uint16_t, bool);
CAST_L4(uint16_t, int8_t);
CAST_L4(uint16_t, uint8_t);
CAST_OPT(uint16_t, int16_t);
//CAST_L4(uint16_t, uint16_t);
CAST_L4(uint16_t, int32_t); //use unpack +mask
CAST_L4(uint16_t, uint32_t); //use unpack + mask
CAST_L4(uint16_t, int64_t);
CAST_L4(uint16_t, uint64_t);
CAST_L4(uint16_t, float);
CAST_L4(uint16_t, double);

//i32 -> all
CAST_L4(int32_t, bool);
CAST_L4(int32_t, int8_t);
CAST_L4(int32_t, uint8_t);
CAST_L4(int32_t, int16_t);
CAST_L4(int32_t, uint16_t);
//CAST_L4(int32_t, svec<4,int32_t>, int32_t);
CAST_OPT(int32_t, uint32_t);
CAST_L4(int32_t, int64_t); //use p8 unpack
CAST_L4(int32_t, uint64_t); //use p8 unpack
//CAST_L4(svec<4,int32_t>, svec<4,float>, float); //use ctf
template <class T> static T svec_cast(svec<4,int32_t> val);
/**
 * @brief cast val from svec<4,int32_t> type to svec<4,float> type.
 */
template <> FORCEINLINE svec<4,float> svec_cast<svec<4,float> > (svec<4,int32_t> val) {
  return _mm_cvtepi32_ps(val.v);
}
//CAST_L4(svec<4,int32_t>, svec<4,double>, double);
template <class T> static T svec_cast(svec<4,int32_t> val);
/**
 * @brief cast val from svec<4,int32_t> type to svec<4,double> type.
 */
template <> FORCEINLINE svec<4,double> svec_cast<svec<4,double> > (svec<4,int32_t> val) {
  __m128d r0 = _mm_cvtepi32_pd(val.v);
  __m128 shuf = _mm_shuffle_ps(_mm_castsi128_ps(val.v),
                               _mm_castsi128_ps(val.v),
                               _MM_SHUFFLE(3, 2, 3, 2));
  __m128d r1 = _mm_cvtepi32_pd(_mm_castps_si128(shuf));
  return svec<4,double>(r0, r1);
}

//u32 -> all
CAST_L4(uint32_t, bool);
CAST_L4(uint32_t, int8_t);
CAST_L4(uint32_t, uint8_t);
CAST_L4(uint32_t, int16_t);
CAST_L4(uint32_t, uint16_t);
CAST_OPT(uint32_t, int32_t);
//CAST_L4(uint32_t, uint32_t);
CAST_L4(uint32_t, int64_t); //use p8 unpack
CAST_L4(uint32_t, uint64_t); //use p8 unpack
CAST_L4(uint32_t, float);
CAST_L4(uint32_t, double);

//i64-> all
CAST_L4(int64_t, bool);
CAST_L4(int64_t, int8_t);
CAST_L4(int64_t, uint8_t);
CAST_L4(int64_t, int16_t);
CAST_L4(int64_t, uint16_t);
CAST_L4(int64_t, int32_t); //use p8 trunk
CAST_L4(int64_t, uint32_t); //use p8 trunk
//CAST_L4(int64_t, int64_t);
CAST_OPT64(int64_t, uint64_t);
CAST_L4(int64_t, float);
CAST_L4(int64_t, double);

//u64 -> all
CAST_L4(uint64_t, bool);
CAST_L4(uint64_t, int8_t);
CAST_L4(uint64_t, uint8_t);
CAST_L4(uint64_t, int16_t);
CAST_L4(uint64_t, uint16_t);
CAST_L4(uint64_t, int32_t); //use p8 pack
CAST_L4(uint64_t, uint32_t); //use p8 pack
CAST_OPT64(uint64_t, int64_t);
//CAST_L4(uint64_t, uint64_t);
CAST_L4(uint64_t, float);
CAST_L4(uint64_t, double);

//float -> all
CAST_L4(float, bool);
CAST_L4(float, int8_t); //use cts + pack+pack
CAST_L4(float, uint8_t); //use ctu + pack + pack
CAST_L4(float, int16_t); //use cts + pack
CAST_L4(float, uint16_t); //use ctu + pack
//CAST_L4(svec<4,float>, int32_t);//use cts
template <class T> static T svec_cast(svec<4,float> val);
/**
 * @brief cast val from svec<4,float> type to svec<4,int32_t> type.
 */
template <> FORCEINLINE svec<4,int32_t> svec_cast<svec<4,int32_t> >(svec<4,float> val) {
  return _mm_cvttps_epi32(val.v);
}
CAST_L4(float, uint32_t); //use ctu
CAST_L4(float, int64_t);
CAST_L4(float, uint64_t);
//CAST_L4(float, float);
//CAST_L4(float, double);
template <class T> static T svec_cast(svec<4,float> val);
/**
 * @brief cast val from svec<4,float> type to svec<4,double> type.
 */
template <> FORCEINLINE svec<4,double> svec_cast<svec<4,double> >(svec<4,float> val) {
  return svec<4,double>(_mm_cvtps_pd(val.v),
                  _mm_cvtps_pd(_mm_shuffle_ps(val.v, val.v,
                                              _MM_SHUFFLE(3, 2, 3, 2))));
}

//double -> all
CAST_L4(double, bool);
CAST_L4(double, int8_t);
CAST_L4(double, uint8_t);
CAST_L4(double, int16_t);
CAST_L4(double, uint16_t);
//CAST_L4(double, int32_t);
template <class T> static T svec_cast(svec<4,double> val);
/**
 * @brief cast val from svec<4,double> type to svec<4,int32_t> type.
 */
template <> FORCEINLINE svec<4,int32_t> svec_cast<svec<4,int32_t> >(svec<4,double> val) {
  __m128i r0 = _mm_cvtpd_epi32(val.v[0]);
  __m128i r1 = _mm_cvtpd_epi32(val.v[1]);
  return _mm_castps_si128(_mm_shuffle_ps(_mm_castsi128_ps(r0), _mm_castsi128_ps(r1),
                                         _MM_SHUFFLE(1, 0, 1, 0)));
}

CAST_L4(double, uint32_t);
CAST_L4(double, int64_t);
CAST_L4(double, uint64_t);
//CAST_L4(double, float);
template <class T> static T svec_cast(svec<4,double> val);
/**
 * @brief cast val from svec<4,double> type to svec<4,float> type.
 */
template <> FORCEINLINE svec<4,float> svec_cast<svec<4,float> >(svec<4,double> val) {
  __m128 r0 = _mm_cvtpd_ps(val.v[0]);
  __m128 r1 = _mm_cvtpd_ps(val.v[1]);
  return _mm_shuffle_ps(r0, r1, _MM_SHUFFLE(1, 0, 1, 0));
}
//CAST_L4(svec<4,double>, double);

////casts bits, only for 32bit i32/u32 <--> float i64/u64<-->double


/**
 * @brief cast based on directly change the __vector type
 */
#define CAST_BITS_OPT(SFROM, STO, func)        \
template <class T> static T svec_cast_bits(svec<LANES,SFROM> val);     \
/**
 * @brief bit cast val from FROM type to TO type.
 */ \
template <> FORCEINLINE svec<LANES,STO> svec_cast_bits<svec<LANES,STO> >(svec<LANES,SFROM> val) {      \
    return svec<LANES,STO>(func(val.v)); \
}

/**
 * @brief cast based on directly change the __vector type
 */
#define CAST_BITS_OPT64(SFROM, STO, func)        \
template <class T> static T svec_cast_bits(svec<LANES,SFROM> val);     \
/**
 * @brief bit cast val from FROM type to TO type.
 */ \
template <> FORCEINLINE svec<LANES,STO> svec_cast_bits<svec<LANES,STO> >(svec<LANES,SFROM> val) {      \
    return svec<LANES,STO>(func(val.v[0]), func(val.v[1])); \
}


/**
 * @brief cast based on directly change the __vector type
 */
CAST_BITS_OPT(int32_t, float, _mm_castsi128_ps);
CAST_BITS_OPT(uint32_t, float, _mm_castsi128_ps);
CAST_BITS_OPT(float, int32_t, _mm_castps_si128);
CAST_BITS_OPT(float, uint32_t, _mm_castps_si128);

CAST_BITS_OPT64(int64_t, double, _mm_castsi128_pd);
CAST_BITS_OPT64(uint64_t, double, _mm_castsi128_pd);
CAST_BITS_OPT64(double, int64_t, _mm_castpd_si128);
CAST_BITS_OPT64(double, uint64_t, _mm_castpd_si128);


//////////////////////////////////////////////////////////////
//
// Class operations based on the above interfaces
//
//////////////////////////////////////////////////////////////

/**
 * @brief this macro uses sse specific intrinsics to do extract, insert
 */
#define SUBSCRIPT_FUNC_IMPL_SSE(STYPE) \
FORCEINLINE STYPE& svec<LANES,STYPE>::operator[](int index) { \
  return ((STYPE *)&v)[index];   \
} \
const FORCEINLINE STYPE  svec<LANES,STYPE>::operator[](int index) const { \
  return svec_extract(*this, index); \
}

//add the impl of i1's
FORCEINLINE void svec<4,bool>::Helper::operator=(uint32_t value) {
  svec_insert(m_self, m_index, value);
}
FORCEINLINE void svec<4,bool>::Helper::operator=(svec<4,bool>::Helper helper) {
  svec_insert(m_self, m_index, helper.operator uint32_t());
}
FORCEINLINE svec<4,bool>::Helper::operator uint32_t() const {
  return svec_extract(*m_self, m_index);
}
const FORCEINLINE uint32_t svec<4,bool>::operator[](int index) const {
  return svec_extract(*this, index);
}
SUBSCRIPT_FUNC_IMPL_SSE(int8_t);
SUBSCRIPT_FUNC_IMPL_SSE(uint8_t);
SUBSCRIPT_FUNC_IMPL_SSE(int16_t);
SUBSCRIPT_FUNC_IMPL_SSE(uint16_t);
SUBSCRIPT_FUNC_IMPL_SSE(int32_t);
SUBSCRIPT_FUNC_IMPL_SSE(uint32_t);
SUBSCRIPT_FUNC_IMPL_SSE(int64_t);
SUBSCRIPT_FUNC_IMPL_SSE(uint64_t);
SUBSCRIPT_FUNC_IMPL_SSE(float);
SUBSCRIPT_FUNC_IMPL_SSE(double);

/**
 * @brief Check if any element in the mask vector is true. 
 * \note This is a reduction operation that returns a scalar value.
 * @return true if at least one element in the mask vector is true, otherwise false
 */
FORCEINLINE bool svec<4,bool>::any_true() { return svec_any_true(*this); }

/**
 * @brief Check if all the elements in the mask vector is true. 
 * \note This is a reduction operation that returns a scalar value.
 * @return true if all the elements in the mask vector are true, otherwise false.
 */
FORCEINLINE bool svec<4,bool>::all_true() { return svec_all_true(*this); }

/**
 * @brief Check all the elements in the mask vector is false. 
 * \note This is a reduction operation that returns a scalar value.
 * @return true if all the elements in the mask vector are false, otherwise false.
 */
FORCEINLINE bool svec<4,bool>::none_true() { return svec_none_true(*this); }

/**
 * @brief Element-wise bit-wise compliment operator. E.g., "~a"
 * @return the result of bit-wise compliment as a boolean vector. 
 */
FORCEINLINE svec<4,bool> svec<4,bool>::operator~() { return svec_not(*this); }

/**
 * @brief Element-wise bit-wise OR operator. E.g., "a | b"
 * @param[in] a a boolean vector
 * @return the result of bit-wise OR as a boolean vector.
 */
FORCEINLINE svec<4,bool> svec<4,bool>::operator|(svec<4,bool> a) { return svec_or(*this, a); }
/**
 * @brief Element-wise bit-wise AND operator. E.g., "a & b"
 * @param[in] a a boolean vector
 * @return the result of bit-wise AND as a boolean vector.
 */
FORCEINLINE svec<4,bool> svec<4,bool>::operator&(svec<4,bool> a) { return svec_and(*this, a); }
/**
 * @brief Element-wise bit-wise XOR operator. E.g., "a ^ b"
 * @param[in] a a boolean vector
 * @return the result of bit-wise XOR as a boolean vector.
 */
FORCEINLINE svec<4,bool> svec<4,bool>::operator^(svec<4,bool> a) { return svec_xor(*this, a); }
/**
 * @brief Element-wise bit-wise not operator. E.g., "!a"
 * @return the result of bit-wise compliment as a boolean vector.
 */
FORCEINLINE svec<4,bool> svec<4,bool>::operator!() { return svec_not(*this); }

/**
 * @brief Element-wise boolean AND operator. E.g., "a && b"
 * @param[in] a a boolean vector
 * @return the result of boolean AND as a boolean vector.
 */
FORCEINLINE svec<4,bool> svec<4,bool>::operator&&(svec<4,bool> a) { return svec_and(*this, a); }
/**
 * @brief Element-wise boolean OR operator. E.g., "a || b"
 * @param[in] a a boolean vector
 * @return the result of boolean OR as a boolean vector.
 */
FORCEINLINE svec<4,bool> svec<4,bool>::operator||(svec<4,bool> a) { return svec_or(*this, a); }
/**
 * @brief Element-wise compare equal. E.g., "a == b"
 * @param[in] a a boolean vector
 * @return the result of compare-equal as a boolean vector
 */
FORCEINLINE svec<4,bool> svec<4,bool>::operator ==(svec<4,bool> a) {
    return svec_equal(*this, a);
}

/**
 * @brief Element-wise compare not equal, return a bool vector. E.g. "a != b"
 * @param[in] a a boolean vector
 * @return the result of compare-not-equal as a boolean vector
 */
FORCEINLINE svec<4,bool> svec<4,bool>::operator !=(svec<4,bool> a) {
    return svec_not_equal(*this, a);
}

VEC_CMP_IMPL(int8_t);
VEC_CMP_IMPL(uint8_t);
VEC_CMP_IMPL(int16_t);
VEC_CMP_IMPL(uint16_t);
VEC_CMP_IMPL(int32_t);
VEC_CMP_IMPL(uint32_t);
VEC_CMP_IMPL(int64_t);
VEC_CMP_IMPL(uint64_t);
VEC_CMP_IMPL(float);
VEC_CMP_IMPL(double);

MVEC_CLASS_METHOD_IMPL(bool);
VEC_CLASS_METHOD_IMPL(int8_t);
VEC_CLASS_METHOD_IMPL(uint8_t);
VEC_CLASS_METHOD_IMPL(int16_t);
VEC_CLASS_METHOD_IMPL(uint16_t);
VEC_CLASS_METHOD_IMPL(int32_t);
VEC_CLASS_METHOD_IMPL(uint32_t);
VEC_CLASS_METHOD_IMPL(int64_t);
VEC_CLASS_METHOD_IMPL(uint64_t);
VEC_CLASS_METHOD_IMPL(float);
VEC_CLASS_METHOD_IMPL(double);

VEC_INT_CLASS_METHOD_IMPL(int8_t, uint8_t);
VEC_INT_CLASS_METHOD_IMPL(uint8_t, uint8_t);
VEC_INT_CLASS_METHOD_IMPL(int16_t, uint16_t);
VEC_INT_CLASS_METHOD_IMPL(uint16_t, uint16_t);
VEC_INT_CLASS_METHOD_IMPL(int32_t, uint32_t);
VEC_INT_CLASS_METHOD_IMPL(uint32_t, uint32_t);
VEC_INT_CLASS_METHOD_IMPL(int64_t, uint64_t);
VEC_INT_CLASS_METHOD_IMPL(uint64_t, uint64_t);

VEC_FLOAT_CLASS_METHOD_IMPL(float);
VEC_FLOAT_CLASS_METHOD_IMPL(double);

#undef LANES
} //end of namespace vsx4
#endif /* POWER_VSX4_H_ */

